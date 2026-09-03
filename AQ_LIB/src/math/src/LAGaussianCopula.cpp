/*! @file
    @brief Class declaration to generate a set of random numbers according to Gaussian Copula

	This class derives from MMRandoBase.
*/

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAGaussianCopula.cpp
//
//  SYNOPSIS    :       LAGaussianCopula
//  DESCRIPTION :		Class declaration to generate a set of random numbers 
//						according to Gaussian Copula. This class derives from MMRandoBase.
//
//  SEE ALSO    :       
//  TYPE        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "LAGaussianCopula.h"
#include "LACholeskyDecompSC.h"

using namespace std;

/*!
    @brief Source code of class to generate a set of random numbers according to Gaussian Copula
	
	This class derives from MMRandoBase.
*/

/*!
	@brief constructor
*/
LAGaussianCopula::LAGaussianCopula() 
: LARandBase(),mpRand(NULL)
{

}

/*!
	@brief destructor
*/
LAGaussianCopula::~LAGaussianCopula()
{
	if (mpRand != NULL)
		delete mpRand;
}

/*!
	@brief copy constructor - invoke the clone method to create a deep copy
*/
LAGaussianCopula::LAGaussianCopula(const LAGaussianCopula& v):
LARandBase(v),mpRand(NULL)
{
	if (v.mpRand) mpRand = dynamic_cast<LARandBase*>( v.mpRand->clone() );
	mMat = v.mMat;
	mVar = v.mVar;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAGaussianCopula::clone() const
{
    try 
    {
        return new LAGaussianCopula(*this);
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
LAGaussianCopula::isTypeOf(function_t id) const
{
    return (id == FN_RAND_GAUSSIANCOPULA ? true : LARandBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAGaussianCopula::getType() const
{
    return FN_RAND_GAUSSIANCOPULA;
}

/*!
    @brief Generate uniform random numbers
    @param[out] variates correlated cumulative normal numbers
*/
void
LAGaussianCopula::getUniforms(DoubleArray& variates)
{
    unsigned int i;
    getGaussians(variates);
    for (i = 0; i < getDim()[0]; i++)
    {
        variates[i] = LADist::normsdist(variates[i]);
    }	
}

/*!
    @brief Generate random normal numbers
    @param[out] variates correlated random normal numbers
*/
void
LAGaussianCopula::getGaussians(DoubleArray& variates)
{
    mpRand->getGaussians(mVar);//get independent gauss rands;
	double val;
	for (unsigned int i = 0; i < variates.size(); i++)
	{
		val = 0.0;
		for (unsigned int j = 0; j <= i; j++)
			val += mMat[i][j] * mVar[j];
		variates[i] = val;
	}
	return;	
}

/*!
    @brief Set seed
    @param[in] seed information necessary for seed setting \n
*/
void
LAGaussianCopula::setSeed(const UlongArray& seed)
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
LAGaussianCopula::setDim(const UintArray& dimValue)
{
 	LARandBase::setDim(dimValue);
	mpRand->setDim(dimValue);
}

/*!
	@brief Set random number generator for negative correlation method
	@param[in] v pointer to random number generator
*/
void
LAGaussianCopula::setRandGenerator(const LARandBase& v)
{
	if (mpRand) delete mpRand;
	mpRand = NULL;
	mpRand = dynamic_cast<LARandBase*>( v.clone() );
	if(mpRand->getDim().size() != 0) setDim(mpRand->getDim());
	if(mpRand->getSeed().size() != 0) setSeed(mpRand->getSeed());
}

/*!
	@brief Set paramter
	@param[in] param parameter matrix to set 
*/
void
LAGaussianCopula::setParam(const DoubleMatrix& param)
{
	LAMatrix mat(param.size(), param.size());
	for (unsigned int i = 0; i < param.size(); i++)
	{
		for (unsigned int j = 0; j < param.size(); j++)
		{
			mat.setValue(i, j, param[i][j]);
		}
	}
	mMat = LACholeskyDecompSC::choleskyDecompositionSC(mat);
	mVar.resize(mMat.row());
	LARandBase::setParam(param);
}
