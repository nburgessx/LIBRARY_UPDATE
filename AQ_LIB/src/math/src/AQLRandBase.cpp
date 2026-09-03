/*! @file
    @brief Implementation to generate random numbers.
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRandBase.h"


//================ AQLRandBase ===================================
/*!
    @brief default constructor
*/
AQLRandBase::AQLRandBase(bool isHalleyMod) 
: AQLCoreFunctionBase(), mIsHalleyMod(isHalleyMod)
{

}

// copy constructor
//AQLRandBase::AQLRandBase(const AQLRandBase& v) 
//: AQLCoreFunctionBase(v)
//{
//}

/*!
    @brief destructor
*/
AQLRandBase::~AQLRandBase() 
{
}


/*!
    @brief check whether this class derives from base class with type id
    @param[in] id function type to be checked 
    @return True or false
*/
bool
AQLRandBase::isTypeOf(function_t id) const
{
    return (id == FN_RANDBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief get type of the function
    @return function type
*/
function_t
AQLRandBase::getType() const
{
    return FN_RANDBASE;
}

/*!
    @brief function to generate normal random numbers
    @param[in] variates variable to store the calculation result
*/
void
AQLRandBase::getGaussians(DoubleArray& variates)
{
    getUniforms(variates);

	// Halley modification
	if (!mIsHalleyMod)
	{
		for (unsigned int i=0;i<mDim[0];i++)
		{
			variates[i] = AQLDist::invNormdist(variates[i]);
		}
	}
	else
	{
		for (unsigned int i=0;i<mDim[0];i++)
		{
			variates[i] = AQLDist::invNormdistHalleyMod(variates[i]);
		}
	}
}

/*!
    @brief function to set dimension information(first element should be the number of dimension)
    @param[in] dimValue dimension information

*/
void
AQLRandBase::setDim(const UintArray& dimValue)
{
	// there is no guarantee of results (This is of valarray specification of C + + standard)
	// if you do not have the same size of both valarray assignment
	//    mDim=dimValue;
	// if the size of the dimValue and mDim  are different, change the size mDim into the size of the dimValue
	// make an assignment to valarray
    if (mDim.size() != dimValue.size()){
        mDim.resize( dimValue.size() );
    }
    mDim = dimValue;

    return;
}

/*!
    @brief function to set seed
    @param[in] seedValue seed information

*/
void
AQLRandBase::setSeed(const UlongArray& seedValue)
{
	// there is no guarantee of results (This is of valarray specification of C + + standard)
	// if you do not have the same size of both valarray assignment
	//    mSeed=seedValue;
	// if the size of the mSeed and seedValue  are different, change the size mSeed into the size of the seedValue
    // make an assignment to valarray
    if (mSeed.size() != seedValue.size()){
        mSeed.resize( seedValue.size() );
    }
    mSeed = seedValue;

    return;
}

/*!
    @brief function to get dimension information to be set
    @return dimension information (unsigned int Valarray)
*/
const UintArray&
AQLRandBase::getDim(void)const
{
    return mDim;
}

/*!
    @brief function to get seed 
    @return seed information (unsigned long Valarray)
*/
const UlongArray&
AQLRandBase::getSeed(void)const
{
    return mSeed;
}
/*!
    @brief function to set parameters
    @param[in] param parameters information

*/								
void
AQLRandBase::setParam(const DoubleMatrix& param)
{
	mParam = param;
}
