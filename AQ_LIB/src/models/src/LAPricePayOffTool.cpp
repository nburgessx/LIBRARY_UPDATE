/*! @file
    @brief source code of tool class of payoff calculation.

*/
//  2006, AlgoQuantHub..
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPricePayOffTool.h"

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
#include "LAMathIndexEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"
#include "LAMathDefine.h"

#include "LAAlgorithm.h"
#include "LABasic.h"

#include "LAPricePayOff.h"
#include "LAPriceCouponTool.h"
#include "LAPriceCFGenUtility.h"
#include "LAPriceCouponBase.h"
#include "LAPriceCouponForDigital2.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAInterpolationBase.h"
#include "LAMathYieldCurvePro.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAMathDateCalculations.h"
#include "LAMathDateUtilities.h"
#include "LACoreUtility.h"
#include <algorithm>

using namespace std;

#define ROUND_STR		"ROUND"
#define ROUND_UP_STR	"ROUNDUP"
#define ROUND_DOWN_STR	"ROUNDDOWN"


/*!
    @brief default constructor
*/
LAPricePayOffTool::LAPricePayOffTool()
: mPaymentTime(0), mTerm(0), mNotional(0), mDenomiRatio(1), mIsRound(false), mRoundDigit(0), mExtraCF(0),mExtraCFFdChg(0)
, mpCpnSelectOperator(0)
, mNotionalCF(0), mAccruedInt(0)
, mpFX_for_PayOff(0), mpFX_for_Notional(0), mpFX_for_ExtraCF(0), mpRefFX_for_PayOff(0), mpRefFX_for_Notional(0), mpRefFX_for_ExtraCF(0), mNotionalCFExchangeRate(1)
, mBaseCur(""), mNotionalCur(""), mExtraCFCur(""), mNotionalCFCur("") 
, mIsArrear(true), mCoupon(0), mCpnPayOffbyPayOffCur(0), mCouponPayOff(0),mPayOff(0)
, mSelectedCpnPos(0)
, mRefCount(0), mIsTermCalc(true), mIsRedemption(false), mpNotionalCFCpnSelectOperator(0), mExtraCFFixingTime(-1), flagForUpFrontFee(true), mSettlementAdjustRatio(1.0)
{

}
/*!
    @brief destructor
*/
LAPricePayOffTool::~LAPricePayOffTool()
{
	for (unsigned int i = 0; i < mpCoupons.size(); i++)
		if (mpCoupons[i] != NULL) delete mpCoupons[i];
	for (unsigned int i = 0; i < mpNotionalCFCoupons.size(); i++)
		if (mpNotionalCFCoupons[i] != NULL) delete mpNotionalCFCoupons[i];
	if (mpCpnSelectOperator != NULL) delete mpCpnSelectOperator;
	if (mpNotionalCFCpnSelectOperator != NULL) delete mpNotionalCFCpnSelectOperator;
}

/*!
    @brief copy constructor
	@param[in] v LAPricePayOffTool object
*/
LAPricePayOffTool::LAPricePayOffTool(const LAPricePayOffTool& v)
: 
mPaymentDate(v.mPaymentDate), mPaymentTime(v.mPaymentTime), mTerm(v.mTerm),
mStart(v.mStart), mEnd(v.mEnd), mDC(v.mDC),
mNotional(v.mNotional), mDenomiRatio(v.mDenomiRatio), mIsRound(v.mIsRound), mRoundFunction(v.mRoundFunction), mRoundDigit(v.mRoundDigit),
mExtraCF(v.mExtraCF),mExtraCFFdChg(v.mExtraCFFdChg),mpCpnSelectOperator(NULL),
mNotionalCF(v.mNotionalCF),mRenotionalFixingDate(v.mRenotionalFixingDate), mAccruedInt(v.mAccruedInt),
mpFX_for_PayOff(v.mpFX_for_PayOff), mpFX_for_Notional(v.mpFX_for_Notional), 
mpFX_for_ExtraCF(v.mpFX_for_ExtraCF), /*mpFX_for_Coupons(v.mpFX_for_Coupons),*/
mpRefFX_for_PayOff(v.mpRefFX_for_PayOff), mpRefFX_for_Notional(v.mpRefFX_for_Notional), mpRefFX_for_ExtraCF(v.mpRefFX_for_ExtraCF),
mNotionalCFExchangeRate(v.mNotionalCFExchangeRate),
mBaseCur(v.mBaseCur), mNotionalCur(v.mNotionalCur), mNotionalCFCur(v.mNotionalCFCur), mExtraCFCur(v.mExtraCFCur), 
mCouponsCur(v.mCouponsCur), mIsArrear(v.mIsArrear),
mCoupon(v.mCoupon), mCpnPayOffbyPayOffCur(v.mCpnPayOffbyPayOffCur), 
mCouponPayOff(v.mCouponPayOff), mPayOff(v.mPayOff), 
mSelectedCpnPos(v.mSelectedCpnPos),
mRefCount(0), mIsTermCalc(v.mIsTermCalc), mIsRedemption(v.mIsRedemption), mpNotionalCFCpnSelectOperator(NULL), mExtraCFFixingTime(v.mExtraCFFixingTime), mSettlementAdjustRatio(v.mSettlementAdjustRatio)
{
	mpNotionalCFCoupons.resize(v.mpNotionalCFCoupons.size(), NULL);
	for (unsigned int i = 0; i < mpNotionalCFCoupons.size(); i++)
	{
		if(v.mpNotionalCFCoupons[i] == NULL) continue;
		try
		{
			mpNotionalCFCoupons[i] = new LAPriceCouponTool(*v.mpNotionalCFCoupons[i]);
		}
		catch(bad_alloc& e)
		{
			throw LACoreSystemError(e.what(), __FILE__, __LINE__);
		}
	}

	if(v.mpNotionalCFCpnSelectOperator != NULL)
	{
		try
		{
			mpNotionalCFCpnSelectOperator = dynamic_cast<LAPriceCouponBase*>(v.mpNotionalCFCpnSelectOperator->clone());
		}
		catch(bad_alloc& e)
		{
			throw LACoreSystemError(e.what(), __FILE__, __LINE__);
		}
	}
	mpCoupons.resize(v.mpCoupons.size(), NULL);
	for (unsigned int i = 0; i < mpCoupons.size(); i++)
	{
		if (v.mpCoupons[i] == NULL) continue;
		try 
		{
			mpCoupons[i] = new LAPriceCouponTool(*v.mpCoupons[i]);
		}
		catch (bad_alloc& e)
		{
			throw LACoreSystemError(e.what(), __FILE__, __LINE__);
		}			
	}
	if (v.mpCpnSelectOperator != NULL) 
		mpCpnSelectOperator = dynamic_cast<LAPriceCouponBase*>(v.mpCpnSelectOperator->clone());
}


/*!
    @brief calculate payoff
*/
double
LAPricePayOffTool::calcPayOff(void) const
{
	// get fx object from reference pointer (for risk)
	if (mpRefFX_for_PayOff)
	{
		mpFX_for_PayOff = &dynamic_cast<const LAMathFXEntity&>(mpRefFX_for_PayOff->get().get());
	}
	if (mpFX_for_Notional)
	{
		mpFX_for_Notional = &dynamic_cast<const LAMathFXEntity&>(mpRefFX_for_Notional->get().get());
	}
	if (mpRefFX_for_ExtraCF)
	{
		mpFX_for_ExtraCF = &dynamic_cast<const LAMathFXEntity&>(mpRefFX_for_ExtraCF->get().get());
	}
	double notionalcf = mNotionalCF;
	if (mpNotionalCFCoupons.size() >= 1)
	{
		double notionalcfcoupon = 0.0;
		unsigned int pos = 0;
		if (mpNotionalCFCpnSelectOperator != NULL)
		{
			DoubleArray x(mpNotionalCFCoupons.size());
			for (unsigned int i = 0; i < x.size(); i++)
				x[i] = mpNotionalCFCoupons[i]->calcCoupon();
			notionalcfcoupon = mpNotionalCFCpnSelectOperator->selectCoupon(x, pos);
		}
		else
		{
			notionalcfcoupon = mpNotionalCFCoupons[0]->calcCoupon();
		}
		if (mNotionalCFCur != mNotionalCFCouponsCur[pos])
		{
			notionalcfcoupon *= mpFX_for_PayOff->getRate(mNotionalCFCouponsCur[pos], mNotionalCFCur, mPaymentTime);
		}
		notionalcf += notionalcfcoupon * mNotional;
	}

	if (notionalcf !=0 && mNotionalCFCur != "" && mNotionalCur != mNotionalCFCur)
	{
		notionalcf *= mNotionalCFExchangeRate;
		if (mBaseCur != mNotionalCFCur)
			notionalcf *= mpFX_for_PayOff->getRate(mNotionalCFCur, mBaseCur, mPaymentTime);
	}
	else if (notionalcf != 0 && mBaseCur != mNotionalCur)
		notionalcf *= mpFX_for_PayOff->getRate(mNotionalCur, mBaseCur, mPaymentTime);

	double extracf = mExtraCF;
	if (mExtraCFFixingTime != -1)
	{	
		if (extracf != 0 && mBaseCur != mExtraCFCur)
			extracf *= mpFX_for_ExtraCF->getRate(mExtraCFCur, mBaseCur, mExtraCFFixingTime);
	}
	else
	{
		if (extracf != 0 && mBaseCur != mExtraCFCur)
			extracf *= mpFX_for_ExtraCF->getRate(mExtraCFCur, mBaseCur, mPaymentTime);
	}

	extracf += mExtraCFFdChg;
	
	double accruedint = mAccruedInt;
	if (accruedint != 0 && mBaseCur != mCouponsCur[mSelectedCpnPos])
		accruedint *= mpFX_for_PayOff->getRate(mCouponsCur[mSelectedCpnPos], mBaseCur, mPaymentTime);


	if (!isCouponPayment())//no coupon
	{
		mCoupon = 0;
		mCpnPayOffbyPayOffCur = mAccruedInt;
		mPayOff = notionalcf + extracf + accruedint;
		mPayOff *= mSettlementAdjustRatio;
		return mPayOff;
	}


    
    double cpncf = calculateCouponCashflow();
    if (mNotionalCur != mCouponsCur[mSelectedCpnPos] && !mIsRedemption){
        if(mpCouponCFFXRateValue.get() != NULL){
            cpncf *= *mpCouponCFFXRateValue;
        }
        else{
            cpncf *= mpFX_for_Notional->getRate(mNotionalCur, mCouponsCur[mSelectedCpnPos], mPaymentTime);
        }
    }



	cpncf /= mDenomiRatio;
	round(cpncf);
	cpncf *= mDenomiRatio;

	mCpnPayOffbyPayOffCur = cpncf;

	if (mBaseCur != mCouponsCur[mSelectedCpnPos])
		cpncf *= mpFX_for_PayOff->getRate(mCouponsCur[mSelectedCpnPos], mBaseCur, mPaymentTime);
	
	mCouponPayOff = cpncf;

	mPayOff = cpncf + extracf + notionalcf;
	mPayOff *= mSettlementAdjustRatio;
	return  mPayOff;
}

/*!
    @brief calculate payoff
*/
void
LAPricePayOffTool::calcDerivationOfLibor(std::map<LAString, std::map<double, double> >& delivationLiborMap, const LAInterpolationBase* pNumeInterp, bool isRec) const
{
	double sgn = (isRec) ? 1.0 : -1.0;
	double notionalcf = mNotional;
	if (!isCouponPayment())//no coupon
		return ;

	if (mpCpnSelectOperator != NULL)
	{
		throw LACoreInvalidData("Not supported in the coupon select case", __FILE__,__LINE__);
	}
	
	std::vector<LAPriceIndexToolBase*> indexs = mpCoupons[0]->getIndexs();
	unsigned int indexSize = indexs.size();
	//correspond into CMSType
	for (unsigned int i = 0; i < indexSize; i++)
	{
		if (!indexs[i]->isTypeOf(INDEX_TOOL))
			continue;

		LAPriceIndexTool* pindex = dynamic_cast<LAPriceIndexTool*>(indexs[i]);
		const  UintArray& indexpos = pindex->getIndexPos();
		const  DoubleArray& fixedrates = pindex->getFixedRates();
		const LAMathIndexEntity* pentity = pindex->getIndexEntity();
		if (indexpos.size() != 1 || fixedrates.size() != 0)
			continue;

		unsigned int pos = indexpos[0];
		double cmsrate = pentity->getIndex()[pos];
		//in libor case pick coefficient
		const DoubleArray& coefvec = mpCoupons[0]->getOperator()->getParam();
		if (coefvec.size() <indexSize)
			throw LACoreInvalidData("Coefficient Size Error",__FILE__,__LINE__);
		double leverage = coefvec[i];
		const DoubleMatrix& gridmat = pentity->getGridMat();
		const DoubleMatrix& dfmat = pentity->getDFMat();
		const DoubleMatrix& termmat = pentity->getTermMat();

		if (gridmat.size() <= pos || dfmat.size() <= pos)
			throw LACoreInvalidData("GridMatSizeError",__FILE__,__LINE__);
		
		if (gridmat[pos].size() < 2)
			continue;

		//calc numeraire
		double numeraire = 0.0;
		for (unsigned int j = 1; j < gridmat[pos].size(); j++)
		{
			numeraire += termmat[pos][j - 1] * dfmat[pos][j];
		}
		
		DoubleVector differentialvec(gridmat[pos].size());
		double actTime = 0.0;
		for (unsigned int j = 0; j < differentialvec.size(); j++)
		{
			actTime = gridmat[pos][j];
			if (0==j)
			{
				differentialvec[j] = leverage * sgn * notionalcf * mTerm * (-actTime) * (cmsrate + dfmat[pos].back()/ numeraire);
			}
			else if (j == differentialvec.size() -1)
			{
				differentialvec[j] = leverage * sgn * notionalcf * mTerm * (actTime) * (termmat[pos][j-1] * dfmat[pos][j] / numeraire ) * (cmsrate + 1 / termmat[pos][j-1]);
			}
			else //this case means cms type not libor type
			{
				differentialvec[j] = leverage * sgn * notionalcf * mTerm * (actTime) * (termmat[pos][j-1] * dfmat[pos][j] / numeraire ) * (cmsrate);
			}
		}

		DoubleVector discountvec(differentialvec.size(),pNumeInterp->value(actTime));
		transform(differentialvec.begin(), differentialvec.end(), discountvec.begin(), differentialvec.begin(), multiplies<double>());

		
		//const LAMathYieldCurvePro& yldPro = pentity->getYieldCurvePro();
		LAString curvetype = pentity->getBasis().get();
		curvetype = (curvetype.size() == 0) ? "STD": curvetype;
		//std::map<LAString, double>& curvemap = yldPro.getCurveDependeny(curvetype);
		
		//std::map<LAString, double>::iterator it;
		//for (it = curvemap.begin(); it != curvemap.end(); ++it)
		//{
			//LAString key = pentity->getCurrency().get() + "_" + it->first;
			LAString key = pentity->getCurrency().get() + "_" + curvetype;
			//double direction = it->second;
			double direction = 1.0;
			for (unsigned int j = 0; j < differentialvec.size(); j++)
			{
				actTime = gridmat[pos][j];
				delivationLiborMap[key][actTime] += differentialvec[j] * direction;
			}
		//}
	}
	return;
}



/*!
    @brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number = 0)
	@param[in] cashlet cashlet object
	@param[in] payoff LAPricePayOff object
	@param[in] current position of payoff(first payoff position = 0)
*/
void
LAPricePayOffTool::setUp(const LADate& basedate, const LAObject& trade,
						unsigned int legNo, const LAObject& cashlet, 
						const LAPricePayOff& payoff,
						unsigned int currentpos)
{
	const LADataHolder* dh;
	
	//initialize
	mpCpnSelectOperator = NULL;
	mpFX_for_PayOff = NULL;
	mpFX_for_Notional = NULL;
	mpFX_for_ExtraCF = NULL;

	mpRefFX_for_PayOff = NULL;
	mpRefFX_for_Notional = NULL;
	mpRefFX_for_ExtraCF = NULL;


	mpNotionalCFCpnSelectOperator = NULL;
	
	for (unsigned int i = 0; i< mpNotionalCFCoupons.size(); i++)
		if (mpNotionalCFCoupons[i] != NULL)	delete mpNotionalCFCoupons[i];
	mpNotionalCFCoupons.clear();
	if (mpNotionalCFCpnSelectOperator != NULL) delete mpNotionalCFCpnSelectOperator;
	mNotionalCFCouponsCur.clear();

	for (unsigned int i = 0; i < mpCoupons.size(); i++)
		if (mpCoupons[i] != NULL) delete mpCoupons[i];
	mpCoupons.clear();
	mBaseCur = "";
    mNotionalCur = "";
	mExtraCFCur = "";
	mCouponsCur.clear();

	//payment date
	dh = &(cashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
	mPaymentDate = dynamic_cast<const LADataDate&>(dh->get()).get();
	//path object
	dh = &(trade.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
	LAPriceDataDayCount dc_path;

	if (ref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
	{
		const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(ref.get().get());
		//daycount of path object
		dc_path = path.getDayCount();
		//payment time
		mPaymentTime = dc_path.getTerm(basedate, mPaymentDate);
	}
	else if (ref.get().get().isTypeOf(ENTITY_PATH))
	{
		const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(ref.get().get());
		//daycount of path object
		dc_path = path.getDayCount();
		//payment time
		mPaymentTime = dc_path.getTerm(basedate, mPaymentDate);
	}	

	//extra cf Fixing Time
	dh = &(cashlet.getData(PRICING_DATA_EXTRACFFIXINGOFFSET, NOCHECK));	
	mExtraCFFixingTime = mPaymentTime;
	if(dh->isDefined() && !dh->isNull())
	{
		dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
		dh = &(legs.get(legNo).getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL));
		const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
		int extraCFFixingOffSet = dynamic_cast<const LADataInt&>((cashlet.getData(PRICING_DATA_EXTRACFFIXINGOFFSET, NOCHECK)).get());
		if (extraCFFixingOffSet < 0)
		{
			//error
			LAString msg = "extraCFFixingOffSet is not minus";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		LADate ExtraCFFixingDate = cal.getBusinessDay(mPaymentDate, -extraCFFixingOffSet);
		mExtraCFFixingTime = dc_path.getTerm(basedate, ExtraCFFixingDate);		
	}
	//base currency
	dh = &(trade.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	mBaseCur = dynamic_cast<const LADataString&>(dh->get()).get();
	dh = &(trade.getData(PRICING_DATA_ISLEGBASECURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		if (dynamic_cast<const LADataBool &>(dh->get()).get())
		{
			dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
			const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
			mBaseCur = dynamic_cast<LADataString &>(legs.get(legNo).getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
		}
	}
	//fx rate for pv
	dh = &(trade.getData(PRICING_DATA_FXRATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
		mpRefFX_for_PayOff = &ref;
		mpFX_for_PayOff = &dynamic_cast<const LAMathFXEntity&>(ref.get().get());	
	}
	//notional
	dh = &(cashlet.getData(PRICING_CALIBRATION_DATAOTIONAL, ISNOTNULL));
	mNotional = dynamic_cast<const LADataDouble&>(dh->get()).get();
	//denominator
	dh = &(cashlet.getData(PRICING_DATA_DENOMINATOR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mDenomiRatio = mNotional / dynamic_cast<const LADataDouble&>(dh->get()).get();
	else
		mDenomiRatio = 1;

	// for non-deliverable (vanilla and exotic) swaps
	dh = &(cashlet.getData(PRICING_DATA_SETTLEMENTADJUSTRATIO, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mSettlementAdjustRatio = dynamic_cast<const LADataDouble&>(dh->get()).get();
		mIsNonDeliverable = true;
	}
	else
	{
		mSettlementAdjustRatio = 1.0;
		mIsNonDeliverable = false;
	}
	dh = &(cashlet.getData(PRICING_DATA_SETTLEMENTFIXINGDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mSettlementFixingDate = dynamic_cast<const LADataDate&>(dh->get()).get();

	//round function
	dh = &(cashlet.getData(PRICING_DATA_ROUNDFUNCTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsRound = true;
		LAString roundfunction = dynamic_cast<const LADataString&>(dh->get()).get();
		roundfunction.toUpper();
		if (roundfunction == ROUND_STR) mRoundFunction = ROUND;
		else if (roundfunction == ROUND_UP_STR) mRoundFunction = ROUND_UP;
		else mRoundFunction = ROUND_DOWN;

		dh = &(cashlet.getData(PRICING_DATA_ROUNDDIGIT, ISNOTNULL));
		mRoundDigit = dynamic_cast<const LADataInt&>(dh->get()).get();
	}

	//notional currecny
	dh = &(cashlet.getData(PRICING_DATA_CURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mNotionalCur = dynamic_cast<const LADataString&>(dh->get()).get();
	else
		mNotionalCur = mBaseCur;

    if( (dh=&cashlet.getData(PRICING_CALIBRATION_DATAOTIONALCFCURRENCY))->isDefined() && !dh->isNull() ){
        mNotionalCFCur = dynamic_cast<const LADataString&>(dh->get()).get();
    }
    else{
        mNotionalCFCur = mNotionalCur;
    }
	
    mpCouponCFFXRateValue.release();
    if( (dh=&cashlet.getData(PRICING_DATA_COUPONCFFXRATEVALUE))->isDefined() && !dh->isNull() ){
        mpCouponCFFXRateValue.reset(new double(dynamic_cast<const LADataDouble&>(dh->get()).get()));
    }

	//fx rate for notional
	dh = &(cashlet.getData(PRICING_DATA_FXRATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
		mpRefFX_for_Notional = &ref;
		mpFX_for_Notional = &dynamic_cast<const LAMathFXEntity&>(ref.get().get());	
	}
	//notional cf
	mNotionalCF = 0;
	dh = &(cashlet.getData(PRICING_CALIBRATION_DATAOTIONALCF, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mNotionalCF = dynamic_cast<const LADataDouble&>(dh->get()).get();

	dh = &(cashlet.getData(PRICING_DATA_RENOTIONALFIXINGDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mRenotionalFixingDate = dynamic_cast<const LADataDate&>(dh->get()).get();
	
	//check
	if (mNotionalCF != 0 && mBaseCur != mNotionalCur &&  mpFX_for_PayOff == NULL)
	{
		//error
		LAString msg = "FxRate is needed for notional cf to change into base currency";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//notionalcf info
	dh = &(cashlet.getData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS, NOCHECK));
	if(dh->isDefined() && !dh->isNull())
	{ 
		const LADataMultiReference& notionalcfinfos = dynamic_cast<const LADataMultiReference&>(dh->get());	

		mNotionalCFCouponsCur.resize(notionalcfinfos.getSize());
		for (unsigned int i = 0; i < notionalcfinfos.getSize(); i++)
		{
			mpNotionalCFCoupons.push_back(new LAPriceCouponTool());
			mpNotionalCFCoupons[i]->setUp(basedate, trade, legNo, notionalcfinfos.get(i).get(), payoff, currentpos); 
			//notional coupon currecny
			dh = &(notionalcfinfos.get(i).getData(PRICING_DATA_CURRENCY, ISNOTNULL));
			mNotionalCFCouponsCur[i] = dynamic_cast<const LADataString&>(dh->get()).get();
			//check
			if(mNotionalCFCur != mNotionalCFCouponsCur[i])
			{ 
				if (mpFX_for_PayOff == NULL)
				{
					//error
					LAString msg = "FxEntity is need When notionalCFcoupon's currency and notionalCF's currency differ ";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
			}
		}
		if (mpNotionalCFCoupons.size() > 1)
		{
			dh = &(cashlet.getData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONSELECTOPERATOR, NOCHECK));
			if (!dh->isDefined() || dh->isNull())
			{
				LAString msg = "NotionalSelectCFFunction is need for more than 2 notionalCFcoupon ";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
			mpNotionalCFCpnSelectOperator = dynamic_cast<LAPriceCouponBase*>(method.getFunction().clone());	
		}
	}

	//extra cf
	mExtraCF = 0;
	dh = &(cashlet.getData(PRICING_DATA_EXTRACF, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mExtraCF = dynamic_cast<const LADataDouble&>(dh->get()).get();
	//extra cf currency
	dh = &(cashlet.getData(PRICING_DATA_EXTRACFCURRENCY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mExtraCFCur = dynamic_cast<const LADataString&>(dh->get()).get();
	else
		mExtraCFCur = mNotionalCur;
	//fx rate for extra cf
	dh = &(cashlet.getData(PRICING_DATA_EXTRACFFXRATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
		mpRefFX_for_ExtraCF = &ref;
		mpFX_for_ExtraCF = &dynamic_cast<const LAMathFXEntity&>(ref.get().get());	
	}
	else
	{
		mpRefFX_for_ExtraCF = mpRefFX_for_Notional;
		mpFX_for_ExtraCF = mpFX_for_Notional;
	}
	
	//check
	if (mExtraCF != 0 && mBaseCur != mExtraCFCur &&  mpFX_for_ExtraCF == NULL)
	{
		//error
		LAString msg = "FxRate is needed for extra cf to change into base currency";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//check for ExtraCFForInitialExchange
	dh = &(cashlet.getData(PRICING_DATA_EXTRACFFORINITIALEXCHANGE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		if (mExtraCFCur != mNotionalCur)
			throw LACoreInvalidData("ExtraCFForInitialExchange must not have ExtraCFCurrency",__FILE__,__LINE__);
		
		double extracfval =  dynamic_cast<const LADataDouble&>(dh->get()).get();
		dh = &(cashlet.getData(PRICING_DATA_FXFORINITIALEXCHANGE, NOCHECK));
		extracfval *= dynamic_cast<const LADataDouble&>(dh->get()).get();
		mExtraCFFdChg = extracfval;
	}
	
	//coupon info
	dh = &(cashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) //except extra cf only
	{
		const LADataMultiReference& couponinfos = dynamic_cast<const LADataMultiReference&>(dh->get());
		
		mCouponsCur.resize(couponinfos.getSize());
		for (unsigned int i = 0; i < couponinfos.getSize(); i++)
		{
			mpCoupons.push_back(createCouponTool(cashlet, couponinfos.get(i).get()));
			mpCoupons[i]->setUp(basedate, trade, legNo, couponinfos.get(i).get(), payoff, currentpos);
			//coupon currecny
			dh = &(couponinfos.get(i).getData(PRICING_DATA_CURRENCY, ISNOTNULL));
			mCouponsCur[i] = dynamic_cast<const LADataString&>(dh->get()).get();
		}

		if (1 == couponinfos.getSize() && mpCoupons[0]->isCompoundCoupon())
			mIsTermCalc = false;
	}
	else
	{
		mIsTermCalc = false;
	}

	//coupon select operator
	dh = &(cashlet.getData(PRICING_DATA_COUPONSELECTOPERATOR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		mpCpnSelectOperator = dynamic_cast<LAPriceCouponBase*>(method.getFunction().clone());	
		mpCpnSelectOperator->setUp(basedate, trade, legNo, cashlet);
	}

	dh = &(cashlet.getData(PRICING_DATA_DAYCOUNT, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mDC = dynamic_cast<const LAPriceDataDayCount&>(dh->get());
		if(mDC.getDayCount() == ACT_ACT_ICMA)
		{
			const LAObjectHolder& leg = dynamic_cast<const LADataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get()).get(legNo);

			const LAString frequency = LACoreUtility::removeQuotation(leg.getData(PRICING_DATA_FREQUENCY, ISNOTNULL).convertToString());	
			const LAString calendar = leg.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).convertToString();
			const LAString slidingrule = leg.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).convertToString();
			
			const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference &>(leg.getData(PRICING_DATA_CASHLETS, ISNOTNULL).get());
			std::vector<LADate> startdates;
			std::vector<LADate> enddates;
			for (unsigned int i = 0; i < cashlets.getSize(); ++i)
			{
				dh = &(cashlets.get(i).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK));
				if(!dh->isDefined() || dh->isNull())
					continue;

				startdates.push_back(dynamic_cast<const LADataDate &>(cashlets.get(i).get().getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get());
				enddates.push_back(dynamic_cast<const LADataDate &>(cashlets.get(i).get().getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get());
			}
			DateMatrix regular_startenddates = LAMathDateUtilities::calcRegularDates(frequency, calendar, slidingrule, startdates, enddates);

			mDC.setCouponsInYear(12/LAMathDateCalculations::getPeriodFrequencyInMonths(frequency));
			mDC.setCouponStartDates(regular_startenddates[0]);
			mDC.setCouponEndDates(regular_startenddates[1]);
		}

	}

	dh = &(cashlet.getData(PRICING_DATA_ISREDEMPTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get())
	{
		mIsRedemption = true;
		mIsTermCalc = false;
		if (mpCpnSelectOperator != NULL)
			(dynamic_cast<LAPriceCouponForDigital2*>(mpCpnSelectOperator))->setCallSpread(false);
	}

	//CFCalcStartDate
	dh = &(cashlet.getData(PRICING_DATA_CFCALCSTARTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mStart = dynamic_cast<const LADataDate&>(dh->get()).get();
	//CFCalcEndDate
	dh = &(cashlet.getData(PRICING_DATA_CFCALCENDDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		mEnd = dynamic_cast<const LADataDate&>(dh->get()).get();
	//term
	mTerm = (mIsTermCalc) ? mDC.getTerm(mStart, mEnd, false) : 1.0;
	//check
	for (unsigned int i = 0; i < mCouponsCur.size(); i++)
	{
		if (mNotionalCur != mCouponsCur[i] && mpFX_for_Notional == NULL && mpCouponCFFXRateValue.get() == NULL)
		{
			//error
			LAString msg = "FxRate is need for notional to change into coupon currency ";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}
		if (mBaseCur != mCouponsCur[i] && mpFX_for_PayOff == NULL)
		{
			//error
			LAString msg = "FxRate in trade object is needed for cf to change into base currency";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	//for plain swap
	dh = &(cashlet.getData(PRICING_DATA_PAYMENTTIMING, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
		mIsArrear = LAPriceCFGenUtility::isArrear(timing);
	}
	else
	{
		dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTTIMING, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
			mIsArrear = LAPriceCFGenUtility::isArrear(timing);
		}
	}
}

// 
/*!
    @brief copy LAPricePayOffTool excepy members about coupon
	@param[in] a copy source LAPricePayOffTool object
	
	@return reference to this object 
*/
LAPricePayOffTool&
LAPricePayOffTool::copy(const LAPricePayOffTool& a)
{

	mPaymentDate = a.mPaymentDate;  
	mPaymentTime = a.mPaymentTime;
	mTerm = a.mTerm;
	mStart = a.mStart;
	mEnd = a.mEnd;
	mDC = a.mDC;
	mNotional = a.mNotional;
	mDenomiRatio = a.mDenomiRatio;
	mIsRound = a.mIsRound;
	mRoundFunction = a.mRoundFunction;
	mRoundDigit = a.mRoundDigit;

	mExtraCF = a.mExtraCF;
	mExtraCFFdChg = a.mExtraCFFdChg;
	mNotionalCF = a.mNotionalCF;
	mRenotionalFixingDate = a.mRenotionalFixingDate;

	mpCpnSelectOperator = NULL;
	mpFX_for_PayOff = a.mpFX_for_PayOff;
	mpFX_for_Notional = a.mpFX_for_Notional;
	mpFX_for_ExtraCF = a.mpFX_for_ExtraCF;
	mBaseCur = a.mBaseCur;
	mNotionalCur = a.mNotionalCur;
	mNotionalCFCur = a.mNotionalCFCur;
	mExtraCFCur = a.mExtraCFCur;
	mNotionalCFExchangeRate = a.mNotionalCFExchangeRate;

	for (unsigned int i = 0; i< mpNotionalCFCoupons.size(); i++)
		if (mpNotionalCFCoupons[i] != NULL)	delete mpNotionalCFCoupons[i];
	mpNotionalCFCoupons.clear();
	if (mpNotionalCFCpnSelectOperator != NULL) delete mpNotionalCFCpnSelectOperator;
	mNotionalCFCouponsCur.clear();

	for (unsigned int i = 0; i < mpCoupons.size(); i++)
		if (mpCoupons[i] != NULL) delete mpCoupons[i];
	mpCoupons.clear();
	if (mpCpnSelectOperator != NULL) delete mpCpnSelectOperator;
	mpCpnSelectOperator = NULL;
	
//	mpFX_for_Coupons.clear();
	mCouponsCur.clear();

	mIsArrear = a.mIsArrear;
	mAccruedInt = a.mAccruedInt;
	mIsTermCalc = a.mIsTermCalc;
	mIsRedemption = a.mIsRedemption;
	mSettlementAdjustRatio = a.mSettlementAdjustRatio;
	return *this;
}


/*!
    @brief calculate accrued interest
	@param[in] date date
	@return accrued interest by base currency
*/
double
LAPricePayOffTool::calcAccruedInt(const LADate& date) const
{
	double ret = calcAccruedIntbyPayOffCur(date);
	if (mBaseCur != mCouponsCur[mSelectedCpnPos])
		ret *= mpFX_for_PayOff->getRate(mCouponsCur[mSelectedCpnPos], mBaseCur, date);

	return ret;

}
/*!
    @brief calculate accrued interest
	@param[in] date date
	@return accrued interest by payoff currency
*/
double
LAPricePayOffTool::calcAccruedIntbyPayOffCur(const LADate& date) const
{
//	LADate tmpdate = date;
//	tmpdate.addDays(1);
	if (!isCouponPayment() || date <= mStart) return 0;
	
	calcPayOff();
	if (date > mEnd)
	{
		if (mIsArrear) return mCpnPayOffbyPayOffCur;
		else return 0;
	}
	

	double ret = mCpnPayOffbyPayOffCur * mDC.getTerm(mStart, date, false) / mTerm;
	if (!mIsArrear) ret -= mCpnPayOffbyPayOffCur;

	return ret;
}

/*!
    @brief default constructor
	@param[in] p potinter to payoff tool 
*/
LAPricePayOffToolHolder::LAPricePayOffToolHolder(LAPricePayOffTool* p)
: mpPayOff(p)
{
	if (mpPayOff != NULL) mpPayOff->mRefCount++;
}
/*!
    @brief destructor
*/
LAPricePayOffToolHolder::~LAPricePayOffToolHolder()
{
	if (mpPayOff != NULL)
	{
		if(mpPayOff->mRefCount-- == 1) delete mpPayOff;
	}
}
/*!
    @brief copy constructor

	@param[in] v LAPricePayOffToolHolder object
*/
LAPricePayOffToolHolder::LAPricePayOffToolHolder (const LAPricePayOffToolHolder& v)
: mpPayOff(NULL)
{
	setPayOff(v.mpPayOff);
}

double
LAPricePayOffTool::getCouponBeforeSelection(unsigned int i) const
{
	return mpCoupons[i]->getCoupon();
}

void 
LAPricePayOffTool::getCompoundedRateInfo(DateVector& start, DateVector& end, DoubleVector& term, DateVector& fixing_date, LAStringVector& fixing_flag, DoubleVector& rate) const
{
    return;
}

LAPriceCouponTool* LAPricePayOffTool::createCouponTool(const LAObject& cashlet, const LAObject& coupon_info)
{ 
    return new LAPriceCouponTool(); 
}

double LAPricePayOffTool::calculateCouponCashflow() const
{
    double cpn;
    if (mpCpnSelectOperator != NULL){
        DoubleArray x(mpCoupons.size());
        for (unsigned int i = 0; i < x.size(); i++)
            x[i] = mpCoupons[i]->calcCoupon();
        cpn = mpCpnSelectOperator->selectCoupon(x, mSelectedCpnPos);
    }
    else{
        mSelectedCpnPos = 0;
        cpn = mpCoupons[0]->calcCoupon();
    }

    mCoupon = cpn;
    return cpn * mTerm * mNotional;
}



void LAPricePayOffToolCompound::setUp(const LADate& basedate, 
                                   const LAObject& trade,
                                   unsigned int legNo,
                                   const LAObject& cashlet, 
                                   const LAPricePayOff& payoff,
                                   unsigned int currentpos)
{
    LAPricePayOffTool::setUp(basedate,
                          trade,
                          legNo,
                          cashlet,
                          payoff,
                          currentpos);

    const LADataHolder* dh;
	mpCoefficient = NULL;
    first_stub = (dh = &cashlet.getData(PRICING_DATA_FIRSTSTUBCOUPON))->isDefined() && !dh->isNull() ? getStubCoupon(dynamic_cast<const LADataString&>(dh->get()).get(), cashlet) : NULL;
    last_stub = (dh = &cashlet.getData(PRICING_DATA_LASTSTUBCOUPON))->isDefined() && !dh->isNull() ? getStubCoupon(dynamic_cast<const LADataString&>(dh->get()).get(), cashlet) : NULL;


    compound_all_days = (dh = &cashlet.getData(PRICING_DATA_COMPOUND_ON_ALL_DAYS))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
    setupStartEndDates(cashlet);
    
    const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&>(cashlet.getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());

	if(dc.getDayCount() == ACT_ACT_ICMA)
	{
		LAString msg = "Daycount ACT/ACT.ICMA is not supported in the compounding case";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	const LAObject& coupon = dynamic_cast<const LADataMultiReference&>(cashlet.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL).get()).get(0).get();
	mpCoefficient = &dynamic_cast<const LADataDoubles&>(coupon.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get());
    const double spread = mpCoefficient->get().back();
    compounding_function = &dynamic_cast<const LACompoundMethod&>(dynamic_cast<const LAPriceDataFunction&>(cashlet.getData(PRICING_DATA_COMPOUNDING_FUNCTION, ISNOTNULL).get()).getFunction());

    
        
    const size_t num_term = start_dates.size();
    rate_term_spread.resize(num_term * 3);
    for(size_t i = 0; i < start_dates.size(); i++){ rate_term_spread[num_term + i] = dc.getTerm(start_dates[i], end_dates[i], false); }
    for(size_t i = 0; i < num_term; i++){ rate_term_spread[num_term*2 + i] = spread; }
}

double LAPricePayOffToolCompound::calculateCouponCashflow() const
{
    // In compound case, coupon selection is not used.
    mSelectedCpnPos = 0;

	 if (mpCoefficient)
	 {
		 const double spread = mpCoefficient->get().back();
		 const size_t num_term = start_dates.size();
		 for(size_t i = 0; i < num_term; i++){ rate_term_spread[num_term*2 + i] = spread; }
	 }

    size_t n = start_dates.size();
    if(compounding_function->isTypeOf(FN_COMPOUNDING9) || 
       compounding_function->isTypeOf(FN_COMPOUNDING10)){           
           if(rate_term_spread.size() > 3*n + 1) throw LACoreInvalidData("size of rate_term_spread is invalid", __FILE__, __LINE__);
           if(rate_term_spread.size() == 3*n)    rate_term_spread.resize(3*n + 1);
           rate_term_spread[3*n] = mDC.getTerm(start_dates[0], end_dates.back());
    }

    LAPriceCouponToolCompound* coupon = dynamic_cast<LAPriceCouponToolCompound*>(mpCoupons[0]);
    if(coupon==NULL){
        LAString msg;
        msg += "CAST ERROR:";
        msg += "coupon tool cannot be casted to LAPriceCouponToolCompound";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    const size_t start_pos = 0;
    const size_t end_pos = n;
    coupon->calcCoupons(rate_term_spread, start_pos, end_pos);

    if(first_stub!=NULL) rate_term_spread[0] = first_stub->calcCoupon();
    if(last_stub!=NULL) rate_term_spread[n-1] = last_stub->calcCoupon();


    return (*compounding_function)(rate_term_spread) * mNotional;
}

LAPriceCouponTool* LAPricePayOffToolCompound::createCouponTool(const LAObject& cashlet, const LAObject& coupon_info)
{ 
    const LADataHolder* dh;
    const LAString& coupon_name = dynamic_cast<const LADataString&>(coupon_info.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
    const LAString& fstub_name = (dh=&cashlet.getData(PRICING_DATA_FIRSTSTUBCOUPON))->isDefined() && !dh->isNull() ? dynamic_cast<const LADataString&>(dh->get()).get() : "";
    const LAString& lstub_name = (dh=&cashlet.getData(PRICING_DATA_LASTSTUBCOUPON))->isDefined() && !dh->isNull() ? dynamic_cast<const LADataString&>(dh->get()).get() : "";
    if(coupon_name == fstub_name || coupon_name == lstub_name){
        return LAPricePayOffTool::createCouponTool(cashlet, coupon_info);
    }
    else{
        return new LAPriceCouponToolCompound(); 
    }
}

void LAPricePayOffToolCompound::setupStartEndDates(const LAObject& cashlet)
{
    /* 
    Set cf calc start/end dates from cashlet object or index object.
    When PRICING_DATA_CFCALCSTARTDATES is not given in both entities,
    they are generated under assumption that compounding term is daily.
    */

    const LADataHolder* dh;
    if((dh = &cashlet.getData(PRICING_DATA_CFCALCSTARTDATES))->isDefined() && !dh->isNull() && !compound_all_days){
        start_dates = dynamic_cast<const LADataDates&>(dh->get()).get();
        end_dates = dynamic_cast<const LADataDates&>(cashlet.getData(PRICING_DATA_CFCALCENDDATES, ISNOTNULL).get()).get();
        return;
    }

    
    const LAObject& coupon = dynamic_cast<const LADataMultiReference&>(cashlet.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL).get()).get(0).get();
    const LAObject& index  = dynamic_cast<const LADataMultiReference&>(coupon.getData(PRICING_DATA_INDEXINFOS, ISNOTNULL).get()).get(0).get();

	if ((dh = &index.getData(PRICING_DATA_CFCALCSTARTDATES))->isDefined() && !dh->isNull() && !compound_all_days) {
		start_dates = dynamic_cast<const LADataDates&>(dh->get()).get();
		end_dates = dynamic_cast<const LADataDates&>(index.getData(PRICING_DATA_CFCALCENDDATES, ISNOTNULL).get()).get();
		return;
	}


    const LAPriceDataCalendar& calendar = dynamic_cast<const LAPriceDataCalendar&>(index.getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
    LAPriceDataSlidingRule sliding_rule(SLIDING_RULE_FOLLOWING);
    const LADate& start_date = dynamic_cast<const LADataDate&>(cashlet.getData(PRICING_DATA_CFCALCSTARTDATE, ISNOTNULL).get()).get();
    const LADate& end_date = dynamic_cast<const LADataDate&>(cashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL).get()).get();

	if (calendar.getCalendar().isHoliday(end_date) || calendar.getCalendar().isHoliday(start_date)) {
		throw LACoreInvalidData("LAPricePayOffToolCompound::setupStartEndDates failed! Start date or end date is a holiday!", __FILE__,__LINE__);
	}

    start_dates.clear();
    end_dates.clear();
    LADate temp_start, temp_end;
    temp_start = temp_end = start_date;
    temp_end.addDays(1);
    temp_end = sliding_rule.getDate(temp_end, calendar);
    while(temp_start != end_date){
        if(compound_all_days){
            LADate temp2 = temp_start;
            while(temp2 != temp_end){
                start_dates.push_back(temp2);
                temp2.addDays(1);
                end_dates.push_back(temp2);
            }
        }
        else{
            start_dates.push_back(temp_start);
            end_dates.push_back(temp_end);
        }
        temp_start = temp_end;
        temp_end.addDays(1);
        temp_end = sliding_rule.getDate(temp_end, calendar);
    }
}

void LAPricePayOffToolCompound::setCashflow(double& gearing, double& forward, double& spread, LADate& fixing_date, LAString& fixing_flag) const 
{
	// gearing must be 1 in a case of compounding
	gearing = 1.;

    const size_t n = start_dates.size();
    forward = rate_term_spread[n-1];
    spread = rate_term_spread[2*n + n-1];
    LAPriceCouponToolCompound* coupon = dynamic_cast<LAPriceCouponToolCompound*>(mpCoupons[0]);
    if(coupon==NULL){
        LAString msg;
        msg += "CAST ERROR:";
        msg += "coupon tool cannot be casted to LAPriceCouponToolCompound";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    coupon->setFixingInfo(fixing_date, fixing_flag);
    return;
}

void 
LAPricePayOffToolCompound::getCompoundedRateInfo(DateVector& start, DateVector& end, DoubleVector& term, DateVector& fixing_date, LAStringVector& fixing_flag, DoubleVector& rate) const
{
    for(DateVector::const_iterator it = start_dates.begin(); it != start_dates.end(); ++it){ start.push_back(*it); }
    for(DateVector::const_iterator it = end_dates.begin(); it != end_dates.end(); ++it){ end.push_back(*it); }
    const size_t n = start_dates.size();
    for(size_t i = 0; i < n; i++){ 
        rate.push_back(rate_term_spread[i]);
        term.push_back(rate_term_spread[n + i]); 
    }

    LAPriceCouponToolCompound* coupon = dynamic_cast<LAPriceCouponToolCompound*>(mpCoupons[0]);
    if(coupon==NULL){
        LAString msg;
        msg += "CAST ERROR:";
        msg += "coupon tool cannot be casted to LAPriceCouponToolCompound";
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    coupon->setFixingInfo(fixing_date, fixing_flag);
}

LAPriceCouponTool* LAPricePayOffToolCompound::getStubCoupon(const LAString& stub_coupon_name, const LAObject& cashlet_info)
{
    const LADataMultiReference& coupon_infos = dynamic_cast<const LADataMultiReference&>(cashlet_info.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL).get());
    for(size_t i = 0; i < coupon_infos.getSize(); i++){
        const LAString& name = dynamic_cast<const LADataString&>(coupon_infos.get(i).get().getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
        if(name == stub_coupon_name) return mpCoupons[i];
    }
    std::stringstream sst;
    sst << stub_coupon_name << " is not set in " << dynamic_cast<const LADataString&>(cashlet_info.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get().getCString() << std::endl;
    throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
}
