
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
    def __init__(self, caption='Network', info=None, get_networks_func=None):
        super().__init__(caption=caption)
        self._set_css_class('network')
        self._info = info
        if get_networks_func is None:
            get_networks_func = lambda: []
        self._get_networks_func = get_networks_func
        self._update_networks()
        self._update_content()

    def _update_networks(self):
        get_networks = self._get_networks_func
        self._networks = '<datalist id="wifi_networks">\n'+ ''.join( '<option value="{}" />'.format(htmlencode(network)) for network in get_networks())+'\n</datalist>\n'

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
<dt>ESSID<dd><input name="essid" type="text" value="{}" list="wifi_networks" maxlength="32" autocomplete="off" />
<button name="action" value="rescan" >rescan</button>
{}
<dt>Password<dd><input name="password" type="text" value="{}" maxlength="128" autocomplete="off" />
<dt>Conn. info<dd>{}
</dl>
<input type="submit" value="set" />
<script>const f=document.currentScript.parentNode; f.elements['password'].disabled = (f.elements['type'].value!='protected');f.elements['essid'].disabled = (f.elements['type'].value=='off')</script>
""".format(protected_sel, open_sel, off_sel, htmlencode(essid), self._networks, htmlencode(password), info))

    def _set(self, formdata):
        mode = formdata.get('type', '')
        essid = formdata.get('essid', '')
        password = formdata.get('password', '')
        action = formdata.get('action', None)

        if action == 'rescan':
            self._update_networks()
            self._update_content()
            return

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
    def __init__(self, caption='Failsafe Network'):
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
        if config.failsafe_auto_fallback:
            checked = "checked "
        else:
            checked = ""

        self._set_form_content("""<dl>
<dt>ESSID<dd><input name="essid" type="text" value="{}" maxlength="32" />
<dt>Password<dd><input name="password" type="text" value="{}" minlength="8" maxlength="128" />
<dt>IP<dd><input name="ip" type="text" value="{}" maxlength="24" />
</dl>
<p><input name="auto_fallback" id="auto_fallback" type="checkbox" value="true" {}/><label for="auto_fallback">Automatically fall back to failsafe mode when network fails on boot</label></p>
<input type="submit" value="set" />""".format(htmlencode(essid), htmlencode(password), htmlencode(ip), htmlencode(checked)))

    def _set(self, formdata):
        essid = formdata['essid']
        password = formdata['password']
        ip = formdata['ip']
        auto_fallback = formdata.get('auto_fallback', '') == 'true'
        if validate_essid(essid) and \
           validate_password(password) and \
           validate_ip(ip):
            config.write_failsafe_conf(essid, password, ip, auto_fallback)
            self._update_content()

def get_form( network_info, get_networks_func, reset_func, form=None ):
    if form == None:
        form = configform.ConfigRoot("/")
    form['network'] = NetworkConf("Network", network_info, get_networks_func)
    form['failsafe'] = FailsafeConf("Failsafe Network")
    form['reset'] = configform.Action(reset_func, lambda: True, 'reset')
    return form

