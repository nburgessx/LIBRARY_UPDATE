#include "BasisComponentCurve.h"
#include "LAMathDefine.h"
#include "LAInterpolationBase.h"
#include "LAPriceDataInterpolation.h"
#include "LADataMultiReference.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataSlidingRule.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LAPriceDataCalendar.h"
#include "LADataHolder.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LADataReference.h"
#include "LADate.h"
#include "LAMatrix.h"
#include "LABasic.h"
#include "LACoreTemplateType.h"
#include "LAAlgorithm.h"
#include "CurveInstruments.h"
#include "LALinearInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "ConstantDeclarations.h"
#include "LADateScheduleHelpers.h"
#include "LADateHelpers.h"
#include "LACurvePricingObject.h"
#include "ExceptionMacros.h"

const int RATE_PRIORITY_PROXIMITY_DAY_TOLERANCE = 20;

using etrading::DiscountFactors;
using etrading::bootstrapLibors;
using etrading::bootstrapFRAs;		

namespace
{
	/*!
	@brief set cut-off date to linear spline interpolator
	@param[out] inter				Interpolator object
	@param[out] yieldData			YieldData object
	@param[in]  indexName			Name of curve where interpolation is being set
	@param[in]  noCurveTypeCheck	Check curve type or not
	*/
	void setLinearSplineCutoffDate(std::shared_ptr<LAInterpolationBase>& inter, LAObject& yieldData, const LAString& indexName, const CurveCalibrationData* curveCalibrationData, bool noCurveTypeCheck = false)
	{
		// Only set linear spline cut off date on swap curve
		if (inter->isHybrid())
		{
			// Get curve type
			LAString suffix;
			if (curveCalibrationData == nullptr)
			{
				suffix = (indexName == STD) ? "" : LAString("_") + indexName;
			}
			else
			{
				LAString curve = curveCalibrationData->getMarketForCurve(indexName);
				suffix = (curve == SWAP) ? "" : LAString("_") + curve;
			}
			suffix.toUpper();

			LAString curveType;
			LADataHolder* dh = &yieldData.getData(CALIBRATION_DATA_CURVETYPE + suffix);
			if (dh->isDefined() && !dh->isNull())
			{
				curveType = dynamic_cast<LADataString&>(dh->get()).get();
			}

			if (curveType == SWAP || curveType == "BASIS" || curveType == "TENORBASIS" || noCurveTypeCheck)
			{
				const LADataHolder* cutOffAttr = &(yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK));
				if (cutOffAttr->isDefined() && !cutOffAttr->isNull())
				{
					double cuttOffDate = dynamic_cast<const LADataDouble&> (cutOffAttr->get()).get();
					inter->setJoinDateAsDouble(cuttOffDate);
				}
				else
				{
					const LADataHolder* cutOffAttr = &(yieldData.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE, NOCHECK));
					if (cutOffAttr->isDefined() && !cutOffAttr->isNull())
					{
						double cuttOffDate = dynamic_cast<const LADataDouble&> (cutOffAttr->get()).get();
						inter->setJoinDateAsDouble(cuttOffDate);
					}
					else
					{
						LAString err = "#Error: Cut off date is not defined when using mixed hybrid interpolation.";
						err += indexName;
						err += "'. Is the correct interpolation method being used?";
						throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
					}
				}
			}
			else
			{
				LAString msg = "#Error: Curve '" + indexName + "' is NOT a swap or basis curve and won't work with mixed hybrid interpolation.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
	}
}


/*
@brief Constructor
*/
BasisComponentCurve::BasisComponentCurve(const LAString& curveName,
										 CurveCalibrationData& curveCalibrationData,
										 const LADate& baseDate,
										 const bool& fastRebuildRequested)
	: GlobalCalibrationComponentCurve(curveName, baseDate, fastRebuildRequested),
	  curveCalibrationData_(curveCalibrationData),
	  interpolationStr_(""),
	  isFwdRenotional_(false),
	  isDiscount_(false),
	  isUSDleg_(false),
	  isSpreadOnAgainstLeg_(false),
	  is_fra_use_(false),
	  isYieldSpreadCalc_(false),
	  isFWDInter_(false),
	  isFwdBasis_(false),
	  spotDate_(),
	  asOfDate_(),
	  liborDate_(),
	  spotTerm_(0.0),
	  a_spotTerm_(0.0),
	  cpd_times_(1),
	  a_cpd_times_(1),
	  dfAdj_againstLeg_(0.0),
	  extrapolateTerms_(0.0),
	  interpType_(FN_INTERPOLATION),
	  spotRateTerm_(""),
	  firstInstrumentTerm_(""),
	  firstInstrumentDate_(),
	  liborRate_(0.0),
	  liborTerm_(0.0)
{
	LAString suffix_mkt = (curveName_ == STD) ? "" : "_" + curveName_;
	suffix_mkt.toUpper();

	// Get the original curve name that is case-insensitive
	LADataHolder *dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_BASISTARGETDF + suffix_mkt, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		curveNameCaseless_ = dynamic_cast<const LADataString &>(dh->get()).get();
	}

	// Initialise pInter_StateVariable_
	dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + suffix_mkt, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		// Try initialising from yieldGen interpolation
		LAPriceDataInterpolation tempInterp = dynamic_cast<const LAPriceDataInterpolation &>(dh->get());
		interpolationStr_ = tempInterp.convertToString();
		pInter_StateVariable_.reset(dynamic_cast<LAInterpolationBase*>(dynamic_cast<const LAPriceDataInterpolation &>(dh->get()).getMethod().clone()));
	}
	else
	{
		// Try initialising from basis interpolation
		dh = &curveCalibrationData.getData(IR_CALIBRATION_DATA_INTERPOLATIONBS + suffix_mkt, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			LAPriceDataInterpolation tempInterp = dynamic_cast<const LAPriceDataInterpolation &>(dh->get());
			interpolationStr_ = tempInterp.convertToString();
			pInter_StateVariable_.reset(dynamic_cast<LAInterpolationBase*>(dynamic_cast<const LAPriceDataInterpolation &>(dh->get()).getMethod().clone()));
		}
		else
		{
			// Try initialising from interpolation
			const LADataHolder *dh = &curveCalibrationData.getData(CALIBRATION_DATA_INTERPOLATION + suffix_mkt, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				LAPriceDataInterpolation tempInterp = dynamic_cast<const LAPriceDataInterpolation &>(dh->get());
				interpolationStr_ = tempInterp.convertToString();
				pInter_StateVariable_.reset(dynamic_cast<LAInterpolationBase*>(dynamic_cast<const LAPriceDataInterpolation &>(dh->get()).getMethod().clone()));
			}
			else
			{
				MLIB_THROW("Interpolation can't be properly initialised for curve '" + curveName_ + "'");
			}
		}
	}

	interpType_ = pInter_StateVariable_->getType();

	data_.clear();

	// Yield curve data as a reference
	yieldDataRef_ = curveCalibrationData.getYieldData();
	LAObjectHolder& objHolder = yieldDataRef_.get();
	LAObject& yieldData = objHolder.get();

	setLinearSplineCutoffDate(pInter_StateVariable_, yieldData, curveName_, nullptr, true);

	// Initiliase the curve in preparation for calibration
	initialise();
}


/*
@brief Calibrate swap curve using only the short end cash and forward instruments
*/
void BasisComponentCurve::initialise()
{
	LAObjectHolder& objHolder = yieldDataRef_.get();
	LAObject& yieldData = objHolder.get();
	LADataHolder *dh = 0;


	//==============================================================================================================
	// Joseph: To review the usage of mBCurveGenMap
	//==============================================================================================================

	//dh = &getData(IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS, NOCHECK);
	//if (dh->isDefined() && !dh->isNull())
	//{
	//	const LAStringVector& nonvec = dynamic_cast<LADataStrings &>(dh->get()).get();
	//	if (nonvec.end() != std::find(nonvec.begin(), nonvec.end(), curveName_))
	//	{
	//		// set generate flag true
	//		for (std::map<LAString, LAString>::const_iterator it = mAssignedCurveMktMap.begin(); it != mAssignedCurveMktMap.end(); it++)
	//		{
	//			if (it->second == mAssignedCurveMktMap[curveName_])
	//			{
	//				const LAString& curveName = it->first;
	//				LAString suffix = "_" + curveName;
	//				mBCurveGenMap[curveName] = true;
	//			}
	//		}
	//		return;
	//	}
	//}
	//if (mBCurveGenMap.find(curveName_) != mBCurveGenMap.end())
	//{
	//	return;
	//}



	LAStringVector gCurveNames;
	dh = &curveCalibrationData_.getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		gCurveNames = dynamic_cast<const LADataStrings &>(dh->get()).get();
	}

	LAString suffix = (curveName_ == STD) ? "" : "_" + curveName_;
	suffix.toUpper();

	// get market data
	const LADataMultiReference* mr_ = &curveCalibrationData_.getMarketDataRef(curveCalibrationData_.getMarketForCurve(curveName_));
	std::vector<LAObject*> data_fwd;
	std::vector<LAObject*> data_fra;
	std::vector<LAObject*> data_libor;
	std::vector<LAObject*> data_ndf;
	for (unsigned int i = 0; i < mr_->getSize(); i++)
	{
		// check use grid
		const LADataHolder *dh = &mr_->get(i).getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const LADataBool &>(dh->get()).get())
		{
			continue;
		}

		const LAString &dataType = dynamic_cast<const LADataString &>((mr_->get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		LAString tmpType = dataType;
		tmpType.toUpper();
		if (tmpType.findString(BASIS) == 0)						// Basis Swaps
		{
			data_.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(FWDFX) == 0)				// FX Forwards
		{
			data_fwd.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(FRA) == 0)					// FRA
		{
			data_fra.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(ZERO) == 0)					// Libor
		{
			data_libor.push_back(&mr_->get(i).get());
		}
		else if (tmpType.findString(NDF) == 0)					// NDF
		{
			data_ndf.push_back(&mr_->get(i).get());
		}
	}

	size_t fra_size = data_fra.size();
	fxfwd_size_ = data_fwd.size();
	ndf_size_ = data_ndf.size();
	bool fxfwd_only = false;

	// data exist check
	if (data_.empty() && data_fwd.empty())
	{
		LAString msg = "#Error: CurveName = " + curveName_ + ", basis/fwdfx data is not set.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	else if (data_fwd.empty() && data_.empty() && !data_fwd.empty()) // only fwdfx case
	{
		data_ = data_fwd;
		fxfwd_only = true;
	}
	else if (!data_fra.empty() && !data_fwd.empty())
	{
		LAString msg = "#Error: FX forwards and FRAs must not be used at the same time to calibrate basis curve";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	is_fra_use_ = false;
	if (!data_fra.empty())
	{
		is_fra_use_ = true;
	}

	// sort
	InstrumentComp comp;
	sort(data_.begin(), data_.end(), comp);

	if (!data_fwd.empty())
	{
		sort(data_fwd.begin(), data_fwd.end(), comp);
		if (!fxfwd_only && comp(data_.front(), data_fwd.back()))
		{
			throw LACoreInvalidData("#Error: ForwardFX Term must be smaller than CCS Term", __FILE__, __LINE__);
		}
	}

	if (!data_fra.empty())
	{
		sort(data_fra.begin(), data_fra.end(), comp);
	}



	//---------------------------------------------------------------------------------------
	// Developer note on 19/06/2018: 
	// Xccy basis curve is not supported in the global curve engine

	bool isForeignCcyLeg = false;
//	// foreign currency flag
//	bool isForeignCcyLeg = false;
//	dh = &curveCalibrationData_.getData(IR_CALIBRATION_DATA_ISFOREIGNCCYLEG + suffix, NOCHECK);
//	if (dh->isDefined() && !dh->isNull())
//	{
//		isForeignCcyLeg = dynamic_cast<const LADataBool &>(dh->get()).get();
//	}


	const LAString &fCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_FORECAST, ISNOTNULL).get()).get();
	const LAString &dCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_DISCOUNT, ISNOTNULL).get()).get();
	const LAString &a_fCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
	const LAString &a_dCurve = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTDISCOUNT, ISNOTNULL).get()).get();

	// check Dummy
	if (fCurve == DUMMY || dCurve == DUMMY)
	{
		throw LACoreInvalidData("#Error: Dummy curve must be used for against curve only.", __FILE__, __LINE__);
	}
	if ((a_fCurve == DUMMY && a_dCurve != DUMMY) || (a_fCurve != DUMMY && a_dCurve == DUMMY))
	{
		throw LACoreInvalidData("#Error: If DUMMY curve is used, both forecast and discount must be DUMMY curve.", __FILE__, __LINE__);
	}
	// check FixedRate
	if (a_fCurve == FIXEDRATE || a_dCurve == FIXEDRATE)
	{
		throw LACoreInvalidData("#Error: FIXEDRATE must be used for target curve only.", __FILE__, __LINE__);
	}
	if ((fCurve == FIXEDRATE && dCurve != FIXEDRATE) || (fCurve != FIXEDRATE && dCurve == FIXEDRATE))
	{
		throw LACoreInvalidData("#Error: If FIXEDRATE is used, both forecast and discount must be FIXEDRATE.", __FILE__, __LINE__);
	}

	isDiscount_ = dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISDISCOUNT, ISNOTNULL).get()).get();
	isSpreadOnAgainstLeg_ = dynamic_cast<const LADataBool &>(data_[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL).get()).get();

	// market convention info
	// cashlet
	const LAPriceDataDayCount &c_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &c_cal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &c_sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL).get());
	const LADate &c_spotdate = dynamic_cast<const LADataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL).get());
	LAString c_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL).get()).get();
	const int c_lag = dynamic_cast<const LADataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETRESETLAG, ISNOTNULL).get());

	// index
	const LAPriceDataDayCount &i_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &i_fixcal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR, ISNOTNULL).get());
	const LAPriceDataCalendar &i_paycal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &i_sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXSLIDINGRULE, ISNOTNULL).get());
	const int i_lag = dynamic_cast<const LADataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXRESETLAG, ISNOTNULL).get());
	LAString i_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXFREQUENCY, ISNOTNULL).get()).get();
	LAString i_accessary = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_INDEXACCESSARY, ISNOTNULL).get()).get();

	if (!isDiscount_)
	{
		// if forecast only
		i_freq.toUpper();
		if (i_freq != SIMPLE)
		{
			throw LACoreInvalidData("#Error: If forecast mode, only simple is possible in frequency.", __FILE__, __LINE__);
		}

		// if FixedRate
		if (fCurve == FIXEDRATE)
		{
			throw LACoreInvalidData("#Error: If FIXEDRATE is used, target must be discount curve.", __FILE__, __LINE__);
		}
	}

	// against cashlet
	const LAPriceDataDayCount &a_c_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &a_c_cal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &a_c_sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE, ISNOTNULL).get());
	const LADate &a_c_spotdate = dynamic_cast<const LADataDate &> (data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE, ISNOTNULL).get());
	LAString a_c_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL).get()).get();

	// against index
	const LAPriceDataDayCount &a_i_dc = dynamic_cast<const LAPriceDataDayCount &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL).get());
	const LAPriceDataCalendar &a_i_fixcal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR, ISNOTNULL).get());
	const LAPriceDataCalendar &a_i_paycal = dynamic_cast<const LAPriceDataCalendar &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR, ISNOTNULL).get());
	const LAPriceDataSlidingRule &a_i_sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE, ISNOTNULL).get());
	const int a_i_lag = dynamic_cast<const LADataInt &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXRESETLAG, ISNOTNULL).get());
	LAString a_i_freq = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXFREQUENCY, ISNOTNULL).get()).get();
	LAString a_i_accessary = dynamic_cast<const LADataString &>(data_[0]->getData(IR_CALIBRATION_DATA_AGTINDEXACCESSARY, ISNOTNULL).get()).get();

	// simultaneous equation
	bool isSimuEq = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSimuEq = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	// issamegridindex
	bool isSameGridIndex = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISSAMEGRIDINDEX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSameGridIndex = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	// isyieldspreadcalc
	isYieldSpreadCalc_ = true;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISYIELDSPREADCALC, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isYieldSpreadCalc_ = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	//---------------------------------------------------------------------------------------
	// Developer note on 19/06/2018: 
	// Global yield curve engine does not support isYieldSpreadCalc == TRUE for now

	isYieldSpreadCalc_ = false;

	//	if (isForeignCcyLeg && isYieldSpreadCalc_)
	//	{
	//		throw LACoreInvalidData("#Error: yield spread calc flag must be FALSE when building xccy curves.", __FILE__, __LINE__);
	//	}

	if (is_fra_use_ && isYieldSpreadCalc_)
	{
		throw LACoreInvalidData("#Error: yield spread calc flag must be FALSE when FRAs are used.", __FILE__, __LINE__);
	}

	//iseomroll
	bool isEomRoll = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEomRoll = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	//---------------------------------------------------------------------------------------
	// Developer note on 19/06/2018: 
	// 'isFwdInter' is NOT supported for now in global curve engine.
	// This is because forward rate is not the intrinsic state variable of swap curve.

	//// isfwdinterpolation
	//bool isFWDInter = false;
	//LAPriceDataInterpolation *a_fwdInter = 0;
	//std::unique_ptr<LAPriceDataInterpolation> fwdInter = 0;
	//dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISFWDINTERPOLATION, NOCHECK));
	//if (dh->isDefined() && !dh->isNull())
	//{
	//	isFWDInter = dynamic_cast<const LADataBool &>(dh->get()).get();
	//	a_fwdInter = &dynamic_cast<LAPriceDataInterpolation &>(data_[0]->getData(IR_CALIBRATION_DATA_FWDINTERPOLATION, ISNOTNULL).get());
	//	if (isDiscount_)
	//		fwdInter.reset(dynamic_cast<LAPriceDataInterpolation *>(a_fwdInter->clone()));
	//}

	// compounding	
	cpd_times_ = 1;
	a_cpd_times_ = 1;
	LAString c_freq_cpd, a_c_freq_cpd;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		c_freq_cpd = dynamic_cast<const LADataString &>(dh->get()).get();
		cpd_times_ = etrading::LADateHelpers::calcCompoundingTimes(c_freq, c_freq_cpd);
	}

	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		a_c_freq_cpd = dynamic_cast<const LADataString &>(dh->get()).get();
		a_cpd_times_ = etrading::LADateHelpers::calcCompoundingTimes(a_c_freq, a_c_freq_cpd);
	}

	// get max term
	const LAString &termMax = dynamic_cast<const LADataString &>((data_.back()->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

	// calc term (apply to month)
	int y, m, d, w;
    etrading::LADateHelpers::termStrtoYMDW(termMax, y, m, d, w);
	m = 12 * y + m;

	c_freq.toUpper();
	unsigned int mUnit = 0;
	LAString refRateTerm;
	if (c_freq == ANNUAL)
	{
		mUnit = 12;
		refRateTerm = "12M";
	}
	else if (c_freq == SEMI_ANNUAL)
	{
		mUnit = 6;
		refRateTerm = "6M";
	}
	else if (c_freq == QUARTERLY)
	{
		mUnit = 3;
		refRateTerm = "3M";
	}
	else if (c_freq == MONTHLY || c_freq == LUNAR)
	{
		mUnit = 1;
		refRateTerm = "1M";
	}
	else
	{
		LAString msg = "#Error: frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// check can be divided ?
	if (m % mUnit != 0 || d != 0)
	{
		LAString msg = "#Error: frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	a_c_freq.toUpper();
	unsigned int a_mUnit = 0;
	LAString a_refRateTerm("");
	if (a_c_freq == ANNUAL)
	{
		a_mUnit = 12;
		a_refRateTerm = "12M";
	}
	else if (a_c_freq == SEMI_ANNUAL)
	{
		a_mUnit = 6;
		a_refRateTerm = "6M";
	}
	else if (a_c_freq == QUARTERLY)
	{
		a_mUnit = 3;
		a_refRateTerm = "3M";
	}
	else if (a_c_freq == MONTHLY || a_c_freq == LUNAR)
	{
		a_mUnit = 1;
		a_refRateTerm = "1M";
	}
	else
	{
		LAString msg = "#Error: frequency is wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if ((c_freq == LUNAR && a_c_freq != LUNAR) || (c_freq != LUNAR && a_c_freq == LUNAR))
	{
		LAString msg = "#Error: if the one frequency is LUNAR, the other must be LUNAR";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int step = 1;
	unsigned int a_step = 1;
	if (mUnit < a_mUnit)
	{
		step = a_mUnit / mUnit;
	}
	else
	{
		a_step = mUnit / a_mUnit;
	}

	// set roll convention
	LAString roll_conv("");
	if (c_freq == LUNAR)
	{
		roll_conv = ROLLCONV_LUNAR;
	}
	else if (isEomRoll)
	{
		roll_conv = ROLLCONV_EOM;
	}
	else
	{
		roll_conv = ROLLCONV_NORMAL;
	}

	// Allow the target leg to use curves from a different external collection. When this happens, source
	// yield data from the external collection
	dh = &curveCalibrationData_.getData(IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION + suffix, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		LAString externalCurveCollection = dynamic_cast<const LADataString &>(dh->get()).get();

		LADataInstance* dataInstance = curveCalibrationData_.getDataInstance();
		LAObjectPool &objPool = dataInstance->getObjectPool();
		LAString err = "#Error: Could not locate the foreign Yield Curve data using '" + externalCurveCollection + "'";

		CurveCalibrationData* externalYieldCurvePro = CurveCalibrationData::getYieldCurvePro(objPool, externalCurveCollection, err);

		yieldData = externalYieldCurvePro->getYieldData().get().get();
	}

	//By Default, the stateVariable of the interpolator is discount factor
	StateVariableEnum stateVariable = STATE_VARIABLE_DF;
	const LADataHolder* handle = &(objHolder.getData(IR_CALIBRATION_DATA_STATEVARIABLE + suffix, NOCHECK));
	if (handle->isDefined() && !handle->isNull())
	{
		stateVariable = etrading::toStateVariableEnum(dynamic_cast<const LADataString&>(handle->get()).get().getCString());

		if (stateVariable != STATE_VARIABLE_DF)
		{
			throw LACoreInvalidData("#Error: For Basis Curve, interpolator's StateVariable only supports DF", __FILE__, __LINE__);
		}
	}

	const StateVariableEnum stateVariableFutureFra = STATE_VARIABLE_LOG_DF;


	// get AsOf date
	asOfDate_ = dynamic_cast<const LADataDate &> (curveCalibrationData_.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get());

	// is xccy marked to market or not
	isFwdRenotional_ = dynamic_cast<const LADataBool& > ((curveCalibrationData_.getData(IR_CALIBRATION_DATA_ISRENOTIONALADJUST + suffix,
																					IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET + suffix, ISNOTNULL)).get()); // Alias Method: First parmeter takes priority
	isUSDleg_ = false;
	LAString currency = dynamic_cast<LADataString&> ((yieldData.getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
	if (currency.toUpper() == "USD")
	{
		isUSDleg_ = true;
	}

	LAPriceDataDayCount dc_act(ACT_365);
	spotTerm_ = dc_act.getTerm(asOfDate_, c_spotdate);
	a_spotTerm_ = dc_act.getTerm(asOfDate_, a_c_spotdate);

	// if spotTerm_ is different between legs, adjust against leg df.
	dfAdj_againstLeg_ = 1.0;
	if (isDiscount_ && (spotTerm_ != a_spotTerm_))
	{
		//		dfAdj_againstLeg_ = a_d_spotdf / a_d_inter.value(spotTerm_);
	}

	// calc extrapolation terms
	LAString maxTerm, maxFreq;
	extrapolateTerms_.clear();
	dh = &curveCalibrationData_.getData(IR_CALIBRATION_DATA_MAXTERM);
	if (dh->isDefined() && !dh->isNull())
	{
		maxTerm = dynamic_cast<LADataString&>(dh->get()).get();
		maxTerm += "Y";
		maxFreq = dynamic_cast<LADataString&>(curveCalibrationData_.getData(IR_CALIBRATION_DATA_MAXTERMFREQ).get()).get();
		const LADate& maxDate = etrading::LADateHelpers::getDate(c_spotdate, maxTerm, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
		DateVector tmp_dates; DoubleArray tmp_taus;
		etrading::updateAccrualPeriodsAndPaymentDates(c_spotdate, maxDate, maxFreq, c_cal, c_sld, dc_act, tmp_dates, extrapolateTerms_, tmp_taus, isEomRoll);
	}

	preSwapSize_ = 0;

	// calc DF from FwdFX
	double fwd_spotTerm = 0.0;
	fxfwd_spotdf_ = 1.0;
	int fxfwd_size_mm = 0;
	fxfwd_terms_.clear();
	fxfwd_dfs_.clear();
	fxfwd_terms_.clear();
	fxfwd_dfs_.clear();

	//if (fxfwd_size_ > 0)
	//{
	//  // isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
	//	const bool isFXForwardQuotedAsOutright = dynamic_cast<const LADataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISRATIO, IR_CALIBRATION_DATA_ISFXOUTRIGHT, ISNOTNULL).get()).get();
	//	const LAPriceDataCalendar &fwd_cal = dynamic_cast<const LAPriceDataCalendar &>(data_fwd[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
	//	const LAPriceDataSlidingRule &fwd_sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_fwd[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
	//	const LADate &fwd_spotdate = dynamic_cast<const LADataDate &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL).get());
	//	const int fwd_spotlag = dynamic_cast<const LADataInt &> (data_fwd[0]->getData(IR_CALIBRATION_DATA_SPOTLAG, ISNOTNULL).get()).get();
	//	fwd_spotTerm = dc_act.getTerm(asof, fwd_spotdate);
	//	fxfwd_spotdf_ = d_inter.value(fwd_spotTerm);
	//	LAString fwd_freq = dynamic_cast<const LADataString &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_FREQUENCY, ISNOTNULL).get()).get();
	//	const bool fwd_eom = dynamic_cast<const LADataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, ISNOTNULL).get()).get();

	//	// set roll convention
	//	LAString fwd_roll_conv("");
	//	if (fwd_freq == LUNAR)
	//	{
	//		fwd_roll_conv = ROLLCONV_LUNAR;
	//	}
	//	else if (fwd_eom)
	//	{
	//		fwd_roll_conv = ROLLCONV_EOM;
	//	}
	//	else
	//	{
	//		fwd_roll_conv = ROLLCONV_NORMAL;
	//	}

	//	// calc extrapolation terms for fwdfx
	//	if (fxfwd_only && extrapolateTerms_.size() > 0)
	//	{
	//		extrapolateTerms_.clear();
	//		const LADate& maxDate = etrading::LADateHelpers::getDate(fwd_spotdate, maxTerm, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
	//		DateVector tmp_dates; DoubleArray tmp_taus;
	//		etrading::updateAccrualPeriodsAndPaymentDates(fwd_spotdate, maxDate, maxFreq, fwd_cal, fwd_sld, dc_act, tmp_dates, extrapolateTerms_, tmp_taus, fwd_eom);
	//	}

	//	double term = 0.0;
	//	double df = 1.0;
	//	LADate end;
	//  // isFXForwardQuotedAsOutright (can be Fx Outright or Forward Points) - aka isRatio
	//	if (isFXForwardQuotedAsOutright)
	//	{
	//		double fwd_ratio_pow = 1.0;
	//		for (size_t i = 0; i < fxfwd_size_; i++)
	//		{
	//			const double fwd_ratio = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	//			const LAString &fwd_termStr = dynamic_cast<const LADataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	//			if (fwd_termStr == "ON")
	//			{
	//				if (fwd_spotlag < 1)
	//				{
	//					continue;
	//				}
	//				else
	//				{
	//					fwd_ratio_pow *= fwd_ratio;
	//					end = etrading::LADateHelpers::getDate(asof, fwd_termStr, SLIDING_RULE_FOLLOWING, &fwd_cal, true);
	//					term = dc_act.getTerm(asof, end);
	//					df = d_inter.value(term) / fwd_ratio_pow;
	//					fxfwd_spotdf_ = df;
	//					++fxfwd_size_mm;
	//				}
	//			}
	//			else if (fwd_termStr == "TN")
	//			{
	//				if (fwd_spotlag < 2)
	//				{
	//					continue;
	//				}
	//				else
	//				{
	//					fwd_ratio_pow *= fwd_ratio;
	//					term = dc_act.getTerm(asof, fwd_spotdate); //term is asof to spot if spotlag != 2
	//					df = d_inter.value(term) / fwd_ratio_pow;
	//					fxfwd_spotdf_ = df;
	//					++fxfwd_size_mm;
	//				}
	//			}
	//			else
	//			{
	//				end = etrading::LADateHelpers::getDate(fwd_spotdate, fwd_termStr, fwd_sld, &fwd_cal, true, &fwd_roll_conv);
	//				term = dc_act.getTerm(asof, end);
	//				df = d_inter.value(term) / (fwd_ratio * fwd_ratio_pow);
	//			}
	//			fxfwd_terms_.push_back(term);
	//			fxfwd_dfs_.push_back(df);
	//		}
	//	}
	//	else
	//	{
	//		const bool fwd_ispriceccy = dynamic_cast<const LADataBool &>(data_fwd[0]->getData(IR_CALIBRATION_DATA_ISPRICECCY, IR_CALIBRATION_DATA_ISDOMESTICCURRENCY, ISNOTNULL).get()).get(); // Alias Method: First Parameter takes Priortity
	//		//get spot fx
	//		const LAMathFXEntity& fx = dynamic_cast<const LAMathFXEntity&>(getFXEntity().get().get());
	//		const LAStringVector spotccys = fx.getCurrencys().get();
	//		const DoubleArray spotfxs = fx.getSpotRates().get();

	//		double spotfx = 0.0;
	//		if (spotfxs.size() == 1)
	//		{
	//			spotfx = spotfxs[0];
	//		}
	//		else if (spotfxs.size() == 2)
	//		{
	//			double spotfx_unitccy = 0.0;
	//			double spotfx_usd_unitccy = 0.0;
	//			for (unsigned int i = 0; i < spotccys.size(); i++)
	//			{
	//				if (spotccys[i] == currency)
	//				{
	//					spotfx_unitccy = spotfxs[i];
	//				}
	//				else if (spotccys[i] == "USD")
	//				{
	//					spotfx_usd_unitccy = spotfxs[i];
	//				}
	//			}

	//			if (spotfx_unitccy == 0.0 || spotfx_usd_unitccy == 0.0)
	//			{
	//				throw LACoreInvalidData("#Error: SpotFX rates do not exist!", __FILE__, __LINE__);
	//			}
	//			spotfx = fwd_ispriceccy ? spotfx_usd_unitccy / spotfx_unitccy : spotfx_unitccy / spotfx_usd_unitccy;
	//		}
	//		else
	//		{
	//			throw LACoreInvalidData("#Error: Please provide one or two FX Spot rates", __FILE__, __LINE__);
	//		}

	//		//search ON&TN fwd spread
	//		double fwd_spread_on = 0.0;
	//		double fwd_spread_tn = 0.0;
	//		for (size_t i = 0; i < fxfwd_size_; i++)
	//		{
	//			const LAString &termStr = dynamic_cast<const LADataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	//			if (termStr == "ON")
	//			{
	//				fwd_spread_on = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	//			}
	//			else if (termStr == "TN")
	//			{
	//				fwd_spread_tn = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	//			}
	//			else if (fwd_spread_on != 0.0 && fwd_spread_tn != 0.0)
	//			{
	//				break;
	//			}
	//		}

	//		const double fwd_fx_on = fwd_spotlag != 1 ? spotfx - fwd_spread_tn - fwd_spread_on : spotfx - fwd_spread_on;
	//		const double fwd_fx_tn = spotfx - fwd_spread_tn;
	//		for (size_t i = 0; i < fxfwd_size_; i++)
	//		{
	//			const LAString &fwd_termStr = dynamic_cast<const LADataString &>((data_fwd[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
	//			double fwd_ratio = 1.0;
	//			if (fwd_termStr == "ON")
	//			{
	//				if (fwd_spotlag < 1)
	//				{
	//					continue;
	//				}
	//				else
	//				{
	//					if (fwd_spotlag == 1)
	//					{
	//						fwd_ratio = fwd_ispriceccy ? fwd_fx_on / spotfx : spotfx / fwd_fx_on;
	//					}
	//					else
	//					{
	//						fwd_ratio = fwd_ispriceccy ? fwd_fx_on / fwd_fx_tn : fwd_fx_tn / fwd_fx_on;
	//					}

	//					end = etrading::LADateHelpers::getDate(asof, fwd_termStr, SLIDING_RULE_FOLLOWING, &fwd_cal, true);
	//					term = dc_act.getTerm(asof, end);
	//					df = d_inter.value(term) * fwd_ratio;
	//					fxfwd_spotdf_ = df;
	//					++fxfwd_size_mm;
	//				}
	//			}
	//			else if (fwd_termStr == "TN")
	//			{
	//				if (fwd_spotlag < 2)
	//				{
	//					continue;
	//				}
	//				else
	//				{
	//					fwd_ratio = fwd_ispriceccy ? fwd_fx_on / spotfx : spotfx / fwd_fx_on;
	//					term = dc_act.getTerm(asof, fwd_spotdate); //term is asof to spot if spotlag != 2
	//					df = d_inter.value(term) * fwd_ratio;
	//					fxfwd_spotdf_ = df;
	//					++fxfwd_size_mm;
	//				}
	//			}
	//			else
	//			{
	//				const double fwd_spread = dynamic_cast<const LADataDouble &>((data_fwd[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
	//				const double fwd_fx = spotfx + fwd_spread;
	//				if (fwd_spotlag == 0)
	//				{
	//					fwd_ratio = fwd_ispriceccy ? spotfx / fwd_fx : fwd_fx / spotfx;
	//				}
	//				else
	//				{
	//					fwd_ratio = fwd_ispriceccy ? fwd_fx_on / fwd_fx : fwd_fx / fwd_fx_on;
	//				}
	//				end = etrading::LADateHelpers::getDate(fwd_spotdate, fwd_termStr, fwd_sld, &fwd_cal, true, &fwd_roll_conv);
	//				term = dc_act.getTerm(fwd_spotdate, end) + fwd_spotTerm;
	//				df = d_inter.value(term) * fwd_ratio;
	//			}
	//			fxfwd_terms_.push_back(term);
	//			fxfwd_dfs_.push_back(df);
	//		}
	//	}
	//	fxfwd_size_ = fxfwd_terms_.size();

	//	preSwapSize_ = fxfwd_size_ - fxfwd_size_mm;
	//}

	//// Finish curve calibration here using FXFwd only
	//if (fxfwd_only)
	//{
	//	DoubleVector terms(fxfwd_terms_.begin(), fxfwd_terms_.end());
	//	DoubleVector dfs(fxfwd_dfs_.begin(), fxfwd_dfs_.end());

	//	// insert front data
	//	DoubleMatrix termsmtx_fwd(2);
	//	termsmtx_fwd[0] = terms;
	//	termsmtx_fwd[0].erase(termsmtx_fwd[0].end() - 1);
	//	termsmtx_fwd[1] = terms;
	//	termsmtx_fwd[1].erase(termsmtx_fwd[1].begin());
	//	if (fwd_spotTerm == 0.0 && !termsmtx_fwd[0].empty() && termsmtx_fwd[0].front() != 0.0)
	//	{
	//		termsmtx_fwd[0].insert(termsmtx_fwd[0].begin(), 0.0);
	//		termsmtx_fwd[1].insert(termsmtx_fwd[1].begin(), terms.front());
	//	}

	//	terms.insert(terms.begin(), 0.0);
	//	dfs.insert(dfs.begin(), 1.0);

	//	// insert extrapolation terms
	//	if (extrapolateTerms_.size() > 0 && extrapolateTerms_.back() + fwd_spotTerm > terms.back())
	//	{
	//		for (unsigned int i = 0; i < extrapolateTerms_.size(); ++i)
	//		{
	//			const double term = extrapolateTerms_[i] + fwd_spotTerm;
	//			if (term <= terms.back())
	//			{
	//				continue;
	//			}

	//			terms.push_back(term);
	//			dfs.push_back(dfs.back());
	//			if (!termsmtx_fwd[0].empty())
	//			{
	//				termsmtx_fwd[0].push_back(termsmtx_fwd[1].back());
	//				termsmtx_fwd[1].push_back(term);
	//			}
	//		}
	//	}

	//	saveBasisCurve(curveName_, terms, termsmtx_fwd, dfs, isDiscount_ ? d_inter.convertToString() : f_inter.convertToString(), data_);

	//	return;
	//}

	// get fwd basis
	isFwdBasis_ = false;
	dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ISFWDBASIS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isFwdBasis_ = dynamic_cast<const LADataBool &>(dh->get()).get();
	}

	//Spot date of Libor	
	for (unsigned int i = 0; i < data_libor.size(); i++)
	{
		const LADate& spotdate = dynamic_cast<const LADataDate&> ((data_libor[i]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get());
		if (i == 0)
		{
			spotDate_ = spotdate;
		}
		else if (spotDate_ != spotdate)
		{
			LAString msg = "#Error: Same rate type must have same spotdate";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	// Curve Results Data Holders
	DiscountFactors dfResults;
	DoubleArray fwds;
	DoubleMatrix fwd_termsmtx;			// Dummy variables for basis curve as basis curve doesn't calculate forward rates as outputs yet
	MoneyMarketData data_moneymarket;	// Dummy variable for basis curve
		
	// calc DF from FRAs
	if (!isDiscount_ && is_fra_use_)
	{
		if (data_libor.size() == 0)
		{
			LAString msg = "#Error: " + refRateTerm + " when FRAs are used in the Basis Curve then Libor fixings must be populated in the corresponding STD swap curve.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		bool is_fwdswap = false;
		dh = &(data_fra[0]->getData(PRICING_DATA_ISFWDSWAP, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			is_fwdswap = dynamic_cast<const LADataBool &>(dh->get()).get();
		}

		// Discard swaps prior to FRAs
		LADate startDate, endDate;
		const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&> ((data_fra.back()->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
		etrading::calculateFraDates(startDate, endDate, c_spotdate, data_fra.back(), refRateTerm, c_freq, dc);

		size_t b_size = data_.size();
		std::vector<LAObject*> temp_Data_Basis(data_.begin(), data_.end());
		data_.clear();
		for (size_t i = 0; i < b_size; ++i)
		{
			const LAString &strTerm = dynamic_cast<const LADataString &>((temp_Data_Basis[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			const LADate swapEnd = etrading::LADateHelpers::getDate(c_spotdate, strTerm, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr

			if (endDate < swapEnd)
			{
				data_.push_back(temp_Data_Basis[i]);
			}
		}

		// FRA interpolation
		// Try getting it from the suffixed INTERPOLATIONFW first, failing that falls back to yieldgen.interpolation
		std::shared_ptr<LAInterpolationBase>  pInter_fw;
		dh = &curveCalibrationData_.getData(IR_CALIBRATION_DATA_INTERPOLATIONFW + LAString("_") + LAString(curveName_).toUpper(), NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			pInter_fw.reset(dynamic_cast<LAInterpolationBase*>(dynamic_cast<const LAPriceDataInterpolation &>(dh->get()).getMethod().clone()));
		}
		else
		{
			dh = &curveCalibrationData_.getData(IR_CALIBRATION_DATA_INTERPOLATIONYG + LAString("_") + LAString(curveName_).toUpper(), NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				pInter_fw.reset(dynamic_cast<LAInterpolationBase*>(dynamic_cast<const LAPriceDataInterpolation &>(dh->get()).getMethod().clone()));
			}
			else
			{
				LAString err = "#Error: Can not locate FRA/Futures interpolation method for curve '" + curveName_ + "'";
				throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
			}
		}
		setLinearSplineCutoffDate(pInter_fw, yieldData, curveName_, nullptr, true);

		// Libor instrument spot date
		spotDate_ = c_spotdate;
		const LADataHolder *dh = &data_libor[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			spotDate_ = dynamic_cast<const LADataDate&> (dh->get());
		}

		// First swap date as cutoff date
		LADate firstSwapDate;
		const LAString &strTerm = dynamic_cast<const LADataString &>((data_[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		if (isFwdBasis_)
		{
			const bool is_date = dynamic_cast<const LADataBool&> ((data_[0]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
			LADate start, end;
			if (is_date)
			{
				firstSwapDate = dynamic_cast<const LADataDate&> ((data_[0]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
			}
			else
			{
				const LAString sterm_str = dynamic_cast<const LADataString&> ((data_[0]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
				start = etrading::LADateHelpers::getDate(c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
				const LAString tenor_str = dynamic_cast<const LADataString&> ((data_[0]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
				firstSwapDate = etrading::LADateHelpers::getDate(start, tenor_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
			}
		}
		else
		{
			firstSwapDate = etrading::LADateHelpers::getDate(c_spotdate, strTerm, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, &roll_conv);
		}

		// Bootstrap Libor Cash Deposits
		dfResults = bootstrapLibors( fwds,
									 fwd_termsmtx,
									 data_libor,
									 data_moneymarket,
									 true,				// is_fra_use
									 false,				// is_f_use
									 c_spotdate,		// spotdate_s
									 spotDate_,
									 refRateTerm,
									 false );			// implyForwards
		 
		// Bootstrap FRA Instruments
		bootstrapFRAs( dfResults,
					   fwds,							// [out] fwd			Forward rates being generated
					   fwd_termsmtx,					// [out] fwd_termsmtx	Forward rate dates
					   data_fra,						// [in] data_fra		FRA instruments
					   data_libor,						// [in] data_libor		Libor instruments
					   pInter_fw.get(),					// [in] pInter_fw		Interpolator specific to FRAs/Futures
					   stateVariableFutureFra,			// [in] stateVariable	StateVariable
					   is_fwdswap,						// [in] is_fwdswap		Using forward starting instruments?
					   c_freq,							// [in] baseFreq		Base frequency
					   refRateTerm,						// [in] refRateTerm		Reference rate term
					   asOfDate_,						// [in] asOfDate		curve AsOfDate
					   c_spotdate,						// [in] spotdate		Spot date of the curve
					   spotDate_,						// [in] spotdate_l		Spot date of the Libor instruments
					   firstSwapDate,					// [in] firstSwapDate	First swap maturity date used for tension cutoff
					   false							// [in] bProduceFwds	Imply forward rates from instruments?
					   );

		preSwapSize_ = dfResults.paymentDates_.size() - 1;
	}

	// calc DF from NDFs
	DateVector NDF_df_dates;
	NDF_DFs_.clear();
	NDF_DFs_.resize(2);
	NDF_DFs_[0].push_back(0.0);		//first term=0.0;
	NDF_DFs_[1].push_back(1.0);		//first df = 1.0;
	NDF_df_dates.push_back(c_spotdate);

	if (isDiscount_ && ndf_size_ != 0)
	{
		for (unsigned int i = 0; i < ndf_size_; ++i)
		{
			// NDF conventions
			const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount &>(data_ndf[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL).get());
			const LAPriceDataCalendar &cal = dynamic_cast<const LAPriceDataCalendar &>(data_ndf[i]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL).get());
			const LAPriceDataSlidingRule &sld = dynamic_cast<const LAPriceDataSlidingRule &>(data_ndf[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
			double rate = dynamic_cast<const LADataDouble&> ((data_ndf[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			const LAString& term = dynamic_cast<const LADataString&> ((data_ndf[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			LADate endDate = etrading::LADateHelpers::getDate(c_spotdate, term, sld, &cal, true, &roll_conv);

			// Interest Rate Convention - Stores instrument daycount and compounding conventions e.g. Simple Interest Act/Act.
			RateConvention rc = LAMathYieldCurve::setRC(CONTINUOUS);
			LAPriceDataDayCount dc_act365(ACT_365);
			LAPriceDataConvention conv(dc_act365.getDayCount(), rc);

			// The market price of a NDF is the implied zero rate that together with foreign discount factor will calculate back to the NDF FX forward
			double tau = dc_act365.getTerm(c_spotdate, endDate);
			double df = conv.getDF(rate, c_spotdate, endDate);
			NDF_DFs_[0].push_back(tau);
			NDF_DFs_[1].push_back(df);
			NDF_df_dates.push_back(endDate);
		}

		preSwapSize_ = NDF_df_dates.size() - 1;
	}

	// Calibrate DFs using basis swap instruments		
	if (!isSimuEq)
	{
		throw LACoreInvalidData("#Error: Invalid Curve Calibration Method. We support Simultaneous-Equation method only.", __FILE__, __LINE__);
	}

	unsigned int b_size = data_.size();
	if (!b_size)
	{
		throw LACoreInvalidData("#Error: Basis data is empty.", __FILE__, __LINE__);
	}

	// Initialise the following list of variables most concerned with internal basis swap pricing
	spreadVec_.resize(b_size, 0.0);
	gridVec_.resize(b_size);
	a_gridVec_.resize(b_size);

	i_gridMatVec_.resize(b_size);
	a_i_gridMatVec_.resize(b_size);

	tauVec_.resize(b_size);
	a_tauVec_.resize(b_size);

	i_termMatVec_.resize(b_size);
	a_i_termMatVec_.resize(b_size);

	effectiveStartGridVec_.resize(b_size);
	a_effectiveStartGridVec_.resize(b_size);

	DoubleArray a_targetPVVec(b_size, 0.0);


	refCurve_logDF_.resize(b_size);

	stateVariable_rates_.resize(preSwapSize_ + b_size);
	stateVariable_grid_.resize(preSwapSize_ + b_size);

	if (fxfwd_size_ > 0)					// Prepopulate DFs if FX forwards are used
	{
		if (fxfwd_terms_.size() > 0 && fwd_spotTerm != spotTerm_)
		{
			LAPriceDataInterpolation tmp_inter = dynamic_cast<LAPriceDataInterpolation &>(curveCalibrationData_.getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get());
			tmp_inter.set(fxfwd_terms_, fxfwd_dfs_);
			fxfwd_spotdf_ = tmp_inter.value(spotTerm_);//to adjust for xccy basis spot
		}

		for (size_t i = fxfwd_size_mm; i < fxfwd_size_; i++)
		{
			stateVariable_grid_[i - fxfwd_size_mm] = fxfwd_terms_[i] - spotTerm_;
			stateVariable_rates_[i - fxfwd_size_mm] = -LAMath::log(fxfwd_dfs_[i] / fxfwd_spotdf_);
		}
	}
	else if (!isDiscount_ && is_fra_use_)	// Prepopulate DFs if FRAs are used
	{
		for (size_t i = 1; i < preSwapSize_ + 1; ++i)	// i == 0 is on the spot date
		{
			stateVariable_grid_[i - 1] = FRA_DFs_[0][i];
			stateVariable_rates_[i - 1] = -LAMath::log(FRA_DFs_[1][i]);
		}
	}
	else if (isDiscount_ && ndf_size_ != 0)	// Prepopulate DFs if NDFs are used
	{
		for (size_t i = 1; i < preSwapSize_ + 1; ++i)	// i == 0 is on the spot date
		{
			stateVariable_grid_[i - 1] = NDF_DFs_[0][i];
			stateVariable_rates_[i - 1] = -LAMath::log(NDF_DFs_[1][i]);
		}
	}

	LAString a_fMarket = curveCalibrationData_.getMarketForCurve(a_fCurve);
	LAString a_dMarket = curveCalibrationData_.getMarketForCurve(a_dCurve);
	bool isSameMarket = false;
	if (a_fMarket == a_dMarket)
	{
		if (a_fMarket == IR_NO_DATA)
		{
			isSameMarket = a_fCurve == a_dCurve;
		}
		else
		{
			isSameMarket = true;
		}
	}

	// Loop through all swaps and prepare for all date information
	double initial = 0.05;
	double stepX = 0.01;
	for (unsigned int i = 0; i < b_size; ++i)
	{
		const double spread = dynamic_cast<const LADataDouble &>((data_[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		spreadVec_[i] = spread;
		const LAString &strTerm = dynamic_cast<const LADataString &>((data_[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

		//-------------------------------------------------------------------------------
		// 1. Create date schedule for the 'against' leg, when certain conditons are met

		if (isSpreadOnAgainstLeg_ ||
			(a_fCurve != DUMMY && (!isSameGridIndex || !isSameMarket || isFWDInter_)))
		{
			DateVector a_dates;
			DoubleArray a_terms_grid;
			DoubleArray a_terms_interval;
			LADate fDate = a_c_spotdate;
			double a_effectiveStartTerm(0.0);

			// Build cash flows of the reference leg, i.e. the 'against' leg
			if (isFwdBasis_)
			{
				const bool is_date = dynamic_cast<const LADataBool&> ((data_[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
				LADate start, end;
				if (is_date)
				{
					start = dynamic_cast<const LADataDate&> ((data_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
					end = dynamic_cast<const LADataDate&> ((data_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
				}
				else
				{
					const LAString sterm_str = dynamic_cast<const LADataString&> ((data_[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
					start = etrading::LADateHelpers::getDate(a_c_spotdate, sterm_str, a_c_sld, &a_c_cal, true, &roll_conv);
					const LAString tenor_str = dynamic_cast<const LADataString&> ((data_[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
					end = etrading::LADateHelpers::getDate(start, tenor_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
				}
				etrading::updateAccrualPeriodsAndPaymentDates(start, end, a_c_freq, a_c_cal, a_c_sld, a_c_dc, a_dates, a_terms_grid, a_terms_interval, isEomRoll, &a_c_spotdate);
				fDate = start;
				a_effectiveStartTerm = dc_act.getTerm(a_c_spotdate, fDate);
			}
			else
			{
				const LADate end = etrading::LADateHelpers::getDate(a_c_spotdate, strTerm, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
				etrading::updateAccrualPeriodsAndPaymentDates(a_c_spotdate, end, a_c_freq, a_c_cal, a_c_sld, a_c_dc, a_dates, a_terms_grid, a_terms_interval, isEomRoll);
			}

			DoubleMatrix a_i_gridMat(a_dates.size());
			DoubleMatrix a_i_termMat(a_dates.size());
			double targetPV = 0.0;
			double df = 1.0;

			for (unsigned int j = 0; j < a_dates.size(); ++j)
			{
				DoubleArray a_i_gridVec;
				DoubleArray a_i_termVec;
				if (isSameGridIndex)
				{
					// Fixing date terms
					a_i_gridVec.push_back(dc_act.getTerm(asOfDate_, fDate));
					LADate fixingEndDate = etrading::LADateHelpers::getDate(fDate, a_refRateTerm, a_c_sld, &a_c_cal, true, nullptr);
					a_i_gridVec.push_back(dc_act.getTerm(asOfDate_, fixingEndDate));

					// Index tau
					a_i_termVec.push_back(a_i_dc.getTerm(fDate, fixingEndDate));

					fDate = a_dates[j];
				}
				else
				{
					if (j == 0)
					{
						curveCalibrationData_.calcIndexGrid(asOfDate_, fDate, a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
					}
					else
					{
						curveCalibrationData_.calcIndexGrid(asOfDate_, a_dates[j - 1], a_i_lag, a_i_dc, a_i_fixcal, a_i_paycal, a_i_sld, a_i_freq, a_i_accessary, a_i_gridVec, a_i_termVec);
					}
				}
				a_i_gridMat[j] = a_i_gridVec;
				a_i_termMat[j] = a_i_termVec;
			}


			a_gridVec_[i] = a_terms_grid;
			a_tauVec_[i] = a_terms_interval;

			a_i_gridMatVec_[i] = a_i_gridMat;
			a_i_termMatVec_[i] = a_i_termMat;

			a_effectiveStartGridVec_[i] = a_effectiveStartTerm;
		}

		//------------------------------------------------------------------------
		// 2. Create date schedule for the usual leg

		LAPriceDataDayCount dc_act365(ACT_365);
		LADate fDate = c_spotdate;
		DateVector dates;
		DoubleArray terms_grid;
		DoubleArray terms_interval;

		if (c_spotdate < a_c_spotdate && !isSameGridIndex)
		{
			// roll from a_c_spotdate

			// Build cash flows of the target leg
			if (isFwdBasis_)
			{
				const bool is_date = dynamic_cast<const LADataBool&> ((data_[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
				LADate start, end;
				if (is_date)
				{
					start = dynamic_cast<const LADataDate&> ((data_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
					end = dynamic_cast<const LADataDate&> ((data_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
				}
				else
				{
					const LAString sterm_str = dynamic_cast<const LADataString&> ((data_[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
					start = etrading::LADateHelpers::getDate(a_c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
					const LAString tenor_str = dynamic_cast<const LADataString&> ((data_[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
					end = etrading::LADateHelpers::getDate(start, tenor_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
				}

				etrading::updateAccrualPeriodsAndPaymentDates(start, end, c_freq, c_cal, c_sld, c_dc, dates, terms_grid, terms_interval, isEomRoll, &a_c_spotdate);
				fDate = start;

				// The date fraction between the **spot date** of the curve and the start date of each **forward starting swap**
				effectiveStartGridVec_[i] = dc_act365.getTerm(a_c_spotdate, start);
			}
			else
			{
				const LADate end = etrading::LADateHelpers::getDate(a_c_spotdate, strTerm, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, &roll_conv);
				etrading::updateAccrualPeriodsAndPaymentDates(a_c_spotdate, end, c_freq, c_cal, c_sld, c_dc, dates, terms_grid, terms_interval, isEomRoll);
				effectiveStartGridVec_[i] = 0.0;
			}
			// calc odd date
			const double odd_term = dc_act.getTerm(c_spotdate, a_c_spotdate);
			const double odd_interval = c_dc.getTerm(c_spotdate, a_c_spotdate, false);
			// term from c_spotdate
			DoubleArray plus_term(terms_grid.size(), odd_term);
			transform(terms_grid.begin(), terms_grid.end(), plus_term.begin(), terms_grid.begin(), std::plus<double>());
			// insert odd term
			dates.insert(dates.begin(), c_spotdate);
			terms_grid.insert(terms_grid.begin(), odd_term);
			terms_interval.insert(terms_interval.begin(), odd_interval);
		}
		else
		{
			if (isFwdBasis_)
			{
				const bool is_date = dynamic_cast<const LADataBool&> ((data_[i]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
				LADate start, end;
				if (is_date)
				{
					start = dynamic_cast<const LADataDate&> ((data_[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
					end = dynamic_cast<const LADataDate&> ((data_[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
				}
				else
				{
					const LAString sterm_str = dynamic_cast<const LADataString&> ((data_[i]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
					start = etrading::LADateHelpers::getDate(c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
					const LAString tenor_str = dynamic_cast<const LADataString&> ((data_[i]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
					end = etrading::LADateHelpers::getDate(start, tenor_str, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
				}
				etrading::updateAccrualPeriodsAndPaymentDates(start, end, c_freq, c_cal, c_sld, c_dc, dates, terms_grid, terms_interval, isEomRoll, &c_spotdate);
				fDate = start;

				// The date fraction between the **spot date** of the curve and the start date of each **forward starting swap**
				effectiveStartGridVec_[i] = dc_act365.getTerm(c_spotdate, start);
			}
			else
			{
				const LADate end = etrading::LADateHelpers::getDate(c_spotdate, strTerm, LAPriceDataSlidingRule(SLIDING_RULE_NO_CHANGE), NULL, true, nullptr); // isAfter = true, rollConv = nullptr
				etrading::updateAccrualPeriodsAndPaymentDates(c_spotdate, end, c_freq, c_cal, c_sld, c_dc, dates, terms_grid, terms_interval, isEomRoll);
				effectiveStartGridVec_[i] = 0.0;
			}
		}

		// calc basis r * t
		DoubleArray dfs(dates.size(), 0.0);
		DoubleMatrix i_gridMat(dates.size());
		DoubleMatrix i_termMat(dates.size());
		DoubleMatrix b_yieldTimeMat(dates.size());

		for (unsigned int j = 0; j < dates.size(); ++j)
		{

			DoubleArray i_gridVec;
			DoubleArray i_termVec;
			if (isSameGridIndex)
			{
				// Fixing date terms
				i_gridVec.push_back(dc_act.getTerm(asOfDate_, fDate));

				LADate fixingEndDate = etrading::LADateHelpers::getDate(fDate, refRateTerm, c_sld, &c_cal, true, nullptr);
				i_gridVec.push_back(dc_act.getTerm(asOfDate_, fixingEndDate));

				// Index tau
				i_termVec.push_back(i_dc.getTerm(fDate, fixingEndDate));

				fDate = dates[j];
			}
			else
			{
				if (j == 0)
				{
					curveCalibrationData_.calcIndexGrid(asOfDate_, fDate, i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
				}
				else
				{
					curveCalibrationData_.calcIndexGrid(asOfDate_, dates[j - 1], i_lag, i_dc, i_fixcal, i_paycal, i_sld, i_freq, i_accessary, i_gridVec, i_termVec);
				}
			}
			i_gridMat[j] = i_gridVec;
			i_termMat[j] = i_termVec;

			if (isDiscount_)
			{
				if (isYieldSpreadCalc_)
				{
//					const double b_yield_time = -LAMath::log(d_inter.value(terms_grid[j] + spotTerm_) / d_spotdf);
//					b_yieldTimeMat[j] = DoubleArray(1, b_yield_time);
				}
				else
				{
					b_yieldTimeMat[j] = DoubleArray(1, 0.0);
				}
			}
			else
			{
				DoubleArray b_yieldTimeVec(i_gridVec.size(), 0.0);
				if (isYieldSpreadCalc_)
				{
//					for (unsigned int k = 1; k < i_gridVec.size(); ++k)
//					{
//						const double df = f_inter.value(i_gridVec[k]);
//						b_yieldTimeVec[k] = -LAMath::log(df / f_spotdf);
//					}
//					const double df1 = f_inter.value(i_gridVec[0]);
//					b_yieldTimeVec[0] = -LAMath::log(df1 / f_spotdf);
				}
				b_yieldTimeMat[j] = b_yieldTimeVec;
			}
		}

		i_gridMatVec_[i] = i_gridMat;
		i_termMatVec_[i] = i_termMat;
		gridVec_[i] = terms_grid;
		tauVec_[i] = terms_interval;

		// b_yieldTimeMat OR
		// b_yieldTimeVec OR
		// b_yieldTimeMatVec
		// The r*t term of discount factors between spot date and each index date of a swap on the **target** curve used for the **target** leg

		refCurve_logDF_[i] = b_yieldTimeMat;

		stateVariable_grid_[preSwapSize_ + i] = terms_grid.back();

		//		const double df = d_inter.value(terms_grid.back() + spotTerm_) / d_spotdf;
		//		stateVariable_rates_[preSwapSize_ + i] = -LAMath::log(df);


		stateVariable_rates_[preSwapSize_ + i] = spread * terms_grid.back();
		//stateVariable_rates_[preSwapSize_ + i] = initial + stepX * i;
	}

	// Only uses this block if the spot Libor rate of the target tenor
	// has been provided in the base STD curve
	if (!isDiscount_ && !is_fra_use_)
	{
		dh = &(data_[0]->getData(IR_CALIBRATION_DATA_SPOTRATETERM, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			// Find maturity date of the first bais swap
			firstInstrumentTerm_ = dynamic_cast<const LADataString &>((data_[0]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if (isFwdBasis_)
			{
				const bool is_date = dynamic_cast<const LADataBool&> ((data_[0]->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
				if (is_date)
				{
					firstInstrumentDate_ = dynamic_cast<const LADataDate&> ((data_[0]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
				}
				else
				{
					const LAString sterm_str = dynamic_cast<const LADataString&> ((data_[0]->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
					LADate start = etrading::LADateHelpers::getDate(c_spotdate, sterm_str, c_sld, &c_cal, true, &roll_conv);
					const LAString tenor_str = dynamic_cast<const LADataString&> ((data_[0]->getData(PRICING_DATA_TENOR, ISNOTNULL)).get()).get();
					firstInstrumentDate_ = etrading::LADateHelpers::getDate(start, tenor_str, c_sld, &c_cal, true, &roll_conv);
				}
			}
			else
			{
				firstInstrumentDate_ = etrading::LADateHelpers::getDate(c_spotdate, firstInstrumentTerm_, c_sld, &c_cal, true, &roll_conv);
			}

			// Spot libor forecasts from spot date
			spotRateTerm_ = dynamic_cast<const LADataString &>(dh->get()).get();
			liborDate_ = etrading::LADateHelpers::getDate(c_spotdate, spotRateTerm_, c_sld, &c_cal, true, &roll_conv);

			if (liborDate_ < firstInstrumentDate_)
			{
				// When no Libor tenor already exists, create an instrument to represent that

				// create data for spotrate
				DoubleArray i_gridVec;
				DoubleArray i_termVec;

				i_gridVec.push_back(dc_act.getTerm(asOfDate_, spotDate_));
				i_gridVec.push_back(dc_act.getTerm(asOfDate_, liborDate_));
				i_termVec.push_back(i_dc.getTerm(spotDate_, liborDate_));

				DoubleMatrix i_gridMat;
				DoubleMatrix i_termMat;
				i_gridMat.push_back(i_gridVec);
				i_termMat.push_back(i_termVec);

				//DoubleArray b_yieldTimeVec(i_gridVec.size());
				//if (isYieldSpreadCalc_)
				//{
				//	for (unsigned int k = 1; k < i_gridVec.size(); ++k)
				//	{
				//		const double df = f_inter.value(i_gridVec[k]);
				//		b_yieldTimeVec[k] = -LAMath::log(df / f_spotdf);
				//	}
				//	const double df1 = f_inter.value(i_gridVec[0]);
				//	b_yieldTimeVec[0] = -LAMath::log(df1 / f_spotdf);
				//}
				//DoubleMatrix b_yieldTimeMat;
				//b_yieldTimeMat.push_back(b_yieldTimeVec);

				const double term = dc_act.getTerm(spotDate_, liborDate_);
				liborTerm_ = i_dc.getTerm(spotDate_, liborDate_);
				liborRate_ = dynamic_cast<const LADataDouble &>(data_libor[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();

				i_gridMatVec_.insert(i_gridMatVec_.begin(), i_gridMat);
				i_termMatVec_.insert(i_termMatVec_.begin(), i_termMat);

				// Developer note 08 July 2018: need to review how refCurve_logDF_ is used in the context
				DoubleArray b_yieldTimeVec(i_gridVec.size(), 0.0);
				DoubleMatrix b_yieldTimeMat;
				b_yieldTimeMat.push_back(b_yieldTimeVec);
				refCurve_logDF_.insert(refCurve_logDF_.begin(), b_yieldTimeMat);

				gridVec_.insert(gridVec_.begin(), DoubleArray(1, term));
				tauVec_.insert(tauVec_.begin(), DoubleArray(1, liborTerm_));
				stateVariable_grid_.insert(stateVariable_grid_.begin(), term);
				stateVariable_rates_.insert(stateVariable_rates_.begin(), liborRate_ * liborTerm_);
				effectiveStartGridVec_.insert(effectiveStartGridVec_.begin(), 0.0);

				spreadVec_.insert(spreadVec_.begin(), 0.0);

				// The Libor instrument's PV at the against leg is no longer static and will
				// be dynamcially calculated using the Libor rate during the solving stage. For
				// now use 0.0 as placeholder.
				a_targetPVVec.insert(a_targetPVVec.begin(), 0.0);

				++b_size;
			}
			else if (liborDate_ == firstInstrumentDate_)
			{
				liborTerm_ = i_dc.getTerm(spotDate_, liborDate_);
				liborRate_ = dynamic_cast<const LADataDouble &>(data_libor[0]->getData(CALIBRATION_DATA_RATE, ISNOTNULL).get()).get();

				data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATE);
				data_[0]->remove(IR_CALIBRATION_DATA_SPOTRATETERM);
			}
			else
			{
				LAString msg = "#Error: spotrateterm is after basis first term. spotrateterm = " + spotRateTerm_ + ", basis first term = " + firstInstrumentTerm_;
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
	}

	// pInter_StateVariable_: Interpolator on the r*tau terms of the Discount Curve
	stateVariable_grid_.insert(stateVariable_grid_.begin(), 0.0);
	stateVariable_rates_.insert(stateVariable_rates_.begin(), 0.0);
	preSwapSize_++;	// To account for the front point just added in
	pInter_StateVariable_->set(stateVariable_grid_, stateVariable_rates_);

	//get convexity adjust interpolation
	pInter_adj_.reset();
	if (isDiscount_ && isFwdRenotional_)
	{
		dh = &(data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& adjustValue_term = dynamic_cast<const LADataDoubles&> (dh->get()).get();
			const DoubleArray& adjustValue
				= dynamic_cast<const LADataDoubles&> ((data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL)).get()).get();
			//pInter_adj_.reset(&(dynamic_cast<LAPriceDataInterpolation&> (data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get())));

			const LAInterpolationBase* temp = &dynamic_cast<LAPriceDataInterpolation &>(data_[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get()).getMethod();
			LAInterpolationBase* temp2 = const_cast<LAInterpolationBase*>(temp);
			pInter_adj_.reset(temp2);


			pInter_adj_->set(adjustValue_term, adjustValue);
		}
	}

	// Sanity checks on data sizes
	swapCount_ = spreadVec_.size();
	if (gridVec_.size() != swapCount_ ||
		tauVec_.size() != swapCount_ ||
		i_gridMatVec_.size() != swapCount_ ||
		i_termMatVec_.size() != swapCount_ ||
		refCurve_logDF_.size() != swapCount_)
	{
		MLIB_THROW("Inconsistent data sizes encountered when trying to construct internal basis swap instrument for basis curve calibration");
	}

	for (unsigned int i = 0; i < swapCount_; ++i)
	{
		const unsigned int gridSize = gridVec_[i].size();
		if (i_gridMatVec_[i].size() != gridSize ||
			tauVec_[i].size() != gridSize ||
			refCurve_logDF_[i].size() != gridSize)
		{
			MLIB_THROW("Inconsistent data sizes encountered when trying to construct internal basis swap instrument for basis curve calibration");
		}
	}
}


/*
    @brief Calculate the PV of all calibration instruments in the solver
    
	@param[out] allPVs		PV of all instruments
*/
void BasisComponentCurve::priceCalibrationInstruments(DoubleArray& allPVs)
{
	allPVs.clear();

	DoubleArray tmpSpreadVec(swapCount_, 0.0);
	if (!isSpreadOnAgainstLeg_)
	{
		tmpSpreadVec = spreadVec_;
	}

	DoubleArray targetPVVec(swapCount_, 0.0);
	DoubleArray a_targetPVVec(swapCount_, 0.0);

	if (isDiscount_) // for discount curve
	{
		if (isFwdRenotional_) // fwd renotional
		{
			if (isUSDleg_) // target renotional
			{
				for (unsigned int i = 0; i < swapCount_; ++i)
				{
					targetPVVec[i] = calcTargetPV_calibDfCurve_MTM(tmpSpreadVec[i], cpd_times_, spotTerm_, gridVec_[i],
						tauVec_[i], i_gridMatVec_[i], i_termMatVec_[i], refCurve_logDF_[i]);
				}
			}
			else // against renotional
			{
				for (unsigned int i = 0; i < swapCount_; ++i)
				{
					targetPVVec[i] = calcTargetPV_calibDfCurve(tmpSpreadVec[i], cpd_times_, gridVec_[i], tauVec_[i],
						i_gridMatVec_[i], i_termMatVec_[i], refCurve_logDF_[i], effectiveStartGridVec_[i]);
				}

				a_targetPVVec.clear();
				a_targetPVVec.resize(swapCount_);

				DoubleArray tmpAgtSpreadVec(swapCount_, 0.0);
				if (isSpreadOnAgainstLeg_)
				{
					tmpAgtSpreadVec = spreadVec_;
				}

				for (unsigned int i = 0; i < swapCount_; ++i)
				{
					a_targetPVVec[i] = calcAgainstPV_MTM(tmpAgtSpreadVec[i], a_cpd_times_,
						a_spotTerm_, spotTerm_, a_gridVec_[i], a_tauVec_[i], a_i_gridMatVec_[i], a_i_termMatVec_[i], effectiveStartGridVec_[i]) * dfAdj_againstLeg_;
				}
			}
		}
		else // spot renotional
		{
			for (unsigned int i = 0; i < swapCount_; ++i)
			{
				targetPVVec[i] = calcTargetPV_calibDfCurve(tmpSpreadVec[i], cpd_times_, gridVec_[i], tauVec_[i],
														i_gridMatVec_[i], i_termMatVec_[i], refCurve_logDF_[i], effectiveStartGridVec_[i]);
			}
		}
	}
	else // for forecast curve
	{
		// A Libor tenor didn't exist on the curve, add it?
		bool addLiborTenor = false;

		// A Libor tenor exists on the curve, update it?
		bool updateLiborTenor = false;

		for (unsigned int i = 0; i < swapCount_; ++i)
		{
			double targetLegCompoundTimes = cpd_times_;

			//-----------------------------------------------------------------
			// 1. Against leg PV - Libor tenor			
			if (!isDiscount_
				&& !is_fra_use_
				&& spotRateTerm_.size() != 0
				&& i == 0)
			{
				if (liborDate_ < firstInstrumentDate_)
				{
					addLiborTenor = true;
				}
				else if (liborDate_ == firstInstrumentDate_)
				{
					updateLiborTenor = true;
				}

				// The Libor instrument's PV value is now a dynmamic one.			
				LAPriceDataDayCount dc_act(ACT_365);
				const double liborFixingTerm = dc_act.getTerm(spotDate_, liborDate_);
				const double df = LAMath::exp(-pInter_df_targetLeg->value(liborFixingTerm));	// Discount factor off the discount curve on the target leg
				a_targetPVVec[i] = (1.0 + liborRate_ * liborTerm_) * df - 1.0;		// PV of one cash flow

				// When adding the Libor instrument or updating a basis instrument of the same tenor as the Libor instrument,
				// do not account for compounding.
				targetLegCompoundTimes = 1;
			}

			// Against leg PV - All subsequent tenors
			if (addLiborTenor && i != 0)
			{
				// When a new Libor tenor is added, the against leg PV for the Libor tenor has been calculated earlier. Here
				// we calculate the against leg PV for subsequent tenors
				a_targetPVVec[i] = calcAgainstPV(isSpreadOnAgainstLeg_ ? spreadVec_[i] : 0.0, a_cpd_times_,
					a_spotTerm_, a_gridVec_[i - 1], a_tauVec_[i - 1], a_i_gridMatVec_[i - 1], a_i_termMatVec_[i - 1], a_effectiveStartGridVec_[i - 1]) * dfAdj_againstLeg_;
			}
			else if ((!addLiborTenor && !updateLiborTenor)
				|| (updateLiborTenor && i != 0))
			{
				// Price target leg PV as usual if we are not inserting a Libor instrument to the curve for calibration
				a_targetPVVec[i] = calcAgainstPV(isSpreadOnAgainstLeg_ ? spreadVec_[i] : 0.0, a_cpd_times_, 
					a_spotTerm_, a_gridVec_[i], a_tauVec_[i], a_i_gridMatVec_[i], a_i_termMatVec_[i], a_effectiveStartGridVec_[i]) * dfAdj_againstLeg_;
			}

			//-----------------------------------------------------------------
			// 2. Target leg PV
			double spreadOnTargetLeg = 0.0;
			if ((!isDiscount_ && !is_fra_use_ && i == 0)
				&& (liborDate_ == firstInstrumentDate_)
				&& !isSpreadOnAgainstLeg_
				&& i == 0)
			{
				// This is the case when a basis swap instrument of the same tenor as the Libor tenor already exists
				// on the curve. The Libor rate will take priority and supersede the basis swap rate of the same tenor.
				// When the basis spread is applied on the target leg, we achieve the superseding by setting the basis spread
				// to 0, and set the against-leg-PV of the same tenor to be the same as the Libor instrument PV
				spreadOnTargetLeg = 0.0;
			}
			else
			{
				spreadOnTargetLeg = tmpSpreadVec[i];
			}

			targetPVVec[i] = calcTargetPV_calibFwdCurve(spreadOnTargetLeg, targetLegCompoundTimes, spotTerm_, gridVec_[i],
				tauVec_[i], i_gridMatVec_[i], i_termMatVec_[i], refCurve_logDF_[i], effectiveStartGridVec_[i]);
		}
	}

	allPVs.resize(swapCount_);
	for (unsigned int i = 0; i < swapCount_; ++i)
	{
		allPVs[i] = targetPVVec[i] - a_targetPVVec[i];
	}
}

/*!
@brief calc against PV (spot renotional)

@param[in] spread : basis spread
@param[in] a_cpd_times_ : compounding times
@param[in] swapSpotDateAsTerm : term of spot date
@param[in] terms_grid : cashlet grid
@param[in] terms_interval : cashlet tau
@param[in] i_gridMat : index grid
@param[in] i_termMat : index tau
@param[in] effectiveStartingTerm : Date fraction between spot date and effective start date

@return against PV
*/
double BasisComponentCurve::calcAgainstPV(const double spread,
											const int a_cpd_times_,
											const double swapSpotDateAsTerm,
											const DoubleArray &terms_grid,
											const DoubleArray &terms_interval,
											const DoubleMatrix &i_gridMat,
											const DoubleMatrix &i_termMat,
											const double effectiveStartingTerm)
{
	double ret = 0.0;
	if (terms_grid.empty())
	{
		return ret;
	}

	const double spotDiscountFactor = LAMath::exp(-pInter_df_againstLeg->value(swapSpotDateAsTerm));
	
	// i_gridMat has the date fractions for fixing start and end dates used to imply forward rates
	// terms_interval has the date fractions that represent each cash flow payment date

	double df = 1.0;
	unsigned int l = 1;
	for (l = 1; l * a_cpd_times_ - 1 < terms_grid.size(); l++) // compound grid
	{
		// calc compounding
		double cpd = 0.0;
		for (unsigned int j = (l - 1) * a_cpd_times_; j < l * a_cpd_times_; j++)
		{
			// calc index
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					double temp = LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j][k] - swapSpotDateAsTerm));
					ant += i_termMat[j][k - 1] * temp;
				}

				double frontDF = LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].front() - swapSpotDateAsTerm));
				double backDF = LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].back() - swapSpotDateAsTerm));
				rate = (frontDF - backDF) / ant;
			}
			cpd += ((rate + spread) + cpd * rate) * terms_interval[j];
		}

		double endTerm = terms_grid[l * a_cpd_times_ - 1];
		df = LAMath::exp(-pInter_df_againstLeg->value(endTerm));

		ret += cpd * df;
	}

	if (terms_grid.size() % a_cpd_times_ != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int j = (l - 1) * a_cpd_times_; j < terms_grid.size(); j++)
		{
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					ant += i_termMat[j][k - 1] * LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j][k]));
				}
				rate = (LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].front())) - LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].back()))) / ant;
			}

			double endTerm = terms_grid[j] + swapSpotDateAsTerm;
			df = LAMath::exp(-pInter_df_againstLeg->value(endTerm)) / spotDiscountFactor;

			ret += (rate + spread) * terms_interval[j] * df;
		}
	}

	// Notional exchange
	// Both 'df' and 'spotDiscountFactor' are discount factors back to the spot date. 
	// 'ret' is the swap PV as of the spot date.
	const double df_start = LAMath::exp(-pInter_df_againstLeg->value(effectiveStartingTerm));
	ret += 1.0 * df - 1.0 * df_start;

	return ret;
}

/*!
@brief calc against PV (forward renotional)

@param[in] isFWDInter : use inter of forward rate or not
@param[in] f_inter : inter of forecast DF
@param[in] fwd_inter : inter of forcast forward rate
@param[in] d_inter : inter of discount DF
@param[in] a_s_inter : inter of against spread*t
@param[in] adjust_inter : inter of convexity adjust
@param[in] spread : basis spread
@param[in] a_cpd_times_ : compounding times
@param[in] swapSpotDateAsTerm : term of spot date
@param[in] a_term_spot : term of against spot date
@param[in] terms_grid : cashlet grid
@param[in] terms_interval : cashlet tau
@param[in] i_gridMat : index grid
@param[in] i_termMat : index tau
@param[in] effectiveStartingTerm : Date fraction between spot date and effective start date

@return against PV
*/
double BasisComponentCurve::calcAgainstPV_MTM(
	const double spread,
	const int a_cpd_times_,
	const double swapSpotDateAsTerm,
	const double a_term_spot,
	const DoubleArray &terms_grid,
	const DoubleArray &terms_interval,
	const DoubleMatrix &i_gridMat,
	const DoubleMatrix &i_termMat,
	const double effectiveStartingTerm)
{
	double ret = 0.0;
	if (terms_grid.empty())
	{
		return ret;
	}

	const double spotDiscountFactor = LAMath::exp(-pInter_df_againstLeg->value(swapSpotDateAsTerm));

	double df = 1.0;
	double df_0 = 1.0;
	double df_0_agt = 1.0;
	double fwdfx = 1.0;
	unsigned int l = 1;
	for (l = 1; l * a_cpd_times_ - 1 < terms_grid.size(); l++) // compound grid
	{
		// calc compounding
		double cpd = 0.0;
		for (unsigned int j = (l - 1) * a_cpd_times_; j < l * a_cpd_times_; j++)
		{
			// calc index
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					ant += i_termMat[j][k - 1] * LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j][k]));
				}

				// r = (DF0 - DF1)/tau*DF1
				rate = (LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].front())) - LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].back()))) / ant;
			}
			cpd += ((rate + spread) + cpd * rate) * terms_interval[j];
		}

		double endTerm = terms_grid[l * a_cpd_times_ - 1] + swapSpotDateAsTerm;
		df = LAMath::exp(-pInter_df_againstLeg->value(endTerm)) / spotDiscountFactor;

		df_0 = LAMath::exp(-pInter_df_againstLeg->value(i_gridMat[l * a_cpd_times_ - 1].front())) / spotDiscountFactor;
		df_0_agt = LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[l * a_cpd_times_ - 1].front() - a_term_spot));
		fwdfx = df_0_agt / df_0; // ForwardFX(domestic/foreign) = DF_foregin / DF_domestic

								 // get convexity adjust value
		double adjust_value = 1.0;
		if (pInter_adj_.use_count() > 0)
		{
			adjust_value = LAMath::exp(-pInter_adj_->value(i_gridMat[l * a_cpd_times_ - 1].front()));
		}

		ret += ((1.0 + cpd) * df - df_0) * fwdfx * adjust_value;
	}
	if (terms_grid.size() % a_cpd_times_ != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int j = (l - 1) * a_cpd_times_; j < terms_grid.size(); j++)
		{
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					ant += i_termMat[j][k - 1] * LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j][k]));
				}
				rate = (LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].front())) - LAMath::exp(-pInter_fwd_againstLeg->value(i_gridMat[j].back()))) / ant;
			}

			double endTerm = terms_grid[j] + swapSpotDateAsTerm;
			df = LAMath::exp(-pInter_df_againstLeg->value(endTerm)) / spotDiscountFactor;

			df_0 = LAMath::exp(-pInter_df_againstLeg->value(i_gridMat[j].front())) / spotDiscountFactor;
			df_0_agt = LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].front() - a_term_spot));
			fwdfx = df_0_agt / df_0;

			double adjust_value = 1.0;
			if (pInter_adj_.use_count() > 0)
			{
				adjust_value = LAMath::exp(-pInter_adj_->value(i_gridMat[j].front()));
			}

			ret += ((1.0 + (rate + spread) * terms_interval[j]) * df - df_0) * fwdfx * adjust_value;
		}
	}
	return ret;
}

/*
@brief calc target PV for Newton-Raphson method to generate Discount Curve (spot renotional)

@param[in] spread : basis spread
@param[in] cpd_times_ : compounding times
@param[in] terms_grid : cashlet grid
@param[in] terms_interval : cashlet tau
@param[in] i_gridMat : index grid
@param[in] i_termMat : index tau
@param[in] b_yieldTimeMat : forecast r*t
@param[in] effectiveStartTerm : Date fractions between spot date and effective start date

@return target PV
*/
double BasisComponentCurve::calcTargetPV_calibDfCurve(const double spread,
													const int cpd_times_,
													const DoubleArray &terms_grid,
													const DoubleArray &terms_interval,
													const DoubleMatrix &i_gridMat,
													const DoubleMatrix &i_termMat,
													const DoubleMatrix &b_yieldTimeMat,
													const double effectiveStartTerm)
{
	double ret = 0.0;
	if (terms_grid.empty())
	{
		return ret;
	}

	for (unsigned int j = 0; j < terms_grid.size(); j++)
	{
		if (b_yieldTimeMat[j].empty())
		{
			throw LACoreInvalidData("#Error: size is not consistent.", __FILE__, __LINE__);
		}
	}

	double df = 1.0;
	unsigned int l = 1;
	for (l = 1; l * cpd_times_ - 1 < terms_grid.size(); l++) // compound grid
	{
		// calc compounding
		double cpd = 0.0;
		for (unsigned int j = (l - 1) * cpd_times_; j < l * cpd_times_; j++)
		{
			// calc index
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					ant += i_termMat[j][k - 1] * LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j][k]));
				}
				rate = (LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].front())) - LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].back()))) / ant;
			}
			cpd += ((rate + spread) + cpd * rate) * terms_interval[j];
		}
		const double y_spread_time = pInter_StateVariable_->value(terms_grid[l * cpd_times_ - 1]);
		const double yield_time = y_spread_time + b_yieldTimeMat[l * cpd_times_ - 1][0];
		df = LAMath::exp(-yield_time);
		ret += cpd * df;
	}
	if (l * cpd_times_ % terms_grid.size() != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int j = (l - 1) * cpd_times_; j < terms_grid.size(); j++)
		{
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					ant += i_termMat[j][k - 1] * LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j][k]));
				}
				rate = (LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].front())) - LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].back()))) / ant;
			}
			const double y_spread_time = pInter_StateVariable_->value(terms_grid[j]);
			const double yield_time = y_spread_time + b_yieldTimeMat[j][0];
			df = LAMath::exp(-yield_time);
			ret += (rate + spread) * terms_interval[j] * df;
		}
	}

	const double startDF_spread_time = pInter_StateVariable_->value(effectiveStartTerm);
	const double startDF = LAMath::exp(-startDF_spread_time);
	return ret + 1.0 * df - 1.0 * startDF;
}

/*
@brief calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)

@param[in] spread : basis spread
@param[in] cpd_times_ : compounding times
@param[in] swapSpotDateAsTerm : cashlet spot term
@param[in] terms_grid : cashlet grid
@param[in] terms_interval : cashlet tau
@param[in] swapSpotDateAsTerm : cashlet spot term
@param[in] i_gridMat : index grid
@param[in] i_termMat : index tau
@param[in] b_yieldTimeMat : base curve r*t

@return target PV
*/
double BasisComponentCurve::calcTargetPV_calibDfCurve_MTM(const double spread,
														const int cpd_times_,
														const double swapSpotDateAsTerm,
														const DoubleArray &terms_grid, 
														const DoubleArray &terms_interval, 
														const DoubleMatrix &i_gridMat, 
														const DoubleMatrix &i_termMat, 
														const DoubleMatrix &b_yieldTimeMat)
{
	double ret = 0.0;
	if (terms_grid.empty())
	{
		return ret;
	}

	for (unsigned int j = 0; j < terms_grid.size(); j++)
	{
		if (b_yieldTimeMat[j].empty())
		{
			throw LACoreInvalidData("#Error: size is not consistent.", __FILE__, __LINE__);
		}
	}

	const double a_df_spot = LAMath::exp(-pInter_df_againstLeg->value(swapSpotDateAsTerm));

	double df = 1.0;
	double df_0 = 1.0;
	double df_0_agt = 1.0;
	double fwdfx = 1.0;
	unsigned int l = 1;
	for (l = 1; l * cpd_times_ - 1 < terms_grid.size(); l++) // compound grid
	{
		// calc compounding
		double cpd = 0.0;
		for (unsigned int j = (l - 1) * cpd_times_; j < l * cpd_times_; j++)
		{
			// calc index
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					ant += i_termMat[j][k - 1] * LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j][k]));
				}
				rate = (LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].front())) - LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].back()))) / ant;
			}
			cpd += ((rate + spread) + cpd * rate) * terms_interval[j];
		}

		const double y_spread_time = pInter_StateVariable_->value(terms_grid[l * cpd_times_ - 1]);
		const double yield_time = y_spread_time + b_yieldTimeMat[l * cpd_times_ - 1][0];
		df = LAMath::exp(-yield_time);

		const double y_spread_time_0 = pInter_StateVariable_->value(i_gridMat[l * cpd_times_ - 1].front() - swapSpotDateAsTerm);
		const double yield_time_0 = y_spread_time_0 + b_yieldTimeMat[l * cpd_times_ - 1].front();
		df_0 = LAMath::exp(-yield_time_0);
		df_0_agt = LAMath::exp(-pInter_df_againstLeg->value(i_gridMat[l * cpd_times_ - 1].front())) / a_df_spot;
		fwdfx = df_0_agt / df_0; // ForwardFX(domestic/foreign) = DF_foregin / DF_domestic

		// get convexity adjust value
		double adjust_value = 1.0;
		if (pInter_adj_)
		{
			adjust_value = LAMath::exp(-pInter_adj_->value(i_gridMat[l * cpd_times_ - 1].front()));
		}

		ret += ((1.0 + cpd) * df - df_0) * fwdfx * adjust_value;
	}

	if (l * cpd_times_ % terms_grid.size() != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int j = (l - 1) * cpd_times_; j < terms_grid.size(); j++)
		{
			double rate = 0.0;
			if (isFWDInter_)
			{
				//rate = fwd_inter->value(i_gridMat[j].front());
				MLIB_THROW("Global curve engine does not support the usage of forward rate interpolation when calibrating basis curves");
			}
			else
			{
				double ant = 0.0;
				for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
				{
					ant += i_termMat[j][k - 1] * LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j][k]));
				}
				rate = (LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].front())) - LAMath::exp(-pInter_fwd_targetLeg->value(i_gridMat[j].back()))) / ant;
			}

			const double y_spread_time = pInter_StateVariable_->value(terms_grid[j]);
			const double yield_time = y_spread_time + b_yieldTimeMat[j][0];
			df = LAMath::exp(-yield_time);

			const double y_spread_time_0 = pInter_StateVariable_->value(i_gridMat[j].front() - swapSpotDateAsTerm);
			const double yield_time_0 = y_spread_time_0 + b_yieldTimeMat[j].front();
			df_0 = LAMath::exp(-yield_time_0);
			df_0_agt = LAMath::exp(-pInter_df_againstLeg->value(i_gridMat[j].front())) / a_df_spot;
			fwdfx = df_0_agt / df_0;

			double adjust_value = 1.0;
			if (pInter_adj_.use_count() > 0)
			{
				adjust_value = LAMath::exp(-pInter_adj_->value(i_gridMat[j].front()));
			}

			ret += ((1.0 + (rate + spread) * terms_interval[j]) * df - df_0) * fwdfx * adjust_value;
		}
	}
	return ret;
}

/*!
@brief calc target PV for Newton-Raphson method to generate Forecast Curve

@param[in] spread : basis spread
@param[in] cpd_times_ : compounding times
@param[in] swapSpotDateAsTerm : term of spot date
@param[in] terms_grid : cashlet grid
@param[in] terms_interval : cashlet tau
@param[in] i_gridMat : index grid
@param[in] i_termMat : index tau
@param[in] b_yieldTimeMat : forecast r*t
@param[in] effectiveStartTerms : Date fractions between spot date and effective start date

@return target PV
*/
double BasisComponentCurve::calcTargetPV_calibFwdCurve(const double spread,
													const int cpd_times_,
													const double swapSpotDateAsTerm,
													const DoubleArray &terms_grid,
													const DoubleArray &terms_interval,
													const DoubleMatrix &i_gridMat,
													const DoubleMatrix &i_termMat,
													const DoubleMatrix &b_yieldTimeMat,
													const double effectiveStartTerms)
{
	double ret = 0.0;
	if (terms_grid.empty())
	{
		return ret;
	}

	for (unsigned int j = 0; j < terms_grid.size(); j++)
	{
		if (b_yieldTimeMat[j].empty())
		{
			throw LACoreInvalidData("#Error: size is not consistent.", __FILE__, __LINE__);
		}
	}
	const double spotDiscountFactor = LAMath::exp(-pInter_df_targetLeg->value(swapSpotDateAsTerm));
	double df_d = spotDiscountFactor;
	unsigned int l = 1;
	for (l = 1; l * cpd_times_ - 1 < terms_grid.size(); l++) // compound grid
	{
		// calc compounding
		double cpd = 0.0;
		for (unsigned int j = (l - 1) * cpd_times_; j < l * cpd_times_; j++)
		{
			// calc index
			double df = 1.0;
			double annuity = 0.0;

			// terms_grid
			// The date fracctions between the **spot date** of the curve and the individual cash flow dates of a swap
			//
			// i_gridMat:
			// The date fraction between the **asof date** of the curve and both the start date and the end date of each cash flow of a spot or forward starting swaps
			//
			// s_inter:
			// Spread interpolator to be added to the r*t term of the target curve

			for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
			{
				const double term = i_gridMat[j][k] - swapSpotDateAsTerm;
				const double y_spread_time = pInter_StateVariable_->value(term);
				const double yield_time = y_spread_time + b_yieldTimeMat[j][k];
				df = LAMath::exp(-yield_time);
				annuity += i_termMat[j][k - 1] * df;
			}

			const double term1 = i_gridMat[j][0] - swapSpotDateAsTerm;
			const double y_spread_time1 = pInter_StateVariable_->value(term1);
			const double yield_time1 = y_spread_time1 + b_yieldTimeMat[j][0];
			const double df1 = LAMath::exp(-yield_time1);

			if (annuity == 0)
			{
				throw LACoreInvalidData("#Error: Unable to solve for the par rate. The swap annuity term is zero.", __FILE__, __LINE__);
			}

			// r = (df1 - df)/tau*df
			const double pvFloat = (df1 - df);
			const double rate = pvFloat / annuity; // calculate the par rate

			cpd += ((rate + spread) + cpd * rate) * terms_interval[j];
		}

		double endTerm = terms_grid[l * cpd_times_ - 1];
		df_d = LAMath::exp(-pInter_df_targetLeg->value(endTerm));

		// ret is the PV of the target leg
		ret += cpd * df_d;
	}
	if (terms_grid.size() % cpd_times_ != 0) // too short term to compound (rest of compound grid)
	{
		for (unsigned int j = (l - 1) * cpd_times_; j < terms_grid.size(); j++)
		{
			double df = 1.0;
			double annuity = 0.0;
			for (unsigned int k = 1; k < i_gridMat[j].size(); k++)
			{
				const double term = i_gridMat[j][k] - swapSpotDateAsTerm;
				const double y_spread_time = pInter_StateVariable_->value(term);
				const double yield_time = y_spread_time + b_yieldTimeMat[j][k];
				df = LAMath::exp(-yield_time);
				annuity += i_termMat[j][k - 1] * df;
			}

			const double term1 = i_gridMat[j][0] - swapSpotDateAsTerm;
			const double y_spread_time1 = pInter_StateVariable_->value(term1);
			const double yield_time1 = y_spread_time1 + b_yieldTimeMat[j][0];
			const double df1 = LAMath::exp(-yield_time1);

			if (annuity == 0)
			{
				throw LACoreInvalidData("#Error: Unable to solve for the par rate. The swap annuity term is zero.", __FILE__, __LINE__);
			}

			const double rate = (df1 - df) / annuity;
			const double endTerm = terms_grid[j] + swapSpotDateAsTerm;
			df_d = LAMath::exp(-pInter_df_targetLeg->value(endTerm)) / spotDiscountFactor;

			ret += (rate + spread) * terms_interval[j] * df_d;
		}
	}
	const double df_d_start = LAMath::exp(-pInter_df_targetLeg->value(effectiveStartTerms));
	ret += 1.0 * df_d - 1.0 * df_d_start;
	return ret;
}

// Post processing results once instruments have been consumed in calibration steps
void BasisComponentCurve::postProcessing(LAObject& yieldCurveProEntity)
{
	LAPriceDataInterpolation *p_base_inter = 0;
	double b_spotdf = 1.0;
	/*if (isDiscount_)
	{
		p_base_inter = &d_inter;
		b_spotdf = d_spotdf;
	}
	else
	{
		p_base_inter = &f_inter;
		b_spotdf = f_spotdf;
	}*/

	// Streamline all swap start and payment date fractions into a single vector 
	DoubleArray terms_mod = gridVec_.back();
	if (isFwdBasis_)
	{
		double grid = 0.0;
		unsigned int pos = 0;

		// Add start date of forward starting swaps to 'terms_mod'. 
		// Start date of forward starting swaps were not part of 'gridVec'
		for (unsigned int i = 0; i < effectiveStartGridVec_.size(); ++i)
		{
			// effectiveStartGridVec_: The date fraction between the spot date of the curve and the start date of each **forward starting swap**. 

			grid = effectiveStartGridVec_[i];
			if (grid == 0.0)
			{
				continue;
			}

			LAAlgorithm::locate(terms_mod, grid, terms_mod.size(), pos);
			if (pos == terms_mod.size() || terms_mod[pos] != grid)
			{
				terms_mod.insert(terms_mod.begin() + pos, grid);
			}
		}

		// Streamline all swap payment dates from 'gridVec_' into 'terms_mod'
		for (unsigned int i = 0; i < gridVec_.size() - 1; ++i)
		{
			for (unsigned int j = 0; j < gridVec_[i].size(); ++j)
			{
				grid = gridVec_[i][j];
				if (grid == 0.0)
				{
					continue;
				}
				LAAlgorithm::locate(terms_mod, grid, terms_mod.size(), pos);
				if (pos == terms_mod.size() || terms_mod[pos] != grid)
				{
					terms_mod.insert(terms_mod.begin() + pos, grid);
				}
			}
		}
	}

	// Streamline all fixing start and end date fractions into terms_mod
	if (!isDiscount_)
	{
		for (size_t i = 0; i < i_gridMatVec_.size(); ++i)
		{
			for (size_t j = 0; j < i_gridMatVec_[i].size(); ++j)
			{
				for (size_t k = 0; k < i_gridMatVec_[i][j].size(); ++k)	// i: trade index; j: cash flow index of trade; k: fixing start/end dates of a particular cash flow
				{
					double fixingDateGrid = i_gridMatVec_[i][j][k] - spotTerm_;

					if (fixingDateGrid < spotTerm_ * -1)
					{
						continue;
					}

					unsigned int pos(0);
					LAAlgorithm::locate(terms_mod, fixingDateGrid, terms_mod.size(), pos);
					if (pos == terms_mod.size() || terms_mod[pos] != fixingDateGrid)
					{
						terms_mod.insert(terms_mod.begin() + pos, fixingDateGrid);
					}
				}
			}
		}

		// Round the date count fractions to the e-9 level and remove duplicated entries in 'terms_mod'
		double precision = 1e-9;
		transform(terms_mod.begin() + 1, terms_mod.end(), terms_mod.begin() + 1, [precision](double x)->double
		{
			return boost::math::round(x / precision) * precision;
		});

		sort(terms_mod.begin(), terms_mod.end());
		removeDuplicatedTerms(terms_mod);

		if (terms_mod.front() == 0.0)
		{
			// remove the front element
			terms_mod.erase(terms_mod.begin());
		}
	}

	if (terms_mod.empty())
	{
		throw LACoreInvalidData("#Error: Dependency curves have not been built. The dependency curve(s) term and/or discount factor lookup table is empty", __FILE__, __LINE__);
	}

	// Insert FRA dates to terms_mod (node points)
	if (!isDiscount_ && is_fra_use_)
	{
		terms_mod.insert(terms_mod.end(), FRA_DFs_[0].begin() + 1, FRA_DFs_[0].end());
		std::sort(terms_mod.begin(), terms_mod.end());
		removeDuplicatedTerms(terms_mod);
	}

	// Insert NDF dates to terms_mod (node points)
	if (isDiscount_ && ndf_size_ != 0)
	{
		terms_mod.insert(terms_mod.end(), NDF_DFs_[0].begin() + 1, NDF_DFs_[0].end());
		std::sort(terms_mod.begin(), terms_mod.end());
		removeDuplicatedTerms(terms_mod);
	}

	// Determine the DFs on all the swap start, fxing and payment dates
	DoubleArray dfs(terms_mod.size());
	if (isYieldSpreadCalc_)
	{
		for (unsigned int i = 0; i < terms_mod.size(); ++i)
		{
			const double y_spread_time = pInter_StateVariable_->value(terms_mod[i]);
			const double b_spread_time = -LAMath::log(p_base_inter->value(terms_mod[i] + spotTerm_) / b_spotdf);
			dfs[i] = LAMath::exp(-(b_spread_time + y_spread_time));
		}
	}
	else
	{
		for (unsigned int i = 0; i < terms_mod.size(); ++i)
		{
			dfs[i] = LAMath::exp(-pInter_StateVariable_->value(terms_mod[i]));
		}
	}

	DoubleArray yield_mod;
	DoubleMatrix termsmtx_fwd(2);

	yield_mod.resize(terms_mod.size(), 0.0);

	// set spot -> asof
	if (spotTerm_ > 0.0)
	{
		double spotdf = 1.0;
		double yield_spot = 0.0;

		if (isDiscount_)
		{
			if (fxfwd_size_ > 0)
			{
				spotdf = fxfwd_spotdf_;
			}
			else if (isYieldSpreadCalc_)
			{
				spotdf = LAMath::exp((-stateVariable_rates_[1] / stateVariable_grid_[1]) * spotTerm_) * b_spotdf;
			}
			else
			{
				spotdf = LAMath::exp((-stateVariable_rates_[1] / stateVariable_grid_[1]) * spotTerm_);
			}
			yield_spot = -LAMath::log(spotdf) / spotTerm_;
		}
		else
		{
			// Retrieve zero rates from the r*tau 
			std::shared_ptr<LAInterpolationBase> tmp_inter(dynamic_cast<LAInterpolationBase*>(pInter_StateVariable_->clone()));

			DoubleArray tmp_yields(dfs.size());
			for (unsigned int i = 0; i < terms_mod.size(); ++i)
			{
				if (terms_mod[i] == 0)
				{
					tmp_yields[i] = 0.0;
				}
				else
				{
					tmp_yields[i] = -LAMath::log(dfs[i]) / terms_mod[i];
				}
			}

			// Retrieve spot rate through zero rate interpolator. Calculate spot DF.
			tmp_inter->set(terms_mod, tmp_yields);
			yield_spot = tmp_inter->value(spotTerm_);
			spotdf = LAMath::exp(-yield_spot * spotTerm_);
		}

		// Re-scale all discount factors and zero rates so that they all start from the asof date of the curve
		for (unsigned int i = 0; i < terms_mod.size(); ++i)
		{
			terms_mod[i] += spotTerm_;
			dfs[i] *= spotdf;
			if (terms_mod[i] == 0)
			{
				yield_mod[i] = 0.0;
			}
			else
			{
				yield_mod[i] = -LAMath::log(dfs[i]) / terms_mod[i];
			}
		}

		// Add spot term, which may not always sit at the front duing to presence os fixing lags
		if (fxfwd_size_ > 0)
		{
			for (int i = fxfwd_size_ - 1; i >= 0; --i)
			{
				if (fxfwd_terms_[i] < terms_mod.front())
				{
					terms_mod.insert(terms_mod.begin(), fxfwd_terms_[i]);
					dfs.insert(dfs.begin(), fxfwd_dfs_[i]);
					yield_mod.insert(yield_mod.begin(), -LAMath::log(fxfwd_dfs_[i]) / fxfwd_terms_[i]);
				}
			}
		}
		else
		{
			unsigned int pos(0);
			LAAlgorithm::locate(terms_mod, spotTerm_, terms_mod.size(), pos);
			if (pos == terms_mod.size() || terms_mod[pos] != spotTerm_)
			{
				terms_mod.insert(terms_mod.begin() + pos, spotTerm_);
				dfs.insert(dfs.begin() + pos, spotdf);
				yield_mod.insert(yield_mod.begin() + pos, yield_spot);
			}
		}
	}
	else
	{
		for (unsigned int i = 0; i < terms_mod.size(); ++i)
		{
			yield_mod[i] = -LAMath::log(dfs[i]) / terms_mod[i];
		}
	}

	termsmtx_fwd[0] = terms_mod;
	termsmtx_fwd[0].erase(termsmtx_fwd[0].end() - 1);
	termsmtx_fwd[1] = terms_mod;
	termsmtx_fwd[1].erase(termsmtx_fwd[1].begin());

	if (spotTerm_ == 0.0 && !termsmtx_fwd[0].empty() && termsmtx_fwd[0].front() != 0.0)
	{
		termsmtx_fwd[0].insert(termsmtx_fwd[0].begin(), 0.0);
		termsmtx_fwd[1].insert(termsmtx_fwd[1].begin(), terms_mod.front());
	}

	// Fix the curve front DF to 1.0
	unsigned int pos(0);
	LAAlgorithm::locate(terms_mod, 0.0, terms_mod.size(), pos);
	if (pos == terms_mod.size() || (pos == 0 && terms_mod[pos] != 0.0))
	{
		terms_mod.insert(terms_mod.begin(), 0.0);
		dfs.insert(dfs.begin(), 1.0);
		yield_mod.insert(yield_mod.begin(), 0.0);
	}
	else if (dfs[pos] != 1.0 || yield_mod[pos] != 0.0)
	{
		terms_mod[pos] = 0.0;
		dfs[pos] = 1.0;
		yield_mod[pos] = 0.0;
	}

	// insert extrapolation terms
	if (extrapolateTerms_.size() > 0 && extrapolateTerms_.back() + spotTerm_ > terms_mod.back())
	{
		for (unsigned int i = 0; i < extrapolateTerms_.size(); ++i)
		{
			const double term = extrapolateTerms_[i] + spotTerm_;
			if (term <= terms_mod.back())
			{
				continue;
			}
			terms_mod.push_back(term);
			dfs.push_back(dfs.back());
			yield_mod.push_back(yield_mod.back());
			if (!termsmtx_fwd[0].empty())
			{
				termsmtx_fwd[0].push_back(termsmtx_fwd[1].back());
				termsmtx_fwd[1].push_back(term);
			}
		}
	}

	DoubleArray _terms = terms_mod;


	// calc modify df
	DoubleArray dfs_mod(_terms.size(), 0.0);
	for (unsigned int i = 0; i < yield_mod.size(); i++)
	{
		dfs_mod[i] = LAMath::exp(-_terms[i] * yield_mod[i]);
	}

	DoubleArray::const_iterator min_it = min_element(_terms.begin(), _terms.end());
	if (!_terms.empty() && *min_it < 0.0)
	{
		LAString msg = curveName_ + " terms, term must be positive.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	min_it = min_element(termsmtx_fwd[0].begin(), termsmtx_fwd[0].end());
	if (!termsmtx_fwd[0].empty() && *min_it < 0.0)
	{
		LAString msg = curveName_ + " terms_fwd, term must be positive.";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// Determine the interpolation scheme adopted by the target curve
	LAString basisInter;
	double joinDateAsDouble = 0.0;
	if (isYieldSpreadCalc_)
	{
		//// When solving is done on the spreads, assume the reference curve's interpolation scheme is the basis interpolation
		//if (isDiscount_)
		//{
		//	basisInter = d_inter.convertToString();
		//	joinDateAsDouble = (d_inter.getType() == FN_LINEARSPLINEINTERPOLATION) ? d_inter.getJoinDateAsDouble() : 0.0;
		//}
		//else
		//{
		//	basisInter = f_inter.convertToString();
		//	joinDateAsDouble = (f_inter.getType() == FN_LINEARSPLINEINTERPOLATION) ? f_inter.getJoinDateAsDouble() : 0.0;
		//}

		//--------------------------------------------------------------------
		// Developer's note on 20 June 2018:
		// Global yield curve engine does not support isYieldSpreadCalc == TRUE for now
		MLIB_THROW("Global yield curve engine does not support isYieldSpreadCalc == TRUE");
	}
	else
	{
		basisInter = interpolationStr_;
		joinDateAsDouble = pInter_StateVariable_->isHybrid() ? pInter_StateVariable_->getJoinDateAsDouble() : 0.0;
	}

	curveCalibrationData_.saveBasisCurve(curveNameCaseless_, _terms, termsmtx_fwd, dfs_mod, basisInter, data_, pInter_StateVariable_->isHybrid(), joinDateAsDouble, true);

}


/* @brief Set target leg DF interpolator	
*  @param[in]	dfInterp		The interpolator object for target leg's discount factors
*/
void BasisComponentCurve::setTargetLegDFInterpolator(const std::shared_ptr<LAInterpolationBase>& dfInterp)
{
	pInter_df_targetLeg = dfInterp;
}

/* @brief Set against leg DF interpolator
*  @param[in]	dfInterp		The interpolator object for target leg's discount factors
*/
void BasisComponentCurve::setAgainstLegDFInterpolator(const std::shared_ptr<LAInterpolationBase>& dfInterp)
{
	pInter_df_againstLeg = dfInterp;
}

/* @brief Set target leg Forecast interpolator	
*  @param[in]	dfInterp		The interpolator object for target leg's discount factors
*/
void BasisComponentCurve::setTargetLegFwdInterpolator(const std::shared_ptr<LAInterpolationBase>& fwdInterp)
{
	pInter_fwd_targetLeg = fwdInterp;
}

/* @brief Set against leg Forecast interpolator	
*  @param[in]	dfInterp		The interpolator object for target leg's discount factors
*/
void BasisComponentCurve::setAgainstLegFwdInterpolator(const std::shared_ptr<LAInterpolationBase>& fwdInterp)
{
	pInter_fwd_againstLeg = fwdInterp;
}