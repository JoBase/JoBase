from JoBase import *

top = Rectangle(height = 500)
bottom = Rectangle(height = 500)

player_1 = Rectangle(width = 30, height = 150, color = WHITE)
player_2 = Rectangle(width = 30, height = 150, color = WHITE)

text = Text(color = WHITE)
ball = Circle(diameter = 30, color = ORANGE)
blob = Rectangle(width = 15, height = 5, color = BLACK)
engine = Physics(0, 0)

score_1 = 0
score_2 = 0

window.color = DARK_GRAY
window.caption = "Tennis"
window.size = 1024, 576

player_1.type = player_2.type = STATIC
player_1.elasticity = player_2.elasticity = 1.2

top.type = bottom.type = STATIC
top.elasticity = bottom.elasticity = 1
top.friction = bottom.friction = 0

ball.elasticity = 1
ball.friction = 1

engine.add(top)
engine.add(bottom)
engine.add(player_1)
engine.add(player_2)
engine.add(ball)

def reset():
    text.content = str(score_1) + " - " + str(score_2)
    ball.speed.x = -300 if randint(0, 1) else 300
    ball.speed.y = random(-100, 100)
    ball.pos = 0

def loop():
    global score_1, score_2

    if window.resize:
        top.width = bottom.width = window.width
        top.top = window.bottom
        bottom.bottom = window.top

        text.top = window.top
        player_1.left = window.left
        player_2.right = window.right

    if key.w: player_1.speed.y = 400
    elif key.s: player_1.speed.y = -400
    else: player_1.speed.y = 0

    if key.up: player_2.speed.y = 400
    elif key.down: player_2.speed.y = -400
    else: player_2.speed.y = 0

    engine.update()
    blob.angle = ball.angle
    blob.pos = ball.pos

    if ball.left > window.right:
        score_1 += 1
        reset()

    elif ball.right < window.left:
        score_2 += 1
        reset()

    player_1.draw()
    player_2.draw()
    ball.draw()
    blob.draw()
    text.draw()

reset()
run()