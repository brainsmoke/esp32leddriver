import sys,os,random,base64,hashlib

secret_filename = os.path.dirname(sys.argv[0])+'/../secret.txt'
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

def head():
    print ("""<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="page.svg">
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">""")

def foot():
    print ("""  </g>
</svg>""")

def label(x, y, essid, key, ip):
    print("""	<g transform="translate({},{})" >
    <text
       xml:space="preserve"
       style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:5.64444px;line-height:1.25;font-family:Cantarell;-inkscape-font-specification:Cantarell;letter-spacing:0px;word-spacing:0px;fill:#000000;fill-opacity:1;stroke:none;stroke-width:0.264583"
       x="6.4129782"
       y="13.171242"
       id="text835"><tspan
         sodipodi:role="line"
         id="tspan833"
         x="6.4129782"
         y="13.171242"
         style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:5.64444px;font-family:Cantarell;-inkscape-font-specification:Cantarell;stroke-width:0.264583">ESSID: {}</tspan><tspan
         sodipodi:role="line"
         x="6.4129782"
         y="20.226791"
         style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:5.64444px;font-family:Cantarell;-inkscape-font-specification:Cantarell;stroke-width:0.264583"
         id="tspan837">Key: {}</tspan><tspan
         sodipodi:role="line"
         x="6.4129782"
         y="27.282343"
         style="font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;font-size:5.64444px;font-family:Cantarell;-inkscape-font-specification:Cantarell;stroke-width:0.264583"
         id="tspan839">IP: {}</tspan></text>
	</g>""".format(3+x*210/3, y*297/8, essid, key, ip))

def json(essid, key, ip):
    print("""{{
	"wifi":
	{{
		"essid": "{}",
		"password": "{}"
	}},
	"network":
	{{
		"ip": "{}"
	}}
}}""".format(essid, key, ip))

if __name__ == '__main__':
    n = int(sys.argv[2])
    if sys.argv[1] == '-svg':
        assert (n % 24 == 0)
        head()
        for i in range(24):
            serial = i+n
            essid = "ledball-{:03d}".format(serial)
            x, y = i%3, i//3
            key = secret(serial)
            ip = '10.0.0.1'
            label(x, y, essid, key, ip)
        foot()
    elif sys.argv[1] == '-json':
        serial = n
        essid = "ledball-{:03d}".format(serial)
        key = secret(serial)
        ip = '10.0.0.1'
        json(essid, key, ip)


