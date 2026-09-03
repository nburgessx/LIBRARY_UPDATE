/*! @file
    @brief Source code of class to represent shift function f(x+a)

	This class derives from LAFunctionBase

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAShiftMethod.h
//
//  SYNOPSIS    :       LAShiftMethod
//  DESCRIPTION :       Source code of class to represent shift function f(x+a)
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAShiftFunc.h"
#include "LABasic.h"
#include "LADist.h"

using namespace std;
//================ LAShiftMethod ===================================
/*!
	@brief constructor
	@param[in] a shift parameter
	@param[in] method function
*/
LAShiftMethod::LAShiftMethod(const DoubleArray& a, const LAFunctionBase& method) 
: LAFunctionBase(method), mpFunc(NULL)
{
	setParam(a);
	mpFunc = dynamic_cast<LAFunctionBase*>(method.clone());

	////////////////////BYFUKUDA
////////	mDiscontPoints = method.getDiscontPoints();
//	set<double>::iterator it;
//	for(it = mDiscontPoints.begin(); it != mDiscontPoints.end(); it++)
//        (*it) -= a[getPos()];

	set<double> temp_set = mDiscontPoints;
	mDiscontPoints.clear();
	set<double>::iterator it;
	for(it = temp_set.begin(); it != temp_set.end(); it++)
		mDiscontPoints.insert((*it) - a[getPos()]);
}

/*!
	@brief destructor
*/
LAShiftMethod::~LAShiftMethod() 
{
	delete mpFunc;
}

/*!
	@brief copy constructor
*/
LAShiftMethod::LAShiftMethod(const LAShiftMethod& v) 
: LAFunctionBase(v), mpFunc(NULL)
{
	if (v.mpFunc) mpFunc = dynamic_cast<LAFunctionBase*>(v.mpFunc->clone());
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAShiftMethod::clone() const
{
    try 
	{
		return new LAShiftMethod(*this);
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
LAShiftMethod::isTypeOf(function_t id) const
{
	return (id == FN_SHIFTFUNC ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAShiftMethod::getType() const
{
	return FN_SHIFTFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAShiftMethod::operator()(const DoubleArray& x) const
{
	DoubleArray xx = x;
	for(unsigned int i = 0; i < xx.size(); i++)
		xx[i] += mParam.at(i);	
	return mpFunc->operator()(xx);
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LAShiftMethod::operator()(const double x) const
{
	double xx = x;
	xx += mParam.at(0);	
	return mpFunc->operator()(xx);
}

/*!
    @brief Return partial derivative value
	@param[in] x integral point
	@param[in] pos variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
LAShiftMethod::partialDerivative(const DoubleArray& x,unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype,double delta) const
{
	DoubleArray xx = x;
	for(unsigned int i = 0; i < xx.size(); i++)
		xx[i] += mParam.at(i);	
	
	return mpFunc->partialDerivative(xx, pos, calctype, difftype, delta);
}

/*!
    @brief Return second partial derivative value
	@param[in] x integral point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/
double
LAShiftMethod::partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	DoubleArray xx = x;
	for(unsigned int i = 0; i < xx.size(); i++)
		xx[i] += mParam.at(i);	
	
	return mpFunc->partialDerivative2(xx, posi, posj, calctype, delta);
}

/*!
    @brief Set check flag
	@param[in] checkflag check flag
*/
void
LAShiftMethod::setCheckFlag(bool checkflag)
{
	mCheckFlag=checkflag;
	mpFunc->setCheckFlag(checkflag);
}
