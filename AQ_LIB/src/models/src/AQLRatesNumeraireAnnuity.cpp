/*! @file
    @brief Source code of annuity numeraire class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesNumeraireAnnuity.h"
#include "AQLModelDynamicsCurve.h"

using namespace std;

//================ AQLRatesNumeraireAnnuity ===================================
/*!
	@brief default constructor
	@param[in] isStochasticIR interest rate is stochastic or not
*/
AQLRatesNumeraireAnnuity::AQLRatesNumeraireAnnuity(bool isStochasticIR)
: AQLRatesNumeraireBase(isStochasticIR)
{

}
/*!
	@brief copy constructor
*/
/*AQLRatesNumeraireAnnuity::AQLRatesNumeraireAnnuity(const AQLRatesNumeraireAnnuity& v) 
: AQLRatesNumeraireBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLRatesNumeraireAnnuity::~AQLRatesNumeraireAnnuity() 
{
	mTerms.clear();
	mTimeGrid.clear();
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesNumeraireAnnuity::clone() const
{
    try 
	{
		return new AQLRatesNumeraireAnnuity(*this);
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
AQLRatesNumeraireAnnuity::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREANNUITY ? true : AQLRatesNumeraireBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesNumeraireAnnuity::getType() const
{
	return FN_NUMERAIREANNUITY;
}


/*!
	@brief get numeraire value at t
	@param[in] t time
	@return numeraire value
*/
double
AQLRatesNumeraireAnnuity::operator()(double t) const
{
	double ret = 0.0;
	if (mIsStochastic)
	{
		const AQLRatesPathElementCurve& curve = getCurve(t);
		for (unsigned int i = 0; i < mTerms.size(); i++)
			ret += mTerms[i] * curve.getP(mTimeGrid[i]);
	}
	else
	{
		const AQLRatesPathElementCurve& curve = getCurve(0);
		double div = curve.getP(t);
		for (unsigned int i = 0; i < mTerms.size(); i++)
			ret += mTerms[i] * curve.getP(mTimeGrid[i]) / div;	
	}
	return ret;
}
/*!
	@brief set annuity info(grid, term)
	@param[in] grid time grid
	@param[in] term term
*/
void
AQLRatesNumeraireAnnuity::setAnnuityInfo(const DoubleArray& grid, const DoubleArray& term)
{
	if (term.size() != grid.size())
	{
		//error
		throw AQLCoreInvalidData("temr size must be same as grid size", __FILE__, __LINE__);	
	}
	if (term.size() == 0)
	{
		//error
		throw AQLCoreInvalidData("temr size and grid size is zero", __FILE__, __LINE__);	
	}
	mTerms = term;
	mTimeGrid = grid;
	
/*	if (mIsStochastic)
	{
		mLastCurve.set_t(grid.back()); 
		mCurves[grid.back()] = &mLastCurve;
	}*/

}
