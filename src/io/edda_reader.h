#ifndef EDDA_READER
#define EDDA_READER

#include <string>
#include "edda_export.h"
#include "dataset/dataset.h"

namespace edda{


	std::shared_ptr<Dataset<Real> > EDDA_EXPORT loadEddaScalarDataset(const std::string &edda_file, const std::string &array_name = "");
	std::shared_ptr<Dataset<VECTOR3> > EDDA_EXPORT loadEddaVector3Dataset(const std::string &edda_file, const std::string &array_name = "");

}

#endif
