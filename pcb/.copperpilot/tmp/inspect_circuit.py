import sys
import xml.etree.ElementTree as ET

sys.stdout.reconfigure(encoding='utf-8')

tree = ET.parse('.copperpilot/tmp/review_netlist.xml')
root = tree.getroot()

comps = {}
for comp in root.findall('.//components/comp'):
    ref = comp.get('ref')
    val = comp.find('value').text if comp.find('value') is not None else ''
    fp = comp.find('footprint').text if comp.find('footprint') is not None else ''
    comps[ref] = {'val': val, 'fp': fp}

nets = {}
for net in root.findall('.//nets/net'):
    name = net.get('name')
    nodes = []
    for node in net.findall('node'):
        nodes.append({
            'ref': node.get('ref'),
            'pin': node.get('pin'),
            'pinf': node.get('pinfunction', '')
        })
    nets[name] = nodes

print('=== ALL COMPONENTS ===')
for ref in sorted(comps.keys()):
    print(f"{ref:6s}: {comps[ref]['val']:20s} [{comps[ref]['fp']}]")

print('\n=== CRITICAL NET CONNECTIONS ===')
interesting_nets = [
    '/ESP32-C3-02/BOOT', '/ESP32-C3-02/EN', '/ESP32-C3-02/DTS', '/ESP32-C3-02/RTS',
    '/ESP32-C3-02/RX_TX', '/ESP32-C3-02/TX_RX', '/ESP32-C3-02/USB_D+', '/ESP32-C3-02/USB_D-',
    'Net-(U4-VBUS)', 'Net-(U4-~{RST})', 'Net-(Q2-B)', 'Net-(Q3-B)',
    '/ESP32-C3-02/CS', '/ESP32-C3-02/CS_SD', '/ESP32-C3-02/MOSI_DI', '/ESP32-C3-02/MISO_DO',
    '/ESP32-C3-02/SCLK', 'Net-(U6-DO(IO1))', 'Net-(J3-DAT0)', 'Net-(U3-IO6)', 'Net-(U3-IO7)',
    '/ESP32-C3-02/SCL', '/ESP32-C3-02/SDA', '/ESP32-C3-02/MIC_OUT', '/ESP32-C3-02/PHOTO_C',
    '/ESP32-C3-02/GPIO18', '/ESP32-C3-02/GPIO19', '/ESP32-C3-02/GPIO8',
    '+5V', '/+5V_USB', '/VBUS', '/VBAT', '3.3V', 'GND'
]

for n in interesting_nets:
    if n in nets:
        nodes_str = ', '.join([f"{node['ref']}.pin{node['pin']}({node['pinf']})" for node in nets[n]])
        print(f"\nNet '{n}':\n  {nodes_str}")
