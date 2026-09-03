/*! @file
    @brief Source code of drift function of LMM sde (spot measure)(log type)



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLPriceDriftLogLMMSpot.cpp
//
//  SYNOPSIS    :       AQLPriceDriftLogLMMSpot
//  DESCRIPTION :       Source code of drift function of LMM sde (spot measure)(log type)
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


#include "AQLPriceDriftLogLMMSpot.h"



using namespace std;


//================ AQLPriceDriftLogLMMSpot ===================================
/*!
	@brief default constructor
	@param[in] Q val for displaced diffusion case
*/
AQLPriceDriftLogLMMSpot::AQLPriceDriftLogLMMSpot(double Q)
: AQLPriceDriftLMMSpot(Q)
{

}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] delta_tenor delta of tenor
	@param[in] Q val for displaced diffusion case
*/
AQLPriceDriftLogLMMSpot::AQLPriceDriftLogLMMSpot(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q)
: AQLPriceDriftLMMSpot(sdeAttrName, i, tenor, delta_tenor, Q)
{

}


/*!
	@brief destructor
*/
AQLPriceDriftLogLMMSpot::~AQLPriceDriftLogLMMSpot() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftLogLMMSpot::clone() const	
{
    try 
	{
		return new AQLPriceDriftLogLMMSpot(*this);
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
AQLPriceDriftLogLMMSpot::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLOGLMMSPOT ? true : AQLPriceDriftLMMBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftLogLMMSpot::getType() const
{
	return FN_DRIFTLOGLMMSPOT;
}


