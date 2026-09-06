import xml.etree.ElementTree as ET

tree = ET.parse('.copperpilot/tmp/review_netlist.xml')
root = tree.getroot()

mic_refs = ['U8', 'MK1', 'R17', 'R26', 'R27', 'R28', 'R29', 'R30', 'R31', 'C16', 'C17', 'C19', 'C20']

for ref in mic_refs:
    print(f"=== {ref} ===")
    for net in root.findall('.//nets/net'):
        for node in net.findall('node'):
            if node.get('ref') == ref:
                print(f"  Pin {node.get('pin')} -> Net {net.get('name')}")
