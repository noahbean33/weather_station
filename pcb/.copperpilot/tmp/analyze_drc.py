import json
import sys

sys.stdout.reconfigure(encoding='utf-8')

with open('.copperpilot/tmp/review-drc.rpt', 'r', encoding='utf-8') as f:
    drc = json.load(f)

print(f"Total DRC violations: {drc.get('total_violations')}")
print(f"Errors: {drc.get('errors')}, Warnings: {drc.get('warnings')}")

unconnected = drc.get('unconnected_items', [])
print(f"\nUnconnected items ({len(unconnected)}):")
for u in unconnected:
    print(f"  - {u.get('description')}:")
    for item in u.get('items', []):
        print(f"      * {item.get('description')} @ pos {item.get('pos')}")

violations_by_type = {}
for v in drc.get('violations', []):
    vtype = v.get('type')
    sev = v.get('severity')
    key = f"[{sev.upper()}] {vtype}: {v.get('description')}"
    violations_by_type.setdefault(vtype, []).append(v)

print(f"\nViolation categories:")
for vtype, vlist in sorted(violations_by_type.items()):
    print(f"\nCategory '{vtype}' ({len(vlist)} items):")
    # print up to 5 unique examples
    descriptions = set(v.get('description') for v in vlist)
    for d in descriptions:
        print(f"  - {d}")
    for v in vlist[:3]:
        item_desc = " <-> ".join(i.get('description') for i in v.get('items', []))
        print(f"      Example: {item_desc}")
