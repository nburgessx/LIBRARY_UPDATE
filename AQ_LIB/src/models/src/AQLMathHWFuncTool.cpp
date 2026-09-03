/*! @file
    @brief Source code for class to represent constant function

	This class derives from AQLFunctionBase
*/
//  2008, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMathHWFuncTool.cpp
//
//  SYNOPSIS    :       AQLMathHWFuncTool
//  DESCRIPTION :       Source code for class to represent constant function
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


#include "AQLMathHWFuncTool.h"

using namespace std;
//================ AQLMathHWFuncTool ===================================
/*!
	@brief default constructor
*/
AQLMathHWFuncTool::AQLMathHWFuncTool(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma)
: AQLFunctionBase(),mpHWMR(&funcHWMR),mpHWSigma(&funcHWSigma)
{
	/*mpHWMR(funcHWMR.clone());
	mpHWSigma(funcHWSigma.clone());*/
}
/*!
	@brief destructor
*/
AQLMathHWFuncTool::~AQLMathHWFuncTool() 
{
	delete mpHWMR;
	delete mpHWSigma;
	mpHWMR=0;
	mpHWSigma=0;
}

/*!
	@brief copy constructor
*/
AQLMathHWFuncTool::AQLMathHWFuncTool(const AQLMathHWFuncTool &rhs) 
:
AQLFunctionBase(rhs),
mpHWMR(rhs.mpHWMR !=0 ? dynamic_cast<AQLMathHWFuncMR*>(rhs.mpHWMR->clone()) : 0),
mpHWSigma(rhs.mpHWSigma !=0 ? dynamic_cast<AQLMathHWFuncSigma*>(rhs.mpHWSigma->clone()) : 0 )
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncTool::clone() const	
{
    try 
	{
		return new AQLMathHWFuncTool(*this);
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
AQLMathHWFuncTool::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOL ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLMathHWFuncTool::getType() const
{
	return FN_HWFUNCTIONTOOL;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncTool::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("This Operator should not be allowded",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncTool::operator()(double x) const
{
	throw AQLCoreInvalidData("This Operator is not allowded",__FILE__,__LINE__);
}


//================ AQLMathHWFuncToolForVar ===================================
/*!
	@brief default constructor
*/
AQLMathHWFuncToolForVar::AQLMathHWFuncToolForVar(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma)
: AQLMathHWFuncTool(funcHWMR,funcHWSigma)
{
}
/*!
	@brief destructor
*/
AQLMathHWFuncToolForVar::~AQLMathHWFuncToolForVar() 
{
}

/*!
	@brief copy constructor
*/
AQLMathHWFuncToolForVar::AQLMathHWFuncToolForVar(const AQLMathHWFuncToolForVar &rhs) 
:
AQLMathHWFuncTool(rhs)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncToolForVar::clone() const	
{
    try 
	{
		return new AQLMathHWFuncToolForVar(*this);
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
AQLMathHWFuncToolForVar::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOLFORVAR ? true : AQLMathHWFuncTool::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLMathHWFuncToolForVar::getType() const
{
	return FN_HWFUNCTIONTOOLFORVAR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncToolForVar::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("This operator is not supported",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncToolForVar::operator()(double x) const
{
	double val = AQLMath::exp(mpHWMR->integrate(0,x)) * mpHWSigma->operator()(x);
	return val * val;
}

//================ AQLMathHWFuncToolForMR ===================================
/*!
	@brief default constructor
*/
AQLMathHWFuncToolForMR::AQLMathHWFuncToolForMR(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma)
: AQLMathHWFuncTool(funcHWMR,funcHWSigma)
{
}
/*!
	@brief destructor
*/
AQLMathHWFuncToolForMR::~AQLMathHWFuncToolForMR() 
{
}

/*!
	@brief copy constructor
*/
AQLMathHWFuncToolForMR::AQLMathHWFuncToolForMR(const AQLMathHWFuncToolForMR &rhs) 
:
AQLMathHWFuncTool(rhs)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncToolForMR::clone() const	
{
    try 
	{
		return new AQLMathHWFuncToolForMR(*this);
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
AQLMathHWFuncToolForMR::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOLFORMR ? true : AQLMathHWFuncTool::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLMathHWFuncToolForMR::getType() const
{
	return FN_HWFUNCTIONTOOLFORMR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncToolForMR::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("This operator is not supported",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncToolForMR::operator()(double x) const
{
	return ExpIntegralInvMR(x);
}

//================ AQLMathHWFuncToolForMRIntegral ===================================
/*!
	@brief default constructor
*/
AQLMathHWFuncToolForMRIntegral::AQLMathHWFuncToolForMRIntegral(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma)
: AQLMathHWFuncTool(funcHWMR,funcHWSigma),mGL(HWGAUSSLEGENDRENUM)
{
	mpToolMR = new AQLMathHWFuncToolForMR(*dynamic_cast<AQLMathHWFuncMR*>(funcHWMR.clone()),
										*dynamic_cast<AQLMathHWFuncSigma*>(funcHWSigma.clone()));
}
/*!
	@brief destructor
*/
AQLMathHWFuncToolForMRIntegral::~AQLMathHWFuncToolForMRIntegral() 
{
	delete mpToolMR;
	mpToolMR=0;
}

/*!
	@brief copy constructor
*/
AQLMathHWFuncToolForMRIntegral::AQLMathHWFuncToolForMRIntegral(const AQLMathHWFuncToolForMRIntegral &rhs) 
:
AQLMathHWFuncTool(rhs),
mpToolMR(rhs.mpToolMR !=0 ? dynamic_cast<AQLMathHWFuncToolForMR*>(rhs.mpToolMR->clone()) : 0 ),
mGL(rhs.mGL)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathHWFuncToolForMRIntegral::clone() const	
{
    try 
	{
		return new AQLMathHWFuncToolForMRIntegral(*this);
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
AQLMathHWFuncToolForMRIntegral::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOLFORMR ? true : AQLMathHWFuncTool::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLMathHWFuncToolForMRIntegral::getType() const
{
	return FN_HWFUNCTIONTOOLFORMR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncToolForMRIntegral::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("NotSupportedNow",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLMathHWFuncToolForMRIntegral::operator()(double x) const
{
	return mGL.integrate(*mpToolMR,0.0,x);
}
