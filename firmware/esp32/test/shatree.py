import os, hashlib, binascii

tmp = memoryview(bytearray(1024))

def sha256sum(name, basedir, prefix=''):
    with open(basedir+name, 'rb') as f:
        n = f.readinto(tmp)
        h = hashlib.sha256(tmp[:n])
        while n > 0:
           n = f.readinto(tmp)
           h.update(tmp[:n])
        print ( "{}  {}{}".format(binascii.hexlify(h.digest()).decode('utf-8'), prefix, name) )

def sha256tree(name, basedir='', prefix=''):
    if os.stat(basedir+name)[0] & 0o40000:
        if name.endswith('/'):
            basename = name
        else:
            basename = name + '/'
        for entry in os.listdir(basedir+name):
            sha256tree(basename+entry, basedir, prefix)
    else:
        sha256sum(name, basedir, prefix)


if __name__ == '__main__':
    import sys
    basedir = ''
    prefix = ''

    if len(sys.argv) > 1:
       basedir = sys.argv[1]

    if len(sys.argv) > 2:
       prefix = sys.argv[2]

    sha256tree('/', basedir, prefix)

