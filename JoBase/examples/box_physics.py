from JoBase import *

window.size = (1024, 576)
window.color = (0, 0, 0)

engine = Physics()
time = 0

floor = Rectangle(width = window.width - 300, height = 20)
floor.color = (1, 1, 1)
floor.type = STATIC
floor.bottom = window.bottom
engine.add(floor)

def loop():
    global time

    engine.update()
    time += 1

    if cursor.press:
        rect = Rectangle(width = 60, height = 60)
        rect.pos = cursor.pos
        rect.mass = 100
        rect.color = (1, 1, 1)
        engine.add(rect)

    if not time % 5:
        box = Rectangle(y = window.top + 100)
        box.size = (random(20, 30), random(20, 30))
        box.color = (random(0, 1), random(0, 1), random(0, 1))
        engine.add(box)

    for rect in engine:
        if rect.top < window.bottom:
            engine.remove(rect)

    for rect in engine:
        rect.draw()

run()