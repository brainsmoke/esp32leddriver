#!/bin/bash
diff -ru <(../fppa-pdk-tools/dispdk 0x2A16 "$1" | sed 's:................::') <(../fppa-pdk-tools/dispdk 0x2AA1 "${1%13.ihx}14.ihx" | sed 's:................::')
