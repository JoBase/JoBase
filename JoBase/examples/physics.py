from JoBase import *

window.caption = "Physics"
window.size = 1024, 576

engine = Physics()
time = 0

floor = Rectangle(width = window.width - 300, height = 20, color = GRAY)
floor.type = STATIC
floor.bottom = camera.bottom
engine.add(floor)

def loop():
    global time

    engine.update()
    time += 1

    if cursor.press:
        box = Rectangle()
        box.size = 60
        box.pos = cursor.pos
        box.mass = 100
        box.color = BLACK

        engine.add(box)

    if not time % 5:
        thing = None

        if randint(0, 1):
            thing = Rectangle(
                width = random(20, 30),
                height = random(20, 30))

        else: thing = Circle(diameter = random(20, 30))
        
        thing.y = camera.top + 100
        thing.color = random(0, 1), random(0, 1), random(0, 1)
        engine.add(thing)

    for thing in engine:
        if thing.top < camera.bottom:
            engine.remove(thing)

    for thing in engine:
        thing.draw()

run()