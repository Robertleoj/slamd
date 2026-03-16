#include <slamd_window/geom/arrows.hpp>
#include <slamd_window/geom/box.hpp>
#include <slamd_window/geom/camera_frustum.hpp>
#include <slamd_window/geom/geometry.hpp>
#include <slamd_window/geom/image.hpp>
#include <slamd_window/geom/mesh.hpp>
#include <slamd_window/geom/plane.hpp>
#include <slamd_window/geom/point_cloud.hpp>
#include <slamd_window/geom/poly_line.hpp>
#include <slamd_window/geom/sphere.hpp>
#include <slamd_window/geom/spheres.hpp>
#include <slamd_window/geom/triad.hpp>

namespace slamd {
namespace _geom {

std::shared_ptr<Geometry> Geometry::deserialize(
    const slamd::flatb::Geometry* geom_fb
) {
    switch (geom_fb->geometry_type()) {
        case (slamd::flatb::GeometryUnion_triad): {
            return Triad::deserialize(geom_fb->geometry_as_triad());
        }
        case (slamd::flatb::GeometryUnion_camera_frustum): {
            return CameraFrustum::deserialize(
                geom_fb->geometry_as_camera_frustum()
            );
        }
        case (slamd::flatb::GeometryUnion_point_cloud): {
            return PointCloud::deserialize(geom_fb->geometry_as_point_cloud());
        }
        case (slamd::flatb::GeometryUnion_image): {
            return Image::deserialize(geom_fb->geometry_as_image());
        }
        case (slamd::flatb::GeometryUnion_box): {
            return Box::deserialize(geom_fb->geometry_as_box());
        }
        case (slamd::flatb::GeometryUnion_sphere): {
            return Sphere::deserialize(geom_fb->geometry_as_sphere());
        }
        case (slamd::flatb::GeometryUnion_arrows): {
            return Arrows::deserialize(geom_fb->geometry_as_arrows());
        }
        case (slamd::flatb::GeometryUnion_poly_line): {
            return PolyLine::deserialize(geom_fb->geometry_as_poly_line());
        }
        case (slamd::flatb::GeometryUnion_mesh): {
            return Mesh::deserialize(geom_fb->geometry_as_mesh());
        }
        case (slamd::flatb::GeometryUnion_plane): {
            return Plane::deserialize(geom_fb->geometry_as_plane());
        }
        case (slamd::flatb::GeometryUnion_spheres): {
            return Spheres::deserialize(geom_fb->geometry_as_spheres());
        }
        default: {
            throw std::runtime_error("Unknown geometry type");
        }
    }
}

}  // namespace _geom
}  // namespace slamd
