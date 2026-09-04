/*! @file
    @brief Souce code of class to generate a set of random numbers according to factor model

	This class derives from AQLRandoBase.
*/


#include "AQLFactorModel.h"

using namespace std;

/*!
    @brief Source code of class to generate a set of random numbers according to factor model
	
	This class derives from AQLRandoBase.
*/

/*!
	@brief constructor
*/
AQLFactorModel::AQLFactorModel() 
: AQLRandBase(),mpRand(NULL)
{

}

/*!
	@brief destructor
*/
AQLFactorModel::~AQLFactorModel()
{
	if (mpRand != NULL)
		delete mpRand;
}

/*!
	@brief copy constructor - invoke the clone method to create a deep copy
*/
AQLFactorModel::AQLFactorModel(const AQLFactorModel& v):
AQLRandBase(v),mOmega(v.mOmega),mpRand(NULL)
{
	mpRand = dynamic_cast<AQLRandBase*>( v.mpRand->clone() );
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLFactorModel::clone() const
{
    try 
    {
        return new AQLFactorModel(*this);
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
AQLFactorModel::isTypeOf(function_t id) const
{
    return (id == FN_RAND_FACTORMODEL ? true : AQLRandBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLFactorModel::getType() const
{
    return FN_RAND_FACTORMODEL;
}

/*!
    @brief Generate uniform random numbers
    @param[out] variates correlated cumulative normal numbers
*/
void
AQLFactorModel::getUniforms(DoubleArray& variates)
{
    unsigned int i;
    unsigned int size = getDim()[0];
	getGaussians(variates);
    for (i = 0; i < size; i++)
    {
        variates[i] = AQLDist::normsdist(variates[i]);
    }	
}
/*!
    @brief Generate random normal numbers
    @param[out] variates correlated random normal numbers
*/
void
AQLFactorModel::getGaussians(DoubleArray& variates)
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
AQLFactorModel::setSeed(const UlongArray& seed)
{
	AQLRandBase::setSeed(seed);
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
AQLFactorModel::setDim(const UintArray& dimValue)
{
 	AQLRandBase::setDim(dimValue);
	mpRand->setDim(dimValue);
}


/*!
	@brief Set random number generator for negative correlation method
	@param[in] v random number generator
*/
void
AQLFactorModel::setRandGenerator(const AQLRandBase& v)
{
	if (mpRand) delete mpRand;
	mpRand = NULL;
	mpRand = dynamic_cast<AQLRandBase*>( v.clone() );
	if(mpRand->getDim().size() != 0) setDim(mpRand->getDim());
	if(mpRand->getSeed().size() != 0) setSeed(mpRand->getSeed());
}

/*!
	@brief Set paramter
	@param[in] param parameter matrix to set: 
*/
void
AQLFactorModel::setParam(const DoubleMatrix& param)
{
	if (param.size() == 0)
	{
		throw AQLCoreInvalidData("Parameter Matrix row size is zero", __FILE__, __LINE__);
	}

	mOmega.resize(param.size());
	mVar.resize(param.size() + param[0].size());
	for (unsigned int i = 0; i < param.size(); i++)
	{
		double omega = 0.0;
		for (unsigned int j = 0; j < param[i].size(); j++)
		{
			omega += AQLMath::sqr(param[i][j]);
		}
		if (omega > 1.0)
		{
			throw AQLCoreInvalidData("Parameter Matrix is something wrong", __FILE__, __LINE__);
		}
		mOmega[i] = AQLMath::sqrt(1-omega);
	}
	AQLRandBase::setParam(param);
}
