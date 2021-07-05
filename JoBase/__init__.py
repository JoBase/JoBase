"""
Copyright (c) 2020, Reuben Ford

Welcome to JoBase.

JoBase is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
at your option) any later version.

JoBase is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
"""

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
