import numpy as np
import matplotlib.pyplot as plt

plt.style.use("bmh")

inf = 200

#=======================================


sites = np.array([
	(2.000000, 2.000000),
	(10.000000, 2.000000),
	(6.000000, 0.100000),
	(9.000000, 5.000000),
])

# Vertex list
verts = np.array([
	(7.200360, 2.733453),
	(5.340514, 3.872134),
	(-0.897282, -4.347282),	# boundary
	(12.897282, -4.347282),	# boundary
	(12.897282, 9.447282),	# boundary
	(-0.897282, 9.447282),	# boundary
	(1.436291, -4.347282),	# boundary
	(10.563709, -4.347282),	# boundary
	(12.897282, 4.632427),	# boundary
	(2.951165, 9.447282),	# boundary
])

# Edge list
v1 = np.array([
	(5.340514, 3.872134),
	(7.200360, 2.733453),
	(7.200360, 2.733453),
	(12.897282, 4.632427),
	(2.951165, 9.447282),
])

v2 = np.array([
	(1.436291, -4.347282),
	(5.340514, 3.872134),
	(10.563709, -4.347282),
	(7.200360, 2.733453),
	(5.340514, 3.872134),
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
