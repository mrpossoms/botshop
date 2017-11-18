import math
import random

points = [ (0, 0, 1) ]

ring_verts = 10
slices = 3

for phi in range(1, slices):
    r = random.random()
    for tau in range(0, ring_verts):
        p = ((math.pi * 0.5) / (slices)) * phi
        t = (2 * math.pi / ring_verts) * tau + r
        point = ( math.cos(t) * math.sin(p),
                  math.sin(t) * math.sin(p),
                  math.cos(p))
        points.append(point)


for point in points:
        print('vec3(%f, %f, %f),' % point)
        # print('%f %f %f' % point)
