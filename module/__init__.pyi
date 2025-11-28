from typing import Union, Sequence

vec = Union[int, float, Sequence[float]]

class Base:
    def draw() -> None: ...

    @property
    def pos(self) -> Vector: ...

    @pos.setter
    def pos(self, value: vec) -> None: ...

    @property
    def position(self) -> Vector: ...

    @position.setter
    def position(self, value: vec) -> None: ...

    @property
    def scale(self) -> Vector: ...

    @scale.setter
    def scale(self, value: vec) -> None: ...

    @property
    def anchor(self) -> Vector: ...

    @anchor.setter
    def anchor(self, value: vec) -> None: ...

    @property
    def color(self) -> Vector: ...

    @color.setter
    def color(self, value: vec) -> None: ...

    x: float
    y: float
    angle: float
    red: float
    green: float
    blue: float
    alpha: float

class Vector:
    def __getattr__(self, name: str) -> float: ...
    def __setattr__(self, name: str, value: float) -> None: ...
    def __getitem__(self, index: int) -> float: ...
    def __len__(self) -> int: ...
    def __bool__(self) -> bool: ...
    def __neg__(self) -> tuple: ...
    def __pos__(self) -> tuple: ...
    def __abs__(self) -> tuple: ...
    def __add__(self, other: vec) -> tuple: ...
    def __sub__(self, other: vec) -> tuple: ...
    def __mul__(self, other: vec) -> tuple: ...
    def __mod__(self, other: vec) -> tuple: ...
    def __floordiv__(self, other: vec) -> tuple: ...
    def __truediv__(self, other: vec) -> tuple: ...
    def __iadd__(self, other: vec) -> Vector: ...
    def __isub__(self, other: vec) -> Vector: ...
    def __imul__(self, other: vec) -> Vector: ...
    def __imod__(self, other: vec) -> Vector: ...
    def __ifloordiv__(self, other: vec) -> Vector: ...
    def __itruediv__(self, other: vec) -> Vector: ...

    @property
    def length(self) -> float: ...

class Points:
    def __len__(self) -> int: ...
    def __getitem__(self, index: int) -> Vector: ...
    def __setitem__(self, index: int, value: vec) -> None: ...

class Button:
    def __bool__(self) -> bool: ...

    @property
    def press(self) -> bool: ...

    @property
    def release(self) -> bool: ...

    @property
    def repeat(self) -> bool: ...

    @property
    def down(self) -> bool: ...

class Window:
    def __init__(self, title = "JoBase", width = 640, height = 480, color = (1, 1, 1)) -> None: ...

    def close() -> None: ...
    def maximize() -> None: ...
    def minimize() -> None: ...
    def restore() -> None: ...

    @property
    def color(self) -> Vector: ...

    @color.setter
    def color(self, value: vec) -> None: ...

    @property
    def size(self) -> Vector: ...

    @size.setter
    def size(self, value: vec) -> None: ...

    @property
    def resize(self) -> bool: ...

    @property
    def time(self) -> float: ...

    @property
    def top(self) -> float: ...

    @property
    def left(self) -> float: ...

    @property
    def bottom(self) -> float: ...

    @property
    def right(self) -> float: ...

    title: str
    red: float
    green: float
    blue: float
    width: float
    height: float

class Key:
    pass

class Mouse:
    def __init__(self, x = 0, y = 0) -> None: ...

    @property
    def pos(self) -> Vector: ...

    @pos.setter
    def pos(self, value: vec) -> None: ...

    @property
    def position(self) -> Vector: ...

    @position.setter
    def position(self, value: vec) -> None: ...

    x: float
    y: float

class Camera:
    def __init__(self, x = 0, y = 0) -> None: ...

    @property
    def pos(self) -> Vector: ...

    @pos.setter
    def pos(self, value: vec) -> None: ...

    @property
    def position(self) -> Vector: ...

    @position.setter
    def position(self, value: vec) -> None: ...

    @property
    def scale(self) -> Vector: ...

    @position.setter
    def scale(self, value: vec) -> None: ...

    @property
    def zoom(self) -> Vector: ...

    @position.setter
    def zoom(self, value: vec) -> None: ...

    x: float
    y: float

class Rect(Base):
    def __init__(self, x = 0, y = 0, width = 50, height = 50, angle = 0, color = (0, 0, 0, 1)) -> None: ...

    @property
    def size(self) -> Vector: ...

    @size.setter
    def size(self, value: vec) -> None: ...

    width: float
    height: float

class Shape(Base):
    def __init__(self, points: Sequence[vec] = ((0, 25), (25, -25), (-25, -25)), x = 0, y = 0, angle = 0, color = (0, 0, 0, 1)) -> None: ...

    @property
    def points(self) -> Points: ...

    @points.setter
    def points(self, value: Sequence[vec]) -> None: ...

class Line(Shape):
    def __init__(self, points: Sequence[vec] = ((-25, -25), (25, 25)), width = 2, x = 0, y = 0, angle = 0, color = (0, 0, 0, 1)) -> None: ...

    loop: bool
    width: float
    miter: float

class Image(Rect):
    def __init__(self, name = MAN, x = 0, y = 0, angle = 0, width = 0, height = 0, color = (1, 1, 1, 1)) -> None: ...

    name: str

class Text(Base):
    def __init__(self, content = "Text", x = 0, y = 0, size = 50, angle = 0, color = (0, 0, 0, 1), font = DEFAULT) -> None: ...

    @property
    def width(self) -> float: ...

    size: float
    content: str
    font: int

class Circle(Base):
    def __init__(self, x = 0, y = 0, radius = 25, color = (0, 0, 0, 1)) -> None: ...

    diameter: float

class Sound:
    def __init__():
        pass

def run() -> None: ...

window: Window
mouse: Mouse
camera: Camera
key: Key

MAN: str
DEFAULT = 0
CODE = 1

WHITE = 1, 1, 1
BLACK = 0, 0, 0
GRAY = .5, .5, .5
DARK_GRAY = .2, .2, .2
LIGHT_GRAY = .8, .8, .8
BROWN = .6, .2, .2
TAN = .8, .7, .6
RED = 1, 0, 0
DARK_RED = .6, 0, 0
SALMON = 1, .5, .5
ORANGE = 1, .5, 0
GOLD = 1, .8, 0
YELLOW = 1, 1, 0
OLIVE = .5, .5, 0
LIME = 0, 1, 0
DARK_GREEN = 0, .4, 0
GREEN = 0, .5, 0
AQUA = 0, 1, 1
BLUE = 0, 0, 1
LIGHT_BLUE = .5, .8, 1
AZURE = .9, 1, 1
NAVY = 0, 0, .5
PURPLE = .5, 0, 1
PINK = 1, .75, .8
MAGENTA = 1, 0, 1

__all__ = [
    "window", "mouse", "camera", "key", "run",
    "Rect", "Shape", "Line", "Image", "Circle", "Text",
    "Sound",
    "MAN", "DEFAULT", "CODE",
    "WHITE", "BLACK", "GRAY", "DARK_GRAY", "LIGHT_GRAY", "BROWN", "TAN", "RED", "DARK_RED", "SALMON", "ORANGE", "GOLD", "YELLOW",
    "OLIVE", "LIME", "DARK_GREEN", "GREEN", "AQUA", "BLUE", "LIGHT_BLUE", "AZURE", "NAVY", "PURPLE", "PINK", "MAGENTA",
]