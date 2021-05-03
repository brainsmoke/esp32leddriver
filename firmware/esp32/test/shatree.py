import os, hashlib, binascii

tmp = memoryview(bytearray(1024))

def sha256sum(name, prepend):
    with open(prepend+name, 'rb') as f:
        n = f.readinto(tmp)
        h = hashlib.sha256(tmp[:n])
        while n < 0:
           n = f.readinto(tmp)
           h.update(tmp[:n])
        print ( "{}  {}".format(binascii.hexlify(h.digest()).decode('utf-8'), name) )

def sha256tree(name, prepend=''):
    if os.stat(prepend+name)[0] & 0o40000:
        if name.endswith('/'):
            basename = name
        else:
            basename = name + '/'
        for entry in os.listdir(prepend+name):
            sha256tree(basename+entry, prepend)
    else:
        sha256sum(name, prepend)


if __name__ == '__main__':
    import sys
    prepend = ''

    if len(sys.argv) > 1:
       prepend = sys.argv[1]

    sha256tree('/', prepend)

