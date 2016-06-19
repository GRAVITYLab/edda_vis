#include <iostream>
#include <string>
#include <cstdio>
#include "edda.h"
#include "distributions/gaussian.h"
#include "distributions/distribution.h"
#include "io/file_reader.h"
#include "filters/stream_tracer.h"
#include "dataset/dataset.h"
#include "io/edda_reader.h"

using namespace std;
using namespace edda;

int main(int argc, char **argv)
{
  srand(time(NULL));  // random seeding

  cout << "Loading sample file" << endl;
  string filename = SAMPLE_DATA_PATH;
  filename = filename + "/isabel_pressure_small.vti";

  // load data with random sampling
  shared_ptr<Dataset<Real> > dataset = loadEddaScalarDataset(filename, "");

  VECTOR3 pos;
  Real value;

  pos = VECTOR3(10,10,10);
  dataset->at_phys(pos, value);
  cout << pos << ": " << value << endl;


  pos = VECTOR3(2.1,2.1,2.1);
  dataset->at_phys(pos, value);
  cout << pos << ": " << value << endl;

  return 0;
}
