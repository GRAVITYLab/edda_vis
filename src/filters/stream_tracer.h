#ifndef COMPUTE_FIELD_LINE_H
#define COMPUTE_FIELD_LINE_H

///
/// \addtogroup filters
///

#include <vector>
#include <cassert>
#include <list>
#include <memory>

#include <common.h>
#include <core/vector_matrix.h>
#include <distributions/distribution.h>
#include <dataset/dataset.h>

namespace edda {

enum TraceDir{TD_FORWARD, TD_BACKWARD};
enum TraceOrd{TO_EULER=0, TO_RK2, TO_RK4, TO_RK45};

///
struct GetPositionAsIs {
    template<class T>
    inline VECTOR3 getPosition(const T &p) const {return static_cast<VECTOR3>( p );}  // Use underlying casting
};

/// \brief Get the position from mean of the distribution
struct GetPositionFromDistributionMean {
    template<class T>
    inline VECTOR3 getPosition(const T &p) const {return edda::dist::getMean( p );}
};

///
/// \brief Uncertain particle tracing.
///
/// Supports Euler and RK2.
/// DataType: Return type of the dataset
/// GetPositionPolicy: We provide policy during advection to obtain a representative direction for uncertain particle locations.
///
template<typename DataType, class GetPositionPolicy = GetPositionAsIs>
class StreamTracer: public GetPositionPolicy{
    std::shared_ptr<Dataset<DataType> > dataset;
public:
    int max_steps = 1000;
    float step_size = .5f;
    TraceDir dir = TD_FORWARD;
    TraceOrd ord = TO_RK2;

    StreamTracer(std::shared_ptr<Dataset<DataType> > &dataset_)
        : dataset(dataset_)
    {
    }

    ReturnStatus euler(
        DataType &pos,
        float dt)
    {
        ReturnStatus r;
        DataType vec;
        r = dataset->at_phys( this->getPosition(pos), vec);
        if (r != SUCCESS)
            return r;
        pos += vec * dt;

        return SUCCESS;
    }

    ReturnStatus runge_kutta2(
        DataType &pos,
        float dt)
    {
        ReturnStatus r;
        DataType vec;
        DataType k1, k2;

        // 1st step of Runge-Kutta
        r = dataset->at_phys( this->getPosition(pos), vec);
        if (r != SUCCESS)
            return r;
        k1 = vec * dt;

        // 2nd step or Runge Kutta
        r = dataset->at_phys( this->getPosition(pos + k1 * .5f), vec);
        if (r != SUCCESS)
            return r;

        k2 = vec * dt;
        pos += k2;

        return SUCCESS;
    }

    ///
    /// \brief Start particle tracing
    /// \param seeds Initial particle locations
    /// \param[out] traces Results.
    /// \return SUCCESS or OUT_OF_BOUND
    ///
    ReturnStatus compute(
            const std::list<DataType> &seeds,
            std::list<std::list<DataType> > &traces
        )
    {
        traces.clear();
        typename std::list<DataType>::const_iterator seedItr = seeds.begin();
        for (int sid = 0; seedItr != seeds.end(); ++seedItr, ++sid)
        {
            DataType p = *seedItr;
            std::list<DataType> singleTrace;

            singleTrace.push_back(p);
            for (int i=0; i< max_steps; i++)
            {
                ReturnStatus r;
                switch (ord) {
                case TO_EULER:
                    r = euler(p, step_size);
                    break;
                case TO_RK2:
                    r = runge_kutta2(p, step_size);
                    break;
                default:
                    r = ReturnStatus::FAIL;
                    assert(false); // should not go here
                    break;
                }
                if (r != SUCCESS)
                    break;
                singleTrace.push_back(p);
            }
            traces.push_back(singleTrace);

        }
        return SUCCESS;
    }

};


} // namespace edda

#endif // COMPUTE_FIELD_LINE_H
