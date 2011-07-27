// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"

#include "SetupMultipleSolutions.hpp"


using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SetupMultipleSolutions, CAction, LibRDM > SetupMultipleSolutions_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SetupMultipleSolutions::SetupMultipleSolutions ( const std::string& name ) : CF::Solver::Action(name)
{
  // options

  m_options.add_option< OptionT<Uint> >( "nb_levels", 1u )
      ->description("Number of solution levels to be created")
      ->pretty_name("Number of levels");
}

void SetupMultipleSolutions::execute()
{
  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();

  const Uint nb_levels = option("nb_levels").value<Uint>();

  CMesh& mesh = *m_mesh.lock();

  CGroup& fields = mysolver.fields();

  // construct vector of variables

  const Uint nbdofs = physical_model().neqs();

  std::string vars;
  for(Uint i = 0; i < nbdofs; ++i)
  {
   vars += "u" + to_str(i) + "[1]";
   if( i != nbdofs-1 ) vars += ",";
  }

  // configure 1st solution

  std::vector< CField::Ptr > solution_fields;

  CField::Ptr solution = find_component_ptr_with_tag<CField>( mesh, RDM::Tags::solution() );
  if ( is_null( solution ) )
  {
    solution =
        mesh.create_field( RDM::Tags::solution() + to_str(0) ,CField::Basis::POINT_BASED,"space[0]",vars).as_ptr<CField>();
    solution->add_tag(Tags::solution());
  }

  solution_fields.push_back(solution);

  // create the other solutions based on the first solution field

  for(Uint k = 1; k < nb_levels; ++k)
  {
    CField::Ptr solution_k = find_component_ptr_with_tag<CField>( mesh, RDM::Tags::solution() + to_str(k));
    if ( is_null( solution_k ) )
    {
      solution_k = mesh.create_field(Tags::solution() + to_str(k), *solution ).as_ptr<CField>();
      solution_k->add_tag(Tags::solution());
    }

    solution_fields.push_back(solution_k);
  }

  for( Uint k = 0; k < solution_fields.size(); ++k)
  {
    if( ! fields.get_child_ptr( solution_fields[k]->name() ) )
      fields.create_component<CLink>( solution_fields[k]->name() ).link_to(solution).add_tag(RDM::Tags::solution());
  }


  /// @todo here we should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

  // configure residual

  CField::Ptr residual = find_component_ptr_with_tag<CField>( mesh, RDM::Tags::residual());
  if ( is_null( residual ) )
  {
    residual = mesh.create_field(Tags::residual(), *solution ).as_ptr<CField>();
    residual->add_tag(Tags::residual());
  }

  if( ! fields.get_child_ptr( RDM::Tags::residual() ) )
    fields.create_component<CLink>( RDM::Tags::residual()   ).link_to(residual).add_tag(RDM::Tags::residual());

  // configure wave_speed

  CField::Ptr wave_speed = find_component_ptr_with_tag<CField>( mesh, RDM::Tags::wave_speed());
  if ( is_null( wave_speed ) )
  {
    wave_speed = mesh.create_scalar_field(Tags::wave_speed(), *solution).as_ptr<CField>();
    wave_speed->add_tag(Tags::wave_speed());
  }

  if( ! fields.get_child_ptr( RDM::Tags::wave_speed() ) )
    fields.create_component<CLink>( RDM::Tags::wave_speed() ).link_to(wave_speed).add_tag(RDM::Tags::wave_speed());


  // configure phi_k

  CField::Ptr phi_k = find_component_ptr_with_tag<CField>( mesh, "phi_k");
  if ( is_null( phi_k ) )
  {
    std::string lvars;
    for(Uint k = 0; k < nb_levels; ++k)
      for(Uint i = 0; i < nbdofs; ++i)
      {
        lvars += "u" + to_str(k) + "_"  + to_str(i) + "[1]";
        if( i != nbdofs*nb_levels-1 ) lvars += ",";
      }

    phi_k = mesh.create_field( "phi_k", CField::Basis::CELL_BASED, "space[0]", vars ).as_ptr<CField>();
    phi_k->add_tag("phi_k");
  }


  /// @todo apply here the bubble insertion if needed

  // parallelize the solution if not yet done


  PECommPattern& pattern = solution->parallelize();

  std::vector<URI> parallel_fields;
  parallel_fields.push_back( solution->uri() );

  for(Uint k = 1; k < nb_levels; ++k)
  {
    solution_fields[k]->parallelize_with( pattern );
    parallel_fields.push_back( solution_fields[k]->uri() );
  }

  mysolver.actions().get_child("Synchronize").configure_option("Fields", parallel_fields);

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
