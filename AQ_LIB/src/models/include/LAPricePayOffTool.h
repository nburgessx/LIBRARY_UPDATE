#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLPriceDataDayCount.h"
#include "LAPriceCFGenUtility.h"


#include <memory>



class AQLObject;
class LAPriceCouponTool;
class LAMathFXEntity;
class AQLFunctionBase;
class LAPricePayOffToolHolder;
class Comp_pLAPricePayOffTool;
class LAPricePayOff;
class LAPriceCouponBase;
class AQLInterpolationBase;
class LACompoundMethod;
class AQLDataReference;
class AQLDataDoubles;

typedef std::vector<LAPricePayOffToolHolder>			PayOffToolHolderVector;   // vector of LAPricePayOffToolHolder
typedef std::vector<LAPricePayOffToolHolder>::iterator	PayOffToolHolderIter;   // iterator of LAPricePayOffToolHolder vector



///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of tool class of payoff calculation.
*/
class LAPricePayOffTool
{
/*friend class Comp_pLAPricePayOffTool;
friend class LAPricePayOff;
friend class LAPriceEventExtraCF;
friend class LAPriceEventOption;
friend class LAPriceEventCpnChange;
friend class LAPriceEventNotionalChange;
friend class LAPriceEventNotExCurChange;
friend class LAPriceEventTerminate;*/
friend class LAPricePayOffToolHolder;

public:
//  LIFECYCLE
    // default constructor	
	LAPricePayOffTool();
    // destructor	
	virtual ~LAPricePayOffTool();
	//	copy constructor
	LAPricePayOffTool (const LAPricePayOffTool& v);
	// calculate payoff
	virtual double				calcPayOff(void) const;
	/*!
		@brief get payoff that has calulated lastly
		@return payoff
	*/	
	double						getPayOff(void) const {return mPayOff;}	
	/*!
		@brief get payoff by payoff currency that has calulated lastly
		@return payoff by payoff currency
	*/		
	double						getCpnPayOffbyPayOffCur(void) const {return mCpnPayOffbyPayOffCur;}	
	/*!
		@brief get payment date
		@return payment date
	*/	
	const AQLDate&				getPaymentDate(void) const {return mPaymentDate;}
	/*!
		@brief get coupon that has calulated lastly
		@return coupon
	*/	
	double						getCoupon(void) const {return mCoupon;}
	/*!
		@brief get coupon that has calulated lastly
		@return coupon before selected though the coupon select operator
	*/	
	double	                    getCouponBeforeSelection(unsigned int i) const;

	/*!
		@brief check coupon payment or only extra cf payment
		@return true:coupon payment,false:extracf only
	*/	
	bool						isCouponPayment(void) const {return mpCoupons.size() > 0;}
	/*!
		@brief get notional
		@return notional
	*/	
	double						getNotional(void) const {return mNotional;}
	/*!
		@brief set notional
		@param[in] notional notional to set
	*/		
	void						setNotional(const double notional) {mNotional = notional;}
	// set up this class
	virtual	void				setUp(const AQLDate& basedate, const AQLObject& trade,
									unsigned int legNo, const AQLObject& cashlet, 
									const LAPricePayOff& payoff,
									unsigned int currentpos);

	// copy LAPricePayOffTool excepy members about coupon
	LAPricePayOffTool&				copy(const LAPricePayOffTool& a);


	// calculate accrued interest
	double						calcAccruedInt(const AQLDate& date) const;
	// calculate accrued interest by payoff currency
	double						calcAccruedIntbyPayOffCur(const AQLDate& date) const;
	// calculate payoff
	virtual void calcDerivationOfLibor(std::map<AQLString, std::map<double, double> >& delivationLiborMap, const AQLInterpolationBase* pNumeInterp, bool isRec) const;
	virtual void getCompoundedRateInfo(DateVector& start, DateVector& end, DoubleVector& term, DateVector& fixing_date, AQLStringVector& fixing_flag, DoubleVector& rate) const;
	virtual void setFlagForUpFrontFee(const bool b){ flagForUpFrontFee = b; }
	virtual bool isUpFrontFee() const { return flagForUpFrontFee; }
    virtual bool isCompounding() const { return false; }


	
	AQLDate	mPaymentDate;// payment date 
	double	mPaymentTime;// payment time
	double	mTerm;// interest calculation term
	AQLDate	mStart;// interest calculation startdate;
	AQLDate  mEnd;// interest calculation enddate;
	AQLPriceDataDayCount mDC;// day count for interest calculation
	double	mNotional;// notional
	double	mDenomiRatio;// denominator ratio
	
	bool mIsRound;// round flag
	RoundFunction	mRoundFunction;// round function
	int	mRoundDigit;// round digit

	double	mExtraCF;// extra cf
	double  mExtraCFFdChg;// extra cf for funding change
	double	mExtraCFFixingTime;// extra cf fixing time

	std::vector<LAPriceCouponTool*>		mpCoupons;// coupon tools
	LAPriceCouponBase*					mpCpnSelectOperator;// coupon select operator

	double	mNotionalCF;// notional cf by mNotionalCur
	AQLDate	mRenotionalFixingDate;// notional cf by mNotionalCur


	double	mAccruedInt;// accrued interest

	mutable const LAMathFXEntity*			mpFX_for_PayOff;// fx rate for payoff
	mutable const LAMathFXEntity*			mpFX_for_Notional;// fx rate for notional
	mutable const LAMathFXEntity*			mpFX_for_ExtraCF;// fx rate for extra cf
	const AQLDataReference*					mpRefFX_for_PayOff;// reference to fx rate for payoff
	const AQLDataReference*					mpRefFX_for_Notional;//reference to fx rate for notional
	const AQLDataReference*					mpRefFX_for_ExtraCF;// reference to fx rate for extra cf
	double									mNotionalCFExchangeRate;// notional cf exchange rate
    std::unique_ptr<const double>			mpCouponCFFXRateValue;

//	std::vector<const LAMathFXEntity*>	mpFX_for_Coupons;// fx rate for coupon

	AQLString	mBaseCur;// base currency
	AQLString	mNotionalCur;// notional currency
	AQLString	mExtraCFCur;// extra cf currency
	AQLString    mNotionalCFCur;// notional cf payment currency
	

	AQLStringVector mCouponsCur;// coupon currency


	bool			mIsArrear;// Arrear(true) or Advance(false)

	mutable double mCoupon;// coupon 
	mutable double mCpnPayOffbyPayOffCur;// coupon payoff by payoff currency
	mutable double mCouponPayOff;// coupon payoff
	mutable double mPayOff;// payoff
	mutable	unsigned int mSelectedCpnPos;// selected coupon number

	std::vector<LAPriceCouponTool*>        mpNotionalCFCoupons;// notional coupon
	LAPriceCouponBase*             mpNotionalCFCpnSelectOperator;// notional cf coupon select operator
	std::vector<AQLString>				mNotionalCFCouponsCur;// notional cf payment currency

	bool flagForUpFrontFee;

	// for non-deliverable (vanilla and exotic) swaps 
	double mSettlementAdjustRatio;
	AQLDate mSettlementFixingDate;
	bool mIsNonDeliverable;

protected:
	/*!
		@brief round(or round_up or round_down) coupon
		@param[in, out] payoff payoff value
	*/
	void						round(double& payoff) const
								{
									if (!mIsRound) return;
									payoff = LAPriceCFGenUtility::round(payoff, mRoundFunction, mRoundDigit);									
								}

    virtual LAPriceCouponTool* createCouponTool(const AQLObject& cashlet, const AQLObject& coupon_info);
    virtual double calculateCouponCashflow() const;

private:
	long mRefCount;// reference count
	bool mIsTermCalc;
	bool mIsRedemption;
};

/*! 
    @brief declaration of holder class of payoff calculation tool class.
*/
class LAPricePayOffToolHolder
{
public:
//  LIFECYCLE
    // default constructor	
	explicit LAPricePayOffToolHolder(LAPricePayOffTool* p = NULL);
    // destructor	
	virtual ~LAPricePayOffToolHolder();
	//	Copy constructor
	LAPricePayOffToolHolder (const LAPricePayOffToolHolder& v);
	/*!
		@brief get payoff tool
		@return payoff tool
	*/	
	const LAPricePayOffTool&	getPayOff() const {return *mpPayOff;}
	/*!
		@brief get payoff tool
		@return payoff tool
	*/		
	LAPricePayOffTool&			getPayOff() {return *mpPayOff;}
	/*!
		@brief set payoff tool
		@param[in] payoff tool
	*/	
	void					setPayOff(LAPricePayOffTool* p) 
							{
								if (mpPayOff != NULL)
								{
									if (mpPayOff->mRefCount-- == 1) delete mpPayOff;
								}
								mpPayOff = p;
								if (mpPayOff != NULL) mpPayOff->mRefCount ++;
							}
	/*!
		@brief assignment operator
		@param[in] objectHolder copy source
		@return holder class of payoff calculator tool class
	*/	
	LAPricePayOffToolHolder&	operator = (const LAPricePayOffToolHolder& objectHolder) {return copy(objectHolder);}


private:
	/*!
		@brief copy method
		@param[in] e copy source
		@return holder class of payoff calculator tool class
	*/	
	LAPricePayOffToolHolder&	copy(const LAPricePayOffToolHolder& e)
							{
								setPayOff(e.mpPayOff);
								return *this;
							}
	LAPricePayOffTool*	mpPayOff;// pointer to payoff tool class

};


/*!
    @brief class that compares two payoff calculator tool class by payment date
*/
class Comp_pLAPricePayOffTool
{
public:
	/*!
		@brief compare maturity date
		@param[in] _Left one pointer to LAPricePayOffTool
		@param[in] _Right another pointer to LAPricePayOffTool
		@return true when right argument payment date > left argument payment date
	*/
	bool operator()(const LAPricePayOffTool* _Left, const LAPricePayOffTool* _Right) const
	{
		return _Left->mPaymentDate < _Right->mPaymentDate;
	};
};

class LAPricePayOffToolCompound : public LAPricePayOffTool
{
public:
	virtual	void setUp(const AQLDate& basedate, 
                       const AQLObject& trade,
                       unsigned int legNo,
                       const AQLObject& cashlet, 
                       const LAPricePayOff& payoff,
                       unsigned int currentpos);
    virtual bool isCompounding() const { return true; }
    virtual void setCashflow(double& gearing, double& forward, double& spread, AQLDate& fixing_date, AQLString& fixing_flag) const;
    void getCompoundedRateInfo(DateVector& start, DateVector& end, DoubleVector& term, DateVector& fixing_date, AQLStringVector& fixing_flag, DoubleVector& rate) const;

protected:
    virtual LAPriceCouponTool* createCouponTool(const AQLObject& cashlet, const AQLObject& coupon_info);
    virtual double calculateCouponCashflow() const;
    virtual void setupStartEndDates(const AQLObject& cashlet);
    virtual LAPriceCouponTool* getStubCoupon(const AQLString& stub_coupon_name, const AQLObject& cashlet_info);

    LAPriceCouponTool *first_stub, *last_stub;

    mutable DoubleArray rate_term_spread;
    const LACompoundMethod* compounding_function;

    bool compound_all_days;
    DateVector start_dates, end_dates;
	const AQLDataDoubles* mpCoefficient;

};

