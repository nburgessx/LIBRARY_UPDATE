/*! @file
    @brief Source code of drift function of LMM sde (spot measure)(log type)



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftLogLMMSpot.cpp
//
//  SYNOPSIS    :       LAPriceDriftLogLMMSpot
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


#include "LAPriceDriftLogLMMSpot.h"



using namespace std;


//================ LAPriceDriftLogLMMSpot ===================================
/*!
	@brief default constructor
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLogLMMSpot::LAPriceDriftLogLMMSpot(double Q)
: LAPriceDriftLMMSpot(Q)
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
LAPriceDriftLogLMMSpot::LAPriceDriftLogLMMSpot(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q)
: LAPriceDriftLMMSpot(sdeAttrName, i, tenor, delta_tenor, Q)
{

}


/*!
	@brief destructor
*/
LAPriceDriftLogLMMSpot::~LAPriceDriftLogLMMSpot() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftLogLMMSpot::clone() const	
{
    try 
	{
		return new LAPriceDriftLogLMMSpot(*this);
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
LAPriceDriftLogLMMSpot::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLOGLMMSPOT ? true : LAPriceDriftLMMBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftLogLMMSpot::getType() const
{
	return FN_DRIFTLOGLMMSPOT;
}


