import numpy as np
from matplotlib import pyplot as plt
from matplotlib.animation import FuncAnimation

plt.style.use("bmh")
plt.rc("font", family="Barlow", size=13)

def point_directrix_parabola(x, focus, directrix):
    result = ((x**2) - 2 * focus[0] * x + focus[0]**2 + focus[1]**2 - directrix**2) / (2 * (focus[1] - directrix))
    return result

def find_intersections(v1, v2, d):
    intersections = []
    for p1, p2 in zip(v1, v2):
        if p1[1] > p2[1]:
            p1, p2 = p2, p1
        if p1[1] <= d <= p2[1]:
            if p1[1] != p2[1]:
                x_intersect = p1[0] + ((d - p1[1]) / (p2[1] - p1[1])) * (p2[0] - p1[0])
                intersections.append((x_intersect, d))
    return intersections


with open('dump', 'r') as file:
    dump = file.read()

sites = None
verts = None
v1 = None
v2 = None
exec(dump)
# sites, verts, v1, v2

all_points = np.vstack([sites, verts])
top_right = np.max(all_points, axis=0)
bottom_left = np.min(all_points, axis=0)

# print(top_right, bottom_left)

frames = 150
step = (top_right[1] - bottom_left[1])/frames
# print(step)
fig = plt.figure(figsize=(8, 8))
h = 1024
dx = (top_right[0] - bottom_left[0]) / (h - 1)
def main(i):
    plt.cla()
    xs = np.linspace(bottom_left[0], top_right[0], h)
    sweepline = top_right[1] - i * step

    plt.scatter(sites[:, 0], sites[:, 1], s=30, c='#88f')
    accumulator = np.full_like(xs, np.inf)

    first = False
    for site in sites:
        if (site[1] < sweepline): continue
        first = True

        functional = (lambda x: point_directrix_parabola(x, site, sweepline))
        result = functional(xs)
        plt.plot(xs, result, c="#333", ls="--", lw="1")
        accumulator = np.vstack([accumulator, result])

    minimal = np.min(accumulator, axis=0)

    if first:
        for start, end in zip(v1, v2):
            ix = int((start[0] - bottom_left[0]) / dx)
            if (start[1] < minimal[ix]): continue
            plt.plot([start[0], end[0]], [start[1], end[1]], c="#b33", lw=2)

        for s in sites:
            ix = int((s[0] - bottom_left[0]) / dx)
            if (s[1] < minimal[ix]): continue
            plt.scatter([s[0]], [s[1]], c='k')

        for v in verts:
            ix = int((v[0] - bottom_left[0]) / dx)
            if (v[1] < minimal[ix]): continue
            plt.scatter([v[0]], [v[1]], c='#911', s=40)

        plt.plot(xs, minimal, c='k', lw=2)


    plt.axhline(sweepline, c='#22c', lw=2.5)

    plt.xlim(bottom_left[0], top_right[0])
    plt.ylim(bottom_left[1], top_right[1])

animation = FuncAnimation(fig, main, frames=frames, interval=1000/15, repeat=True)
plt.show()