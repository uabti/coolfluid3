// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"

#include "Math/Checks.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/IterativeSolver.hpp"

#include "RK.hpp"


using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Math::Checks;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < RK, CAction, LibRDM > RK_Builder;

///////////////////////////////////////////////////////////////////////////////////////

RK::RK ( const std::string& name ) :
  CF::Solver::Action(name)
{
  mark_basic();

  m_options.add_option< OptionT<Real> >( "cfl", 1.0 )
      ->pretty_name("CFL")
      ->description("Courant-Fredrichs-Levy stability number");

  m_options.add_option< OptionT<Real> >( "rkorder", 1u )
      ->pretty_name("RK Order")
      ->description("Order of the Runge-Kutta step");

}


void RK::execute()
{
  RDSolver& mysolver = solver().as_type< RDSolver >();

  if (m_solution.expired())
    m_solution = mysolver.fields().get_child( RDM::Tags::solution() ).follow()->as_ptr_checked<CField>();
  if (m_wave_speed.expired())
    m_wave_speed = mysolver.fields().get_child( RDM::Tags::wave_speed() ).follow()->as_ptr_checked<CField>();
  if (m_residual.expired())
    m_residual = mysolver.fields().get_child( RDM::Tags::residual() ).follow()->as_ptr_checked<CField>();

  CTable<Real>& solution     = m_solution.lock()->data();
  CTable<Real>& wave_speed   = m_wave_speed.lock()->data();
  CTable<Real>& residual     = m_residual.lock()->data();

  const Real CFL = options().option("cfl").value<Real>();

  const Uint k = mysolver.iterative_solver().properties().value<Uint>("iteration");

  // implementation of the RungeKutta update step

//--
  const Uint nbdofs = solution.size();
  const Uint nbvars = solution.row_size();
  for ( Uint i=0; i< nbdofs; ++i )
  {
    if ( is_zero(wave_speed[i][0]) )
    {
      for ( Uint j=0; j< nbvars; ++j )
        if( is_not_zero(residual[i][j]) )
          CFwarn << "residual not null but wave_speed null at node [" << i << "] variable [" << j << "]" << CFendl;
      continue;
    }

    for ( Uint j=0; j< nbvars; ++j )
      solution[i][j] += - update * residual[i][j];
  }

}
//--
////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
