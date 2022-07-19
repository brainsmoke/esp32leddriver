
import machine

import configform, config

from esphttpd import htmlencode

def validate_essid(essid):
    return len(essid) >= 2

def validate_password(password):
    return len(password) >= 8

def validate_network_type(s):
    return s in ("off", "open", "protected")

def validate_ip(ip):
    t = ip.split('.')

    if len(t) != 4:
        return False
    try:
        t = [ int(i) for i in t ]
    except ValueError:
        return False

    for i in t:
        if not 0 <= i <= 255:
             return False

    if sum(t) == 0:
        return False

    return True

class NetworkConf(configform.ConfigFormElem):
    def __init__(self, caption='Network Config', info=None):
        super().__init__(caption=caption)
        self._set_css_class('network')
        self._info = info
        self._update_content()

    def _update_content(self):

        protected_sel, open_sel, off_sel  = "", "", ""
        essid, password = "", ""

        if config.essid == None:
            off_sel = "selected"
        else:
            essid = config.essid
            if config.password == None:
                open_sel = "selected"
            else:
                password = config.password
                protected_sel = "selected"

        info = "no connection info available"
        if self._info != None:
            info = "<dl>{}</dl>".format(''.join( '<dt>{}<dd>{}'.format(
                htmlencode(t),htmlencode(d)) for t, d in self._info ) )

        self._set_form_content("""<dl>
<dt>Type<dd><select name="type" onchange="this.form.elements['password'].disabled = (this.value!='protected');this.form.elements['essid'].disabled = (this.value=='off');">
<option value="protected" {}>Password Protected
<option value="open" onchange="" {}>Open
<option value="off" {}>Off
</select>
<dt>ESSID<dd><input name="essid" type="text" value="{}" maxlength="32" />
<dt>Password<dd><input name="password" type="text" value="{}" maxlength="128" />
<dt>Conn. info<dd>{}
</dl>
<input type="submit" value="set" />
<script>const f=document.currentScript.parentNode; f.elements['password'].disabled = (f.elements['type'].value!='protected');f.elements['essid'].disabled = (f.elements['type'].value=='off')</script>
""".format(protected_sel, open_sel, off_sel, htmlencode(essid), htmlencode(password), info))

    def _set(self, formdata):
        mode = formdata.get('type', '')
        essid = formdata.get('essid', '')
        password = formdata.get('password', '')

        if not validate_network_type(mode):
            return

        if mode == 'off':
            essid, password = None, None

        elif not validate_essid(essid):
            return

        elif mode == 'open':
            password = None

        elif not validate_password(password):
            return

        config.write_network_conf(essid, password)
        self._info = None
        self._update_content()

class FailsafeConf(configform.ConfigFormElem):
    def __init__(self, caption='Network Config'):
        super().__init__(caption=caption)
        self._set_css_class('network')
        self._update_content()

    def _update_content(self):
        essid = config.failsafe_essid
        if essid == None:
            essid = ""
        password = config.failsafe_password
        if password == None:
            password = ""
        ip = config.failsafe_ip
        if not ip:
            ip = ""
        self._set_form_content("""<dl>
<dt>ESSID<dd><input name="essid" type="text" value="{}" maxlength="32" />
<dt>Password<dd><input name="password" type="text" value="{}" minlength="8" maxlength="128" />
<dt>IP<dd><input name="ip" type="text" value="{}" maxlength="24" />
</dl>
<input type="submit" value="set" />""".format(htmlencode(essid), htmlencode(password), htmlencode(ip)))

    def _set(self, formdata):
        if validate_essid(formdata['essid']) and \
           validate_password(formdata['password']) and \
           validate_ip(formdata['ip']):
            config.write_failsafe_conf(formdata['essid'], formdata['password'], formdata['ip'])
            self._update_content()

def get_form( network_info, reset_func ):
    form = configform.ConfigRoot("/")
    form['network'] = NetworkConf("Network Config", network_info)
    form['failsafe'] = FailsafeConf("Failsafe Network Config")
    form['reset'] = configform.Action(reset_func, lambda: True, 'reset')
    return form

