/**
 * @file module.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace py = pybind11;

namespace dunedaq::dal::python {

extern void
register_dal_methods(py::module&);

PYBIND11_MODULE(_daq_dunedaqdal_py, m)
{

  m.doc() = "C++ implementation of the dunedaqdal modules";

  register_dal_methods(m);
}

} // namespace dunedaq::dunedaqdal::python
