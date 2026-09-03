/*! @file
    @brief Source code of abstract base class for one dimensional integration

	This class derives from AQLIntegralBase.
*/



////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQL1DIntegral.cpp
//
//  SYNOPSIS    :       AQL1DIntegral
//  DESCRIPTION :       Source code of abstract base class for one dimensional integration
//						This class derives from AQLIntegralBase
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


#include "AQL1DIntegral.h"
//#include "AQLAlgorithm.h"
#include <algorithm>

using namespace std;
//================ AQL1DIntegral ===================================
/*!
	@brief Constructor
*/
AQL1DIntegral::AQL1DIntegral()
: AQLIntegralBase()
{
}

/*!
	@brief Destructor
*/
AQL1DIntegral::~AQL1DIntegral() 
{
}

/*!
    @brief Check function for this class type
    @param[in] id class type to check
    @return true or false
*/
bool
AQL1DIntegral::isTypeOf(function_t id) const
{
    return (id == FN_1DINTEGRAL ? true : AQLIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
AQL1DIntegral::getType() const
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
AQL1DIntegral::integrate(const AQLFunctionBase& f,
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
AQL1DIntegral::integrate(const AQLFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
{
	if (xl > xu) return integrate(f, xu, xl, grids);
	if (grids.size() == 0) return integrate(f, xl, xu);

//	unsigned int pos1, pos2;

/*	AQLAlgorithm::locate<DoubleArray, double>(grids, xl, grids.size(), pos1);
	AQLAlgorithm::locate<DoubleArray, double>(grids, xu, grids.size(), pos2);

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

double AQL1DIntegral::IntegrateBySampling(const AQLFunctionBase& f, const std::vector<double>& samples) const
{
    int nIntervals = samples.size() - 1;
    if (nIntervals < 1)
        throw AQLCoreInvalidData("The number of integration intervals should be 1 or larger", __FILE__, __LINE__);

    double integral = 0.0;
    for (int i = 0; i < nIntervals; i++)
        integral += integrate(f, samples[i], samples[i + 1]);

    return integral;
}

double AQL1DIntegral::IntegrateByEqualIntervals(const AQLFunctionBase& f, double xl, double xu, int nSplits) const
{
    if (nSplits < 1)
        throw AQLCoreInvalidData("The number of splits should be 1 or larger", __FILE__, __LINE__);

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
AQL1DIntegral::MMInnerFunction::MMInnerFunction(const AQL1DIntegral* pIntegral)
:AQLFunctionBase(), mpFunc(NULL), mXl(0), mXu(0), mpIntegral(pIntegral)
{
}

/*!
    @brief  destructor
*/
AQL1DIntegral::MMInnerFunction::~MMInnerFunction()
{
	delete mpFunc;
}

/*!
    @brief  copy constructor
*/
AQL1DIntegral::MMInnerFunction::MMInnerFunction(const MMInnerFunction& v)
:AQLFunctionBase(v)
{
	mpFunc = dynamic_cast<AQLFunctionBase*>(v.mpFunc->clone());
	mpIntegral = v.mpIntegral;
	mXl = v.mXl;
	mXu = v.mXu;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQL1DIntegral::MMInnerFunction::clone()const
{
    try 
	{
		return new MMInnerFunction(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief  return inner function value

	@param[in] x: point

	@return inner function value
*/
double
AQL1DIntegral::MMInnerFunction::operator()(const DoubleArray& x) const
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
AQL1DIntegral::MMInnerFunction::setFunc(const AQLFunctionBase& method)
{
	mpFunc = dynamic_cast<AQLFunctionBase*>(method.clone());
}

