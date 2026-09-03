/*! @file
    @brief Source code of discount bond numeraire class



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesNumeraireDiscountBond.cpp
//
//  SYNOPSIS    :       LARatesNumeraireDiscountBond
//  DESCRIPTION :       Source code of discount bond numeraire class
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


#include "LARatesNumeraireDiscountBond.h"
#include "LAModelDynamicsCurve.h"

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ LARatesNumeraireDiscountBond ===================================
/*!
	@brief default constructor
	@param[in] T maturity of numeraire discount bond 
	@param[in] isStochasticIR interest rate is stochastic or not
*/
LARatesNumeraireDiscountBond::LARatesNumeraireDiscountBond(double T, bool isStochasticIR)
: mTerminal(T), LARatesNumeraireBase(isStochasticIR) 

{

}
/*!
	@brief copy constructor
*/
/*LARatesNumeraireDiscountBond::LARatesNumeraireDiscountBond(const LARatesNumeraireDiscountBond& v) 
: LARatesNumeraireBase(v)
{

}*/

/*!
	@brief destructor
*/
LARatesNumeraireDiscountBond::~LARatesNumeraireDiscountBond() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesNumeraireDiscountBond::clone() const
{
    try 
	{
		return new LARatesNumeraireDiscountBond(*this);
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
LARatesNumeraireDiscountBond::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREDISCOUNTBOND ? true : LARatesNumeraireBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesNumeraireDiscountBond::getType() const
{
	return FN_NUMERAIREDISCOUNTBOND;
}


/*!
	@brief get numeraire value at t
	@param[in] t time
	@return numeraire value
*/
double
LARatesNumeraireDiscountBond::operator()(double t) const
{
	if (t > mTerminal + INFINITESIMAL || t < 0.0)
	{
		//error
        throw LACoreInvalidData("input t is before 0 or after Terminal", __FILE__, __LINE__);
	}
	if (t > mTerminal) return 1.0;
	if (mIsStochastic) return getCurve(t).getP(mTerminal);
	const LARatesPathElementCurve& curve = getCurve(0);
	return curve.getP(mTerminal) / curve.getP(t);

}

/*!
	@brief set terminal
	@param[in] T terminal
*/	
void
LARatesNumeraireDiscountBond::setTerminal(double T) 
{
	mTerminal = T; 
/*	if (mIsStochastic)
	{
		mLastCurve.set_t(T); 
		mCurves[mTerminal] = &mLastCurve;
	}*/
}
