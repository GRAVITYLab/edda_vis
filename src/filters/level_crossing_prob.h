#ifndef LEVEL_CROSSING_PROB_H
#define LEVEL_CROSSING_PROB_H

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
  template<class RandomIterator>
  class LevelCrossingFunctor {
    int dim[3];
    int celldim[3];
    const double isov;
    const RandomIterator dataIterator;
  public:
    LevelCrossingFunctor(const RandomIterator &dataIterator_, int dim_[3], double isov_)
      : dataIterator(dataIterator_), isov(isov_)
    {
      dim[0] = dim_[0];
      dim[1] = dim_[1];
      dim[2] = dim_[2];
      celldim[0] = dim[0]-1;
      celldim[1] = dim[1]-1;
      celldim[2] = dim[2]-1;
    }


    __host__ __device__
    float operator() (int idx) {
      int i = idx % celldim[0];
      int j = (idx / celldim[0]) % celldim[1];
      int k = idx / celldim[0] / celldim[1];
      float cdf[8];
  #define IJK_TO_IDX(i,j,k)  (i+dim[0]*(j+dim[1]*(k)))
      cdf[0] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i  ,j  ,k  )),  isov);
      cdf[1] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i+1,j  ,k  )),  isov);
      cdf[2] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i  ,j+1,k  )),  isov);
      cdf[3] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i+1,j+1,k  )),  isov);
      cdf[4] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i  ,j  ,k+1)),  isov);
      cdf[5] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i+1,j  ,k+1)),  isov);
      cdf[6] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i  ,j+1,k+1)),  isov);
      cdf[7] = dist::getCdf( *(dataIterator+ IJK_TO_IDX(i+1,j+1,k+1)),  isov);
  #undef IJK_TO_IDX
      float prob1=1., prob2=1.;
      for (int l=0; l<8; l++) {
        prob1 *= cdf[l];
        prob2 *= 1.-cdf[l];
      }
      return 1.-prob1-prob2;
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
template<class RandomIterator>
ReturnStatus levelCrossingProb(RandomIterator dataIterator, int dim[3],
                           double isov, std::shared_ptr<NdArray<Real> > &probField)
{
  int new_dim[3]={dim[0]-1, dim[1]-1, dim[2]-1};
  int newsize = new_dim[0]*new_dim[1]*new_dim[2];

  probField.reset( new NdArray<Real>(3, new_dim) );

  thrust::transform( thrust::make_counting_iterator(0),
                     thrust::make_counting_iterator(newsize),
                     probField->begin(),
                     detail::LevelCrossingFunctor<RandomIterator>( dataIterator, dim, isov)
                     );

  return ReturnStatus::SUCCESS;
}

} // namespace edda
#endif // LEVEL_CROSSING_PROB_H
