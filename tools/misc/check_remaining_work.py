#!/usr/bin/env python3
"""Guard for documents/remaining_work.md: every item must be deletable whole.

The file lists only open work, and a finished item is deleted rather than marked done, so each non-blank line
must be a header or a list item (details are child bullets, not prose paragraphs), and no line may exceed
MAX_LINE_LENGTH characters (split it into a parent line plus sub-bullets). Run from the repo root; `--self-test`
runs the checker's own tests.
"""
import re
import sys
from pathlib import Path

DOC = Path("documents/remaining_work.md")
MAX_LINE_LENGTH = 400
LINE_START = re.compile(r"^(?:#{1,6} |\s*[-*] |\s*\d+\. )")


def find_violations(text, filename=str(DOC)):
    problems = []
    for number, line in enumerate(text.splitlines(), 1):
        if not line.strip():
            continue
        prefix = line.strip()[:60]
        if not LINE_START.match(line):
            problems.append(f"{filename}:{number}: `{prefix}...` is neither a header nor a list item; did you mean to make it a `- ` list item (or a child bullet of the item it explains)?")
        elif len(line) > MAX_LINE_LENGTH:
            problems.append(f"{filename}:{number}: `{prefix}...` is {len(line)} characters (limit {MAX_LINE_LENGTH}); split it into a parent line plus one-line child bullets so a finished part can be deleted whole")
    return problems


def self_test():
    good = "# Title\n\n- Item\n  - Child\n    * Grandchild\n1. Numbered\n#### Group\n"
    assert find_violations(good) == [], find_violations(good)
    prose = find_violations("- Item\nA prose paragraph.\n")
    assert len(prose) == 1 and ":2:" in prose[0] and "list item" in prose[0], prose
    long_item = find_violations("- " + "x" * MAX_LINE_LENGTH + "\n")
    assert len(long_item) == 1 and ":1:" in long_item[0] and "split" in long_item[0], long_item
    assert find_violations("- " + "x" * (MAX_LINE_LENGTH - 2) + "\n") == []
    assert find_violations("-not a list item\n") != []
    print("check_remaining_work: self-test passed")


def main():
    if "--self-test" in sys.argv:
        self_test()
        return 0
    problems = find_violations(DOC.read_text(encoding="utf-8"))
    for problem in problems:
        print(problem, file=sys.stderr)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
