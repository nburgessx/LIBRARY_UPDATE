/*! @file
    @brief Source code of class of linear interpolation of curve path element


*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesCurveLinearInterpolation.cpp
//
//  SYNOPSIS    :       LARatesCurveLinearInterpolation
//  DESCRIPTION :       Source code of class of linear interpolation of curve path element
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


#include "LARatesCurveLinearInterpolation.h"


using namespace std;
//================ LARatesCurveLinearInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
LARatesCurveLinearInterpolation::LARatesCurveLinearInterpolation()
{

}
/*!
	@brief copy constructor
*/
LARatesCurveLinearInterpolation::LARatesCurveLinearInterpolation(const LARatesCurveLinearInterpolation& v) 
: LARatesPEInterpolationBase(v), mValue(v.mValue)
{

}

/*!
	@brief destructor
*/
LARatesCurveLinearInterpolation::~LARatesCurveLinearInterpolation() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesCurveLinearInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveLinearInterpolation(*this);
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
LARatesCurveLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVELINEARINTERPOLATION ? true : LARatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesCurveLinearInterpolation::getType() const
{
	return FN_CURVELINEARINTERPOLATION;
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
LARatesCurveLinearInterpolation::value(double t, double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2)
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const LARatesPathElementCurve&>(val1);
	mValue.mpCurve2 = &dynamic_cast<const LARatesPathElementCurve&>(val2);
	return mValue;
}


//================ LARatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation::LARatesCurveForLinearInterpolation() 
: LARatesPathElementCurve()
{

}


/*!
	@brief destructor
*/
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation::~LARatesCurveForLinearInterpolation() 
{

}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORLINEARINTER ? true : LARatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation::getType() const
{
	return PE_CURVEFORLINEARINTER;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveForLinearInterpolation(*this);
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
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation&
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation::operator = (const LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation& a)
{
	// 自分自身のコピーはしない
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORLINEARINTER)) 
	{	// タイプチェックの際おかしいことが起こったら
		// 例外発生
		LAString err = "Assignment error for LARatesCurveForLinearInterpolation : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	// コピー。	
	mpCurve1 = dynamic_cast<const LARatesCurveForLinearInterpolation&>(a).mpCurve1;
	mpCurve2 = dynamic_cast<const LARatesCurveForLinearInterpolation&>(a).mpCurve2;

	LARatesPathElementCurve::operator = (a);
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
LARatesCurveLinearInterpolation::LARatesCurveForLinearInterpolation::getP (double T) const
{
	double t1 = mpCurve1->get_t();
	double t2 = mpCurve2->get_t();
	if (t1 > m_t || t2 < m_t)
	{
		//error
		throw LACoreInvalidData("Condition of t1 <= t <= t2 is not maintain", __FILE__, __LINE__);
	}
	if (T <= m_t) return 1.0;
	else if (T <= t2)
		return ((m_t - t1) + mpCurve1->getP(T) * (T - m_t)) / (T - t1);
	else
		return 	(mpCurve2->getP(T) * (m_t - t1) + mpCurve1->getP(T) * (t2 - m_t)) / (t2 - t1);
}


