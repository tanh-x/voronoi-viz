import numpy as np

points = np.random.uniform(-7, 7, (20, 2))
points_list = list(map(tuple, points))
for p in points_list: print(p)