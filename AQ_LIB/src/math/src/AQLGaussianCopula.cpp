/*! @file
    @brief Class declaration to generate a set of random numbers according to Gaussian Copula

	This class derives from AQLRandoBase.
*/


#include "AQLGaussianCopula.h"
#include "AQLCholeskyDecompSC.h"

using namespace std;

/*!
    @brief Source code of class to generate a set of random numbers according to Gaussian Copula
	
	This class derives from AQLRandoBase.
*/

/*!
	@brief constructor
*/
AQLGaussianCopula::AQLGaussianCopula() 
: AQLRandBase(),mpRand(NULL)
{

}

/*!
	@brief destructor
*/
AQLGaussianCopula::~AQLGaussianCopula()
{
	if (mpRand != NULL)
		delete mpRand;
}

/*!
	@brief copy constructor - invoke the clone method to create a deep copy
*/
AQLGaussianCopula::AQLGaussianCopula(const AQLGaussianCopula& v):
AQLRandBase(v),mpRand(NULL)
{
	if (v.mpRand) mpRand = dynamic_cast<AQLRandBase*>( v.mpRand->clone() );
	mMat = v.mMat;
	mVar = v.mVar;
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLGaussianCopula::clone() const
{
    try 
    {
        return new AQLGaussianCopula(*this);
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
AQLGaussianCopula::isTypeOf(function_t id) const
{
    return (id == FN_RAND_GAUSSIANCOPULA ? true : AQLRandBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLGaussianCopula::getType() const
{
    return FN_RAND_GAUSSIANCOPULA;
}

/*!
    @brief Generate uniform random numbers
    @param[out] variates correlated cumulative normal numbers
*/
void
AQLGaussianCopula::getUniforms(DoubleArray& variates)
{
    unsigned int i;
    getGaussians(variates);
    for (i = 0; i < getDim()[0]; i++)
    {
        variates[i] = AQLDist::normsdist(variates[i]);
    }	
}

/*!
    @brief Generate random normal numbers
    @param[out] variates correlated random normal numbers
*/
void
AQLGaussianCopula::getGaussians(DoubleArray& variates)
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
AQLGaussianCopula::setSeed(const UlongArray& seed)
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
AQLGaussianCopula::setDim(const UintArray& dimValue)
{
 	AQLRandBase::setDim(dimValue);
	mpRand->setDim(dimValue);
}

/*!
	@brief Set random number generator for negative correlation method
	@param[in] v pointer to random number generator
*/
void
AQLGaussianCopula::setRandGenerator(const AQLRandBase& v)
{
	if (mpRand) delete mpRand;
	mpRand = NULL;
	mpRand = dynamic_cast<AQLRandBase*>( v.clone() );
	if(mpRand->getDim().size() != 0) setDim(mpRand->getDim());
	if(mpRand->getSeed().size() != 0) setSeed(mpRand->getSeed());
}

/*!
	@brief Set paramter
	@param[in] param parameter matrix to set 
*/
void
AQLGaussianCopula::setParam(const DoubleMatrix& param)
{
	AQLNumericMatrix mat(param.size(), param.size());
	for (unsigned int i = 0; i < param.size(); i++)
	{
		for (unsigned int j = 0; j < param.size(); j++)
		{
			mat.setValue(i, j, param[i][j]);
		}
	}
	mMat = AQLCholeskyDecompSC::choleskyDecompositionSC(mat);
	mVar.resize(mMat.row());
	AQLRandBase::setParam(param);
}
