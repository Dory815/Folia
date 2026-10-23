"""Generate and run the pinned md4c diagnostic subset from official failures."""
import argparse
from collections import Counter
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
DEFAULT_OFFICIAL_REPORT = ROOT / 'build' / 'markdown-conformance' / 'results.json'
DEFAULT_OUTPUT = ROOT / 'build' / 'markdown-conformance' / 'md4c'
DEFAULT_EXECUTABLE = ROOT / 'build' / 'core' / 'FoliaMd4cReferenceTests.exe'
EXPECTED_EXAMPLES = 652

sys.path.insert(0, str(HERE))
from canonical_html import canonical as canonical_html


def load_lock():
    return json.loads((HERE / 'sources.lock.json').read_text(encoding='utf-8'))


def verify_official_report(path):
    report = json.loads(path.read_text(encoding='utf-8'))
    lock = load_lock()
    spec = next(item for item in lock['files'] if item['name'] == 'spec.json')
    errors = []
    if report.get('status') != 'completed':
        errors.append(f"official report status must be 'completed', got {report.get('status')!r}")
    if report.get('exit_code') != 1:
        errors.append(f"official report exit_code must be 1, got {report.get('exit_code')!r}")
    if report.get('fixture_count') != EXPECTED_EXAMPLES:
        errors.append(f'official report fixture_count must be {EXPECTED_EXAMPLES}')
    source_hash = (report.get('source') or {}).get('spec_json_sha256')
    if source_hash != spec['sha256']:
        errors.append('official report spec.json SHA-256 does not match sources.lock.json')
    rows = report.get('results')
    if not isinstance(rows, list):
        errors.append('official report results must be an array')
        rows = []
    ids = []
    by_id = {}
    for row in rows:
        if not isinstance(row, dict) or 'example' not in row:
            errors.append('official report contains a result without an example ID')
            continue
        try:
            example = int(row['example'])
        except (TypeError, ValueError):
            errors.append(f'official report has invalid example ID {row.get("example")!r}')
            continue
        ids.append(example)
        by_id[example] = row
    id_counts = Counter(ids)
    duplicates = sorted(example for example, count in id_counts.items() if count > 1)
    missing = sorted(set(range(1, EXPECTED_EXAMPLES + 1)) - set(ids))
    unexpected = sorted(set(ids) - set(range(1, EXPECTED_EXAMPLES + 1)))
    if duplicates:
        errors.append(f'official report has duplicate example IDs: {duplicates}')
    if missing:
        errors.append(f'official report omits example IDs: {missing}')
    if unexpected:
        errors.append(f'official report has out-of-range example IDs: {unexpected}')
    for example in range(1, EXPECTED_EXAMPLES + 1):
        row = by_id.get(example)
        if row is None:
            continue
        if row.get('reported') is not True:
            errors.append(f'official example {example} was not reported by the formal test')
        if row.get('semantic_match') not in (True, False):
            errors.append(f'official example {example} has no strict semantic result')
    failed_ids = [example for example in range(1, EXPECTED_EXAMPLES + 1)
                  if example in by_id and by_id[example].get('semantic_match') is False]
    if not failed_ids:
        errors.append('official report has no strict semantic failures to compare')
    if errors:
        raise ValueError('Invalid official CommonMark report:\n- ' + '\n- '.join(errors))
    return report, by_id, failed_ids


def generate_include(path, failed_ids):
    lines = [
        '// Generated from strict semantic failures in the verified CommonMark 0.31.2 report.',
        '// This is a diagnostic subset for the pinned md4c reference renderer.',
        'static constexpr int md4c_failed_examples[] = {',
    ]
    for start in range(0, len(failed_ids), 12):
        lines.append('    ' + ', '.join(str(example) for example in failed_ids[start:start + 12]) + ',')
    lines.append('};')
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('\n'.join(lines) + '\n', encoding='utf-8')


def read_reference_jsonl(path):
    records = []
    malformed = []
    if not path.exists():
        return records, [{'line': 0, 'error': 'reference test did not create JSONL report'}]
    required = {'example', 'section', 'match', 'expected_html', 'actual_html', 'return_code'}
    for line_number, line in enumerate(path.read_text(encoding='utf-8').splitlines(), 1):
        if not line.strip():
            continue
        try:
            row = json.loads(line)
            if not isinstance(row, dict):
                raise ValueError('record must be a JSON object')
            missing = sorted(required - set(row))
            if missing:
                raise ValueError('record lacks required fields: ' + ', '.join(missing))
            row['example'] = int(row['example'])
            if not isinstance(row['match'], bool):
                raise ValueError('match must be a boolean')
            row['return_code'] = int(row['return_code'])
            records.append(row)
        except (ValueError, TypeError, json.JSONDecodeError) as error:
            malformed.append({'line': line_number, 'error': str(error), 'raw': line})
    return records, malformed


def first_tree_difference(expected, actual, path='$'):
    if type(expected) is not type(actual):
        return {'path': path, 'expected': expected, 'actual': actual}
    if isinstance(expected, dict):
        for key in sorted(set(expected) | set(actual), key=str):
            child_path = f'{path}.{key}'
            if key not in expected:
                return {'path': child_path, 'expected': '<missing>', 'actual': actual[key]}
            if key not in actual:
                return {'path': child_path, 'expected': expected[key], 'actual': '<missing>'}
            diff = first_tree_difference(expected[key], actual[key], child_path)
            if diff:
                return diff
        return None
    if isinstance(expected, (list, tuple)):
        for index, (left, right) in enumerate(zip(expected, actual)):
            diff = first_tree_difference(left, right, f'{path}[{index}]')
            if diff:
                return diff
        if len(expected) != len(actual):
            index = min(len(expected), len(actual))
            return {
                'path': f'{path}[{index}]',
                'expected': expected[index] if index < len(expected) else '<missing>',
                'actual': actual[index] if index < len(actual) else '<missing>',
            }
        return None
    if expected != actual:
        return {'path': path, 'expected': expected, 'actual': actual}
    return None


def semantic_comparison(expected_html, actual_html):
    expected_tree = canonical_html(expected_html)
    actual_tree = canonical_html(actual_html)
    return expected_tree == actual_tree, first_tree_difference(expected_tree, actual_tree)


def fenced(value):
    value = json.dumps(value, ensure_ascii=False)
    longest = max((len(run) for run in re.findall(r'`+', value)), default=0)
    fence = '`' * max(3, longest + 1)
    return f'{fence}json\n{value}\n{fence}'


def markdown_report(report):
    summary = report['summary']
    lines = [
        '# md4c diagnostic comparison',
        '',
        f"Status: **{report['status']}**. Reference test exit code: **{report.get('exit_code')}**.",
        f"Official source: CommonMark {report['source']['commonmark_version']} spec.json SHA-256 `{report['source']['spec_json_sha256']}`.",
        f"Reference renderer: md4c commit `{report['source']['md4c_commit']}` ({report['source']['md4c_license']} license).",
        '',
        'This report contains only examples that failed the official strict semantic comparison. Expected HTML is copied from that verified official report. The raw `match` result preserves exact HTML string comparison; `semantic_match` uses the same canonical HTML projection as the official report. A raw HTML formatting difference can fail the exact comparison while preserving canonical semantic equality; such a formatting difference does not show a CommonMark syntax non-conformance. md4c is an investigation aid and is not the CommonMark conformance criterion.',
        '',
        '## Subset totals',
        '',
        f"- Official failing examples selected: {report['subset_count']}.",
        f"- Exact HTML: {summary['exact_pass']} match, {summary['exact_diff']} differ.",
        f"- Canonical semantic HTML: {summary['semantic_pass']} match, {summary['semantic_diff']} differ; formatting-only differences: {summary['formatting_only']}.",
        f"- Reference records: {summary['reported']}; unreported: {summary['unreported']}; nonzero md_html return codes: {summary['return_code_failures']}.",
        f"- Missing IDs: {len(report['missing_examples'])}; duplicate IDs: {len(report['duplicate_examples'])}; unexpected IDs: {len(report['unexpected_examples'])}; malformed records: {len(report['malformed_records'])}.",
        '',
        '## Differences',
        '',
    ]
    differences = [row for row in report['results']
                   if row.get('match') is False or row.get('return_code') not in (None, 0)]
    if not differences:
        lines.append('No differing reference records were reported.')
    for row in differences:
        if row.get('semantic_match') is True:
            semantic_status = 'MATCH (formatting-only under the shared canonical projection)'
        elif row.get('semantic_match') is False:
            semantic_status = 'DIFFERENT'
        else:
            semantic_status = 'UNREPORTED'
        lines.extend([
            f"### Example {row['example']} — {row['section']}",
            '',
            f"Exact HTML match: {row.get('match')}; canonical semantic match: {semantic_status}.",
            f"md_html return code: {row.get('return_code')}",
        ])
        semantic_diff = row.get('first_semantic_difference')
        if semantic_diff:
            lines.append(f"First semantic tree difference: `{semantic_diff['path']}`")
        lines.extend([
            '',
            '**Expected HTML (official CommonMark report):**',
            '',
            fenced(row.get('expected_html')),
            '',
            '**Actual HTML (md4c):**',
            '',
            fenced(row.get('actual_html')),
            '',
        ])
    lines.extend(['Full per-example records are in `results.json`; raw reference output is in `runner.log` when the executable was run directly. A nonzero executable exit remains unchanged, including exit 1 when exact HTML assertions fail.'])
    return '\n'.join(lines) + '\n'


def run_reference(args, filters):
    official, official_by_id, selected_ids = verify_official_report(args.official_report)
    args.output.mkdir(parents=True, exist_ok=True)
    jsonl_path = args.output / 'results.jsonl'
    report_path = args.output / 'results.json'
    markdown_path = args.output / 'results.md'
    log_path = args.output / 'runner.log'
    timed_out = False
    launch_error = None
    if args.from_jsonl:
        if filters:
            raise ValueError('Boost.UT filters cannot be used with --from-jsonl')
        previous_report = {}
        if report_path.exists():
            previous_report = json.loads(report_path.read_text(encoding='utf-8'))
        exit_code = args.exit_code if args.exit_code is not None else previous_report.get('exit_code')
        if exit_code not in (0, 1):
            raise ValueError('--from-jsonl requires --exit-code 0 or 1, or a prior results.json with that exit code')
        raw, malformed = read_reference_jsonl(args.from_jsonl)
        elapsed = 0.0
        status = 'completed'
    else:
        jsonl_path.unlink(missing_ok=True)
        command = [str(args.executable.resolve()), *filters]
        env = os.environ.copy()
        env['FOLIA_MD4C_REPORT'] = str(jsonl_path.resolve())
        started = time.time()
        stdout = stderr = ''
        exit_code = None
        try:
            completed = subprocess.run(
                command,
                cwd=ROOT,
                env=env,
                text=True,
                encoding='utf-8',
                errors='replace',
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=args.timeout,
                check=False,
            )
            stdout, stderr = completed.stdout, completed.stderr
            exit_code = completed.returncode
        except subprocess.TimeoutExpired as error:
            timed_out = True
            stdout = error.stdout.decode('utf-8', errors='replace') if isinstance(error.stdout, bytes) else (error.stdout or '')
            stderr = error.stderr.decode('utf-8', errors='replace') if isinstance(error.stderr, bytes) else (error.stderr or '')
        except OSError as error:
            launch_error = str(error)
        elapsed = time.time() - started
        log_path.write_text(
            'COMMAND: ' + subprocess.list2cmdline(command) + '\n\nSTDOUT:\n' + stdout + '\nSTDERR:\n' + stderr,
            encoding='utf-8',
        )
        if stdout:
            print(stdout, end='' if stdout.endswith('\n') else '\n')
        if stderr:
            print(stderr, file=sys.stderr, end='' if stderr.endswith('\n') else '\n')
        raw, malformed = read_reference_jsonl(jsonl_path)

    indexed = {}
    duplicates = []
    for row in raw:
        example = row['example']
        if example in indexed:
            duplicates.append(example)
        indexed[example] = row
    expected_id_set = set(selected_ids)
    missing = sorted(expected_id_set - set(indexed))
    unexpected = sorted(set(indexed) - expected_id_set)
    results = []
    for example in selected_ids:
        official_row = official_by_id[example]
        reference_row = indexed.get(example)
        result = {
            'example': example,
            'section': official_row.get('section'),
            'match': reference_row.get('match') if reference_row else None,
            'expected_html': official_row.get('expected_html'),
            'actual_html': reference_row.get('actual_html') if reference_row else None,
            'return_code': reference_row.get('return_code') if reference_row else None,
            'error': reference_row.get('error') if reference_row else None,
        }
        if reference_row:
            try:
                semantic_match, semantic_diff = semantic_comparison(
                    result['expected_html'], result['actual_html'])
                result['semantic_match'] = semantic_match
                result['first_semantic_difference'] = semantic_diff
                result['formatting_only'] = result['match'] is False and semantic_match is True
            except Exception as error:
                result['semantic_match'] = None
                result['first_semantic_difference'] = None
                result['formatting_only'] = False
                result['semantic_error'] = str(error)
        else:
            result['semantic_match'] = None
            result['first_semantic_difference'] = None
            result['formatting_only'] = False
        if reference_row and reference_row.get('expected_html') != result['expected_html']:
            result['expected_html_mismatch'] = True
        results.append(result)

    if not args.from_jsonl:
        normal = exit_code in (0, 1) and not timed_out and not launch_error
        status = ('completed' if normal else 'timed_out' if timed_out else
                  'abnormal_exit' if exit_code is not None else 'launch_error')
    report = {
        'status': status,
        'exit_code': exit_code,
        'elapsed_seconds': round(elapsed, 3),
        'timeout_seconds': args.timeout,
        'filtered': bool(filters),
        'filters': filters,
        'offline_report_regeneration': bool(args.from_jsonl),
        'official_report': str(args.official_report.resolve()),
        'subset_count': len(selected_ids),
        'official_failed_examples': selected_ids,
        'source': {
            'commonmark_version': official['source']['commonmark_version'],
            'spec_json_sha256': official['source']['spec_json_sha256'],
            'md4c_commit': official['source']['md4c_commit'],
            'md4c_license': official['source']['md4c_license'],
        },
        'md4c_is_conformance_criterion': False,
        'expected_html_source': 'Verified official CommonMark report, strict semantic failures only.',
        'jsonl_record_count': len(raw),
        'missing_examples': missing,
        'unexpected_examples': unexpected,
        'duplicate_examples': sorted(set(duplicates)),
        'malformed_records': malformed,
        'launch_error': launch_error,
        'summary': {
            'reported': len(indexed),
            'exact_pass': sum(row.get('match') is True for row in results),
            'exact_diff': sum(row.get('match') is False for row in results),
            'semantic_pass': sum(row.get('semantic_match') is True for row in results),
            'semantic_diff': sum(row.get('semantic_match') is False for row in results),
            'formatting_only': sum(row.get('formatting_only') is True for row in results),
            'unreported': sum(row.get('semantic_match') is None for row in results),
            'return_code_failures': sum(row.get('return_code') not in (None, 0) for row in results),
        },
        'results': results,
    }
    report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    markdown_path.write_text(markdown_report(report), encoding='utf-8')
    print(f'md4c diagnostic report: {markdown_path}')
    print(f'JSON report: {report_path}')

    if timed_out:
        print(f'Reference test timed out after {args.timeout:g}s; partial results are recorded.', file=sys.stderr)
        return 124
    if launch_error:
        print(f'Could not launch reference test executable: {launch_error}', file=sys.stderr)
        return 2
    if exit_code not in (0, 1):
        print(f'Reference test exited abnormally with code {exit_code}; partial results are recorded.', file=sys.stderr)
        return exit_code if exit_code is not None else 2
    if missing or unexpected or duplicates or malformed or any(row.get('expected_html_mismatch') for row in results):
        print('Reference JSONL does not contain the complete, consistent official-failure subset.', file=sys.stderr)
        return exit_code if exit_code != 0 else 1
    return exit_code


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--generate', type=Path, help='write the C++ failed-example include and exit')
    parser.add_argument('--official-report', type=Path, default=DEFAULT_OFFICIAL_REPORT)
    parser.add_argument('--executable', type=Path, default=DEFAULT_EXECUTABLE)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument('--from-jsonl', type=Path, help='regenerate reports from existing JSONL without running md4c')
    parser.add_argument('--exit-code', type=int, help='native test exit code when using --from-jsonl')
    parser.add_argument('--timeout', type=float, default=120.0)
    args, filters = parser.parse_known_args()
    if args.timeout <= 0:
        parser.error('--timeout must be greater than zero')
    try:
        if args.generate:
            _, _, failed_ids = verify_official_report(args.official_report)
            generate_include(args.generate, failed_ids)
            print(f'Generated {len(failed_ids)} official-failure IDs: {args.generate}')
            return 0
        return run_reference(args, filters)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(str(error), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
