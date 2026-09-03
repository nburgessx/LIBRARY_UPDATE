
#include "stdafx.h"
#include "test_functions.h"

#include <vector>
#include "MBTemplateType.h"  
#include "MMAnalyticFormula.h"


using namespace std;

double doSomeFormula(const double z) {
	//return z*z;
  return AnalyticFormulae::diffNormdist(z);
}


