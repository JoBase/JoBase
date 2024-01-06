from JoBase import *
import math

SPEED = 0
DAMAGE = 1
EXPLODE = 2
REGEN_B = 3
REGEN_H = 4
HEALTH = 5
ARTILLERY = 6

class Particle:
    def __init__(self, x, y, size, color, text = 'empty', growspeed = .1, dir = 0, speed = 0):
        self.x = x
        self.y = y
        self.size = size
        self.color = color
        self.text = False
        self.growspeed = growspeed
        self.speed = abs(speed)
        self.dir = dir
        if text != 'empty': self.text = text

        if len(self.color) < 4:
            self.color.append(1)

    def update(self):
        self.x += math.cos(self.dir) * self.speed
        self.y += math.sin(self.dir) * self.speed

        self.size += self.growspeed
        self.color[3] -= .1

        color = [self.color[0], self.color[1], self.color[2], self.color[3]]
        if color[3] > 1: color[3] = 1

        if self.text: fillText(self.text, self.x - cam.x, self.y - cam.y, self.size, color)
        else: fillRect(self.x, self.y, self.size, self.size, color)

        if self.color[3] < 0: particles.remove(self)

class Camera:
    def __init__(self):
        self.x = 0
        self.y = 0

        self.booms = []
    
    def shake(self, x_shake, y_shake, time):
        self.booms.append({'x': x_shake, 'y': y_shake, 'time': time})
    
    def update(self):
        # move camera
        dist_x = hero.x - self.x
        dist_y = hero.y - self.y
        self.x += dist_x / 10
        self.y += dist_y / 10

        limit = top + (window.height / 2 / scale)
        if self.y < limit: self.y = limit

        # shake camera
        for item in self.booms:
            self.x += random(-item['x'], item['x'])
            self.y += random(-item['y'], item['y'])
            item['time'] -= 1
            if item['time'] < 0: self.booms.remove(item)

        # set camera
        camera.x = self.x * scale
        camera.y = -self.y * scale

class Bullet:
    def __init__(self, x, y, states, force_x = 0, dir = 0): 
        self.x = x
        self.y = y
        self.force_x = 0
        if force_x > 0: self.force_x = force_x

        self.speed_x = 0
        self.speed_y = 0

        self.w = .31
        self.h = .29
        self.x -= self.w / 2
        self.frame = 0
        self.dir = dir
        self.attack = False
        self.lifespan = 100

        self.speed = states[0][1] # speed
        self.damage = states[1][1] # damage
        self.explode = states[2][1] # explode

        self.puff = 0
    
    def clr(self):
        arr = [1, 1, 1]
        if self.explode:
            if random(0, 2) > 1:
                s = random(.3, .5)
                arr = [s, s, s]
            else: arr = [random(.8, 1), random(.3, .6), 0]
        
        return arr

    def update(self):
        self.frame += 1
        self.lifespan -= 1
        self.puff -= abs(self.speed_x)

        if self.explode > 0 and self.lifespan < 0 and self.attack:
            angle = (math.pi * 2) / self.explode
            state = [
                ['SPEED', math.ceil(self.speed)],
                ['FORCE', math.ceil(self.damage)],
                ['EXPLODE', math.floor(self.explode / 2)]
            ]
            for i in range(math.ceil(self.explode)):
                hero.bullets.append(Bullet(self.x, self.y, state, 0, i * angle))
            particles.append(Particle(self.x, self.y, 1, [.8, .4, 0]))

        if (self.lifespan < 0 or
            self.x - cam.x > (window.width / 2 / scale) + self.w or
            self.x - cam.x < -(window.width / 2 / scale) - self.w or
            self.y - cam.y > (window.height / 2 / scale) + self.h or
            self.y - cam.y < -(window.height / 2 / scale) - self.h):
            hero.bullets.remove(self)
            return

        top = self.y - self.h / 2
        bot = self.y + self.h / 2
        left = self.x - self.w / 2
        right = self.x + self.w / 2

        hit = False
        for i in blobs:
            if (not hit and right > i.x - i.w / 2 and left < i.x + i.w / 2 and
                bot > i.y - i.h / 2 and top < i.y + i.h / 2):
                i.upg['health'][0] -= self.damage
                self.lifespan = -1
                if i.upg['health'][0] <= 0:
                    if not i.attack: self.attack = True
                    i.attack = True
                hit = True
        
        self.speed_x = self.force_x + math.cos(self.dir) * (self.speed / 20)
        self.speed_y = math.sin(self.dir) * (self.speed / 20)
        
        self.x += self.speed_x
        self.y += self.speed_y

        if self.puff < 0:
            arr = self.clr()
            particles.append(Particle(
                self.x, self.y + random(-self.w / 3, self.w / 3),
                .1, [arr[0], arr[1], arr[2], random(1, 2) - self.speed_x], 'empty',
                random(.01, .04), self.dir, self.speed_x / 2))

            self.puff = 1

        self.draw()
    
    def draw(self):
        color = [0, 0, 0]
        if self.explode: color[0] += math.sin(self.frame / 7)
        if self.damage: color[2] += self.damage / 10
        fillRect(self.x, self.y, self.w, self.h, color)

class Heli:
    def __init__(self, x, y):
        self.reset(x, y)
    
    def reset(self, x = 0, y = 0):
        self.x = x
        self.y = y
        self.w = 1
        self.h = 1.15
        self.speed_x = 0
        self.speed_y = 0
        self.damping_x = 40
        self.damping_y = 100
        self.momentum = .9
        self.gravity = .003
        self.land = False
        self.bullets = []
        self.points = 100
        self.ready_pad = False

        # title, current, increment, max, price, color
        self.upg = [
            # BULLET
            ['SPEED', 1, 1, 10, 1, 'blue'], #             0
            ['DAMAGE', 1, 1, 'inf', 500, 'blue'], #       1
            ['EXPLODE', 0, 1, 10, 80, 'blue'], #          2
            ['REGEN (bullet)', 1, 1, 30, 1, 'blue'], #    3
            # HELI
            ['REGEN (health)', 1, 8, 50, 7, 'green'], #   4
            ['HEALTH', 5, 15, 'inf', 30, 'green'], #      5
            ['ARTILLERY', 1, 1, 8, 280, 'green'], #       6
        ]

        self.regen = 0
        self.regen_max = 100
        self.life = self.upg[HEALTH][1]
        self.dead = 0
    
    def die(self):
        if not self.dead: cam.shake(1, 1, 30)
        self.dead += 1
        self.life = 0

        self.speed_x *= self.momentum
        self.speed_y += self.gravity
        self.x += self.speed_x
        self.y += self.speed_y

        if self.y > bot - self.h / 2:
            self.speed_y = 0
            self.y = bot - self.h / 2

        self.draw()
    
    def kill(self, force):
        force = force / 10
        cam.shake(force, force, 10)
        self.life -= force
    
    def update(self):
        global pad_area

        if self.life <= 0 or self.dead > 0:
            self.die()
            return

        if self.life < self.upg[HEALTH][1]: self.life += self.upg[REGEN_H][1] / 1000
        else: self.life = self.upg[HEALTH][1]

        # physics logic
        if not self.land:
            self.ready_pad = True

            if LEFT: self.speed_x -= 1 / self.damping_x
            if RIGHT: self.speed_x += 1 / self.damping_x
            if DOWN: self.speed_y += 1 / self.damping_y
        if UP: self.speed_y -= 1 / self.damping_y

        self.speed_x *= self.momentum
        self.speed_y += self.gravity
        self.x += self.speed_x
        self.y += self.speed_y

        if self.y < top + self.h / 2:
            self.speed_y = 0
            self.y = top + self.h / 2
        elif self.y > bot - self.h / 2:
            self.speed_y = 0
            self.land = True
            self.y = bot - self.h / 2
        else: self.land = False

        # pad
        if onPad(self.x):
            pad_area = True
            if self.y >= bot - self.h / 2 and self.ready_pad:
                self.ready_pad = False
                shop.open()
        if hero.points < 0: hero.points = 0
    
        # bullets
        for i in self.bullets: i.update()

        self.regen += self.upg[REGEN_B][1]
        if self.regen >= self.regen_max:
            cam.shake(.1, .1, 5)
            guns = self.upg[ARTILLERY][1]
            radius = 1
            increment = radius / guns
            for i in range(guns):
                angle = (i * increment) - (((guns - 1) * increment) / 2)
                angle += random(-.1, .1)
                self.bullets.append(Bullet(self.x + self.w / 2, self.y, self.upg, self.speed_x, angle))
            self.regen = 0

        self.draw()

    def draw(self):
        body_w = 1
        body_h = .9
        y = self.y + (self.h - body_h) / 2
        wind_w = .5
        wind_h = .6
        blade_w = .95
        blade_h = .1
        bar_w = .07
        bar_h = self.h - body_h

        # body
        fillRect(self.x, y, body_w, body_h, [0, .6, .3])
        # glass
        fillRect(self.x + self.w / 2 - wind_w / 2, y, wind_w, wind_h, [0, .5, .7])
        # blade
        fillRect(self.x, y - self.h / 2, bar_w, bar_h, [.1, .1, .1])
        # skid
        fillRect(self.x, self.y - self.h / 2 + blade_h / 2, blade_w * math.cos(self.speed_y * 60), blade_h, [.1, .1, .1])

class Blob:
    def __init__(self, x, y, level = 'none', speed_x = 0, speed_y = 0):
        self.x = x
        self.y = y
        self.w = 1
        self.h = 1

        self.index = x

        self.speed_x = speed_x
        self.speed_y = speed_y
        self.momentum = .99
        self.time = random(0, 10)
        self.move = 0
        self.move_time = 100
        self.dir = 0
        self.attack = False
        self.kill = False
        self.color = [.6, .1, .1]

        self.lev = math.floor((self.x / pad_timer * 2) * hash(self.x + self.y * self.y))
        if level != 'none': self.lev = level

        # current, increase, max, viciousness
        self.upg = {
            'health': [1, 1, 'inf', 0],
            'speed': [0, .5, 20, 1],
            'size': [1, .5, 1.5, 2],
            'spawn': [0, 1, 30, 3]
        }

        self.generate()
        
        size = self.upg['size'][0]
        self.w = size + hash(self.x * self.y) * .2
        self.h = size + hash(self.x * self.y) * .2

        self.spawn = self.upg['spawn'][0]
        self.spawn_ang = 0
        self.spawn_max = 0
        if self.spawn: self.spawn_max = 200 / self.spawn
        self.spawn_timer = random(0, self.spawn_max)
        self.spawn_force = .05

    def generate(self):
        def apply(array, level, priority):
            level -= array[3] # viciousness
            if level < 0: level = 0

            # if not the blob's priority randomise the effectiveness
            if not priority: level = level * hash(level + 10)

            array[0] += level * array[1] # level * increase

            if array[2] != 'inf': # max level
                if array[0] > array[2]: array[0] = array[2]
            return array

        # decide what the blob's best at based on the level
        priority = math.floor(hash(self.lev) * self.lev) % (len(self.upg) - 1) == self.lev # bool

        # generate blob abilities
        self.color = [hash(self.lev + 2), hash(self.lev), hash(self.lev + 1)]
        self.upg['health'] = apply(self.upg['health'], self.lev, priority)
        self.upg['speed'] = apply(self.upg['speed'], self.lev, priority)
        self.upg['size'] = apply(self.upg['size'], self.lev, priority)
        self.upg['spawn'] = apply(self.upg['spawn'], self.lev, priority)

    def update(self):
        if self.lev > 5 and self.spawn:
            self.spawn_timer -= 1
            if self.spawn_timer < 0:
                self.spawn_ang += 2
                blobs.insert(0, Blob(self.x, self.y, 0,
                    math.cos(self.spawn_ang) * self.spawn_force,
                    math.sin(self.spawn_ang) * self.spawn_force))
                self.spawn_timer = self.spawn_max

        if self.upg['health'][0] <= 0:
            value = math.ceil(self.lev * .8) + 1
            hero.points += value
            particles.append(Particle(self.x, self.y, self.w / 4, [0, 0, 0, 2], '+' + str(value)))
            particles.append(Particle(self.x, self.y, self.w / 2, self.color))

            self.kill = True

        # move
        self.move -= 1
        if self.move < 0:
            self.dir = random(0, 7)
            self.move = self.move_time

        _SPEED = self.upg['speed'][0] / 3000

        if _SPEED:
            move_x = hero.x - self.x
            move_y = hero.y - self.y
            hypot = math.hypot(move_x, move_y) / _SPEED
            move_x /= hypot
            move_y /= hypot

            self.speed_x += move_x + math.cos(self.dir) * _SPEED
            self.speed_y += move_y + math.sin(self.dir) * _SPEED

        if self.y > bot - self.h / 2:
            self.y = bot - self.h / 2
            self.speed_y *= -.5
        elif self.y < top + self.h / 2:
            self.y = top + self.h / 2
            self.speed_y *= -.5

        self.speed_x *= self.momentum
        self.speed_y *= self.momentum
        self.x += self.speed_x
        self.y += self.speed_y

        if (hero.x + hero.w / 2 > self.x - self.w / 2 and hero.x - hero.w / 2 < self.x + self.w / 2 and
            hero.y + hero.h / 2 > self.y - self.h / 2 and hero.y - hero.h / 2 < self.y + self.h / 2):
            hero.kill(self.upg['health'][0] * .65)
            self.upg['health'][0] -= .1

        self.draw()

    def draw(self):
        eyebrow = hash(self.lev * 5) > .5
        eyebrow_weight = .05 + hash(self.lev * 7) * .03
        eyebrow_bend = hash(1 + self.lev * 3) * .06

        pupil_size = .15 + hash(self.lev + 1) * .05
        pupil_look = .05 + hash(self.lev + 2) * .05
        eye_size = .3 + hash(self.lev + 3) * .05
        eye_apart = .2 + hash(self.lev + 4) * .1

        # body
        fillRect(self.x, self.y, self.w, self.h, self.color)
        # eyes
        eye1_x = self.x + self.w * eye_apart
        eye2_x = self.x - self.w * eye_apart
        fillRect(eye1_x, self.y, self.w * eye_size, self.h * eye_size, [1, .9, .9])
        fillRect(eye2_x, self.y, self.w * eye_size, self.h * eye_size, [1, .9, .9])
        # pupils
        look_x = hero.x - self.x
        look_y = hero.y - self.y
        hypot = math.hypot(look_x, look_y) + .01
        look_x /= hypot / pupil_look
        look_y /= hypot / pupil_look
        fillRect(eye1_x + look_x, self.y + look_y, self.w * pupil_size, self.h * pupil_size, [.1, 0, 0])
        fillRect(eye2_x + look_x, self.y + look_y, self.w * pupil_size, self.h * pupil_size, [.1, 0, 0])
        # eyebrow
        if eyebrow:
            h = self.h * eye_size
            _h = self.h * eyebrow_weight
            fillRect(eye1_x, self.y - h / 2, self.w * eye_size, _h, [.1, 0, 0], eyebrow_bend * 360)
            fillRect(eye2_x, self.y - h / 2, self.w * eye_size, _h, [.1, 0, 0], -eyebrow_bend * 360)

class Shop:
    def __init__(self):
        self.active = False
        self.current = 0
        self.smooth_current = 0
        self.price_increase = 1.9
        self.zoom = 0

    def open(self):
        self.active = True
    
    def close(self):
        self.zoom = 0
        self.current = 0
        self.smooth_current = 0
        hero.life = hero.upg[HEALTH][1]
        self.active = False
    
    def drawList(self, list):
        idx = 0
        for i in list:
            if i[5] == 'blue': color = [.3, .4, .5]
            elif i[5] == 'green': color = [.3, .5, .4]

            if self.current == idx: color = [.1, .6, .1]

            cost = str(math.ceil(i[4])) + 'p'
            if i[3] != 'inf':
                if i[1] >= i[3]: cost = 'MAX'

            self.drawBox(idx, color, i[0] + ': ' + str(math.floor(i[1])), cost, '')

            idx += 1
    
    def drawBox(self, y, color, left, right, center):
        w = window.width / scale * .7
        h = 2
        gap = .5
        y = (y - self.smooth_current) * (h + gap)

        # box
        realFillRect(.1, y + .1, w, h, [0, 0, 0, .3])
        realFillRect(0, y, w, h, color)

        # text
        if left != '': fillText(left, -w / 2 + .3, y, 1, [1, 1, 1], 'left')
        if right != '': fillText(right, w / 2 - 1.5, y, 1, [1, 1, 1], 'right')
        if center != '': fillText(center, 0, y, 1, [1, 1, 1])

        # arrow
        res = 4
        width = .8 / res
        for i in range(res): realFillRect(w / 2 - 1 + i * width, y, width, 1 - (i / res), [.9, .9, .9])

    def update(self):
        if self.zoom < 2: self.zoom += .05
        else: self.zoom = 2

        self.smooth_current += (self.current - self.smooth_current) / 8

        # window
        box_w = window.width / scale
        box_h = window.height / scale
        realFillRect(0, 0, box_w * self.zoom, box_h * self.zoom, [.2, .2, .2])

        if self.zoom > 1:
            # points
            self.drawBox(-1, [.3, .3, .3], '', '', 'POINTS: ' + str(hero.points))

            # options
            self.drawList(hero.upg)

            # close button
            i = len(hero.upg)
            color = [.6, .2, .1]
            if self.current == i: color = [.9, 0, 0]
            self.drawBox(i, color, '', '', 'CLOSE')

            # select option
            if key.press:
                if UP and self.current > 0: self.current -= 1
                if DOWN and self.current < len(hero.upg): self.current += 1

                if RIGHT:
                    # close button
                    if self.current >= len(hero.upg): self.close()
                    
                    # other buttons
                    else:
                        value = math.ceil(hero.upg[self.current][4])
                        if hero.points >= value:
                            max = hero.upg[self.current][3]
                            if max == 'inf' or hero.upg[self.current][1] < max:
                                hero.upg[self.current][1] += hero.upg[self.current][2]

                                # price change
                                hero.points -= value
                                if hero.points < 0: hero.points = 0
                                hero.upg[self.current][4] *= self.price_increase

        realFillRect(0, 0, box_w * self.zoom, box_h * self.zoom, [0, .3, .2, 2 - self.zoom])

def onPad(x, size = 'none'):
    if size == 'none': size = pad_width

    after_start = x > pad_width
    if x % pad_timer < size and after_start:
        return True

def hash(seed):
    seed *= 15485863
    return (seed * seed * seed % 2038074743) / 2038074743

def fillRect(x, y, width, height, color, angle = 0):
    RECT.color = [1, 1, 1, 1]
    RECT.angle = 0

    RECT.x = x * scale
    RECT.y = -y * scale
    RECT.width = width * scale
    RECT.height = height * scale
    RECT.color = color
    RECT.angle = angle

    RECT.draw()

def realFillRect(x, y, width, height, color):
    RECT.color = [1, 1, 1, 1]
    RECT.x = x * scale + camera.x
    RECT.y = -y * scale + camera.y
    RECT.width = width * scale
    RECT.height = height * scale
    RECT.color = color

    RECT.draw()

def fillText(text, x, y, size, color, ahr = ''):
    TEXT.color = [1, 1, 1, 1]
    TEXT.x = x * scale + camera.x
    TEXT.y = -y * scale + camera.y
    TEXT.font_size = size * scale
    TEXT.color = color
    TEXT.content = text

    if ahr == 'left': TEXT.x += TEXT.width / 2
    elif ahr == 'right': TEXT.x -= TEXT.width / 2

    TEXT.draw()

def distance(pos):
    return math.ceil(pos / 3)

def makeBlob(index):
    dist = (bot - 1) - (top + 1)
    cam_x = cam.x + (window.width / scale) / 1.5
    x = (cam_x % pad_timer) + (cam_x / 4000)
    frequency = .1 + x / 1000
    if index <= 5: frequency = 0

    if hash(index * index) < frequency:
        blobs.append(Blob(index, top + 1 + hash(index) * dist))

def drawBars():
    global start

    width = window.width / scale
    height = window.height / scale

    if box['dir'] == 1:
        box['close'] += .08
        if box['close'] > 1:
            hero.bullets = []
            particles.append(Particle(cam.x, cam.y, .9, [0,0,0,15], 'SQUIRT THE BLOBS!', .005))
            cam.shake(1, 1, 10)

            box['dir'] = -1
            start = False

    elif box['dir'] == -1:
        box['close'] -= .04
        if box['close'] < 0: box['close'] = 0
    
    bar = box['close'] * width / 2
    rim = 2

    realFillRect(-width / 2 + bar / 2, 0, bar, height, [.15, .15, .15])
    realFillRect(width / 2 - bar / 2, 0, bar, height, [.15, .15, .15])
    realFillRect(-width / 2 + bar - rim / 2, 0, rim, height, [.25, .25, .25])
    realFillRect(width / 2 - bar + rim / 2, 0, rim, height, [.25, .25, .25])

def loop():
    global scale, time, blobs, screen_left, screen_right, pad_area, start, website, web_zoom, web_alpha, webpage_alpha, UP, DOWN, LEFT, RIGHT
    scale = (window.width + window.height) / 40
    if scale == 0: scale = .1
    width = window.width / scale
    height = window.height / scale
    time += .1

    if key.up or key.w or key.z: UP = True
    else: UP = False
    if key.down or key.s: DOWN = True
    else: DOWN = False
    if key.left or key.a or key.q: LEFT = True
    else: LEFT = False
    if key.right or key.d: RIGHT = True
    else: RIGHT = False

    if website:
        realFillRect(0, 0, width, height, [0, 0, 0, webpage_alpha])
        
        alpha = (time / 5) - 5
        if alpha > 1:
            webpage_alpha -= .07
            if webpage_alpha < 0: website = False
        else:
            if time > 2:
                text = 'JOACHIMFORD.UK'
                fillText(text, 0, 0, 2, [0, 0, 0])
                text_w = TEXT.width / scale
                text_h = TEXT.height / scale

                if web_zoom > 1: web_zoom -= .04
                else:
                    web_zoom = 1
                    web_alpha -= .03
                    if web_alpha < -1: web_alpha = -1

                    fillText(text, 0, 0, 2, [.6, .6, .6])
                    realFillRect(0, text_h, text_w * (-web_alpha), .1, [.8, .8, 0])

                realFillRect(0, 0, text_w * web_zoom, text_h * web_zoom, [1, 1, 1, (2 - web_zoom) + web_alpha])
            realFillRect(0, 0, width, height, [0, 0, 0, alpha])

    elif start:
        s = .1
        realFillRect(0, 0, width, height, [.1, .7, .9])

        length = len(home_text)
        gap = 1.3
        for i in range(length):
            item = home_text[i]
            CHAR = 0
            SPEED = 1
            CURR = 2

            item[SPEED] += .1
            item[CURR] += item[SPEED] / 3
            if item[CURR] > 0:
                item[CURR] = 0
                item[SPEED] *= -.7

            X = (i * gap) - length * gap / 2 + gap
            width = gap - item[CURR]

            fillText(item[CHAR], X, item[CURR], 2, [math.sin(time), 0, 0])
            realFillRect(X, 1, width, .2, [0, 0, 0, .3 + item[CURR] / 10])

        alpha = time / 5 - 9
        if alpha > 1: alpha = 1
        if alpha > 0:
            fillText('Press a key', 0, 3 + math.sin(time * 2) * .1, 1, [0, 0, 0, alpha])
            if key.press: box['dir'] = 1

        hero.speed_y = (s / 2) + (time / 50)
        hero.y = -3 - s + random(-s, s)
        hero.x = random(-s, s)
        if alpha > 0 and s and math.floor(time * 10) % 10 == 0:
            hero.bullets.append(Bullet(hero.x, hero.y, [[0,3], [0,1], [0,1]], .2))

        for i in particles: i.update()
        for i in hero.bullets: i.update()
        hero.draw()

        drawBars()

    else:
        if shop.zoom < 1:
            # PARTICLES
            for i in particles: i.update()

            # BLOB AND PAD LOGIC
            left = math.floor(cam.x - width / 1.5)
            right = math.floor(cam.x + width / 1.5) + 2

            # update & remove
            for item in blobs: item.update()
            for item in blobs:
                if item.x < left or item.x > right or item.kill: blobs.remove(item)

            # placement
            for i in range(left, right):
                if onPad(i, 1):
                    color = [.1, 1, .2, .3]
                    if pad_area: color = [.1, .8, .6, .5]
                    fillRect(i + pad_width / 2, cam.y, pad_width, height, color)
                    fillRect(i + pad_width / 2, bot - .15, pad_width, .3, [.1, .1, .1])

                    if pad_area:
                        # arrow
                        bar_h = .6
                        bar_y = -2
                        sin_y = math.sin(time) * .4
                        res = 4
                        seg_height = .6 / res
                        fillRect(i + pad_width / 2, bot + bar_y + sin_y, .5, bar_h, [.8, 0, 0])
                        for j in range(res):
                            fillRect(
                                i + pad_width / 2, bot + bar_y + bar_h / 2 + j * seg_height + sin_y,
                                1 - (j / res), seg_height, [.8, 0, 0])

                    pad_area = False
                else:
                    if i > screen_right:
                        screen_right = i
                        screen_left = left
                        makeBlob(i)
                    if i < screen_left:
                        screen_left = i
                        screen_right = right
                        makeBlob(i)

            hero.update()

            # SCENERY
            ground_height = 20
            seg_gap = .5

            for i in range(math.floor(left / seg_gap), math.floor(right / seg_gap)):
                _hash1 = hash(i * 2)
                _hash2 = hash(i * i)

                shade = _hash1 / 15
                x = i * seg_gap
                y = bot - shade

                fillRect(x, y + ground_height / 2, seg_gap, ground_height, [.4 + shade, .3 + shade, .2 + shade])
                if _hash1 < .7:
                    grass = _hash2 / 2
                    fillRect(x, y - grass / 2, .15, grass, [.2, .6, 0])
                
                if _hash1 < .05:
                    tree = 2 + _hash2
                    bush = 1.6
                    fillRect(x, y - tree / 2, .2, tree, [.4, .26, .1])
                    fillRect(x, y - tree, bush, bush, [.2, .5, 0])

                if _hash2 < .05:
                    cloud = 2 + _hash1
                    cloud_y = 5 + _hash1 * 8
                    fillRect(x, bot - cloud_y, cloud * 1.5, cloud, [1, 1, 1, .2])

            # BARS
            bar_size = 6
            pad = .15
            main_bar = bar_size - pad * 2
            x = -width / 2 + bar_size / 2 + pad
            y = -height / 2

            # health
            _width = hero.life * (main_bar / hero.upg[HEALTH][1])
            realFillRect(x, y + .7, bar_size, 1, [.2 ,.2, .2])
            realFillRect(x - bar_size / 2 + _width / 2 + pad, y + .7, _width, .7, [.1, .4, .1])
            # regen
            _width = hero.regen * (main_bar / hero.regen_max)
            realFillRect(x, y + 1.8, bar_size, 1, [.2 ,.2, .2])
            realFillRect(x - bar_size / 2 + _width / 2 + pad, y + 1.8, _width, .7, [.2, .3, .6])
            # points
            realFillRect(x, y + 2.9, bar_size, 1, [.2 ,.2, .2])
            fillText(str(hero.points) + 'p', -width / 2 + pad * 2, y + 2.9, .8, [.9, .4, 0], 'left')
            # distance
            realFillRect(x, y + 4, bar_size, 1, [.2 ,.2, .2])
            fillText(str(distance(hero.x)) + 'm', -width / 2 + pad * 2, y + 4, .8, [0, .9, .4], 'left')

            if hero.dead > 0:
                over_timer = 20
                dist_timer = 200
                again_timer = 300

                realFillRect(.1, .1, 15, 8, [0, 0, 0, .3])
                realFillRect(0, 0, 15, 8, [.34, .34, .34, hero.dead / 20])
                if hero.dead > over_timer:
                    dead = hero.dead - over_timer
                    fillText('GAME OVER', 0, 0, 2, [0, 0, 0, dead / 20])
                if hero.dead > dist_timer:
                    dead = hero.dead - dist_timer
                    alpha = dead / 100
                    if alpha > 1: alpha = 1

                    fillText(
                        'You travelled ' + str(distance(hero.x)) + 'm!',
                        0, 2 + math.cos(dead / 5) * .3, 1,
                        [0, 1, 1, alpha])

                if hero.dead > again_timer:
                    dead = hero.dead - again_timer
                    alpha = dead / 10
                    if alpha > 1: alpha = 1

                    fillText(
                        'Press a key to try again',
                        0, -2, 1, [.5, .1, 0, alpha])
                
                    if key.press:
                        hero.reset()
                        screen_left = 0
                        screen_right = 0

            drawBars()
            cam.update()
        
        if shop.active: shop.update()

hero = Heli(0, 0)
cam = Camera()
shop = Shop()
RECT = Rectangle(0, 0, 1, 1)
TEXT = Text()
home_text = [['S'],['Q'],['U'],['I'],['R'],['T'],['C'],['O'],['P'],['T'],['E'],['R']]
for i in range(len(home_text)):
    item = home_text[i]
    item.append(i) # speed
    item.append(-5) # curr

top = -10
bot = 5
scale = 1
time = 0

screen_left = 0
screen_right = 0
blobs = []
particles = []

pad_timer = 300
pad_width = 6
pad_area = False

website = True
web_zoom = 2 # box that displays the text
web_alpha = 0 # alpha of text
webpage_alpha = 1 # alpha of page

start = True

UP = False
DOWN = False
LEFT = False
RIGHT = False

box = {
    'dir': 0,
    'close': 0
}

window.color = LIGHT_BLUE
run()
