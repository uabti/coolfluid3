// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::Tecplot::Writer"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/DynTable.hpp"
#include "mesh/List.hpp"
#include "mesh/Table.hpp"
#include "mesh/Geometry.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct TecWriterTests_Fixture
{
  /// common setup for each test case
  TecWriterTests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TecWriterTests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TecWriterTests_TestSuite, TecWriterTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh )
{

  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.Neu.Reader","meshreader");

  meshreader->configure_option("read_groups",true);

  // the mesh to store in
  Mesh& mesh = Core::instance().root().create_component<Mesh>( "mesh" );

  meshreader->read_mesh_into("quadtriag.neu",mesh);


  Uint nb_ghosts=0;


  Field& nodal = mesh.geometry().create_field("nodal","nodal[vector]");
  for (Uint n=0; n<nodal.size(); ++n)
  {
    for(Uint j=0; j<nodal.row_size(); ++j)
      nodal[n][j] = n;
  }


  FieldGroup& elems = mesh.create_space_and_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED,"cf3.mesh.LagrangeP0");

  Field& cell_centred = elems.create_field("cell_centred","cell_centred[vector]");
  for (Uint e=0; e<cell_centred.size(); ++e)
  {
    for(Uint j=0; j<cell_centred.row_size(); ++j)
      cell_centred[e][j] = e;
  }


  FieldGroup& P2 = mesh.create_space_and_field_group("nodes_P2",FieldGroup::Basis::POINT_BASED,"cf3.mesh.LagrangeP2");

  Field& nodesP2 = P2.create_field("nodesP2","nodesP2[vector]");
  for (Uint e=0; e<nodesP2.size(); ++e)
  {
    for(Uint j=0; j<nodesP2.row_size(); ++j)
      nodesP2[e][j] = nodesP2.coordinates()[e][j];
  }


  std::vector<Field::Ptr> fields;
  fields.push_back(nodal.as_ptr<Field>());
  fields.push_back(cell_centred.as_ptr<Field>());
  fields.push_back(nodesP2.as_ptr<Field>());
  MeshWriter::Ptr tec_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.Tecplot.Writer","meshwriter");
  tec_writer->configure_option("cell_centred",true);
  tec_writer->set_fields(fields);
  tec_writer->write_from_to(mesh,"quadtriag.plt");

  BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( threeD_test )
{

  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.Neu.Reader","meshreader");

  meshreader->configure_option("number_of_processors",(Uint) Comm::instance().size());
  meshreader->configure_option("rank",(Uint) Comm::instance().rank());
  meshreader->configure_option("Repartition",false);
  meshreader->configure_option("OutputRank",(Uint) 2);

  // the file to read from
  boost::filesystem::path fp_in ("hextet.neu");

  // the mesh to store in
  Mesh::Ptr mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);
  meshreader->do_read_mesh_into(fp_in,mesh);
  CFinfo.setFilterRankZero(true);

  boost::filesystem::path fp_out ("hextet.msh");
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK(true);

}
*/
////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( read_multiple_2D )
{

  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.Neu.Reader","meshreader");

  meshreader->configure_option("Repartition",true);
  meshreader->configure_option("OutputRank",(Uint) 0);

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  Mesh::Ptr mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);



  for (Uint count=1; count<=2; ++count)
  {
    CFinfo << "\n\n\nMesh parallel:" << CFendl;
    meshreader->do_read_mesh_into(fp_in,mesh);
  }



  CFinfo.setFilterRankZero(true);
  CFinfo << mesh->tree() << CFendl;
  CFinfo << meshreader->tree() << CFendl;
  MeshTransformer::Ptr info  = build_component_abstract_type<MeshTransformer>("Info","info");
  info->transform(mesh);


  boost::filesystem::path fp_out ("quadtriag_mult.msh");
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK_EQUAL(1,1);

}
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

