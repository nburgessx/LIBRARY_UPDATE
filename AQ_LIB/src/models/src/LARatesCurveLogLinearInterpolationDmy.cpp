/*! @file
    @brief Source code of class of linear interpolation of curve path element


*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesCurveLogLinearInterpolationDmy.cpp
//
//  SYNOPSIS    :       LARatesCurveLogLinearInterpolationDmy
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


#include "LARatesCurveLogLinearInterpolationDmy.h"
#include "LABasic.h"
#include "LAModelDynamicsLMMCurve.h"
#include "LAModelDynamicsDDLMMCurve.h"
#include "LAAlgorithm.h"

using namespace std;
//================ LARatesCurveLogLinearInterpolationDmy ===================================
/*!
	@brief default constructor
	
*/
LARatesCurveLogLinearInterpolationDmy::LARatesCurveLogLinearInterpolationDmy()
{
}
/*!
	@brief copy constructor
*/
LARatesCurveLogLinearInterpolationDmy::LARatesCurveLogLinearInterpolationDmy(const LARatesCurveLogLinearInterpolationDmy& v) 
: LARatesCurveLogLinearInterpolation(v)
{
}

/*!
	@brief destructor
*/
LARatesCurveLogLinearInterpolationDmy::~LARatesCurveLogLinearInterpolationDmy() 
{
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesCurveLogLinearInterpolationDmy::clone() const
{
    try 
	{
		return new LARatesCurveLogLinearInterpolationDmy(*this);
    }
    catch (bad_alloc &e)
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
LARatesCurveLogLinearInterpolationDmy::isTypeOf(function_t id) const
{
	return (id==FN_CURVELOGLINEARINTERPOLATIONDMY ? true : LARatesCurveLogLinearInterpolation::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesCurveLogLinearInterpolationDmy::getType() const
{
	return FN_CURVELOGLINEARINTERPOLATIONDMY;
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
LARatesCurveLogLinearInterpolationDmy::value(double t, double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2)
{
	(void)t1, (void)t2;
	if (mValues.end() == mValues.find(t))
	{
		LARatesCurveForLogLinearInterpolationDmy &curve = mValues[t];
		curve.set_t(t);
		curve.mpCurve1 = &dynamic_cast<const LARatesPathElementCurve&>(val1);
		curve.mpCurve2 = &dynamic_cast<const LARatesPathElementCurve&>(val2);
		
		if(curve.mSpreads.empty() && val1.isTypeOf(PE_DDLMMCURVE))
		{
			curve.mSpreads= dynamic_cast<const LARatesPathElementDDLMMCurve* >(curve.mpCurve1)->getSpreads();
			curve.volmat = mVol;
		
		}
		else if (!val1.isTypeOf(PE_DDLMMCURVE))
		{
			curve.mSpreads.resize(mVol[0].size(), 0.0);
			curve.volmat = mVol;
		
		}
		
		const DoubleArray& tenor = *(dynamic_cast<const LARatesPathElementLMMCurve *>(curve.mpCurve1)->getTenor()); 
		unsigned int tSize = dynamic_cast<const LARatesPathElementLMMCurve *>(curve.mpCurve1)->getTenor()->size();
		LAAlgorithm::locate<DoubleArray, double>(tenor, t, tSize, curve.mfixpos);
	}

	return mValues[t];	
}

//================ LARatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy() 
: LARatesCurveForLogLinearInterpolation()
{
}


/*!
	@brief destructor
*/
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::~LARatesCurveForLogLinearInterpolationDmy() 
{
}

//copy constructor
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy(const LARatesCurveForLogLinearInterpolationDmy& v) 
: LARatesCurveForLogLinearInterpolation(v)
{

}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORLOGLINEARINTERDMY ? true : LARatesCurveForLogLinearInterpolation::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::getType() const
{
	return PE_CURVEFORLOGLINEARINTERDMY;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::clone() const
{
    try 
	{
		return new LARatesCurveForLogLinearInterpolationDmy(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy&
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::operator = (const LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy& a)
{
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORLOGLINEARINTERDMY)) 
	{
		LAString err = "Assignment error for LARatesCurveForLogLinearInterpolationDmy : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	
	LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::operator=(a);

	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
LARatesCurveLogLinearInterpolationDmy::LARatesCurveForLogLinearInterpolationDmy::getP (double T) const
{
	if (mPVal.end() == mPVal.find(T))
	{
		mPVal.insert(make_pair(T, LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::getP(T)));
	}

	return mPVal[T];
}

