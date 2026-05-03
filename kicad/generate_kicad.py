#!/usr/bin/env python3
"""Generate KiCad schematic and PCB files for SN74AHCT125N breakout board."""

import uuid
import json
import os

def uid():
    return str(uuid.uuid4())

BASE = os.path.dirname(os.path.abspath(__file__))
PROJ_DIR = os.path.join(BASE, "sn74ahct125n")

# ──────────────────────────────────────────────
# Generate .kicad_sch (Schematic)
# ──────────────────────────────────────────────

sheet_uuid = uid()

# Component positions (schematic coords, mm, Y-down)
u1_x, u1_y = 152.4, 101.6
c1_x, c1_y = 177.8, 76.2
j1_x, j1_y = 76.2, 101.6
j2_x, j2_y = 220.98, 96.52

# Net label definitions: (name, x, y, angle)
# angle: 0=right, 90=up(screen), 180=left, 270=down(screen)
# We'll connect using net labels

u1_uuid = uid()
c1_uuid = uid()
j1_uuid = uid()
j2_uuid = uid()

# Power symbol instances
vcc1_uuid = uid()
vcc2_uuid = uid()
gnd1_uuid = uid()
gnd2_uuid = uid()
flg_vcc_uuid = uid()
flg_gnd_uuid = uid()

sch = f"""(kicad_sch
  (version 20231120)
  (generator "eeschema")
  (generator_version "8.0")
  (uuid "{sheet_uuid}")
  (paper "A4")
  (title_block
    (title "SN74AHCT125N Breakout Board")
    (date "2025-05-03")
    (rev "1.0")
    (comment 1 "Level Shifter 3.3V to 5V")
    (comment 2 "For ESP32 Radio Bluetooth Project")
  )
  (lib_symbols
  )
  (text "SN74AHCT125N - Quadruple Bus Buffer\\nLevel Shifter 3.3V → 5V\\n\\nJ1: Input (ESP32 3.3V side)\\nJ2: Output (5V side)\\nC1: 100nF Bypass Capacitor\\n\\nTie ~{{OE}} pins LOW to enable outputs"
    (exclude_from_sim no)
    (at 30.48 30.48 0)
    (effects
      (font (size 1.524 1.524))
      (justify left)
    )
    (uuid "{uid()}")
  )

  (symbol
    (lib_id "SN74AHCT125N:SN74AHCT125N")
    (at {u1_x} {u1_y} 0)
    (unit 1)
    (exclude_from_sim no)
    (in_bom yes)
    (on_board yes)
    (dnp no)
    (uuid "{u1_uuid}")
    (property "Reference" "U1"
      (at {u1_x} {u1_y - 20} 0)
      (effects (font (size 1.27 1.27)))
    )
    (property "Value" "SN74AHCT125N"
      (at {u1_x} {u1_y + 20} 0)
      (effects (font (size 1.27 1.27)))
    )
    (property "Footprint" "SN74AHCT125N:DIP-14_W7.62mm"
      (at {u1_x} {u1_y + 22} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
    (property "Datasheet" "https://www.ti.com/lit/ds/symlink/sn74ahct125.pdf"
      (at {u1_x} {u1_y + 24} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
  )

  (symbol
    (lib_id "SN74AHCT125N:C")
    (at {c1_x} {c1_y} 0)
    (unit 1)
    (exclude_from_sim no)
    (in_bom yes)
    (on_board yes)
    (dnp no)
    (uuid "{c1_uuid}")
    (property "Reference" "C1"
      (at {c1_x + 2} {c1_y} 0)
      (effects (font (size 1.27 1.27)) (justify left))
    )
    (property "Value" "100nF"
      (at {c1_x + 2} {c1_y + 2} 0)
      (effects (font (size 1.27 1.27)) (justify left))
    )
    (property "Footprint" "SN74AHCT125N:C_Disc_D5.0mm_W2.5mm_P2.50mm"
      (at {c1_x} {c1_y + 4} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
  )

  (symbol
    (lib_id "SN74AHCT125N:Conn_01x10_Pin")
    (at {j1_x} {j1_y} 0)
    (mirror x)
    (unit 1)
    (exclude_from_sim no)
    (in_bom yes)
    (on_board yes)
    (dnp no)
    (uuid "{j1_uuid}")
    (property "Reference" "J1"
      (at {j1_x} {j1_y - 17} 0)
      (effects (font (size 1.27 1.27)))
    )
    (property "Value" "INPUT"
      (at {j1_x} {j1_y + 17} 0)
      (effects (font (size 1.27 1.27)))
    )
    (property "Footprint" "SN74AHCT125N:PinHeader_1x10_P2.54mm_Vertical"
      (at {j1_x} {j1_y + 19} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
  )

  (symbol
    (lib_id "SN74AHCT125N:Conn_01x06_Pin")
    (at {j2_x} {j2_y} 0)
    (unit 1)
    (exclude_from_sim no)
    (in_bom yes)
    (on_board yes)
    (dnp no)
    (uuid "{j2_uuid}")
    (property "Reference" "J2"
      (at {j2_x} {j2_y - 12} 0)
      (effects (font (size 1.27 1.27)))
    )
    (property "Value" "OUTPUT"
      (at {j2_x} {j2_y + 12} 0)
      (effects (font (size 1.27 1.27)))
    )
    (property "Footprint" "SN74AHCT125N:PinHeader_1x06_P2.54mm_Vertical"
      (at {j2_x} {j2_y + 14} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
  )

  (symbol
    (lib_id "SN74AHCT125N:VCC")
    (at {u1_x} {u1_y - 21} 0)
    (unit 1)
    (exclude_from_sim no)
    (in_bom no)
    (on_board no)
    (dnp no)
    (uuid "{vcc1_uuid}")
    (property "Reference" "#PWR01"
      (at {u1_x} {u1_y - 25} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
    (property "Value" "VCC"
      (at {u1_x} {u1_y - 23} 0)
      (effects (font (size 1.27 1.27)))
    )
  )

  (symbol
    (lib_id "SN74AHCT125N:GND")
    (at {u1_x} {u1_y + 20} 0)
    (unit 1)
    (exclude_from_sim no)
    (in_bom no)
    (on_board no)
    (dnp no)
    (uuid "{gnd1_uuid}")
    (property "Reference" "#PWR02"
      (at {u1_x} {u1_y + 24} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
    (property "Value" "GND"
      (at {u1_x} {u1_y + 22} 0)
      (effects (font (size 1.27 1.27)))
    )
  )

  (symbol
    (lib_id "SN74AHCT125N:VCC")
    (at {c1_x} {c1_y - 5} 0)
    (unit 1)
    (exclude_from_sim no)
    (in_bom no)
    (on_board no)
    (dnp no)
    (uuid "{vcc2_uuid}")
    (property "Reference" "#PWR03"
      (at {c1_x} {c1_y - 9} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
    (property "Value" "VCC"
      (at {c1_x} {c1_y - 7} 0)
      (effects (font (size 1.27 1.27)))
    )
  )

  (symbol
    (lib_id "SN74AHCT125N:GND")
    (at {c1_x} {c1_y + 5} 0)
    (unit 1)
    (exclude_from_sim no)
    (in_bom no)
    (on_board no)
    (dnp no)
    (uuid "{gnd2_uuid}")
    (property "Reference" "#PWR04"
      (at {c1_x} {c1_y + 9} 0)
      (effects (font (size 1.27 1.27)) hide)
    )
    (property "Value" "GND"
      (at {c1_x} {c1_y + 7} 0)
      (effects (font (size 1.27 1.27)))
    )
  )

"""

# Net labels connecting J1 to U1 inputs and J2 to U1 outputs
# J1 pin mapping: Pin1=VCC, Pin2=GND, Pin3=~OE1, Pin4=1A, Pin5=~OE2, Pin6=2A, Pin7=~OE3, Pin8=3A, Pin9=~OE4, Pin10=4A
# J2 pin mapping: Pin1=VCC, Pin2=GND, Pin3=1Y, Pin4=2Y, Pin5=3Y, Pin6=4Y

# Labels near J1 pins (placed to the right of connector pins)
j1_labels = [
    ("VCC", j1_x - 3.81, j1_y - 11.43, 0),
    ("GND", j1_x - 3.81, j1_y - 8.89, 0),
    ("~{OE1}", j1_x - 3.81, j1_y - 6.35, 0),
    ("1A", j1_x - 3.81, j1_y - 3.81, 0),
    ("~{OE2}", j1_x - 3.81, j1_y - 1.27, 0),
    ("2A", j1_x - 3.81, j1_y + 1.27, 0),
    ("~{OE3}", j1_x - 3.81, j1_y + 3.81, 0),
    ("3A", j1_x - 3.81, j1_y + 6.35, 0),
    ("~{OE4}", j1_x - 3.81, j1_y + 8.89, 0),
    ("4A", j1_x - 3.81, j1_y + 11.43, 0),
]

# Labels near U1 input pins
u1_input_labels = [
    ("~{OE1}", u1_x - 8.89, u1_y - 10.16, 180),
    ("1A", u1_x - 8.89, u1_y - 7.62, 180),
    ("~{OE2}", u1_x - 8.89, u1_y - 5.08, 180),
    ("2A", u1_x - 8.89, u1_y - 2.54, 180),
    ("~{OE3}", u1_x - 8.89, u1_y + 5.08, 180),
    ("3A", u1_x - 8.89, u1_y + 2.54, 180),
    ("~{OE4}", u1_x - 8.89, u1_y + 10.16, 180),
    ("4A", u1_x - 8.89, u1_y + 7.62, 180),
]

# Labels near U1 output pins
u1_output_labels = [
    ("1Y", u1_x + 8.89, u1_y - 10.16, 0),
    ("2Y", u1_x + 8.89, u1_y - 5.08, 0),
    ("3Y", u1_x + 8.89, u1_y + 0, 0),
    ("4Y", u1_x + 8.89, u1_y + 5.08, 0),
]

# Labels near J2 pins
j2_labels = [
    ("VCC", j2_x - 3.81, j2_y - 6.35, 180),
    ("GND", j2_x - 3.81, j2_y - 3.81, 180),
    ("1Y", j2_x - 3.81, j2_y - 1.27, 180),
    ("2Y", j2_x - 3.81, j2_y + 1.27, 180),
    ("3Y", j2_x - 3.81, j2_y + 3.81, 180),
    ("4Y", j2_x - 3.81, j2_y + 6.35, 180),
]

all_labels = j1_labels + u1_input_labels + u1_output_labels + j2_labels

for name, lx, ly, angle in all_labels:
    sch += f"""  (label "{name}"
    (at {lx:.2f} {ly:.2f} {angle})
    (effects (font (size 1.27 1.27)) (justify left))
    (uuid "{uid()}")
  )
"""

# Wires: VCC/GND for U1 (vertical from power pins to power symbols)
wires = [
    # U1 VCC pin to VCC symbol
    (u1_x, u1_y - 16.51, u1_x, u1_y - 21),
    # U1 GND pin to GND symbol
    (u1_x, u1_y + 16.51, u1_x, u1_y + 20),
    # C1 pin 1 to VCC
    (c1_x, c1_y - 2.54, c1_x, c1_y - 5),
    # C1 pin 2 to GND
    (c1_x, c1_y + 2.54, c1_x, c1_y + 5),
]

for x1, y1, x2, y2 in wires:
    sch += f"""  (wire
    (pts (xy {x1:.2f} {y1:.2f}) (xy {x2:.2f} {y2:.2f}))
    (stroke (width 0) (type default))
    (uuid "{uid()}")
  )
"""

sch += f"""
  (sheet_instances
    (path "/"
      (page "1")
    )
  )
  (symbol_instances
    (path "/{u1_uuid}" (reference "U1") (unit 1) (value "SN74AHCT125N") (footprint "SN74AHCT125N:DIP-14_W7.62mm"))
    (path "/{c1_uuid}" (reference "C1") (unit 1) (value "100nF") (footprint "SN74AHCT125N:C_Disc_D5.0mm_W2.5mm_P2.50mm"))
    (path "/{j1_uuid}" (reference "J1") (unit 1) (value "INPUT") (footprint "SN74AHCT125N:PinHeader_1x10_P2.54mm_Vertical"))
    (path "/{j2_uuid}" (reference "J2") (unit 1) (value "OUTPUT") (footprint "SN74AHCT125N:PinHeader_1x06_P2.54mm_Vertical"))
    (path "/{vcc1_uuid}" (reference "#PWR01") (unit 1) (value "VCC") (footprint ""))
    (path "/{gnd1_uuid}" (reference "#PWR02") (unit 1) (value "GND") (footprint ""))
    (path "/{vcc2_uuid}" (reference "#PWR03") (unit 1) (value "VCC") (footprint ""))
    (path "/{gnd2_uuid}" (reference "#PWR04") (unit 1) (value "GND") (footprint ""))
  )
)
"""

with open(os.path.join(PROJ_DIR, "sn74ahct125n.kicad_sch"), "w") as f:
    f.write(sch)

print("Schematic generated.")

# ──────────────────────────────────────────────
# Generate .kicad_pcb (PCB Layout)
# ──────────────────────────────────────────────

# Board dimensions: 55mm x 35mm
board_w, board_h = 55, 35
board_x, board_y = 100, 80  # origin offset

# Component positions on PCB (mm)
# U1 (DIP-14) center
u1_pcb_x = board_x + 27.5
u1_pcb_y = board_y + 17.5

# C1 (bypass cap) near VCC pin of U1
c1_pcb_x = board_x + 40
c1_pcb_y = board_y + 8

# J1 (input header 1x10) on the left
j1_pcb_x = board_x + 5
j1_pcb_y = board_y + 5.5

# J2 (output header 1x06) on the right
j2_pcb_x = board_x + 50
j2_pcb_y = board_y + 10.5

# DIP-14 pad positions relative to footprint origin (pin 1)
# Pin 1 is at (0,0), pins go down in 2.54mm increments
# Pins 8-14 are on the other side at x=7.62, going up
dip14_pads = {}
for i in range(7):
    dip14_pads[i+1] = (0, i * 2.54)
for i in range(7):
    dip14_pads[14-i] = (7.62, i * 2.54)

# Absolute U1 pad positions
def u1_pad(pin):
    dx, dy = dip14_pads[pin]
    return (u1_pcb_x + dx, u1_pcb_y + dy)

# J1 pad positions (1x10 vertical, pads at y = n*2.54)
def j1_pad(pin):
    return (j1_pcb_x, j1_pcb_y + (pin-1) * 2.54)

# J2 pad positions (1x06 vertical)
def j2_pad(pin):
    return (j2_pcb_x, j2_pcb_y + (pin-1) * 2.54)

# C1 pad positions (pad 1 at origin, pad 2 at x+2.5)
def c1_pad(pin):
    if pin == 1:
        return (c1_pcb_x, c1_pcb_y)
    return (c1_pcb_x + 2.5, c1_pcb_y)

# Net definitions
nets = {
    0: "",       # unconnected
    1: "VCC",
    2: "GND",
    3: "OE1",
    4: "A1",
    5: "OE2",
    6: "A2",
    7: "OE3",
    8: "A3",
    9: "OE4",
    10: "A4",
    11: "Y1",
    12: "Y2",
    13: "Y3",
    14: "Y4",
}

# Pad-to-net assignments
# U1: pin1=OE1, pin2=A1, pin3=Y1, pin4=OE2, pin5=A2, pin6=Y2, pin7=GND,
#     pin8=Y3, pin9=A3, pin10=OE3, pin11=Y4, pin12=A4, pin13=OE4, pin14=VCC
u1_nets = {1:3, 2:4, 3:11, 4:5, 5:6, 6:12, 7:2, 8:13, 9:8, 10:7, 11:14, 12:10, 13:9, 14:1}

# J1: pin1=VCC, pin2=GND, pin3=OE1, pin4=A1, pin5=OE2, pin6=A2, pin7=OE3, pin8=A3, pin9=OE4, pin10=A4
j1_nets = {1:1, 2:2, 3:3, 4:4, 5:5, 6:6, 7:7, 8:8, 9:9, 10:10}

# J2: pin1=VCC, pin2=GND, pin3=Y1, pin4=Y2, pin5=Y3, pin6=Y4
j2_nets = {1:1, 2:2, 3:11, 4:12, 5:13, 6:14}

# C1: pin1=VCC, pin2=GND
c1_nets = {1:1, 2:2}

pcb_uuid = uid()

pcb = f"""(kicad_pcb
  (version 20231120)
  (generator "pcbnew")
  (generator_version "8.0")
  (general
    (thickness 1.6)
    (legacy_teardrops no)
  )
  (paper "A4")
  (layers
    (0 "F.Cu" signal)
    (31 "B.Cu" signal)
    (32 "B.Adhes" user "B.Adhesive")
    (33 "F.Adhes" user "F.Adhesive")
    (34 "B.Paste" user)
    (35 "F.Paste" user)
    (36 "B.SilkS" user "B.Silkscreen")
    (37 "F.SilkS" user "F.Silkscreen")
    (38 "B.Mask" user "B.Mask")
    (39 "F.Mask" user "F.Mask")
    (40 "Dwgs.User" user "User.Drawings")
    (41 "Cmts.User" user "User.Comments")
    (42 "B.CrtYd" user "B.Courtyard")
    (43 "F.CrtYd" user "F.Courtyard")
    (44 "B.Fab" user "B.Fab")
    (45 "F.Fab" user "F.Fab")
    (46 "User.1" user)
    (47 "User.2" user)
    (48 "User.3" user)
    (49 "User.4" user)
    (50 "User.5" user)
    (51 "User.6" user)
    (52 "User.7" user)
    (53 "User.8" user)
    (54 "User.9" user)
  )
  (setup
    (pad_to_mask_clearance 0)
    (allow_soldermask_bridges_in_footprints no)
    (pcbplotparams
      (layerselection 0x00010fc_ffffffff)
      (plot_on_all_layers_selection 0x0000000_00000000)
      (disableapertmacros no)
      (usegerberextensions no)
      (usegerberattributes yes)
      (usegerberadvancedattributes yes)
      (creategerberjobfile yes)
      (dashed_line_dash_ratio 12.000000)
      (dashed_line_gap_ratio 3.000000)
      (svgprecision 4)
      (plotframeref no)
      (viasonmask no)
      (mode 1)
      (useauxorigin no)
      (hpglpennumber 1)
      (hpglpenspeed 20)
      (hpglpendiameter 15.000000)
      (pdf_front_fp_property_popups yes)
      (pdf_back_fp_property_popups yes)
      (dxfpolygonmode yes)
      (dxfimperialunits yes)
      (dxfusepcbnewfont yes)
      (psnegative no)
      (psa4output no)
      (plotreference yes)
      (plotvalue yes)
      (plotfptext yes)
      (plotinvisibletext no)
      (sketchpadsonfab no)
      (subtractmaskfromsilk no)
      (outputformat 1)
      (mirror no)
      (drillshape 1)
      (scaleselection 1)
      (outputdirectory "")
    )
  )
"""

# Add nets
for net_id, net_name in nets.items():
    pcb += f'  (net {net_id} "{net_name}")\n'

# Board outline (Edge.Cuts)
pcb += f"""
  (gr_rect
    (start {board_x} {board_y})
    (end {board_x + board_w} {board_y + board_h})
    (stroke (width 0.15) (type default))
    (fill none)
    (layer "Edge.Cuts")
    (uuid "{uid()}")
  )
"""

# Title silkscreen
pcb += f"""
  (gr_text "SN74AHCT125N"
    (at {board_x + board_w/2} {board_y + 3})
    (layer "F.SilkS")
    (effects (font (size 1.5 1.5) (thickness 0.2)))
    (uuid "{uid()}")
  )
  (gr_text "Level Shifter 3.3V→5V"
    (at {board_x + board_w/2} {board_y + board_h - 2})
    (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15)))
    (uuid "{uid()}")
  )
  (gr_text "INPUT"
    (at {j1_pcb_x} {j1_pcb_y - 3})
    (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15)))
    (uuid "{uid()}")
  )
  (gr_text "OUTPUT"
    (at {j2_pcb_x} {j2_pcb_y - 3})
    (layer "F.SilkS")
    (effects (font (size 1 1) (thickness 0.15)))
    (uuid "{uid()}")
  )
"""

# Pin labels on silkscreen near J1
j1_pin_labels = ["VCC", "GND", "~OE1", "1A", "~OE2", "2A", "~OE3", "3A", "~OE4", "4A"]
for i, label in enumerate(j1_pin_labels):
    py = j1_pcb_y + i * 2.54
    pcb += f"""  (gr_text "{label}"
    (at {j1_pcb_x + 3} {py})
    (layer "F.SilkS")
    (effects (font (size 0.8 0.8) (thickness 0.12)) (justify left))
    (uuid "{uid()}")
  )
"""

# Pin labels near J2
j2_pin_labels = ["VCC", "GND", "1Y", "2Y", "3Y", "4Y"]
for i, label in enumerate(j2_pin_labels):
    py = j2_pcb_y + i * 2.54
    pcb += f"""  (gr_text "{label}"
    (at {j2_pcb_x - 3} {py})
    (layer "F.SilkS")
    (effects (font (size 0.8 0.8) (thickness 0.12)) (justify right))
    (uuid "{uid()}")
  )
"""

# Mounting holes (optional, 4 corners)
mh_offset = 2.5
mh_positions = [
    (board_x + mh_offset, board_y + mh_offset),
    (board_x + board_w - mh_offset, board_y + mh_offset),
    (board_x + mh_offset, board_y + board_h - mh_offset),
    (board_x + board_w - mh_offset, board_y + board_h - mh_offset),
]
for mx, my in mh_positions:
    pcb += f"""  (gr_circle
    (center {mx} {my})
    (end {mx + 1.5} {my})
    (stroke (width 0.15) (type default))
    (fill none)
    (layer "Edge.Cuts")
    (uuid "{uid()}")
  )
"""


def write_footprint(ref, value, fp_name, x, y, angle, pad_nets, pads_info):
    """Generate footprint placement in PCB."""
    s = f"""
  (footprint "{fp_name}"
    (layer "F.Cu")
    (uuid "{uid()}")
    (at {x} {y} {angle})
    (property "Reference" "{ref}"
      (at 0 -2.5 {angle})
      (layer "F.SilkS")
      (effects (font (size 1 1) (thickness 0.15)))
    )
    (property "Value" "{value}"
      (at 0 2.5 {angle})
      (layer "F.Fab")
      (effects (font (size 1 1) (thickness 0.15)))
    )
"""
    for pin_num, (px, py, shape) in pads_info.items():
        net_id = pad_nets.get(pin_num, 0)
        net_name = nets.get(net_id, "")
        net_str = f'(net {net_id} "{net_name}")' if net_id > 0 else ""
        s += f"""    (pad "{pin_num}" thru_hole {shape}
      (at {px} {py})
      (size 1.6 1.6)
      (drill 0.8)
      (layers "*.Cu" "*.Mask")
      {net_str}
      (uuid "{uid()}")
    )
"""
    s += "  )\n"
    return s


# U1 footprint (DIP-14)
u1_pads = {}
for i in range(7):
    shape = "rect" if i == 0 else "oval"
    u1_pads[i+1] = (0, i*2.54, shape)
for i in range(7):
    u1_pads[14-i] = (7.62, i*2.54, "oval")

pcb += write_footprint("U1", "SN74AHCT125N", "SN74AHCT125N:DIP-14_W7.62mm",
                        u1_pcb_x, u1_pcb_y, 0, u1_nets, u1_pads)

# C1 footprint
c1_pads = {1: (0, 0, "circle"), 2: (2.5, 0, "circle")}
pcb += write_footprint("C1", "100nF", "SN74AHCT125N:C_Disc_D5.0mm_W2.5mm_P2.50mm",
                        c1_pcb_x, c1_pcb_y, 0, c1_nets, c1_pads)

# J1 footprint (1x10 pin header)
j1_pads = {}
for i in range(10):
    shape = "rect" if i == 0 else "oval"
    j1_pads[i+1] = (0, i*2.54, shape)
pcb += write_footprint("J1", "INPUT", "SN74AHCT125N:PinHeader_1x10_P2.54mm_Vertical",
                        j1_pcb_x, j1_pcb_y, 0, j1_nets, j1_pads)

# J2 footprint (1x06 pin header)
j2_pads = {}
for i in range(6):
    shape = "rect" if i == 0 else "oval"
    j2_pads[i+1] = (0, i*2.54, shape)
pcb += write_footprint("J2", "OUTPUT", "SN74AHCT125N:PinHeader_1x06_P2.54mm_Vertical",
                        j2_pcb_x, j2_pcb_y, 0, j2_nets, j2_pads)

# ──────────────────────────────────────────────
# Copper traces
# ──────────────────────────────────────────────

traces = []

def add_trace(net_id, width, layer, *points):
    for i in range(len(points)-1):
        traces.append((net_id, width, layer, points[i], points[i+1]))

# Power traces (0.5mm width)
pw = 0.5
# Signal traces (0.25mm width)
sw = 0.3

# --- VCC net (net 1) ---
# U1 pin14 (VCC) to C1 pin1
p_u1_14 = u1_pad(14)
p_c1_1 = c1_pad(1)
# Route: U1 pin14 -> up -> right -> C1 pin1
add_trace(1, pw, "F.Cu", p_u1_14, (p_u1_14[0], p_c1_1[1]), p_c1_1)

# J1 pin1 (VCC) to U1 VCC via top trace
p_j1_1 = j1_pad(1)
add_trace(1, pw, "F.Cu", p_j1_1, (p_j1_1[0], board_y + 2), (p_u1_14[0], board_y + 2), (p_u1_14[0], p_c1_1[1]))

# J2 pin1 (VCC) to C1 pin1
p_j2_1 = j2_pad(1)
add_trace(1, pw, "F.Cu", p_j2_1, (p_j2_1[0], p_c1_1[1]), p_c1_1)

# --- GND net (net 2) ---
# U1 pin7 (GND) to C1 pin2
p_u1_7 = u1_pad(7)
p_c1_2 = c1_pad(2)
add_trace(2, pw, "B.Cu", p_u1_7, (p_u1_7[0], board_y + board_h - 2), (p_c1_2[0], board_y + board_h - 2), (p_c1_2[0], p_c1_2[1]))

# J1 pin2 (GND) to U1 GND
p_j1_2 = j1_pad(2)
add_trace(2, pw, "B.Cu", p_j1_2, (p_j1_2[0], board_y + board_h - 2), (p_u1_7[0], board_y + board_h - 2))

# J2 pin2 (GND) to GND bus
p_j2_2 = j2_pad(2)
add_trace(2, pw, "B.Cu", p_j2_2, (p_j2_2[0], board_y + board_h - 2))

# --- Signal traces ---
# OE1: J1 pin3 -> U1 pin1
p_j1_3 = j1_pad(3)
p_u1_1 = u1_pad(1)
add_trace(3, sw, "F.Cu", p_j1_3, (p_u1_1[0], p_u1_1[1]))

# A1: J1 pin4 -> U1 pin2
p_j1_4 = j1_pad(4)
p_u1_2 = u1_pad(2)
add_trace(4, sw, "F.Cu", p_j1_4, (p_u1_2[0], p_u1_2[1]))

# OE2: J1 pin5 -> U1 pin4
p_j1_5 = j1_pad(5)
p_u1_4 = u1_pad(4)
add_trace(5, sw, "F.Cu", p_j1_5, (p_u1_4[0], p_u1_4[1]))

# A2: J1 pin6 -> U1 pin5
p_j1_6 = j1_pad(6)
p_u1_5 = u1_pad(5)
add_trace(6, sw, "F.Cu", p_j1_6, (p_u1_5[0], p_u1_5[1]))

# OE3: J1 pin7 -> U1 pin10
p_j1_7 = j1_pad(7)
p_u1_10 = u1_pad(10)
# Route on back copper to avoid crossing
add_trace(7, sw, "B.Cu", p_j1_7, (p_j1_7[0], p_u1_10[1]), (p_u1_10[0], p_u1_10[1]))

# A3: J1 pin8 -> U1 pin9
p_j1_8 = j1_pad(8)
p_u1_9 = u1_pad(9)
add_trace(8, sw, "B.Cu", p_j1_8, (p_j1_8[0], p_u1_9[1]), (p_u1_9[0], p_u1_9[1]))

# OE4: J1 pin9 -> U1 pin13
p_j1_9 = j1_pad(9)
p_u1_13 = u1_pad(13)
add_trace(9, sw, "B.Cu", p_j1_9, (p_j1_9[0], p_u1_13[1]), (p_u1_13[0], p_u1_13[1]))

# A4: J1 pin10 -> U1 pin12
p_j1_10 = j1_pad(10)
p_u1_12 = u1_pad(12)
add_trace(10, sw, "B.Cu", p_j1_10, (p_j1_10[0], p_u1_12[1]), (p_u1_12[0], p_u1_12[1]))

# Y1: U1 pin3 -> J2 pin3
p_u1_3 = u1_pad(3)
p_j2_3 = j2_pad(3)
add_trace(11, sw, "F.Cu", p_u1_3, (p_j2_3[0], p_j2_3[1]))

# Y2: U1 pin6 -> J2 pin4
p_u1_6 = u1_pad(6)
p_j2_4 = j2_pad(4)
add_trace(12, sw, "F.Cu", p_u1_6, (p_j2_4[0], p_j2_4[1]))

# Y3: U1 pin8 -> J2 pin5
p_u1_8 = u1_pad(8)
p_j2_5 = j2_pad(5)
add_trace(13, sw, "F.Cu", p_u1_8, (p_j2_5[0], p_j2_5[1]))

# Y4: U1 pin11 -> J2 pin6
p_u1_11 = u1_pad(11)
p_j2_6 = j2_pad(6)
add_trace(14, sw, "F.Cu", p_u1_11, (p_j2_6[0], p_j2_6[1]))

# Write traces
for net_id, width, layer, p1, p2 in traces:
    net_name = nets[net_id]
    pcb += f"""  (segment
    (start {p1[0]:.4f} {p1[1]:.4f})
    (end {p2[0]:.4f} {p2[1]:.4f})
    (width {width})
    (layer "{layer}")
    (net {net_id})
    (uuid "{uid()}")
  )
"""

# Ground fill zone on B.Cu
pcb += f"""
  (zone
    (net 2)
    (net_name "GND")
    (layer "B.Cu")
    (uuid "{uid()}")
    (hatch edge 0.5)
    (connect_pads (clearance 0.3))
    (min_thickness 0.25)
    (filled_areas_thickness no)
    (fill yes (thermal_gap 0.5) (thermal_bridge_width 0.5))
    (polygon
      (pts
        (xy {board_x + 0.5} {board_y + 0.5})
        (xy {board_x + board_w - 0.5} {board_y + 0.5})
        (xy {board_x + board_w - 0.5} {board_y + board_h - 0.5})
        (xy {board_x + 0.5} {board_y + board_h - 0.5})
      )
    )
  )
"""

pcb += ")\n"

with open(os.path.join(PROJ_DIR, "sn74ahct125n.kicad_pcb"), "w") as f:
    f.write(pcb)

print("PCB generated.")

# ──────────────────────────────────────────────
# Generate .kicad_pro (Project File)
# ──────────────────────────────────────────────

project = {
    "meta": {
        "filename": "sn74ahct125n.kicad_pro",
        "version": 1
    },
    "net_settings": {
        "classes": [
            {
                "bus_width": 12,
                "clearance": 0.2,
                "diff_pair_gap": 0.25,
                "diff_pair_via_gap": 0.25,
                "diff_pair_width": 0.2,
                "line_style": 0,
                "microvia_diameter": 0.3,
                "microvia_drill": 0.1,
                "name": "Default",
                "pcb_color": "rgba(0, 0, 0, 0.000)",
                "schematic_color": "rgba(0, 0, 0, 0.000)",
                "track_width": 0.3,
                "via_diameter": 0.6,
                "via_drill": 0.3,
                "wire_width": 6
            },
            {
                "bus_width": 12,
                "clearance": 0.2,
                "diff_pair_gap": 0.25,
                "diff_pair_via_gap": 0.25,
                "diff_pair_width": 0.2,
                "line_style": 0,
                "microvia_diameter": 0.3,
                "microvia_drill": 0.1,
                "name": "Power",
                "pcb_color": "rgba(255, 0, 0, 1.000)",
                "schematic_color": "rgba(255, 0, 0, 1.000)",
                "track_width": 0.5,
                "via_diameter": 0.8,
                "via_drill": 0.4,
                "wire_width": 6
            }
        ],
        "meta": {
            "version": 3
        },
        "net_colors": None,
        "netclass_assignments": None,
        "netclass_patterns": [
            {
                "netclass": "Power",
                "pattern": "VCC"
            },
            {
                "netclass": "Power",
                "pattern": "GND"
            }
        ]
    },
    "pcbnew": {
        "last_paths": {
            "gencad": "",
            "idf": "",
            "netlist": "",
            "plot": "",
            "pos_files": "",
            "specctra_dsn": "",
            "step": "",
            "svg": "",
            "vrml": ""
        },
        "page_layout_descr_file": ""
    },
    "schematic": {
        "annotate_start_num": 0,
        "bom_export_filename": "",
        "bom_fmt_presets": [],
        "bom_fmt_settings": {
            "field_delimiter": ",",
            "keep_line_breaks": False,
            "keep_tabs": False,
            "name": "",
            "ref_delimiter": ",",
            "ref_range_delimiter": "",
            "string_delimiter": "\""
        },
        "connection_grid_size": 50.0,
        "drawing": {
            "dashed_lines_dash_length_ratio": 12.0,
            "dashed_lines_gap_length_ratio": 3.0,
            "default_line_thickness": 6.0,
            "default_text_size": 50.0,
            "field_names": [],
            "intersheets_ref_own_page": False,
            "intersheets_ref_prefix": "",
            "intersheets_ref_short": False,
            "intersheets_ref_show": False,
            "intersheets_ref_suffix": "",
            "junction_size_choice": 3,
            "label_size_ratio": 0.375,
            "operating_point_overlay_i_precision": 3,
            "operating_point_overlay_i_range": "~A",
            "operating_point_overlay_v_precision": 3,
            "operating_point_overlay_v_range": "~V",
            "overbar_offset_ratio": 1.23,
            "pin_symbol_size": 25.0,
            "text_offset_ratio": 0.15
        },
        "legacy_lib_dir": "",
        "legacy_lib_list": [],
        "meta": {
            "version": 1
        },
        "net_format_name": "",
        "page_layout_descr_file": "",
        "plot_directory": "",
        "spice_current_sheet_as_root": False,
        "spice_external_command": "spice \"%I\"",
        "spice_model_current_sheet_as_root": True,
        "spice_save_all_currents": False,
        "spice_save_all_dissipations": False,
        "spice_save_all_voltages": False,
        "subpart_first_id": 65,
        "subpart_id_separator": 0
    },
    "sheets": [
        [
            "e63e39d7-6ac0-4ffd-8aa3-1841a4541b55",
            ""
        ]
    ],
    "text_variables": {}
}

with open(os.path.join(PROJ_DIR, "sn74ahct125n.kicad_pro"), "w") as f:
    json.dump(project, f, indent=2)

print("Project file generated.")
print("All KiCad files generated successfully!")
print(f"Project directory: {PROJ_DIR}")
