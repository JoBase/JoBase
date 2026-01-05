from typing import Union, Sequence

vec = Union[int, float, Sequence[float]]

class Base:
    def draw(self) -> None: ...
    def blit(self, screen: Screen) -> None: ...
    def collide(self, target: Base | Mouse) -> bool: ...
    def look_at(self, target: Base | Camera) -> None: ...
    def move_to(self, target: Base | Camera, value = 1) -> None: ...

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
    top: float
    bottom: float
    left: float
    right: float

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
    def __init__(self, title = "JoBase", width: float = 640, height: float = 480, color = (1, 1, 1)) -> None: ...

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
    def __init__(self, x: float = 0, y: float = 0) -> None: ...

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
    def __init__(self, x: float = 0, y: float = 0) -> None: ...

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
    def __init__(self, x: float = 0, y: float = 0, width: float = 50, height: float = 50, angle: float = 0, color = (0, 0, 0, 1)) -> None: ...

    @property
    def size(self) -> Vector: ...

    @size.setter
    def size(self, value: vec) -> None: ...

    width: float
    height: float

class Shape(Base):
    def __init__(self, points: Sequence[vec] = ((0, 25), (25, -25), (-25, -25)), x: float = 0, y: float = 0, angle: float = 0, color = (0, 0, 0, 1)) -> None: ...

    @property
    def points(self) -> Points: ...

    @points.setter
    def points(self, value: Sequence[vec]) -> None: ...

class Line(Shape):
    def __init__(self, points: Sequence[vec] = ((-25, -25), (25, 25)), width: float = 2, x: float = 0, y: float = 0, angle: float = 0, color = (0, 0, 0, 1)) -> None: ...

    loop: bool
    width: float
    miter: float

class Image(Rect):
    def __init__(self, name = MAN, x: float = 0, y: float = 0, angle: float = 0, width: float = 0, height: float = 0, color = (1, 1, 1, 1)) -> None: ...

    name: str

class Text(Base):
    def __init__(self, content = "Text", x: float = 0, y: float = 0, size: float = 50, angle: float = 0, color = (0, 0, 0, 1), font = DEFAULT) -> None: ...

    @property
    def width(self) -> float: ...

    size: float
    content: str
    font: int

class Circle(Base):
    def __init__(self, x: float = 0, y: float = 0, radius: float = 25, color = (0, 0, 0, 1)) -> None: ...

    diameter: float

class Sound:
    def __init__(self, name = PICKUP) -> None: ...
    def play() -> None: ...
    def pause() -> None: ...
    def resume() -> None: ...

    @property
    def amp(self) -> float: ...

    @property
    def samples(self) -> tuple[float, ...]: ...

    @property
    def playing(self) -> bool: ...

    @property
    def paused(self) -> bool: ...

class Screen(Rect):
    def __init__(self, width = window.width, height = window.height) -> None: ...
    def save(self, name: str) -> None: ...

def run() -> None: ...
def random(x: float, y: float = 1) -> None: ...

window: Window
mouse: Mouse
camera: Camera
key: Key

PICKUP: str
BLIP: str

MAN: str
COIN: str
ENEMY: str

DEFAULT = 0
CODE = 1
SERIF = 2
DISPLAY = 3
PIXEL = 4

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
    "window", "mouse", "camera", "key", "run", "random",
    "Rect", "Shape", "Line", "Image", "Circle", "Text",
    "Sound", "Screen",
    "MAN", "COIN", "ENEMEY",
    "DEFAULT", "CODE", "SERIF", "DISPLAY", "PIXEL",
    "PICKUP", "BLIP",
    "WHITE", "BLACK", "GRAY", "DARK_GRAY", "LIGHT_GRAY", "BROWN", "TAN", "RED", "DARK_RED", "SALMON", "ORANGE", "GOLD", "YELLOW",
    "OLIVE", "LIME", "DARK_GREEN", "GREEN", "AQUA", "BLUE", "LIGHT_BLUE", "AZURE", "NAVY", "PURPLE", "PINK", "MAGENTA"
]