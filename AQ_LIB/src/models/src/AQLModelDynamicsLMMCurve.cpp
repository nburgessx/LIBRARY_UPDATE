/*! @file
    @brief Source code of of path element class that represents curve object for libor market model


*/
// Copyright (C) 2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "AQLModelDynamicsLMMCurve.h"
#include "AQLAlgorithm.h"

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ AQLRatesPathElementLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
AQLRatesPathElementLMMCurve::AQLRatesPathElementLMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: AQLRatesPathElementCurve(t), mpRefCount(0), mpTenor(0), mpDeltaTenor(0),
   mpInitialData_L(0), mpInitialCurve(0), mtP(0.0), mlt(0.0), mlt_(0.0), mPos(10000)
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
	mpInitialCache = new map<double, double>();
	if ((*mpTenor)[0] == 0.0)//if first element is 0, erase it
	{
		mpTenor->erase(mpTenor->begin());
		mpDeltaTenor->erase(mpDeltaTenor->begin());
	}

	mpInitialData_L = new SCALARARRAY(mpTenor->size() - 1);

	if (m_t == 0.0)
	{
		mtP = 1.0;
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

/*		double criteria = 1.0;
		for (unsigned int i = 1; i < AQLSTRING_DOUBLESIZE; i++)
			criteria *= 0.1;
*/		
		if (diff > INFINITESIMAL)
		{
			//error
			throw AQLCoreInvalidData("Curve start time is not on tenor grid", __FILE__, __LINE__);
		}
	}
	
}


/*!
	@brief destructor
*/
AQLRatesPathElementLMMCurve::~AQLRatesPathElementLMMCurve() 
{
	clear();
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLRatesPathElementLMMCurve::AQLRatesPathElementLMMCurve(const AQLRatesPathElementLMMCurve& v)
: AQLRatesPathElementCurve(v),
mpRefCount(v.mpRefCount), mpDeltaTenor(v.mpDeltaTenor),
mpTenor(v.mpTenor), mpInitialData_L(v.mpInitialData_L), mpInitialCurve(v.mpInitialCurve), mpInitialCache(v.mpInitialCache), mtP(v.mtP),
mlt(v.mlt), mlt_(v.mlt_), mPos(v.mPos) 
{	
	(*mpRefCount)++;
}
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementLMMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_LMMCURVE ? true : AQLRatesPathElementCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementLMMCurve::getType() const
{
	return PE_LMMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesPathElementLMMCurve::clone() const
{
    try 
	{
		return new AQLRatesPathElementLMMCurve(*this);
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
AQLRatesPathElementLMMCurve&
AQLRatesPathElementLMMCurve::operator = (const AQLRatesPathElementLMMCurve& a)
{
	// 
	if (this == &a) return *this;

/*	if (!a.isTypeOf(PE_LMMCURVE)) 
	{	// 
		// 
		AQLString err = "Assignment error for AQLRatesPathElementLMMCurve : from ";
		err += AQLString(a.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}*/

	// 
	clear();
	
	mValue = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mValue;
	mpInitialData_L = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mpInitialData_L;
	mpInitialCurve = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mpInitialCurve;
	mpTenor = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mpTenor;
	mpDeltaTenor = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mpDeltaTenor;
	mpRefCount = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mpRefCount;
	mpInitialCache = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mpInitialCache;
	mtP = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mtP;
	mlt = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mlt;
	mlt_ = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mlt_;
	mPos = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a).mPos;

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
AQLRatesPathElementLMMCurve::getP (double T) const
{
	if (m_t > T + INFINITESIMAL)
	{
		throw AQLCoreInvalidData("maturity is before start", __FILE__, __LINE__);
	}
	else if (T > mpTenor->back() + INFINITESIMAL)
	{
		throw AQLCoreInvalidData("maturity is after last tenor", __FILE__, __LINE__);
	}
	else if (m_t + INFINITESIMAL >= T) return 1.0;
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
			
			lt *= 1.0 + mValue[j] * multiple;
			
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
		double P = 0.0;
		map<double, double>::iterator it = mpInitialCache->find(_T);
		if (it != mpInitialCache->end())
		{
			P = it->second;
		}
		else
		{
			P = mpInitialCurve->getP(_T);
			mpInitialCache->insert(make_pair(_T, P));
		}
		ret = (lt - 1.0) / (lt_ - 1.0) * (mtP / P - 1.0) + 1.0;		
		//ret = (lt - 1.0) / (lt_ - 1.0) * (mpInitialCurve->getP(m_t) / mpInitialCurve->getP(_T) - 1.0) + 1.0;
		ret = 1.0 / ret;
	}

	return ret;
}

/*!
    @brief set initial time of this curve
	@param[in] t initial time
*/
void
AQLRatesPathElementLMMCurve::set_t (double t)
{
	if (m_t == t) return;
	AQLRatesPathElementCurve::set_t(t);
	mValue.clear();
	if (mpInitialCurve)
	{
		mtP = mpInitialCurve->getP(m_t);
	}
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

/*		double criteria = 1.0;
		for (unsigned int i = 1; i < AQLSTRING_DOUBLESIZE; i++)
			criteria *= 0.1;
*/		
		if (diff > INFINITESIMAL)
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
AQLRatesPathElementLMMCurve::getL (double ts, double te, double delta, const AQLRatesPathElementCurve& curve) const
{
	return (curve.getP(ts) / curve.getP(te) - 1.0) / delta;
}


/*!
    @brief set value
	@param[in] curve curve object
*/	
void
AQLRatesPathElementLMMCurve::set(const AQLRatesPathElementBase& a)
{
	if (a.isTypeOf(PE_LMMCURVE))
	{
		*this = dynamic_cast<const AQLRatesPathElementLMMCurve&>(a);
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
			delete mpInitialCache;
		}
		
		mpRefCount = new int(1);
		mpInitialCurve = dynamic_cast<AQLRatesPathElementCurve*>(curve.clone());
		mpInitialData_L = new SCALARARRAY(mpDeltaTenor->size());	
		mpInitialCache = new map<double, double>();
		mtP = mpInitialCurve->getP(m_t);
	
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; j >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			double te = (*mpTenor)[j + 1];
			(*mpInitialData_L)[j] = getL(ts, te, (*mpDeltaTenor)[j], curve);
			if (i >= 0) mValue[i] = (*mpInitialData_L)[j];
		}
	}
	else //L
	{
		int i, j;
		for (i = mValue.size() - 1, j = mpDeltaTenor->size() - 1; i >= 0; i--, j--)
		{
			double ts = (*mpTenor)[j];
			double te = (*mpTenor)[j + 1];
			mValue[i] = getL(ts, te, (*mpDeltaTenor)[j], curve);
		}	
	}

}

/*!
    @brief clear data
	@param[in] curve curve object
*/	
void
AQLRatesPathElementLMMCurve::clear()
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
		delete mpInitialCache;
		mpInitialData_L = 0;
		mpTenor = 0;
		mpDeltaTenor = 0;
		mpInitialCurve = 0;
		mpRefCount = 0;
	}
	mValue.clear();
}
