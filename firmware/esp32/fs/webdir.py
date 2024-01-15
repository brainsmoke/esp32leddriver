import gc, re, os, esphttpd

from esphttpd import urldecode, urlencode_path, htmlencode, error, redirect

MIME_DICT = {
    '.css'  : 'text/css',
    '.gz'   : 'application/gzip',
    '.gif'  : 'image/gif',
    '.html' : 'text/html',
    '.ico'  : 'image/x-icon',
    '.jpg'  : 'image/jpeg',
    '.jpeg' : 'image/jpeg',
    '.js'   : 'text/javascript',
    '.json' : 'text/javascript',
    '.pdf'  : 'application/pdf',
    '.png'  : 'image/png',
    '.py'   : 'text/x-python',
    '.svg'  : 'image/svg+xml',
    '.txt'  : 'text/plain',
    ''      : 'application/octetstream',
}

COMPRESSION_EXTENSION = {
    'gzip' : ( 'gzip', '.gz' ),
    'deflate' : ( 'deflate', '.zlib' ),
    'compress' : ( 'compress', '.Z' ),
}

_buffer = bytearray(4096)

def write_file(req, filename):
    with open(filename, 'rb') as f:
        while True:
            n = f.readinto(_buffer)
            if n <= 0:
                break
            req.write(_buffer[:n])

def get_size(sz):
    return str(sz)

def write_index(req, dirname, ext=''):
    req.write('''<body><table><tbody>''')
    req.write("<tr><td><a href='.'>.</a><td>[DIR]</tr><tr><td><a href='..'>..</a><td>[DIR]</tr>")
    for l in os.ilistdir(dirname):
        name = l[0]
        s = os.stat('/'.join( (dirname, name) ) )
        type_ = s[0]
        size  = s[6]
        if type_ & 0o40000:
            req.write("<tr><td><a href='{}/'>{}</a><td>[DIR]</tr>".format(urlencode_path(name), htmlencode(name)))
        else:
            if ext:
                end = name.rfind(ext)
                if end != -1:
                    name = name[:end]
            req.write("<tr><td><a href='{}'>{}</a><td>{}</tr>".format(urlencode_path(name), htmlencode(name), htmlencode(get_size(size))))

_traversal_regex = re.compile('/\\.\\.(/|$)')
def _does_traversal(s):
    return bool(_traversal_regex.search(s))

_extension_regex        = re.compile('\\.[^/\\.]+$')
_double_extension_regex = re.compile('\\.[^/\\.]+\\.[^/\\.]+$')

def get_mime(filepath, mime_dict=MIME_DICT):
    ext = ''
    match = _double_extension_regex.search(filepath)
    if match == None:
        match = _extension_regex.search(filepath)

    if match != None:
        ext = match.group(0)
        if ext not in mime_dict:
            ext = ''

    return mime_dict[ext]


IF_DIR = 0o040000
IF_REG = 0o100000

def is_dir(path):
    try:
        return bool( os.stat(path)[0] & IF_DIR )
    except OSError:
        return False

def is_reg(path):
    try:
        return bool( os.stat(path)[0] & IF_REG )
    except OSError:
        return False

def safe_utf8(s):
    try:
        return s.decode('utf-8')
    except UnicodeError:
        return None

def add_webdir(server, path, directory, index=False, compression=None, cache_control=None, mime_dict=MIME_DICT):
    directory = directory.rstrip('/')
    basepath = path.rstrip('/')
    content_encoding, ext = None, ''
    if compression:
        content_encoding, ext = COMPRESSION_EXTENSION[compression]

    @server.route(basepath+'/*', "GET")
    def webdir(req):
        path = req.get_path()
        path = urldecode(path)
        path = safe_utf8(path)
        if path == None:
            return error(req, 404, "not found")

        if _does_traversal(path):
            return error(req, 403, "traversal not supported")


        filepath = directory + path[len(basepath):]

        has_trailing_slash = filepath[-1] == '/'

        if filepath[-1] == '/':
            filepath = filepath.rstrip('/')
            if filepath == '':
                filepath = '/'

        if is_dir(filepath):
            if not index:
                return error(req, 404, "not found")
            else:
                if has_trailing_slash:
                    return write_index(req, filepath, ext)
                else:
                    return redirect(req, req.get_path()+b'/', 303)

        mime = get_mime(filepath, mime_dict)

        if ext:
            filepath = filepath+ext

        if has_trailing_slash or not is_reg(filepath):
            return error(req, 404, "not found")

        req.set_content_type(mime)

        if content_encoding:
            req.add_header('Content-Encoding', content_encoding)

        if cache_control:
            req.add_header('Cache-Control', cache_control)

        write_file(req, filepath)
        print("[webdir] {}".format(filepath))
        

