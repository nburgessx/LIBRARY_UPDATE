/*! @file
    @brief Calibration method create class for hw
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLString.h"
#include "AQLObjectPool.h"
#include "AQLObject.h"
#include "AQLPriceDataType.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLMathDefine.h"
#include "AQLDealUtils.h"
#include "AQLCalibrationParametersFXVannaVolga.h"
#include "AQLStaticData.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsFXVannaVolga.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLMarketData.h"
#include "AQLDataMatrix.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
AQLCalibrationParametersFXVannaVolga::AQLCalibrationParametersFXVannaVolga(void)
: AQLCalibrationParameters()
{
}

// destructor
/*!

*/
AQLCalibrationParametersFXVannaVolga::~AQLCalibrationParametersFXVannaVolga(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return AQLString object name

*/
AQLString 
AQLCalibrationParametersFXVannaVolga::createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx)
{
	AQLString keyFX = fx;
	const AQLString CALIBINFONAME = keyFX.toUpper() + "_VannaVolgaCalibInfoEntity";
	AQLObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		// do nothing
		return CALIBINFONAME;
	}
	AQLObject *info = new AQLObject;
	objPool.set(CALIBINFONAME, info);
	// set name
	info->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(CALIBINFONAME);
	keyFX.toLower();

	AQLDate asOfDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	
	// basestrikes
	AQLStringVector baseStrikes = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_BASESTRIKES).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(PRICING_DATA_BASESTRIKES, new AQLDataStrings(baseStrikes));
	

	// strTerms calendar
	AQLString strTenorCal = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM_CALENDAR);
	info->add(PRICING_DATA_TERMCALENDAR, new AQLPriceDataCalendar()).convertFromString(strTenorCal);
	AQLPriceDataCalendar& cal = dynamic_cast<AQLPriceDataCalendar &>(info->getData(PRICING_DATA_TERMCALENDAR, ISNOTNULL).get());
	// strTerms slidingrule
	AQLString strTenorSliding = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM_SLIDINGRULE).toUpper();
	info->add(PRICING_DATA_TERMSLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(strTenorSliding);
	AQLPriceDataSlidingRule& sr = dynamic_cast<AQLPriceDataSlidingRule &>(info->getData(PRICING_DATA_TERMSLIDINGRULE, ISNOTNULL).get());
	// strTerms daycount
	AQLString strTenorDayCount = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM_DAYCOUNT).toUpper();
	info->add(PRICING_DATA_TERMDAYCOUNT, new AQLPriceDataDayCount()).convertFromString(strTenorDayCount);	
	//// create market param
	//// strTerms
	AQLStringVector strTerms = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataStrings(strTerms));
	unsigned int termSize = strTerms.size();
	// get fx vol file
	AQLString volFile = AQLMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VOLATILITY_FILE));


	// vanna-volga get volatility for base strikes
	unsigned int strikeSize = baseStrikes.size();
	if (strikeSize != 3)
		throw AQLCoreInvalidData("Vanna Volga method needs 3 strikes",__FILE__,__LINE__);
	//check atm strikes
	if (baseStrikes.end() == std::find(baseStrikes.begin(), baseStrikes.end(), "ATM"))
		throw AQLCoreInvalidData("Vanna Volga method needs ATM strikes",__FILE__,__LINE__);

	AQLStringVector calVec(termSize);         // fixingcalendar
	AQLStringVector slidingVec(termSize);     // slidingrule
	AQLStringVector soptOrFwdVec(termSize);   // strike
	//DoubleMatrix volMat(termSize, DoubleVector(strikeSize,0.0));

	DoubleMatrix volMat(strikeSize, DoubleVector(termSize,0.0));
	//get volatlity
	for (unsigned int i = 0; i < baseStrikes.size(); i++)
	{
		AQLString keystr = baseStrikes[i] + "_VOL";
		
		for (unsigned int j = 0; j < termSize; j++)
		{
			AQLString lterm = strTerms[j];
			lterm.toLower();
			volMat[i][j] = AQLMarketData::getVolatilityVal(volFile, lterm, keystr);
		}
	}

	
	// fx volatility matrix
	info->add(PRICING_DATA_FXVOLATILITYMATRIX, new AQLDataDoubleMatrix(volMat));


	//strangle solver version

	//maturity method
	AQLString matumethod = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_MATURITYMETHOD);
	info->add("MaturityMethod", new AQLDataString()).convertFromString(matumethod.toUpper());

	
	AQLMathFXEntity fx_tmp = *AQLMarketData::getFXEntity(objPool, "FORWARDRATE");
	unsigned int spotlag = fx_tmp.getSpotLag(fx);
	AQLString spotlagday = AQLString(static_cast<int>(spotlag)) + "D";

	// deltatype
	AQLString strdeltatype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_DELTATYPE);
	strdeltatype.toUpper();
	AQLString stratmtype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_ATMTYPE);
	stratmtype.toUpper();
	

	DateVector matuterms(termSize);
	DateVector deliveryterms(termSize);
	AQLStringVector deltatypes(termSize,strdeltatype),atmtypes(termSize,stratmtype);
	for (unsigned int i = 0; i < termSize; i++)
	{	
		AQLString lterm = strTerms[i];
		lterm.toLower();

		// vanna-volga warning this must be changed in all of sources
		//AQLString tmpdate = AQLMarketData::aqlConvertToTerm(strTerms[i]);
		AQLString tmpdate = strTerms[i];
		matuterms[i] = AQLDateCalculations::getDate(asOfDate,tmpdate,sr,&cal,true);
		deliveryterms[i] = AQLDateCalculations::getDate(matuterms[i],spotlagday,sr,&cal,true);

		deltatypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_VANNAVOLGA_DELTATYPE, lterm);
		deltatypes[i].toUpper();
		
		atmtypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_VANNAVOLGA_ATMTYPE, lterm);
		atmtypes[i].toUpper();
	}
	info->add("MaturityDates", new AQLDataDates(matuterms));
	info->add("DeliveryDates", new AQLDataDates(deliveryterms));
	info->add("DeltaTypes", new AQLDataStrings(deltatypes));
	info->add("ATMTypes", new AQLDataStrings(atmtypes));

	return CALIBINFONAME;
}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return AQLString value

*/
AQLString 
AQLCalibrationParametersFXVannaVolga::getCalibStaticDataValue(const AQLString &key, const AQLString &grid)
{
	AQLString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == AQ_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

