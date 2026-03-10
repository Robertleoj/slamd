import numpy as np


def process_radii(
    radii: np.ndarray | float, 
    n: int
) -> np.ndarray:
    if isinstance(radii, float) or isinstance(radii, int):
        radii_np = np.full((n,), float(radii), dtype=np.float32)
    elif isinstance(radii, np.ndarray):
        if radii.shape == (n,):
            radii_np = radii.astype(np.float32)
        else:
            raise ValueError(f"Invalid radii shape: {radii.shape}")
    else:
        raise TypeError("radii must be a float or a 1D np.ndarray")

    return radii_np


def process_color(
    colors: np.ndarray | tuple[int, int, int], 
    n: int
) -> np.ndarray:
    if isinstance(colors, tuple):
        colors_np = np.tile(np.array(colors, dtype=np.float32) / 255.0, (n, 1))
    elif isinstance(colors, np.ndarray):
        colors = colors.astype(np.float32)
        if colors.max() > 1.0:
            colors /= 255.0
        if colors.shape == (3,) or colors.shape == (1, 3):
            colors_np = np.tile(colors.reshape(1, 3), (n, 1))
        elif colors.shape == (n, 3):
            colors_np = colors
        else:
            raise ValueError(f"Invalid color shape: {colors.shape}")
    else:
        raise TypeError("colors must be an np.ndarray or a (R, G, B) tuple")

    return colors_np

def process_single_color(color: np.ndarray | tuple[int, int, int]) -> np.ndarray:
    if isinstance(color, tuple):
        return np.array(color, dtype=np.float32) / 255.0
    elif isinstance(color, np.ndarray):
        color = color.astype(np.float32)
        if color.max() > 1.0:
            color /= 255.0
        if color.shape in [(3,), (1, 3)]:
            return color.reshape(3,)
        else:
            raise ValueError(f"Invalid color shape: {color.shape}")
    else:
        raise TypeError("color must be an np.ndarray or a (R, G, B) tuple")