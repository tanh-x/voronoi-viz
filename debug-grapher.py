import numpy as np
import matplotlib.pyplot as plt

plt.style.use("bmh")

inf = 200

#=======================================


sites = np.array([
	(61.000000, 121.000000),
	(71.000000, 121.000000),
	(101.000000, 121.000000),
	(141.000000, 121.000000),
	(81.000000, 61.000000),
	(71.000000, 39.000000),
	(71.000000, 142.250000),
])

# Vertex list
verts = np.array([
	(215.333333, -13.333333),
	(21.846154, 74.615385),
	(121.000000, 81.000000),
	(66.000000, 89.333333),
	(86.000000, 92.666667),
	(86.000000, 131.625000),
	(121.000000, 181.036765),
	(66.000000, 131.625000),
	(-48.988438, -83.726466),
	(286.167925, -83.726466),
	(286.167925, 251.429897),
	(-48.988438, 251.429897),
	(-48.988438, 185.737206),
	(286.167925, -73.801887),
	(142.369344, 251.429897),
	(-48.988438, 65.977020),
])

# Edge list
v1 = np.array([
	(-48.988438, 185.737206),
	(86.000000, 131.625000),
	(215.333333, -13.333333),
	(215.333333, -13.333333),
	(86.000000, 131.625000),
	(121.000000, 181.036765),
	(66.000000, 89.333333),
	(142.369344, 251.429897),
	(66.000000, 131.625000),
	(86.000000, 131.625000),
	(86.000000, 92.666667),
	(66.000000, 89.333333),
	(121.000000, 81.000000),
	(21.846154, 74.615385),
])

v2 = np.array([
	(66.000000, 131.625000),
	(66.000000, 131.625000),
	(286.167925, -73.801887),
	(21.846154, 74.615385),
	(121.000000, 181.036765),
	(121.000000, 81.000000),
	(21.846154, 74.615385),
	(121.000000, 181.036765),
	(66.000000, 89.333333),
	(86.000000, 92.666667),
	(121.000000, 81.000000),
	(86.000000, 92.666667),
	(215.333333, -13.333333),
	(-48.988438, 65.977020),
])



#=======================================
plt.figure(figsize=(8, 8))
plt.scatter(sites[:, 0], sites[:, 1], s=30, c='r')
plt.scatter(verts[:, 0], verts[:, 1], c='k')
plt.scatter(v2[:, 0], v2[:, 1], c='#A447C9')

# Plot edges
for start, end in zip(v1, v2):
    plt.plot([start[0], end[0]], [start[1], end[1]], '-')

plt.grid(True)
plt.axis('equal')
plt.show()
