#!/usr/bin/env python3
# get model from config dir (without adding a jq dependency)

import json, sys, re

filename = (sys.argv+['.','.'])[1]+'/hardware.json'

model = None
try:
    model = str(json.load(open(filename, "r"))['model'])
except OSError:
    print ("Error: cannot read model defined in {}".format(filename), file=sys.stderr)
except json.decoder.JSONDecodeError:
    print ("Error: json parsing error in {}".format(filename), file=sys.stderr)
except KeyError:
    print ("Error: no model defined in {}".format(filename), file=sys.stderr)
else:
    if not re.search(r'^/models/', model) or re.search(r'/\.\.(/|$)', model):
        print ("Error: unexpected model string", file=sys.stderr)
    else:
        print (model)
        sys.exit(0)

sys.exit(1)

