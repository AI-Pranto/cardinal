/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#pragma once

#define LIBMESH

#include "ExternalProblem.h"
#include "PostprocessorInterface.h"
#include "CardinalEnums.h"
#include "openmc/tallies/tally.h"

/**
 * Base class for all MOOSE wrappings of OpenMC
 */
class OpenMCProblemBase : public ExternalProblem, public PostprocessorInterface
{
public:
  OpenMCProblemBase(const InputParameters & params);

  static InputParameters validParams();

  virtual ~OpenMCProblemBase() override;

  /**
   * Convert from a MooseEnum for a trigger metric to an OpenMC enum
   * @param[in] trigger trigger metric
   * @return OpenMC enum
   */
  openmc::TriggerMetric triggerMetric(tally::TallyTriggerTypeEnum trigger) const;

  /**
   * Convert from a MooseEnum for tally estimator to an OpenMC enum
   * @param[in] estimator MOOSE estimator enum
   * @return OpenMC enum
   */
  openmc::TallyEstimator tallyEstimator(tally::TallyEstimatorEnum estimator) const;

  /**
   * Check whether the user has already created a variable using one of the protected
   * names that the OpenMC wrapping is using.
   * @param[in] name variable name
   */
  void checkDuplicateVariableName(const std::string & name) const;

  /// Run a k-eigenvalue OpenMC simulation
  void externalSolve() override;

  /// Import temperature and density from a properties.h5 file
  void importProperties() const;

  /**
   * \brief Compute the sum of a tally within each bin
   *
   * For example, suppose we have a cell tally with 4 bins, one for each of 4
   * different cells. This function will return the sum of the tally in each of
   * those bins, so the return xtensor will have a length of 4, with each value
   * representing the sum for that bin.
   *
   * @param[in] tally OpenMC tally
   * @param[in] score tally score
   * @return tally sum within each bin
   */
  xt::xtensor<double, 1> tallySum(openmc::Tally * tally, const unsigned int & score) const;

  /**
   * Compute the sum of a tally across all of its bins
   * @param[in] tally OpenMC tallies (multiple if repeated mesh tallies)
   * @param[in] score tally score
   * @return tally sum
   */
  double tallySumAcrossBins(std::vector<openmc::Tally *> tally, const unsigned int & score) const;

  /**
   * Compute the mean of a tally across all of its bins
   * @param[in] tally OpenMC tallies (multiple if repeated mesh tallies)
   * @param[in] score tally score
   * @return tally mean
   */
  double tallyMeanAcrossBins(std::vector<openmc::Tally *> tally, const unsigned int & score) const;

  /**
   * Type definition for storing the relevant aspects of the OpenMC geometry; the first
   * value is the cell index, while the second is the cell instance.
   */
  typedef std::pair<int32_t, int32_t> cellInfo;

  /**
   * Get the material name given its index. If the material does not have a name,
   * return the ID.
   * @param[in] index
   * @return material name
   */
  std::string materialName(const int32_t index) const;

  /**
   * Compute relative error
   * @param[in] sum sum of scores
   * @param[in] sum_sq sum of scores squared
   * @param[in] n_realizations number of realizations
   */
  xt::xtensor<double, 1> relativeError(const xt::xtensor<double, 1> & sum,
    const xt::xtensor<double, 1> & sum_sq, const int & n_realizations) const;

  /**
   * Get the density conversion factor (multiplicative factor)
   * @return density conversion factor from kg/m3 to g/cm3
   */
  const Real & densityConversionFactor() const { return _density_conversion_factor; }

  /**
   * Get the number of particles used in the current Monte Carlo calculation
   * @return number of particles
   */
  int nParticles() const;

  /**
   * Get the cell ID from the cell index
   * @param[in] index cell index
   * @return cell ID
   */
  int32_t cellID(const int32_t index) const;

  /**
   * Get the material ID from the material index
   * @param[in] index material index
   * @return cell material ID
   */
  int32_t materialID(const int32_t index) const;

  /**
   * Print point coordinates with a neater formatting than the default MOOSE printing
   * @return formatted point string
   */
  std::string printPoint(const Point & p) const;

  /**
   * Get a descriptive, formatted, string describing a material
   * @param[in] index material index
   * @return descriptive string
   */
  std::string printMaterial(const int32_t & index) const;

  /**
   * Write the source bank to HDF5 for postprocessing or for use in subsequent solves
   * @param[in] filename file name
   */
  void writeSourceBank(const std::string & filename);

  /**
   * Get the path output
   * @return path output
   */
  std::string pathOutput() const { return _path_output; }

  /**
   * Get the total (i.e. summed across all ranks, if distributed)
   * number of elements in a given block
   * @param[in] block_id subdomainID
   * return number of elements in block
   */
  unsigned int numElemsInSubdomain(const SubdomainID & id) const;

  /**
   * Whether the element is owned by this rank
   * @return whether element is owned by this rank
   */
  bool isLocalElem(const Elem * elem) const;

  /**
   * Get the global element ID from the local element ID
   * @param[in] id local element ID
   * @return global element ID
   */
  unsigned int globalElemID(const unsigned int & id) const { return _local_to_global_elem[id]; }

  /**
   * Set the cell temperature, and print helpful error message if a failure occurs; this sets
   * the temperature for the id + instance, which could be one of N cells filling the 'cell_info'
   * parent cell (which is what we actually use for error printing)
   * @param[in] id cell ID
   * @param[in] instance cell instance
   * @param[in] T temperature
   * @param[in] cell_info cell info for which we are setting interior temperature, for error printing
   */
  virtual void setCellTemperature(const int32_t & id, const int32_t & instance, const Real & T,
    const cellInfo & cell_info) const;

  /**
   * Set the cell density, and print helpful error message if a failure occurs
   * @param[in] density density
   * @param[in] cell_info cell info for which we are setting the density
   */
  virtual void setCellDensity(const Real & density, const cellInfo & cell_info) const;

  /**
   * Get a descriptive, formatted, string describing a cell
   * @param[in] cell_info cell index, instance pair
   * @return descriptive string describing cell
   */
  virtual std::string printCell(const cellInfo & cell_info) const;

  /**
   * Get the fill of an OpenMC cell
   * @param[in] cell_info cell ID, instance
   * @param[out] fill_type fill type of the cell, one of MATERIAL, UNIVERSE, or LATTICE
   * @return indices of what is filling the cell
   */
  virtual std::vector<int32_t> cellFill(const cellInfo & cell_info, int & fill_type) const;

  /**
   * Whether the cell has a material fill (if so, then get the material index)
   * @param[in] cell_info cell ID, instance
   * @param[out] material_index material index in the cell
   * @return whether the cell is filled by a material
   */
  bool materialFill(const cellInfo & cell_info, int32_t & material_index) const;

  /**
   * Whether a cell contains any fissile materials; for now, this simply returns true for
   * cells filled by universes or lattices because we have yet to implement something more
   * sophisticated that recurses down into all the fills
   * @param[in] cell_info cell ID, instance
   * @return whether cell contains fissile material
   */
  virtual bool cellHasFissileMaterials(const cellInfo & cell_info) const;

protected:
  /**
   * Set an auxiliary elemental variable to a specified value
   * @param[in] var_num variable number
   * @param[in] elem_ids element IDs to set
   * @param[in] value value to set
   */
  void fillElementalAuxVariable(const unsigned int & var_num,
                                const std::vector<unsigned int> & elem_ids,
                                const Real & value);

  /**
   * Get name of source bank file to write
   * @param[out] file name
   */
  std::string sourceBankFileName() const
  {
    return _path_output + "initial_source_" + std::to_string(_fixed_point_iteration) + ".h5";
  }

  /// Whether to print diagnostic information about model setup and the transfers
  const bool & _verbose;

  /// Power by which to normalize the OpenMC results, for k-eigenvalue mode
  const Real * _power;

  /// Source strength by which to normalize the OpenMC results, for fixed source mode
  const Real * _source_strength;

  /**
   * Whether to take the starting fission source from iteration \f$n\f$ as the
   * fission source converged from iteration \f$n-1\f$. Setting this to true will
   * in most cases lead to improved convergence of the initial source as iterations
   * progress because you don't "start from scratch" each iteration and do the same
   * identical (within a random number seed) converging of the fission source.
   */
  bool _reuse_source;

  /**
   * Whether the OpenMC model consists of a single coordinate level; this can
   * in some cases be used for more verbose error messages
   */
  const bool _single_coord_level;

  /// Total number of unique OpenMC cell IDs + instances combinations
  long unsigned int _n_openmc_cells;

  /**
   * Fixed point iteration index used in relaxation; because we sometimes run OpenMC
   * in a pseudo-transient coupling with NekRS, we simply increment this by 1 each
   * time we call openmc::run(). This uses a zero indexing, so after the first iteration,
   * we have finished iteration 0, and so on.
   */
  int _fixed_point_iteration;

  /// Directory where OpenMC output files are written
  std::string _path_output;

  /**
   * Number of digits to use to display the cell ID for diagnostic messages; this is
   * estimated conservatively based on the total number of cells, even though there
   * may be distributed cells such that the maximum cell ID is far smaller than the
   * total number of cells.
   */
  const int _n_cell_digits;

  /// Mapping from local element indices to global element indices for this rank
  std::vector<unsigned int> _local_to_global_elem;

  /// Conversion unit to transfer between kg/m3 and g/cm3
  static constexpr Real _density_conversion_factor{0.001};

  /// ID used by OpenMC to indicate that a material fill is VOID
  static constexpr int MATERIAL_VOID{-1};
};
