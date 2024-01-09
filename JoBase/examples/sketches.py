import JoBase

class Draw(Line):
    PRECISION = 5

    def __init__(self):
        super().__init__(width = 5)

        self.list = []
        self.selected = 0
        self.text = Text("Click and drag to draw a line", color = DARK_GRAY, font_size = 20)

    def select(self, item):
        self.selected = item.point
        self.color = item.rect.color

    def draw(self):
        if self.text:
            self.text.draw()

        if cursor.press:
            self.list = [(cursor.x, cursor.y)]
            self.text = None

        if cursor.hold:
            self.points = self.list + [cursor.pos]
            super().draw()

            if cursor.move:
                x, y = cursor.pos - self.list[-1]
                dist = hypot(x, y)

                if dist > self.PRECISION:
                    for _ in range(int(dist / self.PRECISION)):
                        self.list.append([self.list[-1][0] + x / dist * self.PRECISION, self.list[-1][1] + y / dist * self.PRECISION])

        if cursor.release and len(self.list) > 1:
            line = Line(self.points, width = self.width, color = self.color)
            type = STATIC if not self.selected else DYNAMIC
            list = [e for e in ground if e.collide(line) and e.body.type == type]

            if len(list):
                list.append(line)

                for item in list:
                    item.body = list[0].body

            else: line.body = engine.body(type)
            ground.append(line)

class Button():
    def __init__(self, color, x):
        self.rect = Rectangle(width = 20, height = 20, color = color)
        self.circle = Circle(diameter = 20, color = color)
        self.point = x

    def draw(self):
        x = camera.left + self.point * 30 + 10
        y = camera.top - 10

        if cursor.press and cursor.collide(self.rect):
            line.select(self)

        self.rect.left = self.circle.left = x
        self.rect.top = self.circle.top = y
        self.circle.draw() if line.selected == self.point else self.rect.draw()

engine = Physics()
line = Draw()

joints = []
ground = []

black = Button(BLACK, 0)
red = Button(RED, 1)

def loop():
    engine.update()
    line.draw()

    for item in ground:
        item.draw()

    red.draw()
    black.draw()

run()
