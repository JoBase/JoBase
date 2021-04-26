import pyglet
from .resources import *

from .setup import run
from .setup import window
from .setup import key
from .setup import mouse

from .graphics import Image
from .graphics import Circle
from .graphics import Line

from .math import distance
from .math import angle
from .math import direction
from .math import random_number
from .math import random_decimal
from .math import rotate_point
from .math import rotate_toward_goal

from .collision import point_collides_with_shape
from .collision import shape_collides_with_shape
from .collision import point_collides_with_point

for name in dir(pyglet.window.Window):
    if name.startswith('CURSOR_'):
        globals()[name[7:]] = getattr(pyglet.window.Window, name)
        
# dependencies: pyglet, shapely