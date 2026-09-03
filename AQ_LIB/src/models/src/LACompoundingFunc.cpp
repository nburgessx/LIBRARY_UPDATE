/*! @file
    @brief Source code of class to represent linear function

    This class derives from LAFunctionBase

*/
//  2006, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACompoundMethod.cpp
//
//  SYNOPSIS    :       LACompoundMethod
//  DESCRIPTION :       Source code of class to represent linear function
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LACompoundingFunc.h"
#include "LABasic.h"
#include "LADist.h"
#include <numeric>

using namespace std;
//================ LACompoundMethod ===================================
/*!
	@brief default constructor
*/
LACompoundMethod::LACompoundMethod() 
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LACompoundMethod::~LACompoundMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LACompoundMethod::clone() const
{
    try 
	{
		return new LACompoundMethod(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LACompoundMethod::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LACompoundMethod::getType() const
{
	return FN_COMPOUNDING;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LACompoundMethod::operator()(const DoubleArray& x) const
{
	//calc compounding rate
	if (x.size() % 3 != 0)
		throw LACoreInvalidData("Compounidng params Error",__FILE__,__LINE__);

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
LACompoundMethod::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}

double 
LACompoundMethod::getCompoundedVal(double coupon, double margin) const
{
	return coupon + margin;	
}

double 
LACompoundMethod::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}

//////////////LACompoundMethod1//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod1::LACompoundMethod1() 
: LACompoundMethod(){}

LACompoundMethod1::~LACompoundMethod1() {}

LACoreFunctionBase*	
LACompoundMethod1::clone() const	
{
    try 
	{
		return new LACompoundMethod1(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod1::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING1 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod1::getType() const
{
	return FN_COMPOUNDING1;
}
double 
LACompoundMethod1::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}
double 
LACompoundMethod1::getCompoundedVal(double coupon, double margin) const
{
	return coupon + margin;	
}
double 
LACompoundMethod1::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}


//////////////LACompoundMethod2//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod2::LACompoundMethod2() 
: LACompoundMethod(){}

LACompoundMethod2::~LACompoundMethod2() {}

LACoreFunctionBase*	
LACompoundMethod2::clone() const	
{
    try 
	{
		return new LACompoundMethod2(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod2::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING2 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod2::getType() const
{
	return FN_COMPOUNDING2;
}
double 
LACompoundMethod2::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon ; 
}
double 
LACompoundMethod2::getCompoundedVal(double coupon, double margin) const
{
	return coupon + margin;	
}
double 
LACompoundMethod2::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}

//////////////LACompoundMethod3//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod3::LACompoundMethod3() 
: LACompoundMethod(){}

LACompoundMethod3::~LACompoundMethod3() {}

LACoreFunctionBase*	
LACompoundMethod3::clone() const	
{
    try 
	{
		return new LACompoundMethod3(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod3::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING3 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod3::getType() const
{
	return FN_COMPOUNDING3;
}
double 
LACompoundMethod3::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon ; 
}
double 
LACompoundMethod3::getCompoundedVal(double coupon, double margin) const
{
	return coupon ;	
}
double 
LACompoundMethod3::getAdditionalVal(double coupon, double margin) const
{
	return margin;	
}

//////////////LACompoundMethod4//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod4::LACompoundMethod4() 
: LACompoundMethod(){}

LACompoundMethod4::~LACompoundMethod4() {}

LACoreFunctionBase*	
LACompoundMethod4::clone() const	
{
    try 
	{
		return new LACompoundMethod4(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod4::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING4 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod4::getType() const
{
	return FN_COMPOUNDING4;
}
double 
LACompoundMethod4::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}
double 
LACompoundMethod4::getCompoundedVal(double coupon, double margin) const
{
	return coupon ;	
}
double 
LACompoundMethod4::getAdditionalVal(double coupon, double margin) const
{
	return 0.0;	
}

//////////////LACompoundMethod5//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod5::LACompoundMethod5() 
: LACompoundMethod(){}

LACompoundMethod5::~LACompoundMethod5() {}

LACoreFunctionBase*	
LACompoundMethod5::clone() const	
{
    try 
	{
		return new LACompoundMethod5(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod5::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING5 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod5::getType() const
{
	return FN_COMPOUNDING5;
}
double 
LACompoundMethod5::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0 + coupon + margin; 
}
double 
LACompoundMethod5::getCompoundedVal(double coupon, double margin) const
{
	return coupon ;	
}
double 
LACompoundMethod5::getAdditionalVal(double coupon, double margin) const
{
	return margin;	
}

//////////////LACompoundMethod6//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod6::LACompoundMethod6() 
: LACompoundMethod(){}

LACompoundMethod6::~LACompoundMethod6() {}

LACoreFunctionBase*	
LACompoundMethod6::clone() const	
{
    try 
	{
		return new LACompoundMethod6(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod6::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING6 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod6::getType() const
{
	return FN_COMPOUNDING6;
}
double 
LACompoundMethod6::getCompoundingCoeff(double coupon, double margin) const
{
	return 1.0; 
}
double 
LACompoundMethod6::getCompoundedVal(double coupon, double margin) const
{
	return 0.0 ;	
}
double 
LACompoundMethod6::getAdditionalVal(double coupon, double margin) const
{
	return coupon + margin;	
}

//Compounding (ISDA)
//////////////LACompoundMethod7//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod7::LACompoundMethod7() 
: LACompoundMethod(){}

LACompoundMethod7::~LACompoundMethod7() {}

LACoreFunctionBase*	
LACompoundMethod7::clone() const	
{
    try 
	{
		return new LACompoundMethod7(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod7::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING7 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod7::getType() const
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
LACompoundMethod7::operator()(const DoubleArray& x) const
{
	//calc compounding rate
	if (x.size() % 3 != 0)
		throw LACoreInvalidData("Compounidng params Error",__FILE__,__LINE__);

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
//////////////LACompoundMethod8//////////////////
/*!
	@brief default constructor
*/
LACompoundMethod8::LACompoundMethod8() 
: LACompoundMethod(){}

LACompoundMethod8::~LACompoundMethod8() {}

LACoreFunctionBase*	
LACompoundMethod8::clone() const	
{
    try 
	{
		return new LACompoundMethod8(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod8::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING8 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod8::getType() const
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
LACompoundMethod8::operator()(const DoubleArray& x) const
{
	//calc compounding rate
	if (x.size() % 3 != 0)
		throw LACoreInvalidData("Compounidng params Error",__FILE__,__LINE__);

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




LACompoundMethod9::LACompoundMethod9() 
: LACompoundMethod(){}

LACompoundMethod9::~LACompoundMethod9() {}

LACoreFunctionBase*	
LACompoundMethod9::clone() const	
{
    try 
	{
		return new LACompoundMethod9(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod9::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING9 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod9::getType() const
{
	return FN_COMPOUNDING9;
}

double
LACompoundMethod9::operator()(const DoubleArray& x) const
{
	if(x.size() % 3 != 1){
		throw LACoreInvalidData("Compounidng params Error",__FILE__,__LINE__);
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




LACoreFunctionBase*	
LACompoundMethod10::clone() const	
{
    try 
	{
		return new LACompoundMethod10(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod10::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING10 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod10::getType() const
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
LACompoundMethod10::operator()(const DoubleArray& x) const
{
	if (x.size() % 3 != 1){
		throw LACoreInvalidData("Compounding params Error",__FILE__,__LINE__);
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



LACoreFunctionBase*	
LACompoundMethod11::clone() const	
{
    try 
	{
		return new LACompoundMethod11(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
bool
LACompoundMethod11::isTypeOf(function_t id) const
{
	return (id == FN_COMPOUNDING11 ? true : LACompoundMethod::isTypeOf(id));
}
function_t
LACompoundMethod11::getType() const
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
LACompoundMethod11::operator()(const DoubleArray& x) const
{
	if (x.size() % 3 != 0)
    {
        throw LACoreInvalidData("#Error: Unable to calculate the compound rate",__FILE__,__LINE__);
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