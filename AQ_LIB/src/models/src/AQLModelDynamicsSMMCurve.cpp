/*! @file
    @brief Source code of of path element class that represents curve object for swap market model
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "AQLModelDynamicsSMMCurve.h"
#include "AQLAlgorithm.h"

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ AQLRatesPathElementSMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
AQLRatesPathElementSMMCurve::AQLRatesPathElementSMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: AQLRatesPathElementCurve(t), mpRefCount(0), mpTenor(0), mpDeltaTenor(0),
   mpInitialData_S(0), mpInitialData_L(0), mpInitialCurve(0)
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
	if ((*mpTenor)[0] == 0.0)		//if first element is 0, erase it
	{
		mpTenor->erase(mpTenor->begin());
		mpDeltaTenor->erase(mpDeltaTenor->begin());
	}

	mpInitialData_S = new SCALARARRAY(mpTenor->size() - 1);
	mpInitialData_L = new SCALARARRAY(mpTenor->size() - 1);
	set_t();
}


/*!
	@brief destructor
*/
AQLRatesPathElementSMMCurve::~AQLRatesPathElementSMMCurve() 
{
	clear();
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLRatesPathElementSMMCurve::AQLRatesPathElementSMMCurve(const AQLRatesPathElementSMMCurve& v)
: AQLRatesPathElementCurve(v),
mpRefCount(v.mpRefCount), mpDeltaTenor(v.mpDeltaTenor),
mpTenor(v.mpTenor), mpInitialData_S(v.mpInitialData_S), mpInitialData_L(v.mpInitialData_L), mpInitialCurve(v.mpInitialCurve), mData_L(v.mData_L)
{	
	(*mpRefCount)++;
}
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementSMMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_SMMCURVE ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementSMMCurve::getType() const
{
	return PE_SMMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesPathElementSMMCurve::clone() const
{
    try 
	{
		return new AQLRatesPathElementSMMCurve(*this);
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
AQLRatesPathElementSMMCurve&
AQLRatesPathElementSMMCurve::operator = (const AQLRatesPathElementSMMCurve& a)
{
	// 
	if (this == &a) return *this;
	// 
	clear();
	mValue = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mValue;
	mpInitialData_S = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mpInitialData_S;
	mpInitialData_L = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mpInitialData_L;
	mpInitialCurve = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mpInitialCurve;
	mpTenor = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mpTenor;
	mpDeltaTenor = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mpDeltaTenor;
	mpRefCount = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mpRefCount;
	mData_L = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a).mData_L;

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
AQLRatesPathElementSMMCurve::getP (double T) const
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
	
	double lt = 1.0;
	double lt_ = 1.0;
	double end_t = T;
	bool isOnTenor = false;
	int i, j;
	unsigned int pos  = 0;
	unsigned int tSize = mpTenor->size();
	double _T = T > mpTenor->back() ? mpTenor->back() : T;
	AQLAlgorithm::locate<DoubleArray,double>(*mpTenor, _T, tSize, pos);
	if ((*mpTenor)[pos] == _T)
		isOnTenor = true;	


	for (i = pos - 1, j = mValue.size() - (tSize/* - 1*/ - pos); j >= 0; j--, i--)
//    for (i = mpTenor->size() - 1, j = mValue.size(); j >= 0; j--, i--)
	{
/*		if ((*mpTenor)[i] == _T)
		{
			isOnTenor = true;
			continue;
		}*/
		
		if (/*(*mpTenor)[i] < _T && */(*mpTenor)[i] + INFINITESIMAL >= m_t)
		{
			double multiple = (end_t - (*mpTenor)[i]) * (*mpDeltaTenor)[i] / ((*mpTenor)[i + 1] - (*mpTenor)[i]);
			
			lt *= 1.0 + mData_L[j] * multiple;
			
			if (!isOnTenor)
				lt_ *= 1.0 + (*mpInitialData_L)[i] * multiple;
			
			end_t = (*mpTenor)[i];
		}
		else/* if ((*mpTenor)[i] < _T && (*mpTenor)[i] < m_t)*/
		{
			break;
		}
	}

	double ret;
	if (isOnTenor) ret = 1.0 / lt;
	else
	{
		//see "Mind the cap" by Peter jackel
		ret = (lt - 1.0) / (lt_ - 1.0) * (mpInitialCurve->getP(m_t) / mpInitialCurve->getP(_T) - 1.0) + 1.0;
		ret = 1.0 / ret;
	}

	return ret;
}

/*!
    @brief set initial time of this curve
	@param[in] t initial time
*/
void
AQLRatesPathElementSMMCurve::set_t (double t)
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
AQLRatesPathElementSMMCurve::set_t(void)
{
	if (m_t == 0.0)
	{
		mValue.resize(mpTenor->size() - 1);
		mData_L.resize(mpTenor->size() - 1);
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
		mData_L.resize(mpTenor->size() - pos - 1);
	}
	else
	{
		//
		double diff;
		if ((*mpTenor)[pos] - m_t > m_t - (*mpTenor)[pos - 1])
		{
			diff = m_t - (*mpTenor)[pos - 1];
			mValue.resize(mpTenor->size() - pos);
			mData_L.resize(mpTenor->size() - pos);
		}
		else
		{
			diff = (*mpTenor)[pos] - m_t;
			mValue.resize(mpTenor->size() - pos - 1);
			mData_L.resize(mpTenor->size() - pos - 1);
		}

		double criteria = 1.0;
		for (unsigned int i = 1; i < AQLSTRING_DOUBLESIZE; i++)
			criteria *= 0.1;
		
		if (diff > criteria)
		{
			//error
			throw AQLCoreInvalidData("Curve start time is not on tenor grid", __FILE__, __LINE__);
		}
	}
}

/*!
    @brief get initial libor rate
	@param[in] ts start time
	@param[in] te end time
	@param[in] delta delta between ts and te of Libor day count
	@param[in] curve curve object

	@return initial libor rate
*/
double
AQLRatesPathElementSMMCurve::getL (double ts, double te, double delta, const AQLRatesPathElementCurve& curve) const
{
	return (curve.getP(ts) / curve.getP(te) - 1.0) / delta;
}

double
AQLRatesPathElementSMMCurve::getS (DoubleArray tenor, DoubleArray deltatenor, const AQLRatesPathElementCurve& curve) const
{
	double annuity = getA(tenor, deltatenor, curve);
	return (curve.getP(tenor.front()) - curve.getP(tenor.back())) / annuity;
}
/*
double
AQLRatesPathElementSMMCurve::getS (double ts, double te, double annuity, const AQLRatesPathElementCurve& curve) const
{
	return (curve.getP(ts) - curve.getP(te)) / annuity;
}
*/
double
AQLRatesPathElementSMMCurve::getA (DoubleArray tenor, DoubleArray deltatenor, const AQLRatesPathElementCurve& curve) const
{
	double ret = 0.0;
	for (unsigned int i = 1; i < tenor.size(); i++)
		ret += deltatenor[i - 1] * curve.getP(tenor[i]);
	return ret;
}


/*!
    @brief set value
	@param[in] curve curve object
*/	
//void
//AQLRatesPathElementSMMCurve::set(const SCALARARRAY& a) 
//{	
//	int i, j;
//	for (i = a.size() - 1, j = mValue.size() - 1; j >= 0; j--, i--)
//		mValue[j] = a[i];
//	DoubleArray q(mValue.size());
//	//mData_L.resize(mValue.size());
//	for (i = a.size() - 1, j = mValue.size() - 1; j >= 0; j--, i--)
//	{
//		int m, n;	double p = 1.0;
//		for (m = j; m <= mValue.size() - 1; m++)
//		{
//			if (j + 1 <= m)
//				for (n = j + 1; n <= m; n++)
//					p *= q[n];
//			q[j] += (*mpDeltaTenor)[m] * p;
//		}
//		q[j] *= mValue[j];
//		
//		p = 1.0;
//		if (j + 1 <= mValue.size() - 1)
//			for (n = j + 1; n <= mValue.size() - 1; n++)
//				p *= q[n];
//		q[j] += p;
//		q[j] = 1 / q[j];
//		mData_L[j] = (1 / q[j] - 1.0) / (*mpDeltaTenor)[i];
//	}
//}



/*!
    @brief set value
	@param[in] curve curve object
*/	
void
AQLRatesPathElementSMMCurve::set(const AQLRatesPathElementBase& a)
{
	if (a.isTypeOf(PE_SMMCURVE))
	{
		*this = dynamic_cast<const AQLRatesPathElementSMMCurve&>(a);
		return;
	}
	
	AQLRatesPathElementCurve::set(a);
	const AQLRatesPathElementCurve& curve = dynamic_cast<const AQLRatesPathElementCurve&>(a);
	DoubleArray tenor = *mpTenor;
	DoubleArray deltatenor = *mpDeltaTenor;
	
	int i, j;
	if (curve.get_t() == 0.0)//a0
	{
		DoubleArray* p = mpTenor;
		mpTenor = new DoubleArray(*p);
		DoubleArray* p2 = mpDeltaTenor;
		mpDeltaTenor = new DoubleArray(*p2);
		if ((*mpRefCount)-- == 1)
		{
			mpInitialData_S->clear();
			mpInitialData_L->clear();
			p->clear();
			p2->clear();
			delete mpInitialData_S;
			delete mpInitialData_L;
			delete p;
			delete p2;
			delete mpInitialCurve;
			delete mpRefCount;
		}
		
		mpRefCount = new int(1);
		mpInitialCurve = dynamic_cast<AQLRatesPathElementCurve*>(curve.clone());
		mpInitialData_S = new SCALARARRAY(mpDeltaTenor->size());	
		mpInitialData_L = new SCALARARRAY(mpDeltaTenor->size());	
	
		for (i = mValue.size() - mpDeltaTenor->size(), j = 0; j <= mpDeltaTenor->size() - 1;
			 i++, j++, tenor.erase(tenor.begin()), deltatenor.erase(deltatenor.begin()))
		{
			(*mpInitialData_S)[j] = getS(tenor, deltatenor, curve);
			if (i >= 0) 
				mValue[i] = (*mpInitialData_S)[j];
		}

		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; j >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			double te = (*mpTenor)[j + 1];
			(*mpInitialData_L)[j] = getL(ts, te, (*mpDeltaTenor)[j], curve);
			if (i >= 0) mData_L[i] = (*mpInitialData_L)[j];
		}
	
	}
	else //S
	{
		for (i = mValue.size() - mpDeltaTenor->size(), j = 0; j <= mpDeltaTenor->size() - 1;
			 i++, j++, tenor.erase(tenor.begin()), deltatenor.erase(deltatenor.begin()))
		{
			if (i >= 0) 
				mValue[i] = getS(tenor, deltatenor, curve);
		}
		
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; i >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			double te = (*mpTenor)[j + 1];
			mData_L[i] = getL(ts, te, (*mpDeltaTenor)[j], curve);
		}	
	
		// mData_LLibor
		//mData_L.resize(mValue.size());
	/*	for (int i = mValue.size(); i >= 0; i--)
		{
			double p = 1.0;
			double a = 0.0;
			
			for (unsigned int j = i; j < mValue.size() + 1; j++)
			{
				p *= curve.getP(tenor[j]);
				a += deltatenor[j - 1] * curve.getP(tenor[j]);
			}
			p = (1 - mValue[i] * a) / (p + mValue[i] * deltatenor[i - 1]);
			mData_L[i] = (1 / p - 1) / deltatenor[i - 1];
		}
		*/
	}

}

/*!
    @brief clear data
	@param[in] curve curve object
*/	
void
AQLRatesPathElementSMMCurve::clear()
{
	if ((*mpRefCount)-- == 1)
	{
		mpInitialData_S->clear();
		mpInitialData_L->clear();
		mpTenor->clear();
		mpDeltaTenor->clear();
		delete mpInitialData_S;
		delete mpInitialData_L;
		delete mpTenor;
		delete mpDeltaTenor;
		delete mpInitialCurve;
		delete mpRefCount;
		mpInitialData_S = 0;
		mpInitialData_L = 0;
		mpTenor = 0;
		mpDeltaTenor = 0;
		mpInitialCurve = 0;
		mpRefCount = 0;
	}
	mValue.clear();
	mData_L.clear();
}
