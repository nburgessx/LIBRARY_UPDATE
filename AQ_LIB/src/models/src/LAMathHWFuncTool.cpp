/*! @file
    @brief Source code for class to represent constant function

	This class derives from AQLFunctionBase
*/
//  2008, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathHWFuncTool.cpp
//
//  SYNOPSIS    :       LAMathHWFuncTool
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


#include "LAMathHWFuncTool.h"

using namespace std;
//================ LAMathHWFuncTool ===================================
/*!
	@brief default constructor
*/
LAMathHWFuncTool::LAMathHWFuncTool(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma)
: AQLFunctionBase(),mpHWMR(&funcHWMR),mpHWSigma(&funcHWSigma)
{
	/*mpHWMR(funcHWMR.clone());
	mpHWSigma(funcHWSigma.clone());*/
}
/*!
	@brief destructor
*/
LAMathHWFuncTool::~LAMathHWFuncTool() 
{
	delete mpHWMR;
	delete mpHWSigma;
	mpHWMR=0;
	mpHWSigma=0;
}

/*!
	@brief copy constructor
*/
LAMathHWFuncTool::LAMathHWFuncTool(const LAMathHWFuncTool &rhs) 
:
AQLFunctionBase(rhs),
mpHWMR(rhs.mpHWMR !=0 ? dynamic_cast<LAMathHWFuncMR*>(rhs.mpHWMR->clone()) : 0),
mpHWSigma(rhs.mpHWSigma !=0 ? dynamic_cast<LAMathHWFuncSigma*>(rhs.mpHWSigma->clone()) : 0 )
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAMathHWFuncTool::clone() const	
{
    try 
	{
		return new LAMathHWFuncTool(*this);
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
LAMathHWFuncTool::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOL ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAMathHWFuncTool::getType() const
{
	return FN_HWFUNCTIONTOOL;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncTool::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("This Operator should not be allowded",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncTool::operator()(double x) const
{
	throw AQLCoreInvalidData("This Operator is not allowded",__FILE__,__LINE__);
}


//================ LAMathHWFuncToolForVar ===================================
/*!
	@brief default constructor
*/
LAMathHWFuncToolForVar::LAMathHWFuncToolForVar(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma)
: LAMathHWFuncTool(funcHWMR,funcHWSigma)
{
}
/*!
	@brief destructor
*/
LAMathHWFuncToolForVar::~LAMathHWFuncToolForVar() 
{
}

/*!
	@brief copy constructor
*/
LAMathHWFuncToolForVar::LAMathHWFuncToolForVar(const LAMathHWFuncToolForVar &rhs) 
:
LAMathHWFuncTool(rhs)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAMathHWFuncToolForVar::clone() const	
{
    try 
	{
		return new LAMathHWFuncToolForVar(*this);
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
LAMathHWFuncToolForVar::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOLFORVAR ? true : LAMathHWFuncTool::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAMathHWFuncToolForVar::getType() const
{
	return FN_HWFUNCTIONTOOLFORVAR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncToolForVar::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("This operator is not supported",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncToolForVar::operator()(double x) const
{
	double val = AQLMath::exp(mpHWMR->integrate(0,x)) * mpHWSigma->operator()(x);
	return val * val;
}

//================ LAMathHWFuncToolForMR ===================================
/*!
	@brief default constructor
*/
LAMathHWFuncToolForMR::LAMathHWFuncToolForMR(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma)
: LAMathHWFuncTool(funcHWMR,funcHWSigma)
{
}
/*!
	@brief destructor
*/
LAMathHWFuncToolForMR::~LAMathHWFuncToolForMR() 
{
}

/*!
	@brief copy constructor
*/
LAMathHWFuncToolForMR::LAMathHWFuncToolForMR(const LAMathHWFuncToolForMR &rhs) 
:
LAMathHWFuncTool(rhs)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAMathHWFuncToolForMR::clone() const	
{
    try 
	{
		return new LAMathHWFuncToolForMR(*this);
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
LAMathHWFuncToolForMR::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOLFORMR ? true : LAMathHWFuncTool::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAMathHWFuncToolForMR::getType() const
{
	return FN_HWFUNCTIONTOOLFORMR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncToolForMR::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("This operator is not supported",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncToolForMR::operator()(double x) const
{
	return ExpIntegralInvMR(x);
}

//================ LAMathHWFuncToolForMRIntegral ===================================
/*!
	@brief default constructor
*/
LAMathHWFuncToolForMRIntegral::LAMathHWFuncToolForMRIntegral(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma)
: LAMathHWFuncTool(funcHWMR,funcHWSigma),mGL(HWGAUSSLEGENDRENUM)
{
	mpToolMR = new LAMathHWFuncToolForMR(*dynamic_cast<LAMathHWFuncMR*>(funcHWMR.clone()),
										*dynamic_cast<LAMathHWFuncSigma*>(funcHWSigma.clone()));
}
/*!
	@brief destructor
*/
LAMathHWFuncToolForMRIntegral::~LAMathHWFuncToolForMRIntegral() 
{
	delete mpToolMR;
	mpToolMR=0;
}

/*!
	@brief copy constructor
*/
LAMathHWFuncToolForMRIntegral::LAMathHWFuncToolForMRIntegral(const LAMathHWFuncToolForMRIntegral &rhs) 
:
LAMathHWFuncTool(rhs),
mpToolMR(rhs.mpToolMR !=0 ? dynamic_cast<LAMathHWFuncToolForMR*>(rhs.mpToolMR->clone()) : 0 ),
mGL(rhs.mGL)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAMathHWFuncToolForMRIntegral::clone() const	
{
    try 
	{
		return new LAMathHWFuncToolForMRIntegral(*this);
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
LAMathHWFuncToolForMRIntegral::isTypeOf(function_t id) const
{
	return (id == FN_HWFUNCTIONTOOLFORMR ? true : LAMathHWFuncTool::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAMathHWFuncToolForMRIntegral::getType() const
{
	return FN_HWFUNCTIONTOOLFORMR;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncToolForMRIntegral::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("NotSupportedNow",__FILE__,__LINE__);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAMathHWFuncToolForMRIntegral::operator()(double x) const
{
	return mGL.integrate(*mpToolMR,0.0,x);
}
