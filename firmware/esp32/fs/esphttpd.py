import gc, re, os, binascii

_urlencode_unreserved = 'A-Za-z0-9\\._\\-~'
_urlencode_unreserved_path = _urlencode_unreserved + '/'
#_urlencode_reserved = "!#\\$&'\\(\\)\\*\\+,:;=\\?@\\[\\]"

_urldecode_regex = re.compile('%[A-Fa-f0-9][A-Fa-f0-9]')

_urlencode_path_regex = re.compile('[^'+_urlencode_unreserved_path+']')

html_entities = {
    '"' : '&quot;',
    '\'': '&apos;',
    '&' : '&amp;',
    '<' : '&lt;',
    '>' : '&gt;',
}

_htmlencode_regex = re.compile('[\\\"\\\'\\&\\<\\>]')

def htmlencode_esc(m):
    return html_entities[m.group()]

def htmlencode(s):
    return _htmlencode_regex.sub(htmlencode_esc, s)

def urlencode_path_esc(m):
    return '%'+binascii.hexlify(m.group(0),'%')

def urlencode_path(s):
    return _urlencode_path_regex.sub(urlencode_path_esc, s)


def urldecode_esc(m):
    return binascii.unhexlify(m.group(0)[1:3])

# bytes -> bytes
def urldecode(s):
    return _urldecode_regex.sub(urldecode_esc, s)

# bytes -> bytes
def querydecode(s):
    return _urldecode_regex.sub(urldecode_esc, s.replace(b'+', b' '))

def redirect(req, location, code=303):
    req.set_status(code)
    req.add_header("Location", location)
    req.write_all(location)
    return True

def error(req, num, text):
    req.set_status(num)
    req.write_all(text)

def session_dict( req ):
    d = req.get_session_ctx()
    if d == None:
        d = {}
        req.set_session_ctx(d)
    return d

class ResponseBuffer:
    def __init__(self, size):
        from _esphttpd import _bufcpy
        self._buf = memoryview(bytearray(size))
        self._cur = 0
        self._bufcpy = _bufcpy

    def set_request(self, req):
        self._req = req
        self._cur = 0

    def write(self, s):
        ix = self._bufcpy(self._buf, self._cur, s)
        if ix == -1:
            self._req.write(self._buf[:self._cur])
            self._cur = 0
            ix = self._bufcpy(self._buf, self._cur, s)
            if ix == -1:
                self._req.write(s)
            else:
                self._cur = ix
        else:
            self._cur = ix


    def flush(self):
        self._req.write_all(self._buf[:self._cur])
        self._cur = 0

class HTTP_Server:

    def _loop(self):
        print("[starting] event_loop()")
        self._server.event_loop()
        print("[stopped] event_loop()")

    def _register_route(self, path, method, func):
        self._server.register(path, method, func)

    def __init__(self, use_tls=False, keyfile=None, certfile=None, writebuf_size=65536):
        self._routes = []
        self._running = False
        self._use_tls = use_tls
        self._keyfile = keyfile
        self._certfile = certfile
        self._writebuf = ResponseBuffer(writebuf_size)

    def start(self):
        from _thread import start_new_thread
        from _esphttpd import http_server
        self._server = http_server()

        self._running=True

        if self._use_tls:
            with open(self._keyfile, "rb") as f:
                key = f.read(-1)+'\0'
            with open(self._certfile, "rb") as f:
                cert = f.read(-1)+'\0'

            self._server.start(key, cert)
        else:
            self._server.start()

        key, cert = None, None
        gc.collect()

        self._thread = start_new_thread(self._loop, ())

        for path, method, func in self._routes:
            self._register_route(path, method, func)

    def stop(self):
        self._server.stop()
        self._running = False

    def buffered(self, func):
        def wrapper(req):
            self._writebuf.set_request(req)
            ret = func(req, self._writebuf)
            self._writebuf.flush()
            return ret
        return wrapper

    def route(self, path, method="GET"):
        print("adding route: ", path)
        def wrap(func):
            self._routes.append( (path, method, func) )
            if self._running:
                self._register_route(path, method, func)
            return func
        return wrap

    def del_route(self, *args):
        print("del route", args)
        for i in range(len(self._routes)-1, -1, -1):
            route = self._routes[i][:len(args)]
            if route == args:
                self._routes.pop( i )
                if self._running:
                    self._server.unregister(*args)

