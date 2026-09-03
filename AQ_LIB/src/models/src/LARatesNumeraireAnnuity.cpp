/*! @file
    @brief Source code of annuity numeraire class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesNumeraireAnnuity.cpp
//
//  SYNOPSIS    :       LARatesNumeraireAnnuity
//  DESCRIPTION :       Source code of annuity numeraire class
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


#include "LARatesNumeraireAnnuity.h"
#include "LAModelDynamicsCurve.h"

using namespace std;

//================ LARatesNumeraireAnnuity ===================================
/*!
	@brief default constructor
	@param[in] isStochasticIR interest rate is stochastic or not
*/
LARatesNumeraireAnnuity::LARatesNumeraireAnnuity(bool isStochasticIR)
: LARatesNumeraireBase(isStochasticIR)
{

}
/*!
	@brief copy constructor
*/
/*LARatesNumeraireAnnuity::LARatesNumeraireAnnuity(const LARatesNumeraireAnnuity& v) 
: LARatesNumeraireBase(v)
{

}*/

/*!
	@brief destructor
*/
LARatesNumeraireAnnuity::~LARatesNumeraireAnnuity() 
{
	mTerms.clear();
	mTimeGrid.clear();
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesNumeraireAnnuity::clone() const
{
    try 
	{
		return new LARatesNumeraireAnnuity(*this);
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
LARatesNumeraireAnnuity::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREANNUITY ? true : LARatesNumeraireBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesNumeraireAnnuity::getType() const
{
	return FN_NUMERAIREANNUITY;
}


/*!
	@brief get numeraire value at t
	@param[in] t time
	@return numeraire value
*/
double
LARatesNumeraireAnnuity::operator()(double t) const
{
	double ret = 0.0;
	if (mIsStochastic)
	{
		const LARatesPathElementCurve& curve = getCurve(t);
		for (unsigned int i = 0; i < mTerms.size(); i++)
			ret += mTerms[i] * curve.getP(mTimeGrid[i]);
	}
	else
	{
		const LARatesPathElementCurve& curve = getCurve(0);
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
LARatesNumeraireAnnuity::setAnnuityInfo(const DoubleArray& grid, const DoubleArray& term)
{
	if (term.size() != grid.size())
	{
		//error
		throw LACoreInvalidData("temr size must be same as grid size", __FILE__, __LINE__);	
	}
	if (term.size() == 0)
	{
		//error
		throw LACoreInvalidData("temr size and grid size is zero", __FILE__, __LINE__);	
	}
	mTerms = term;
	mTimeGrid = grid;
	
/*	if (mIsStochastic)
	{
		mLastCurve.set_t(grid.back()); 
		mCurves[grid.back()] = &mLastCurve;
	}*/

}
