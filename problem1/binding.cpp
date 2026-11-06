// pybind11 bindings for the scattering classes in scattering.h
// Build with the %%bash cell at the top of problem1.ipynb (g++/clang++ with $(python3 -m pybind11 --includes))
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>   // converts std::vector<std::pair<double,double>> to a list of (r, theta) tuples
#include "scattering.h"

namespace py = pybind11;

// Bind CrossSection<V> for one potential type V.
// (CrossSection stores a copy of the potential, so no keep_alive is needed.)
template <typename V>
void bind_cross_section(py::module_ &m, const char *name) {
  py::class_<CrossSection<V>>(m, name)
    .def(py::init<V const &, double, double, double, unsigned int>(),
         py::arg("V"), py::arg("E"), py::arg("b"), py::arg("r_max"), py::arg("steps"))
    // Release the GIL during the (pure C++) calculation, so many trajectories can be
    // computed in parallel with python threads (e.g. concurrent.futures.ThreadPoolExecutor)
    .def("calculate_trajectory", &CrossSection<V>::calculate_trajectory,
         py::call_guard<py::gil_scoped_release>())
    .def("get_trajectory", &CrossSection<V>::get_trajectory);
}

PYBIND11_MODULE(scattering, m) {
  m.doc() = "Classical scattering from central potentials (pybind11 bindings for scattering.h)";

  // Potentials
  py::class_<lennard_jones>(m, "lennard_jones")
    .def(py::init<double, double>(), py::arg("V0"), py::arg("r0") = 1.0)
    .def("__call__", &lennard_jones::operator())
    .def("V0", &lennard_jones::V0)
    .def("r0", &lennard_jones::r0);

  py::class_<hard_sphere_potential>(m, "hard_sphere_potential")
    .def(py::init<double, double>(), py::arg("V0"), py::arg("r0") = 1.0)
    .def("__call__", &hard_sphere_potential::operator())
    .def("V0", &hard_sphere_potential::V0)
    .def("r0", &hard_sphere_potential::r0);

  py::class_<inverse_square>(m, "inverse_square")
    .def(py::init<double>(), py::arg("V0"))
    .def("__call__", &inverse_square::operator())
    .def("V0", &inverse_square::V0);

  // Cross sections: one binding per potential type
  bind_cross_section<hard_sphere_potential>(m, "CrossSection_hard_sphere_potential");
  bind_cross_section<lennard_jones>(m, "CrossSection_lennard_jones");
  bind_cross_section<inverse_square>(m, "CrossSection_inverse_square");

  // To add your own potential "my_potential" (defined in scattering.h):
  //   py::class_<my_potential>(m, "my_potential")
  //     .def(py::init<double>(), py::arg("V0"))
  //     .def("__call__", &my_potential::operator());
  //   bind_cross_section<my_potential>(m, "CrossSection_my_potential");
}
