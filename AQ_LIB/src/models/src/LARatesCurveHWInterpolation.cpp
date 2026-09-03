/*! @file
    @brief Source code of class of linear interpolation of curve path element


*/
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesCurveHWInterpolation.cpp
//
//  SYNOPSIS    :       LARatesCurveHWInterpolation
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

//+++++ include +++++
#include "LARatesCurveHWInterpolation.h"
#include "LAModelDynamicsHW1FCurve.h"

using namespace std;
//================ LARatesCurveHWInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/

LARatesCurveHWInterpolation::LARatesCurveHWInterpolation()
{
}

/*!
	@brief copy constructor
*/
LARatesCurveHWInterpolation::LARatesCurveHWInterpolation(const LARatesCurveHWInterpolation& v) 
:
LARatesPEInterpolationBase(v),
mValue(v.mValue)
{
}

/*!
	@brief destructor
*/
LARatesCurveHWInterpolation::~LARatesCurveHWInterpolation() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesCurveHWInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveHWInterpolation(*this);
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
LARatesCurveHWInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVEHWINTERPOLATION ? true : LARatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesCurveHWInterpolation::getType() const
{
	return FN_CURVEHWINTERPOLATION;
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
LARatesCurveHWInterpolation::value( double t, double t1, double t2, 
                                 const LARatesPathElementBase& val1,
								 const LARatesPathElementBase& val2
                               )
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const LARatesPathElementCurve&>(val1);
	mValue.mpCurve2 = &dynamic_cast<const LARatesPathElementCurve&>(val2);
	return mValue;
}


/*!
    @brief clear the cache
    @return 
*/
void
LARatesCurveHWInterpolation::init(void)
{
	map<double, LARatesPathElementBase*>::iterator itr;
	for(itr =  mValue.curve_cache.begin(); itr!= mValue.curve_cache.end();++itr)
	{
		delete itr->second ;
		itr->second = 0;
	}
	
	mValue.curve_cache.clear();
	mValue.is_curve_cache.clear();
	mValue.initialF_cache.clear();
	return;
}

//================ LARatesPathElementHWCurve ===================================

LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation::LARatesCurveForHWInterpolation() 
: LARatesPathElementCurve(), is_cloned(false)
{
}

/*!
	@brief destructor
*/
LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation::~LARatesCurveForHWInterpolation() 
{
    map<double, LARatesPathElementBase*>::iterator itr;
	for(itr = curve_cache.begin(); itr!= curve_cache.end();++itr)
	{
		delete itr->second ;
		itr->second = 0;
	}
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORHW ? true : LARatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation::getType() const
{
	return PE_CURVEFORHW;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveForHWInterpolation(*this);
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
LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation&
LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation::operator = (const LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation& a)
{
	// 自分自身のコピーはしない
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORHW)) 
	{	// タイプチェックの際おかしいことが起こったら
		// 例外発生
		LAString err = "Assignment error for LARatesCurveForHWInterpolation : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	// コピー。	
	mpCurve1 = dynamic_cast<const LARatesCurveForHWInterpolation&>(a).mpCurve1;
	mpCurve2 = dynamic_cast<const LARatesCurveForHWInterpolation&>(a).mpCurve2;

	LARatesPathElementCurve::operator = (a);
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
LARatesCurveHWInterpolation::LARatesCurveForHWInterpolation::getP (double T) const
{
	double t1 = mpCurve1->get_t();
	double t2 = mpCurve2->get_t();

	if (t1 > m_t || t2 < m_t)
	{
		//error
		throw LACoreInvalidData("Condition of t1 <= t <= t2 is not maintain", __FILE__, __LINE__);
	}
	    
    if (T <= m_t) return 1.0;

	double ft1 = dynamic_cast<const LARatesPathElementHW1FCurve*>(mpCurve1)->initialF();
	double ft2 = dynamic_cast<const LARatesPathElementHW1FCurve*>(mpCurve2)->initialF();
   	double rt1= mpCurve1->get()[0] - ft1;
	double rt2 = mpCurve2->get()[0] - ft2;
	
	double r = rt1 + ( rt2 - rt1 ) * ( m_t - t1 ) / ( t2 - t1 );
 
	if(!is_curve_cache[m_t])	
    {
        curve_cache[m_t] = mpCurve1->clone();
        dynamic_cast<LARatesPathElementCurve*>(curve_cache[m_t])->set_t(m_t);        
		initialF_cache[m_t] = dynamic_cast<const LARatesPathElementHW1FCurve*>(mpCurve1)->initialF(m_t);
		is_curve_cache[m_t] = true;
    }
	curve_cache[m_t]->set(r + initialF_cache[m_t]);
	return dynamic_cast<LARatesPathElementCurve*>(curve_cache[m_t])->getP(T);

}



