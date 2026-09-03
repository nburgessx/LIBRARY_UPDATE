/*! @file
    @brief Source code of abstract base class for one dimensional integration

	This class derives from LAIntegralBase.
*/



////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LA1DIntegral.cpp
//
//  SYNOPSIS    :       LA1DIntegral
//  DESCRIPTION :       Source code of abstract base class for one dimensional integration
//						This class derives from LAIntegralBase
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


#include "LA1DIntegral.h"
//#include "LAAlgorithm.h"
#include <algorithm>

using namespace std;
//================ LA1DIntegral ===================================
/*!
	@brief Constructor
*/
LA1DIntegral::LA1DIntegral()
: LAIntegralBase()
{
}

/*!
	@brief Destructor
*/
LA1DIntegral::~LA1DIntegral() 
{
}

/*!
    @brief Check function for this class type
    @param[in] id class type to check
    @return true or false
*/
bool
LA1DIntegral::isTypeOf(function_t id) const
{
    return (id == FN_1DINTEGRAL ? true : LAIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
LA1DIntegral::getType() const
{
    return FN_1DINTEGRAL;  
}

/*!
    @brief  method for integration, which is inherited from base class

    @param[in] f target function
	@param[in] x integral region

	@return integral result
*/
double
LA1DIntegral::integrate(const LAFunctionBase& f,
								const std::vector<std::pair<double,double> >& x)const
{
	if (x.size() == 1)return integrate(f, x[0].first, x[0].second);

	MMInnerFunction innerfunc(this);
	innerfunc.setFunc(f);
	innerfunc.setIntegralRegion(x[x.size()-1].first, x[x.size()-1].second);
	for (int i = x.size() - 2; i >= 0; i--)
	{
		innerfunc.setFunc(innerfunc);
		innerfunc.setIntegralRegion(x[i].first, x[i].second);
	}
	DoubleArray xx(0);
	return innerfunc(xx);
}

/*!
    @brief  method for integration

    @param[in] f target function
	@param[in] xl integral lower bound
	@param[in] xu integral upper bound
	@param[in] grids boundary of partitions  
	@return integral result

	@note each partition integrate separately, and sum results
	@note grids must be sorted in advance
*/
double
LA1DIntegral::integrate(const LAFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
{
	if (xl > xu) return integrate(f, xu, xl, grids);
	if (grids.size() == 0) return integrate(f, xl, xu);

//	unsigned int pos1, pos2;

/*	LAAlgorithm::locate<DoubleArray, double>(grids, xl, grids.size(), pos1);
	LAAlgorithm::locate<DoubleArray, double>(grids, xu, grids.size(), pos2);

	double sum = 0.0;

	if (pos1 == pos2)
	{
   		return integrate(f, xl, xu);
	}

	sum = 0.0;
	sum += integrate(f, xl, grids[pos1]);
	for (unsigned int i = pos1; i < pos2 - 1; i++)
	{
		sum += integrate(f, grids[i], grids[i+1]);
	}
	sum += integrate(f, grids[pos2 - 1], xu);
	return sum;*/

	double sum = 0.0;
	set<double>::const_iterator itl, itu;
	itl = lower_bound<set<double>::const_iterator, double>(grids.begin(), grids.end(), xl);
	itu = lower_bound<set<double>::const_iterator, double>(grids.begin(), grids.end(), xu);
	
	if (itl == itu)
	{
   		return integrate(f, xl, xu);
	}
	
	sum = 0.0;
	itu--;
	sum += integrate(f, xl, *itl);
	for (; itl != itu; )
	{
		double a = *itl++;
		sum += integrate(f, a, *itl);
	}
	sum += integrate(f, *itu, xu);
	return sum;

}

double LA1DIntegral::IntegrateBySampling(const LAFunctionBase& f, const std::vector<double>& samples) const
{
    int nIntervals = samples.size() - 1;
    if (nIntervals < 1)
        throw LACoreInvalidData("The number of integration intervals should be 1 or larger", __FILE__, __LINE__);

    double integral = 0.0;
    for (int i = 0; i < nIntervals; i++)
        integral += integrate(f, samples[i], samples[i + 1]);

    return integral;
}

double LA1DIntegral::IntegrateByEqualIntervals(const LAFunctionBase& f, double xl, double xu, int nSplits) const
{
    if (nSplits < 1)
        throw LACoreInvalidData("The number of splits should be 1 or larger", __FILE__, __LINE__);

    vector<double> samples;
    double delta = (xu - xl) / nSplits;
    samples.push_back(xl);
    for (int i = 0; i < nSplits; i++)
        samples.push_back(samples.back() + delta);

    return IntegrateBySampling(f, samples);
}

/*!
    @brief  constructor
	@param[in] pIntegral: pointer to integral method
*/
LA1DIntegral::MMInnerFunction::MMInnerFunction(const LA1DIntegral* pIntegral)
:LAFunctionBase(), mpFunc(NULL), mXl(0), mXu(0), mpIntegral(pIntegral)
{
}

/*!
    @brief  destructor
*/
LA1DIntegral::MMInnerFunction::~MMInnerFunction()
{
	delete mpFunc;
}

/*!
    @brief  copy constructor
*/
LA1DIntegral::MMInnerFunction::MMInnerFunction(const MMInnerFunction& v)
:LAFunctionBase(v)
{
	mpFunc = dynamic_cast<LAFunctionBase*>(v.mpFunc->clone());
	mpIntegral = v.mpIntegral;
	mXl = v.mXl;
	mXu = v.mXu;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LA1DIntegral::MMInnerFunction::clone()const
{
    try 
	{
		return new MMInnerFunction(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief  return inner function value

	@param[in] x: point

	@return inner function value
*/
double
LA1DIntegral::MMInnerFunction::operator()(const DoubleArray& x) const
{
	DoubleArray tmp = x;
	tmp.resize(x.size() + 1);
	mpFunc->operator()(tmp.size() - 1, tmp);
	return mpFunc->integral(mXl, mXu, mpIntegral);
}

/*!
    @brief  set inner function

    @param[in] f: pointer to function
*/
void
LA1DIntegral::MMInnerFunction::setFunc(const LAFunctionBase& method)
{
	mpFunc = dynamic_cast<LAFunctionBase*>(method.clone());
}

