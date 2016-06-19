#ifndef EDDA_READER
#define EDDA_READER

#include <string>
#include "dataset/dataset.h"

namespace edda{


  std::shared_ptr<Dataset<Real> > loadEddaScalarDataset(const std::string &edda_file, const std::string &array_name="");
  std::shared_ptr<Dataset<VECTOR3> > loadEddaVector3Dataset(const std::string &edda_file, const std::string &array_name="");

}

#endif
