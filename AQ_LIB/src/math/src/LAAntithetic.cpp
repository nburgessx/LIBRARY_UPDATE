/*! @file
    @brief Implementation of a class for a negative correlation method.
*/

//  2005, Fixed Income Group, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAAntithetic.cpp
//
//  SYNOPSIS    :       LAAntithetic
//  DESCRIPTION :       
//                      
//                     
//                      
//  SEE ALSO    :       LAAntithetic.h
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LAAntithetic.h"

using namespace std;

/*! 
	@brief default constructor
*/
LAAntithetic::LAAntithetic() : 
	mpInner(NULL), isOdd(true)
{
}

/*!
	@brief destructor
*/
LAAntithetic::~LAAntithetic()
{
	if (mpInner != NULL)
		delete mpInner;
}

/*!
	@brief copy constructor - invoke the clone method to create a deep copy
*/
LAAntithetic::LAAntithetic(const LAAntithetic& v):
LARandBase(v), mpInner(NULL)
{
	if (v.mpInner) mpInner = dynamic_cast<LARandBase*>( v.mpInner->clone() );
	isOdd = v.isOdd;
	nextVariates = v.nextVariates;
}

/*!
	@brief deep copy of this object
	@return pointer to LACoreFunctionBase
*/
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAAntithetic::clone() const
{
    try 
	{
		return new LAAntithetic(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
	@brief function to set the random number generator to apply a negative correlation method
*/
void
LAAntithetic::setInnerGenerator(const LARandBase& v)
{
	if (mpInner) delete mpInner;
	mpInner = NULL;
	
	mpInner = dynamic_cast<LARandBase*>( v.clone() );
	setDim(mpInner->getDim());
	nextVariates.resize(getDim()[0]);
}

/*!
	@brief check whether this class derives from base class with type id
	@param[in] id function type to be checked
	@return True or false
*/
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAAntithetic::isTypeOf(function_t id) const
{
	return (id == FN_RAND_ANTI ? true : LARandBase::isTypeOf(id));
}

/*!
	@brief get type of the function
	@return function type
*/
/*!
    @brief Return this function type
    @return function type
*/
function_t
LAAntithetic::getType() const
{
	return FN_RAND_ANTI;
}

/*!
	@brief function to generate uniform random numbers
	@param[in] variates variable to store the calculation result
*/
void
LAAntithetic::getUniforms(DoubleArray& variates)
{
	// if the following block affects performance, remove it.
	if (mpInner == NULL)
	{
		// exception
		throw LACoreSystemError("Inner generator is not set.", __FILE__, __LINE__);
	}

	if (isOdd)
	{
		mpInner->getUniforms(variates);
		for (unsigned int i=0; i<variates.size(); i++) 
		{ 
			nextVariates[i] = 1.0 - variates[i]; 
		}
		isOdd = false;
	}
	else
	{
		variates = nextVariates;
		isOdd = true;
	}
}


/*!
	@brief function to set dimension information(first element should be the number of dimension)
	@param [in] dimValue information necessary for dimension setting\n
		    dimValue[0]=total dimension,\n
            dimValue[1]=quasi-random number dimension
*/
void
LAAntithetic::setDim(const UintArray& dimValue)
{
	LARandBase::setDim(dimValue);
	mpInner->setDim(dimValue);
	nextVariates.resize(dimValue[0]);

	isOdd = true;
}


/*!
	@brief function to set seed
	@param[in] seed hold (multi) seed information
*/
void
LAAntithetic::setSeed(const UlongArray& seed)
{
	LARandBase::setSeed(seed);
	mpInner->setSeed(seed);
	isOdd = true;
}




