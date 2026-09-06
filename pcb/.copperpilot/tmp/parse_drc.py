import sys
import re

with open('.copperpilot/tmp/review-drc.rpt', 'r', encoding='utf-8') as f:
    lines = f.readlines()

violations = []
curr = []
for line in lines:
    if line.startswith('['):
        if curr:
            violations.append(''.join(curr))
            curr = []
    if curr or line.startswith('['):
        curr.append(line)
if curr:
    violations.append(''.join(curr))

categories = {}
for v in violations:
    match = re.match(r'\[(.*?)\]', v)
    cat = match.group(1) if match else 'other'
    categories.setdefault(cat, []).append(v)

print(f"Total DRC violations: {len(violations)}")
for cat, vlist in categories.items():
    print(f"\n--- Category: {cat} ({len(vlist)} items) ---")
    for v in vlist[:5]:
        print(v.strip())
        print('---')
    if len(vlist) > 5:
        print(f"... and {len(vlist)-5} more")
