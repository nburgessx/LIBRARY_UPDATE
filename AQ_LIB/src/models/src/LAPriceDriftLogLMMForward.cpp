/*! @file
    @brief Source code of drift function of LMM sde (forward measure)(log type)



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftLogLMMForward.cpp
//
//  SYNOPSIS    :       LAPriceDriftLogLMMForward
//  DESCRIPTION :       Source code of drift function of LMM sde (forward measure)(log type)
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


#include "LAPriceDriftLogLMMForward.h"



using namespace std;


//================ LAPriceDriftLogLMMForward ===================================
/*!
	@brief default constructor
	@param[in] point forward measure point(DiscountBond who's maturity is  mPoint-th forward libor cf time(mTenor[mPoint + 1]) is numerarire)  
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLogLMMForward::LAPriceDriftLogLMMForward(unsigned int point, double Q)
: LAPriceDriftLMMForward(point, Q)
{

}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] delta_tenor delta of tenor
	@param[in] point forward measure point(DiscountBond who's maturity is  mPoint-th forward libor cf time(mTenor[mPoint + 1]) is numerarire)  
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLogLMMForward::LAPriceDriftLogLMMForward(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, unsigned int point, double Q)
: LAPriceDriftLMMForward(sdeAttrName, i, tenor, delta_tenor, point, Q)
{

}


/*!
	@brief destructor
*/
LAPriceDriftLogLMMForward::~LAPriceDriftLogLMMForward() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftLogLMMForward::clone() const	
{
    try 
	{
		return new LAPriceDriftLogLMMForward(*this);
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
LAPriceDriftLogLMMForward::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLOGLMMFORWARD ? true : LAPriceDriftLMMBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftLogLMMForward::getType() const
{
	return FN_DRIFTLOGLMMFORWARD;
}


