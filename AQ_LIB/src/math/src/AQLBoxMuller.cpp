/*! @file
    @brief Implementation of the Box-Muller method.
*/

//  2006, Fixed Income Group, AlgoQuantHub.


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLBoxMuller.cpp
//
//  SYNOPSIS    :       AQLBoxMuller
//  DESCRIPTION :       
//                      
//                     
//                      
//  SEE ALSO    :       AQLBoxMuller.h
//  TYPE        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "AQLBoxMuller.h"

using namespace std;

/*!
	@brief constructor
*/
AQLBoxMuller::AQLBoxMuller() 
: AQLRandBase(),mpRand(NULL),mIsOdd(true)
{

}

/*!
	@brief destructor
*/
AQLBoxMuller::~AQLBoxMuller()
{
	if (mpRand) delete mpRand;
}

/*!
	@brief copy constructor - invoke the clone method to create a deep copy
*/
AQLBoxMuller::AQLBoxMuller(const AQLBoxMuller& v):
AQLRandBase(v),mpRand(NULL)
{
	if (v.mpRand) mpRand = dynamic_cast<AQLRandBase*>( v.mpRand->clone() );
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLBoxMuller::clone() const
{
    try 
    {
        return new AQLBoxMuller(*this);
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
AQLBoxMuller::isTypeOf(function_t id) const
{
    return (id == FN_RAND_BOXMULLER ? true : AQLRandBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLBoxMuller::getType() const
{
    return FN_RAND_BOXMULLER;
}

/*!
    @brief Generate uniform random numbers
    @param[out] variates correlated cumulative normal numbers
*/
void
AQLBoxMuller::getUniforms(DoubleArray& variates)
{
	mpRand->getUniforms(variates);
}

/*!
    @brief Generate random normal numbers
    @param[out] variates correlated random normal numbers
*/
void
AQLBoxMuller::getGaussians(DoubleArray& variates)
{
	DoubleArray  u1(2);

    if ( variates.size() != mNextVar.size() )
    {
        mNextVar.resize( variates.size() );
        mIsOdd = true;
    }

    if ( mIsOdd )
    {
	    for ( unsigned int i=0; i<variates.size(); i++ )
	    {
    		double x = 0, y = 0, s = 0;
		    do {
			    getUniforms(u1);
			    x = 2 * u1[0] - 1;
			    y = 2 * u1[1] - 1;
			    s = x*x + y*y;
		    } while ( s == 0.0 || s >= 1.0  );

            double multiple = AQLMath::sqrt( -2.0 * AQLMath::log( s ) / s );
            variates[i] = x * multiple;
            mNextVar[i] = y * multiple;
	    }
        mIsOdd = false;
    }
    else
    {
        for ( unsigned int i=0; i<variates.size(); i++ )
        {
            variates[i] = mNextVar[i];
        }
        mIsOdd = true;
    }
}

/*!
    @brief Set seed
    @param[in] seed information necessary for seed setting \n
*/
void
AQLBoxMuller::setSeed(const UlongArray& seed)
{
	AQLRandBase::setSeed(seed);
	mpRand->setSeed(seed);

    mIsOdd = true;
}

/*!
    @brief  Set dimension\n
            (overriding)

    @param[in] dimValue information necessary for dimension setting\n
		    dimValue[0]=total dimension,\n
            dimValue[1]=quasi-random number dimension
*/
void
AQLBoxMuller::setDim(const UintArray& dimValue)
{
 	AQLRandBase::setDim( dimValue );
    mNextVar.resize( dimValue[0] );
    mIsOdd = true;

	// set inner generator's dimension 1
	UintArray  mDim(1);
	mDim[0] = 2;
	mpRand->setDim( mDim );
}

/*!
	@brief Set random number generator for negative correlation method
	@param[in] v pointer to random number generator
*/
void
AQLBoxMuller::setRandGenerator(const AQLRandBase& v)
{
	if (mpRand) delete mpRand;
	mpRand = NULL;
	mpRand = dynamic_cast<AQLRandBase*>( v.clone() );

	// set inner generator's dimension 2
	UintArray  mDim(1);
	mDim[0] = 2;
	if ( mpRand )  mpRand->setDim( mDim );

	if ( mpRand->getSeed().size() != 0 )  setSeed( mpRand->getSeed() );
}

