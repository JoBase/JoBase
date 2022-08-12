from JoBase import *

player = Rectangle(width = 150, height = 30, color = WHITE)
text = Text(color = LIGHT_BLUE, font_size = 20)
ball = Circle(diameter = 30, color = ORANGE)

score = 0
best = 0

window.color = DARK_GRAY
window.caption = "Tennis"

def update():
    text.content = "Score: " + str(score) + ", Best: " + str(best)

def reset():
    ball.pos.y = 200
    ball.speed.x = -2 if randint(0, 1) else 2
    ball.speed.y = 0

    update()

def loop():
    global score, best

    if ball.top < camera.bottom:
        if score > best:
            best = score

        score = 0
        reset()

    if ball.collides_with(player):
        ball.speed.y = abs(ball.speed.y)
        ball.speed.x *= 1.1

        score += 1
        update()

    if ball.right > camera.right or ball.left < camera.left:
        ball.speed.x *= -1

    if key.left: player.x -= 5
    if key.right: player.x += 5

    player.bottom = camera.bottom
    text.left = camera.left + 10
    text.top = camera.top - 10

    ball.speed.y -= 0.1
    ball.pos += ball.speed

    player.draw()
    ball.draw()
    text.draw()

reset()
run()