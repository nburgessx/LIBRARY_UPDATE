/*! @file
    @brief Sorce code for class to integrate with Monte Carlo method
	
	This class derives from LAIntegralBase

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAIntegralMC.cpp
//
//  SYNOPSIS    :       LAIntegralMC
//  DESCRIPTION :       Source code for class to integrate with Monte Carlo method
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


#include "LAIntegralMC.h"
#include "LAFunctionBase.h"
#include "LARandBase.h"

using namespace std;
//================ LAIntegralMC ===================================
/*!
	@brief Constructor
	@param[in] num Number of sampling point 
	@param[in] v Rand generator class
*/
LAIntegralMC::LAIntegralMC(unsigned long num,const LARandBase& v)
: LAIntegralBase(),mpRand(NULL),mMCNum(num)
{
	mpRand = dynamic_cast<LARandBase*>(v.clone());
}

/*!
	@brief Destructor
*/
LAIntegralMC::~LAIntegralMC() 
{
	delete mpRand;
}

/*!
	@brief copy constructor
*/
LAIntegralMC::LAIntegralMC(const LAIntegralMC& v) 
:mpRand(NULL),mMCNum(v.mMCNum)
{
	mpRand = dynamic_cast<LARandBase*>(v.mpRand->clone());
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAIntegralMC::clone() const
{
    try 
	{
        return new LAIntegralMC(*this);
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
LAIntegralMC::isTypeOf(function_t id) const
{
    return (id == FN_INTEGRALMC ? true : LAIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t
LAIntegralMC::getType() const
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
LAIntegralMC::integrate(const LAFunctionBase& f,
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
LAIntegralMC::setRandGenerator(const LARandBase& v)
{
	if (mpRand)delete mpRand;
	mpRand = NULL;
	mpRand = dynamic_cast<LARandBase*>(v.clone());
}

/*!
    @brief Set seed
    @param[in] seedValue information necessary for seed setting

*/
void
LAIntegralMC::setSeed(const UlongArray& seedValue)
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
LAIntegralMC::setDim(const UintArray& dimValue)
{
	mpRand->setDim(dimValue);
}
