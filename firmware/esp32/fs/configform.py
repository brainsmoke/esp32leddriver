import binascii

# very much work in progress / operating on static & trusted inputs hopefully :-P
# pre-generate as much of the output to speed up http response
# yes, I hate this too :-P

class ConfigElem:

    def __init__(self, caption=None, tag='div'):
        self._tag = tag
        self._class = ''
        self._attribs = {}
        self._attrib_string = '' # cached string based on the above
        self._path = ''          # form action destination
        self._extra_attrib = ''  # used to insert form arguments
        self._caption = caption
        self._update()

    # pre-calculate dynamically generated html
    def _update(self):
        if len(self._class) or 'class' in self._attribs:
            classes = ' class="{}"'.format(''.join( (self._class, self._attribs.get('class', '')) ))
        else:
            classes = ''

        self._start = '<{}{}{}{}>\n'.format(self._tag, classes, self._extra_attrib, self._attrib_string)
        self._end = '</{}>\n'.format(self._tag)

    def _set_css_class(self, classname):
        self._class = classname
        self._update()

    def _set_html_attr(self, attr):
        for name, value in attr.items():
            if value == None:
                if name in self._attribs:
                    del self._attribs[name]
            else:
                self._attribs[name] = value

        self._attrib_string = ''.join( ' {}="{}"'.format(name, self._attribs[name]) for name in self._attribs if name != 'class' )
        self._update()

    def _set(self, value):
        raise ValueError("element unsettable")

    def _lookup(self, path_list, ix):
        if len(path_list) == ix:
            return self
        else:
            raise ValueError("bad path")

    def lookup(self, path):
        path_list = path.split('/')
        if path_list[0] != '':
            raise ValueError("bad path")

        return self._lookup(path_list, 1)

    def set(self, path, value):
        obj = self.lookup(path)
        obj._set(value)

    def set_html_attr(self, path, attr={}):
        obj = self.lookup(path)
        obj._set_html_attr(attr)

    def set_path(self, path):
        self._path = path
        self._update()

class ConfigGroup(ConfigElem):

    def __init__(self, path, caption=None, tag='div'):
        self._list = []
        self._dict = {}
        super().__init__(caption=caption, tag=tag)
        if path:
            self.set_path(path)
        self._set_css_class('group')

    def _update(self):
        super()._update()
        if self._caption:
            self._start = '{}<h2>{}</h2>\n'.format(self._start, self._caption)

    def __setitem__(self, name, obj):
        if name in self._dict:
            raise ValueError("name already used")
        if '/' in name:
            raise ValueError("bad name")
        self._list.append( (name, obj) )
        self._dict[name] = obj
        obj.set_path(self._basepath(self._path)+name)

    def __getitem__(self, name):
        return self._dict[name]

    def __contains__(self, name):
        return name in self._dict

    def _lookup(self, path_list, ix):
        if len(path_list) <= ix or path_list[ix] not in self._dict:
            raise ValueError("bad path")

        return self._dict[path_list[ix]]._lookup(path_list, ix+1)

    def _basepath(self, path):
        if path.endswith('/'):
            return path
        else:
            return path + '/'

    def set_path(self, path):
        self._path = path

        base = self._basepath(path)

        for name, obj in self._list:
            obj.set_path(base + name)

        self._update()

    def add_color(self, name, getter, setter, caption=None):
        self[name] = Color(getter, setter, caption=caption)

    def add_slider(self, name, min, max, step, getter, setter, caption=None):
        self[name] = Slider(min, max, step, getter, setter, caption=caption)

    def add_action(self, name, action_func, enabled_func, caption=None):
        self[name] = Action(action_func, enabled_func, caption=caption)

    def add_group(self, name, caption=None):
        tree = ConfigGroup(path=None, caption=caption)
        self[name] = tree
        return tree

    def add_select_group(self, name, get_selected_func, caption=None):
        tree = ConfigSelectGroup(get_selected_func, caption=caption)
        self[name] = tree
        return tree

    def print(self, indent='', name=''):
        print('{}{}:'.format(indent, name))
        group_indent=indent+'    '
        for name, elem in self._list:
            elem.print(group_indent, name)

    def html(self, out, csrf_tag=''):
        out.write(self._start);

        for name, obj in self._list:
            obj.html(out, csrf_tag)

        out.write(self._end);

class ConfigRoot(ConfigGroup):
    def __init__(self, path, caption=None):
        self._list = []
        self._dict = {}
        super().__init__(path, caption=caption, tag='main')
        self._set_css_class('root')

class ConfigSelectGroup(ConfigGroup):
    def __init__(self, selected_func, caption=None):
        self._get_selected_func = selected_func
        super().__init__(path=None, caption=caption)
        self._set_css_class('select_group')

    def print(self, indent='', name=''):
        print('{}{}: [{}]'.format(indent, name, ','.join( name for name, _ in self._list ) ) )
        group_indent=indent+'    '
        for name, elem in self._list:
            elem.print(group_indent, name)

    def html(self, out, csrf_tag=''):
        out.write(self._start);
        name = self._get_selected_func()

        if name in self._dict:
            self[name].html(out, csrf_tag)

        out.write(self._end);

class ConfigFormElem(ConfigElem):

    def __init__(self, caption=None):
        self._form_content = ''
        super().__init__(caption=caption, tag='form')

    def _set_form_content(self, content):
        self._form_content = content
        self._update()

    def _update(self):
        self._extra_attrib = ' method="POST" action="{}"'.format(self._path)

        super()._update()
        if self._caption:
            self._start = '{}<h4>{}</h4>\n'.format(self._start, self._caption)

        self._start += self._form_content

    def html(self, out, csrf_tag=''):
        out.write(self._start);
        out.write(csrf_tag);
        out.write(self._end);

def _parse_color(s):
    if len(s) != 7 or s[0] != '#':
        raise ValueError("bad color {}".format(s))

    return ( int(s[1:3], 16), int(s[3:5], 16), int(s[5:7], 16) )

def _html_color(rgb):
    return '#{:02x}{:02x}{:02x}'.format(rgb[0]&0xff,rgb[1]&0xff,rgb[2]&0xff)

class Color(ConfigFormElem):
    def __init__(self, getter, setter, caption=None):
        self.getter, self.setter = getter, setter
        super().__init__(caption=caption)
        self._set_form_content('<input type="color" name="value" value="{}" onchange="this.form.submit();" />'.format(_html_color(self.getter())))
        self._set_css_class('color')

    def _set(self, value):
        self.setter(_parse_color(value))
        self._set_form_content('<input type="color" name="value" value="{}" onchange="this.form.submit();" />'.format(_html_color(self.getter())))
        self._update()

    def print(self, indent='', name=''):
        print('{}{}: rgb{}'.format(indent, name, self.getter()))


class Slider(ConfigFormElem):
    def __init__(self, min, max, step, getter, setter, caption=None):
        self.min, self.max, self.step, self.getter, self.setter = min, max, step, getter, setter
        if int(step) == step and int(min) == min and int(max) == max:
            self.type = int
        else:
            self.type = float
        super().__init__(caption=caption)
        self._set_css_class('slider')

        self._set_form_content('<input name="value" type="range" min="{}" max="{}" step="{}" value="{}" onchange="this.form.submit();" />'.format(self.min, self.max, self.step, self.getter()))
        self._update()

    def _set(self, value):
        self.setter(self.type(value))
        self._set_form_content('<input name="value" type="range" min="{}" max="{}" step="{}" value="{}" onchange="this.form.submit();" />'.format(self.min, self.max, self.step, self.getter()))
        self._update()

    def print(self, indent='', name=''):
        print('{}{}: cur={}, min={}, max={}, step={}'.format(indent, name, self.getter(), self.min, self.max, self.step))

class Action(ConfigFormElem):
    def __init__(self, action_func, enabled_func, caption=None):
        self.action_func = action_func
        self.enabled_func = enabled_func
        super().__init__(caption=None)
        self._text = caption
        self._set_css_class('action')

    def _set(self, value):
        if self.enabled_func():
            self.action_func()

    def print(self, indent='', name=''):
        print('{}{}: action, enabled={}'.format(indent, name, self.enabled_func()))

    def html(self, out, csrf_tag=''):
        out.write(self._start);
        out.write('<input type="submit" value="{}"{} />'.format(self._text, (' disabled','')[bool(self.enabled_func())]))
        out.write(csrf_tag);
        out.write(self._end);

