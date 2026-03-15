"""Multiple windows — two scenes side by side."""

import slamd
import numpy as np

vis = slamd.Visualizer("Two Windows")

scene1 = vis.scene("camera view")
scene2 = vis.scene("point cloud")

# Scene 1: camera frustum with an image
K = np.array([[500, 0, 320], [0, 500, 240], [0, 0, 1]], dtype=np.float64)
w, h = 640, 480
img = np.zeros((h, w, 3), dtype=np.uint8)
img[:h // 2] = [140, 180, 230]   # sky
img[h // 2:] = [80, 130, 60]     # ground

scene1.set_object("/origin", slamd.geom.Triad())
scene1.set_object("/cam/frustum", slamd.geom.CameraFrustum(K, w, h, img, 0.8))
scene1.set_object("/cam/triad", slamd.geom.Triad(scale=0.3))

cam_tf = np.eye(4)
cam_tf[0, 3] = 2.0
cam_tf[2, 3] = 1.0
scene1.set_transform("/cam", cam_tf)

# Scene 2: point cloud
n = 50000
rng = np.random.default_rng(42)
theta = rng.uniform(0, 2 * np.pi, n)
r = rng.exponential(2.0, n)
z = rng.normal(0, 0.5, n)
positions = np.column_stack([r * np.cos(theta), r * np.sin(theta), z]).astype(np.float32)

dist = np.sqrt(positions[:, 0]**2 + positions[:, 1]**2)
d = dist / dist.max()
colors = np.column_stack([0.9 * d, 0.3 + 0.5 * (1 - d), 0.8 * (1 - d)]).astype(np.float32)

scene2.set_object("/cloud", slamd.geom.PointCloud(positions, colors, 0.08, 0.6))
scene2.set_object("/origin", slamd.geom.Triad())

vis.hang_forever()
