/*! @file
    @brief Source code of drift function of LMM sde (forward measure)(log type)



*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftLogLMMForward.h"



using namespace std;


//================ AQLPriceDriftLogLMMForward ===================================
/*!
	@brief default constructor
	@param[in] point forward measure point(DiscountBond who's maturity is  mPoint-th forward libor cf time(mTenor[mPoint + 1]) is numerarire)  
	@param[in] Q val for displaced diffusion case
*/
AQLPriceDriftLogLMMForward::AQLPriceDriftLogLMMForward(unsigned int point, double Q)
: AQLPriceDriftLMMForward(point, Q)
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
AQLPriceDriftLogLMMForward::AQLPriceDriftLogLMMForward(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, unsigned int point, double Q)
: AQLPriceDriftLMMForward(sdeAttrName, i, tenor, delta_tenor, point, Q)
{

}


/*!
	@brief destructor
*/
AQLPriceDriftLogLMMForward::~AQLPriceDriftLogLMMForward() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftLogLMMForward::clone() const	
{
    try 
	{
		return new AQLPriceDriftLogLMMForward(*this);
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
AQLPriceDriftLogLMMForward::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLOGLMMFORWARD ? true : AQLPriceDriftLMMBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftLogLMMForward::getType() const
{
	return FN_DRIFTLOGLMMFORWARD;
}


