import gc

def redirect(req, location):
    req.set_status(303)
    req.add_header("Location", location)
    req.write_all(location)

class HTTP_Server:

    def _loop(self):
        print("[starting] event_loop()")
        self._server.event_loop()
        print("[stopped] event_loop()")

    def _register_route(self, path, method, func):
        self._server.register(path, method, func)

    def __init__(self, use_tls=False, keyfile=None, certfile=None):
        self._routes = []
        self._running = False
        self._use_tls = use_tls
        self._keyfile = keyfile
        self._certfile = certfile

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

    def route(self, path, method="GET"):
        def wrap(func):
            self._routes.append( (path, method, func) )
            if self._running:
                self._register_route(path, method, func)
            return func
        return wrap

    def del_route(self, *args):
        for i in range(len(self._routes)-1, -1, -1):
            route = self._routes[i][:len(args)]
            if route == args:
                self._routes.pop( i )
                if self._running:
                    self._server.unregister(*args)

