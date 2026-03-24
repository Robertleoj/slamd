#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <slamd/slamd.hpp>
#include <slamd/spawn_window.hpp>

namespace py = pybind11;

namespace pybind11::detail {

template <>
struct type_caster<glm::vec3> {
    PYBIND11_TYPE_CASTER(glm::vec3, _("numpy.ndarray"));

    bool load(
        handle src,
        bool
    ) {
        auto buf = py::array_t < float,
             py::array::c_style | py::array::forcecast > ::ensure(src);
        if (!buf || buf.ndim() != 1 || buf.shape(0) != 3) {
            return false;
        }
        value = glm::vec3(buf.at(0), buf.at(1), buf.at(2));
        return true;
    }

    static handle cast(
        const glm::vec3& v,
        return_value_policy,
        handle
    ) {
        return py::array_t<float>({3}, {sizeof(float)}, &v[0]).release();
    }
};

template <>
struct type_caster<glm::mat4> {
    PYBIND11_TYPE_CASTER(glm::mat4, _("numpy.ndarray"));

    bool load(
        handle src,
        bool
    ) {
        auto buf = py::array_t < float,
             py::array::c_style | py::array::forcecast > ::ensure(src);
        if (!buf || buf.ndim() != 2 || buf.shape(0) != 4 || buf.shape(1) != 4) {
            return false;
        }
        glm::mat4 m(1.0f);
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[j][i] = buf.at(i, j);  // GLM column-major
            }
        }
        value = m;
        return true;
    }

    static handle cast(
        const glm::mat4& m,
        return_value_policy,
        handle
    ) {
        return py::array_t<float>(
                   {4, 4},
                   {sizeof(float) * 4, sizeof(float)},
                   &m[0][0]
        )
            .release();
    }
};

template <>
struct type_caster<glm::mat3> {
    PYBIND11_TYPE_CASTER(glm::mat3, _("numpy.ndarray"));

    bool load(
        handle src,
        bool
    ) {
        auto buf = py::array_t < float,
             py::array::c_style | py::array::forcecast > ::ensure(src);
        if (!buf || buf.ndim() != 2 || buf.shape(0) != 3 || buf.shape(1) != 3) {
            return false;
        }
        glm::mat4 m(1.0f);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                m[j][i] = buf.at(i, j);  // GLM column-major
            }
        }
        value = m;
        return true;
    }

    static handle cast(
        const glm::mat4& m,
        return_value_policy,
        handle
    ) {
        return py::array_t<float>(
                   {3, 3},
                   {sizeof(float) * 3, sizeof(float)},
                   &m[0][0]
        )
            .release();
    }
};

// vector<glm::vec3>
template <>
struct type_caster<std::vector<glm::vec3>> {
    PYBIND11_TYPE_CASTER(std::vector<glm::vec3>, _("numpy.ndarray"));

    bool load(
        handle src,
        bool
    ) {
        auto buf = py::array_t < float,
             py::array::c_style | py::array::forcecast > ::ensure(src);
        if (!buf || buf.ndim() != 2 || buf.shape(1) != 3) {
            return false;
        }
        ssize_t n = buf.shape(0);
        value.resize(n);
        for (ssize_t i = 0; i < n; ++i) {
            value[i] = glm::vec3(buf.at(i, 0), buf.at(i, 1), buf.at(i, 2));
        }
        return true;
    }

    static handle cast(
        const std::vector<glm::vec3>& vecs,
        return_value_policy,
        handle
    ) {
        std::vector<ssize_t> shape = {static_cast<ssize_t>(vecs.size()), 3};
        std::vector<ssize_t> strides = {
            static_cast<ssize_t>(sizeof(float) * 3),
            static_cast<ssize_t>(sizeof(float))
        };

        py::array_t<float> arr(py::buffer_info(
            nullptr,                                 // no data yet
            sizeof(float),                           // size of one float
            py::format_descriptor<float>::format(),  // format string
            2,                                       // ndim
            shape,                                   // shape
            strides                                  // strides
        ));

        auto r = arr.mutable_unchecked<2>();
        for (size_t i = 0; i < vecs.size(); ++i) {
            for (int j = 0; j < 3; ++j) {
                r(i, j) = vecs[i][j];
            }
        }

        return arr.release();
    }
};

template <>
struct type_caster<slamd::data::Image> {
    PYBIND11_TYPE_CASTER(slamd::data::Image, _("numpy.ndarray"));

    bool load(
        handle src,
        bool
    ) {
        // Ensure it's a 3D numpy array of uint8
        auto buf = py::array_t < uint8_t,
             py::array::c_style | py::array::forcecast > ::ensure(src);
        if (!buf || buf.ndim() != 3) {
            return false;
        }

        size_t height = buf.shape(0);
        size_t width = buf.shape(1);
        size_t channels = buf.shape(2);

        std::vector<uint8_t> data(buf.size());
        std::memcpy(data.data(), buf.data(), buf.size());

        value = slamd::data::Image{std::move(data), width, height, channels};
        return true;
    }

    static handle cast(
        const slamd::data::Image&,
        return_value_policy,
        handle
    ) {
        throw std::runtime_error("Casting data::Image → numpy not supported");
    }
};

template <>
struct type_caster<std::vector<float>>
    : list_caster<std::vector<float>, float> {
    using base = list_caster<std::vector<float>, float>;
    static constexpr auto name =
        const_name("Union[List[float], numpy.ndarray]");

    bool load(
        handle src,
        bool convert
    ) {
        // Fast path: NumPy float32 array
        if (py::isinstance<py::array_t<float>>(src)) {
            auto arr = py::array_t<float>::ensure(src);
            if (!arr) {
                return false;
            }

            py::buffer_info info = arr.request();
            if (info.ndim != 1) {
                return false;
            }

            value.resize(info.shape[0]);
            std::memcpy(value.data(), info.ptr, info.shape[0] * sizeof(float));
            return true;
        }

        // Fallback: standard py::list behavior
        return base::load(src, convert);
    }

    // Cast works fine from base
};

}  // namespace pybind11::detail

void define_private_geom(
    py::module_& m
) {
    py::class_<slamd::geom::Geometry, std::shared_ptr<slamd::geom::Geometry>>(
        m,
        "Geometry"
    );

    py::class_<
        slamd::geom::Box,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Box>>(m, "Box");

    py::class_<
        slamd::geom::Plane,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Plane>>(m, "Plane");

    py::class_<
        slamd::geom::Arrows,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Arrows>>(m, "Arrows");

    py::class_<
        slamd::geom::CameraFrustum,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::CameraFrustum>>(m, "CameraFrustum");

    py::class_<
        slamd::geom::PointCloud,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::PointCloud>>(m, "PointCloud")
        .def(
            "update_positions",
            &slamd::geom::PointCloud::update_positions,
            py::arg("positions")
        )
        .def(
            "update_colors",
            &slamd::geom::PointCloud::update_colors,
            py::arg("colors")
        )
        .def(
            "update_radii",
            &slamd::geom::PointCloud::update_radii,
            py::arg("radii")
        );

    py::class_<
        slamd::geom::Spheres,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Spheres>>(m, "Spheres")
        .def(
            "update_positions",
            &slamd::geom::Spheres::update_positions,
            py::arg("positions")
        )
        .def(
            "update_colors",
            &slamd::geom::Spheres::update_colors,
            py::arg("colors")
        )
        .def(
            "update_radii",
            &slamd::geom::Spheres::update_radii,
            py::arg("radii")
        );

    py::class_<
        slamd::geom::PolyLine,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::PolyLine>>(m, "PolyLine");

    py::class_<
        slamd::geom::Mesh,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Mesh>>(m, "Mesh")
        .def(
            "update_positions",
            &slamd::geom::Mesh::update_positions,
            py::arg("positions"),
            py::arg("recompute_normals") = true
        )
        .def(
            "update_colors",
            &slamd::geom::Mesh::update_colors,
            py::arg("colors")
        )
        .def(
            "update_normals",
            &slamd::geom::Mesh::update_normals,
            py::arg("normals")
        );

    py::class_<
        slamd::geom::Sphere,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Sphere>>(m, "Sphere");

    py::class_<
        slamd::geom::Triad,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Triad>>(m, "Triad");

    py::class_<
        slamd::geom::Image,
        slamd::geom::Geometry,
        std::shared_ptr<slamd::geom::Image>>(m, "Image");
}

void define_geom(
    py::module_& m
) {
    m.def(
        "Box",
        &slamd::geom::box,
        py::arg("dims") = glm::vec3(1.0f),
        py::arg("color") = glm::vec3(0.8f, 0.2f, 0.0f),
        "Create a Box geometry"
    );
    m.def(
        "Arrows",
        &slamd::geom::arrows,
        py::arg("starts"),
        py::arg("ends"),
        py::arg("colors"),
        py::arg("thickness"),
        "Create an Arrows geometry"
    );
    m.def(
        "Plane",
        &slamd::geom::plane,
        py::arg("normal"),
        py::arg("point"),
        py::arg("color"),
        py::arg("radius"),
        py::arg("alpha"),
        "Create a Plane geometry"
    );

    m.def(
        "CameraFrustum",
        [](glm::mat3 intrinsics_matrix,
           size_t image_width,
           size_t image_height,
           std::optional<slamd::data::Image> image,
           float scale = 1.0) {
            if (!image.has_value()) {
                return slamd::geom::camera_frustum(
                    intrinsics_matrix,
                    image_width,
                    image_height,
                    scale
                );
            }
            return slamd::geom::camera_frustum(
                intrinsics_matrix,
                image_width,
                image_height,
                std::move(image.value()),
                scale
            );
        },
        py::arg("intrinsics_matrix"),
        py::arg("image_width"),
        py::arg("image_height"),
        py::arg("image") = std::nullopt,
        py::arg("scale") = 1.0f,
        "Create a CameraFrustum geometry"
    );

    // Overload: per-point color + per-point radius
    m.def(
        "PointCloud",
        [](const std::vector<glm::vec3>& positions,
           const std::vector<glm::vec3>& colors,
           const std::vector<float>& radii,
           float min_brightness) {
            return slamd::geom::point_cloud(
                positions,
                colors,
                radii,
                min_brightness
            );
        },
        py::arg("positions"),
        py::arg("colors"),
        py::arg("radii"),
        py::arg("min_brightness") = 1.0f,
        "Create a PointCloud with per-point color and radius"
    );

    m.def(
        "Spheres",
        [](const std::vector<glm::vec3>& positions,
           const std::vector<glm::vec3>& colors,
           const std::vector<float>& radii,
           float min_brightness) {
            return slamd::geom::spheres(
                positions,
                colors,
                radii,
                min_brightness
            );
        },
        py::arg("positions"),
        py::arg("colors"),
        py::arg("radii"),
        py::arg("min_brightness") = 0.3f,
        "Create Spheres with per-point color and radius"
    );

    m.def(
        "PolyLine",
        &slamd::geom::poly_line,
        py::arg("points"),
        py::arg("thickness"),
        py::arg("color"),
        py::arg("min_brightness"),
        "Create a PolyLine geometry"
    );

    m.def(
        "Mesh",
        [](const std::vector<glm::vec3>& positions,
           const std::vector<glm::vec3>& vertex_colors,
           const std::vector<uint32_t>& triangle_indices) {
            slamd::data::MeshData data = slamd::data::MeshDataBuilder()
                                             .set_positions(positions)
                                             .set_colors(vertex_colors)
                                             .set_indices(triangle_indices)
                                             .compute_normals()
                                             .build();

            return slamd::geom::mesh(std::move(data));
        },
        py::arg("vertices"),
        py::arg("vertex_colors"),
        py::arg("triangle_indices"),
        "Create a SimpleMesh geometry from raw data"
    );

    m.def(
        "Mesh",
        [](const std::vector<glm::vec3>& positions,
           const std::vector<glm::vec3>& vertex_colors,
           const std::vector<uint32_t>& triangle_indices,
           const std::vector<glm::vec3>& normals) {
            slamd::data::MeshData data = slamd::data::MeshDataBuilder()
                                             .set_positions(positions)
                                             .set_colors(vertex_colors)
                                             .set_indices(triangle_indices)
                                             .set_normals(normals)
                                             .build();

            return slamd::geom::mesh(std::move(data));
        },
        py::arg("vertices"),
        py::arg("vertex_colors"),
        py::arg("triangle_indices"),
        py::arg("vertex_normals"),
        "Create a SimpleMesh geometry from raw data"
    );

    m.def(
        "Mesh",
        [](const std::vector<glm::vec3>& positions,
           const std::vector<glm::vec3>& vertex_colors,
           const std::vector<uint32_t>& triangle_indices,
           const std::vector<glm::vec3>& normals,
           float alpha) {
            slamd::data::MeshData data = slamd::data::MeshDataBuilder()
                                             .set_positions(positions)
                                             .set_colors(vertex_colors)
                                             .set_indices(triangle_indices)
                                             .set_normals(normals)
                                             .set_alpha(alpha)
                                             .build();

            return slamd::geom::mesh(std::move(data));
        },
        py::arg("vertices"),
        py::arg("vertex_colors"),
        py::arg("triangle_indices"),
        py::arg("vertex_normals"),
        py::arg("alpha"),
        "Create a SimpleMesh geometry from raw data with transparency"
    );

    m.def(
        "Sphere",
        &slamd::geom::sphere,
        py::arg("radius") = 1.0f,
        py::arg("color") = glm::vec3(0.8f, 0.2f, 0.0f),
        "Create a Sphere geometry"
    );

    m.def(
        "Triad",
        [](std::optional<glm::mat4> pose, float scale, float thickness) {
            if (pose.has_value()) {
                return slamd::geom::triad(pose.value(), scale, thickness);
            } else {
                return slamd::geom::triad(scale, thickness);
            }
        },
        py::arg("pose") = std::nullopt,
        py::arg("scale") = 1.0f,
        py::arg("thickness") = 0.1f,
        "Create a Triad geometry"
    );
}

PYBIND11_MODULE(
    bindings,
    m
) {
    m.doc() = "slamd visualization library";

    py::class_<slamd::Scene, std::shared_ptr<slamd::Scene>>(m, "Scene")
        .def(py::init([]() {
            return slamd::scene();
        }))
        .def(
            "set_transform",
            &slamd::Scene::set_transform,
            py::arg("path"),
            py::arg("transform")
        )
        .def(
            "set_object",
            [](slamd::Scene& self,
               const std::string& path,
               std::shared_ptr<slamd::geom::Geometry> object) {
                self.set_object(path, object);
            },
            py::arg("path"),
            py::arg("object")
        )
        .def("clear", &slamd::Scene::clear, py::arg("path"));

    py::class_<
        slamd::_vis::Visualizer,
        std::shared_ptr<slamd::_vis::Visualizer>>(m, "Visualizer")
        .def(
            py::init([](std::string name, uint16_t port) {
                return slamd::visualizer(name, false, port);
            }),
            py::arg("name"),
            py::arg("port") = 5555
        )
        .def(
            "add_scene",
            &slamd::_vis::Visualizer::add_scene,
            py::arg("name"),
            py::arg("scene")
        )
        .def("scene", &slamd::_vis::Visualizer::scene, py::arg("name"))
        .def("delete_scene", &slamd::_vis::Visualizer::delete_scene)
        .def("stop", &slamd::_vis::Visualizer::stop);

    m.def(
        "spawn_window",
        &slamd::spawn_window,
        py::arg("port") = 5555,
        py::arg("executable_path") = std::nullopt
    );

    auto geom_types = m.def_submodule("_geom_types");
    define_private_geom(geom_types);

    auto geom = m.def_submodule("geom");
    define_geom(geom);
}
