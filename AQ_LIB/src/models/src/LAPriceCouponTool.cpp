/*! @file
    @brief source code of tool class of coupon calculation.

*/
//  2006, AlgoQuantHub..
///
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceCouponTool.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAObject.h"
#include "LAObjectHolder.h"


#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataFunction.h"

#include "LAAlgorithm.h"


#include "LAMathDefine.h"
#include "LAPriceCFGenUtility.h"
#include "LACompoundingFunc.h"

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
LAPriceCouponTool::LAPriceCouponTool()
: mpOperator(NULL), mpPayOff(NULL), mpObservationOperator(NULL)
, mCurrentPos(0), mIsCap(false), mIsFloor(false), mpCap(NULL), mpFloor(NULL)
, mLegNo(0), mIsRound(false), mRoundDigit(0), mCoupon(0),mIsCompoundCoupon(false), mpCoefficient(0)
{

}
/*!
    @brief destructor

*/
LAPriceCouponTool::~LAPriceCouponTool()
{
	if (mpOperator) delete mpOperator;
	for (unsigned int i = 0; i < mpIndexs.size(); i++)
		if (mpIndexs[i] != NULL) delete mpIndexs[i];

}

/*!
    @brief copy constructor
	@param[in] v LAPriceCouponTool object
*/
LAPriceCouponTool::LAPriceCouponTool(const LAPriceCouponTool& v)
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
		mpOperator = dynamic_cast<LAFunctionBase*>(v.mpOperator->clone());

}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number = 0)
	@param[in] couponinfo coupon information object
	@param[in] payoff LAPricePayOff object
	@param[in] current position of payoff that this coupon correspond to (first payoff position = 0)
*/
void
LAPriceCouponTool::setUp(const LADate& basedate,	
                      const LAObject& trade,
					  unsigned int legNo,
					  const LAObject& couponinfo,
					  const LAPricePayOff& payoff,
					  unsigned int currentpos)
{
	const LADataHolder* dh;
	
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
		LAString roundfunction = dynamic_cast<const LADataString&>(dh->get()).get();
		roundfunction.toUpper();
		if (roundfunction == ROUND_STR) mRoundFunction = ROUND;
		else if (roundfunction == ROUND_UP_STR) mRoundFunction = ROUND_UP;
		else mRoundFunction = ROUND_DOWN;

		dh = &(couponinfo.getData(PRICING_DATA_ROUNDDIGIT, ISNOTNULL));
		mRoundDigit = dynamic_cast<const LADataInt&>(dh->get()).get();
	}


	//cap floor
	dh = &(couponinfo.getData(PRICING_DATA_MAXCOUPON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsCap = true;
		const LAPriceDataFunction& capfunc= dynamic_cast<const LAPriceDataFunction&>(dh->get());
	    mpCap = &dynamic_cast<const LAFunctionBase&>(capfunc.getFunction());
	}
	dh = &(couponinfo.getData(PRICING_DATA_MINCOUPON, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsFloor = true;
		const LAPriceDataFunction& floorfunc= dynamic_cast<const LAPriceDataFunction&>(dh->get());
	    mpFloor = &dynamic_cast<const LAFunctionBase&>(floorfunc.getFunction());
	}



	//index info
	dh = &(couponinfo.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
	const LADataMultiReference& indexinfos
			= dynamic_cast<const LADataMultiReference&>(dh->get());
	mpIndexs.resize(indexinfos.getSize(), NULL);
	const LADate& paydate = (mpPayOff->getPayOff())[mLegNo].at(currentpos).getPayOff().getPaymentDate();
	for (unsigned int i = 0; i < indexinfos.getSize(); i++)
	{	
		mpIndexs[i] = createIndexTool(indexinfos.get(i).get(), basedate);
		mpIndexs[i]->setUp(basedate, paydate, trade, indexinfos.get(i).get(), payoff);
	}

	//operator
	dh = &(couponinfo.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
	const LAFunctionBase& method = dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
    mpOperator = dynamic_cast<LAFunctionBase*>(method.clone());
	
	//coefficient
	mpCoefficient = &dynamic_cast<const LADataDoubles &>(couponinfo.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get());
	mpOperator->setParam(mpCoefficient->get());
	//dh = &(couponinfo.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL));
	//const DoubleArray& coeff = dynamic_cast<const LADataDoubles&>(dh->get()).get();

	//mpOperator->setParam(coeff);

	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());


	//ObservationStartDate
	dh = &(couponinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get()).get();	
		dh = &(couponinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL));	 
		const LADate& enddate = dynamic_cast<const LADataDate&>(dh->get()).get();	
	
		for (unsigned int i = 0; i < currentpos; i++)
		{
			const LADate& paymentdate = (mpPayOff->getPayOff())[mLegNo][i].getPayOff().getPaymentDate();
			if (paymentdate >=startdate && paymentdate <= enddate && 
				(mpPayOff->getPayOff())[mLegNo][i].getPayOff().isCouponPayment())
				mPayOffPos.push_back(i);
		}
		dh = &(couponinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
	
		mIsObservationTerm = true;


		// past coupon
		//get past paymentdate
		DateVector needdates;
		dh = &(legs.get(legNo).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
		const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < cashlets.getSize(); i++)
		{
			dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
			dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			const LADate& paymentdate = dynamic_cast<const LADataDate&>(dh->get()).get();
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
				isSavePastFixing = dynamic_cast<const LADataBool &>(dh->get()).get();
			}
			if (isSavePastFixing)
			{
				DateVector paymentdates;
				DoubleArray coupons;
				dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
					dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
					coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				}
				for (unsigned int i = 0; i < needdates.size(); i++)
				{
					unsigned int pos;
					if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
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
				const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
				const DoubleArray& coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				for (unsigned int i = 0; i < needdates.size(); i++)
				{
					unsigned int pos;
					if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
					{
						//error
						LAString msg = "Past Coupon of ";
						msg += LADataDate(needdates[i]).convertToString();
						msg += " is needed.";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
			const DoubleArray& couponpayoffs = dynamic_cast<const LADataDoubles&>(dh->get()).get();		
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
LAPriceCouponTool::getPastCoupons(DoubleArray& x) const
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
LAPriceCouponTool::getPastCouponPayoffs(DoubleArray& x) const
{
	int j = x.size() - mCurrentPos - 1;
	int i_e = (mpPayOff->getPayOff())[mLegNo].size();
	for (unsigned int i = 0; i < mCurrentPos && i < i_e; i++)
	{
		if((mpPayOff->getPayOff())[mLegNo][i].getPayOff().isCouponPayment())
		{	
			const LAPricePayOffTool *mp = &(mpPayOff->getPayOff())[mLegNo][i].getPayOff();		
			x[j++] = mp->getCoupon()* mp->mTerm * mp->mNotional;
		}
	}
}

LAPriceIndexToolBase* LAPriceCouponTool::createIndexTool(const LAObject& indexInfo, const LADate& baseDate)
{
	std::unique_ptr<LAPriceIndexToolBase> ret;

	const LADataHolder* dh;

	LAString indexType = dynamic_cast<const LADataString&>(indexInfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
	indexType.toUpper();

	if(indexType == FIXED_RATE) ret.reset(new LAPriceIndexToolFixed());
	else if(indexType == CPN)   ret.reset(new LAPriceIndexToolCpn());
	else if(indexType == CPNCF) ret.reset(new LAPriceIndexToolCpnCF());
	else{
		dh = &indexInfo.getData(PRICING_DATA_OBSERVATIONOPERATOR);
		if (dh->isDefined() && !dh->isNull()) ret.reset(new LAPriceIndexTool());
		else{
			dh = &indexInfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL);
			const LADate& fixingdate = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (baseDate >= fixingdate){
				dh = &indexInfo.getData(PRICING_DATA_FIXEDRATE);
				if (dh->isDefined() && !dh->isNull()) ret.reset(new LAPriceIndexToolFixed());
				else                                  ret.reset(new LAPriceIndexTool());
			}
			else{
				ret.reset(new LAPriceIndexTool());
			}
		}
	}

	return ret.release();
}






void LAPriceCouponToolCompound::calcCoupons(DoubleVector& coupons, const size_t start_pos, const size_t end_pos)
{
    LAPriceIndexToolCompound* index = dynamic_cast<LAPriceIndexToolCompound*>(mpIndexs[0]);
    if(index==NULL){
        LAString msg;
        msg += "CAST ERROR:";
        msg += "index tool cannot be casted to LAPriceIndexToolCompound";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    index->calcIndices(coupons, start_pos, end_pos);
    for(size_t i = start_pos; i < end_pos; i++){
        round(coupons[i]);
        capfloor(coupons[i]);
    }
}

LAPriceIndexToolBase* LAPriceCouponToolCompound::createIndexTool(const LAObject& indexInfo, const LADate& baseDate)
{
    const LADataHolder* dh;
    if((dh=&indexInfo.getData(PRICING_DATA_ISCOMPOUNDINGCOUPON))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get()){
        return new LAPriceIndexToolCompound();
    }
    else{
        return LAPriceCouponTool::createIndexTool(indexInfo, baseDate);
    }
}

void
LAPriceCouponToolCompound::setUp(const LADate& basedate,	
                              const LAObject& trade,
                              unsigned int legNo,
                              const LAObject& couponinfo,
                              const LAPricePayOff& payoff,
                              unsigned int currentpos)
{
    LAPriceCouponTool::setUp(basedate, 
                          trade,
                          legNo,
                          couponinfo,
                          payoff,
                          currentpos);
    mIsCompoundCoupon = true;
}

void 
LAPriceCouponToolCompound::setFixingInfo(LADate& fixing_date, LAString& fixing_flag) const
{
    const LAPriceIndexToolCompound* index = dynamic_cast<const LAPriceIndexToolCompound*>(mpIndexs[0]);
    if(index==NULL){
        LAString msg;
        msg += "CAST ERROR:";
        msg += "index tool cannot be casted to LAPriceIndexToolCompound";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    index->setFixingInfo(fixing_date, fixing_flag);
    return;
}

void 
LAPriceCouponToolCompound::setFixingInfo(DateVector& fixing_date, LAStringVector& fixing_flag) const
{
    const LAPriceIndexToolCompound* index = dynamic_cast<const LAPriceIndexToolCompound*>(mpIndexs[0]);
    if(index==NULL){
        LAString msg;
        msg += "CAST ERROR:";
        msg += "index tool cannot be casted to LAPriceIndexToolCompound";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    index->setFixingInfo(fixing_date, fixing_flag);
    return;
}
