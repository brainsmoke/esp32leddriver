
import machine

import configform, config

from esphttpd import htmlencode

def validate_essid(essid):
    return len(essid) >= 2

def validate_password(password):
    return len(password) >= 8

def validate_network_type(s):
    return s in ("open", "protected")

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
    def __init__(self, caption='Network Config'):
        super().__init__(caption=caption)
        self._set_css_class('network')
        self._update_content()

    def _update_content(self):
        essid = config.essid
        password_string = config.password
        if essid == None:
            essid = ""
            password_string = ""

        protected_sel, open_sel,  = "selected", ""
        if password_string == None:
            password_string = ""
            protected_sel, open_sel = "", "selected"

        self._set_form_content("""<dl>
<dt>Type<dd><select name="type" onchange="this.form.elements['password'].disabled = (this.value=='open');">
<option value="protected" {}>Password Protected
<option value="open" onchange="" {}>Open
</select>
<dt>ESSID<dd><input name="essid" type="text" value="{}" maxlength="32" />
<dt>Password<dd><input name="password" type="text" value="{}" maxlength="128" />
</dl>
<input type="submit" value="set" />
<script>const f=document.currentScript.parentNode; f.elements['password'].disabled = (f.elements['type'].value=='open')</script>
""".format(protected_sel, open_sel, htmlencode(essid), htmlencode(password_string)))

    def _set(self, formdata):
        if validate_essid(formdata['essid']) and \
           validate_network_type(formdata['type']):
            if formdata['type'] != 'open' and validate_password(formdata['password']):
                config.write_network_conf(formdata['essid'], formdata['password'])
            else:
                config.write_network_conf(formdata['essid'], None)
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

def get_form():
    form = configform.ConfigRoot("/")
    form['network'] = NetworkConf("Network Config")
    form['failsafe'] = FailsafeConf("Failsafe Network Config")
    form['reset'] = configform.Action(machine.reset, lambda: True, 'reset')
    return form

