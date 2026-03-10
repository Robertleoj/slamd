from __future__ import annotations
from pathlib import Path
import threading
import subprocess
from sys import argv
from . import geom, geom2d


from .bindings import (
    __doc__,
    Visualizer as Visualizer_internal,
    Canvas,
    Scene,
    # geom,
    spawn_window as spawn_window_internal,
)


def _executable_path():
    executable_path = Path(__file__).parent / "slamd_window"

    if not executable_path.exists():
        print("Executable path not found! Assuming dev install, passing None")
        executable_path = None

    return executable_path


class Visualizer:
    """A visualizer instance.

    Starts a TCP server and can be connected to by slamd windows.

    Args:
        name: A name for the visualizer.
        spawn: If True, spawns a visualizer window.
        port: The port number for communication.
    """

    def __init__(self, name: str, spawn=True, port: int = 5555) -> None:
        self._impl = Visualizer_internal(name, port)

        if spawn:
            spawn_window(port)

    def hang_forever(self):
        """Block execution forever (used to keep the visualizer alive)."""
        threading.Event().wait()

    def add_scene(self, name: str, scene: Scene) -> None:
        """Add a named 3D scene to the visualizer.

        Args:
            name: Name of the scene.
            scene: The Scene object to add.
        """
        return self._impl.add_scene(name, scene)

    def add_canvas(self, name: str, canvas: Canvas) -> None:
        """Add a named 2D canvas to the visualizer.

        Args:
            name: Name of the canvas.
            canvas: The Canvas object to add.
        """
        return self._impl.add_canvas(name, canvas)

    def canvas(self, name: str) -> Canvas:
        """Create and add a new 2D canvas to the visualizer.

        Args:
            name: Name of the canvas.

        Returns:
            The newly created Canvas object.
        """
        return self._impl.canvas(name)

    def scene(self, name: str) -> Scene:
        """Create and add a new 3D scene to the visualizer.

        Args:
            name: Name of the scene.

        Returns:
            The newly created Scene object.
        """
        return self._impl.scene(name)

    def delete_scene(self, name: str) -> None:
        """Delete a scene by name.

        Args:
            name: Name of the scene to delete.
        """
        self._impl.delete_scene(name)

    def delete_canvas(self, name: str) -> None:
        """Delete a canvas by name

        Args:
            name: Name of the canvas to delete.
        """
        self._impl.delete_canvas(name)


def spawn_window(port: int = 5555) -> None:
    """Spawn a visualizer window on the given port.

    Args:
        port: Port number to spawn the visualizer on.
    """
    executable_path = _executable_path()

    spawn_window_internal(
        port,
        str(executable_path) if executable_path is not None else None,
    )


def _window_cli():
    exe_path = _executable_path()

    if exe_path is None:
        raise RuntimeError("Can't find exe path")

    subprocess.run([exe_path, *argv[1:]])


__all__ = [
    "__doc__",
    "Visualizer",
    "Canvas",
    "Scene",
    "geom",
    "geom2d",
    "spawn_window",
    "_window_cli",
]
