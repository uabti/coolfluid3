// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/LineSolutionP0.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LineSolutionP0, Mesh::ShapeFunction, LibSF > LineSolutionP0_Builder;

////////////////////////////////////////////////////////////////////////////////

LineSolutionP0::LineSolutionP0(const std::string& name) : Mesh::ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void LineSolutionP0::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void LineSolutionP0::compute_gradient(const MappedCoordsT& mappedCoord, GradientT& result)
{
  result(KSI, 0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix LineSolutionP0::s_mapped_sf_nodes =  ( RealMatrix(1,1) <<
    0.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF