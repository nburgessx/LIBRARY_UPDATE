#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLFunctionBase.h"
#include "AQLConstant.h"
#include "LARatesAccruedCouponFuncBase.h"
#include "LAPriceIndexTool.h"
#include "LAPriceCFGenUtility.h"

#include <cmath>




class AQLObject;
class LAPricePayOff;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of tool class of coupon calculation.
*/
class LAPriceCouponTool
{
public:
//  LIFECYCLE
    // default constructor	
	LAPriceCouponTool();
    // destructor	
	virtual ~LAPriceCouponTool();
    // copy constructor	
	LAPriceCouponTool(const LAPriceCouponTool& v);
	/*!
		@brief calculate coupon
		@return coupon
	*/			
	virtual double				calcCoupon(void) const
								{
									unsigned int i;
									for (i = 0 ; i < mpIndexs.size(); i++)
										mX[i] = mpIndexs[i]->calcIndex();
									if (mpCoefficient)
									{
										mpOperator->setParam(mpCoefficient->get());
									}
									double coupon = mpOperator->operator ()(mX);
									
									if (mIsObservationTerm)
									{
										if (mFixedRates.size() != 0)
										{
											for(i = 0; i < mFixedRates.size(); i++)
												mX2[i] = mFixedRates[i];
										}
										if (mPayOffPos.size() != 0)
										{
											for(i = 0; i < mPayOffPos.size(); i++)
												mX2[i + mFixedRates.size()] = mpPayOff->getPayOff()[mLegNo][mPayOffPos[i]]
													.getPayOff().getCoupon();
											mX2[i + mFixedRates.size()] = coupon;
											coupon = mpObservationOperator->operator ()(mX2);
										}
									}
									round(coupon);
									
									capfloor(coupon);
									mCoupon = coupon;
									return coupon;
								}
	/*!
		@brief get coupon that has calulated lastly
		@return coupon
	*/		
	double						getCoupon(void) const {return mCoupon;}
	// set up this class
	virtual	void				setUp(const AQLDate& basedate,	
									const AQLObject& trade,
									unsigned int legNo,
									const AQLObject& couponinfo,
									const LAPricePayOff& payoff,
									unsigned int currentpos);

	// ! Get Index Value
	DoubleArray					&getX(void) const{ return mX;}

	bool						isCompoundCoupon(void) const {return mIsCompoundCoupon;}
	// ! Get Operator
	const AQLFunctionBase*		getOperator(void) const{ return mpOperator;}

	std::vector<LAPriceIndexToolBase*> getIndexs(void) const {return mpIndexs;}// index

protected:
	/*!
		@brief round(or round_up or round_down) coupon
		@param[in, out] coupon coupon value
	*/
	void						round(double& coupon) const
								{
									if (!mIsRound) return;
									coupon = LAPriceCFGenUtility::round(coupon, mRoundFunction, mRoundDigit);									
								}
	/*!
		@brief cap or floor coupon
		@param[in, out] coupon coupon value
	*/
	void						capfloor(double& coupon) const
								{
									double notionalandterm = 1.0;
									if((mIsCap && mpCap->isTypeOf(FN_ACCRUEDCOUPONFUNCBASE))
										|| (mIsFloor && mpFloor->isTypeOf(FN_ACCRUEDCOUPONFUNCBASE)))
									{
										getPastCouponPayoffs(mX3);
										const LAPricePayOffTool *mp = &(mpPayOff->getPayOffMaster())[mLegNo][mCurrentPos].getPayOff();	
										notionalandterm = mp->mTerm * mp->mNotional;
										mX3.back() = coupon*notionalandterm;
									}
									else if((mIsCap && !mpCap->isTypeOf(FN_CONSTANT))
										|| (mIsFloor && !mpFloor->isTypeOf(FN_CONSTANT)))
									{
										getPastCoupons(mX3);
										mX3.back() = coupon;
									}
									if (mIsCap)
									{
										double accruedcap = (*mpCap)(mX3);
										double accruedcoupon = coupon * notionalandterm;
										coupon = accruedcoupon < accruedcap ? coupon : accruedcap/notionalandterm;	
									}
                                    if (mIsFloor)
									{
										double accruedfloor = (*mpFloor)(mX3);
										double accruedcoupon = coupon * notionalandterm;
										coupon = accruedcoupon > accruedfloor ? coupon : accruedfloor/notionalandterm;
									}
								}
	// get past coupons
	void						getPastCoupons(DoubleArray& x) const;
	void						getPastCouponPayoffs(DoubleArray& x) const;

	virtual LAPriceIndexToolBase* createIndexTool(const AQLObject& indexInfo, const AQLDate& baseDate);
						
	
	std::vector<LAPriceIndexToolBase*> mpIndexs;// index
	AQLFunctionBase* mpOperator;// operator 
	
    const LAPricePayOff* mpPayOff;// payoff
	UintArray mPayOffPos;// past payoff positions that are used to calutate this coupon
	const AQLFunctionBase* mpObservationOperator;// observation operator
	unsigned int mCurrentPos;//! current payoff position
	DoubleArray	mFixedRates;// fixed past coupons that are used to calutate this coupon
	bool mIsObservationTerm;// flag of coupon observation term or not

	//cap floor
	bool mIsCap;// cap flag
	bool mIsFloor;// < floor flag
	const AQLFunctionBase*	mpCap;// cap
	const AQLFunctionBase*	mpFloor;// floor
	

	unsigned int mLegNo;// leg number that are corresponed to this coupon	

	//round
	bool mIsRound;// round flag
	RoundFunction	mRoundFunction;// round function
	int	mRoundDigit;// round digit
		
	mutable double mCoupon;// varibable for storing a calculated coupon 
	mutable DoubleArray	mX;// variable for coupon calculation
	mutable DoubleArray	mX2;// variable for coupon calculation
	mutable DoubleArray	mX3;// variable for coupon calculation

	bool mIsCompoundCoupon;
	const AQLDataDoubles* mpCoefficient;
};

class LAPriceCouponToolCompound : public LAPriceCouponTool
{
public:
	virtual	void setUp(const AQLDate& basedate,	
                       const AQLObject& trade,
                       unsigned int legNo,
                       const AQLObject& couponinfo,
                       const LAPricePayOff& payoff,
                       unsigned int currentpos);
    virtual void calcCoupons(DoubleVector& coupons, const size_t start_pos, const size_t end_pos);
    virtual void setFixingInfo(AQLDate& fixing_date, AQLString& fixing_flag) const;
    virtual void setFixingInfo(DateVector& fixing_date, AQLStringVector& fixing_flag) const;
protected:
    virtual LAPriceIndexToolBase* createIndexTool(const AQLObject& indexInfo, const AQLDate& baseDate);
};
