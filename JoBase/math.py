import math
import random

def distance(x1, y1, x2, y2):
    return math.hypot(x2 - x1, y2 - y1)

def angle(x1, y1, x2, y2):
    return math.degrees(math.atan2(y2 - y1, x2 - x1))

def direction(length, angle):
    radians = math.radians(angle)
    return length * math.cos(radians), length * math.sin(radians)

def random_number(number1, number2):
    if number1 > number2: return random.randint(int(number2), int(number1))
    return random.randint(int(number1), int(number2))

def random_decimal(number1, number2):
    if number1 > number2: return random.random() * (number1 - number2 + 1) + number2
    return random.random() * (number2 - number1 + 1) + number1

def rotate_point(x, y, center_x, center_y, rotation):
    point_x, point_y = direction(distance(center_x, center_y, x, y), angle(center_x, center_y, x, y) + rotation)
    return point_x + center_x, point_y + center_y

def rotate_toward_goal(current, goal, increment = 1):
    angle = goal - current
    
    if abs(angle) < increment: current = goal
    
    elif abs(angle) > 180:
        if angle < 0: current += increment
        else: current -= increment
        
    else:
        if angle < 0: current -= increment
        else: current += increment
        
    if current >= 360: current -= 360
    elif current < 0: current += 360
    
    return current    