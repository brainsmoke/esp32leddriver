import sys,os,random,base64,hashlib,json

import svg

def file_path(filename):
    path = os.path.dirname(filename)
    if path == '':
        path = '.'
    return path

secret_filename = file_path(sys.argv[0])+'/../secret.txt'
n_randbytes = 1234

def make_blob():
    with open(secret_filename, "w") as f:
        f.write(random.randbytes(n_randbytes).hex())


def get_blob():
    if not os.path.exists(secret_filename):
        make_blob()
    with open(secret_filename, "r") as f:
        d = bytes.fromhex(f.read())
        assert(len(d) == n_randbytes)
    return d

def secret(serial):
    b = get_blob()
    hash_in = b + hashlib.sha512(str(serial).encode('utf-8')).digest() + b
    hash_out = hashlib.sha512(hash_in).digest()
    key = base64.b64encode(hash_out).translate(b''.maketrans(b'1ilI0oO',b'!@#$%}{'))[:9].decode('utf-8')
    return key

def svg_qr(x, y, width, s):
    import qr

    paths, orig_width = qr.vector(s)

    svg.start_group(x, y)

    def translate(x, y):
        return width*x/orig_width, width*(orig_width-y)/orig_width

    paths = [ [ translate(x, y) for x, y in path ] for path in paths ]

    svg.path(paths, stroke=None, fill="#000000")
    
    svg.end_group()

def mecard_escape(s):
    return s.translate({ x:'\\'+x for x in '\\;,":' })

def wifi_mecard(essid, key):
    if key == '' or key == None:
        return f'WIFI:S:{mecard_escape(essid)};;'
    else:
        return f'WIFI:T:WPA;S:{mecard_escape(essid)};P:{mecard_escape(key)};;'

def label(row, col, essid, key, ip):
    w, h = 210/3, 297/8

    svg.start_group(col*w, row*h)
    svg.text(f'ESSID: {essid}', w/2+4, h/2-6)
    svg.text(f'Key: {key}', w/2+4, h/2)
    svg.text(f'IP: {ip}', w/2+4, h/2+6)
    svg_qr(2,2, h-4, wifi_mecard(essid, key))
    svg.end_group()

def failsafe_json(essid, key, ip):
    return f"""{{
	"wifi":
	{{
		"essid": {json.dumps(str(essid))},
		"password": {json.dumps(str(key))}
	}},

	"network":
	{{
		"ip": {json.dumps(str(ip))}
	}},

	"auto_fallback": true
}}"""

def gen_failsafe(serial):
    essid = "ledball-{:03d}".format(serial)
    key = secret(serial)
    ip = '10.0.0.1'
    return failsafe_json(essid, key, ip)

if __name__ == '__main__':
    if sys.argv[1] == '-svg':
        n = int(sys.argv[2])
        assert (n % 24 == 0)
        svg.header()
        for i in range(24):
            conf = json.loads(gen_failsafe(i+n))
            col, row = i%3, i//3
            label(row, col, conf['wifi']['essid'], conf['wifi']['password'], conf['network']['ip'])
        svg.footer()
    elif sys.argv[1] == '-json':
        n = int(sys.argv[2])
        print(gen_failsafe(n))
    elif sys.argv[1] == '-svg-from-json':
        conf = json.loads(open(sys.argv[2], 'r').read())
        svg.header(210/3, 297/8)
        label(0,0, conf['wifi']['essid'], conf['wifi']['password'], conf['network']['ip'])
        svg.footer()

