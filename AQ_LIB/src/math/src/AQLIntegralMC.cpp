/*! @file
    @brief Sorce code for class to integrate with Monte Carlo method
	
	This class derives from AQLIntegralBase

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLIntegralMC.cpp
//
//  SYNOPSIS    :       AQLIntegralMC
//  DESCRIPTION :       Source code for class to integrate with Monte Carlo method
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


#include "AQLIntegralMC.h"
#include "AQLFunctionBase.h"
#include "AQLRandBase.h"

using namespace std;
//================ AQLIntegralMC ===================================
/*!
	@brief Constructor
	@param[in] num Number of sampling point 
	@param[in] v Rand generator class
*/
AQLIntegralMC::AQLIntegralMC(unsigned long num,const AQLRandBase& v)
: AQLIntegralBase(),mpRand(NULL),mMCNum(num)
{
	mpRand = dynamic_cast<AQLRandBase*>(v.clone());
}

/*!
	@brief Destructor
*/
AQLIntegralMC::~AQLIntegralMC() 
{
	delete mpRand;
}

/*!
	@brief copy constructor
*/
AQLIntegralMC::AQLIntegralMC(const AQLIntegralMC& v) 
:mpRand(NULL),mMCNum(v.mMCNum)
{
	mpRand = dynamic_cast<AQLRandBase*>(v.mpRand->clone());
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLIntegralMC::clone() const
{
    try 
	{
        return new AQLIntegralMC(*this);
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
AQLIntegralMC::isTypeOf(function_t id) const
{
    return (id == FN_INTEGRALMC ? true : AQLIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t
AQLIntegralMC::getType() const
{
    return FN_INTEGRALMC;  
}

/*!
    @brief  method for integration, which is inherited from base class

    @param[in] f target function
	@param[in] x integral region

	@return integral result
*/
double
AQLIntegralMC::integrate(const AQLFunctionBase& f,
								const std::vector<std::pair<double,double> >& x)const
{
	DoubleArray randarray,scale;
	randarray.resize(x.size());
	scale.resize(x.size());
	int sign=1;
	unsigned int i;
	for (i=0;i<x.size();i++)
	{
		scale[i]=x[i].second-x[i].first;
		if (scale[i]<0)
		{
			sign=-sign;
			scale[i]=-scale[i];
		}
	}

	double ret = 0.0;
	for (unsigned long l=0; l<mMCNum; l++)
	{
		mpRand->getUniforms(randarray);
		for (i = 0; i < randarray.size(); i++) randarray[i] *= scale[i];
		for (i=0; i<randarray.size(); i++) randarray[i] += x[i].first;
		ret += f(randarray);
	}
	ret /= mMCNum;
	ret *= sign;

	for (i=0; i<x.size(); i++)
	{
		ret *= scale[i];
	}
	return ret;
}

/*!
	@brief set random number generation
	@param[in] v random number generation
*/
void
AQLIntegralMC::setRandGenerator(const AQLRandBase& v)
{
	if (mpRand)delete mpRand;
	mpRand = NULL;
	mpRand = dynamic_cast<AQLRandBase*>(v.clone());
}

/*!
    @brief Set seed
    @param[in] seedValue information necessary for seed setting

*/
void
AQLIntegralMC::setSeed(const UlongArray& seedValue)
{
	mpRand->setSeed(seedValue);
}

/*!
    @brief  Set dimension

    @param[in] dimValue information necessary for dimension setting\n
		    dimValue[0]=total dimension,\n
            dimValue[1]=quasi-random number dimension
*/
void
AQLIntegralMC::setDim(const UintArray& dimValue)
{
	mpRand->setDim(dimValue);
}
