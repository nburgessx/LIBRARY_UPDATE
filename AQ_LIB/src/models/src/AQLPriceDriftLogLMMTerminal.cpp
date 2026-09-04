/*! @file
    @brief Source code of drift function of LMM sde (terminal measure)(log type)
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftLogLMMTerminal.h"


using namespace std;



//================ AQLPriceDriftLogLMMTerminal ===================================
/*!
	@brief default constructor
	@param[in] Q val for displaced diffusion case
*/
AQLPriceDriftLogLMMTerminal::AQLPriceDriftLogLMMTerminal(double Q)
: AQLPriceDriftLMMTerminal(Q)
{

}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] deltatenor delta of tenor
	@param[in] Q val for displaced diffusion case
*/
AQLPriceDriftLogLMMTerminal::AQLPriceDriftLogLMMTerminal(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q)
: AQLPriceDriftLMMTerminal(sdeAttrName, i, tenor, delta_tenor, Q)
{

}



/*!
	@brief destructor
*/
AQLPriceDriftLogLMMTerminal::~AQLPriceDriftLogLMMTerminal() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftLogLMMTerminal::clone() const	
{
    try 
	{
		return new AQLPriceDriftLogLMMTerminal(*this);
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
AQLPriceDriftLogLMMTerminal::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLOGLMMTERMINAL ? true : AQLPriceDriftLMMBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftLogLMMTerminal::getType() const
{
	return FN_DRIFTLOGLMMTERMINAL;
}

