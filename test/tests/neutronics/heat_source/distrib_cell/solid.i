[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = ../stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  solid_blocks = '100'
  fluid_blocks = '200'
  tally_blocks = '100 200'
  tally_name = heat_source
  tally_type = cell
  check_zero_tallies = false

  # This turns off the density and temperature update on the first syncSolutions;
  # this uses whatever temperature and densities are set in OpenMCs XML files for first step
  initial_properties = xml

  verbose = true
  fluid_cell_level = 0
  solid_cell_level = 0
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'density'
[]
