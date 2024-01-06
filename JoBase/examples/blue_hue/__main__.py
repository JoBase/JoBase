# Blue Hue Copyright 2022 joachimford.uk

from JoBase import *
import math

# block types
BRICK = 'x'
COIN = 'o'
BASE = '.'
CRUMBLE = '#'
DARKNESS = '?'
BRIDGE = '='

KEY_R = 'r'
KEY_B = 'b'
KEY_P = 'p'
LOCK_R = 'R'
LOCK_B = 'B'
LOCK_P = 'P'

BOUNCE_STRONG = '_'
BOUNCE_WEAK = '-'
SPIKE_UP = '^'
SPIKE_DOWN = 'v'
SPIKE_LEFT = '<'
SPIKE_RIGHT = '>'
AIR = ' '
START = '&'
END = '|'

spikes = 4
spike_width = 1 / spikes / 2
spike_height = .3

crumble_off_time = 100
crumble_on_time = 20

# make your own levels!
LEVELS = [
    [
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                                                      x',
    'x                                     x                                x',
    'x                        x           xx                                x',
    'x&         o  o  o  o   xxx        xxxxxxxx                        o  |x',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx^xxxxx^xxxxx^xxxxxxxxxxx',
    'x x x     x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x xx',
    'x x x     x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x xx',
    'x x x     x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x xx',
    'x x x     x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x xx',
    'x x x     x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x xx',
    'x x x     x x x x x x x x x x x x x x x x x x x x x x x x x x x x x x xx',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
    ],[
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'x............................................................................x',
    'x............................................................................x',
    'x............................................................................x',
    'x.....xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.....x',
    'x.....x                                                                x.....x',
    'x.....x                                                                x.....x',
    'x.....x                                                                x.....x',
    'x.....x                                                                x.....x',
    'x.....x                                                                x.....x',
    'x.....x&  o  o  o  o                   o  o  o                         x.....x',
    'x.....xxxxxxxxxxxxxxxxx##############xxxxxxxxxxxx#xxxxxxxxxxxxxxxxxxxxxx.....x',
    'x.....................x              x..........x x..........................x',
    'x.....................x              x......xxxxx xxxx.......................x',
    'x.....xvvvvvvvvx......x              x......x        xxxxxvx.................x',
    'x.....>        <......x^^^^^^^^^^^^^^x......x              <.................x',
    'x.....>        <............................x              <.................x',
    'x.....>        <............................x       o      <.................x',
    'x.....>        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx###xxxxxxxxxx^x.................x',
    'x.....>    |                                    x............................x',
    'x.....x^^^^xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx............................x',
    'x............................................................................x',
    'x............................................................................x',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
    ],[
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'x..................................................................................x',
    'x..................................................................................x',
    'x....xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx......x',
    'x....x   ##                                o                                x......x',
    'x....x   ##                               xxxxxxxxx                         x......x',
    'x....x&  ##                              xx???????xx                        x......x',
    'x....xxxxxxxxxxxx     o   xx xx         xx??xxxxx??xx           x           x......x',
    'x...............x xxxxxxx            o xx???????xx????   o    ? x    |      x......x',
    'x...............x x.....x       xx^xxxxxxxxxxxx?xxxxxxxxxxxxxxxxxx^xxxxx??xxx......x',
    'x...............x x.....x       x.............x?x????????????????????????xxxx......x',
    'x...............x x.....x^^^^^^^x.............x?x????????????x?????????xxxxxx......x',
    'x...............x x...........................x?x???????????xxx??????xxxxxxxx......x',
    'x........xvvvvvvx xvvvvvvvvvvx................x?x??????????xxxxx????xxxxxxxxx......x',
    'x........>                   <..........xxxxxxx?xxxxxxxxx??xxxxxxxxxxxxxxxxxx......x',
    'x........>                   <..........>?????????????xx??xxxxxxxxxxxxxxxxxxx......x',
    'x........x^^^^^^^^^^^^^^^^^^^x..........>?????????????x??xxxxxxxxxxxxxxxxxxxx......x',
    'x.......................................>???????????????xxxxxxxxxxxxxxxxxxxxx......x',
    'x.......................................xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx......x',
    'x..................................................................................x',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    ],[
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'x                                                                                               x',
    'x                     xoooo                                                                     x',
    'x                     xoooo                                                                     x',
    'x                     xoooo   o o o o o o o o o o o o o o                                       x',
    'x                     xxxxxxxxxx?xxxxxxxxxxxxxxxxxxxxxxxxx                                      x',
    'x                        xxxxxxx?xxxxxxxxxxxxxxxxxxxxxxxxx                                      x',
    'x                            xxx?xxxxxxxxxxxxxxxxxxxxxxxxx                                      x',
    'x               xooooo                                    _     o o o o o                       x',
    'x               xooooo                                    xxxxxxxxxx?xxxxxxxxxxxxxxxx           x',
    'x               xxxxxx                                    xxxxx            xxxxx                x',
    'x                                                         xxxxx o o o o o oxxxxx                x',
    'x|    o o o o o        _          x                       ?xx??o o o o o o xx???                x',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                       ????x o o o o o o???xx     _          x',
    'x                                                         xxxxxxxxxxxxxxxxxxxxxx x x x          x',
    'x                                                       -                                       x',
    'x                                                     - x                                       x',
    'x                          o o o o o                - x                                         x',
    'x                         o o o o o               - x                                           x',
    'x                         xxxxxxxxxx  - - - - - - x                                             x',
    'x                         x........x  x x x x x x                                               x',
    'x                         xxxxxxxxxx                                                            x',
    'x                                                                                               x',
    'x&                       _                                                                      x',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    ],[
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'xxxxxxxxxxxxxxxxx????????????????xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'xxxxxxxxxx????????xxxxxxxxxxxxxx??xxxxxxxxxxxx????????xxxxxxxxxxxxxxxxxxxo o o o  x',
    'x??????????xxxxxxxxxxxxxxxx????xx??????????????xxxxxx?????xxxxxxxxxxxxxxx?xxxxxx| x',
    'x?xxxxxxxxxxxxxxxxxxxxxx????xx?xxxxxxxxxxxxxxxxxx???xxxx??oo?????xxxxxxxx?x....xx x',
    'xo o o o?????????????????xxxxx?xxxxxxxxxxxxbRoo?????xxx??xooxxxx??xxxxxxx?x.....x x',
    'xxxxo o xxxxxxxxxxxxxxxxxxxxxx?xxxxxxxxxxxxxxopxxx??xx??xxxxxxxxx??xxxxxx?x.....x x',
    'xxxxxo xxxxxxxxxxxxxxxxxxxxxxx?xxxxxxxxxxxxx??xxx??xx??xxxxxxxxxxx??xxxxx?x.....x x',
    'xxx o xxxxooxxxxxxxxxxxxxxxxxx?xxxxooxxxxxx??xxx??xxx?xxxxxxxxxooxx???????x.....x_x',
    'xo o xxxxxro?xxxxxxxxxxxxxxxxx?xxxxoo???????xxx??xxxx?xxxxxxxx?oo?xxxxxxxxx.....x x',
    'x o xxxxxxxx????xxxxxxxxxxxxxx?xxxx?xxxxxxxxxx??xxxxx?xxxxxxx??xx??xxxxxxxx.....x x',
    'xoxxxo oxxxxxxx??xxxx??????????????????xxxxxx??xxxxxx?????????xxxx????xxxxx.....x x',
    'x x o x o o oxxx???x????????????????????xxx???xxxxxx????xxxxxxxxxxxx??xxxxx.....x x',
    'xoxoxxxxxxxx xxxxx?P?????????????????????????xxxxxxx??xxxooxxxxxxx???xxxxxx.....x_x',
    'x_o xxxxxxo -xxooxxxxx?????????????????xxxxx???xxxxxx????oo??xxxxx??xxxxxxx.....x x',
    'xxxxxxxx o xxxxro??xxxxxxxxxxxxxxxxxxxxxxxxxx??xxxxxxxxxxxxx??xxxxx???ooxxx.....x x',
    'xxxxxxx oxxxxxxxx???????????????????????????B?xxxxxxxxxxxxxxx??xxxxxxxooxxx.....x x',
    'xxxxxx oxxoo??????xRxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?xxxxxxxxxxxxxxxxxx x',
    'x&o o oxxxooxxxxxxx????????????????????????????????????????????????????????#     _x',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
    ],[
    'xvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'x o o o o o o o o o o o o o o o o o o o o o o o o      B                                                                               x',
    'x #############xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx             x###x                                                             x',
    'xo o o o o o o xoooboooooooooooo              o        xxxxxxxxxxxxxxx   xxxxxxxxxxxxx=                                                x',
    'x############# xoxxxxxxxxxxxxxxx=            xxx  o                 x     x          x=                                                x',
    'x o o o o o o oxox.............x=            x.x xxx  o             xooooox          x=                                                x',
    'x #############xox.............x= o o o o o  x.x-x.x-xxx            #ooooox          x=                                                x',
    'xo o o o o o o xox.............xxxxxxxxxxxxxxx.xxx.xxx.xxxxxxxxxxxxxxxoooxx  xxxxxx  xo o o                                            x',
    'x############# xox..................................................xxxxxx  xxo o o oxxxxxx x                                          x',
    'x o o o o o o oxox.............xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      xxo o o o x o o oxx                                         x',
    'x #############xox.............x                                          xxo o o o o o o o x                                          x',
    'xo o o o o o r # x.............x                              o o oxxxxxxxx  vvvvvvvvvvvvvvvx -                                        x',
    'xxxxxxxxxxxxxxxx xxxxxxxxxxxxxxx                             ======x oooo x                 <xx=====xxxxxxx                            x',
    'x              x                                        o o o o o  x oooo ?  o ? o ?  o ? o  o   o  o  o  x                            x',
    'x              xxxxxxxxxxxxxxxxo o o o o oxxxxx###xxxxxxxxxxxxxxxRxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                            x',
    'x                             xxxxxx=xxxxxx   x   x             xox                                                                    x',
    'x                                   o         x^^^x             xox                                                                    x',
    'x                                   o                           xox                                                                    x',
    'x                                   o         xxxxxxxxxxxxxxxxxxxoxx                                                                   x',
    'x                                   o         xoooooooooooooooooooox                                                                   x',
    'x                                   o         xoxxxxxxxxxxxxxxxxxxxx                                                                   x',
    'x                                   o         xooooooooooopoooooooox                                                                   x',
    'x                                   o         xxxxxxxxxxxxxxxxxxxxox                                                                   x',
    'x                                   o                            xox                                                                   x',
    'x                                   o                             o                                                                    x',
    'x                                   o                             o                                                                    x',
    'x                                   o                             o                                                                    x',
    'x                                   o                             o                                                                    x',
    'x                                  x=x                            o                                               xxxxxx               x',
    'x                                 xx xx                                                                           .....x               x',
    'x=========================xxxxxxxxx   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.x               x',
    'x                         x  o o o o o o o o o o o o o o o o o o o o o ??? o o o o o o o o???o o o o o o o o o o o o x.x               x',
    'x                         x o o o o o o o o o o o o o o o o o o o o o o???o o r o o o o o ??? o o o o o o o o o o o ox.x               x',
    'x                         x xx=========================================xxx================xxx========================x.x               x',
    'x                         x x                                           x                  x                         x.x               x',
    'x                         x?x                                           x                  x                         x.x               x',
    'x                                                                       x                  x                         x.x               x',
    'x        o o o o o                                                      x                  x                         x.x               x',
    'x                                        xxxxxxxxx                      x                  x                         x.x               x',
    'x        x=======x                       x.......x=xx                   x                  x                         x.x               x',
    'x&      xx  p    xx                     -x.......xbox                   ? b                x                         ..x               x',
    'xxxxxxxxx         xxxxxxxxxRxxxxxxxx=xxxxxxxxxxxxxx=xxxxxxxxxxxxxx xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx#xxxxxx=xxxxxxxxxxxxxxxxxxxx?x',
    'x                                  x x>  o o o      x o o o       Bxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx#xxxxxx xxxxxxxxxxxxxxxxxxxx?x',
    'x                   =xxxxxxxxxxxxxxx x>   xxxx      xxx#####xxxxxxR  o o o o o o o o o o o o o o o o            x xxxxxxxxxxxxxxxxxxxx?x',
    'x                   =x             x-xxxxo r x-            xxo  o Pxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx o        x xxxxxxxxxxxxxxxxxxxx?x',
    'x ooo     x===x     =x             x_xxxxxxxxxxxxxx=xxxxxxxxooxxxxB        rxxxxxxxxxxxxxxxxxxxxxxxxxxx o       x-xxxxxxxxxxxxxxxxxxxx?x',
    'x ooo    xx _ xx    =x             x_    x........x-   o o  oxx   Rxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx o o o   _xxxxxxxxxxxxxxxxxxxx?x',
    'x ooo   xxx _ ###      o o o o o   xxxx  xx.......x_   xxxxxxxx#xxPxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?x',
    'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  xxxxxxxxxxxxxxxxxxxxx x  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?x',
    'x                                      xx o o o o o o o o o o o x                                                                      x',
    'x                                       xxxxxxxxxxxxxxxxxxxxxxxxx                                                                      x',
    'x                                                                                                                                      x',
    'x                                                             o   o   o                                                                x',
    'x                                                            xxx xxx xxx                                                               x',
    'x                                                            x.x x.x x.x                                                               x',
    'x                                                            xxx xxx xxx                                                               x',
    'x                                                           o                                                                          x',
    'x                                                           _   o   o   o                                                              x',
    'x                                                          xxx xxx xxx xxx                                                             x',
    'x                                                          x.x x.x x.x x.x                                                             x',
    'x                                                          xxx xxx xxx xxx                                                             x',
    'x                                                         o                                                                            x',
    'x                                                         _   o   o   o   o                                                            x',
    'x                                                        xxx xxx xxx xxx xxx                                                           x',
    'x                                                        x.x x.x x.x x.x x.x                                                           x',
    'x                                                        xxx xxx xxx xxx xxx                                                           x',
    'x                                                       o                                                                              x',
    'x                                                       _   o   o   o   o   o                                                          x',
    'x                                                      xxx xxx xxx xxx xxx xxx                                                         x',
    'x                                                      x.x x.x x.x x.x x.x x.x                                                         x',
    'x                                                      xxx xxx xxx xxx xxx xxx                                                         x',
    'x                                                     o                                                                                x',
    'x                                                     _   o   o   o   o   o   o                                                        x',
    'x                                                    xxx xxx xxx xxx xxx xxx xxx                                                       x',
    'x                                                    x.x x.x x.x x.x x.x x.x x.x                                                       x',
    'x                                                    xxx xxx xxx xxx xxx xxx xxx         xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    'x                                                   o                                    x                                             x',
    'x                                                   _   o   o   o   o   o   o   o        x                                             x',
    'x                                                  xxx xxx xxx xxx xxx xxx xxx xxx       x                                             x',
    'x                                                  x.x x.x x.x x.x x.x x.x x.x x.x       x                                             x',
    'x                                                  xxx xxx xxx xxx xxx xxx xxx xxx       x        x x xxx x x  x x xxx xxx             x',
    'x                                                 o                                      x        xxx x x x x  xxx  x  x x             x',
    'x                                                 _   o   o   o   r   o   o   o   o      x          x xxx xxx  xxx xxx x x             x',
    'x                                                xxx xxx xxx xxx xxx xxx xxx xxx xxx     R                                            |x',
    'x^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^x.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx',
    '                                                 xxx xxx xxx xxx xxx xxx xxx xxx xxx                                                    '
    ],[
    'xxxxxxxxxx',
    'xoooooooox',
    'xoooooooox',
    'x&ooooooox',
    'xxxxxxxxxx'
    ]
]

def levelConvert(level):
    world.list = []
    # set world width & height
    world.width = len(level[0])
    world.height = len(level)

    # lump the world together
    str = ''
    for i in level: str += i

    # convert
    for block_type in str: world.add(block_type)

class World:
    def __init__(self):
        self.width = 0
        self.height = 0
        self.list = []

        self.coins = 0

        self.level = {
            'index': -1,
            'change': False,
            'reset': False,
            'alpha': 0,
            'enter': False
        }

    def posToItem(self, x, y):
        index = posToIndex(x, y, self.width, self.height)
        if index == 'void': return {'key': AIR}
        return self.list[index]

    def generate(self, new = True):
        self.coins = 0
        hero.do_hurt = 0
        hero.speed_x = 0
        hero.speed_y = 0
        hero.keys_b = 0
        hero.keys_p = 0
        hero.keys_r = 0

        if new: self.level['index'] += 1
        levelConvert(LEVELS[self.level['index']])
    
    def add(self, key):
        # block types
        if key == START:
            pos = indexToPos(len(self.list), self.width)
            hero.x = pos[0]
            hero.y = pos[1]
            hero.speed_x = 0
            hero.speed_y = 0
        elif key == COIN: self.coins += 1

        # add block to the array
        block = {
            # all blocks
            'key': key,

            # crumble
            'solid_mode': True,
            'triggered': False,
            'time': crumble_on_time,

            # coin
            'collect': 0,

            # bounce
            'happy_width': .7,
            'width': .7,
            'speed': 0,
            'ready': 1,

            # lock
            'locked': True,

            # key
            'pickup': False
        }


        self.list.append(block)
    
    def update(self):
        self.draw()

        # change level
        if self.level['change']:
            realFillRect(0, 0, window.width, window.height, [1, 1, 0, self.level['alpha']])

            # reverse fading when opaque
            if self.level['alpha'] > 1:
                self.level['enter'] = True
                self.list = []
                self.generate()
                makeBackground()
                self.level['alpha'] = 1

            # stop fading when transparent
            elif self.level['alpha'] <= 0:
                self.level['enter'] = False
                self.level['change'] = False

            # fade
            if self.level['enter']: self.level['alpha'] -= .03
            else: self.level['alpha'] += .03
        
                # change level
        if self.level['reset']:
            realFillRect(0, 0, window.width, window.height, [.3, 0, 0, self.level['alpha']])

            # reverse fading when opaque
            if self.level['alpha'] > 1:
                self.level['enter'] = True
                self.list = []
                self.generate(False)
                makeBackground()
                self.level['alpha'] = 1

            # stop fading when transparent
            elif self.level['alpha'] <= 0:
                self.level['enter'] = False
                self.level['reset'] = False

            # fade
            if self.level['enter']: self.level['alpha'] -= .03
            else: self.level['alpha'] += .03
    
    def draw(self):
        h_half = window.width / 2 / scale
        v_half = window.height / 2 / scale

        TOP = math.floor(cam_y - v_half)
        BOTTOM = math.floor(cam_y + v_half) + 2
        LEFT = math.floor(cam_x - h_half)
        RIGHT = math.floor(cam_x + h_half) + 2

        for x in range(LEFT, RIGHT):
            for y in range(TOP, BOTTOM):
                item = world.posToItem(x, y)

                if item['key'] == BRICK: fillRect(x, y, 1, 1, [0, .1, .4])
                elif item['key'] == BASE: fillRect(x, y, 1, 1, [0, .3, .6])
            
                elif item['key'] == COIN:
                    if item['collect'] >= 1: item['collect'] += 1

                    size = .4 + item['collect'] / 15
                    fillRect(x, y, size, size, [1, 1, 0, 1 - item['collect'] / 5], time)

                elif item['key'] == CRUMBLE:
                    if item['triggered']: item['time'] -= 1
                    if item['time'] <= 0: item['solid_mode'] = False
                    if item['time'] < -crumble_off_time and not collide([x, y, 1, 1], [hero.x, hero.y, hero.width, hero.height]):
                        item['triggered'] = False
                        item['solid_mode'] = True
                        item['time'] = crumble_on_time

                    if item['solid_mode']:
                        shake = 0
                        if item['triggered']: shake = .05
                        fillRect(x + random(-shake, shake), y + random(-shake, shake), 1, 1, [.3, .2, .1])
                    else:
                        fillRect(x, y, 1, 1, [.3, .2, .1, 1 + item['time'] / 10])
            
                elif item['key'] == DARKNESS:
                    dis_x = x - hero.x
                    dis_y = y - hero.y
                    dis = (dis_x * dis_x + dis_y * dis_y)
                    alpha = dis / 15

                    X = x - round(dis_x)
                    Y = y - round(dis_y)

                    for i in range(X + Y):
                        if X > x: X -= 1
                        elif X < x: X += 1
                        if Y > y: Y -= 1
                        elif Y < y: Y += 1

                        block = world.posToItem(X, Y)['key']
                        if block != AIR and block != DARKNESS:
                            alpha = 1
                            break

                    fillRect(x, y, 1, 1, [0, .1, .4, alpha])
                
                elif item['key'] == BRIDGE:
                    fillRect(x, y - .35, 1, .2, [.5, .3, 0])
            
                elif item['key'] == KEY_R or item['key'] == KEY_B or item['key'] == KEY_P:
                    if item['pickup'] == False:
                        color = [1, 0, 0]
                        if item['key'] == KEY_B: color = [0, 1, 1]
                        elif item['key'] == KEY_P: color = [.6, 0, 1]

                        Y = y + math.sin(time / 7) * .05
                        fillRect(x, y, .6, .6, [1, 1, 1, .3], time / 4)

                        fillRect(x - .2, Y, .4, .4, [0, 0, 0])
                        fillRect(x - .2, Y, .2, .2, color)
                        fillRect(x + .2, Y - .03, .4, .15, [0, 0, 0])
                        fillRect(x + .35, Y + .1, .1, .14, [0, 0, 0])
                        fillRect(x + .2, Y + .1, .1, .14, [0, 0, 0])
                
                elif item['key'] == LOCK_R or item['key'] == LOCK_B or item['key'] == LOCK_P:
                    if item['locked']:
                        color = [.6, 0, 0]
                        if item['key'] == LOCK_B: color = [0, .6, .6]
                        elif item['key'] == LOCK_P: color = [.5, 0, 1]

                        fillRect(x, y, 1, 1, color)
                        fillRect(x, y, .9, .9, [.3, .3, .3])
                        fillRect(x, y - .1, .3, .3, [0, 0, 0])
                        fillRect(x, y + .15, .15, .3, [0, 0, 0])
                
                elif item['key'] == BOUNCE_STRONG or item['key'] == BOUNCE_WEAK:
                    item['ready'] += 1

                    if item['key'] == BOUNCE_STRONG: color = [.5, .1, 0]
                    elif item['key'] == BOUNCE_WEAK: color = [.1, .4, 0]
                    height = .7 + (item['happy_width'] - item['width'])
                    fillRect(x, y - height / 2 + .5, item['width'], height, color)

                    item['speed'] *= .9
                    item['speed'] += (item['happy_width'] - item['width']) / 5
                    item['width'] += item['speed']

                elif item['key'] == SPIKE_UP:
                    fillRect(x, y, 1, 1, [0, .2, .4])
                    for i in range(spikes):
                        fillRect(x - .5 + spike_width + i / spikes, y - .5 - spike_height / 2, spike_width, spike_height, [.2, .2, .2])
                
                elif item['key'] == SPIKE_DOWN:
                    fillRect(x, y, 1, 1, [0, .2, .4])
                    for i in range(spikes):
                        fillRect(x - .5 + spike_width + i / spikes, y + .5 + spike_height / 2, spike_width, spike_height, [.2, .2, .2])
                
                elif item['key'] == SPIKE_LEFT:
                    fillRect(x, y, 1, 1, [0, .2, .4])
                    for i in range(spikes):
                        fillRect(x - .5 - spike_height / 2, y - .5 + spike_width + i / spikes, spike_height, spike_width, [.2, .2, .2])
                
                elif item['key'] == SPIKE_RIGHT:
                    fillRect(x, y, 1, 1, [0, .2, .4])
                    for i in range(spikes):
                        fillRect(x + .5 + spike_height / 2, y - .5 + spike_width + i / spikes, spike_height, spike_width, [.2, .2, .2])

                elif item['key'] == END:
                    if self.coins <= 0:
                        fillRect(x, y, .7, .7, [1, .9, .8, .5], time * 6)
                        fillRect(x, y, .5, .5, [1, .9, .3], time * 8)
                    else:
                        fillRect(x, y, .7, .7, [1, 1, 1, .5], time / 3)
                        fillRect(x, y, .5, .5, [.4, .4, .4])

class Hero:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = .6
        self.height = .9
        self.color = [.2, .95, .3]
        self.dir = 0
        self.walk = 0

        # physics
        self.speed_x = 0
        self.speed_y = 0
        self.momentum = .7
        self.damping = 3
        self.gravity = .03
        self.in_air = True

        # max movement speed
        self.max_x = 1
        self.max_y = 1

        self.keys_r = 0
        self.keys_b = 0
        self.keys_p = 0

        self.do_hurt = 0

    def jump(self, force):
        self.speed_y = -force
        self.in_air = True

    def hurt(self):
        if world.level['change'] == False:
            self.do_hurt += 1
            self.speed_x = 0
            self.speed_y = 0
            self.dir = 0

            realFillRect(0, 0, window.width, window.height, [1, random(.1, .4), 0, .8])

            world.level['reset'] = True
    
    def control(self):
        self.dir = 0
        if UP and not self.in_air: self.jump(.3)
        if LEFT: self.dir = -.1
        if RIGHT: self.dir = .1
    
    def collide(self):
        list = [
            {'x': round(self.x - self.width / 2), 'y': round(self.y - self.height / 2)},
            {'x': round(self.x + self.width / 2), 'y': round(self.y - self.height / 2)},
            {'x': round(self.x - self.width / 2), 'y': round(self.y + self.height / 2)},
            {'x': round(self.x + self.width / 2), 'y': round(self.y + self.height / 2)}
        ]

        hurt = False
        bounce = False

        for obj in list:
            ME = [self.x, self.y, self.width, self.height]
            OBJ = [obj['x'], obj['y'], 1, 1]
            overlap = merge(ME, OBJ, self.gravity)

            item = world.posToItem(obj['x'], obj['y'])
            KEY = item['key']

            _SPIKE = KEY == SPIKE_UP or KEY == SPIKE_DOWN or KEY == SPIKE_LEFT or KEY == SPIKE_RIGHT
            _SPIKE_ON = (
                KEY == SPIKE_UP and overlap['y'] > 0 or
                KEY == SPIKE_DOWN and overlap['y'] < 0 or
                KEY == SPIKE_LEFT and overlap['x'] > 0 or
                KEY == SPIKE_RIGHT and overlap['x'] < 0
            )

            _CRUMBLE = KEY == CRUMBLE and item['solid_mode'] == True

            _LOCK_R = KEY == LOCK_R and item['locked'] == True
            _LOCK_B = KEY == LOCK_B and item['locked'] == True
            _LOCK_P = KEY == LOCK_P and item['locked'] == True

            _KEY = KEY == KEY_R or KEY == KEY_B or KEY == KEY_P

            if collide(ME, OBJ):
                # solid block collision
                if KEY == BRICK or _SPIKE or _CRUMBLE or _LOCK_R or _LOCK_B or _LOCK_P:
                    if overlap['x']:
                        self.x -= overlap['x']
                        self.speed_x = 0
                    else:
                        self.y -= overlap['y']
                        self.speed_y = 0
                        if overlap['y'] >= 0: self.in_air = False
                
                if KEY == BRIDGE and self.speed_y > 0:
                    goal = obj['y'] - .45 - self.height / 2
                    self.in_air = False
                    if self.y < obj['y'] - .45:
                        self.speed_y = 0
                        self.y += (goal - self.y) / 10
                    else: self.speed_y = -.2

                # coin collision
                elif KEY == COIN:
                    if item['collect'] == 0:
                        item['collect'] = 1
                        world.coins -= 1
                
                # key collision
                elif _KEY and item['pickup'] == False:
                    if KEY == KEY_R: self.keys_r += 1
                    elif KEY == KEY_B: self.keys_b += 1
                    elif KEY == KEY_P: self.keys_p += 1

                    item['pickup'] = True

                # finish line collision
                elif item['key'] == END and world.coins <= 0: world.level['change'] = True   

            if _SPIKE_ON: hurt = True
            elif _CRUMBLE: item['triggered'] = True 
            elif KEY == BOUNCE_STRONG or KEY == BOUNCE_WEAK: bounce = item

            elif _LOCK_R and self.keys_r > 0:
                self.keys_r -= 1
                item['locked'] = False
            elif _LOCK_B and self.keys_b > 0:
                self.keys_b -= 1
                item['locked'] = False
            elif _LOCK_P and self.keys_p > 0:
                self.keys_p -= 1
                item['locked'] = False

        if hurt: self.hurt()
        if bounce and bounce['ready'] > 0:
            if self.speed_y > 0 or key.up: self.speed_y = 0
            if bounce['key'] == BOUNCE_STRONG: self.speed_y -= .53
            else: self.speed_y -= .38

            self.in_air = True

            bounce['width'] = 1
            bounce['ready'] = -10

    def physics(self):
        # This function calculates physics and makes
        # sure the player doesn't glitch through walls.
        # If the player is moving really quickly, it
        # calculates between its previous and current
        # frames to check if it collided with anything.

        # CALCULATE HOW MANY ITERATIONS IT SHOULD DO
        prev = {'x': self.x - self.speed_x, 'y': self.y - self.speed_y}
        dis_x = prev['x'] - self.x
        dis_y = prev['y'] - self.y
        dis = math.sqrt(dis_x * dis_x + dis_y * dis_y)
        segments = math.ceil(dis * 4)
        if segments == 0: segments = 1
        
        for i in range(segments):
            # UPDATE POSITION OF PLAYER
            self.x += self.speed_x / segments
            self.y += self.speed_y / segments

            # CHECK FOR PLAYER COLLISION
            if not self.do_hurt: self.collide()
        
        # NORMAL PHYSICS CALCULATIONS
        self.speed_x *= self.momentum
        self.speed_x += self.dir / self.damping
        self.speed_y += self.gravity
        if not self.do_hurt: self.collide()

    def update(self):
        alive = not self.do_hurt

        if alive:
            self.walk += self.speed_x * 2
            self.physics()
            self.control()

        else: self.hurt()

        self.draw()

    def draw(self):
        x = self.x - math.sin(self.walk) * .3
        y = self.y + .2 + math.cos(self.walk * 2) * .05

        fillRect(self.x - math.sin(self.walk + math.pi) * .3, y, .25, .25, [0, .4, 0])
        fillRect(self.x, self.y, self.width, self.height, self.color)
        fillRect(x, y, .25, .25, [0, .5, 0])

        R = self.keys_r
        B = self.keys_b
        P = self.keys_p

        if R > 0 or B > 0 or P > 0:
            fillRect(x, y, .25, .25, [0, 0, 0])
            if R >= B and R >= P: fillRect(x, y, .15, .15, [1, 0, 0])
            elif B >= R and B >= P: fillRect(x, y, .15, .15, [0, .9, .9])
            elif P >= B and P >= R: fillRect(x, y, .15, .15, [.6, 0, 1])

        change_x = 1.3 * self.speed_x
        fillRect(self.x + change_x + .1, self.y - .2, .12, .12, [0, 0, 0])
        fillRect(self.x + change_x - .1, self.y - .2, .12, .12, [0, 0, 0])

class Camera:
    def __init__(self):
        self.x = 0
        self.y = 0

        self.booms = []
    
    def shake(self, x_shake, y_shake, period):
        self.booms.append({'x': x_shake, 'y': y_shake, 'period': period})
    
    def update(self):
        # move camera
        dist_x = hero.x - self.x
        dist_y = hero.y - self.y
        self.x += dist_x / 10
        self.y += dist_y / 10

        # shake camera
        for index in range(len(self.booms)):
            item = self.booms[index]
            self.x += random(-item['x'], item['x'])
            self.y += random(-item['y'], item['y'])
            item['period'] -= 1
            if item['period'] < 0: self.booms.remove(index)

        # set camera
        camera.x = self.x * scale
        camera.y = -self.y * scale

# draw rectangle with scale applied
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

def realFillRect(x, y, width, height, color, angle = 0):
    RECT.color = [1, 1, 1, 1]
    RECT.angle = 0

    RECT.x = x + camera.x
    RECT.y = -y + camera.y
    RECT.width = width
    RECT.height = height
    RECT.color = color
    RECT.angle = angle

    RECT.draw()

def drawText(text, x, y, size, color, font):
    TEXT.color = [1, 1, 1, 1]

    TEXT.content = text
    TEXT.x = x * scale
    TEXT.y = -y * scale
    TEXT.font_size = size * scale
    TEXT.color = color
    TEXT.font = font

    TEXT.draw()

def posToIndex(x, y, width, height):
    y -= 1
    if x < 0 or x >= width or y < 0 or y >= height: return 'void'
    return x + y * width

def indexToPos(index, width):
    return [index % width, math.floor(index / width)]

def collide(obj1, obj2):
    x_1 = obj1[0]
    y_1 = obj1[1]
    w_1 = obj1[2] / 2
    h_1 = obj1[3] / 2

    x_2 = obj2[0]
    y_2 = obj2[1]
    w_2 = obj2[2] / 2
    h_2 = obj2[3] / 2

    touch_x = x_1 + w_1 > x_2 - w_2 and x_1 - w_1 < x_2 + w_2
    touch_y = y_1 + h_1 > y_2 - h_2 and y_1 - h_1 < y_2 + h_2

    if touch_x and touch_y: return True

def merge(obj1, obj2, bias):
    x_1 = obj1[0]
    y_1 = obj1[1]
    w_1 = obj1[2] / 2
    h_1 = obj1[3] / 2

    x_2 = obj2[0]
    y_2 = obj2[1]
    w_2 = obj2[2] / 2
    h_2 = obj2[3] / 2

    margin = {
        'left': (x_1 + w_1) - (x_2 - w_2),
        'right': (x_1 - w_1) - (x_2 + w_2),
        'top': (y_1 + h_1) - (y_2 - h_2),
        'bottom': (y_1 - h_1) - (y_2 + h_2)
    }

    smallest_x = margin['right']
    smallest_y = margin['bottom']
    
    if margin['left'] < -margin['right']: smallest_x = margin['left']
    if margin['top'] < -margin['bottom']: smallest_y = margin['top']

    if abs(smallest_x) < abs(smallest_y) - bias: return {'x': smallest_x, 'y': 0}
    return {'x': 0, 'y': smallest_y}

def makeBackground():
    global bg_list_w, bg_list_h

    bg_list_w = world.width
    bg_list_h = world.height

    for i in range (bg_list_w * bg_list_h):
        bg_list.append({
            'size': random(.2, 1),
            'color': [random(.1, .6), random(.4, .7), random(.8, 1)]
        })

def drawBackground():
    h_half = window.width / 2 / scale
    v_half = window.height / 2 / scale

    top = math.floor(cam_y - v_half)
    bottom = math.floor(cam_y + v_half) + 2
    left = math.floor(cam_x - h_half)
    right = math.floor(cam_x + h_half) + 2

    LEFT = 0
    RIGHT = bg_list_w
    TOP = 0
    BOTTOM = bg_list_h

    if left > 0: LEFT = left
    if right < bg_list_w: RIGHT = right
    if top > 0: TOP = top
    if bottom < bg_list_h: BOTTOM = bottom

    for x in range(LEFT, RIGHT):
        for y in range(TOP, BOTTOM):
            index = posToIndex(x, y, bg_list_w, bg_list_h)
            if index == 'void': continue

            item = bg_list[index]
            fillRect(x, y, item['size'], item['size'], item['color'])

def loop():
    global scale, cam_x, cam_y, time, phase, UP, LEFT, RIGHT
    time += 1

    # resize calculations
    scale = (window.width + window.height) / 30
    cam_x = camera.x / scale
    cam_y = -camera.y / scale

    UP = key.up or key.w or key.z
    LEFT = key.left or key.a or key.q
    RIGHT = key.right or key.d

    if phase < 1:
        phase += .004

        realFillRect(0, 0, window.width, window.height, [0, 0, 0])
        drawText('Blue Hue', 0, 0, 2, [0, 1, 1], PENCIL)
    
    elif phase < 2:
        phase += .004

        realFillRect(0, 0, window.width, window.height, [0, 0, 0])
        drawText('A JoBase Game', 0, -.5, .8, [1, 1, 1], TYPEWRITER)
        drawText('By Joachim Ford', 0, .5, .8, [1, 1, 1], TYPEWRITER)
    
    elif phase < 3:
        phase += .003

        realFillRect(0, 0, window.width, window.height, [0, 0, 0])

        # get width and height of heading
        drawText('JoachimFord.uk', 0, 0, 1.1, [0, 0, 0], CODE)
        width = TEXT.width / scale
        height = TEXT.height / scale

        # shake vars
        x = 0
        y = 0

        if web_box[2] > 0 and web_box[2] < 1:
            x = random(-web_box[2] / 5, web_box[2] / 5)
            y = random(-web_box[2] / 5, web_box[2] / 5)

        # draw heading if ready
        def drawHeading():
            drawText('JoachimFord.uk', x, y, 1.1, [1, 1, 1], CODE)

            size = (phase % 1) * 3
            if size > .3: size = .3
            fillRect(x + width / 2, y - height / 2, size, size, [1, 1, 0], -time * 5)

            web_box[0] -= .01
            web_box[1] -= .01
            if web_box[2] > 0: web_box[2] -= .04
        
        # move box
        if web_box[2] == 1:
            if web_box[0] < 1: web_box[0] += .05 / (1 - web_box[0]) / 2
            if web_box[0] > 1: web_box[0] = 1
            if web_box[1] < 1: web_box[1] += .05
            if web_box[1] > 1: web_box[1] = 1
        else: drawHeading()
        if web_box[0] == 1 and web_box[1] == 1: drawHeading()
        
        fillRect(x, y, web_box[0] * width, web_box[1] * height, [1, 1, 1, web_box[2]])

    elif phase < 4:
        phase = 3
        realFillRect(0, 0, window.width, window.height, [0, 0, 0])
        drawText('Control the player with', 0, -.5, .8, [1, 1, 1], TYPEWRITER)
        drawText('the arrow keys, WASD, or ZQSD.', 0, .5, .8, [1, 1, 1], TYPEWRITER)
        drawText('Press a key to begin', 0, 1.5, .7, [1, 0, 1], TYPEWRITER)

        if key.press: phase = 4

    else:
        phase = 4
        # update & draw game
        drawBackground()

        hero.update()
        world.update()
        CAM.update()

window.color = (.2, .5, .9)

hero = Hero(0, 0)
RECT = Rectangle(0, 0, 1, 1)
TEXT = Text()
CAM = Camera()

UP = 0
LEFT = 0
RIGHT = 0

scale = 0
time = 0

bg_list = []
bg_list_w = 0
bg_list_h = 0

phase = 0

# width, height, alpha
web_box = [0, 0, 1]

world = World()
world.generate()

makeBackground()
run()
