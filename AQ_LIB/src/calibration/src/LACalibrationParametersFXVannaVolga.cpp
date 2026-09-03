/*! @file
    @brief Calibration method create class for hw
*/
//  2011, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersFXVannaVolga.cpp
//
//  DESCRIPTION :     Calibration method create class for hw
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAString.h"
#include "LAObjectPool.h"
#include "LAObject.h"
#include "LAPriceDataType.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LAMathDefine.h"
#include "LADealUtils.h"
#include "LACalibrationParametersFXVannaVolga.h"
#include "LAStaticData.h"
#include "LADefinitions.h"
#include "LADefinitionsFXVannaVolga.h"
#include "LADefinitionsCalibration.h"
#include "LAMarketData.h"
#include "LADataMatrix.h"

using namespace std;

// constructor
/*!
	@param funFlg

*/
LACalibrationParametersFXVannaVolga::LACalibrationParametersFXVannaVolga(void)
: LACalibrationParameters()
{
}

// destructor
/*!

*/
LACalibrationParametersFXVannaVolga::~LACalibrationParametersFXVannaVolga(void)
{
}


 
/*!
    @brief create  calibration info object

	@param[out] objPool
	@param[out] fx
	@return LAString object name

*/
LAString 
LACalibrationParametersFXVannaVolga::createCalibrationInfo(LAObjectPool &objPool, const LAString &fx)
{
	LAString keyFX = fx;
	const LAString CALIBINFONAME = keyFX.toUpper() + "_VannaVolgaCalibInfoEntity";
	LAObjectHolder objHolder = objPool.getObject(CALIBINFONAME, ENCHKTYPE_NOCHECK);
	if (objHolder.isDefined())
	{
		// do nothing
		return CALIBINFONAME;
	}
	LAObject *info = new LAObject;
	objPool.set(CALIBINFONAME, info);
	// set name
	info->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(CALIBINFONAME);
	keyFX.toLower();

	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	
	// basestrikes
	LAStringVector baseStrikes = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_BASESTRIKES).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(PRICING_DATA_BASESTRIKES, new LADataStrings(baseStrikes));
	

	// strTerms calendar
	LAString strTenorCal = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM_CALENDAR);
	info->add(PRICING_DATA_TERMCALENDAR, new LAPriceDataCalendar()).convertFromString(strTenorCal);
	LAPriceDataCalendar& cal = dynamic_cast<LAPriceDataCalendar &>(info->getData(PRICING_DATA_TERMCALENDAR, ISNOTNULL).get());
	// strTerms slidingrule
	LAString strTenorSliding = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM_SLIDINGRULE).toUpper();
	info->add(PRICING_DATA_TERMSLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(strTenorSliding);
	LAPriceDataSlidingRule& sr = dynamic_cast<LAPriceDataSlidingRule &>(info->getData(PRICING_DATA_TERMSLIDINGRULE, ISNOTNULL).get());
	// strTerms daycount
	LAString strTenorDayCount = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM_DAYCOUNT).toUpper();
	info->add(PRICING_DATA_TERMDAYCOUNT, new LAPriceDataDayCount()).convertFromString(strTenorDayCount);	
	//// create market param
	//// strTerms
	LAStringVector strTerms = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_TERM).toUpper().toToken(MULTI_STATIC_DATA_DELIMITER);
	info->add(IR_CALIBRATION_DATA_OPTIONMATURITY, new LADataStrings(strTerms));
	unsigned int termSize = strTerms.size();
	// get fx vol file
	LAString volFile = LAMarketData::getNumFileName(mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VOLATILITY_FILE));


	//hishida vannavolga get volatility for base strikes
	unsigned int strikeSize = baseStrikes.size();
	if (strikeSize != 3)
		throw LACoreInvalidData("Vanna Volga method needs 3 strikes",__FILE__,__LINE__);
	//check atm strikes
	if (baseStrikes.end() == std::find(baseStrikes.begin(), baseStrikes.end(), "ATM"))
		throw LACoreInvalidData("Vanna Volga method needs ATM strikes",__FILE__,__LINE__);

	LAStringVector calVec(termSize);         // fixingcalendar
	LAStringVector slidingVec(termSize);     // slidingrule
	LAStringVector soptOrFwdVec(termSize);   // strike
	//DoubleMatrix volMat(termSize, DoubleVector(strikeSize,0.0));

	DoubleMatrix volMat(strikeSize, DoubleVector(termSize,0.0));
	//get volatlity
	for (unsigned int i = 0; i < baseStrikes.size(); i++)
	{
		LAString keystr = baseStrikes[i] + "_VOL";
		
		for (unsigned int j = 0; j < termSize; j++)
		{
			LAString lterm = strTerms[j];
			lterm.toLower();
			volMat[i][j] = LAMarketData::getVolatilityVal(volFile, lterm, keystr);
		}
	}

	
	// fx volatility matrix
	info->add(PRICING_DATA_FXVOLATILITYMATRIX, new LADataDoubleMatrix(volMat));


	//strangle solver version

	//maturity method
	LAString matumethod = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_MATURITYMETHOD);
	info->add("MaturityMethod", new LADataString()).convertFromString(matumethod.toUpper());

	
	LAMathFXEntity fx_tmp = *LAMarketData::getFXEntity(objPool, "FORWARDRATE");
	unsigned int spotlag = fx_tmp.getSpotLag(fx);
	LAString spotlagday = LAString(static_cast<int>(spotlag)) + "D";

	// deltatype
	LAString strdeltatype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_DELTATYPE);
	strdeltatype.toUpper();
	LAString stratmtype =  mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_VANNAVOLGA_ATMTYPE);
	stratmtype.toUpper();
	

	DateVector matuterms(termSize);
	DateVector deliveryterms(termSize);
	LAStringVector deltatypes(termSize,strdeltatype),atmtypes(termSize,stratmtype);
	for (unsigned int i = 0; i < termSize; i++)
	{	
		LAString lterm = strTerms[i];
		lterm.toLower();

		//hishida vannavolga warning this must be changed in all of sources
		//LAString tmpdate = LAMarketData::convertToMLibTerm(strTerms[i]);
		LAString tmpdate = strTerms[i];
		matuterms[i] = LAMathDateCalculations::getDate(asOfDate,tmpdate,sr,&cal,true);
		deliveryterms[i] = LAMathDateCalculations::getDate(matuterms[i],spotlagday,sr,&cal,true);

		deltatypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_VANNAVOLGA_DELTATYPE, lterm);
		deltatypes[i].toUpper();
		
		atmtypes[i] = getCalibStaticDataValue(keyFX + FX_KEY_CALIB_VANNAVOLGA_ATMTYPE, lterm);
		atmtypes[i].toUpper();
	}
	info->add("MaturityDates", new LADataDates(matuterms));
	info->add("DeliveryDates", new LADataDates(deliveryterms));
	info->add("DeltaTypes", new LADataStrings(deltatypes));
	info->add("ATMTypes", new LADataStrings(atmtypes));

	return CALIBINFONAME;
}


/*!
    @brief get calibration property value

	@param[in] key
	@param[in] grid
	@return LAString value

*/
LAString 
LACalibrationParametersFXVannaVolga::getCalibStaticDataValue(const LAString &key, const LAString &grid)
{
	LAString ret = mpCalibStaticData->getStaticData(key + "." + grid);

	if (ret == MLIB_NO_DATA)
	{
		return mpCalibStaticData->getStaticData(key);
	}
	
	return ret;
}

