import slamd
import numpy as np


def random_poses(n: int, t_scale: float = 10.0) -> np.ndarray:
    poses = []
    for _ in range(n):
        M = np.random.randn(3, 3)
        Q, R = np.linalg.qr(M)
        if np.linalg.det(Q) < 0:
            Q[:, 0] *= -1

        t = np.random.uniform(-t_scale, t_scale, size=(3,))

        T = np.eye(4)
        T[:3, :3] = Q
        T[:3, 3] = t
        poses.append(T)

    return np.stack(poses, axis=0)


def main():
    vis = slamd.Visualizer("poses", port=4399)
    scene = vis.scene("poses")

    for i, pose_mat in enumerate(random_poses(100, 10)):
        scene.set_object(f"/triad_{i}", slamd.geom.Triad(pose_mat))

    last_pose_mat = np.eye(4)
    last_pose_mat[0, 3] = 20

    scene.set_object("/triad_x", slamd.geom.Triad(last_pose_mat, 10))

    vis.hang_forever()


if __name__ == "__main__":
    main()
