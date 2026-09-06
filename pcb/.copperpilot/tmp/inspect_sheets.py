import xml.etree.ElementTree as ET

tree = ET.parse('.copperpilot/tmp/review_netlist.xml')
root = tree.getroot()

# Find components on user_interface sheet
ui_comps = ['D5', 'D6', 'R14', 'R18', 'J6', 'SW1', 'SW2']

for comp in root.findall('.//components/comp'):
    ref = comp.get('ref')
    src = comp.find('sheetpath').get('names') if comp.find('sheetpath') is not None else ''
    print(f"{ref:6s}: sheet {src}")
