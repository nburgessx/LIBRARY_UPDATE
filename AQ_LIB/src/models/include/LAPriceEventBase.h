#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreFunctionBase.h"
#include "LACoreFunctionHolder.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADate.h"
#include "LAPricePayOffTool.h"
#include "LAPriceIndexTool.h"


// LAPriceEventBase's function id
#define FN_IR_TRIGGERACTIONBASE		2003
// LAPriceEventBase's function name
#define FN_IR_TRIGGERACTIONBASE_STR	"fn_ir_triggeractionbase"



class LAPriceEventHolder;
class LAPriceEventOption;
class LARatesNumeraireBase;
class LAPricePayOff;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of abstract base class of trigger action.
*/
class LAPriceEventBase : public LACoreFunctionBase
{
friend class LAPriceEventTool;
public:
//  LIFECYCLE
    // constructor	
	LAPriceEventBase();
    // destructor	
	virtual ~LAPriceEventBase();
	// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
	/*!
		@brief make copy(clone) of this class
	    @return deep copy of this class
	*/	
	virtual LACoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
	// Return this class type
    virtual function_t          getType() const;
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
	virtual void	            doAction(const LADate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<LAPriceEventHolder*>& futureaction,
										 std::vector<LAPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter) = 0;

	// set up this class
	virtual	void				setUp(const LADate& basedate,	
									const LAObject& trade,
									LAObject& triggerinfo,
									const LAPricePayOff& payoff,
									bool isCall);
	



protected:
	UintArray	mTargetLegNo;// target leg number(first leg is number 0)
};

/*! 
    @brief declaration of holder class of trigger action class.
*/
class LAPriceEventHolder
{
friend class Comp_pTriggerActionHolder;
friend class LAPriceEventTool;
friend class LAPriceEventBase;
friend class LAPricePayOff;

public:
//  LIFECYCLE
    // constructor	
	LAPriceEventHolder();
    // destructor	
	virtual ~LAPriceEventHolder();

    /*!
		@brief get action
		@return action
	*/
	const std::vector<LAPriceEventBase*>&
								getAction() const {return mAction;}
	
	// check trigger hit and if hit, execute trigger action
	virtual void	            doAction(std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<LAPriceEventHolder*>& futureaction,
										 std::vector<LAPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter);
	/*!
		@brief set up this class
	    @param[in] basedate basedate
	    @param[in] trade trade object
	    @param[in,out] triggerinfo trigger/call information object
	    @param[in] payoff payoff
	    @param[in] isCall call flag(true:call,false:trigger)
	*/
	virtual	void				setUp(const LADate& basedate,	
									const LAObject& trade,
									LAObject& triggerinfo,
									const LAPricePayOff& payoff,
									bool isCall)
								{
									(void)payoff; (void)triggerinfo; (void)trade; (void)basedate; //20070411--Nagase--åxçêçÌèúÇgccÇ…Ç‡ëŒâû
									mIsCall = isCall;
									if (isCall) ;//mX.resize(mIndex.size() + 1);
									else mX.resize(mIndex.size());
								}
	
	/*!
		@brief get id

	    @return id 
	*/	
	unsigned int				getID(void) const {return mID;}
	/*!
		@brief get action dates
	    @return action dates 
	*/	
	const DateVector&			getActionDates(void) const {return mActionDates;}
	/*!
		@brief get action times
	    @return action times 
	*/	
	const DoubleArray&			getActionTimes(void) const {return mActionTimes;}
	/*!
		@brief return call or not

	    @return call(true), trigger(false) 
	*/	
	bool						isCall() const {return mIsCall;}
	// calculate call rebate
	double						calcCallRebate(std::vector<PayOffToolHolderVector>& payoff,
											std::vector<PayOffToolHolderVector>& extrapayoff,
											std::vector<LAPriceEventHolder*>& futureaction,
											std::vector<LAPriceEventHolder*>& pastaction,
											std::vector<PayOffToolHolderIter>& iter) const;
	// calculate index
	void						calcIndex(DoubleArray& index);
protected:
	
private:


	unsigned int mID;// id number
	LADate	mExpiryDate;// expiry date
	double	mExpiryTime;// expiry time
	DateVector	mActionDates;// action date
	DoubleArray mActionTimes;// action time
	bool	mIsOneTime;// one time trigger(true) or multi time trigger(false)  
	bool*	mpEffectiveFlag;// trigger effective or not 	
	LAFunctionBase*			mpOperator;// operator for trigger hit judge
	LACoreFunctionHolder		mOperatorHolder;// function holder of operator for trigger hit judge
	DoubleArray mX;// variable for trigger hit judge
	std::vector<LAPriceIndexToolBase*>	mIndex;// reference index 
	std::vector<LAPriceEventBase*>		mAction;// trigger action
	bool	mIsCall;							// call flag
	const LARatesNumeraireBase*		mpNumeraire;// numeraire
	BoolVector						mRcvPay;	// rcv(true) or pay(false) flag of each leg (use for call)


};


/*!
    @brief class that compares two action holder by expiry date
*/
class Comp_pTriggerActionHolder
{
public:
	/*!
		@brief compare maturity date
		@param[in] _Left one pointer to TriggerActionBase
		@param[in] _Right another pointer to TriggerActionBase
		@return true when right argument expiry date > left argument expiry date
	*/
	bool operator()(const LAPriceEventHolder* _Left, const LAPriceEventHolder* _Right) const
	{
		if (_Left->mExpiryDate < _Right->mExpiryDate) return true;
		if (_Left->mExpiryDate == _Right->mExpiryDate)
            return _Left->mID < _Right->mID;
		return false;
	};
};

