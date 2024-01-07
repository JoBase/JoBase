import JoBase

window.title = "Physics Demo"

# create the physics engine
engine = Physics()
shapes = []

# create the ground
floor = Rectangle(0, -150, 600, 20)
floor.body = engine.body(STATIC)

message = Text("Left and right click to create shapes", font_size = 20, color = GRAY)
start = True

def create_circle():
    return Circle(diameter = random(10, 30))

def create_box():
    return Rectangle(width = random(10, 30), height = random(10, 30))

def create_star():
    return Shape((
        (-15, 0), (-5, 5),
        (0, 15), (5, 5),
        (15, 0), (5, -5),
        (0, -15), (-5, -5)
    ))

# function for creating a random shape
def create_shape():
    # make a random number between 0 and 2
    type = randint(0, 2)

    # 0 = circle, 1 = box, 2 = star
    shape = create_circle() if not type else create_box() if type == 1 else create_star()

    shape.color = random(), random(), random()
    shape.body = engine.body()
    shape.pos = cursor.pos

    shapes.append(shape)

# function for creating a heavy block
def create_wall():
    wall = Rectangle()

    wall.body = engine.body()
    wall.pos = cursor.pos

    # make it heavy
    wall.mass = 100

    shapes.append(wall)

def loop():
    global start

    engine.update()
    floor.draw()

    # draw the starting text
    if start:
        message.draw()

    # draw the shapes in reverse order (prevents flashes)
    for shape in reversed(shapes):
        shape.draw()

        if shape.top < camera.bottom:
            shapes.remove(shape)

    # left click and hold
    if cursor.left.hold:
        create_shape()
        start = False

    # right click
    elif cursor.right.press:
        create_wall()
        start = False

run()
