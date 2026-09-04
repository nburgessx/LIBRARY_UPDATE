/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLFunctionBase

*/
//  2006, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCompoundingFunc.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include <numeric>

using namespace std;
//================ AQLCompoundMethod ===================================
/*!
	@brief default constructor
*/
AQLCompoundMethod::AQLCompoundMethod() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQLCompoundMethod::~AQLCompoundMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLCompoundMethod::clone() const
{
    try 
	{
		return new AQLCompoundMethod(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLCompoundMethod::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLCompoundMethod::getType() const
{
	return FN_COMPOUNDING;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLCompoundMethod::operator()(const DoubleArray& x) const
{
	//calc compounding rate
	if (x.size() % 3 != 0)
		throw AQLCoreInvalidData("Compounidng params Error",__FILE__,__LINE__);

	// x[i] ... coupon
	// x[i+N] ... term
	// x[i+2N] ... margin
	unsigned int N = (x.size() / 3);
	double rate = 0.0;
	double margin = 0.0;
	double tau = 0.0;
	double compoundS = 0.0;
	double notcompoundS = 0.0;
	for (unsigned int i = 0; i < N; i++)
	{
		rate = x[i];
		tau = x[i+N];
		margin = x[i+2*N];

		double cpnr = rate * tau;
		double cpnm = margin * tau;

		compoundS *= getCompoundingCoeff(cpnr,cpnm);
		compoundS += getCompoundedVal(cpnr,cpnm);

		notcompoundS += getAdditionalVal(cpnr,cpnm);
	}

	double ret = compoundS + notcompoundS;
	return ret;
}

double 
AQLCompoundMethod::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}

double 
AQLCompoundMethod::getCompoundedVal(double coupon, double margin) const
{
	return coupon + margin;	
}

double 
AQLCompoundMethod::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}

//////////////AQLCompoundMethod1//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod1::AQLCompoundMethod1() 
: AQLCompoundMethod(){}

AQLCompoundMethod1::~AQLCompoundMethod1() {}

AQLCoreFunctionBase*	
AQLCompoundMethod1::clone() const	
{
    try 
	{
		return new AQLCompoundMethod1(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod1::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING1 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod1::getType() const
{
	return FN_COMPOUNDING1;
}
double 
AQLCompoundMethod1::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}
double 
AQLCompoundMethod1::getCompoundedVal(double coupon, double margin) const
{
	return coupon + margin;	
}
double 
AQLCompoundMethod1::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}


//////////////AQLCompoundMethod2//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod2::AQLCompoundMethod2() 
: AQLCompoundMethod(){}

AQLCompoundMethod2::~AQLCompoundMethod2() {}

AQLCoreFunctionBase*	
AQLCompoundMethod2::clone() const	
{
    try 
	{
		return new AQLCompoundMethod2(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod2::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING2 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod2::getType() const
{
	return FN_COMPOUNDING2;
}
double 
AQLCompoundMethod2::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon ; 
}
double 
AQLCompoundMethod2::getCompoundedVal(double coupon, double margin) const
{
	return coupon + margin;	
}
double 
AQLCompoundMethod2::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}

//////////////AQLCompoundMethod3//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod3::AQLCompoundMethod3() 
: AQLCompoundMethod(){}

AQLCompoundMethod3::~AQLCompoundMethod3() {}

AQLCoreFunctionBase*	
AQLCompoundMethod3::clone() const	
{
    try 
	{
		return new AQLCompoundMethod3(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod3::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING3 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod3::getType() const
{
	return FN_COMPOUNDING3;
}
double 
AQLCompoundMethod3::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon ; 
}
double 
AQLCompoundMethod3::getCompoundedVal(double coupon, double margin) const
{
	return coupon ;	
}
double 
AQLCompoundMethod3::getAdditionalVal(double coupon, double margin) const
{
	return margin;	
}

//////////////AQLCompoundMethod4//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod4::AQLCompoundMethod4() 
: AQLCompoundMethod(){}

AQLCompoundMethod4::~AQLCompoundMethod4() {}

AQLCoreFunctionBase*	
AQLCompoundMethod4::clone() const	
{
    try 
	{
		return new AQLCompoundMethod4(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod4::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING4 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod4::getType() const
{
	return FN_COMPOUNDING4;
}
double 
AQLCompoundMethod4::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}
double 
AQLCompoundMethod4::getCompoundedVal(double coupon, double margin) const
{
	return coupon ;	
}
double 
AQLCompoundMethod4::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}

//////////////AQLCompoundMethod5//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod5::AQLCompoundMethod5() 
: AQLCompoundMethod(){}

AQLCompoundMethod5::~AQLCompoundMethod5() {}

AQLCoreFunctionBase*	
AQLCompoundMethod5::clone() const	
{
    try 
	{
		return new AQLCompoundMethod5(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod5::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING5 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod5::getType() const
{
	return FN_COMPOUNDING5;
}
double 
AQLCompoundMethod5::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}
double 
AQLCompoundMethod5::getCompoundedVal(double coupon, double margin) const
{
	return coupon ;	
}
double 
AQLCompoundMethod5::getAdditionalVal(double coupon, double margin) const
{
	return margin;	
}

//////////////AQLCompoundMethod6//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod6::AQLCompoundMethod6() 
: AQLCompoundMethod(){}

AQLCompoundMethod6::~AQLCompoundMethod6() {}

AQLCoreFunctionBase*	
AQLCompoundMethod6::clone() const	
{
    try 
	{
		return new AQLCompoundMethod6(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod6::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING6 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod6::getType() const
{
	return FN_COMPOUNDING6;
}
double 
AQLCompoundMethod6::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0; 
}
double 
AQLCompoundMethod6::getCompoundedVal(double coupon, double margin) const
{
	return 0.0 ;	
}
double 
AQLCompoundMethod6::getAdditionalVal(double coupon, double margin) const
{
	return coupon + margin;	
}

//Compounding (ISDA)
//////////////AQLCompoundMethod7//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod7::AQLCompoundMethod7() 
: AQLCompoundMethod(){}

AQLCompoundMethod7::~AQLCompoundMethod7() {}

AQLCoreFunctionBase*	
AQLCompoundMethod7::clone() const	
{
    try 
	{
		return new AQLCompoundMethod7(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod7::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING7 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod7::getType() const
{
	return FN_COMPOUNDING7;
}

//Compounding (ISDA)
/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLCompoundMethod7::operator()(const DoubleArray& x) const
{
	//calc compounding rate
	if (x.size() % 3 != 0)
		throw AQLCoreInvalidData("Compounidng params Error",__FILE__,__LINE__);

	// x[i] ... coupon
	// x[i+N] ... term
	// x[i+2N] ... margin
	unsigned int N = (x.size() / 3);
	double rate = 0.0;
	double margin = 0.0;
	double tau = 0.0;
	double compoundS = 1.0;
	for (unsigned int i = 0; i < N; i++)
	{
		rate = x[i];
		tau = x[i+N];
		margin = x[i+2*N];
		double cpnr = rate * tau;
		double cpnm = margin * tau;
		compoundS *= 1.0 + cpnr + cpnm;
	}
	compoundS -= 1.0;

	double ret = compoundS;
	return ret;
}


//Flat Compounding (ISDA)
//////////////AQLCompoundMethod8//////////////////
/*!
	@brief default constructor
*/
AQLCompoundMethod8::AQLCompoundMethod8() 
: AQLCompoundMethod(){}

AQLCompoundMethod8::~AQLCompoundMethod8() {}

AQLCoreFunctionBase*	
AQLCompoundMethod8::clone() const	
{
    try 
	{
		return new AQLCompoundMethod8(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod8::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING8 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod8::getType() const
{
	return FN_COMPOUNDING8;
}

//Compounding (ISDA)
/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLCompoundMethod8::operator()(const DoubleArray& x) const
{
	//calc compounding rate
	if (x.size() % 3 != 0)
		throw AQLCoreInvalidData("Compounidng params Error",__FILE__,__LINE__);

	// x[i] ... coupon
	// x[i+n] ... term
	// x[i+2n] ... margin
	const size_t n = (x.size() / 3);
	DoubleVector coupons(n), terms(n), margins(n);
	copy(x.begin(),       x.begin() + n,   coupons.begin());
	copy(x.begin() + n,   x.begin() + 2*n, terms.begin());
	copy(x.begin() + 2*n, x.begin() + 3*n, margins.begin());

	DoubleVector v(n);
	double last_coupon = 0;
	for(size_t i = 0; i < n; i++){
		v[i] = terms[i] * (coupons[i] + margins[i]) + terms[i] * coupons[i] * last_coupon;
		last_coupon += v[i];
	}

	return accumulate(v.begin(), v.end(), 0.);
}




AQLCompoundMethod9::AQLCompoundMethod9() 
: AQLCompoundMethod(){}

AQLCompoundMethod9::~AQLCompoundMethod9() {}

AQLCoreFunctionBase*	
AQLCompoundMethod9::clone() const	
{
    try 
	{
		return new AQLCompoundMethod9(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod9::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING9 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod9::getType() const
{
	return FN_COMPOUNDING9;
}

double
AQLCompoundMethod9::operator()(const DoubleArray& x) const
{
	if(x.size() % 3 != 1){
		throw AQLCoreInvalidData("Compounidng params Error",__FILE__,__LINE__);
	}

	// x[i] ... coupon
	// x[i+N] ... term
	// x[i+2N] ... margin
	const size_t n = (x.size()-1) / 3;
	DoubleVector coupons(n), terms(n), margins(n);
	copy(x.begin(),       x.begin() + n,   coupons.begin());
	copy(x.begin() + n,   x.begin() + 2*n, terms.begin());
	copy(x.begin() + 2*n, x.begin() + 3*n, margins.begin());

	
	const double whole_term = x.back();
	double compounded_value = 1;
	for(size_t i = 0; i < n; i++){
		compounded_value *= (1 + coupons[i] * terms[i]);
	}
	compounded_value -= 1;


	return compounded_value + margins[0] * whole_term;
}




AQLCoreFunctionBase*	
AQLCompoundMethod10::clone() const	
{
    try 
	{
		return new AQLCompoundMethod10(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod10::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING10 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod10::getType() const
{
	return FN_COMPOUNDING10;
}

//Compounding (ISDA)
/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLCompoundMethod10::operator()(const DoubleArray& x) const
{
	if (x.size() % 3 != 1){
		throw AQLCoreInvalidData("Compounding params Error",__FILE__,__LINE__);
	}

	// x[i] ... coupon
	// x[i+N] ... term
	// x[i+2N] ... margin
	const size_t n = (x.size()-1) / 3;
	DoubleVector coupons(n), margins(n);
	copy(x.begin(),       x.begin() + n,   coupons.begin());
	copy(x.begin() + 2*n, x.begin() + 3*n, margins.begin());

	double rate = 0.0;
	for (unsigned int i = 0; i < n; i++){
		rate += coupons[i] + margins[i];
	}

	const double whole_term = x.back();
	return (rate/n) * whole_term;
}



AQLCoreFunctionBase*	
AQLCompoundMethod11::clone() const	
{
    try 
	{
		return new AQLCompoundMethod11(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
AQLCompoundMethod11::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING11 ? true : AQLCompoundMethod::isTypeOf(id));
}
function_t
AQLCompoundMethod11::getType() const
{
	return FN_COMPOUNDING10;
}

//Compounding (ISDA) for AlgoQuantHub
/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLCompoundMethod11::operator()(const DoubleArray& x) const
{
	if (x.size() % 3 != 0)
    {
        throw AQLCoreInvalidData("#Error: Unable to calculate the compound rate",__FILE__,__LINE__);
	}

	// x[i] ... coupon
	// x[i+N] ... term
	// x[i+2N] ... margin
	const size_t n = (x.size()) / 3;
	DoubleVector coupons(n), terms(n), margins(n);
	copy(x.begin(),       x.begin() + n,   coupons.begin());
	copy(x.begin() + n,   x.begin() + 2*n, terms.begin());
	copy(x.begin() + 2*n, x.begin() + 3*n, margins.begin());

	double rate = 0.0;
	for (unsigned int i = 0; i < n; i++)
    {
		rate += terms[i] * (coupons[i] + margins[i]);
	}

	return rate;
}