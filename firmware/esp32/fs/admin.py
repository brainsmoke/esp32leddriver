
import machine

import configform, config

from esphttpd import htmlencode

def validate_essid(essid):
    return True

def validate_password(password):
    return True

def validate_ip(ip):
    return True

class NetworkConf(configform.ConfigFormElem):
    def __init__(self, caption='Network Config'):
        super().__init__(caption=caption)
        self._set_css_class('network')
        self._update_content()

    def _update_content(self):
        essid = config.essid
        if not essid:
            essid = ""
        password = config.password
        if not password:
            password = ""
        self._set_form_content("""<dl>
<dt>ESSID<dd><input name="essid" type="text" value="{}" maxlength="32" />
<dt>Password<dd><input name="password" type="text" value="{}" maxlength="128" />
</dl>
<input type="submit" value="set" />""".format(htmlencode(essid), htmlencode(password)))

    def _set(self, formdata):
        if validate_essid(formdata['essid']) and \
           validate_password(formdata['password']):
            config.write_network_conf(formdata['essid'], formdata['password'])
        self._update_content()

class FailsafeConf(configform.ConfigFormElem):
    def __init__(self, caption='Network Config'):
        super().__init__(caption=caption)
        self._set_css_class('network')
        self._update_content()

    def _update_content(self):
        essid = config.failsafe_essid
        if not essid:
            essid = ""
        password = config.failsafe_password
        if not password:
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

