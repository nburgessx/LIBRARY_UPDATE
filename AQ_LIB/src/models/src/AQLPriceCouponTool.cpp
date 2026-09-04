/*! @file
    @brief source code of tool class of coupon calculation.
*/
///
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPriceCouponTool.h"

#include "AQLDataHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"


#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataFunction.h"

#include "AQLAlgorithm.h"


#include "AQLMathDefine.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLCompoundingFunc.h"

#include <algorithm>

using namespace std;

#define ROUND_STR		"ROUND"
#define ROUND_UP_STR	"ROUNDUP"
#define ROUND_DOWN_STR	"ROUNDDOWN"

#define LIBOR	"LIBOR"
#define CMS		"CMS"
#define FIXED_RATE	"FIXEDRATE"
#define FX_RATE	"FXRATE"
#define CMT	"CMT"
#define CUSTOM	"CUSTOM"
#define CPN		"CPN"
#define CPNCF	"CPNCF"
#define OIS	"OIS"


/*!
    @brief default constructor
*/
AQLPriceCouponTool::AQLPriceCouponTool()
: mpOperator(NULL), mpPayOff(NULL), mpObservationOperator(NULL)
, mCurrentPos(0), mIsCap(false), mIsFloor(false), mpCap(NULL), mpFloor(NULL)
, mLegNo(0), mIsRound(false), mRoundDigit(0), mCoupon(0),mIsCompoundCoupon(false), mpCoefficient(0)
{

}
/*!
    @brief destructor

*/
AQLPriceCouponTool::~AQLPriceCouponTool()
{
	if (mpOperator) delete mpOperator;
	for (unsigned int i = 0; i < mpIndexs.size(); i++)
		if (mpIndexs[i] != NULL) delete mpIndexs[i];

}

/*!
    @brief copy constructor
	@param[in] v AQLPriceCouponTool object
*/
AQLPriceCouponTool::AQLPriceCouponTool(const AQLPriceCouponTool& v)
: mpOperator(NULL), mpPayOff(v.mpPayOff), mPayOffPos(v.mPayOffPos)
, mpObservationOperator(v.mpObservationOperator), mCurrentPos(v.mCurrentPos)
, mFixedRates(v.mFixedRates), mIsObservationTerm(v.mIsObservationTerm)
, mIsCap(v.mIsCap), mIsFloor(v.mIsFloor), mpCap(v.mpCap), mpFloor(v.mpFloor)
, mLegNo(v.mLegNo), mIsRound(v.mIsRound), mRoundFunction(v.mRoundFunction)
, mRoundDigit(v.mRoundDigit), mCoupon(v.mCoupon), mX(v.mX), mX2(v.mX2), mX3(v.mX3),mIsCompoundCoupon(v.mIsCompoundCoupon), mpCoefficient(v.mpCoefficient)
{
	mpIndexs.resize(v.mpIndexs.size(), NULL);
	for (unsigned int i = 0; i < mpIndexs.size(); i++)
	{
		if (v.mpIndexs[i] == NULL) continue;
		mpIndexs[i] = v.mpIndexs[i]->clone();
	}
	if (v.mpOperator != NULL) 
		mpOperator = dynamic_cast<AQLFunctionBase*>(v.mpOperator->clone());

}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number = 0)
	@param[in] couponinfo coupon information object
	@param[in] payoff AQLPricePayOff object
	@param[in] current position of payoff that this coupon correspond to (first payoff position = 0)
*/
void
AQLPriceCouponTool::setUp(const AQLDate& basedate,	
                      const AQLObject& trade,
					  unsigned int legNo,
					  const AQLObject& couponinfo,
					  const AQLPricePayOff& payoff,
					  unsigned int currentpos)
{
	const AQLDataHolder* dh;
	
	//initialize
	mIsRound = false;
	mIsCap = false;
	mIsFloor = false;
	mX.clear();
	mX2.clear();
	mX3.clear();

	if (mpOperator != NULL) delete mpOperator;
	mpOperator = NULL;
	for (unsigned int i = 0; i < mpIndexs.size(); i++)
		if (mpIndexs[i] != NULL) delete mpIndexs[i];
	mpIndexs.clear();

	mPayOffPos.clear();
	mpObservationOperator = NULL;
	mIsObservationTerm = false;	

	mFixedRates.clear();

	mLegNo = legNo;
	mpPayOff = &payoff;
	mCurrentPos = currentpos;

	//round function
	dh = &(couponinfo.getData(PRICING_DATA_ROUNDFUNCTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsRound = true;
		AQLString roundfunction = dynamic_cast<const AQLDataString&>(dh->get()).get();
		roundfunction.toUpper();
		if (roundfunction == ROUND_STR) mRoundFunction = ROUND;
		else if (roundfunction == ROUND_UP_STR) mRoundFunction = ROUND_UP;
		else mRoundFunction = ROUND_DOWN;

		dh = &(couponinfo.getData(PRICING_DATA_ROUNDDIGIT, ISNOTNULL));
		mRoundDigit = dynamic_cast<const AQLDataInt&>(dh->get()).get();
	}


	//cap floor
	dh = &(couponinfo.getData(PRICING_DATA_MAXCOUPON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsCap = true;
		const AQLPriceDataFunction& capfunc= dynamic_cast<const AQLPriceDataFunction&>(dh->get());
	    mpCap = &dynamic_cast<const AQLFunctionBase&>(capfunc.getFunction());
	}
	dh = &(couponinfo.getData(PRICING_DATA_MINCOUPON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsFloor = true;
		const AQLPriceDataFunction& floorfunc= dynamic_cast<const AQLPriceDataFunction&>(dh->get());
	    mpFloor = &dynamic_cast<const AQLFunctionBase&>(floorfunc.getFunction());
	}



	//index info
	dh = &(couponinfo.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
	const AQLDataMultiReference& indexinfos
			= dynamic_cast<const AQLDataMultiReference&>(dh->get());
	mpIndexs.resize(indexinfos.getSize(), NULL);
	const AQLDate& paydate = (mpPayOff->getPayOff())[mLegNo].at(currentpos).getPayOff().getPaymentDate();
	for (unsigned int i = 0; i < indexinfos.getSize(); i++)
	{	
		mpIndexs[i] = createIndexTool(indexinfos.get(i).get(), basedate);
		mpIndexs[i]->setUp(basedate, paydate, trade, indexinfos.get(i).get(), payoff);
	}

	//operator
	dh = &(couponinfo.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
	const AQLFunctionBase& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();
    mpOperator = dynamic_cast<AQLFunctionBase*>(method.clone());
	
	//coefficient
	mpCoefficient = &dynamic_cast<const AQLDataDoubles &>(couponinfo.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get());
	mpOperator->setParam(mpCoefficient->get());
	//dh = &(couponinfo.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
	//const DoubleArray& coeff = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();

	//mpOperator->setParam(coeff);

	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());


	//ObservationStartDate
	dh = &(couponinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLDate& startdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();	
		dh = &(couponinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL));	 
		const AQLDate& enddate = dynamic_cast<const AQLDataDate&>(dh->get()).get();	
	
		for (unsigned int i = 0; i < currentpos; i++)
		{
			const AQLDate& paymentdate = (mpPayOff->getPayOff())[mLegNo][i].getPayOff().getPaymentDate();
			if (paymentdate >=startdate && paymentdate <= enddate && 
				(mpPayOff->getPayOff())[mLegNo][i].getPayOff().isCouponPayment())
				mPayOffPos.push_back(i);
		}
		dh = &(couponinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();
	
		mIsObservationTerm = true;


		// past coupon
		//get past paymentdate
		DateVector needdates;
		dh = &(legs.get(legNo).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		const AQLDataMultiReference& cashlets = dynamic_cast<const AQLDataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
			dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			const AQLDate& paymentdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
			if (paymentdate < startdate) continue;
			if (paymentdate > basedate || paymentdate > enddate) break;
			needdates.push_back(paymentdate);
		}		
		if (needdates.size() > 0)
		{
			// is save past fixing
			bool isSavePastFixing = false;
			dh = &(trade.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				isSavePastFixing = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			}
			if (isSavePastFixing)
			{
				DateVector paymentdates;
				DoubleArray coupons;
				dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
					dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
					coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				}
				for (unsigned int i = 0; i < needdates.size(); i++)
				{
					unsigned int pos;
					if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
					{
						mFixedRates.push_back(0.0);
					}
					else
					{
						mFixedRates.push_back(coupons.at(pos));
					}
				}
			}
			else
			{
				// past coupon
				dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
				const DateVector& paymentdates = dynamic_cast<const AQLDataDates&>(dh->get()).get();
				dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
				const DoubleArray& coupons = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();
				for (unsigned int i = 0; i < needdates.size(); i++)
				{
					unsigned int pos;
					if (!AQLAlgorithm::find<DateVector, AQLDate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
					{
						//error
						AQLString msg = "Past Coupon of ";
						msg += AQLDataDate(needdates[i]).convertToString();
						msg += " is needed.";
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					mFixedRates.push_back(coupons.at(pos));
				}					
			}
		}
	}

	mX.resize(indexinfos.getSize());
	mX2.resize(mFixedRates.size() + mPayOffPos.size() + 1);
	if ((mIsCap && !mpCap->isTypeOf(FN_CONSTANT))
		|| (mIsFloor && !mpFloor->isTypeOf(FN_CONSTANT)))
	{
		dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONPAYOFFS, NOCHECK)); 
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& couponpayoffs = dynamic_cast<const AQLDataDoubles&>(dh->get()).get();		
			mX3 = couponpayoffs;
			mX3.resize(couponpayoffs.size() + mCurrentPos + 1);
		}
		else mX3.resize(mCurrentPos + 1);
	}

}

/*!
	@brief get past coupons
	@param[out] x past coupons
*/
void
AQLPriceCouponTool::getPastCoupons(DoubleArray& x) const
{
	int j = x.size() - mCurrentPos - 1;
	for (unsigned int i = 0; i < mCurrentPos; i++)
	{
		if((mpPayOff->getPayOff())[mLegNo][i].getPayOff().isCouponPayment())
			x[j++] = (mpPayOff->getPayOff())[mLegNo][i].getPayOff().getCoupon();
	}

}

/*!
	@brief get accruedpast couponpayoffs
	@param[out] x accruedpast couponpayoffs
*/
void
AQLPriceCouponTool::getPastCouponPayoffs(DoubleArray& x) const
{
	int j = x.size() - mCurrentPos - 1;
	int i_e = (mpPayOff->getPayOff())[mLegNo].size();
	for (unsigned int i = 0; i < mCurrentPos && i < i_e; i++)
	{
		if((mpPayOff->getPayOff())[mLegNo][i].getPayOff().isCouponPayment())
		{	
			const AQLPricePayOffTool *mp = &(mpPayOff->getPayOff())[mLegNo][i].getPayOff();		
			x[j++] = mp->getCoupon()* mp->mTerm * mp->mNotional;
		}
	}
}

AQLPriceIndexToolBase* AQLPriceCouponTool::createIndexTool(const AQLObject& indexInfo, const AQLDate& baseDate)
{
	std::unique_ptr<AQLPriceIndexToolBase> ret;

	const AQLDataHolder* dh;

	AQLString indexType = dynamic_cast<const AQLDataString&>(indexInfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
	indexType.toUpper();

	if(indexType == FIXED_RATE) ret.reset(new AQLPriceIndexToolFixed());
	else if(indexType == CPN)   ret.reset(new AQLPriceIndexToolCpn());
	else if(indexType == CPNCF) ret.reset(new AQLPriceIndexToolCpnCF());
	else{
		dh = &indexInfo.getData(PRICING_DATA_OBSERVATIONOPERATOR);
		if (dh->isDefined() && !dh->isNull()) ret.reset(new AQLPriceIndexTool());
		else{
			dh = &indexInfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL);
			const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
			if (baseDate >= fixingdate){
				dh = &indexInfo.getData(PRICING_DATA_FIXEDRATE);
				if (dh->isDefined() && !dh->isNull()) ret.reset(new AQLPriceIndexToolFixed());
				else                                  ret.reset(new AQLPriceIndexTool());
			}
			else{
				ret.reset(new AQLPriceIndexTool());
			}
		}
	}

	return ret.release();
}






void AQLPriceCouponToolCompound::calcCoupons(DoubleVector& coupons, const size_t start_pos, const size_t end_pos)
{
    AQLPriceIndexToolCompound* index = dynamic_cast<AQLPriceIndexToolCompound*>(mpIndexs[0]);
    if(index==NULL){
        AQLString msg;
        msg += "CAST ERROR:";
        msg += "index tool cannot be casted to AQLPriceIndexToolCompound";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    index->calcIndices(coupons, start_pos, end_pos);
    for(size_t i = start_pos; i < end_pos; i++){
        round(coupons[i]);
        capfloor(coupons[i]);
    }
}

AQLPriceIndexToolBase* AQLPriceCouponToolCompound::createIndexTool(const AQLObject& indexInfo, const AQLDate& baseDate)
{
    const AQLDataHolder* dh;
    if((dh=&indexInfo.getData(PRICING_DATA_ISCOMPOUNDINGCOUPON))->isDefined() && !dh->isNull() && dynamic_cast<const AQLDataBool&>(dh->get()).get()){
        return new AQLPriceIndexToolCompound();
    }
    else{
        return AQLPriceCouponTool::createIndexTool(indexInfo, baseDate);
    }
}

void
AQLPriceCouponToolCompound::setUp(const AQLDate& basedate,	
                              const AQLObject& trade,
                              unsigned int legNo,
                              const AQLObject& couponinfo,
                              const AQLPricePayOff& payoff,
                              unsigned int currentpos)
{
    AQLPriceCouponTool::setUp(basedate, 
                          trade,
                          legNo,
                          couponinfo,
                          payoff,
                          currentpos);
    mIsCompoundCoupon = true;
}

void 
AQLPriceCouponToolCompound::setFixingInfo(AQLDate& fixing_date, AQLString& fixing_flag) const
{
    const AQLPriceIndexToolCompound* index = dynamic_cast<const AQLPriceIndexToolCompound*>(mpIndexs[0]);
    if(index==NULL){
        AQLString msg;
        msg += "CAST ERROR:";
        msg += "index tool cannot be casted to AQLPriceIndexToolCompound";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    index->setFixingInfo(fixing_date, fixing_flag);
    return;
}

void 
AQLPriceCouponToolCompound::setFixingInfo(DateVector& fixing_date, AQLStringVector& fixing_flag) const
{
    const AQLPriceIndexToolCompound* index = dynamic_cast<const AQLPriceIndexToolCompound*>(mpIndexs[0]);
    if(index==NULL){
        AQLString msg;
        msg += "CAST ERROR:";
        msg += "index tool cannot be casted to AQLPriceIndexToolCompound";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    index->setFixingInfo(fixing_date, fixing_flag);
    return;
}
