/*! @file
    @brief Source code of drift function of LMM sde (terminal measure)(log type)



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftLogLMMTerminal.cpp
//
//  SYNOPSIS    :       LAPriceDriftLogLMMTerminal
//  DESCRIPTION :       Source code of drift function of LMM sde (terminal measure)(log type)
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


#include "LAPriceDriftLogLMMTerminal.h"


using namespace std;



//================ LAPriceDriftLogLMMTerminal ===================================
/*!
	@brief default constructor
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLogLMMTerminal::LAPriceDriftLogLMMTerminal(double Q)
: LAPriceDriftLMMTerminal(Q)
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
LAPriceDriftLogLMMTerminal::LAPriceDriftLogLMMTerminal(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q)
: LAPriceDriftLMMTerminal(sdeAttrName, i, tenor, delta_tenor, Q)
{

}



/*!
	@brief destructor
*/
LAPriceDriftLogLMMTerminal::~LAPriceDriftLogLMMTerminal() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftLogLMMTerminal::clone() const	
{
    try 
	{
		return new LAPriceDriftLogLMMTerminal(*this);
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
LAPriceDriftLogLMMTerminal::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLOGLMMTERMINAL ? true : LAPriceDriftLMMBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftLogLMMTerminal::getType() const
{
	return FN_DRIFTLOGLMMTERMINAL;
}

