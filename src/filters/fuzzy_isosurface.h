#ifndef FUZZY_ISOSURFACE_H
#define FUZZY_ISOSURFACE_H

// Implementation of "Analysis of Large-Scale Scalar Data Using Hixels" by Thompson et al.

#include <memory>
#include <common.h>
#include <dataset/dataset.h>
#include <distributions/distribution.h>
#include <core/shared_ary.h>
#include <dataset/abstract_data_array.h>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include "core/ndarray.h"
#include "core/gmm_array.h"
#include "distributions/gaussian_mixture.h"

namespace edda{

namespace detail{

  ///
  /// The functor for computing level crossing probability for a cell
  ///
  class FuzzyIsocontour {
    const double isov;
  public:
    FuzzyIsocontour(double isov_)
      : isov(isov_)
    {}

    __host__ __device__
    template <typename Dist>
    float operator() (const Dist &distr) {
      double b = dist::getCdf(distr, isov);
      double a = 1 - b;
      double g;
      if (a==0)
        g = b;
      else if (b==0)
        g = a;
      else
        g = a/b - b/a;

      return g;
    }
  };
} // namespace detail



/// \breif Compute level crossing probablility.
///
/// \param dataIterator  a random-accessible iterator for the input array, in size dim[0]*dim[1]*dim[2]
/// \param isov   isovalue to query
/// \param[out] probField   output array.
///
///  Compute cell-wise probability of level crossing given distributions on the grid points.
/// The output will be allocated with size (dim[0]-1)*(dim[1]-1)*(dim[2]-1).
template<class Iterator>
ReturnStatus FuzzyIsosurface(Iterator dataIterator, int dim[3],
                           double isov, std::shared_ptr<NdArray<Real> > &probField)
{
  int size = dim[0]*dim[1]*dim[2];
  probField.reset( new NdArray<Real>(3, dim) );

  thrust::transform( dataIterator,
                     dataIterator + size,
                     probField->begin(),
                     detail::FuzzyIsocontour(isov)
                     );

  return ReturnStatus::SUCCESS;
}

} // namespace edda
#endif // LEVEL_CROSSING_PROB_H
