'use strict';

const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

function argumentsOf(argv) {
  const options = {};
  for (let i = 2; i < argv.length; i += 1) {
    const key = argv[i];
    if (key === '--check') {
      options.check = true;
      continue;
    }
    if (!['--family', '--input', '--output'].includes(key) || !argv[i + 1]) {
      throw new Error('Usage: node layout-source-table.cjs --family fraction|radical --input witness.json --output table.txt [--check]');
    }
    if (options[key.slice(2)]) throw new Error(`Duplicate option ${key}`);
    options[key.slice(2)] = argv[++i];
  }
  if (!['fraction', 'radical'].includes(options.family) || !options.input || !options.output) {
    throw new Error('Usage: node layout-source-table.cjs --family fraction|radical --input witness.json --output table.txt [--check]');
  }
  return options;
}

function flatten(value, prefix, fields) {
  if (value === null || typeof value !== 'object') {
    fields.set(prefix, value);
    return;
  }
  for (const [key, child] of Object.entries(value)) {
    flatten(child, prefix ? `${prefix}.${key}` : key, fields);
  }
}

function token(value) {
  if (value === null || value === undefined) return '~';
  if (typeof value === 'string') {
    // std::quoted handles quote/backslash escaping; this schema intentionally
    // has no JSON control escapes or ambiguous missing-value string.
    if (value === '~' || /[\u0000-\u001f\u007f]/.test(value)) {
      throw new Error('Source table strings must not contain control characters or the null marker');
    }
    return JSON.stringify(value);
  }
  if (typeof value === 'boolean') return value ? 'true' : 'false';
  if (typeof value === 'number' && Number.isFinite(value)) {
    return Object.is(value, -0) ? '-0' : String(value);
  }
  throw new Error(`Unsupported fixture scalar: ${String(value)}`);
}

function hash(buffer) {
  return crypto.createHash('sha256').update(buffer).digest('hex');
}

function provenance(witness, inputPath, inputBytes) {
  const lines = [
    '# folia-layout-source-v1',
    `# witness ${path.basename(inputPath)} sha256=${hash(inputBytes)}`,
  ];
  for (const group of ['sourceHashes', 'cjsLocks', 'cjsHashes']) {
    for (const [name, digest] of Object.entries(witness[group] || {})) {
      lines.push(`# ${group} ${name} sha256=${digest}`);
    }
  }
  return lines;
}

function render(witness, family, inputPath, inputBytes) {
  const expectedCount = family === 'fraction' ? 288 : 192;
  if (!Array.isArray(witness.cases) || witness.cases.length !== expectedCount) {
    throw new Error(`${family} witness must contain ${expectedCount} cases`);
  }

  const rows = witness.cases.map((item, index) => {
    const id = family === 'fraction' ? index + 1 : item.id;
    if (!Number.isSafeInteger(id) || id < 1) throw new Error(`Invalid case id at row ${index + 1}`);
    const fields = new Map();
    flatten(item.input, 'input', fields);
    flatten(item.expected, 'expected', fields);
    return { id, fields };
  });
  const columns = [];
  const seen = new Set();
  for (const row of rows) {
    for (const key of row.fields.keys()) {
      if (!seen.has(key)) {
        seen.add(key);
        columns.push(key);
      }
    }
  }

  const lines = provenance(witness, inputPath, inputBytes);
  lines.push(['id', ...columns].join(' '));
  for (const row of rows) {
    lines.push([String(row.id), ...columns.map(key => token(row.fields.get(key)))].join(' '));
  }
  return `${lines.join('\n')}\n`;
}

const options = argumentsOf(process.argv);
const inputPath = path.resolve(options.input);
const outputPath = path.resolve(options.output);
const inputBytes = fs.readFileSync(inputPath);
const witness = JSON.parse(inputBytes.toString('utf8'));
const contents = render(witness, options.family, inputPath, inputBytes);
if (options.check) {
  const current = fs.readFileSync(outputPath, 'utf8');
  if (current !== contents) throw new Error(`Fixture differs from generated output: ${outputPath}`);
  const header = contents.split('\n').find(line => line.startsWith('id '));
  process.stdout.write(`verified ${options.family}: ${witness.cases.length} cases, ${header.split(' ').length - 1} data columns\n`);
} else {
  fs.writeFileSync(outputPath, contents);
  process.stdout.write(`wrote ${options.family}: ${witness.cases.length} cases to ${outputPath}\n`);
}
