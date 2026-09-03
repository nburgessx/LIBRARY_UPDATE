/*! @file
    @brief Source code of class of linear interpolation of curve path element


*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesCurveStepLinearInterpolation.cpp
//
//  SYNOPSIS    :       LARatesCurveStepLinearInterpolation
//  DESCRIPTION :       Source code of class of steplinear interpolation of curve path element
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


#include "LARatesCurveStepLinearInterpolation.h"


using namespace std;
//================ LARatesCurveLinearInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
LARatesCurveStepLinearInterpolation::LARatesCurveStepLinearInterpolation()
{

}
/*!
	@brief copy constructor
*/
LARatesCurveStepLinearInterpolation::LARatesCurveStepLinearInterpolation(const LARatesCurveStepLinearInterpolation& v) 
: LARatesPEInterpolationBase(v), mValue(v.mValue)
{

}

/*!
	@brief destructor
*/
LARatesCurveStepLinearInterpolation::~LARatesCurveStepLinearInterpolation() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesCurveStepLinearInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveStepLinearInterpolation(*this);
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
LARatesCurveStepLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVESTEPLINEARINTERPOLATION ? true : LARatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesCurveStepLinearInterpolation::getType() const
{
	return FN_CURVESTEPLINEARINTERPOLATION;
}
/*!
    @brief Return interpolated value
	@param[in] t point to get value
	@param[in] t1 point of left end
	@param[in] t2 point of right end
	@param[in] val1 value at t1
	@param[in] val2 value at t2

	@return interpolated value
*/
const LARatesPathElementBase&
LARatesCurveStepLinearInterpolation::value(double t, double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2)
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const LARatesPathElementCurve&>(val1);
	return mValue;
}


//================ LARatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation::LARatesCurveForStepLinearInterpolation() 
: LARatesPathElementCurve()
{

}


/*!
	@brief destructor
*/
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation::~LARatesCurveForStepLinearInterpolation() 
{

}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORSTEPLINEARINTER ? true : LARatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation::getType() const
{
	return PE_CURVEFORSTEPLINEARINTER;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveForStepLinearInterpolation(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation&
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation::operator = (const LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation& a)
{
	// 自分自身のコピーはしない
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORSTEPLINEARINTER)) 
	{	// タイプチェックの際おかしいことが起こったら
		// 例外発生
		LAString err = "Assignment error for LARatesCurveForStepLinearInterpolation : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	LARatesPathElementCurve::operator = (a);
	// コピー。	
	mpCurve1 = dynamic_cast<const LARatesCurveForStepLinearInterpolation&>(a).mpCurve1;
	
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
LARatesCurveStepLinearInterpolation::LARatesCurveForStepLinearInterpolation::getP (double T) const
{
	return mpCurve1->getP(T);
}


