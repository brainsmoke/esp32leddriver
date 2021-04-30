
import binascii, hashlib, os, re

from csrf import *

def test_csrf():

    class FakeReq:

        def __init__(self, cookie_header, session_ctx):
            self.cookie_header = cookie_header
            self.session_ctx = session_ctx
            self.cookie_out = b''

        def simulate_next_request(self):
            self.cookie_header = b'csrf='+self.cookie_out+b';'

        def simulate_reconnect(self):
            self.session_ctx = None

        def get_host(self):
            host = b'::FFFF:127.0.0.1'
            print ( 'get_host() = {}'.format( repr(b'::FFFF:127.0.0.1') ) )
            return host

        def add_header( self, name, value ):
            print ( 'add_header(name={}, value={})'.format(repr(name), repr(value)) )
            assert (name == b'Set-Cookie')
            self.cookie_out = re.match(b'(^| )csrf=([0-9a-fA-f]*)(;|$)', value).group(2)

        def get_header( self, name ):
            print ( 'get_header(name={}) = {}'.format(repr(name), repr(self.cookie_header)) )
            assert (name == b'Cookie')
            return self.cookie_header

        def get_session_ctx( self ):
            print ( 'get_session_ctx() = {}'.format(repr(self.session_ctx)) )
            return self.session_ctx

        def set_session_ctx( self, ctx ):
            print ( 'set_session_ctx(ctx={})'.format(repr(ctx)) )
            self.session_ctx = ctx

    print ("host_random(): ", repr(host_random()))

    print ("generate_csrf_cookie()")
    cookie = generate_csrf_cookie()
    print ("generate_csrf_cookie() = {}: ".format(repr(cookie)))

    req = FakeReq(cookie_header=b'', session_ctx=None)
    print ("generate_csrf_token(req, {})".format(repr(cookie)))
    print ("generate_csrf_token(req, {}) = {}: ".format(repr(cookie), repr(generate_csrf_token(req, cookie))))

    print ("set_csrf_cookie( req, {} )".format(cookie))
    print ("set_csrf_cookie( req, {} ) = {}".format(cookie, set_csrf_cookie( req, cookie )))

    print ("get_csrf_cookie( req )")
    print ("get_csrf_cookie( req ) = {}".format(get_csrf_cookie( req )))

    print ("verify_csrf_token(req, '')")
    print ("verify_csrf_token(req, '') = {}".format(verify_csrf_token(req, b'')))

    print("req = FakeReq(cookie_header=b'csrf=c5475bb1810a963283b9b4748c57830d;', session_ctx=None)")
    req = FakeReq(cookie_header=b'csrf=c5475bb1810a963283b9b4748c57830d;', session_ctx=None)

    print ("verify_csrf_token(req, b'')")
    print ("verify_csrf_token(req, b'') = {}".format(repr(verify_csrf_token(req, b''))))
    print ("verify_csrf_token(req, b'1352135')")
    print ("verify_csrf_token(req, b'1352135') = {}".format(repr(verify_csrf_token(req, b'1352135'))))
    print ("verify_csrf_token(req, b'bad@$^#@$^$^#')")
    print ("verify_csrf_token(req, b'bad@$^#@$^$^#') = {}".format(repr(verify_csrf_token(req, b'bad@$^#@$^$^#'))))

    print("req = FakeReq(cookie_header=b'', session_ctx=None)")
    req = FakeReq(cookie_header=b'', session_ctx=None)
 
    print ("get_csrf_token( req )")
    cookie = get_csrf_token( req )
    print ("get_csrf_token( req ) = {}".format(repr(cookie)))
    print("req.simulate_next_request()")
    req.simulate_next_request()

    print ("verify_csrf_token(req, b'')")
    print ("verify_csrf_token(req, b'') = {}".format(repr(verify_csrf_token(req, b''))))
    print ("verify_csrf_token(req, b'1352135')")
    print ("verify_csrf_token(req, b'1352135') = {}".format(repr(verify_csrf_token(req, b'1352135'))))
    print ("verify_csrf_token(req, b'bad@$^#@$^$^#')")
    print ("verify_csrf_token(req, b'bad@$^#@$^$^#') = {}".format(repr(verify_csrf_token(req, b'bad@$^#@$^$^#'))))
    print ("good: verify_csrf_token(req, {})".format(repr(cookie)))
    print ("good: verify_csrf_token(req, {}) = {}".format(repr(cookie), repr(verify_csrf_token(req, cookie))))
    print ("verify_csrf_token(req, b'bad@$^#@$^$^#')")
    print ("verify_csrf_token(req, b'bad@$^#@$^$^#') = {}".format(repr(verify_csrf_token(req, b'bad@$^#@$^$^#'))))

    req = FakeReq(cookie_header=b'', session_ctx=None)

    print ("get_csrf_token( req )")
    cookie = get_csrf_token( req )
    print ("get_csrf_token( req ) = {}".format(repr(cookie)))
    print("req.simulate_next_request()")
    req.simulate_next_request()
    print("req.simulate_reconnect()")
    req.simulate_reconnect()

    print ("good: verify_csrf_token(req, {})".format(repr(cookie)))
    print ("good: verify_csrf_token(req, {}) = {}".format(repr(cookie), repr(verify_csrf_token(req, cookie))))



test_csrf()
