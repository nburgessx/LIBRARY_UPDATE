/*! @file
    @brief Source code of class of linear interpolation of curve path element
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesCurveLogLinearInterpolationDmy.h"
#include "AQLBasic.h"
#include "AQLModelDynamicsLMMCurve.h"
#include "AQLModelDynamicsDDLMMCurve.h"
#include "AQLAlgorithm.h"

using namespace std;
//================ AQLRatesCurveLogLinearInterpolationDmy ===================================
/*!
	@brief default constructor
	
*/
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveLogLinearInterpolationDmy()
{
}
/*!
	@brief copy constructor
*/
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveLogLinearInterpolationDmy(const AQLRatesCurveLogLinearInterpolationDmy& v) 
: AQLRatesCurveLogLinearInterpolation(v)
{
}

/*!
	@brief destructor
*/
AQLRatesCurveLogLinearInterpolationDmy::~AQLRatesCurveLogLinearInterpolationDmy() 
{
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesCurveLogLinearInterpolationDmy::clone() const
{
    try 
	{
		return new AQLRatesCurveLogLinearInterpolationDmy(*this);
    }
    catch (bad_alloc &e)
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
AQLRatesCurveLogLinearInterpolationDmy::isTypeOf(function_t id) const
{
	return (id==FN_CURVELOGLINEARINTERPOLATIONDMY ? true : AQLRatesCurveLogLinearInterpolation::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesCurveLogLinearInterpolationDmy::getType() const
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
const AQLRatesPathElementBase&
AQLRatesCurveLogLinearInterpolationDmy::value(double t, double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2)
{
	(void)t1, (void)t2;
	if (mValues.end() == mValues.find(t))
	{
		AQLRatesCurveForLogLinearInterpolationDmy &curve = mValues[t];
		curve.set_t(t);
		curve.mpCurve1 = &dynamic_cast<const AQLRatesPathElementCurve&>(val1);
		curve.mpCurve2 = &dynamic_cast<const AQLRatesPathElementCurve&>(val2);
		
		if(curve.mSpreads.empty() && val1.isTypeOf(PE_DDLMMCURVE))
		{
			curve.mSpreads= dynamic_cast<const AQLRatesPathElementDDLMMCurve* >(curve.mpCurve1)->getSpreads();
			curve.volmat = mVol;
		
		}
		else if (!val1.isTypeOf(PE_DDLMMCURVE))
		{
			curve.mSpreads.resize(mVol[0].size(), 0.0);
			curve.volmat = mVol;
		
		}
		
		const DoubleArray& tenor = *(dynamic_cast<const AQLRatesPathElementLMMCurve *>(curve.mpCurve1)->getTenor()); 
		unsigned int tSize = dynamic_cast<const AQLRatesPathElementLMMCurve *>(curve.mpCurve1)->getTenor()->size();
		AQLAlgorithm::locate<DoubleArray, double>(tenor, t, tSize, curve.mfixpos);
	}

	return mValues[t];	
}

//================ AQLRatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy() 
: AQLRatesCurveForLogLinearInterpolation()
{
}


/*!
	@brief destructor
*/
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::~AQLRatesCurveForLogLinearInterpolationDmy() 
{
}

//copy constructor
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy(const AQLRatesCurveForLogLinearInterpolationDmy& v) 
: AQLRatesCurveForLogLinearInterpolation(v)
{

}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORLOGLINEARINTERDMY ? true : AQLRatesCurveForLogLinearInterpolation::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::getType() const
{
	return PE_CURVEFORLOGLINEARINTERDMY;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::clone() const
{
    try 
	{
		return new AQLRatesCurveForLogLinearInterpolationDmy(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy&
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::operator = (const AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy& a)
{
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORLOGLINEARINTERDMY)) 
	{
		AQLString err = "Assignment error for AQLRatesCurveForLogLinearInterpolationDmy : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	
	AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::operator=(a);

	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesCurveLogLinearInterpolationDmy::AQLRatesCurveForLogLinearInterpolationDmy::getP (double T) const
{
	if (mPVal.end() == mPVal.find(T))
	{
		mPVal.insert(make_pair(T, AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::getP(T)));
	}

	return mPVal[T];
}

