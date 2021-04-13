
class HTTP_Server:

	def __init__(self):
		import uasyncio
		from _esphttpd import http_server
		self._server = http_server()
		self._server.start()

		async def loop():
			print("[starting] event_loop()")
			self._server.event_loop()
			print("[stopped] event_loop()")

		self._task = uasyncio.create_task(loop())

	def route(self, path, method):
		def wrap(func):
			print("wrapper")
			self._server.register(path, method, func)
			print("end wrapper")
			return func
		return wrap


server = HTTP_Server()

@server.route("/test", "GET")
def hello(req):
	req.write_all("Hello World")
	return True

import uasyncio
uasyncio.get_event_loop().run_forever()


