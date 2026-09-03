/*! @file
    @brief Souce code of class to generate a set of random numbers according to factor model

	This class derives from MMRandoBase.
*/

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAFactorModel.cpp
//
//  SYNOPSIS    :       LAFactorModel
//  DESCRIPTION :		Source code of class to generate a set of random numbers
//						according to factor model.This class derives from MMRandoBase.
//
//
//
//  SEE ALSO    :       
//  TYPE        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "LAFactorModel.h"

using namespace std;

/*!
    @brief Source code of class to generate a set of random numbers according to factor model
	
	This class derives from MMRandoBase.
*/

/*!
	@brief constructor
*/
LAFactorModel::LAFactorModel() 
: LARandBase(),mpRand(NULL)
{

}

/*!
	@brief destructor
*/
LAFactorModel::~LAFactorModel()
{
	if (mpRand != NULL)
		delete mpRand;
}

/*!
	@brief copy constructor - invoke the clone method to create a deep copy
*/
LAFactorModel::LAFactorModel(const LAFactorModel& v):
LARandBase(v),mOmega(v.mOmega),mpRand(NULL)
{
	mpRand = dynamic_cast<LARandBase*>( v.mpRand->clone() );
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAFactorModel::clone() const
{
    try 
    {
        return new LAFactorModel(*this);
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
LAFactorModel::isTypeOf(function_t id) const
{
    return (id == FN_RAND_FACTORMODEL ? true : LARandBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAFactorModel::getType() const
{
    return FN_RAND_FACTORMODEL;
}

/*!
    @brief Generate uniform random numbers
    @param[out] variates correlated cumulative normal numbers
*/
void
LAFactorModel::getUniforms(DoubleArray& variates)
{
    unsigned int i;
    unsigned int size = getDim()[0];
	getGaussians(variates);
    for (i = 0; i < size; i++)
    {
        variates[i] = LADist::normsdist(variates[i]);
    }	
}
/*!
    @brief Generate random normal numbers
    @param[out] variates correlated random normal numbers
*/
void
LAFactorModel::getGaussians(DoubleArray& variates)
{
    mpRand->getGaussians(mVar);
	for (unsigned int i = 0; i < variates.size(); i++)
	{
		variates[i] = 0;
		for(unsigned int j = 0; j < mParam[i].size(); j++)
			variates[i] += mParam[i][j] * mVar[j];
		variates[i] += mOmega[i] * mVar[mVar.size()-variates.size()+i];
	}
	return;
}

/*!
    @brief Set seed
    @param[in] seed information necessary for seed setting \n

*/
void
LAFactorModel::setSeed(const UlongArray& seed)
{
	LARandBase::setSeed(seed);
	mpRand->setSeed(seed);
}

/*!
    @brief  Set dimension\n
            (overriding)

    @param[in] dimValue information necessary for dimension setting\n
		    dimValue[0]=total dimension,\n
            dimValue[1]=quasi-random number dimension
*/
void
LAFactorModel::setDim(const UintArray& dimValue)
{
 	LARandBase::setDim(dimValue);
	mpRand->setDim(dimValue);
}


/*!
	@brief Set random number generator for negative correlation method
	@param[in] v random number generator
*/
void
LAFactorModel::setRandGenerator(const LARandBase& v)
{
	if (mpRand) delete mpRand;
	mpRand = NULL;
	mpRand = dynamic_cast<LARandBase*>( v.clone() );
	if(mpRand->getDim().size() != 0) setDim(mpRand->getDim());
	if(mpRand->getSeed().size() != 0) setSeed(mpRand->getSeed());
}

/*!
	@brief Set paramter
	@param[in] param parameter matrix to set: 
*/
void
LAFactorModel::setParam(const DoubleMatrix& param)
{
	if (param.size() == 0)
	{
		throw LACoreInvalidData("Parameter Matrix row size is zero", __FILE__, __LINE__);
	}

	mOmega.resize(param.size());
	mVar.resize(param.size() + param[0].size());
	for (unsigned int i = 0; i < param.size(); i++)
	{
		double omega = 0.0;
		for (unsigned int j = 0; j < param[i].size(); j++)
		{
			omega += LAMath::sqr(param[i][j]);
		}
		if (omega > 1.0)
		{
			throw LACoreInvalidData("Parameter Matrix is something wrong", __FILE__, __LINE__);
		}
		mOmega[i] = LAMath::sqrt(1-omega);
	}
	LARandBase::setParam(param);
}
