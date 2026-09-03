#pragma once

#include <iostream>
#include <cassert>
#include <sstream>
#include <float.h> 
#include <limits>
#include <vector>

#ifndef DBL_EPSILON
#define DBL_EPSILON     2.2204460492503131e-016 /* smallest such that 1.0+DBL_EPSILON != 1.0 */
#endif
#ifndef DBL_DIG
#define DBL_DIG         15                      /* # of decimal digits of precision */
#endif

#ifndef ASSERT
#define ASSERT assert
#endif

#if !defined (WIN32) || !defined (WIN64)
#include <cmath>
#define _isnan std::isnan
#endif

void optionalDebugOutput(char * message);

namespace ieee {
	const double  NaN = std::numeric_limits< double >::quiet_NaN();
	bool isNumber(double  d);
} // namespace ieee

namespace Abort {
	bool abortHasBeenRequested();
} // namespace Abort

inline void optionalDebugOutput(char * message) {
	std::cerr << message << std::endl;
}

inline bool ieee::isNumber(double  d) {
	return !_isnan(d);
}

inline bool Abort::abortHasBeenRequested() {
	return false;
}

class AQLFunctionVector {
protected:
   std::vector <double > m_workSpaceForArgumentVector, m_workSpaceForFunctionVector;
public:

   virtual void operator()(std::vector<double > & f, const std::vector<double > & x) = 0;

   /// By default, the request for the Jacobian uses the above centre differencing method.
   virtual void operator()(std::vector<std::vector<double > > &jacobian, const std::vector <double > &x);

   /// By default, the combined request for the Jacobian and the function vector delegates to the individual requests for function vector and jacobian.
   virtual void operator()(std::vector<std::vector<double > > &jacobian, std::vector<double > &f, const std::vector <double > &x);

   virtual ~AQLFunctionVector();
   virtual unsigned long lengthOfArgumentVector() = 0;
   virtual unsigned long lengthOfFunctionVector() = 0;

   /// This function can request an unconditional aborting of the solver algorithm.
   virtual bool stopNow();

   virtual double  finiteDifferencingWidth();

   ///
   /// Iterative solvers may need to know when progress in x is no longer worth it.
   ///
   virtual double  argumentTolerance();

   ///
   /// Iterative solvers may need to know when progress in f(x) is no longer worth it.
   ///
   virtual double  functionTolerance();

   ///
   /// Iterative solvers need to know when to stop.
   ///
   virtual unsigned long maximumNumberOfIterations();

   // In many cases, just doing numerical centre differencing is good enough, and in many
   // other cases, it is good to test your results of having provided the Jacobian analytically
   // against this numerical approximation of the Jacobian.
   void setJacobianByCentreDifferencing(std::vector<std::vector<double > >&jacobian, const std::vector <double > &x);


   //	Convenience function. Very often the constraint is that all x must be nonnegative.
   bool anyNegative(const std::vector <double > &x);
   
   //	Convenience function. Very often the constraint is that all x must be positive.
   bool anyNonPositive(const std::vector <double > &x);
   
   //	Convenience function. Check if a numerical error has occurred.
   bool anyNotANumber(const std::vector <double > &x);
   
   
   /*The fitting procedure will attempt to honour your request to disallow certain parameter areas.
   the method constraintsAreViolated() should return true if the parameter vector is to be disallowed.
   By default, we only check that all numbers are still valid numerical values.*/
   virtual bool constraintsAreViolated(const std::vector <double > &x);
};

