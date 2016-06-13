#ifndef MC_STREAMLINE_H
#define MC_STREAMLINE_H
#include "random_sample_field.h"
#include "core/tuple.h"

namespace edda{

namespace detail{
  template <class FieldIterator>
  struct FieldInterp
  {
    Tuple3<int> dim;
    FieldIterator fieldBegin;
    FieldInterp (FieldIterator fieldBegin_, Tuple3<int> dim_)
      : fieldBegin(fieldBegin_), dim(dim_) { }
    template <class Dist> //, ENABLE_IF_BASE_OF(Dist, dist::Distribution)>
    __host__ __device__
    VECTOR3 operator() (thrust::tuple<Tuple3<Dist>, thrust::default_random_engine> &tuple)
    {
      Dist &dist = thrust::get<0>(tuple);
      return dist::getSample(dist, thrust::get<1>(tuple));
    }
  };

  template <class FieldIterator>
  struct MCStreamline_functor
  {
    FieldIterator fieldBegin;

    template <typename Dist>
    __host__ __device__
    double opeartor() (thrust::tuple<Dist, VECTOR3 )
  };
}

///
/// \brief  Monte-Carlo streamline computation
///
template <class FieldIterator, class ParticleIterator, class OutputIterator>
void mc_streamline(FieldIterator fieldBegin, FieldIterator fieldEnd,
                   ParticleIterator pBegin, ParticleIterator pEnd,
                   int steps,
                   OutputIterator out)
{
  static int rseed ; // random generator seeds
  rseed += time(NULL) % 10000;
  int particles = pEnd-pBegin;
  for (int i=0; i<steps; i++)
    thrust::transform( thrust::make_zip_iterator(thrust::make_tuple(pBegin, randomEngineIterator(rseed)) ) ,
                       thrust::make_zip_iterator(thrust::make_tuple(pEnd, randomEngineIterator(rseed+particles)) ),
                       out+i*particles,
                       detail::MCStreamline_functor<FieldIterator>(fieldBegin) ) ;

  //detail::GetSample_functor()
  seed += n;
}



} // edda

#endif // MC_STREAMLINE_H

