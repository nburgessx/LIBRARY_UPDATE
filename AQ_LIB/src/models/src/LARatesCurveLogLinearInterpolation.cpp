/*! @file
    @brief Source code of class of linear interpolation of curve path element


*/
//  2007, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesCurveLogLinearInterpolation.cpp
//
//  SYNOPSIS    :       LARatesCurveLogLinearInterpolation
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


#include "LARatesCurveLogLinearInterpolation.h"
#include "LABasic.h"
#include "LAModelDynamicsLMMCurve.h"
#include "LAModelDynamicsDDLMMCurve.h"
#include "LAAlgorithm.h"

const double INFINITESIMAL = 1E-7;
#define ISINCLUDEVOL

const double LOGFLOOR = 1E-10;
using namespace std;
//================ LARatesCurveLogLinearInterpolation ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
LARatesCurveLogLinearInterpolation::LARatesCurveLogLinearInterpolation()
{
}
/*!
	@brief copy constructor
*/
LARatesCurveLogLinearInterpolation::LARatesCurveLogLinearInterpolation(const LARatesCurveLogLinearInterpolation& v) 
: LARatesPEInterpolationBase(v), mValue(v.mValue), mVol(v.mVol)
{
}

/*!
	@brief destructor
*/
LARatesCurveLogLinearInterpolation::~LARatesCurveLogLinearInterpolation() 
{
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesCurveLogLinearInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveLogLinearInterpolation(*this);
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
LARatesCurveLogLinearInterpolation::isTypeOf(function_t id) const
{
	return (id==FN_CURVELOGLINEARINTERPOLATION ? true : LARatesPEInterpolationBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesCurveLogLinearInterpolation::getType() const
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
const LARatesPathElementBase&
LARatesCurveLogLinearInterpolation::value(double t, double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2)
{
	if (t1 > t2) return value(t, t2, t1, val2, val1);

	mValue.set_t(t);
	mValue.mpCurve1 = &dynamic_cast<const LARatesPathElementCurve&>(val1);
	mValue.mpCurve2 = &dynamic_cast<const LARatesPathElementCurve&>(val2);

	if (val1.isTypeOf(PE_DDLMMCURVE))
	{
		const LARatesPathElementDDLMMCurve *pDDCurve = dynamic_cast<const LARatesPathElementDDLMMCurve* >(mValue.mpCurve1);
		
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
	
	const DoubleArray& tenor = *( dynamic_cast<const LARatesPathElementLMMCurve*>(mValue.mpCurve1)->getTenor()); 
	unsigned int tSize = tenor.size();
	LAAlgorithm::locate<DoubleArray, double>(tenor,t,tSize,mValue.mfixpos);
	
	return mValue;
	
}

//================ LARatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] t start time of this curve 

*/
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::LARatesCurveForLogLinearInterpolation() 
: LARatesPathElementCurve(), mpCurve1(0), mpCurve2(0), mfixpos(0), mSpreadsID(0)
{
	mpDiscount1for = new DoubleVector();
}


/*!
	@brief destructor
*/
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::~LARatesCurveForLogLinearInterpolation() 
{
	delete mpDiscount1for;
}

//copy constructor
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::LARatesCurveForLogLinearInterpolation(const LARatesCurveForLogLinearInterpolation& v) 
: LARatesPathElementCurve(), mSpreads(v.mSpreads), volmat(v.volmat),mfixpos(v.mfixpos), mSpreadsID(v.mSpreadsID)
{
	mpCurve1 = dynamic_cast<const LARatesCurveForLogLinearInterpolation&>(v).mpCurve1;
	mpCurve2 = dynamic_cast<const LARatesCurveForLogLinearInterpolation&>(v).mpCurve2;
	mpDiscount1for = new DoubleVector(*v.mpDiscount1for);
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::isTypeOf(pathelement_t id) const
{
	return (id == PE_CURVEFORLOGLINEARINTER ? true : LARatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::getType() const
{
	return PE_CURVEFORLOGLINEARINTER;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::clone() const
		//20070410--Nagase--g++エラー修正 宣言にthrowを追加
{
    try 
	{
		return new LARatesCurveForLogLinearInterpolation(*this);
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
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation&
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::operator = (const LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation& a)
{
	// 自分自身のコピーはしない
	if (this == &a) return *this;

	if (!a.isTypeOf(PE_CURVEFORLOGLINEARINTER)) 
	{	// タイプチェックの際おかしいことが起こったら
		// 例外発生
		LAString err = "Assignment error for LARatesCurveForLinearInterpolation : from ";
		err += LAString(a.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	
	delete mpDiscount1for;
	// コピー。	
	LARatesPathElementCurve::operator = (a);
	mpCurve1 = dynamic_cast<const LARatesCurveForLogLinearInterpolation&>(a).mpCurve1;
	mpCurve2 = dynamic_cast<const LARatesCurveForLogLinearInterpolation&>(a).mpCurve2;
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
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::getP (double T) const
{
	double t1 = mpCurve1->get_t();
	double t2 = mpCurve2->get_t();
	if (t1 > m_t || t2 < m_t)
	{
		//error
		throw LACoreInvalidData("Condition of t1 <= t <= t2 is not maintain", __FILE__, __LINE__);
	}
	if (T <= m_t) return 1.0;
	
	
	//cache 
	const LARatesPathElementLMMCurve* curve1 = dynamic_cast<const LARatesPathElementLMMCurve*>(mpCurve1);
	
	unsigned int pos = 0;
	const DoubleArray* tenor = curve1->getTenor(); 
	const DoubleArray* deltatenor = curve1->getDeltaTenor(); 
	LAAlgorithm::locate<DoubleArray, double>(*tenor,T,tenor->size(),pos);
	
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
	
	const LARatesPathElementLMMCurve* curve2 = dynamic_cast<const LARatesPathElementLMMCurve*>(mpCurve2);
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
			double L_b = LAMath::max(liborvec_b[bpos], LOGFLOOR);
			double L_a = LAMath::max(liborvec_a[apos], LOGFLOOR);
						
			double logL = weight_a * LAMath::log(L_a) + weight_b * LAMath::log(L_b);
	#ifdef ISINCLUDEVOL
			logL += 0.5 * vol * vol * tdiv;
	#endif
			
			double L = LAMath::exp(logL) - mSpreads[num - 1];
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
		L_a = LAMath::max(L_a, LOGFLOOR);
		L_b = LAMath::max(L_b, LOGFLOOR);
		logL = weight_a * LAMath::log(L_a) + weight_b * LAMath::log(L_b);
	}
	vol = volmat[mfixpos][pos - 1];
	logL += 0.5 * vol * vol * tdiv;
	
	double L = LAMath::exp(logL) - mSpreads[pos - 1];
	ret /= (1.0 + L * delta);
	return ret;
}

void
LARatesCurveLogLinearInterpolation::LARatesCurveForLogLinearInterpolation::set_t(double t)
{
	LARatesPathElementCurve::set_t(t);
	if(t == m_t)
		mpDiscount1for->clear();
}

