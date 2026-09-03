/*! @file
    @brief Source code of of path element class that represents curve object for HJM


*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif
#define GAUSSLEGENDREPOINTNUM 20


#include "AQLModelDynamicsHJMCurve.h"
#include "AQLAlgorithm.h"
#include "AQL1DDataSet.h"
#include "AQLGaussLegendre.h"
#include "AQLBasic.h"

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ AQLRatesPathElementHJMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
AQLRatesPathElementHJMCurve::AQLRatesPathElementHJMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: AQLRatesPathElementCurve(t), mpRefCount(0), mpTenor(0), mpDeltaTenor(0),
   mpInitialData_L(0), mpInitialCurve(0)
{
	if (tenor.size() <= 1 || (tenor[0] == 0.0 && tenor.size() <= 2))
	{
		//error
		throw AQLCoreInvalidData("tenor size is one", __FILE__, __LINE__);
	}
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw AQLCoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
	}

	mpRefCount = new int(1);
	mpTenor = new DoubleArray(tenor);	
	mpDeltaTenor = new DoubleArray(delta_tenor);
	if ((*mpTenor)[0] == 0.0)//if first element is 0, erase it
	{
		mpTenor->erase(mpTenor->begin());
		mpDeltaTenor->erase(mpDeltaTenor->begin());
	}

	mpInitialData_L = new SCALARARRAY(mpTenor->size() - 1);
	set_t();
}

/*!
	@brief destructor
*/
AQLRatesPathElementHJMCurve::~AQLRatesPathElementHJMCurve() 
{
	clear();
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLRatesPathElementHJMCurve::AQLRatesPathElementHJMCurve(const AQLRatesPathElementHJMCurve& v)
: AQLRatesPathElementCurve(v),
mpRefCount(v.mpRefCount), mpDeltaTenor(v.mpDeltaTenor),
   mpTenor(v.mpTenor), mpInitialData_L(v.mpInitialData_L), mpInitialCurve(v.mpInitialCurve)
{	
	(*mpRefCount)++;
}
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementHJMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_HJMCURVE ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementHJMCurve::getType() const
{
	return PE_HJMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesPathElementHJMCurve::clone() const
{
    try 
	{
		return new AQLRatesPathElementHJMCurve(*this);
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
AQLRatesPathElementHJMCurve&
AQLRatesPathElementHJMCurve::operator = (const AQLRatesPathElementHJMCurve& a)
{
	// 
	if (this == &a) return *this;

/*	if (!a.isTypeOf(PE_HJMCURVE)) 
	{	// 
		// 
		AQLString err = "Assignment error for AQLRatesPathElementHJMCurve : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}*/

	// 
	clear();
	
	mValue = dynamic_cast<const AQLRatesPathElementHJMCurve&>(a).mValue;
	mpInitialData_L = dynamic_cast<const AQLRatesPathElementHJMCurve&>(a).mpInitialData_L;
	mpInitialCurve = dynamic_cast<const AQLRatesPathElementHJMCurve&>(a).mpInitialCurve;
	mpTenor = dynamic_cast<const AQLRatesPathElementHJMCurve&>(a).mpTenor;
	mpDeltaTenor = dynamic_cast<const AQLRatesPathElementHJMCurve&>(a).mpDeltaTenor;
	mpRefCount = dynamic_cast<const AQLRatesPathElementHJMCurve&>(a).mpRefCount;

	(*mpRefCount)++;

	AQLRatesPathElementCurve::operator = (a);
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesPathElementHJMCurve::getP (double T) const
{
	if (m_t > T + INFINITESIMAL)
	{
		throw AQLCoreInvalidData("maturity is before start", __FILE__, __LINE__);
	}
	else if (T > mpTenor->back() + INFINITESIMAL)
	{
		throw AQLCoreInvalidData("maturity is after last tenor", __FILE__, __LINE__);
	}
	else if (m_t >= T) return 1.0;
	else if (m_t == 0.0) return mpInitialCurve->getP(T);
	

	unsigned int pos  = 0;
	AQLGaussLegendre GL(GAUSSLEGENDREPOINTNUM);// integral method

	AQLAlgorithm::locate<DoubleArray,double>(*mpTenor, m_t, mpTenor->size(), pos);
	
	DoubleArray tenor = *mpTenor;
	tenor.erase(tenor.begin(), tenor.begin() + pos + 1);

	AQL1DDataSet method;
	//method.setParam(mValue);
	method.set(tenor, mValue);
	method.setInterpolation(*mpInter);
	
	double ret = method.integral(m_t, T, &GL);
	ret = AQLMath::exp(-ret * (T - m_t));

	return ret;	//method.integral(m_t, T, &GL);

}

/*!
    @brief set initial time of this curve
	@param[in] t initial time
*/
void
AQLRatesPathElementHJMCurve::set_t (double t)
{
	if (m_t == t) return;
	AQLRatesPathElementCurve::set_t(t);
	mValue.clear();
	set_t();
}

/*!
    @brief set initial time of this curve
	
*/
void
AQLRatesPathElementHJMCurve::set_t(void)
{
	if (m_t == 0.0)
	{
		mValue.resize(mpTenor->size() - 1);
		return;
	}	
	unsigned int pos;
	AQLAlgorithm::locate<DoubleArray, double>(*mpTenor, m_t, mpTenor->size(), pos);
	if (pos == mpTenor->size())
	{
		throw AQLCoreInvalidData("curve start time is after last tenor", __FILE__, __LINE__);
	}
	else if ((*mpTenor)[pos] == m_t)
	{
		mValue.resize(mpTenor->size() - pos - 1);
	}
	else
	{
		//
		double diff;
		if ((*mpTenor)[pos] - m_t > m_t - (*mpTenor)[pos - 1])
		{
			diff = m_t - (*mpTenor)[pos - 1];
			mValue.resize(mpTenor->size() - pos);
		}
		else
		{
			diff = (*mpTenor)[pos] - m_t;
			mValue.resize(mpTenor->size() - pos - 1);
		}
		
		if (diff > INFINITESIMAL)
		{
			//error
			throw AQLCoreInvalidData("Curve start time is not on tenor grid", __FILE__, __LINE__);
		}
	}
	
}


/*!
    @brief set value
	@param[in] curve curve object
*/	
void
AQLRatesPathElementHJMCurve::set(const AQLRatesPathElementBase& a)
{
	if (a.isTypeOf(PE_HJMCURVE))
	{
		*this = dynamic_cast<const AQLRatesPathElementHJMCurve&>(a);
		return;
	}
	
	AQLRatesPathElementCurve::set(a);
	const AQLRatesPathElementCurve& curve = dynamic_cast<const AQLRatesPathElementCurve&>(a);
	
	

	if (curve.get_t() == 0.0)//a0
	{
		DoubleArray* p = mpTenor;
		mpTenor = new DoubleArray(*p);
		DoubleArray* p2 = mpDeltaTenor;
		mpDeltaTenor = new DoubleArray(*p2);
		if ((*mpRefCount)-- == 1)
		{
			mpInitialData_L->clear();
			p->clear();
			p2->clear();
			delete mpInitialData_L;
			delete p;
			delete p2;
			delete mpInitialCurve;
			delete mpRefCount;
		}
		
		mpRefCount = new int(1);
		mpInitialCurve = dynamic_cast<AQLRatesPathElementCurve*>(curve.clone());
		mpInitialData_L = new SCALARARRAY(mpDeltaTenor->size());	
	
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; j >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			//double te = (*mpTenor)[j + 1];
			(*mpInitialData_L)[j] = curve.getF(ts);
			if (i >= 0) mValue[i] = (*mpInitialData_L)[j];
		}
	}
	else //L
	{
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; i >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			//double te = (*mpTenor)[j + 1];
			mValue[i] = curve.getF(ts);
		}	
	}

}
void
AQLRatesPathElementHJMCurve::setInterpolationMethod(AQLInterpolationBase* pinter) 
{
	mpInter = pinter; 
}

/*!
    @brief clear data
	@param[in] curve curve object
*/	
void
AQLRatesPathElementHJMCurve::clear()
{
	if ((*mpRefCount)-- == 1)
	{
		mpInitialData_L->clear();
		mpTenor->clear();
		mpDeltaTenor->clear();
		delete mpInitialData_L;
		delete mpTenor;
		delete mpDeltaTenor;
		delete mpInitialCurve;
		delete mpRefCount;
		mpInitialData_L = 0;
		mpTenor = 0;
		mpDeltaTenor = 0;
		mpInitialCurve = 0;
		mpRefCount = 0;
	}
	mValue.clear();
}
