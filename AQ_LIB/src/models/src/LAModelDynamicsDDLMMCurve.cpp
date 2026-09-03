/*! @file
    @brief Source code of of path element class that represents curve object for displaced diffusion libor market model


*/
// Copyright (C) 2007, Mizuho International London..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



const double INFINITESIMAL = 1E-7;
const double TRANCATEINF = 1E-10; 
//const double TRANCATEINF = -1E10;
#include "LAModelDynamicsDDLMMCurve.h"
#include "LAAlgorithm.h"

unsigned int  LARatesPathElementDDLMMCurve::mSpreadsID = 0;

using namespace std;

//================ LARatesPathElementDDLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] s spread to L
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
LARatesPathElementDDLMMCurve::LARatesPathElementDDLMMCurve(double s, double constShift, const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: LARatesPathElementLMMCurve(tenor, delta_tenor, t), mSpread(s), mConstShift(constShift), mTrancateVal(TRANCATEINF), mSpreads(DoubleArray(mValue.size(), s))
{	
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementDDLMMCurve::LARatesPathElementDDLMMCurve(const LARatesPathElementDDLMMCurve& v)
: LARatesPathElementLMMCurve(v), mSpreads(v.mSpreads), mTrancateVal(v.mTrancateVal), mSpread(v.mSpread), mConstShift(v.mConstShift)
{	
}

/*!
	@brief destructor
*/
LARatesPathElementDDLMMCurve::~LARatesPathElementDDLMMCurve() 
{
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementDDLMMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_DDLMMCURVE ? true : LARatesPathElementLMMCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementDDLMMCurve::getType() const
{
	return PE_DDLMMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesPathElementDDLMMCurve::clone() const
{
    try 
	{
		return new LARatesPathElementDDLMMCurve(*this);
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
LARatesPathElementDDLMMCurve&
LARatesPathElementDDLMMCurve::operator = (const LARatesPathElementDDLMMCurve& a)
{
	mSpread = a.mSpread;
	LARatesPathElementLMMCurve::operator = (a);
	return *this;
}

/*!
    @brief set value
	@param[in] curve curve object
*/

void
LARatesPathElementDDLMMCurve::set(const LARatesPathElementBase& a)
{
	LARatesPathElementLMMCurve::set(a);
	unsigned int size = mValue.size();
	if(0 == m_t)
	{
		mSpreads.resize(size);
		for (unsigned int i = 0; i < size; i++)
		{
			//(*mpInitialData_L)[i] = LAMath::max((*mpInitialData_L)[i], mTrancateVal);
			mSpreads[i] = (*mpInitialData_L)[i] * mSpread + mConstShift * (1. + mSpread); 
		}

		++mSpreadsID;
	}

	for (unsigned int i = 0; i < size; i++)
	{
		mValue[i] = LAMath::max(mValue[i] + mSpreads[i], mTrancateVal);
	}
	
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
LARatesPathElementDDLMMCurve::getP (double T) const
{
	if (m_t > T + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is before start", __FILE__, __LINE__);
	}
	else if (T > mpTenor->back() + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is after last tenor", __FILE__, __LINE__);
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
	LAAlgorithm::locate<DoubleArray,double>(*mpTenor, _T, tSize, pos);
	if ((*mpTenor)[pos] == _T)
		isOnTenor = true;


	double f_multiple = 0.0;
	double f_lt = 1.0;
	double f_lt_ = 1.0;

	unsigned int j_ = mValue.size() - (tSize - pos);
	if (j_ >= 0)
	{
		unsigned int i_ = pos - 1;
		if ((*mpTenor)[i_] + INFINITESIMAL >= m_t)
		{
			f_multiple = (end_t - (*mpTenor)[i_]) * (*mpDeltaTenor)[i_] / ((*mpTenor)[i_ + 1] - (*mpTenor)[i_]);
			f_lt *= 1.0 + (mValue[j_]-mSpreads[i_]) * f_multiple;
			f_lt_ *= 1.0 + (*mpInitialData_L)[i_] * f_multiple;
		}
	}
	
	int j_lim = 0;
	if (mPos <= pos)
	{
		lt *= mlt;
		lt_ *= mlt_;
		j_lim = mValue.size() - (tSize - mPos);
	}
	else
	{
		j_lim = 0;
	}

	end_t = (*mpTenor)[pos - 1];
	for (i = pos - 2 , j = mValue.size() - (tSize - pos) - 1; j >= j_lim; j--, i--)
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
			
			lt *= 1.0 + (mValue[j]-mSpreads[i]) * multiple;
			
			//if (!isOnTenor)
				lt_ *= 1.0 + (*mpInitialData_L)[i] * multiple;
			
			end_t = (*mpTenor)[i];
		}
		else/* if ((*mpTenor)[i] < _T && (*mpTenor)[i] < m_t)*/
		{
			break;
		}
	}

	mlt = lt;
	mlt_ = lt_;
	mPos = pos;

	lt *= f_lt;
	lt_ *= f_lt_;

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
		//ret = (lt - 1.0) / (lt_ - 1.0) * (mtP / P - 1.0) + 1.0;
		ret = lt / lt_ * (mtP / P);
		//ret = (lt - 1.0) / (lt_ - 1.0) * (mpInitialCurve->getP(m_t) / mpInitialCurve->getP(_T) - 1.0) + 1.0;
		ret = 1.0 / ret;
	}

	return ret;
}

