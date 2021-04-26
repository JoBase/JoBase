import pyglet, sys
from .collision import point_collides_with_shape, shape_collides_with_shape

class Window:
    def __init__(self, width, height, caption):
        self._update()
                    
        self._window = pyglet.window.Window(width, height, caption, True)
        self._fullscreen = False
        self._visible = True
        self._maximum_size = [10000, 10000]
        self._minimum_size = [0, 0]
        self._key = True
        self._mouse = True
        self._caption = caption
        self._color = (0, 0, 0)
                
    def begin(self):
        
        def loop(time):
            self._window.clear()
            
            module.loop()
                
            if self._close:
                self._window.close()
                
            self._update()
            mouse._update()
            key._update()

        pyglet.clock.schedule_interval(loop, 1 / 60)
        
        @self._window.event
        def on_draw():
            loop(None)
            
        @self._window.event
        def on_key_press(symbol, modifiers):
            key._press = True
            key._list.append(symbol)
        
        @self._window.event
        def on_key_release(symbol, modifiers):
            key._release = True
            key._list.remove(symbol)
        
        @self._window.event
        def on_mouse_press(x, y, button, modifiers):
            mouse._press = True
        
            if button is pyglet.window.mouse.LEFT: mouse._left = True
            elif button is pyglet.window.mouse.MIDDLE: mouse._middle = True
            elif button is pyglet.window.mouse.RIGHT: mouse._right = True
                
        @self._window.event
        def on_mouse_release(x, y, button, modifiers):
            mouse._release = True
            
            if button is pyglet.window.mouse.LEFT: mouse._left = False
            elif button is pyglet.window.mouse.MIDDLE: mouse._middle = False
            elif button is pyglet.window.mouse.RIGHT: mouse._right = False
                    
        @self._window.event
        def on_mouse_scroll(x, y, scroll_x, scroll_y):
            mouse._scroll = True
            mouse._scroll_x = scroll_x
            mouse._scroll_y = scroll_y
                    
        @self._window.event
        def on_mouse_motion(x, y, dx, dy):
            mouse._move = True
            mouse._move_x = dx
            mouse._move_y = dy
            mouse._position = [x, y]
                    
        @self._window.event
        def on_mouse_enter(x, y):
            mouse._enter = True
        
        @self._window.event
        def on_mouse_leave(x, y):
            mouse._leave = True
                    
        @self._window.event
        def on_activate():
            self._activate = True
                    
        @self._window.event
        def on_deactivate():
            self._deactivate = True
                    
        @self._window.event
        def on_close():
            self._close = True
                    
        @self._window.event
        def on_hide():
            self._hide = True
                    
        @self._window.event
        def on_move(x, y):
            self._move = True
                    
        @self._window.event
        def on_resize(width, height):
            self._resize = True
                    
        @self._window.event
        def on_show():
            self._show = True
                    
        @self._window.event
        def on_text(text):
            self._text = text
                    
    def _update(self):    
        self._activate = False
        self._deactivate = False
        self._close = False
        self._hide = False
        self._move = False
        self._resize = False
        self._show = False
        self._text = ''
        
    def _get_shape(self):
        return (0, self.height), (self.width, self.height), (self.width, 0), (0, 0)
        
    @property
    def activate(self):
        return self._activate
    
    @property
    def deactivate(self):
        return self._deactivate
    
    @property
    def close(self):
        return self._close
    
    @property
    def hide(self):
        return self._hide
    
    @property
    def move(self):
        return self._move
    
    @property
    def resize(self):
        return self._resize
    
    @property
    def show(self):
        return self._show
    
    @property
    def text(self):
        return self._text

    def quit(self):
        self._window.close()

    def maximize(self):
        self._window.maximize()

    def minimize(self):
        self._window.minimize()
        
    def make_active(self):
        self._window.activate()
        
    def collides_with(self, other):
        if type(other) is Mouse: return point_collides_with_shape(other.x, other.y, self._get_shape())
        return shape_collides_with_shape(self._get_shape(), other._get_shape())
        
    @property
    def red(self):
        return self.color[0]
    
    @red.setter
    def red(self, value):
        self.color = (value, self.color[1], self.color[2])
    
    @property
    def green(self):
        return self.color[1]
    
    @green.setter
    def green(self, value):
        self.color = (self.color[0], value, self.color[2])
    
    @property
    def blue(self):
        return self.color[2]
    
    @blue.setter
    def blue(self, value):
        self.color = (self.color[0], self.color[1], value)
        
    @property
    def color(self):
        return self._color
    
    @color.setter
    def color(self, value):
        pyglet.gl.glClearColor(*value, 1)
        self._color = value

    @property
    def x(self):
        return self.position[0]
    
    @x.setter
    def x(self, value):
        self.position = (value, self.position[1])

    @property
    def y(self):
        return self.position[1]
    
    @y.setter
    def y(self, value):
        self.position = (self.position[0], value)
    
    @property
    def position(self):
        return self._window.get_location()
    
    @position.setter
    def position(self, value):
        self._window.set_location(*value)

    @property
    def width(self):
        return self.size[0]
    
    @width.setter
    def width(self, value):
        self.size = (value, self.size[1])

    @property
    def height(self):
        return self.size[1]
    
    @height.setter
    def height(self, value):
        self.size = (self.size[0], value)
    
    @property
    def size(self):
        return self._window.get_size()
    
    @size.setter
    def size(self, value):
        self._window.set_size(*value)

    @property
    def caption(self):
        return self._caption
    
    @caption.setter
    def caption(self, value):
        self._window.set_caption(value)
        self._caption = value
        
    @property
    def fullscreen(self):
        return self._fullscreen
    
    @fullscreen.setter
    def fullscreen(self, value):
        self._window.set_fullscreen(value)
        self._fullscreen = value
    
    @property
    def visible(self):
        return self._visible
    
    @visible.setter
    def visible(self, value):
        self._window.set_visible(value)
        self._visible = value
    
    @property
    def maximum_width(self):
        return self.maximum_size[0]
    
    @maximum_width.setter
    def maximum_width(self, value):
        self.maximum_size = (value, self.maximum_size[1])
    
    @property
    def maximum_height(self):
        return self.maximum_size[1]
    
    @maximum_height.setter
    def maximum_height(self, value):
        self.maximum_size = (self.maximum_size[0], value)   
    
    @property
    def maximum_size(self):
        return self._maximum_size
    
    @maximum_size.setter
    def maximum_size(self, value):
        self._window.set_maximum_size(*value)
        self._maximum_size = value
    
    @property
    def minimum_width(self):
        return self.minimum_size[0]
    
    @minimum_width.setter
    def minimum_width(self, value):
        self.minimum_size = (value, self.minimum_size[1])
    
    @property
    def minimum_height(self):
        return self.minimum_size[1]
    
    @minimum_height.setter
    def minimum_height(self, value):
        self.minimum_size = (self.minimum_size[0], value)
    
    @property
    def minimum_size(self):
        return self._minimum_size
    
    @minimum_size.setter
    def minimum_size(self, value):
        self._window.set_minimum_size(*value)
        self._minimum_size = value
    
    @property
    def key(self):
        return self._key
    
    @key.setter
    def key(self, value):
        self._window.set_exclusive_keyboard(not value)
        self._key = value
    
    @property
    def mouse(self):
        return self._mouse
    
    @mouse.setter
    def mouse(self, value):
        self._window.set_exclusive_mouse(not value)
        self._mouse = value  

class Key:
    def __init__(self):
        self._update()
        self._list = []
                
    def _update(self):
        self._press = False
        self._release = False
        
    @property
    def press(self):
        return self._press
    
    @property
    def release(self):
        return self._release
    
    def __getattr__(self, name):
        if name.upper() in dir(pyglet.window.key):
            return getattr(pyglet.window.key, name.upper()) in self._list
                
class Mouse:
    def __init__(self):
        self._visible = True
        self._cursor = window._window.CURSOR_DEFAULT        
        self._position = (0, 0)
        
        self._left = False
        self._middle = False
        self._right = False        
        
        self._update()
        
    def _update(self):
        self._press = False
        self._release = False
        self._enter = False
        self._leave = False
        self._scroll = False
        self._scroll_x = 0
        self._scroll_y = 0
        self._move = False
        self._move_x = 0
        self._move_y = 0
        
    def collides_with(self, other):
        return point_collides_with_shape(self.x, self.y, other._get_shape())
        
    @property
    def scroll(self):
        return self._scroll
    
    @property
    def scroll_x(self):
        return self._scroll_x
    
    @property
    def scroll_y(self):
        return self._scroll_y
    
    @property
    def move(self):
        return self._move
    
    @property
    def move_x(self):
        return self._move_x
    
    @property
    def move_y(self):
        return self._move_y
    
    @property
    def press(self):
        return self._press
    
    @property
    def release(self):
        return self._release
    
    @property
    def left(self):
        return self._left
    
    @property
    def middle(self):
        return self._middle
    
    @property
    def right(self):
        return self._right
    
    @property
    def enter(self):
        return self._enter
    
    @property
    def leave(self):
        return self._leave
            
    @property
    def x(self):
        return self.position[0]
    
    @x.setter
    def x(self, value):
        self.position = (value, self.position[1])
    
    @property
    def y(self):
        return self.position[1]
    
    @y.setter
    def y(self, value):
        self.position = (self.position[0], value)
    
    @property
    def position(self):
        return self._position
    
    @position.setter
    def position(self, value):
        window._window.set_mouse_position(*value)
        self._position = value
    
    @property
    def visible(self):
        return self._visible
    
    @visible.setter
    def visible(self, value):
        window._window.set_mouse_visible(value)
        self._visible = value
    
    @property
    def cursor(self):
        return self._cursor
    
    @cursor.setter
    def cursor(self, value):
        window._window.set_mouse_cursor(window._window.get_system_mouse_cursor(value))
        self._cursor = value
    
def run():
    if 'loop' in dir(module):
        window.begin()
        
    pyglet.app.run()
    
module = sys.modules['__main__']
window = Window(800, 600, "JoBase")
key = Key()
mouse = Mouse()