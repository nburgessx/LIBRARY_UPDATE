/*! @file
    @brief source code of trigger action class(option occur action).

*/
//  2006, AlgoQuantHub..
///
//////////////////////////////////////////////////////////
//いくかのパターンでテスト実施ずみ 
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceEventOption.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAObject.h"
#include "LAObjectHolder.h"

#include "LAMathDefine.h"
#include "LAMathValuableEntity.h"

#include "LAPricePayOff.h"

using namespace std;



/*!
    @brief Constructor
*/
LAPriceEventOption::LAPriceEventOption()
: LAPriceEventBase()
{

}
/*!
    @brief Destructor

*/
LAPriceEventOption::~LAPriceEventOption()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceEventOption::clone() const    
{
    try 
	{
        return new LAPriceEventOption(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceEventOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONOPTION ? true : LAPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceEventOption::getType() const
{
	return FN_IR_TRIGGERACTIONOPTION;
}

/*!
	@brief execute trigger action
	@param[in] actiondate action date
	@param[in] actiontime action time
	@param[in,out] payoff payoff
	@param[in,out] extrapayoff extra payoff
	@param[in,out] futureaction future action
	@param[in,out] pastaction past action
	@param[in,out] iter position of nearest payoff from this action expiry date
*/	
void
LAPriceEventOption::doAction(const LADate& actiondate,
								  double actiontime,
								  vector<PayOffToolHolderVector>& payoff,
									 vector<PayOffToolHolderVector>& extrapayoff,
									 vector<LAPriceEventHolder*>& futureaction,
									 vector<LAPriceEventHolder*>& pastaction,
									 vector<PayOffToolHolderIter>& iter)
{
	(void)iter; (void)pastaction; (void)futureaction; (void)payoff; //20070411--Nagase--警告削除をgccにも対応
	for(unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
		LAPricePayOffToolHolder ph(new LAPricePayOffTool());
		LAPricePayOffTool& payofftool = ph.getPayOff();
		payofftool.mPaymentDate = actiondate;
		payofftool.mPaymentTime = actiontime;
		payofftool.mExtraCF = mOption->value(actiondate);
		payofftool.calcPayOff();
		extrapayoff[mTargetLegNo[i]].push_back(ph);
	}
}

/*!
	@brief set up this class
    @param[in] basedate basedate
    @param[in] trade trade object
    @param[in,out] triggerinfo trigger/call information object
    @param[in] payoff payoff
    @param[in] isCall call flag(true:call,false:trigger)
*/
void
LAPriceEventOption::setUp(const LADate& basedate,	
									const LAObject& trade,
									LAObject& triggerinfo,
									const LAPricePayOff& payoff,
									bool isCall)
{
	LAPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);

	LADataHolder* dh;

	//option 
	dh = &(triggerinfo.getData(PRICING_DATA_OPTION, ISNOTNULL));
	LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
	mOption = &dynamic_cast<LAMathObjectValue&>(ref.get().get());

}


