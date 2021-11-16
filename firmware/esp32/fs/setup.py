
import ujson

network = {}

def query(prompt, default=None):
    s = input(prompt + " ["+str(default)+"]: ")
    if s == '':
        return default
    else:
        return s


essid = query("wifi essid", default=None)
if essid == None:
    network['wifi'] = { }
else:
    password = query("wifi password", default=None)

    if password == None:
        raise ValueError("no password given")

    network['wifi'] = { 'essid': essid, 'password': password }

with open("/secret/network.json", "w") as f:
    ujson.dump(network, f)
