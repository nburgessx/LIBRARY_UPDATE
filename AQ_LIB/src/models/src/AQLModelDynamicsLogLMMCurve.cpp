/*! @file
    @brief Source code of of path element class that represents curve object for libor market model (log type)
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif



#include "AQLModelDynamicsLogLMMCurve.h"
#include "AQLAlgorithm.h"
#include "AQLBasic.h"
#include <limits>

using namespace std;
const double INFINITESIMAL = 1E-7; 
//================ AQLRatesPathElementLogLMMCurve ===================================
/*!
	@brief default constructor
	@param[in] tenor tenor
	@param[in] delta_tenor delta tenor
	@param[in] t start time of this curve 

*/
AQLRatesPathElementLogLMMCurve::AQLRatesPathElementLogLMMCurve(const DoubleArray& tenor, const DoubleArray& delta_tenor, double t) 
: AQLRatesPathElementLMMCurve(tenor, delta_tenor, t)
{

}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLRatesPathElementLogLMMCurve::AQLRatesPathElementLogLMMCurve(const AQLRatesPathElementLogLMMCurve& v)
: AQLRatesPathElementLMMCurve(v)
{	
}

/*!
	@brief destructor
*/
AQLRatesPathElementLogLMMCurve::~AQLRatesPathElementLogLMMCurve() 
{

}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
/*AQLRatesPathElementLogLMMCurve::AQLRatesPathElementLogLMMCurve(const AQLRatesPathElementLogLMMCurve& v)
: AQLRatesPathElementCurve(v),
mpRefCount(v.mpRefCount), mpDeltaTenor(v.mpDeltaTenor),
   mpTenor(v.mpTenor), mpInitialData_L(v.mpInitialData_L), mpInitialCurve(v.mpInitialCurve)
{	
	(*mpRefCount)++;
}*/
/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementLogLMMCurve::isTypeOf(pathelement_t id) const
{
	return (id == PE_LOGLMMCURVE ? true : AQLRatesPathElementLMMCurve::isTypeOf(id));
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementLogLMMCurve::getType() const
{
	return PE_LOGLMMCURVE;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLRatesPathElementBase*	
AQLRatesPathElementLogLMMCurve::clone() const
{
    try 
	{
		return new AQLRatesPathElementLogLMMCurve(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}



/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesPathElementLogLMMCurve::getP (double T) const
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
	{
/*		if ((*mpTenor)[i] == _T)
		{
			isOnTenor = true;
			continue;
		}*/
		if (/*(*mpTenor)[i] < _T && */(*mpTenor)[i] + INFINITESIMAL >= m_t)
		{
			double multiple = (end_t - (*mpTenor)[i]) * (*mpDeltaTenor)[i] / ((*mpTenor)[i + 1] - (*mpTenor)[i]);
			
			lt *= 1.0 + AQLMath::exp(mValue[j]) * multiple;
			
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
    @brief set value
	@param[in] curve curve object
*/	
void
AQLRatesPathElementLogLMMCurve::set(const AQLRatesPathElementBase& a)
{
	AQLRatesPathElementLMMCurve::set(a);
	unsigned int size = mValue.size();
	for (unsigned int i = 0; i < size; i++)
	{
		if (mValue[i] > 0.0) mValue[i] = AQLMath::log(mValue[i]);
		else mValue[i] = -std::numeric_limits<SCALAR>::max();
	}
}

