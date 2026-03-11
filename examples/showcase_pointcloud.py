"""
Showcase: a real colored point cloud (Redwood living room fragment) displayed
with camera frustums along a synthetic trajectory.

Download the data first:
    mkdir -p data
    curl -L -o data/fragment.ply \
        "https://github.com/isl-org/open3d_downloads/releases/download/20220201-data/fragment.ply"
"""

import slamd
import numpy as np
from pathlib import Path


DATA_PATH = Path(__file__).parent.parent / "data" / "fragment.ply"


def load_fragment_ply(path: Path):
    """Load the binary PLY point cloud with RGB colors."""
    with open(path, "rb") as f:
        while True:
            line = f.readline()
            if b"end_header" in line:
                break

        vertex_dtype = np.dtype(
            [
                ("x", "<f4"),
                ("y", "<f4"),
                ("z", "<f4"),
                ("r", "u1"),
                ("g", "u1"),
                ("b", "u1"),
                ("nx", "<f4"),
                ("ny", "<f4"),
                ("nz", "<f4"),
                ("curvature", "<f4"),
            ]
        )
        raw = np.fromfile(f, dtype=vertex_dtype, count=196133)

    positions = np.column_stack((raw["x"], raw["y"], raw["z"])).astype(np.float32)
    colors = (
        np.column_stack((raw["r"], raw["g"], raw["b"])).astype(np.float32) / 255.0
    )

    # Drop points with no color
    mask = colors.sum(axis=1) > 0
    return positions[mask], colors[mask]


def look_at_pose(pos, target):
    """Build a 4x4 camera pose looking from pos toward target (Z-forward)."""
    forward = target - pos
    forward /= np.linalg.norm(forward)
    right = np.cross(np.array([0, 0, 1.0]), forward)
    right /= np.linalg.norm(right)
    up = np.cross(forward, right)

    T = np.eye(4, dtype=np.float32)
    T[:3, 0] = right
    T[:3, 1] = up
    T[:3, 2] = forward
    T[:3, 3] = pos
    return T


def make_camera_poses(cloud_positions):
    """Walk-through trajectory inside the room, roughly at eye height."""
    # Room spans roughly X: 0.6-4.0, Y: 0.8-2.4, Z: 0.6-2.6
    # Walk along X (the long axis), staying in the middle of Y, at a
    # reasonable height, looking slightly ahead and toward center
    center_y = cloud_positions[:, 1].mean()
    eye_z = 1.8  # eye height within the room

    # Waypoints walking through the room
    waypoints = [
        [1.0, center_y, eye_z],
        [1.5, center_y - 0.15, eye_z],
        [2.0, center_y + 0.1, eye_z],
        [2.5, center_y, eye_z],
        [3.0, center_y + 0.15, eye_z],
        [3.5, center_y - 0.1, eye_z],
    ]

    poses = []
    for i, wp in enumerate(waypoints):
        pos = np.array(wp)
        # Look toward the next waypoint (last one looks same direction as previous)
        if i < len(waypoints) - 1:
            target = np.array(waypoints[i + 1])
        else:
            target = pos + (pos - np.array(waypoints[i - 1]))
        # Tilt gaze slightly downward
        target[2] -= 0.3
        poses.append(look_at_pose(pos, target))

    return poses


def main():
    if not DATA_PATH.exists():
        print(f"Data not found at {DATA_PATH}")
        print("Download it with:")
        print('  mkdir -p data')
        print(
            '  curl -L -o data/fragment.ply '
            '"https://github.com/isl-org/open3d_downloads/releases/download/20220201-data/fragment.ply"'
        )
        return

    vis = slamd.Visualizer("Point Cloud Scene")
    scene = vis.scene("scene")

    positions, colors = load_fragment_ply(DATA_PATH)
    radii = np.full(len(positions), 0.008, dtype=np.float32)
    scene.set_object(
        "/cloud", slamd.geom.PointCloud(positions, colors, radii, 0.7)
    )

    # Camera frustums orbiting the cloud
    center = positions.mean(axis=0)
    poses = make_camera_poses(8, center, radius=2.0, height=center[2] + 0.5)

    K = np.array([[500, 0, 320], [0, 500, 240], [0, 0, 1]], dtype=np.float64)
    for i, T in enumerate(poses):
        scene.set_object(
            f"/cameras/cam_{i}/frustum",
            slamd.geom.CameraFrustum(K, 640, 480, scale=0.15),
        )
        scene.set_transform(f"/cameras/cam_{i}", T)

    # Trajectory line through camera positions
    traj = np.array([T[:3, 3] for T in poses], dtype=np.float32)
    scene.set_object(
        "/cameras/trajectory",
        slamd.geom.PolyLine(traj, 0.01, np.array([0.2, 0.7, 1.0]), 0.9),
    )

    scene.set_object("/origin", slamd.geom.Triad(scale=0.3))

    vis.hang_forever()


if __name__ == "__main__":
    main()
