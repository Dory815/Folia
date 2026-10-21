"""Normalize official HTML into semantic trees for the CommonMark tests."""
from html.parser import HTMLParser

VOID = {'area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link',
        'meta', 'param', 'source', 'track', 'wbr'}
CONTAINERS = {'root', 'blockquote', 'ul', 'ol', 'li'}
BLOCKS = CONTAINERS | {'p', 'pre', 'hr', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6'}


class HtmlTree(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.root = ['root', [], []]
        self.stack = [self.root]

    def handle_starttag(self, tag, attrs):
        node = [tag, sorted((k, v or '') for k, v in attrs), []]
        self.stack[-1][2].append(node)
        if tag not in VOID:
            self.stack.append(node)

    def handle_startendtag(self, tag, attrs):
        self.handle_starttag(tag, attrs)
        if tag not in VOID:
            self.handle_endtag(tag)

    def handle_endtag(self, tag):
        for index in range(len(self.stack) - 1, 0, -1):
            if self.stack[index][0] == tag:
                del self.stack[index:]
                return
        self.stack[-1][2].append(['unmatched-close', [('tag', tag)], []])

    def handle_data(self, data):
        children = self.stack[-1][2]
        if children and isinstance(children[-1], str):
            children[-1] += data
        else:
            children.append(data)

    def handle_comment(self, data):
        self.stack[-1][2].append(['comment', [('text', data)], []])

    def handle_decl(self, data):
        self.stack[-1][2].append(['declaration', [('text', data)], []])

    def handle_pi(self, data):
        self.stack[-1][2].append(['processing', [('text', data)], []])


def canonical(html, *, relax_list_paragraphs=False):
    """Ignore HTML formatting between block containers, not inline/code spaces.

    Optional li/p unwrapping is a separately reported diagnostic, because Folia
    currently exposes no list-tightness bit. Strict totals never use it.
    """
    parser = HtmlTree()
    parser.feed(html)
    parser.close()

    def clean(node):
        tag, attrs, children = node
        output = []
        for child in children:
            if isinstance(child, list):
                child = clean(child)
                if relax_list_paragraphs and tag == 'li' and child[0] == 'p':
                    output.extend(child[2])
                    continue
            if isinstance(child, str) and tag in CONTAINERS and not child.strip(' \t\r\n\f\v'):
                continue
            if isinstance(child, str) and output and isinstance(output[-1], str):
                output[-1] += child
            else:
                output.append(child)
        if tag in CONTAINERS:
            for i, child in enumerate(output):
                if not isinstance(child, str):
                    continue
                if i == 0 or isinstance(output[i - 1], list) and output[i - 1][0] in BLOCKS:
                    child = child.lstrip('\n')
                if i + 1 == len(output) or isinstance(output[i + 1], list) and output[i + 1][0] in BLOCKS:
                    child = child.rstrip('\n')
                output[i] = child
        return [tag, attrs, [x for x in output if x != '']]

    return clean(parser.root)

