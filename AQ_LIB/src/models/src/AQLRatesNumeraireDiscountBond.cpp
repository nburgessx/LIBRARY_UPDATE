/*! @file
    @brief Source code of discount bond numeraire class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesNumeraireDiscountBond.h"
#include "AQLModelDynamicsCurve.h"

using namespace std;
const double INFINITESIMAL = 1E-7; 

//================ AQLRatesNumeraireDiscountBond ===================================
/*!
	@brief default constructor
	@param[in] T maturity of numeraire discount bond 
	@param[in] isStochasticIR interest rate is stochastic or not
*/
AQLRatesNumeraireDiscountBond::AQLRatesNumeraireDiscountBond(double T, bool isStochasticIR)
: mTerminal(T), AQLRatesNumeraireBase(isStochasticIR) 

{

}
/*!
	@brief copy constructor
*/
/*AQLRatesNumeraireDiscountBond::AQLRatesNumeraireDiscountBond(const AQLRatesNumeraireDiscountBond& v) 
: AQLRatesNumeraireBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLRatesNumeraireDiscountBond::~AQLRatesNumeraireDiscountBond() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesNumeraireDiscountBond::clone() const
{
    try 
	{
		return new AQLRatesNumeraireDiscountBond(*this);
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
AQLRatesNumeraireDiscountBond::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREDISCOUNTBOND ? true : AQLRatesNumeraireBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesNumeraireDiscountBond::getType() const
{
	return FN_NUMERAIREDISCOUNTBOND;
}


/*!
	@brief get numeraire value at t
	@param[in] t time
	@return numeraire value
*/
double
AQLRatesNumeraireDiscountBond::operator()(double t) const
{
	if (t > mTerminal + INFINITESIMAL || t < 0.0)
	{
		//error
        throw AQLCoreInvalidData("input t is before 0 or after Terminal", __FILE__, __LINE__);
	}
	if (t > mTerminal) return 1.0;
	if (mIsStochastic) return getCurve(t).getP(mTerminal);
	const AQLRatesPathElementCurve& curve = getCurve(0);
	return curve.getP(mTerminal) / curve.getP(t);

}

/*!
	@brief set terminal
	@param[in] T terminal
*/	
void
AQLRatesNumeraireDiscountBond::setTerminal(double T) 
{
	mTerminal = T; 
/*	if (mIsStochastic)
	{
		mLastCurve.set_t(T); 
		mCurves[mTerminal] = &mLastCurve;
	}*/
}
