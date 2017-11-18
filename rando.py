import random
import math

points = []

def r():
    return random.random() * 2 - 1

def norm():
    x, y, z = r(), r(), r()
    l = math.sqrt(x ** 2 + y ** 2 + z ** 2)
    return x / l, y / l, z /l

for _ in range(0, 32):
    points.append(norm())

for point in points:
        print('vec3(%f, %f, %f),' % point)
        # print('%f %f %f' % point)
