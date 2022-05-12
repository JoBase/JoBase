from JoBase import *

man = Image(MAN)
coin = Image(COIN)
enemy = Image(ENEMY)

text = Text("Score: 0", size = 30)
score = 0

window.caption = "Coin Collector"

def loop():
    global score

    man.draw()
    coin.draw()
    enemy.draw()
    text.draw()

    if key.up: man.y += 3
    elif key.down: man.y -= 3
    elif key.left: man.x -= 3
    elif key.right: man.x += 3

    enemy.look_at(coin)
    enemy.move_toward(coin, 1.5)

    if man.collides_with(coin):
        coin.x = random(window.left, window.right)
        coin.y = random(window.bottom, window.top)

        score += 1
        text.content = "Score: " + str(score)

    if enemy.collides_with(coin):
        coin.x = random(window.left, window.right)
        coin.y = random(window.bottom, window.top)

        score -= 1
        text.content = "Score: " + str(score)

    text.bottom = window.bottom
    text.left = window.left + 5

run()