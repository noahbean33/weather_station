import xml.etree.ElementTree as ET

tree = ET.parse('.copperpilot/tmp/review_netlist.xml')
root = tree.getroot()

def get_comp_pins(comp_ref):
    print(f"=== Pins for {comp_ref} ===")
    for net in root.findall('.//nets/net'):
        net_name = net.get('name')
        for node in net.findall('node'):
            if node.get('ref') == comp_ref:
                print(f"  Pin {node.get('pin'):>3s} ({node.get('pinfunction',''):20s}) -> Net: {net_name}")

get_comp_pins('U4')
get_comp_pins('U3')
get_comp_pins('U6')
get_comp_pins('U7')
get_comp_pins('U8')
get_comp_pins('SW1')
get_comp_pins('SW2')
get_comp_pins('Q2')
get_comp_pins('Q3')
