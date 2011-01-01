// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstddef>
#include <iostream>
#include <ostream>

#include <dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/geometrytype.hh>

#include <dune/grid/genericgeometry/geometry.hh>
#include <dune/grid/genericgeometry/geometrytraits.hh>

#include <dune/localfunctions/lagrange/q1.hh>

#include "test-fe.hh"

int main(int argc, char** argv) {
  try {
    // tolerance for floating-point comparisons
    static const double eps = 1e-9;
    // stepsize for numerical differentiation
    static const double delta = 1e-5;

    int result = 77;

    { // 1D
      std::cout << "== Checking global-valued Q1 elements (with dim=1)"
                << std::endl;

      Dune::GeometryType gt;
      gt.makeLine();

      Dune::FieldVector<double, 1> corners[2];
      corners[0][0] = -.5;
      corners[1][0] =  .5;
      typedef Dune::GenericGeometry::BasicGeometry<
          1, Dune::GenericGeometry::DefaultGeometryTraits<double, 1, 1>
          > Geometry;
      Geometry geo(gt, corners);

      Dune::Q1FiniteElementFactory<Geometry, double> feFactory;
      bool success = testFE(geo, feFactory.make(geo), eps, delta);

      if(success && result != 1)
        result = 0;
      else
        result = 1;
    }

    { // 2D
      std::cout << "== Checking global-valued Q1 elements (with dim=2)"
                << std::endl;

      Dune::GeometryType gt;
      gt.makeQuadrilateral();

      Dune::FieldVector<double, 2> corners[4];
      corners[0][0] = -.5; corners[0][1] = 0;
      corners[1][0] = 0  ; corners[1][1] = -.5;
      corners[2][0] =  .5; corners[2][1] = 0;
      corners[3][0] = 0  ; corners[3][1] =  .5;
      typedef Dune::GenericGeometry::BasicGeometry<
          2, Dune::GenericGeometry::DefaultGeometryTraits<double, 2, 2>
          > Geometry;
      Geometry geo(gt, corners);

      Dune::Q1FiniteElementFactory<Geometry, double> feFactory;
      bool success = testFE(geo, feFactory.make(geo), eps, delta);

      if(success && result != 1)
        result = 0;
      else
        result = 1;
    }

    { // 3D
      std::cout << "== Checking global-valued Q1 elements (with dim=3)"
                << std::endl;

      Dune::GeometryType gt;
      gt.makeHexahedron();

      Dune::FieldVector<double, 3> corners[8];
      corners[0][0] = -.7; corners[0][1] = -.6; corners[0][2] = -.5;
      corners[1][0] =  .4; corners[1][1] = -.3; corners[1][2] = -.7;
      corners[2][0] = -.6; corners[2][1] =  .5; corners[2][2] = -.4;
      corners[3][0] =  .3; corners[3][1] =  .7; corners[3][2] = -.6;
      corners[4][0] = -.5; corners[4][1] = -.4; corners[4][2] =  .3;
      corners[5][0] =  .7; corners[5][1] = -.6; corners[5][2] =  .5;
      corners[6][0] = -.4; corners[6][1] =  .3; corners[6][2] =  .7;
      corners[7][0] =  .6; corners[7][1] =  .5; corners[7][2] =  .4;
      typedef Dune::GenericGeometry::BasicGeometry<
          3, Dune::GenericGeometry::DefaultGeometryTraits<double, 3, 3>
          > Geometry;
      Geometry geo(gt, corners);

      Dune::Q1FiniteElementFactory<Geometry, double> feFactory;
      bool success = testFE(geo, feFactory.make(geo), eps, delta);

      if(success && result != 1)
        result = 0;
      else
        result = 1;
    }

    return result;
  }
  catch (const Dune::Exception& e) {
    std::cerr << e << std::endl;
    throw;
  }
}