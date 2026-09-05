/*! @file
    @brief Source code of class of linear interpolation of curve path element
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesCurveLogLinearInterpolation.h"
#include "AQLBasic.h"
#include "AQLModelDynamicsLMMCurve.h"
#include "AQLModelDynamicsDDLMMCurve.h"
#include "AQLAlgorithm.h"

const double INFINITESIMAL = 1E-7;
#define ISINCLUDEVOL

const double LOGFLOOR = 1E-10;
using namespace std;
//================ AQLRatesCurveLogLinearInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveLogLinearInterpolation()
{
}
/*!
	@brief copy constructor
*/
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveLogLinearInterpolation(const AQLRatesCurveLogLinearInterpolation& v) 
: AQLRatesPEInterpolationBase(v), mValue(v.mValue), mVol(v.mVol)
{
}

/*!
	@brief destructor
*/
AQLRatesCurveLogLinearInterpolation::~AQLRatesCurveLogLinearInterpolation() 
{
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesCurveLogLinearInterpolation::clone() const
		// g++ throw
{
    try 
	{
		return new AQLRatesCurveLogLinearInterpolation(*this);
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
AQLRatesCurveLogLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVELOGLINEARINTERPOLATION ? true : AQLRatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesCurveLogLinearInterpolation::getType() const
{
	return FN_CURVELOGLINEARINTERPOLATION;
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
AQLRatesCurveLogLinearInterpolation::value(double t, double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2)
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const AQLRatesPathElementCurve&>(val1);
	mValue.mpCurve2 = &dynamic_cast<const AQLRatesPathElementCurve&>(val2);

	if (val1.isTypeOf(PE_DDLMMCURVE))
	{
		const AQLRatesPathElementDDLMMCurve *pDDCurve = dynamic_cast<const AQLRatesPathElementDDLMMCurve* >(mValue.mpCurve1);
		
		if (mValue.mSpreadsID != pDDCurve->getSpreadsID())
		{
			mValue.mSpreads = pDDCurve->getSpreads();
			mValue.volmat = mVol;
			mValue.mSpreadsID = pDDCurve->getSpreadsID();
		}
	}
	else if (!val1.isTypeOf(PE_DDLMMCURVE))
	{
		mValue.mSpreads.resize(mVol[0].size(), 0.0);
		mValue.volmat = mVol;
	
	}
	
	const DoubleArray& tenor = *( dynamic_cast<const AQLRatesPathElementLMMCurve*>(mValue.mpCurve1)->getTenor()); 
	unsigned int tSize = tenor.size();
	AQLAlgorithm::locate<DoubleArray, double>(tenor,t,tSize,mValue.mfixpos);
	
	return mValue;
	
}

//================ AQLRatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation() 
: AQLRatesPathElementCurve(), mpCurve1(0), mpCurve2(0), mfixpos(0), mSpreadsID(0)
{
	mpDiscount1for = new DoubleVector();
}


/*!
	@brief destructor
*/
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::~AQLRatesCurveForLogLinearInterpolation() 
{
	delete mpDiscount1for;
}

//copy constructor
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation(const AQLRatesCurveForLogLinearInterpolation& v) 
: AQLRatesPathElementCurve(), mSpreads(v.mSpreads), volmat(v.volmat),mfixpos(v.mfixpos), mSpreadsID(v.mSpreadsID)
{
	mpCurve1 = dynamic_cast<const AQLRatesCurveForLogLinearInterpolation&>(v).mpCurve1;
	mpCurve2 = dynamic_cast<const AQLRatesCurveForLogLinearInterpolation&>(v).mpCurve2;
	mpDiscount1for = new DoubleVector(*v.mpDiscount1for);
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORLOGLINEARINTER ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::getType() const
{
	return PE_CURVEFORLOGLINEARINTER;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::clone() const
		// g++ throw
{
    try 
	{
		return new AQLRatesCurveForLogLinearInterpolation(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief equal operator
    @param[in] a source object

    @return this object
*/
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation&
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::operator = (const AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation& a)
{
	// 
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORLOGLINEARINTER)) 
	{	// 
		// 
		AQLString err = "Assignment error for AQLRatesCurveForLinearInterpolation : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	
	delete mpDiscount1for;
	// 	
	AQLRatesPathElementCurve::operator = (a);
	mpCurve1 = dynamic_cast<const AQLRatesCurveForLogLinearInterpolation&>(a).mpCurve1;
	mpCurve2 = dynamic_cast<const AQLRatesCurveForLogLinearInterpolation&>(a).mpCurve2;
	mpDiscount1for = new DoubleVector(*a.mpDiscount1for);
	mSpreads = a.mSpreads;
	volmat = a.volmat;
	mfixpos = a.mfixpos;
	mSpreadsID = a.mSpreadsID;

	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::getP (double T) const
{
	double t1 = mpCurve1->get_t();
	double t2 = mpCurve2->get_t();
	if (t1 > m_t || t2 < m_t)
	{
		//error
		throw AQLCoreInvalidData("Condition of t1 <= t <= t2 is not maintain", __FILE__, __LINE__);
	}
	if (T <= m_t) return 1.0;
	
	
	//cache 
	const AQLRatesPathElementLMMCurve* curve1 = dynamic_cast<const AQLRatesPathElementLMMCurve*>(mpCurve1);
	
	unsigned int pos = 0;
	const DoubleArray* tenor = curve1->getTenor(); 
	const DoubleArray* deltatenor = curve1->getDeltaTenor(); 
	AQLAlgorithm::locate<DoubleArray, double>(*tenor,T,tenor->size(),pos);
	
	double ret = 1.0;
	if (pos == mfixpos)
		return curve1->getP(T) / curve1->getP(m_t);
	
	
	unsigned int sizeCache = mpDiscount1for->size();
	if (0 == sizeCache && pos != 0)
	{
		if (mfixpos > 0)
		{
			for (unsigned int i = 0; i < mfixpos; i++)
				(*mpDiscount1for).push_back(1.0);
		}
		double df = curve1->getP(t2) / curve1->getP(m_t);
		(*mpDiscount1for).push_back(df);
	}
	
	const AQLRatesPathElementLMMCurve* curve2 = dynamic_cast<const AQLRatesPathElementLMMCurve*>(mpCurve2);
	unsigned int num = mpDiscount1for->size();
	double t_diff = t2 - t1;
	double mt_diff1 = m_t - t1;
	double mt_diff2 = t2 - m_t;

	double tdiv = mt_diff2 * mt_diff1 / t_diff;
	double weight_a = mt_diff1 / t_diff;
	double weight_b = mt_diff2 / t_diff;
	double vol = volmat[mfixpos][num - 1];
	if (num < pos && pos >= 2)
	{
		const DoubleArray& liborvec_b = curve1->get();
		const DoubleArray& liborvec_a = curve2->get();
		unsigned int bpos = num - 1 - mfixpos;
		unsigned int apos = bpos;
		if (liborvec_b.size() == liborvec_a.size() + 1)
			bpos = apos + 1;
		while (num < pos && pos >= 2)
		{			
			double L_b = AQLMath::max(liborvec_b[bpos], LOGFLOOR);
			double L_a = AQLMath::max(liborvec_a[apos], LOGFLOOR);
						
			double logL = weight_a * AQLMath::log(L_a) + weight_b * AQLMath::log(L_b);
	#ifdef ISINCLUDEVOL
			logL += 0.5 * vol * vol * tdiv;
	#endif
			
			double L = AQLMath::exp(logL) - mSpreads[num - 1];
			double df = (*mpDiscount1for)[num - 1] / (1.0 + L * (*deltatenor)[num - 1]);
			(*mpDiscount1for).push_back(df);
			
			num++;
			bpos++;
			apos++;
			vol = volmat[mfixpos][num - 1];
		}
	}

	ret = (*mpDiscount1for)[pos - 1];
	double T_b = (*tenor)[pos - 1];
	double delta = (*deltatenor)[pos - 1] * (T - T_b) / ((*tenor)[pos] - (*tenor)[pos - 1]);
	double L_b = curve1->getL(T_b, T, delta, *curve1) + mSpreads[pos - 1];
	double logL = 0.0;
	if (T <= T_b + INFINITESIMAL)
	{	
		return ret;
	}
	else
	{
		double L_a = curve2->getL(T_b, T, delta, *curve2) + mSpreads[pos - 1];
		L_a = AQLMath::max(L_a, LOGFLOOR);
		L_b = AQLMath::max(L_b, LOGFLOOR);
		logL = weight_a * AQLMath::log(L_a) + weight_b * AQLMath::log(L_b);
	}
	vol = volmat[mfixpos][pos - 1];
	logL += 0.5 * vol * vol * tdiv;
	
	double L = AQLMath::exp(logL) - mSpreads[pos - 1];
	ret /= (1.0 + L * delta);
	return ret;
}

void
AQLRatesCurveLogLinearInterpolation::AQLRatesCurveForLogLinearInterpolation::set_t(double t)
{
	AQLRatesPathElementCurve::set_t(t);
	if(t == m_t)
		mpDiscount1for->clear();
}

