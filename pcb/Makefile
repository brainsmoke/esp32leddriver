

PROJECTS=edgedriver_v4 edgedriver_v4.1 edgedriver_compact
PCBA=edgedriver_v4 edgedriver_v4.1

BASENAME=edgedriver
BOARDHOUSE=jlc
PARTNO_MARKING=JLCJLCJLCJLC
BOM_JLC_OPTS=--fields='Value,Reference,Footprint,LCSC' --labels='Comment,Designator,Footprint,JLCPCB Part \#' --group-by=LCSC --ref-range-delimiter='' --exclude-dnp

REQUIRE_DRC=y

PCB_VARIABLES=-D PCB_ORDER_NUMBER="$(PARTNO_MARKING)"
GERBER_OPTS=--no-x2 --no-netlist --no-protel-ext $(PCB_VARIABLES)
DRC_OPTS=--exit-code-violations $(PCB_VARIABLES)
DRILL_OPTS=--format=excellon --excellon-oval-format=route --excellon-separate-th
BOM_OPTS=$(BOM_JLC_OPTS)
POS_OPTS=--exclude-dnp --side front --units=mm --format=csv

LAYERS2=F.Cu B.Cu F.Mask B.Mask F.Paste B.Paste F.Silkscreen B.Silkscreen Edge.Cuts
LAYERS4=$(LAYERS2) In1.Cu In2.Cu
LAYERS :=$(LAYERS4)

SCAD_DIR=
SCAD_DEPS=
SCAD_PARAM_DIR=
SCAD_PARAM_SET=default
SCAD_PARTS=
SCAD_DEFINES=

include rules.mk

