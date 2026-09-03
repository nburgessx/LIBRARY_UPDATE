/*! @file
    @brief Source code of of path element class that represents curve object for displaced diffusion libor market model (log type)


*/
// Copyright (C) 2007, Mizuho International London..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "LAModelDynamicsLogDDLMMCurve.h"
#include "LAAlgorithm.h"
#include "LABasic.h"
#include <limits>

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ LARatesPathElementLogDDLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] s spread to L
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
LARatesPathElementLogDDLMMCurve::LARatesPathElementLogDDLMMCurve(double s, const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: LARatesPathElementLMMCurve(tenor, delta_tenor, t), mSpread(s)
{
	;	
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesPathElementLogDDLMMCurve::LARatesPathElementLogDDLMMCurve(const LARatesPathElementLogDDLMMCurve& v)
: LARatesPathElementLMMCurve(v), mSpread(v.mSpread)
{	
}

/*!
	@brief destructor
*/
LARatesPathElementLogDDLMMCurve::~LARatesPathElementLogDDLMMCurve() 
{
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementLogDDLMMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_LOGDDLMMCURVE ? true : LARatesPathElementLMMCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementLogDDLMMCurve::getType() const
{
	return PE_LOGDDLMMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LARatesPathElementBase*	
LARatesPathElementLogDDLMMCurve::clone() const
{
    try 
	{
		return new LARatesPathElementLogDDLMMCurve(*this);
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
LARatesPathElementLogDDLMMCurve&
LARatesPathElementLogDDLMMCurve::operator = (const LARatesPathElementLogDDLMMCurve& a)
{
	mSpread = a.mSpread;
	LARatesPathElementLMMCurve::operator = (a);
	return *this;
}

/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
LARatesPathElementLogDDLMMCurve::getP (double T) const
{
	if (m_t > T + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is before start", __FILE__, __LINE__);
	}
	else if (T > mpTenor->back() + INFINITESIMAL)
	{
		throw LACoreInvalidData("maturity is after last tenor", __FILE__, __LINE__);
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
	LAAlgorithm::locate<DoubleArray,double>(*mpTenor, _T, tSize, pos);
	if ((*mpTenor)[pos] == _T)
		isOnTenor = true;

	for (i = pos - 1, j = mValue.size() - (tSize/* - 1*/ - pos); j >= 0; j--, i--)
	{
/*		if ((*mpTenor)[i] == _T)
		{
			isOnTenor = true;
			continue;
		}*/
		if (/*(*mpTenor)[i] < _T && */(*mpTenor)[i] + INFINITESIMAL >= m_t)
		{
			double multiple = (end_t - (*mpTenor)[i]) * (*mpDeltaTenor)[i] / ((*mpTenor)[i + 1] - (*mpTenor)[i]);
			
			lt *= 1.0 + LAMath::exp(mValue[j] - mSpread) * multiple;
			
			if (!isOnTenor)
				lt_ *= 1.0 + (*mpInitialData_L)[i] * multiple;
			
			end_t = (*mpTenor)[i];
		}
		else /*if ((*mpTenor)[i] < _T && (*mpTenor)[i] < m_t)*/
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
//		ret = (lt - 1.0) / (lt_ - 1.0) * (mpInitialCurve->getP(m_t) / mpInitialCurve->getP(_T) - 1.0) + 1;
		ret = 1.0 / ret;
	}

	return ret;
}




/*!
    @brief set value
	@param[in] curve curve object
*/	
void
LARatesPathElementLogDDLMMCurve::set(const LARatesPathElementBase& a)
{
	LARatesPathElementLMMCurve::set(a);
	unsigned int size = mValue.size();
	for (unsigned int i = 0; i < size; i++)
	{
		if (mValue[i] + mSpread > 0.0) mValue[i] = LAMath::log(mValue[i] + mSpread);
		else mValue[i] = -std::numeric_limits<SCALAR>::max();
	}
}



