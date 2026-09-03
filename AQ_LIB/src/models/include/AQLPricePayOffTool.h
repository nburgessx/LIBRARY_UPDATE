#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceCFGenUtility.h"


#include <memory>



class AQLObject;
class AQLPriceCouponTool;
class AQLMathFXEntity;
class AQLFunctionBase;
class AQLPricePayOffToolHolder;
class Comp_pLAPricePayOffTool;
class AQLPricePayOff;
class AQLPriceCouponBase;
class AQLInterpolationBase;
class AQLCompoundMethod;
class AQLDataReference;
class AQLDataDoubles;

typedef std::vector<AQLPricePayOffToolHolder>			PayOffToolHolderVector;   // vector of AQLPricePayOffToolHolder
typedef std::vector<AQLPricePayOffToolHolder>::iterator	PayOffToolHolderIter;   // iterator of AQLPricePayOffToolHolder vector



///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of tool class of payoff calculation.
*/
class AQLPricePayOffTool
{
/*friend class Comp_pLAPricePayOffTool;
friend class AQLPricePayOff;
friend class AQLPriceEventExtraCF;
friend class AQLPriceEventOption;
friend class AQLPriceEventCpnChange;
friend class AQLPriceEventNotionalChange;
friend class AQLPriceEventNotExCurChange;
friend class AQLPriceEventTerminate;*/
friend class AQLPricePayOffToolHolder;

public:
//  LIFECYCLE
    // default constructor	
	AQLPricePayOffTool();
    // destructor	
	virtual ~AQLPricePayOffTool();
	//	copy constructor
	AQLPricePayOffTool (const AQLPricePayOffTool& v);
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
									const AQLPricePayOff& payoff,
									unsigned int currentpos);

	// copy AQLPricePayOffTool excepy members about coupon
	AQLPricePayOffTool&				copy(const AQLPricePayOffTool& a);


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

	std::vector<AQLPriceCouponTool*>		mpCoupons;// coupon tools
	AQLPriceCouponBase*					mpCpnSelectOperator;// coupon select operator

	double	mNotionalCF;// notional cf by mNotionalCur
	AQLDate	mRenotionalFixingDate;// notional cf by mNotionalCur


	double	mAccruedInt;// accrued interest

	mutable const AQLMathFXEntity*			mpFX_for_PayOff;// fx rate for payoff
	mutable const AQLMathFXEntity*			mpFX_for_Notional;// fx rate for notional
	mutable const AQLMathFXEntity*			mpFX_for_ExtraCF;// fx rate for extra cf
	const AQLDataReference*					mpRefFX_for_PayOff;// reference to fx rate for payoff
	const AQLDataReference*					mpRefFX_for_Notional;//reference to fx rate for notional
	const AQLDataReference*					mpRefFX_for_ExtraCF;// reference to fx rate for extra cf
	double									mNotionalCFExchangeRate;// notional cf exchange rate
    std::unique_ptr<const double>			mpCouponCFFXRateValue;

//	std::vector<const AQLMathFXEntity*>	mpFX_for_Coupons;// fx rate for coupon

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

	std::vector<AQLPriceCouponTool*>        mpNotionalCFCoupons;// notional coupon
	AQLPriceCouponBase*             mpNotionalCFCpnSelectOperator;// notional cf coupon select operator
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
									payoff = AQLPriceCFGenUtility::round(payoff, mRoundFunction, mRoundDigit);									
								}

    virtual AQLPriceCouponTool* createCouponTool(const AQLObject& cashlet, const AQLObject& coupon_info);
    virtual double calculateCouponCashflow() const;

private:
	long mRefCount;// reference count
	bool mIsTermCalc;
	bool mIsRedemption;
};

/*! 
    @brief declaration of holder class of payoff calculation tool class.
*/
class AQLPricePayOffToolHolder
{
public:
//  LIFECYCLE
    // default constructor	
	explicit AQLPricePayOffToolHolder(AQLPricePayOffTool* p = NULL);
    // destructor	
	virtual ~AQLPricePayOffToolHolder();
	//	Copy constructor
	AQLPricePayOffToolHolder (const AQLPricePayOffToolHolder& v);
	/*!
		@brief get payoff tool
		@return payoff tool
	*/	
	const AQLPricePayOffTool&	getPayOff() const {return *mpPayOff;}
	/*!
		@brief get payoff tool
		@return payoff tool
	*/		
	AQLPricePayOffTool&			getPayOff() {return *mpPayOff;}
	/*!
		@brief set payoff tool
		@param[in] payoff tool
	*/	
	void					setPayOff(AQLPricePayOffTool* p) 
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
	AQLPricePayOffToolHolder&	operator = (const AQLPricePayOffToolHolder& objectHolder) {return copy(objectHolder);}


private:
	/*!
		@brief copy method
		@param[in] e copy source
		@return holder class of payoff calculator tool class
	*/	
	AQLPricePayOffToolHolder&	copy(const AQLPricePayOffToolHolder& e)
							{
								setPayOff(e.mpPayOff);
								return *this;
							}
	AQLPricePayOffTool*	mpPayOff;// pointer to payoff tool class

};


/*!
    @brief class that compares two payoff calculator tool class by payment date
*/
class Comp_pLAPricePayOffTool
{
public:
	/*!
		@brief compare maturity date
		@param[in] _Left one pointer to AQLPricePayOffTool
		@param[in] _Right another pointer to AQLPricePayOffTool
		@return true when right argument payment date > left argument payment date
	*/
	bool operator()(const AQLPricePayOffTool* _Left, const AQLPricePayOffTool* _Right) const
	{
		return _Left->mPaymentDate < _Right->mPaymentDate;
	};
};

class AQLPricePayOffToolCompound : public AQLPricePayOffTool
{
public:
	virtual	void setUp(const AQLDate& basedate, 
                       const AQLObject& trade,
                       unsigned int legNo,
                       const AQLObject& cashlet, 
                       const AQLPricePayOff& payoff,
                       unsigned int currentpos);
    virtual bool isCompounding() const { return true; }
    virtual void setCashflow(double& gearing, double& forward, double& spread, AQLDate& fixing_date, AQLString& fixing_flag) const;
    void getCompoundedRateInfo(DateVector& start, DateVector& end, DoubleVector& term, DateVector& fixing_date, AQLStringVector& fixing_flag, DoubleVector& rate) const;

protected:
    virtual AQLPriceCouponTool* createCouponTool(const AQLObject& cashlet, const AQLObject& coupon_info);
    virtual double calculateCouponCashflow() const;
    virtual void setupStartEndDates(const AQLObject& cashlet);
    virtual AQLPriceCouponTool* getStubCoupon(const AQLString& stub_coupon_name, const AQLObject& cashlet_info);

    AQLPriceCouponTool *first_stub, *last_stub;

    mutable DoubleArray rate_term_spread;
    const AQLCompoundMethod* compounding_function;

    bool compound_all_days;
    DateVector start_dates, end_dates;
	const AQLDataDoubles* mpCoefficient;

};

