
import binascii, hashlib, os, re

import esphttpd

_host_random = None
def host_random():
    global _host_random
    if _host_random == None:
        _host_random = os.urandom(16)
    return _host_random

# Generating tokens

def generate_csrf_cookie():
    return os.urandom(16)

def generate_csrf_token( req, cookie ):
    hash_ = hashlib.sha256( cookie )
    hash_.update( req.get_host() )
    hash_.update( host_random() )
    return binascii.b2a_base64(hash_.digest())[:24]

# Getting/setting cookies

def set_csrf_cookie( req, cookie ):
    req.add_header( b'Set-Cookie', b'csrf=' + binascii.hexlify(cookie) + b'; SameSite=Lax; HttpOnly' )

_get_cookie_regex = re.compile(b'(^| )csrf=([^;]*)(;|$)')

def get_csrf_cookie( req ):
    cookie_header = req.get_header(b'Cookie')
    match = _get_cookie_regex.match( cookie_header )
    if match == None:
        return None
    hexcookie = match.group(2)
    if len(hexcookie) != 32:
        return None
    return binascii.unhexlify(hexcookie)

# Interface

def verify_csrf_token( req, token ):
    d = esphttpd.session_dict( req )
    sess_token = d.get( 'csrf_token', None ) # token present in current session (= tcp connection)
    if sess_token != None and token == sess_token:
        return True

    cookie = get_csrf_cookie( req )
    if cookie == None:
        return False
    token_from_cookie = generate_csrf_token( req, cookie )

    if token == token_from_cookie:
        d['csrf_token'] = token_from_cookie
        return True

    return False

def get_csrf_token( req ):
    d = esphttpd.session_dict( req )
    token = d.get( 'csrf_token', None )
    if token != None:
        return token

    cookie = get_csrf_cookie( req )
    if cookie == None:
        cookie = generate_csrf_cookie()
        set_csrf_cookie( req, cookie )

    token = generate_csrf_token( req, cookie )
    d['csrf_token'] = token
    return token

