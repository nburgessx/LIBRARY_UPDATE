/*! @file
    @brief Source code of class to represent delayed caplet function
*/

#include "LABlackScholesDelayedCapletOptionPayoff.h"
#include "LABasic.h"
#include "LADist.h"
#include "LAAnalyticFormula.h"

LABlackScholesDelayedCapletOption::LABlackScholesDelayedCapletOption()
: LABlackScholesCapletOption(), mTimingTerm(0.0), mDFFromPayment2End(0.0)
{

}

/*!
	@brief destructor
*/
LABlackScholesDelayedCapletOption::~LABlackScholesDelayedCapletOption()
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LABlackScholesDelayedCapletOption::clone() const
{
    try 
	{
		return new LABlackScholesDelayedCapletOption(*this);
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
LABlackScholesDelayedCapletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_DELAYEDCAPLETOPTIONFUNC ? true : LABlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LABlackScholesDelayedCapletOption::getType() const
{
	return FN_IR_DELAYEDCAPLETOPTIONFUNC;
}

void
LABlackScholesDelayedCapletOption::setConvexityFactors(const DoubleArray& x)
{
	if (x.size() < 3 )
		throw LACoreInvalidData("Convexity factors size error", __FILE__, __LINE__);

	mTimingTerm = x[0];
	mDFFromPayment2End = x[1];
	mVolCutoff = x[2];
}

double
LABlackScholesDelayedCapletOption::calcConvexityAdjustment(AnalyticParam& param, double premium) const
{
	if (std::fabs(mTimingTerm) < 1. / 365)
	{
		return 0.;
	}

	AnalyticBKParam& x = dynamic_cast<AnalyticBKParam&>(param);
	double vol = x.Vol;
	if (mVolCutoff > 0.)
	{
		vol = LAMath::min(vol, mVolCutoff);
	}

	const double ret = mTimingTerm * mDFFromPayment2End * x.Nu * x.F * (x.F * LAMath::exp(vol*vol*x.Te) * LADist::normsdist(AnalyticFormulae::BKd1(x)) - x.K * LADist::normsdist(AnalyticFormulae::BKd2(x)) - premium);
	return ret;
}

