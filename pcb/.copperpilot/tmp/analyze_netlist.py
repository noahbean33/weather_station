import sys
import os
import xml.etree.ElementTree as ET

# Ensure UTF-8 output
sys.stdout.reconfigure(encoding='utf-8')

tree = ET.parse('.copperpilot/tmp/review.xml')
root = tree.getroot()

components = {}
comps_elem = root.find('components')
if comps_elem is not None:
    for comp in comps_elem.findall('comp'):
        ref = comp.get('ref')
        val = comp.find('value').text if comp.find('value') is not None else ''
        fp = comp.find('footprint').text if comp.find('footprint') is not None else ''
        fields = {}
        fields_elem = comp.find('fields')
        if fields_elem is not None:
            for f in fields_elem.findall('field'):
                fields[f.get('name')] = f.text
        components[ref] = {'value': val, 'footprint': fp, 'fields': fields}

print(f"Total components: {len(components)}")
for ref, data in sorted(components.items()):
    print(f"  {ref}: {data['value']} [{data['footprint']}]")

nets = {}
nets_elem = root.find('nets')
if nets_elem is not None:
    for net in nets_elem.findall('net'):
        name = net.get('name')
        nodes = []
        for node in net.findall('node'):
            pin = node.get('pin')
            pinf = node.get('pinfunction', '')
            nodes.append(f"{node.get('ref')}.pin{pin}({pinf})")
        nets[name] = nodes

print(f"\nTotal nets: {len(nets)}")
for name, nodes in sorted(nets.items()):
    print(f"Net '{name}':")
    for node in sorted(nodes):
        print(f"    {node}")
