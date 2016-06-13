#ifndef RANDOM_SAMPLE_FIELD
#define RANDOM_SAMPLE_FIELD

#include <thrust/iterator/zip_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include "common.h"
#include "core/thrust_common.h"
#include "distributions/variant.h"
#include "distributions/gaussian_mixture.h"

namespace edda {

namespace detail {
    struct GetSample {
      ///
      /// \param tuple0 distribution
      /// \param tuple1 Thrust random engine
      ///
      template <class Dist> //, ENABLE_IF_BASE_OF(Dist, dist::Distribution)>
      __host__ __device__
      double operator() (thrust::tuple<Dist, thrust::default_random_engine> &tuple)
      {
        Dist &dist = thrust::get<0>(tuple);
        return dist::getSample(dist, thrust::get<1>(tuple));
      }

      template <class Dist> //, ENABLE_IF_BASE_OF(Dist, dist::Distribution)>
      __host__ __device__
      VECTOR3 operator() (thrust::tuple<Tuple3<Dist>, thrust::default_random_engine> &tuple)
      {
        Tuple3<Dist> &distvec = thrust::get<0>(tuple);
        thrust::default_random_engine &randEng = thrust::get<1>(tuple);
        return VECTOR3(dist::getSample(distvec, randEng),
                       dist::getSample(distvec, randEng),
                       dist::getSample(distvec, randEng));
      }
    };
} // namespace detail

///
/// Random sampling an array. Output iterator must point to elements in type 'Real'
///
template <class InputIterator, class OutputIterator>
void randomSampleField(InputIterator begin, InputIterator end, OutputIterator out)
{
  static int seed ;
  seed += time(NULL) % 10000;
  int n = end-begin;
  thrust::transform( thrust::make_zip_iterator(thrust::make_tuple(begin, randomEngineIterator(seed)) ) ,
                     thrust::make_zip_iterator(thrust::make_tuple(end, randomEngineIterator(seed+n)) ),
                     out, detail::GetSample()) ;
  seed += n;
}


} // edda

#endif // RANDOM_SAMPLE_FIELD
