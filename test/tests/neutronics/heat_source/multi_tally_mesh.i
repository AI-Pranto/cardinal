[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
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

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  particles = 1000.0
  solid_blocks = '100'
  verbose = true
  solid_cell_level = 0

  tally_type = mesh
  mesh_template = ../meshes/sphere.e
  mesh_translations = '0 0 0
                       0 0 4
                       0 0 8'
  check_tally_sum = false

  tally_score = 'kappa_fission heating'

  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [heating]
    type = ElementIntegralVariablePostprocessor
    variable = heating
  []
[]

[Outputs]
  csv = true
[]
