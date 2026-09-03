/*! @file
    @brief Source code of class to represent delayed caplet function
*/

#include "LABlackScholesDelayedCapletOptionPayoff.h"
#include "AQLBasic.h"
#include "AQLDist.h"
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
AQLCoreFunctionBase*	
LABlackScholesDelayedCapletOption::clone() const
{
    try 
	{
		return new LABlackScholesDelayedCapletOption(*this);
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
		throw AQLCoreInvalidData("Convexity factors size error", __FILE__, __LINE__);

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
		vol = AQLMath::min(vol, mVolCutoff);
	}

	const double ret = mTimingTerm * mDFFromPayment2End * x.Nu * x.F * (x.F * AQLMath::exp(vol*vol*x.Te) * AQLDist::normsdist(AnalyticFormulae::BKd1(x)) - x.K * AQLDist::normsdist(AnalyticFormulae::BKd2(x)) - premium);
	return ret;
}

