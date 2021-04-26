import pyglet, math

from .math import rotate_point
from .resources import MAN
from .collision import point_collides_with_shape, shape_collides_with_shape
from .setup import Mouse

class Base():
    def __init__(self, x, y, rotation, color, opacity, amount, shape, texture = 't3f'):        
        self._position = (x, y)
        self._scale = (1, 1)
        self._rotation = rotation
        self._color = color
        self._opacity = opacity
        self._anchor = (0, 0)
        
        self._vertices = pyglet.graphics.Batch().add(amount, shape, None, 'v2f', 'c4B', texture)
        self._colors = [color for index in range(amount)]
        
    def __del__(self):
        try: self._vertices.delete()
        except: pass
        
    def _bind(self):
        pyglet.gl.glPushAttrib(pyglet.gl.GL_COLOR_BUFFER_BIT)
        pyglet.gl.glEnable(pyglet.gl.GL_BLEND)
        pyglet.gl.glBlendFunc(pyglet.gl.GL_SRC_ALPHA, pyglet.gl.GL_ONE_MINUS_SRC_ALPHA)
        
    def _unbind(self):
        pyglet.gl.glDisable(pyglet.gl.GL_BLEND)
        pyglet.gl.glPopAttrib()
        
    def _make_colors(self):
        colors = [(int(color[0] * 255), int(color[1] * 255), int(color[2] * 255), int(self.opacity * 255)) for color in self._colors]
        self._vertices.colors[:] = [value for color in colors for value in color]
        
    def _get_shape(self):
        shape = []
        
        for index in range(int(len(self._vertices.vertices) / 2)):
            shape.append((
                self._vertices.vertices[index * 2],
                self._vertices.vertices[index * 2 + 1]))
            
        return shape        
        
    def collides_with(self, other):
        if type(other) is Mouse: return point_collides_with_shape(other.x, other.y, self._get_shape())
        return shape_collides_with_shape(self._get_shape(), other._get_shape())
        
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
        self._position = value
        self._make_vertices()
    
    @property
    def scale_x(self):
        return self._scale[0]
    
    @scale_x.setter
    def scale_x(self, value):
        self._scale = (value, self._scale[1])
        self._make_vertices()
    
    @property
    def scale_y(self):
        return self._scale[1]
    
    @scale_y.setter
    def scale_y(self, value):
        self._scale = (self._scale[0], value)
        self._make_vertices()
    
    @property
    def scale(self):
        return (self._scale[0] + self._scale[1]) / 2
    
    @scale.setter
    def scale(self, value):
        self._scale = (value, value)
        self._make_vertices()
    
    @property
    def rotation(self):
        return self._rotation
    
    @rotation.setter
    def rotation(self, value):
        self._rotation = value
        self._make_vertices()
    
    @property
    def anchor_x(self):
        return self._anchor[0]
    
    @anchor_x.setter
    def anchor_x(self, value):
        self._anchor = (value, self.anchor[1])
    
    @property
    def anchor_y(self):
        return self._anchor[1]
    
    @anchor_y.setter
    def anchor_y(self, value):
        self._anchor = (self.anchor[0], value)
    
    @property
    def anchor(self):
        return self._anchor
    
    @anchor.setter
    def anchor(self, value):
        self._anchor = value
        self._make_vertices()
    
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
        self._color = value
        self.colors = [value for index in range(len(self._colors))]
    
    @property
    def colors(self):
        return self._colors
    
    @colors.setter
    def colors(self, value):
        self._colors = value
        self._make_colors()   
    
    @property
    def opacity(self):
        return self._opacity
    
    @opacity.setter
    def opacity(self, value):
        self._opacity = value
        self._make_colors()

    @property
    def top(self):
        y = None
        
        for index in range(int(len(self._vertices.vertices) / 2)):
            if y is None or self._vertices.vertices[index * 2 + 1] > y:
                y = self._vertices.vertices[index * 2 + 1]
                
        return y
    
    @top.setter
    def top(self, value):
        self.y += value - self.top
    
    @property
    def bottom(self):
        y = None
        
        for index in range(int(len(self._vertices.vertices) / 2)):
            if y is None or self._vertices.vertices[index * 2 + 1] < y:
                y = self._vertices.vertices[index * 2 + 1]
                
        return y
    
    @bottom.setter
    def bottom(self, value):
        self.y += value - self.bottom
    
    @property
    def left(self):
        x = None
        
        for index in range(int(len(self._vertices.vertices) / 2)):
            if x is None or self._vertices.vertices[index * 2] < x:
                x = self._vertices.vertices[index * 2]
                
        return x
    
    @left.setter
    def left(self, value):
        self.x += value - self.left
    
    @property
    def right(self):
        x = None
        
        for index in range(int(len(self._vertices.vertices) / 2)):
            if x is None or self._vertices.vertices[index * 2] > x:
                x = self._vertices.vertices[index * 2]
                
        return x
    
    @right.setter
    def right(self, value):
        self.x += value - self.right
    
class Image(Base):
    def __init__(self, image = MAN, x = 0, y = 0, rotation = 0):
        self._texture = pyglet.image.load(image).get_texture()
        super().__init__(x, y, 0, (1, 1, 1), 1, 4, pyglet.gl.GL_QUADS, ('t3f', self._texture.tex_coords))

        self._image = image
        self._width = self._texture.width
        self._height = self._texture.height

        self._make_vertices()
        self._make_colors()

    def _make_vertices(self):
        x = self.x - self.anchor_x
        y = self.y - self.anchor_y
        
        x1 = self.scale_x * -self.width / 2 + x
        y1 = self.scale_y * -self.height / 2 + y
        x2 = self.scale_x * self.width / 2 + x
        y2 = self.scale_y * -self.height / 2 + y        
        x3 = self.scale_x * -self.width / 2 + x
        y3 = self.scale_y * self.height / 2 + y
        x4 = self.scale_x * self.width / 2 + x
        y4 = self.scale_y * self.height / 2 + y
        
        x1, y1 = rotate_point(x1, y1, self.x, self.y, self.rotation)
        x2, y2 = rotate_point(x2, y2, self.x, self.y, self.rotation)
        x3, y3 = rotate_point(x3, y3, self.x, self.y, self.rotation)
        x4, y4 = rotate_point(x4, y4, self.x, self.y, self.rotation)
        
        self._vertices.vertices[:] = (x1, y1, x2, y2, x4, y4, x3, y3)
        
    def draw(self):
        pyglet.gl.glEnable(self._texture.target)
        pyglet.gl.glBindTexture(self._texture.target, self._texture.id)
        self._bind()
        self._vertices.draw(pyglet.gl.GL_QUADS)
        self._unbind()
        pyglet.gl.glDisable(self._texture.target)
        
    @property
    def width(self):
        return self._width

    @width.setter
    def width(self, value):
        self._width = value
        self._make_vertices()
        
    @property
    def height(self):
        return self._height
        
    @height.setter
    def height(self, value):
        self._height = value
        self._make_vertices()
        
    @property
    def image(self):
        return self._image
        
    @image.setter
    def image(self, image):
        self._texture = pyglet.image.load(image).get_texture()
        vertices = self._vertices.vertices[:]
        colors = self._vertices.colors[:]
        self._vertices = pyglet.graphics.Batch().add(4, pyglet.gl.GL_QUADS, None, 'v2f', 'c4B', ('t3f', self._texture.tex_coords))
        self._vertices.vertices[:] = vertices
        self._vertices.colors[:] = colors
        
        self._image = image
        self.width = self._texture.width
        self.height = self._texture.height

class Circle(Base):
    def __init__(self, x = 0, y = 0, radius = 50, color = (1, 1, 1), opacity = 1):
        super().__init__(x, y, 0, color, opacity, radius + 2, pyglet.gl.GL_TRIANGLE_FAN)
        
        self._radius = radius
        
        self._make_vertices()
        self._make_colors()
        
    def _make_vertices(self):
        radians = math.pi * 2 / self.radius
        x, y = rotate_point(self.x - self.anchor_x, self.y - self.anchor_y, self.x, self.y, self.rotation)
        vertices = [x, y]
        
        for index in range(self.radius):
            vertices.extend(rotate_point(
                x + self.radius * self.scale_x * math.cos(index * radians),
                y + self.radius * self.scale_y * math.sin(index * radians),
                x, y, self.rotation))
            
        vertices.extend((vertices[2], vertices[3]))
        self._vertices.vertices[:] = vertices

    def draw(self):
        self._bind()
        self._vertices.draw(pyglet.gl.GL_TRIANGLE_FAN)
        self._unbind()
    
    @property
    def radius(self):
        return self._radius

    @radius.setter
    def radius(self, value):
        self._radius = value
        self._make_vertices()
    
class Line(Base):
    def __init__(self, x1 = 0, y1 = 0, x2 = 100, y2 = 100, width = 1, color = (1, 1, 1), opacity = 1):
        super().__init__((x1 + x2) / 2, (y1 + y2) / 2, 0, color, opacity, 4, pyglet.gl.GL_QUADS)
                
        self._position1 = (x1, y1)
        self._position2 = (x2, y2)
        self._width = width
        
        self._make_vertices()
        self._make_colors()
        
    def _make_vertices(self):
        x = self.x - (self.x1 + self.x2) / 2 - self.anchor_x
        y = self.y - (self.y1 + self.y2) / 2 - self.anchor_y
        
        x1 = self.scale_x * self.x1 + x
        y1 = self.scale_y * self.y1 + y
        x2 = self.scale_x * self.x2 + x
        y2 = self.scale_y * self.y2 + y
        
        x1, y1 = rotate_point(x1, y1, self.x, self.y, self.rotation)
        x2, y2 = rotate_point(x2, y2, self.x, self.y, self.rotation)
        
        length = math.hypot(x1 - x2, y1 - y2)
        width = self.width / 2
        
        nx = (y1 - y2) / length
        ny = -(x1 - x2) / length
        
        self._vertices.vertices[:] = (
            x1 + nx * width, y1 + ny * width,
            x1 - nx * width, y1 - ny * width,
            x2 - nx * width, y2 - ny * width,
            x2 + nx * width, y2 + ny * width)

    def draw(self):
        self._bind()
        self._vertices.draw(pyglet.gl.GL_QUADS)
        self._unbind()
        
    @property
    def x1(self):
        return self.position1[0]
    
    @x1.setter
    def x1(self, value):
        self.position1 = (value, self.position1[1])
    
    @property
    def y1(self):
        return self.position1[1]
    
    @y1.setter
    def y1(self, value):
        self.position1 = (self.position1[0], value)
    
    @property
    def position1(self):
        return self._position1
    
    @position1.setter
    def position1(self, value):
        self._position1 = value
        self._make_vertices()
    
    @property
    def x2(self):
        return self.position2[0]
    
    @x2.setter
    def x2(self, value):
        self.position2 = (value, self.position2[1])
    
    @property
    def y2(self):
        return self.position2[1]
    
    @y2.setter
    def y2(self, value):
        self.position2 = (self.position2[0], value)
    
    @property
    def position2(self):
        return self._position2
    
    @position2.setter
    def position2(self, value):
        self._position2 = value
        self._make_vertices()
    
    @property
    def width(self):
        return self._width
    
    @width.setter
    def width(self, value):
        self._width = value
        self._make_vertices()