//
// LAUpdateStaticDataManager.cpp
// This file was previously called YieldCurveFactory.cpp and before that LACurveSetup.cpp
//

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAUpdateStaticDataManager.h"
#include "LADateScheduleHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "LACurvePricingObject.h"

#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataReference.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LADefinitionsCalibration.h"
#include "LAStaticData.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAFunctionUtilities.h"
#include "LAMathCorrelation.h"
#include "LACompoundingFunc.h"
#include "LAMathFXEntity.h"
#include "LAStaticDataImport.h"
#include "LAMarketData.h"

#include "FolderConfig.h"
#include "CurveCalibrationData.h"
#include "LAUpdateModelObject.h"
#include "CurveBuildDefaults.h"
#include "CurveInstruments.h"
#include "ParameterValidation.h"

#include <memory>
#include <sstream>
#include <boost/algorithm/string.hpp>   // For boost::iequals case insenstive string comparisons

using namespace etrading;

namespace
{
    const LAString LF = "\n";
}

namespace etrading
{
	// ===================== LEGACY - ARBITRAGE FREE CURVE =====================================================================
	void LAUpdateStaticDataManager::SetUpArbFreeCurve(LADataInstance* dataInstance,
												      const LAString& curveID,
												      const LAStringMatrix& generateProp, 
												      const LAStringMatrix& moneyConv,
												      const LAStringMatrix& liborRates, 
												      const LAStringMatrix& liborConv,
												      const LAStringMatrix& swapRates, 
												      const LAStringMatrix& swapConv,
												      const LAStringMatrix& fra3mRates,
												      const LAStringMatrix& fra6mRates,
												      const LAStringMatrix& fraConv,
												      const LAStringMatrix& xccyBasisRates, 
												      const LAStringMatrix& xccyBasisConv,
												      const LAStringMatrix& threeSixRates,
												      const LAStringMatrix& threeSixConv,
												      const LAStringMatrix& futureRates, 
												      const LAStringMatrix& futureConv,
												      const LAStringMatrix& adjustData,
												      const LAString& curveNames_3ML,
												      const LAString& curveNames_6ML,
												      const LAString& curveNames_DF,
												      const LAString& curveName_DF2)
    {
	    LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	    LAObjectPool &objPool = dataInstance->getObjectPool();
	    LAObject* pyld = NULL;	
	
	    LAStringMatrix tmpInfo = generateProp;
	    upper(tmpInfo);
	    LADate asofdate	= stringToDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	    LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {
		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; tmpCurrency.toLower();

	    LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	    LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (useMarkets == AQ_NO_DATA) useMarkets = "";
	    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
	    {
		    if (useMarkets == "") useMarkets = SWAP;
		    else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
	    }
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT);

        MLIB_2D_MATRIX_CHECK( generateProp, "Invalid Curve Properties or generateProp Conventions" )
	    for(size_t i=0; i<generateProp.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		    key.toLower();
		    LAString data = generateProp[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	
	    MLIB_2D_MATRIX_CHECK( moneyConv, "Invalid Money Market Conventions" )
        for(size_t i=0; i<moneyConv.size(); i++)
        {
		    LAString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0];
		    key.toLower(); 
		    LAString data = moneyConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	
        MLIB_2D_MATRIX_CHECK( liborConv, "Invalid Libor Fixing and Reset Conventions" )
	    for(size_t i=0; i<liborConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.libor." + liborConv[i][0];
		    key.toLower(); 
		    LAString data = liborConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

        MLIB_2D_MATRIX_CHECK( swapConv, "Invalid Swap Conventions" )
	    for(size_t i=0; i<swapConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0];
		    key.toLower(); 
		    LAString data = swapConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

        MLIB_2D_MATRIX_CHECK( fraConv, "Invalid FRA Conventions" )
	    for(size_t i=0; i<fraConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.fra." + fraConv[i][0];
		    key.toLower(); 
		    LAString data = fraConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

        MLIB_2D_MATRIX_CHECK( xccyBasisConv, "Invalid Xccy Basis Conventions" )
	    for(size_t i=0; i<xccyBasisConv.size(); i++)
	    {
		    LAString key;
		    LAString data = xccyBasisConv[i][1];
		    if (tmpCurrency == "usd")
		    {
			    key = tmpCurrency + ".sde.yield.basis." + xccyBasisConv[i][0];
		    }
		    else
		    {
			    key = tmpCurrency + ".sde.yield.basis." + xccyBasisConv[i][0] + ".xccybasis";
			    if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
			    tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
			    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),XCCYBASIS)) 
			    {
				    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + XCCYBASIS;
			    }
		    }
		    key.toLower(); 
		    irStaticData.setStaticData(key, data);
	    }
	    if (tmpCurrency != "usd") irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + ".xccybasis", MARKETTYPE_BASIS);

        MLIB_2D_MATRIX_CHECK( threeSixConv, "Invalid 3X6 Tenor Basis Conventions" )
	    for(size_t i=0; i<threeSixConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.basis." + threeSixConv[i][0] + ".3m6mbasis";
		    key.toLower(); 
		    LAString data = threeSixConv[i][1];
		    if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + ".3m6mbasis", MARKETTYPE_BASIS);

        MLIB_2D_MATRIX_CHECK( futureConv, "Invalid Futures Conventions" )
		for(size_t i=0; i<futureConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.future." + futureConv[i][0];
		    key.toLower(); 
		    LAString data = futureConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    LAString usegrid_libor = "",usegrid_swap = "",usegrid_fra3m = "",usegrid_fra6m = "",usegrid_xccy = "",
		    usegrid_3m6m = "",usegrid_future = "";

	    //set Libor Object;
	    LAString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE);
	    if (liborfile == AQ_NO_DATA)
	    {
		    liborfile = LAString("data/in/") + tmpCurrency + LAString("_yield_libor.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE, liborfile);
	    }
	    LAString liborstream;
        MLIB_2D_MATRIX_CHECK( liborRates, "Invalid Libor Fixing Rate Conventions" )
	    for(size_t i=0; i<liborRates.size(); i++)
	    {
		    liborstream += liborRates[i][0];
		    double lrate = liborRates[i][1].getDoubleValue() * 100.0;
		    liborstream += "," + LAString(lrate) + LF;

		    if (liborRates[i].size() == 3)
		    {
			    LAString useGridFrag = liborRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") usegrid_libor += liborRates[i][0] + ":";
			    else usegrid_libor += "NONE:";
		    }
	    }
	    std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);

	    //set Swap Object;
	    LAString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE);
	    if (swapfile == AQ_NO_DATA)
	    {
		    swapfile = LAString("data/in/") + tmpCurrency + LAString("_yield_swap.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE, swapfile);
	    }
	    LAString swapstream;
        MLIB_2D_MATRIX_CHECK( swapRates, "Invalid Swap Rates" )
	    for(size_t i=0; i<swapRates.size(); i++)
	    {
		    swapstream += swapRates[i][0];
		    double srate = swapRates[i][1].getDoubleValue() * 100.0;
		    swapstream += "," + LAString(srate) + LF;

		    if (swapRates[i].size() == 3)
		    {
			    LAString useGridFrag = swapRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") usegrid_swap += swapRates[i][0] + ":";
			    else usegrid_swap += "NONE:";
		    }
	    }
	    std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);

	    //set fra3m Object;
	    bool isFRAUse = false;
	    LAString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE);
	    LADataBool tmpAttrB;
	    if (isFRAUse_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFRAUse_str);
		    isFRAUse = tmpAttrB.get();
	    }
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE);
	    if (isFRAUse)
	    {
            AQ_REQUIRE( fraConv.size() > 0, "FRA Conventions Block Missing - Must provide FRA convention settings when IsFRAUse = True" )
            AQ_REQUIRE( fra3mRates.size() > 0 || fra6mRates.size() > 0 , "FRA Rates Block Missing - Must provide FRA instrument data when IsFRAUse = True" )

		    LAString fra3mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE);
		    if (fra3mfile == AQ_NO_DATA)
		    {
			    fra3mfile = LAString("data/in/") + tmpCurrency + LAString("_yield_3mfra.csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_FILE, fra3mfile);
		    }
		    LAString fra3mstream;
            MLIB_2D_MATRIX_CHECK( fra3mRates, "Invalid 3M FRA Rates" )
		    for(size_t i=0; i<fra3mRates.size(); i++)
		    {
			    fra3mstream += fra3mRates[i][0];
			    double frarate = fra3mRates[i][1].getDoubleValue() * 100.0;
			    fra3mstream += "," + LAString(frarate) + LF;

			    if (fra3mRates[i].size() == 3)
			    {
				    LAString useGridFrag = fra3mRates[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fra3m += fra3mRates[i][0] + ":";
				    else usegrid_fra3m += "NONE:";
			    }
		    }
		    std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fra3mfile), pfra3mstream);		

		    //set fra6m Object;
		    LAString fra6mfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE);
		    if (fra6mfile == AQ_NO_DATA)
		    {
			    fra6mfile = LAString("data/in/") + tmpCurrency + LAString("_yield_6mfra.csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_FILE, fra6mfile);
		    }
		    LAString fra6mstream;
            MLIB_2D_MATRIX_CHECK( fra6mRates, "Invalid 6M FRA Rates" )
		    for(size_t i=0; i<fra6mRates.size(); i++)
		    {
			    fra6mstream += fra6mRates[i][0];
			    double frarate = fra6mRates[i][1].getDoubleValue() * 100.0;
			    fra6mstream += "," + LAString(frarate) + LF;

			    if (fra6mRates[i].size() == 3)
			    {
				    LAString useGridFrag = fra6mRates[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fra6m += fra6mRates[i][0] + ":";
				    else usegrid_fra6m += "NONE:";
			    }
		    }
		    std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fra6mfile), pfra6mstream);
	    }

	    ///Xccy Basis Rate
	    LAString xccyBasisFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "xccybasis");
	    if (xccyBasisFile == AQ_NO_DATA)
	    {
		    xccyBasisFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap_xccybasis.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "xccybasis", xccyBasisFile);
	    }
	    LAString xccyBasisStream;
        MLIB_2D_MATRIX_CHECK( xccyBasisRates, "Invalid Xccy Basis Rates" )
	    for(size_t i=0; i<xccyBasisRates.size(); i++)
	    {
		    xccyBasisStream += xccyBasisRates[i][0];
		    double brate = xccyBasisRates[i][1].getDoubleValue() * 10000.0;
		    xccyBasisStream += "," + LAString(brate) + LF;

		    if (xccyBasisRates[i].size() == 3)
		    {
			    LAString useGridFrag = xccyBasisRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") usegrid_xccy += xccyBasisRates[i][0] + ":";
			    else usegrid_xccy += "NONE:";
		    }
	    }
	    std::istringstream *pXccyBasisStream = new std::istringstream(xccyBasisStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(xccyBasisFile), pXccyBasisStream);

	    ///36 Basis Rate
	    LAString threeSixFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "3m6mbasis");
	    if (threeSixFile == AQ_NO_DATA)
	    {
		    threeSixFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap_3m6mbasis.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + "3m6mbasis", threeSixFile);
	    }
	    LAString threeSixStream;
        MLIB_2D_MATRIX_CHECK( threeSixRates, "Invalid 3X6 Tenor Basis Rates" )
	    for(size_t i=0; i<threeSixRates.size(); i++)
	    {
		    threeSixStream += threeSixRates[i][0];
		    double brate = threeSixRates[i][1].getDoubleValue() * 10000.0;
		    threeSixStream += "," + LAString(brate) + LF;

		    if (threeSixRates[i].size() == 3)
		    {
			    LAString useGridFrag = threeSixRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") usegrid_3m6m += threeSixRates[i][0] + ":";
			    else usegrid_3m6m += "NONE:";
		    }
	    }
	    std::istringstream *pThreeSixStream = new std::istringstream(threeSixStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(threeSixFile), pThreeSixStream);

	    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),THREESIXBASIS) && threeSixRates.size())
	    {
		    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + THREESIXBASIS;
	    }

	    //set future Object;
	    bool isFutureUse = false;
	    LAString tmpFutureStr = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE);
	    if (tmpFutureStr != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(tmpFutureStr);
		    isFutureUse = tmpAttrB.get();
	    }
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE);
	    if (isFutureUse)
	    {
            AQ_REQUIRE( futureConv.size() > 0, "Futures Conventions Block Missing - Must provide futures convention settings when IsFutureUse = True" )
            AQ_REQUIRE( futureRates.size() > 0, "Futures Rates Block Missing - Must provide futures instrument data when IsFutureUse = True" )

		    LAString futureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE);
		    if (futureFile == AQ_NO_DATA)
		    {
			    futureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_future") + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE, futureFile);
		    }
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(futureFile), createFutureStream(futureRates, usegrid_future));
	    }
	
	    //set Adjust Data Object;
	    LAString adjustValueFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + "xccybasis");
	    if (adjustValueFile == AQ_NO_DATA)
	    {
		    adjustValueFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisadjust.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + "xccybasis", adjustValueFile);
	    }
	    LAString adjustValueStream;
	    MLIB_2D_MATRIX_CHECK( adjustData, "Invalid Basis Swap Adjustment or adjustData Rates")
	    for(size_t i=0; i<adjustData.size(); i++)
	    {
		    adjustValueStream += adjustData[i][0];
		    adjustValueStream += "," + adjustData[i][1] + LF;	
	    }
	    std::istringstream *pAdjustValueStream = new std::istringstream(adjustValueStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjustValueFile), pAdjustValueStream);


	    ///////////////
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "3m6mbasis");
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "xccybasis");
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID);
	    int find;
	    if (usegrid_libor != "") 
	    {
		    usegrid_libor = usegrid_libor.subString(0, usegrid_libor.size() - 2);
		    while ((find = usegrid_libor.findString("NONE:")) != -1) usegrid_libor.remove(find, 5);
		    if ((find = usegrid_libor.findString(":NONE")) != -1) usegrid_libor.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID, usegrid_libor);
	    }
	    if (usegrid_swap != "") 
	    {
		    usegrid_swap = usegrid_swap.subString(0, usegrid_swap.size() - 2);
		    while ((find = usegrid_swap.findString("NONE:")) != -1) usegrid_swap.remove(find, 5);
		    if ((find = usegrid_swap.findString(":NONE")) != -1) usegrid_swap.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID, usegrid_swap);
	    }
	    if (usegrid_fra3m != "") 
	    {
		    usegrid_fra3m = usegrid_fra3m.subString(0, usegrid_fra3m.size() - 2);
		    while ((find = usegrid_fra3m.findString("NONE:")) != -1) usegrid_fra3m.remove(find, 5);
		    if ((find = usegrid_fra3m.findString(":NONE")) != -1) usegrid_fra3m.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID, usegrid_fra3m);
	    }
	    if (usegrid_fra6m != "") 
	    {
		    usegrid_fra6m = usegrid_fra6m.subString(0, usegrid_fra6m.size() - 2);
		    while ((find = usegrid_fra6m.findString("NONE:")) != -1) usegrid_fra6m.remove(find, 5);
		    if ((find = usegrid_fra6m.findString(":NONE")) != -1) usegrid_fra6m.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID, usegrid_fra6m);
	    }
	    if (usegrid_3m6m != "") 
	    {
		    usegrid_3m6m = usegrid_3m6m.subString(0, usegrid_3m6m.size() - 2);
		    while ((find = usegrid_3m6m.findString("NONE:")) != -1) usegrid_3m6m.remove(find, 5);
		    if ((find = usegrid_3m6m.findString(":NONE")) != -1) usegrid_3m6m.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "3m6mbasis", usegrid_3m6m);
	    }
	    if (usegrid_xccy != "") 
	    {
		    usegrid_xccy = usegrid_xccy.subString(0, usegrid_xccy.size() - 2);
		    while ((find = usegrid_xccy.findString("NONE:")) != -1) usegrid_xccy.remove(find, 5);
		    if ((find = usegrid_xccy.findString(":NONE")) != -1) usegrid_xccy.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + "xccybasis", usegrid_xccy);
	    }
	    if (usegrid_future != "") 
	    {
		    usegrid_future = usegrid_future.subString(0, usegrid_future.size() - 2);
		    while ((find = usegrid_future.findString("NONE:")) != -1) usegrid_future.remove(find, 5);
		    if ((find = usegrid_future.findString(":NONE")) != -1) usegrid_future.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID, usegrid_future);
	    }

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, CURVETYPE_ARBFREE);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE, "TRUE");

	    if (curveNames_3ML == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME, CURVENAME_3ML);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MLCURVENAME, curveNames_3ML);
	    }
	    if (curveNames_6ML == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME, CURVENAME_6ML);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MLCURVENAME, curveNames_6ML);
	    }
	    if (curveNames_DF == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME, CURVENAME_DF);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DFCURVENAME, curveNames_DF);
	    }
	
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	    if (curveName_DF2 != "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2, curveName_DF2);
	    }

        // ============================== 2.  CURVE CALIBRATION ========================================================
        
		
		std::unique_ptr<LAObjectPoolBase> objectPoolManager( new UpdateObjectPoolForSDEsAndCurves(currency) );
	    objectPoolManager->loadModelDataAndCalibrate(currency, *dataInstance, true, false);
        
		
		// ============================== 3. CURVE OBJECT DATA ===========+=========================================

		// Set Curve Build TimeStamp
          LAString	CurveIDTool			= curveID + TOOL;
	    LAString	CurveIDManager		= curveID + CURVETYPE_ARBFREE + MANAGER;

	    // Curve Object Manager (curve, grids, curve information)
	    LAObject* laCurveObject = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    laCurveObject = new LAObject;
		    objPool.set(CurveIDManager,laCurveObject);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    laCurveObject = &objPool.getObject(CurveIDManager).get();
	    }
	    laCurveObject->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	    laCurveObject->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);
    };

	
	// ===================== LEGACY FLOATER CURVE ================================================================================================
	void LAUpdateStaticDataManager::setUpFloater( LADataInstance* dataInstance,
												  const LAString& curveID,
												  const LAString& discountCurveName,
												  const LAString& forecastCurveName,
												  const LAStringMatrix& generateProp, 
												  const LAStringMatrix& basisMkt, 
												  const LAStringMatrix& basisConv,
												  const LAStringMatrix& swapConv,
												  const LAStringMatrix& adjustData )
    {
		// ============================== 1. CURVE STATIC DATA =====================================================
		
		LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	    LAObjectPool &objPool = dataInstance->getObjectPool();

	    LAStringMatrix tmpInfo = generateProp;
	    upper(tmpInfo);
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {
		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; tmpCurrency.toLower();
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);

	    //insert property
	    LAStringMatrix tmpProp(basisConv.size() + swapConv.size() + generateProp.size(), LAStringVector(2));
	    LAString tmpBasisCurveName = XCCYBASIS; tmpBasisCurveName.toLower();
	    MLIB_2D_MATRIX_CHECK( basisConv, "Invalid Basis Swap Conventions" )
        for(size_t i=0; i<basisConv.size(); i++)
	    {		
		    LAString key = tmpCurrency + ".sde.yield.basis." + basisConv[i][0] + "." + tmpBasisCurveName;
		    key.toLower();
		    LAString data = basisConv[i][1];
		    if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		    irStaticData.setStaticData(key,data);
	    }

        MLIB_2D_MATRIX_CHECK( swapConv, "Invalid Swap Conventions" )
	    for(size_t i=0; i<swapConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0];
		    key.toLower(); 
		    LAString data = swapConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key,data);
	    }

        MLIB_2D_MATRIX_CHECK( generateProp, "Invalid Curve Properties or generateProp Conventions" )
	    for(size_t i=0; i<generateProp.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0];
		    key.toLower();
		    LAString data = generateProp[i][1];
		    data.toUpper();
		    irStaticData.setStaticData(key,data);
	    }

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_DISCOUNT, discountCurveName);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_FORECAST, forecastCurveName);

	    if (!basisMkt.empty())
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FLOATER_BASISNAME, XCCYBASIS);

		    LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
		    LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		    if (useMarkets == AQ_NO_DATA) useMarkets = "";
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),XCCYBASIS)) 
		    {
			    if (useMarkets == "") useMarkets = XCCYBASIS;
			    else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + XCCYBASIS;
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
		
		    LAString tmpBasisCurveName = XCCYBASIS;
		    tmpBasisCurveName.toLower();

		    LAString basisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName);
		    if (basisfile == AQ_NO_DATA)
		    {
			    basisfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap_xccybasis.csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName, basisfile);
		    }
		    LAString basisstream;
		    LAString usegrid = "";

            MLIB_2D_MATRIX_CHECK( basisMkt, "Invalid Basis Market Rates" )
		    for(size_t i=0; i<basisMkt.size(); i++)
		    {
			    basisstream += basisMkt[i][0];
			    double brate = basisMkt[i][1].getDoubleValue() * 10000.0;
			    basisstream += "," + LAString(brate) + LF;

			    if (basisMkt[i].size() == 3)
			    {
				    LAString useGridFrag = basisMkt[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid += basisMkt[i][0] + ":";
				    else usegrid += "NONE:";
			    }
		    }
		    std::istringstream *pbasisstream = new std::istringstream(basisstream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(basisfile), pbasisstream);

		    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName);
		    int find;
		    if (usegrid != "") 
		    {
			    usegrid = usegrid.subString(0, usegrid.size() - 2);
			    while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
			    if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName, usegrid);
		    }

		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + tmpBasisCurveName, MARKETTYPE_BASIS);

		    //set Adjust Data Object;
		    if (adjustData.size() > 0)
		    {
			    LAString adjustValueFile = irStaticData.getStaticData(tmpCurrency + 
				    STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + tmpBasisCurveName);
			    if (adjustValueFile == AQ_NO_DATA)
			    {
				    adjustValueFile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisadjust.csv");
				    irStaticData.setStaticData(tmpCurrency + 
					    STATIC_DATA_KEY_YIELD_BASIS_ADJUSTVALUE_FILE + "." + tmpBasisCurveName, adjustValueFile);
			    }
			    LAString adjustValueStream;
                MLIB_2D_MATRIX_CHECK( adjustData, "Invalid Basis Swap Adjustment Data" )
			    for(size_t i=0; i<adjustData.size(); i++)
			    {
				    adjustValueStream += adjustData[i][0];
				    adjustValueStream += "," + adjustData[i][1] + LF;	
			    }
			    std::istringstream *pAdjustValueStream = new std::istringstream(adjustValueStream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjustValueFile), pAdjustValueStream);
		    }
	    }

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, CURVETYPE_FLOATER);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATEFLOATERS, CURVETYPE_FLOATER);
	    

        // ============================== 2. CURVE CALIBRATION ========================================================

		
		std::unique_ptr<LAObjectPoolBase> objectPoolManager( new UpdateObjectPoolForSDEsAndCurves(currency) );
	    objectPoolManager->loadModelDataAndCalibrate(currency, *dataInstance, true, false);


		// ============================== 3. CURVE OBJECT DATA =====================================================


	    // Set the Curve Build TimeStamp
	    LAString	CurveIDTool			= curveID + TOOL;
	    LAString	CurveIDManager		= curveID + CURVETYPE_FLOATER+ MANAGER;
	    
        // Curve Object Manager (curve, grids, curve information)
	    LAObject* mae = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    mae = new LAObject;
		    objPool.set(CurveIDManager,mae);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    mae	= &objPool.getObject(CurveIDManager).get();
	    }
	    mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	    mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);
    }


	// ========================== OIS CURVE ===========================================================================================
	void LAUpdateStaticDataManager::loadStaticDataOISCurve( LADataInstance* dataInstance,
															const LAString& curveID,
															const LAString& marketName,
															const LAStringMatrix& generateProp,
															const LAStringMatrix& oisRates,
															const LAStringMatrix& oisConv,
															const LAString& curveNames,
															const LAStringMatrix& histRates,
															const LAStringMatrix& lobasisRates,
															const LAStringMatrix& lobasisConv,
															const LAStringMatrix& swapRates,
															const LAStringMatrix& swapConv )
	{
		LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
		
	    LAObjectPool &objPool = dataInstance->getObjectPool();
	    LAObject* pyld = NULL;
	
	    LAStringMatrix tmpInfo = generateProp;
	    upper(tmpInfo);
	    LADate asofdate	= stringToDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	    LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);

	    if (objHolder.isDefined())
	    {

		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; tmpCurrency.toLower();

	    if (marketName == "" || marketName == STD) 
		    throw LACoreInvalidData("Do not use STD or blank for ois curve name!",__FILE__,__LINE__);
	    LAString staticDataSuffix;
	    LAString suffix_data;
	    staticDataSuffix = "." + marketName;
	    staticDataSuffix.toLower();
	    LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	    LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (useMarkets == AQ_NO_DATA) useMarkets = "";
	    LAString tmpCurveName = marketName; tmpCurveName.toUpper();
	    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
	    {
		    if (useMarkets == "") useMarkets = marketName;
		    else useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + marketName;
	    }
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
	    if (curveNames == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, marketName);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	    }
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + staticDataSuffix, "DAILYCOMPOUNDING");

	    // Set curve type
		LAString curveType = CURVETYPE_OIS;

		// If CurveType is specified, get it from user
		if (LAFunctionUtilities::findRowsNumber(tmpInfo, CURVEINPUT_CURVETYPE) >= 0)
		{
			curveType = chgrow(tmpInfo, CURVEINPUT_CURVETYPE, 1);
			AQ_REQUIRE(curveType == CURVETYPE_OIS || curveType == CURVETYPE_ARR, "CurveType can only be OIS or ARR.");
		}

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + staticDataSuffix, curveType);

	    suffix_data = "_" + marketName;
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix);

	    MLIB_2D_MATRIX_CHECK( generateProp, "Invalid Curve Properties or generateProp Conventions")
	    for(size_t i=0; i<generateProp.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = generateProp[i][1];
		    if (key.findString("dfcurvename") == -1) data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

        // *** Important *** Must remove keys that can accept blank values, since these will not be removed by default
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + staticDataSuffix );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + staticDataSuffix );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + staticDataSuffix );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD + staticDataSuffix );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD + staticDataSuffix );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD + staticDataSuffix );
		irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE + staticDataSuffix );

        // Check if we need Libor-OIS and Swap Static and Market Data, which is required if the 'longtermconvention' parameter in the OISConvention 'oisConv' block is set to 'lobasis'
        // Pay attention to the use of lower case here.
        bool requireLiborOisAndSwapData = false;

        MLIB_2D_MATRIX_CHECK( oisConv, "Invalid OIS Conventions")	
	    for(size_t i=0; i<oisConv.size(); i++)
	    {
            LAString oisConventionKey = oisConv[i][0];

            LAString key = tmpCurrency + ".sde.yield.ois." + oisConventionKey + staticDataSuffix;
		    key.toLower(); 
		    
            LAString data = oisConv[i][1];
		    data.toLower();
		    
            irStaticData.setStaticData(key, data);

            // Check if we need Libor-OIS and Swap Static and Market Data, which is required if the 'longtermconvention' parameter in the OISConvention 'oisConv' block is set to 'lobasis'
            if( boost::iequals( oisConventionKey.c_str(), "longtermconvention") && boost::iequals( data.c_str(), "lobasis" ) )
            {
                requireLiborOisAndSwapData = true;
            }
	    }	

	    LAString oisFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + staticDataSuffix);
	    if (oisFile == AQ_NO_DATA)
	    {
		    oisFile = LAString("data/in/") + tmpCurrency + LAString("_yield_ois_oiscurve.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + staticDataSuffix, oisFile);
	    }

	    LAString fedFundFutureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + staticDataSuffix);
	    if (fedFundFutureFile == AQ_NO_DATA)
	    {
		    fedFundFutureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_fffuture.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + staticDataSuffix, fedFundFutureFile);
	    }

	    LAString histFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + staticDataSuffix);
	    if (histFile == AQ_NO_DATA)
	    {
		    histFile = LAString("data/in/") + tmpCurrency + LAString("_yield_historical_ois_oiscurve.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + staticDataSuffix, histFile);
	    }

	    LAString oisStream;
	    LAString fedFundFutureStream;
	    LAString usegrid_ois = "";
	    LAString usegrid_fffuture = "";
        MLIB_2D_MATRIX_CHECK( oisRates, "Invalid OIS Swap Rates" )
	    for(size_t i=0; i<oisRates.size(); i++)
	    {
		    if (oisRates[i][0].findString("FF") != -1)
		    {
			    fedFundFutureStream += oisRates[i][0];

			    if (oisRates[i].size() != 3 && oisRates[i].size() != 5)
				    throw LACoreInvalidData("FF input size error",__FILE__,__LINE__);

			    //in case of FF, quoted value is price
			    double oisRate = oisRates[i][1].getDoubleValue();
			    fedFundFutureStream += "," + LAString(oisRate);

			    if (oisRates[i].size() == 5)
			    {
				    const LADate& startdate = stringToDate(oisRates[i][2]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    fedFundFutureStream += "," + startdate_str;
				    const LADate& enddate = stringToDate(oisRates[i][3]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    fedFundFutureStream += "," + enddate_str;
			    }

			    fedFundFutureStream += LF;
			
			    if (oisRates[i].size() == 5)
			    {
				    LAString useGridFrag = oisRates[i][4]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fffuture += oisRates[i][0] + ":";
				    else usegrid_fffuture += "NONE:";
			    }
			    else if (oisRates[i].size() == 3)
			    {
				    LAString useGridFrag = oisRates[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fffuture += oisRates[i][0] + ":";
				    else usegrid_fffuture += "NONE:";
			    }
		    }
		    else
		    {
			    oisStream += oisRates[i][0];

                // Allow Central Bank Swap instruments to be specified in the OIS curve for all markets ...
                if (isCentralBankSwap(oisRates[i][0]))     
			    {
				    if (oisRates[i].size() < 4)
					    throw LACoreInvalidData("#Error: short term market needs StartDate and EndDate",__FILE__,__LINE__);

				    double oisRate = oisRates[i][1].getDoubleValue() * 100.0;
				    oisStream += "," + LAString(oisRate);

				    const LADate& startdate = stringToDate(oisRates[i][2]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    oisStream += "," + startdate_str;
				    const LADate& enddate = stringToDate(oisRates[i][3]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    oisStream += "," + enddate_str;
			    }
				// ARR curve's future part
				else if (isFuture(oisRates[i][0]))
				{
					const int NUM_COLUMNS_FUTURE = 5; // Term, Rate, StartDate, EndDate, VolOrConvAdj
					if (oisRates[i].size() < NUM_COLUMNS_FUTURE)
					{
						throw LACoreInvalidData("#Error: future part needs VolOrConvexAdj", __FILE__, __LINE__);
					}

					double futurePrice = oisRates[i][1].getDoubleValue();
					oisStream += "," + LAString(futurePrice);

					const LADate& startdate = stringToDate(oisRates[i][2]);
					LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
					oisStream += "," + startdate_str;

					const LADate& enddate = stringToDate(oisRates[i][3]);
					LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
					oisStream += "," + enddate_str;

					double volOrConvAdj = oisRates[i][4].getDoubleValue();
					oisStream += "," + LAString(volOrConvAdj);

				}
			    else // Outright Swaps
			    {
					// If a futures start- or end date is provided in the oisMatrix column 2 and 3 then this is not an outright swap.
					// Futures and MPC swaps need the term to be specified with a FUTURE or MPC prefix.
					if( ( oisRates[i].size() > 3 ) && ( oisRates[i][2].size() > 0 || oisRates[i][3].size() > 0 ) )
					{
						LAString msg = "#Error: Invalid Term: Instrument " + LAString( int(i) + 1 ) + " must have FUTURE or MPC term prefix, since futures start- and/or end-date provided";
						throw LACoreInvalidData( msg.c_str(), __FILE__, __LINE__ );
					}

				    double oisRate = oisRates[i][1].getDoubleValue() * 100.0;
				    oisStream += "," + LAString(oisRate);
			    }
			
			    oisStream += LF;
			
			    if (oisRates[i].size() == 5)
			    {
				    LAString useGridFrag = oisRates[i][4]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_ois += oisRates[i][0] + ":";
				    else usegrid_ois += "NONE:";
			    }
			    else if (oisRates[i].size() == 3)
			    {
				    LAString useGridFrag = oisRates[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_ois += oisRates[i][0] + ":";
				    else usegrid_ois += "NONE:";
			    }
		    }
	    }
 	    std::istringstream *pOISStream = new std::istringstream(oisStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(oisFile), pOISStream);
	    std::istringstream *pFedFundFutureStream = new std::istringstream(fedFundFutureStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fedFundFutureFile), pFedFundFutureStream);


	    ///////////////
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + staticDataSuffix);
	    int find;
	    if (usegrid_ois != "") 
	    {
		    usegrid_ois = usegrid_ois.subString(0, usegrid_ois.size() - 2);
		    while ((find = usegrid_ois.findString("NONE:")) != -1) usegrid_ois.remove(find, 5);
		    if ((find = usegrid_ois.findString(":NONE")) != -1) usegrid_ois.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + staticDataSuffix, usegrid_ois);
	    }
	    if (usegrid_fffuture != "") 
	    {
		    usegrid_fffuture = usegrid_fffuture.subString(0, usegrid_fffuture.size() - 2);
		    while ((find = usegrid_fffuture.findString("NONE:")) != -1) usegrid_fffuture.remove(find, 5);
		    if ((find = usegrid_fffuture.findString(":NONE")) != -1) usegrid_fffuture.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + staticDataSuffix, usegrid_fffuture);
	    }

	    LAString histStream;
	    MLIB_2D_MATRIX_CHECK( histRates, "Invalid OIS Fixing and Reset Rates")
	    for(size_t i=0; i<histRates.size(); i++)
	    {
		    const LADate& histdate = stringToDate(histRates[i][0]);
		    LAString histdate_str = histdate.stringWithFormat("YYYYMMDD");
		    histStream += histdate_str;
		    double histRate = histRates[i][1].getDoubleValue() * 100.0;
		    histStream += "," + LAString(histRate);
		    histStream += LF;
	    }
	    std::istringstream *pHISTStream = new std::istringstream(histStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(histFile), pHISTStream);

        // Check if Libor-OIS and Swap Data is Required
        if ( requireLiborOisAndSwapData )
        {
            AQ_REQUIRE( lobasisConv.size() > 0, "Missing Static Data: Libor-OIS Basis Conventions are Required when the OIS 'LongTermConvention' parameter is set to use 'LOBASIS'" )
        }

        MLIB_2D_MATRIX_CHECK( lobasisConv, "Invalid Libor-OIS Basis Conventions")
	    for(size_t i=0; i<lobasisConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.basis." + lobasisConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = lobasisConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }	
	    LAString lobasisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + staticDataSuffix);
	    if (lobasisfile == AQ_NO_DATA)
	    {
		    lobasisfile = LAString("data/in/") + tmpCurrency + LAString("_yield_lobasis") + staticDataSuffix + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + staticDataSuffix, lobasisfile);
	    }

        // Check if Libor-OIS and Swap Data is Required
        if ( requireLiborOisAndSwapData )
        {
            AQ_REQUIRE( lobasisRates.size() > 0, "Missing Market Data: Libor-OIS Basis Rates are Required when the OIS 'LongTermConvention' parameter is set to use 'LOBASIS'" )
        }

	    LAString lobasisstream;
	    MLIB_2D_MATRIX_CHECK( lobasisRates, "Invalid Libor-OIS Basis Rates")
		const size_t lobasisColumnSize = ( lobasisRates.size() > 0 ) ? lobasisRates[0].size() : 0;

		for(size_t i=0; i<lobasisRates.size(); i++)
	    {
			// Libor-OIS Rates Columns: Term, Rate, LiborType (Optional), UseMarketData (Optional)
			lobasisstream += lobasisRates[i][0];
		    const double lobrate = lobasisRates[i][1].getDoubleValue() * 100.0;
		    lobasisstream += "," + LAString(lobrate);
			
			// Optional Column 3: LiborType
			if ( lobasisColumnSize > 2 )
			{
				const LAString liborType = lobasisRates[i][2];
				lobasisstream += "," + liborType;
			}

			// Optional Column 4: UseMarketData
			if ( lobasisColumnSize > 3 )
			{
				const LAString useMarketData = lobasisRates[i][3];
				lobasisstream += "," + useMarketData;
			}

			lobasisstream += LF;
	    }
	    std::istringstream *plobasisstream = new std::istringstream(lobasisstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(lobasisfile), plobasisstream);

        // Check if Libor-OIS and Swap Data is Required
        if ( requireLiborOisAndSwapData )
        {
            AQ_REQUIRE( swapConv.size() > 0, "Missing Static Data: Swap Conventions are Required when the OIS 'LongTermConvention' parameter is set to use 'LOBASIS'" )
        }

	    MLIB_2D_MATRIX_CHECK( swapConv, "Invalid Swap Conventions")
	    for(size_t i=0; i<swapConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = swapConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }	
	    LAString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	    if (swapfile == AQ_NO_DATA)
	    {
		    swapfile = LAString("data/in/") + tmpCurrency + LAString("_yield_swap") + staticDataSuffix + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix, swapfile);
	    }

        // Check if Libor-OIS and Swap Data is Required
        if ( requireLiborOisAndSwapData )
        {
            AQ_REQUIRE( swapRates.size() > 0, "Missing Market Data: Swap Rates are Required when OIS 'LongTermConvention' parameter is set to use 'LOBASIS'" )
        }

	    LAString swapstream;
	    MLIB_2D_MATRIX_CHECK( swapRates, "Invalid Swap Rates")
	    for(size_t i=0; i<swapRates.size(); i++)
	    {
			//Swap Tenor
			swapstream += swapRates[i][0];
			//Swap Rate
			double srate = swapRates[i][1].getDoubleValue() * 100.0;
			swapstream += "," + LAString(srate);

			if (swapRates[i].size() > 2)
			{
				//BasisSwapType
				swapstream += "," + swapRates[i][2];
			}
			
			swapstream += LF;

		}
	    std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);
	

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_SWAP);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	}

	// ========================== OIS CURVE ===========================================================================================
	void LAUpdateStaticDataManager::calibrateOISCurve( LADataInstance* dataInstance,
													   const LAString& curveID,
													   const LAString& marketName,
													   const LAStringMatrix& generateProp )
	{
		// 1. CURVE CALIBRATION
		// *********************
        
		LabelValueBlock curvePropertiesLVB( generateProp );
		const LAString currency = curvePropertiesLVB.getCompulsoryValueAsLAString( IRS_KEY::CURRENCY, "CurvePropertiesLVB" );
		
		std::unique_ptr<LAObjectPoolBase> objectPoolManager( new UpdateObjectPoolForSDEsAndCurves( currency) );
	    objectPoolManager->loadModelDataAndCalibrate( currency, *dataInstance, true, false, curveID, marketName );


        // 2. UPDATE CURVE OBJECT MANAGER
		// ******************************
        		
		// Object Pool
		LAObjectPool &objPool = dataInstance->getObjectPool();

	    // Set the Curve Build Timestamp
	    LAString	CurveIDTool			= curveID + TOOL;
	    LAString	CurveIDManager		= curveID + marketName + MANAGER;
	    
        // LA Curve Object Build Manager
	    LAObject* laCurveObject = NULL;
	    if( !objPool.getObject(CurveIDManager).isDefined() )
	    {	
		    laCurveObject = new LAObject;
		    objPool.set( CurveIDManager, laCurveObject );
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    laCurveObject = &objPool.getObject(CurveIDManager).get();
	    }
	    laCurveObject->add("Time",			      new LADataString() ).convertFromString(LAString(LATime::now()));
	    laCurveObject->add(CALIBRATION_DATA_NAME, new LADataString() ).convertFromString(CurveIDManager);

	}

	// ========================== OIS CURVE ===========================================================================================
	void LAUpdateStaticDataManager::setUpOISCurve( LADataInstance* dataInstance,
												   const LAString& curveID,
                                                   const LAString& marketName,
                                                   const LAStringMatrix& generateProp, 
                                                   const LAStringMatrix& oisRates, 
                                                   const LAStringMatrix& oisConv,
                                                   const LAString& curveNames,
                                                   const LAStringMatrix& histRates,
                                                   const LAStringMatrix& lobasisRates, 
                                                   const LAStringMatrix& lobasisConv, 
                                                   const LAStringMatrix& swapRates, 
                                                   const LAStringMatrix& swapConv )
    {
		// 1. Curve Static Data
		loadStaticDataOISCurve( dataInstance, curveID, marketName, generateProp, oisRates, oisConv,
                                curveNames, histRates, lobasisRates, lobasisConv, swapRates, swapConv );
	    
		// 2. Calibrate Curve
		calibrateOISCurve( dataInstance, curveID, marketName, generateProp );
    };


	// ============================== SWAP CURVE =======================================================================================
	void LAUpdateStaticDataManager::loadStaticDataSwapCurve( LADataInstance* dataInstance,
															 const LAString& curveID,
															 const LAString& marketName,
															 const LAStringMatrix& generateProp,
															 const LAStringMatrix& moneyConv,
															 const LAStringMatrix& liborRates,
															 const LAStringMatrix& liborConv,
															 const LAStringMatrix& swapRates,
															 const LAStringMatrix& swapConv,
															 const LAStringMatrix& fra3mRates,
															 const LAStringMatrix& fra6mRates,
															 const LAStringMatrix& fraConv,
															 const LAStringMatrix& futureRates,
															 const LAStringMatrix& futureConv,
															 const LAStringMatrix& adjustSwapConv,
															 const LAStringMatrix& adjustSwapRates,
															 const LAString& curveNames,
															 const LAString& curveName_DF2 )
	{
		LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
		
	    LAObjectPool &objPool = dataInstance->getObjectPool();
	    LAObject* pyld = NULL;

	    LAStringMatrix tmpInfo = generateProp;
	    upper(tmpInfo);
	    LADate asofdate	= stringToDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	    LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {

		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; tmpCurrency.toLower();

	    LAString generateCurveName = (marketName == "") ? STD : marketName;
	    generateCurveName.toUpper();
	    LAString staticDataSuffix;
	    LAString suffix_data;

	    // The useMarkets string is a list of default market names for a particular currency in the form 
	    // of '1M3MBasis:3M6MBasis:SWAP:XCCYBasis' for example. This is loaded directly from the ir.properties file
	    LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	    LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (useMarkets == AQ_NO_DATA) 
	    {
		    useMarkets = "";
	    }

	    // If the user-given market name is not STD, concatenate it to useMarkets 
	    if (generateCurveName != STD)
	    {
		    staticDataSuffix = "." + generateCurveName;
		    staticDataSuffix.toLower();
		    LAString tmpCurveName = generateCurveName; 
		    tmpCurveName.toUpper();
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		    {
			    if (useMarkets == "") 
			    {
				    useMarkets = generateCurveName;
			    }
			    else 
			    {
				    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + generateCurveName;
			    }
		    }	
	    }
	    else
	    {	
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
		    {
			    if (useMarkets == "") 
			    {
				    useMarkets = SWAP;
			    }
			    else 
			    {
				    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
			    }
		    }
	    }
	
	    if (curveNames == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, marketName);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	    }

	    // Set curve type
	    LAString curveType = CURVETYPE_SWAP;
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + staticDataSuffix, curveType);

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2);
	    if (curveName_DF2 != "" && marketName == STD)
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_DF2, curveName_DF2);
	    }

	    if (generateCurveName != STD)
	    {
		    suffix_data = "_" + generateCurveName;
	    }
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + staticDataSuffix);

	    bool isswaptenoradjust = false;
	    MLIB_2D_MATRIX_CHECK( generateProp, "Invalid CurveProperties or generateProp Conventions" )
	    for(size_t i=0; i<generateProp.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = generateProp[i][1];
		    if (key.findString("dfcurvename") == -1) 
		    {
			    data.toLower();
		    }
		    irStaticData.setStaticData(key, data);

		    LAString tmpProp = generateProp[i][0];
		    tmpProp.toLower();
		    if (tmpProp == "isswaptenoradjust")
		    {
			    key = generateProp[i][1];
			    isswaptenoradjust = (key.toUpper() == "TRUE");
		    }
	    }

	    MLIB_2D_MATRIX_CHECK( moneyConv, "Invalid Money Market Conventions" )	
	    for(size_t i=0; i<moneyConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = moneyConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	
	    MLIB_2D_MATRIX_CHECK( liborConv, "Invalid Libor Fixing and Reset Conventions" )
	    for(size_t i=0; i<liborConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.libor." + liborConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = liborConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( swapConv, "Invalid Swap Conventions" )
	    for(size_t i=0; i<swapConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.swap." + swapConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = swapConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( fraConv, "Invalid FRA Conventions" )
	    for(size_t i=0; i<fraConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.fra." + fraConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = fraConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( futureConv, "Invalid Future Conventions" )
	    for(size_t i=0; i<futureConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.future." + futureConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = futureConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    //////////////////modify for aud curve///////////////////////////////////////////////

	    //swap adjust mode
	    LAString tmpBasisCurveName;
	    if (isswaptenoradjust)
	    {
            AQ_REQUIRE( adjustSwapConv.size() > 0, "Adjustment Basis Swap Conventions Block Missing - Must provide Basis Swap convention settings when isSwapTenorAdjust = True" )
            AQ_REQUIRE( adjustSwapRates.size() > 0, "Adjustment Basis Swap Rates Block Missing - Must provide Basis Swap rates when isSwapTenorAdjust = True" )
        
		    tmpBasisCurveName = THREESIXBASIS;
		    LAString adjsuffix_prop;
		    LAString adjsuffix_data;
		    if (tmpBasisCurveName != STD)
		    {
			    adjsuffix_prop = "." + tmpBasisCurveName;
			    adjsuffix_prop.toLower();
			    adjsuffix_data = "_" + tmpBasisCurveName;
			    adjsuffix_data.toLower();
		    }

		    MLIB_2D_MATRIX_CHECK( adjustSwapConv, "Invalid Basis Swap Adjustment or adjustSwapConv Conventions")
		    for(size_t i=0; i<adjustSwapConv.size(); i++)
		    {
			    LAString key = tmpCurrency + ".sde.yield.basis." + adjustSwapConv[i][0] + adjsuffix_prop;
			    key.toLower(); 
			    LAString data = adjustSwapConv[i][1];
			    if (key.findString("discount") == -1 && key.findString("forecast") == -1) 
			    {
				    data.toLower();
			    }
			    irStaticData.setStaticData(key, data);
		    }
	
		    tmpBasisCurveName.toLower();
		    LAString basisEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName;

		    //swapfile
		    LAString adjfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + adjsuffix_prop);
		    if (adjfile == AQ_NO_DATA)
		    {
			    adjfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap") + adjsuffix_data + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + adjsuffix_prop, adjfile);
		    }

		    LAString adjstream;
		    LAString usegrid = "";
            MLIB_2D_MATRIX_CHECK( adjustSwapRates, "Invalid Basis Swap Adjustment or adjustSwapRates Rates" )
		    for(size_t i=0; i<adjustSwapRates.size(); i++)
		    {
			    adjstream += adjustSwapRates[i][0];
			    double adjrate = adjustSwapRates[i][1].getDoubleValue() * 10000.0;
			    adjstream += "," + LAString(adjrate) + LF;

			    if (adjustSwapRates[i].size() == 3)
			    {
				    LAString useGridFrag = adjustSwapRates[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid += adjustSwapRates[i][0] + ":";
				    else usegrid += "NONE:";
			    }
		    }
		    std::istringstream *padjstream = new std::istringstream(adjstream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjfile), padjstream);

		    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName);
		    int find;
		    if (usegrid != "") 
		    {
			    usegrid = usegrid.subString(0, usegrid.size() - 2);
			    while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
			    if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName, usegrid);
		    }

		    LAString tmpCurveName = tmpBasisCurveName; tmpCurveName.toUpper();
		    tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		    { 
			    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + tmpBasisCurveName;
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + tmpBasisCurveName, MARKETTYPE_BASIS);

		    //////////////////modify for aud curve///////////////////////////////////////////////
	    }

	    LAString usegrid_libor = "",usegrid_swap = "",usegrid_fra = "",usegrid_future = "";

	    //set Libor Object;
	    LAString liborEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix;

	    LAString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
	    if (liborfile == AQ_NO_DATA)
	    {
		    liborfile = LAString("data/in/") + tmpCurrency + LAString("_yield_libor") + suffix_data + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix, liborfile);
	    }
	    LAString liborstream;
        MLIB_2D_MATRIX_CHECK( liborRates, "Invalid Libor Fixing Rates" )
	    for(size_t i=0; i<liborRates.size(); i++)
	    {
		    liborstream += liborRates[i][0];
		    double lrate = liborRates[i][1].getDoubleValue() * 100.0;
		    liborstream += "," + LAString(lrate) + LF;

		    if (liborRates[i].size() == 3)
		    {
			    LAString useGridFrag = liborRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") 
			    {
				    usegrid_libor += liborRates[i][0] + ":";
			    }
			    else 
			    {
				    usegrid_libor += "NONE:";
			    }
		    }
	    }
	    std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);

	    //set fwd swap
	    bool areSwapsForwardStarting = false;
	    LAString isFwdSwap_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
	    LADataBool tmpAttrB;
	    if (isFwdSwap_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFwdSwap_str);
		    areSwapsForwardStarting = tmpAttrB.get();
	    }

	    //set Swap Object;
	    LAString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix);
	    if (swapfile == AQ_NO_DATA)
	    {
		    swapfile = LAString("data/in/") + tmpCurrency + LAString("_yield_swap") + suffix_data + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + staticDataSuffix, swapfile);
	    }
	    LAString swapstream;
        MLIB_2D_MATRIX_CHECK( swapRates, "Invalid Swap Rates" )
	    for(size_t i=0; i<swapRates.size(); i++)
	    {
		    swapstream += swapRates[i][0];
		    double srate = swapRates[i][1].getDoubleValue() * 100.0;
		    swapstream += "," + LAString(srate);
		    // for fwd swap
		    if (areSwapsForwardStarting)
		    {
			    if (swapRates[i].size() < 5)
			    {
                    throw LACoreInvalidData("FwdSwap size error",__FILE__,__LINE__);
			    }

			    LAString isDate_str = swapRates[i][2];
			    swapstream += "," + isDate_str.toUpper();
			    if (isDate_str == "TRUE") 
			    {
				    const LADate& startdate = stringToDate(swapRates[i][3]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    swapstream += "," + startdate_str;
				    const LADate& enddate = stringToDate(swapRates[i][4]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    swapstream += "," + enddate_str;
			    }
			    else
			    {
				    LAString startterm_str = swapRates[i][3];
				    swapstream += "," + startterm_str.toUpper();
				    LAString tenor_str = swapRates[i][4];
				    swapstream += "," + tenor_str.toUpper();
			    }
			    // set use grid
			    if (swapRates[i].size() == 6)
			    {
				    LAString useGridFrag = swapRates[i][5]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") 
				    {
					    usegrid_swap += swapRates[i][0] + ":";
				    }
				    else 
				    {
					    usegrid_swap += "NONE:";
				    }
			    }
		    }
		    // set use grid
		    else if (swapRates[i].size() == 3)
		    {
			    LAString useGridFrag = swapRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") 
			    {
				    usegrid_swap += swapRates[i][0] + ":";
			    }
			    else 
			    {
				    usegrid_swap += "NONE:";
			    }
		    }
		    swapstream += LF;
	    }
	    std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);

	    //set FRA Object;
	    bool isFRAUse = false;
	    LAString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	    if (isFRAUse_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFRAUse_str);
		    isFRAUse = tmpAttrB.get();
	    }
	
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix);

	    if (isFRAUse)
	    {
            AQ_REQUIRE( fraConv.size() > 0, "FRA Conventions Block Missing - Must provide FRA convention settings when IsFRAUse = True" )
            AQ_REQUIRE( fra3mRates.size() > 0 || fra6mRates.size() > 0 , "FRA Rates Block Missing - Must provide FRA instrument data when IsFRAUse = True" )

		    LAString fraFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix);
		    if (fraFile == AQ_NO_DATA)
		    {
			    fraFile = LAString("data/in/") + tmpCurrency + LAString("_yield_fra") + suffix_data + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix, fraFile);
		    }

		    if (fra3mRates.size() != 0)
		    {
			    LAString fra3mstream = etrading::buildFRAMarketDataFile(fraFile, fra3mRates, areSwapsForwardStarting, usegrid_fra);	

			    std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfra3mstream);
		    }
		    else if (fra6mRates.size() != 0)
		    {
			    LAString fra6mstream = etrading::buildFRAMarketDataFile(fraFile, fra6mRates, areSwapsForwardStarting, usegrid_fra);

			    std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfra6mstream);
		    }
		    else
		    {
			    throw LACoreInvalidData("#Error - No FRA market data is provided",__FILE__,__LINE__);
		    }
	    }

	    //set future Object;
	    bool isFutureUse = false;
	    LAString tmpFutureStr = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + staticDataSuffix);
	    if (tmpFutureStr != AQ_NO_DATA)
	    {
		    LADataBool tmpAttrB;
		    tmpAttrB.convertFromString(tmpFutureStr);
		    isFutureUse = tmpAttrB.get();
	    }
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
	    if (isFutureUse)
	    {
            AQ_REQUIRE( futureConv.size() > 0, "Futures Conventions Block Missing - Must provide futures convention settings when IsFutureUse = True" )
            AQ_REQUIRE( futureRates.size() > 0, "Futures Rates Block Missing - Must provide futures instrument data when IsFutureUse = True" )

		    LAString futureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix);
		    if (futureFile == AQ_NO_DATA)
		    {
			    futureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_future") + suffix_data + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + staticDataSuffix, futureFile);
		    }
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(futureFile), createFutureStream(futureRates, usegrid_future));
	    }

	    ///////////////
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + staticDataSuffix);
	    int find;
	    if (usegrid_libor != "") 
	    {
		    usegrid_libor = usegrid_libor.subString(0, usegrid_libor.size() - 2);
		    while ((find = usegrid_libor.findString("NONE:")) != -1) 
		    {
			    usegrid_libor.remove(find, 5);
		    }

		    if ((find = usegrid_libor.findString(":NONE")) != -1) 
		    {
			    usegrid_libor.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix, usegrid_libor);
	    }
	    if (usegrid_swap != "") 
	    {
		    usegrid_swap = usegrid_swap.subString(0, usegrid_swap.size() - 2);
		    while ((find = usegrid_swap.findString("NONE:")) != -1) 
		    {
			    usegrid_swap.remove(find, 5);
		    }

		    if ((find = usegrid_swap.findString(":NONE")) != -1) 
		    {
			    usegrid_swap.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + staticDataSuffix, usegrid_swap);
	    }
	    if (usegrid_fra != "") 
	    {
		    usegrid_fra = usegrid_fra.subString(0, usegrid_fra.size() - 2);
		    while ((find = usegrid_fra.findString("NONE:")) != -1) 
		    {
			    usegrid_fra.remove(find, 5);
		    }

		    if ((find = usegrid_fra.findString(":NONE")) != -1) 
		    {
			    usegrid_fra.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + staticDataSuffix, usegrid_fra);
	    }
	    if (usegrid_future != "") 
	    {
		    usegrid_future = usegrid_future.subString(0, usegrid_future.size() - 2);
		    while ((find = usegrid_future.findString("NONE:")) != -1) 
		    {
			    usegrid_future.remove(find, 5);
		    }

		    if ((find = usegrid_future.findString(":NONE")) != -1) 
		    {
			    usegrid_future.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + staticDataSuffix, usegrid_future);
	    }

	    LAString tmpCurveName = generateCurveName; tmpCurveName.toUpper();
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);
	}

	// ============================== SWAP CURVE =======================================================================================
	void LAUpdateStaticDataManager::calibrateSwapCurve( LADataInstance* dataInstance,
														const LAString& curveID,
														const LAString& marketName,
														const LAStringMatrix& generateProp )
	{
		// 1. Calibrate Curve
		// *********************
        
		LabelValueBlock curvePropertiesLVB( generateProp );
		const LAString currency = curvePropertiesLVB.getCompulsoryValueAsLAString( IRS_KEY::CURRENCY, "CurvePropertiesLVB" );
		
		std::unique_ptr<LAObjectPoolBase> objectPoolManager( new UpdateObjectPoolForSDEsAndCurves(currency) );
	    objectPoolManager->loadModelDataAndCalibrate(currency, *dataInstance, true, false, curveID, marketName);
        
		
		// 2. Update Curve Object Data
		// *****************************

		// Get the object pool
		LAObjectPool &objPool = dataInstance->getObjectPool();

	    // Set Curve Build TimeStamp
        LAString CurveIDTool = curveID + TOOL;
	    LAString CurveIDManager	= curveID + marketName + MANAGER;

	    // Curve Object Manager (curve, grids, curve information)
	    LAObject* laCurveObject = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    laCurveObject = new LAObject;
		    objPool.set(CurveIDManager,laCurveObject);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    laCurveObject	= &objPool.getObject(CurveIDManager).get();
	    }
	    laCurveObject->add("Time",					new LADataString()	).convertFromString(LAString(LATime::now()));
	    laCurveObject->add(CALIBRATION_DATA_NAME,	new LADataString()	).convertFromString(CurveIDManager);	
	}


	// ============================== SWAP CURVE =======================================================================================
	void LAUpdateStaticDataManager::setUpSwapCurve(LADataInstance* dataInstance,
											        const LAString& curveID,
											        const LAString& marketName,
											        const LAStringMatrix& generateProp, 
											        const LAStringMatrix& moneyConv,
											        const LAStringMatrix& liborRates, 
											        const LAStringMatrix& liborConv,
											        const LAStringMatrix& swapRates, 
											        const LAStringMatrix& swapConv,
											        const LAStringMatrix& fra3mRates,
											        const LAStringMatrix& fra6mRates,
											        const LAStringMatrix& fraConv,
											        const LAStringMatrix& futureRates, 
											        const LAStringMatrix& futureConv,
											        const LAStringMatrix& adjustSwapConv,
											        const LAStringMatrix& adjustSwapRates,
											        const LAString& curveNames,
											        const LAString& curveName_DF2)
    {
		// 1. Load Static Data
		loadStaticDataSwapCurve( dataInstance, curveID, marketName, generateProp, moneyConv, liborRates, liborConv,
								 swapRates, swapConv, fra3mRates, fra6mRates, fraConv,futureRates, futureConv,
								 adjustSwapConv, adjustSwapRates, curveNames, curveName_DF2 );
		
		// 2. Calibrate Curve
		calibrateSwapCurve( dataInstance, curveID, marketName, generateProp );
        
	};


	// ============================== BASIS CURVE =======================================================================================
	void LAUpdateStaticDataManager::loadStaticDataBasisCurve( LADataInstance* dataInstance,
															  const LAString& curveID,
															  const LAString& marketName,
															  const LAStringMatrix& basisRates,
															  const LAStringMatrix& basisConv,
															  const LAStringMatrix& fwdFXs,
															  const LAStringMatrix& fwdConv,
															  const LAStringMatrix& spotFXs,
															  const LAStringMatrix& generateProp,
															  const LAStringMatrix& moneyConv,
															  const LAString& curveNames,
															  const LAStringMatrix& fraConv,
															  const LAStringMatrix& fraRates,
															  const LAStringMatrix& liborConv,
															  const LAStringMatrix& liborRates )
	{
		LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();

	    LAObjectPool &objPool = dataInstance->getObjectPool();

	    LAStringMatrix tmpInfo = generateProp;
	    upper(tmpInfo);
	    LADate asofdate = stringToDate( chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1) );
        LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {

		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; tmpCurrency.toLower();
	
	    //set market rate	
	    if (marketName == "" || marketName == STD) 
	    {
		    throw LACoreInvalidData("Do not use STD or blank for basis curve name!",__FILE__,__LINE__);
	    }

	    LAString staticDataSuffix;
	    LAString suffix_data;
	    staticDataSuffix = "." + marketName;
	    staticDataSuffix.toLower();
	    suffix_data = "_" + marketName;
	    suffix_data.toLower();

	    // Clear certain control parameters first
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FIXINGSOURCE + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDBASIS + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + staticDataSuffix);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + staticDataSuffix);

	    LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	    LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (useMarkets == AQ_NO_DATA) 
	    {
		    useMarkets = "";
	    }
	    LAString tmpCurveName = marketName; 
	    tmpCurveName.toUpper();
	
	    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
	    {
		    if (useMarkets == "") 
		    {
			    useMarkets = marketName;
		    }
		    else 
		    {
			    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + marketName;
		    }
	    }
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);

	    if (curveNames == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, marketName);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	    }

	    LAString curveType = CURVETYPE_BASIS;
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + staticDataSuffix, curveType);
	
	    // By default, set the "isfwdinter" property using false.
	    // If the user has specified the property in the basisConv info, allow the user setting to override
	    // this default in the following "basis info" code block.
	    {
		    LAString key = tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISFWDINTER + staticDataSuffix;
		    key.toLower();
		    LAString data = "false";
		    irStaticData.setStaticData( key, data );
	    }

	    //basis info
	    MLIB_2D_MATRIX_CHECK( basisConv, "Invalid Basis Conventions" )
        for(size_t i=0; i<basisConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.basis." + basisConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = basisConv[i][1];
		    if (key.findString("discount") == -1 && key.findString("forecast") == -1)
		    {
			    data.toLower();
		    }
		    irStaticData.setStaticData(key,data);
	    }
	    //generator info
	    MLIB_2D_MATRIX_CHECK( generateProp, "Invalid Curve Properties" )
	    for(size_t i=0; i<generateProp.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = generateProp[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key,data);
	    }
	    //money info
	    MLIB_2D_MATRIX_CHECK( moneyConv, "Invalid Money Market Conventions" )
	    for(size_t i=0; i<moneyConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = moneyConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key,data);
	    }
	    //FRA info
	    MLIB_2D_MATRIX_CHECK( fraConv, "Invalid FRA Conventions" )
	    for(size_t i=0; i<fraConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.fra." + fraConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = fraConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	    //fwdfx info
	    MLIB_2D_MATRIX_CHECK( fwdConv, "Invalid Forward FX Conventions" )
	    for(size_t i=0; i<fwdConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.basis.fwdfx." + fwdConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = fwdConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key,data);
	    }
	    // Libor market convention
	    MLIB_2D_MATRIX_CHECK( liborConv, "Invalid Libor Conventions" )
	    for(size_t i=0; i<liborConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.libor." + liborConv[i][0] + staticDataSuffix;
		    key.toLower(); 
		    LAString data = liborConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	
	    //set Libor data file
	    LAString liborUseGrid = "";
	    LAString isLiborProvided_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FIXINGSOURCE + staticDataSuffix);
	    if (isLiborProvided_str != AQ_NO_DATA)
	    {
		    if (isLiborProvided_str.toUpper() == ITSELF)
		    {
			    LAString liborEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix;

			    LAString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
			    if (liborfile == AQ_NO_DATA)
			    {
				    liborfile = LAString("data/in/") + tmpCurrency + LAString("_yield_libor") + suffix_data + LAString(".csv");
				    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix, liborfile);
			    }
			    LAString liborstream;
                MLIB_2D_MATRIX_CHECK( liborRates, "Invalid Libor Fixing or Reset Rates" )
			    for(size_t i=0; i<liborRates.size(); i++)
			    {
				    liborstream += liborRates[i][0];
				    double lrate = liborRates[i][1].getDoubleValue() * 100.0;
				    liborstream += "," + LAString(lrate) + LF;

				    if (liborRates[i].size() == 3)
				    {
					    LAString useGridFrag = liborRates[i][2]; upper(useGridFrag);
					    if (useGridFrag == "TRUE") 
					    {
						    liborUseGrid += liborRates[i][0] + ":";
					    }
					    else 
					    {
						    liborUseGrid += "NONE:";
					    }
				    }
			    }
			    std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);
		    }
	    }

	    //set FRA file
	    bool isFRAUse = false;
	    LAString fraUseGrid = "";
	    LAString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	    LADataBool tmpAttrB;
	    if (isFRAUse_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFRAUse_str);
		    isFRAUse = tmpAttrB.get();
	    }

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix);
	    if (isFRAUse)
	    {
            AQ_REQUIRE( fraConv.size() > 0, "FRA Conventions Block Missing - Must provide FRA convention settings when IsFRAUse = True" )
            AQ_REQUIRE( fraRates.size() > 0, "FRA Rates Block Missing - Must provide FRA instrument data when IsFRAUse = True" )

		    //set fwd swap
		    bool areSwapsForwardStarting = false;
		    LAString isFwdSwap_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
		    if (isFwdSwap_str != AQ_NO_DATA)
		    {
			    tmpAttrB.convertFromString(isFwdSwap_str);
			    areSwapsForwardStarting = tmpAttrB.get();
		    }

		    LAString fraFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix);
		    if (fraFile == AQ_NO_DATA)
		    {
			    fraFile = LAString("data/in/") + tmpCurrency + LAString("_yield_fra") + suffix_data + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix, fraFile);
		    }
		
		    LAString fraStream = etrading::buildFRAMarketDataFile(fraFile, fraRates, areSwapsForwardStarting, fraUseGrid);	
		    std::istringstream *pfraStream = new std::istringstream(fraStream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfraStream);
	    }

	    //set fwd basis
	    bool isFwdBasis = false;
	    LAString isFwdBasis_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDBASIS + staticDataSuffix);
	    if (isFwdBasis_str.toUpper() == "TRUE")
	    {
		    isFwdBasis = true;
	    }

	    //basis file
	    LAString basisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix);
	    if (basisfile == AQ_NO_DATA)
	    {
		    basisfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap") + suffix_data + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix, basisfile);
	    }
	    LAString basisstream;
	    LAString usegrid = "";
	    MLIB_2D_MATRIX_CHECK( basisRates, "Invalid Basis Rates" )
	    for (unsigned int i = 0; i < basisRates.size();i++)
	    {
		    basisstream += basisRates[i][0];
		    const double brate = basisRates[i][1].getDoubleValue() * 10000.0;
		    basisstream += "," + LAString(brate);
		    // for fwd basis
		    if (isFwdBasis)
		    {
			    if (basisRates[i].size() < 5)
			    {
                    throw LACoreInvalidData("#Error: Invalid Tenor Basis market data, wrong number of columns. Market Data should consist of 5 columns specifiying forward dates when the 'isFwdBasis' flag is set to true.",__FILE__,__LINE__);
			    }

			    LAString isDate_str = basisRates[i][2];
			    basisstream += "," + isDate_str.toUpper();
			    if (isDate_str == "TRUE") 
			    {
				    const LADate& startdate = stringToDate(basisRates[i][3]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    basisstream += "," + startdate_str;
				    const LADate& enddate = stringToDate(basisRates[i][4]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    basisstream += "," + enddate_str;
			    }
			    else
			    {
				    LAString startterm_str = basisRates[i][3];
				    basisstream += "," + startterm_str.toUpper();
				    LAString tenor_str = basisRates[i][4];
				    basisstream += "," + tenor_str.toUpper();
			    }
			    // set use grid
			    if (basisRates[i].size() == 6)
			    {
				    LAString useGridFrag = basisRates[i][5]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") 
				    {
					    usegrid += basisRates[i][0] + ":";
				    }
				    else 
				    {
					    usegrid += "NONE:";
				    }
			    }
		    }
		    else if (basisRates[i].size() == 3)
		    {
			    LAString useGridFrag = basisRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") 
			    {
				    usegrid += basisRates[i][0] + ":";
			    }
			    else 
			    {
				    usegrid += "NONE:";
			    }
		    }
		    basisstream += LF;
	    }
	    std::istringstream *pbasisstream = new std::istringstream(basisstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(basisfile), pbasisstream);

	    //fwdfx file
	    LAString fwdfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + staticDataSuffix);
	    LAString fwdstream;
	    LAString fwdusegrid = "";
	    MLIB_2D_MATRIX_CHECK( fwdFXs, "Invalid Forward FXs" )
	    for (unsigned int i = 0; i < fwdFXs.size();i++)
	    {
		    fwdstream += fwdFXs[i][0];
		    const double fwdfx = fwdFXs[i][1].getDoubleValue();
		    fwdstream += "," + LAString(fwdfx) + LF;

		    if (fwdFXs[i].size() == 3)
		    {
			    LAString useGridFrag = fwdFXs[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") 
			    {
				    fwdusegrid += fwdFXs[i][0] + ":";
			    }
			    else 
			    {
				    fwdusegrid += "NONE:";
			    }
		    }
	    }
	    std::istringstream *pfwdstream = new std::istringstream(fwdstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fwdfile), pfwdstream);

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	    // Set up UseGrids
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix);
	    int find;
	    if (liborUseGrid != "") 
	    {
		    liborUseGrid = liborUseGrid.subString(0, liborUseGrid.size() - 2);
		    while ((find = liborUseGrid.findString("NONE:")) != -1) 
		    {
			    liborUseGrid.remove(find, 5);
		    }

		    if ((find = liborUseGrid.findString(":NONE")) != -1) 
		    {
			    liborUseGrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix, liborUseGrid);
	    }

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + staticDataSuffix);
	    if (fraUseGrid != "")		// FRA UseGrid
	    {
		    fraUseGrid = fraUseGrid.subString(0, fraUseGrid.size() - 2);
		    while ((find = fraUseGrid.findString("NONE:")) != -1) 
		    {
			    fraUseGrid.remove(find, 5);
		    }

		    if ((find = fraUseGrid.findString(":NONE")) != -1) 
		    {
			    fraUseGrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + staticDataSuffix, fraUseGrid);
	    }

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + staticDataSuffix);	
	    if (usegrid != "")			// Basis UseGrid
	    {
		    usegrid = usegrid.subString(0, usegrid.size() - 2);
		    while ((find = usegrid.findString("NONE:")) != -1) 
		    {
			    usegrid.remove(find, 5);
		    }

		    if ((find = usegrid.findString(":NONE")) != -1) 
		    {
			    usegrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + staticDataSuffix, usegrid);
	    }

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + staticDataSuffix);
	    if (fwdusegrid != "")		// Forward UseGrid
	    {
		    fwdusegrid = fwdusegrid.subString(0, fwdusegrid.size() - 2);
		    while ((find = fwdusegrid.findString("NONE:")) != -1) 
		    {
			    fwdusegrid.remove(find, 5);
		    }

		    if ((find = fwdusegrid.findString(":NONE")) != -1) 
		    {
			    fwdusegrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + staticDataSuffix, fwdusegrid);
	    }

	    //set fxentity
	    LAMathFXEntity* pFwd = NULL;
	    LAObjectHolder ehfx = objPool.getObject(FORWARDFX, ENCHKTYPE_NOCHECK);
	    if (!ehfx.isDefined())
	    {
		    pFwd = new LAMathFXEntity(dataInstance);
		    objPool.set(FORWARDFX,pFwd);
	    }
	    else
	    {
		    dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get()).reset();
		    pFwd = &dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get());
	    }
	    pFwd->getName().convertFromString(FORWARDFX);
	    pFwd->getFXType().convertFromString("FORWARDRATE");
	    LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, FORWARDFX);
	    const int ccySize = spotFXs.size();
	    LAStringVector ccys(ccySize);
	    DoubleVector spotrates(ccySize);
	    LAString unitccy = irStaticData.getStaticData(KEY_FXSPOTRATES_UNITCCY);
	    unitccy.toUpper();
	    for(unsigned int i = 0; i < (size_t)ccySize; ++i)
	    {
            if (spotFXs[i].size() < 3)
		    {
			    throw LACoreInvalidData("spotrate file size error", __FILE__, __LINE__);
		    }
		    LAString tmpccy = spotFXs[i][0];
		    /*if (tmpccy.toUpper() != unitccy)
		    {
			    throw LACoreInvalidData("The first column of a spot rate file must be unit currency", __FILE__, __LINE__);
		    }*/
		    ccys[i] = spotFXs[i][1];
		    ccys[i].toUpper();
		    spotrates[i] = spotFXs[i][2].getDoubleValue();
	    }
        pFwd->getCurrencys().set(ccys);
	    pFwd->getSpotRates().set(spotrates);

	    //set generate df
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_BASIS);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	}
	

	// ============================== BASIS CURVE =======================================================================================
	void LAUpdateStaticDataManager::calibrateBasisCurve( LADataInstance* dataInstance,
														 const LAString& curveID,
														 const LAString& marketName,
														 const LAStringMatrix& generateProp )
	{
		// 1. Curve Calibration
		// ***********************
	    
		LabelValueBlock curvePropertiesLVB( generateProp );
		const LAString currency = curvePropertiesLVB.getCompulsoryValueAsLAString( IRS_KEY::CURRENCY, "CurvePropertiesLVB" );

		std::unique_ptr<LAObjectPoolBase> objectPoolManager( new UpdateObjectPoolForSDEsAndCurves(currency) );
	    objectPoolManager->loadModelDataAndCalibrate(currency, *dataInstance, true, false, curveID, marketName );
        

	    // 2. Update Curve Object Data
		// ****************************

		// Get the object pool
		LAObjectPool &objPool = dataInstance->getObjectPool();

		// Set Curve TimeStamp
	    LAString	CurveIDTool			= curveID + TOOL;
	    LAString	CurveIDManager		= curveID + marketName + MANAGER;
	    
        // Curve Object Build Manager (curve, grids, curve information)
	    LAObject* laCurveObject = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    laCurveObject = new LAObject;
		    objPool.set(CurveIDManager,laCurveObject);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    laCurveObject = &objPool.getObject(CurveIDManager).get();
	    }
	    laCurveObject->add("Time",					new LADataString()	).convertFromString(LAString(LATime::now()));
	    laCurveObject->add(CALIBRATION_DATA_NAME,	new LADataString()	).convertFromString(CurveIDManager);

	}


	// ============================== BASIS CURVE =======================================================================================
	void LAUpdateStaticDataManager::setUpBasisCurve( LADataInstance* dataInstance,
													 const LAString& curveID, 
													 const LAString& marketName, 
													 const LAStringMatrix& basisRates, 
													 const LAStringMatrix& basisConv,
													 const LAStringMatrix& fwdFXs, 
													 const LAStringMatrix& fwdConv,
													 const LAStringMatrix& spotFXs, 
													 const LAStringMatrix& generateProp,
													 const LAStringMatrix& moneyConv,
													 const LAString& curveNames,
													 const LAStringMatrix& fraConv,
													 const LAStringMatrix& fraRates,
													 const LAStringMatrix& liborConv,
													 const LAStringMatrix& liborRates)
    {
		// 1. Load Static Data
		loadStaticDataBasisCurve( dataInstance, curveID, marketName, basisRates, basisConv, fwdFXs, fwdConv,
                                  spotFXs, generateProp, moneyConv, curveNames, fraConv, fraRates,liborConv,
                                  liborRates );
		
		// 2. Calibrate Curve
		calibrateSwapCurve( dataInstance, curveID, marketName, generateProp );
    }


	// =============================== FX FORWARD CURVE ======================================================================================
	void LAUpdateStaticDataManager::loadStaticDataFwdFXConstantCurve(LADataInstance* dataInstance,
																	 const LAString& curveID,
																	 const LAString& marketName,
																	 const LAStringMatrix& fwdfxconstConv,
																	 const LAStringMatrix& generateProp,
																	 const LAString& curveNames)
	{
		LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
		
	    LAObjectPool &objPool = dataInstance->getObjectPool();

	    LAStringMatrix tmpInfo = generateProp;
	    upper(tmpInfo);
	    LADate asofdate = stringToDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
        LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {

		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; tmpCurrency.toLower();
	
	    //set market rate	
	    if (marketName == "" || marketName == STD) 
		    throw LACoreInvalidData("Do not use STD or blank for basis curve name!",__FILE__,__LINE__);

	    LAString staticDataSuffix;
	    LAString suffix_data;
	    staticDataSuffix = "." + marketName;
	    staticDataSuffix.toLower();
	    suffix_data = "_" + marketName;
	    suffix_data.toLower();

	    LAString tmpCurveName = marketName; tmpCurveName.toUpper();
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST, "TRUE");
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDFXCONST + staticDataSuffix, "TRUE");
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FWDFXCONST_USEMARKET, marketName);

	    if (curveNames == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, marketName);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	    }
	
	    // Set curve type
	    LAString curveType = CURVETYPE_FWDFXCONST;
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + staticDataSuffix, curveType);

	    // By default, set the "isfwdinter" property using false.
	    // If the user has specified the property in the fwdfxconstConv info, allow the user setting to override
	    // this default in the following "basis info" code block.
	    {
		    LAString key = tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ISFWDINTER + staticDataSuffix;
		    key.toLower();
		    LAString data = "false";
		    irStaticData.setStaticData( key, data );
	    }
	    //basis info
	    MLIB_2D_MATRIX_CHECK( fwdfxconstConv, "Invalid Forward FX Const Conventions" )
	    for(size_t i=0; i<fwdfxconstConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.basis." + fwdfxconstConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = fwdfxconstConv[i][1];
		    if (key.findString("discount") == -1 && key.findString("forecast") == -1) data.toLower();
		    irStaticData.setStaticData(key,data);
	    }

	    MLIB_2D_MATRIX_CHECK( generateProp, "Invalid Curve Properties or generateProp Conventions" )
	    for(size_t i=0; i<generateProp.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = generateProp[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key,data);
	    }

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	    //set generate df
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + staticDataSuffix, MARKETTYPE_BASIS);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);
	}

	
	// =============================== FX FORWARD CURVE ======================================================================================
	void LAUpdateStaticDataManager::calibrateFwdFXConstantCurve( LADataInstance* dataInstance,
														         const LAString& curveID,
														         const LAString& marketName,
														         const LAStringMatrix& generateProp )
	{
		// 1. Curve Calibration
		// ***********************

		LabelValueBlock curvePropertiesLVB( generateProp );
		const LAString currency = curvePropertiesLVB.getCompulsoryValueAsLAString( IRS_KEY::CURRENCY, "CurvePropertiesLVB" );

		LAUpdateCurveObject *entityPoolManager = new UpdateObjectPoolForSDEsAndCurves(currency);
	    entityPoolManager->loadFwdFXConstCurveDataAndCalibrate(currency, *dataInstance, curveID, marketName);
	    delete entityPoolManager;


		// 2. Update Curve Object Data
		// *****************************
	    
		// Get the object pool
		LAObjectPool &objPool = dataInstance->getObjectPool();

		//ylddata
	    LAString	CurveIDTool			= curveID + TOOL;
	    LAString	CurveIDManager		= curveID + marketName + MANAGER;
	    
		//curve entities manager (curve, grids, curveinformation entities)
	    LAObject* mae = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    mae = new LAObject;
		    objPool.set(CurveIDManager,mae);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    mae	= &objPool.getObject(CurveIDManager).get();
	    }
	    mae->add("Time",				new LADataString()	).convertFromString(LAString(LATime::now()));
	    mae->add(CALIBRATION_DATA_NAME,	new LADataString()	).convertFromString(CurveIDManager);
	}


	// =============================== FX FORWARD CURVE ======================================================================================
    void LAUpdateStaticDataManager::setUpFwdFXConstantCurve(LADataInstance* dataInstance,
													        const LAString& curveID, 
													        const LAString& marketName, 
													        const LAStringMatrix& fwdfxconstConv,
													        const LAStringMatrix& generateProp,
													        const LAString& curveNames)
    {
		// 1. Load Static Data
		loadStaticDataFwdFXConstantCurve(dataInstance, curveID, marketName, fwdfxconstConv, generateProp, curveNames);

		// 2. Calibrate Curve
		calibrateFwdFXConstantCurve( dataInstance, curveID, marketName, generateProp );
    }


    void LAUpdateStaticDataManager::populateStaticDataManagerForSwapCurve(LAStaticData &irStaticData,
									    const LAString& useMarkets,
									    const LAString& currency,
									    const LAString& curveNames_swap,
									    const LAString& marketName_swap,
									    const LAString& generateCurveName_swap,
									    const LAStringMatrix& generateProp_swap,
									    const LAStringMatrix& moneyConv_swap,
									    const LAStringMatrix& liborRates_swap,
									    const LAStringMatrix& liborConv_swap,
									    const LAStringMatrix& swapRates_swap,
									    const LAStringMatrix& swapConv_swap,
									    const LAStringMatrix& fra3mRates_swap,
									    const LAStringMatrix& fra6mRates_swap,
									    const LAStringMatrix& fraConv_swap,
									    const LAStringMatrix& futureRates_swap,
									    const LAStringMatrix& futureConv_swap,
									    const LAStringMatrix& adjustSwapConv_swap,
									    const LAStringMatrix& adjustSwapRates_swap)
    {

	    LAString suffix_prop_swap("");
	    LAString suffix_data_swap("");

	    // If the user-given market name is not STD, concatenate it to useMarkets 
	    LAString useMarkets_swap = useMarkets;
	    LAStringVector useMarkets_vector = useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (generateCurveName_swap != STD)
	    {
		    suffix_data_swap = "_" + generateCurveName_swap;

		    suffix_prop_swap = "." + generateCurveName_swap;
		    suffix_prop_swap.toLower();

		    LAString tmpCurveName = generateCurveName_swap;
		    tmpCurveName.toUpper();
		    if (useMarkets_vector.end() == std::find(useMarkets_vector.begin(), useMarkets_vector.end(), tmpCurveName))
		    {
			    if (useMarkets_swap == "")
			    {
				    useMarkets_swap = generateCurveName_swap;
			    }
			    else
			    {
				    useMarkets_swap += LAString(MULTI_STATIC_DATA_DELIMITER) + generateCurveName_swap;
			    }
		    }
	    }
	    else
	    {
		    if (useMarkets_vector.end() == std::find(useMarkets_vector.begin(), useMarkets_vector.end(), SWAP))
		    {
			    if (useMarkets_swap == "")
			    {
				    useMarkets_swap = SWAP;
			    }
			    else
			    {
				    useMarkets_swap += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
			    }
		    }
	    }

	    if (curveNames_swap == "")
	    {
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_swap, marketName_swap);
	    }
	    else
	    {
		    // New code for dual bootstrapping - ensure the implicit curve name is one of the curve index names
		    LAString indexNames = curveNames_swap;
		    if (indexNames.findString(marketName_swap) == -1)
		    {
			    indexNames = marketName_swap + LAString(MULTI_STATIC_DATA_DELIMITER) + indexNames;
		    }

		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_swap, indexNames);
	    }

	    // Set curve type
	    LAString curveType = CURVETYPE_SWAP;
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_swap, curveType);

	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + suffix_prop_swap);

	    bool isswaptenoradjust = false;
	    MLIB_2D_MATRIX_CHECK( generateProp_swap, "Invalid Curve Properties or GenerateProp Conventions" )
	    for (size_t i = 0; i<generateProp_swap.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.generator." + generateProp_swap[i][0] + suffix_prop_swap;
		    key.toLower();
		    LAString data = generateProp_swap[i][1];
		    if (key.findString("dfcurvename") == -1)
		    {
			    data.toLower();
		    }
		    irStaticData.setStaticData(key, data);

		    LAString tmpProp = generateProp_swap[i][0];
		    tmpProp.toLower();
		    if (tmpProp == "isswaptenoradjust")
		    {
			    key = generateProp_swap[i][1];
			    isswaptenoradjust = (key.toUpper() == "TRUE");
		    }
	    }

	    MLIB_2D_MATRIX_CHECK( moneyConv_swap, "Invalid Money Market Conventions" )
	    for (size_t i = 0; i<moneyConv_swap.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.moneymarket." + moneyConv_swap[i][0] + suffix_prop_swap;
		    key.toLower();
		    LAString data = moneyConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( liborConv_swap, "Invalid Libor Fixing Conventions" )
	    for (size_t i = 0; i<liborConv_swap.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.libor." + liborConv_swap[i][0] + suffix_prop_swap;
		    key.toLower();
		    LAString data = liborConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( swapConv_swap, "Invalid Swap Conventions" )
	    for (size_t i = 0; i<swapConv_swap.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.swap." + swapConv_swap[i][0] + suffix_prop_swap;
		    key.toLower();
		    LAString data = swapConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( fraConv_swap, "Invalid FRA Conventions" )
	    for (size_t i = 0; i<fraConv_swap.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.fra." + fraConv_swap[i][0] + suffix_prop_swap;
		    key.toLower();
		    LAString data = fraConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( futureConv_swap, "Invalid Future Conventions" )
	    for (size_t i = 0; i<futureConv_swap.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.future." + futureConv_swap[i][0] + suffix_prop_swap;
		    key.toLower();
		    LAString data = futureConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    //swap adjust mode
	    LAString tmpBasisCurveName;
	    if (isswaptenoradjust)
	    {
            AQ_REQUIRE( adjustSwapConv_swap.size() > 0, "Adjustment Basis Swap Conventions Block Missing - Must provide Basis Swap convention settings when isSwapTenorAdjust = True" )
            AQ_REQUIRE( adjustSwapRates_swap.size() > 0, "Adjustment Basis Swap Rates Block Missing - Must provide Basis Swap rates when isSwapTenorAdjust = True" )

		    tmpBasisCurveName = THREESIXBASIS;
		    LAString adjsuffix_prop;
		    LAString adjsuffix_data;
		    if (tmpBasisCurveName != STD)
		    {
			    adjsuffix_prop = "." + tmpBasisCurveName;
			    adjsuffix_prop.toLower();
			    adjsuffix_data = "_" + tmpBasisCurveName;
			    adjsuffix_data.toLower();
		    }

		    MLIB_2D_MATRIX_CHECK( adjustSwapConv_swap, "Invalid Future Conventions" )
		    for (size_t i = 0; i<adjustSwapConv_swap.size(); i++)
		    {
			    LAString key = currency + ".sde.yield.basis." + adjustSwapConv_swap[i][0] + adjsuffix_prop;
			    key.toLower();
			    LAString data = adjustSwapConv_swap[i][1];
			    if (key.findString("discount") == -1 && key.findString("forecast") == -1)
			    {
				    data.toLower();
			    }
			    irStaticData.setStaticData(key, data);
		    }

		    tmpBasisCurveName.toLower();
		    LAString basisEntityName = currency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName;

		    //swapfile
		    LAString adjfile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FILE + adjsuffix_prop);
		    if (adjfile == AQ_NO_DATA)
		    {
			    adjfile = LAString("data/in/") + currency + LAString("_yield_basisswap") + adjsuffix_data + LAString(".csv");
			    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + adjsuffix_prop, adjfile);
		    }

		    LAString adjstream;
		    LAString usegrid = "";
		    MLIB_2D_MATRIX_CHECK( adjustSwapRates_swap, "Invalid Basis Swap Rates / AdjustSwapRates Conventions" )
		    for (size_t i = 0; i<adjustSwapRates_swap.size(); i++)
		    {
			    adjstream += adjustSwapRates_swap[i][0];
			    double adjrate = adjustSwapRates_swap[i][1].getDoubleValue() * 10000.0;
			    adjstream += "," + LAString(adjrate) + LF;

			    if (adjustSwapRates_swap[i].size() == 3)
			    {
				    LAString useGridFrag = adjustSwapRates_swap[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid += adjustSwapRates_swap[i][0] + ":";
				    else usegrid += "NONE:";
			    }
		    }
		    std::istringstream *padjstream = new std::istringstream(adjstream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjfile), padjstream);

		    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName);
		    int find;
		    if (usegrid != "")
		    {
			    usegrid = usegrid.subString(0, usegrid.size() - 2);
			    while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
			    if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
			    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName, usegrid);
		    }

		    LAString tmpCurveName = tmpBasisCurveName; tmpCurveName.toUpper();
		    useMarkets_vector = useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		    if (useMarkets_vector.end() == std::find(useMarkets_vector.begin(), useMarkets_vector.end(), tmpCurveName))
		    {
			    useMarkets_swap += LAString(MULTI_STATIC_DATA_DELIMITER) + tmpBasisCurveName;
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + tmpBasisCurveName, MARKETTYPE_BASIS);
	    }

	    LAString usegrid_libor = "", usegrid_swap = "", usegrid_fra = "", usegrid_future = "";

	    //set Libor Object;
	    LAString liborEntityName = currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + suffix_prop_swap;

	    LAString liborfile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + suffix_prop_swap);
	    if (liborfile == AQ_NO_DATA)
	    {
		    liborfile = LAString("data/in/") + currency + LAString("_yield_libor") + suffix_data_swap + LAString(".csv");
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + suffix_prop_swap, liborfile);
	    }
	    LAString liborstream;
	    MLIB_2D_MATRIX_CHECK( liborRates_swap, "Invalid Libor Rates" )
	    for (size_t i = 0; i<liborRates_swap.size(); i++)
	    {
		    liborstream += liborRates_swap[i][0];
		    double lrate = liborRates_swap[i][1].getDoubleValue() * 100.0;
		    liborstream += "," + LAString(lrate) + LF;

		    if (liborRates_swap[i].size() == 3)
		    {
			    LAString useGridFrag = liborRates_swap[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE")
			    {
				    usegrid_libor += liborRates_swap[i][0] + ":";
			    }
			    else
			    {
				    usegrid_libor += "NONE:";
			    }
		    }
	    }
	    std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);

	    //set fwd swap
	    bool areSwapsForwardStarting = false;
	    LAString isFwdSwap_str = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + suffix_prop_swap);
	    LADataBool tmpAttrB;
	    if (isFwdSwap_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFwdSwap_str);
		    areSwapsForwardStarting = tmpAttrB.get();
	    }

	    //set Swap Object;
	    LAString swapfile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_prop_swap);
	    if (swapfile == AQ_NO_DATA)
	    {
		    swapfile = LAString("data/in/") + currency + LAString("_yield_swap") + suffix_data_swap + LAString(".csv");
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_prop_swap, swapfile);
	    }
	    LAString swapstream;
	    MLIB_2D_MATRIX_CHECK( swapRates_swap, "Invalid Swap Rates" )
	    for (size_t i = 0; i<swapRates_swap.size(); i++)
	    {
		    swapstream += swapRates_swap[i][0];
		    double srate = swapRates_swap[i][1].getDoubleValue() * 100.0;
		    swapstream += "," + LAString(srate);
		    // for fwd swap
		    if (areSwapsForwardStarting)
		    {
			    if (swapRates_swap[i].size() < 5)
			    {
				    throw LACoreInvalidData("FwdSwap size error", __FILE__, __LINE__);
			    }

			    LAString isDate_str = swapRates_swap[i][2];
			    swapstream += "," + isDate_str.toUpper();
			    if (isDate_str == "TRUE")
			    {
				    const LADate& startdate = stringToDate(swapRates_swap[i][3]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    swapstream += "," + startdate_str;
				    const LADate& enddate = stringToDate(swapRates_swap[i][4]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    swapstream += "," + enddate_str;
			    }
			    else
			    {
				    LAString startterm_str = swapRates_swap[i][3];
				    swapstream += "," + startterm_str.toUpper();
				    LAString tenor_str = swapRates_swap[i][4];
				    swapstream += "," + tenor_str.toUpper();
			    }
			    // set use grid
			    if (swapRates_swap[i].size() == 6)
			    {
				    LAString useGridFrag = swapRates_swap[i][5]; upper(useGridFrag);
				    if (useGridFrag == "TRUE")
				    {
					    usegrid_swap += swapRates_swap[i][0] + ":";
				    }
				    else
				    {
					    usegrid_swap += "NONE:";
				    }
			    }
		    }
		    // set use grid
		    else if (swapRates_swap[i].size() == 3)
		    {
			    LAString useGridFrag = swapRates_swap[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE")
			    {
				    usegrid_swap += swapRates_swap[i][0] + ":";
			    }
			    else
			    {
				    usegrid_swap += "NONE:";
			    }
		    }
		    swapstream += LF;
	    }
	    std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);

	    //set FRA Object;
	    bool isFRAUse = false;
	    LAString isFRAUse_str = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + suffix_prop_swap);
	    if (isFRAUse_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFRAUse_str);
		    isFRAUse = tmpAttrB.get();
	    }

	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_FILE + suffix_prop_swap);

	    if (isFRAUse)
	    {
            AQ_REQUIRE( fraConv_swap.size() > 0, "FRA Conventions Block Missing - Must provide FRA convention settings when IsFRAUse = True" )
            AQ_REQUIRE( fra3mRates_swap.size() > 0 || fra6mRates_swap.size() > 0 , "FRA Rates Block Missing - Must provide FRA instrument data when IsFRAUse = True" )

		    LAString fraFile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_FILE + suffix_prop_swap);
		    if (fraFile == AQ_NO_DATA)
		    {
			    fraFile = LAString("data/in/") + currency + LAString("_yield_fra") + suffix_data_swap + LAString(".csv");
			    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_FILE + suffix_prop_swap, fraFile);
		    }

		    if (fra3mRates_swap.size() != 0)
		    {
			    LAString fra3mstream = etrading::buildFRAMarketDataFile(fraFile, fra3mRates_swap, areSwapsForwardStarting, usegrid_fra);

			    std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfra3mstream);
		    }
		    else if (fra6mRates_swap.size() != 0)
		    {
			    LAString fra6mstream = etrading::buildFRAMarketDataFile(fraFile, fra6mRates_swap, areSwapsForwardStarting, usegrid_fra);

			    std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfra6mstream);
		    }
		    else
		    {
			    throw LACoreInvalidData("#Error - No FRA market data is provided", __FILE__, __LINE__);
		    }
	    }

	    //set future Object;
	    bool isFutureUse = false;
	    LAString tmpFutureStr = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + suffix_prop_swap);
	    if (tmpFutureStr != AQ_NO_DATA)
	    {
		    LADataBool tmpAttrB;
		    tmpAttrB.convertFromString(tmpFutureStr);
		    isFutureUse = tmpAttrB.get();
	    }
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + suffix_prop_swap);
	    if (isFutureUse)
	    {
            AQ_REQUIRE( futureConv_swap.size() > 0, "Futures Conventions Block Missing - Must provide futures convention settings when IsFutureUse = True" )
            AQ_REQUIRE( futureRates_swap.size() > 0, "Futures Rates Block Missing - Must provide futures instrument data when IsFutureUse = True" )

		    LAString futureFile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + suffix_prop_swap);
		    if (futureFile == AQ_NO_DATA)
		    {
			    futureFile = LAString("data/in/") + currency + LAString("_yield_future") + suffix_data_swap + LAString(".csv");
			    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + suffix_prop_swap, futureFile);
		    }
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(futureFile), createFutureStream(futureRates_swap, usegrid_future));
	    }


	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + suffix_prop_swap);
	    int find;
	    if (usegrid_libor != "")
	    {
		    usegrid_libor = usegrid_libor.subString(0, usegrid_libor.size() - 2);
		    while ((find = usegrid_libor.findString("NONE:")) != -1)
		    {
			    usegrid_libor.remove(find, 5);
		    }

		    if ((find = usegrid_libor.findString(":NONE")) != -1)
		    {
			    usegrid_libor.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + suffix_prop_swap, usegrid_libor);
	    }
	    if (usegrid_swap != "")
	    {
		    usegrid_swap = usegrid_swap.subString(0, usegrid_swap.size() - 2);
		    while ((find = usegrid_swap.findString("NONE:")) != -1)
		    {
			    usegrid_swap.remove(find, 5);
		    }

		    if ((find = usegrid_swap.findString(":NONE")) != -1)
		    {
			    usegrid_swap.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + suffix_prop_swap, usegrid_swap);
	    }
	    if (usegrid_fra != "")
	    {
		    usegrid_fra = usegrid_fra.subString(0, usegrid_fra.size() - 2);
		    while ((find = usegrid_fra.findString("NONE:")) != -1)
		    {
			    usegrid_fra.remove(find, 5);
		    }

		    if ((find = usegrid_fra.findString(":NONE")) != -1)
		    {
			    usegrid_fra.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + suffix_prop_swap, usegrid_fra);
	    }
	    if (usegrid_future != "")
	    {
		    usegrid_future = usegrid_future.subString(0, usegrid_future.size() - 2);
		    while ((find = usegrid_future.findString("NONE:")) != -1)
		    {
			    usegrid_future.remove(find, 5);
		    }

		    if ((find = usegrid_future.findString(":NONE")) != -1)
		    {
			    usegrid_future.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + suffix_prop_swap, usegrid_future);
	    }

	    // Record more curve info
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffix_prop_swap, useMarkets_swap);
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_swap, CURVETYPE_SWAP);
    }

    void LAUpdateStaticDataManager::populateStaticDataManagerForOISCurve(LAStaticData &irStaticData,
													    const LAString& useMarkets,
													    const LAString& currency,
													    const LAString& curveNames_OIS,
													    const LAString& marketName_OIS, 
													    const LAString& generateCurveName_OIS,
													    const LAStringMatrix& generateProp_OIS,
													    const LAStringMatrix& oisRates_OIS,
													    const LAStringMatrix& oisConv_OIS,
													    const LAStringMatrix& histRates_OIS,
													    const LAStringMatrix& lobasisRates_OIS,
													    const LAStringMatrix& lobasisConv_OIS,
													    const LAStringMatrix& swapConv_OIS)
    {
	    LAString suffix_prop_OIS("");
	    LAString suffix_data_OIS("");

	    // If the user-given market name is not STD, concatenate it to useMarkets 
	    LAString useMarkets_OIS = useMarkets;
	    LAStringVector tmpUseMarkets = useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (generateCurveName_OIS != STD)
	    {
		    suffix_data_OIS = "_" + generateCurveName_OIS;

		    suffix_prop_OIS = "." + generateCurveName_OIS;
		    suffix_prop_OIS.toLower();

		    LAString tmpCurveName = generateCurveName_OIS;
		    tmpCurveName.toUpper();
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(), tmpUseMarkets.end(), tmpCurveName))
		    {
			    if (useMarkets_OIS == "")
			    {
				    useMarkets_OIS = generateCurveName_OIS;
			    }
			    else
			    {
				    useMarkets_OIS += LAString(MULTI_STATIC_DATA_DELIMITER) + generateCurveName_OIS;
			    }
		    }
	    }
	    else
	    {
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(), tmpUseMarkets.end(), SWAP))
		    {
			    if (useMarkets_OIS == "")
			    {
				    useMarkets_OIS = SWAP;
			    }
			    else
			    {
				    useMarkets_OIS += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
			    }
		    }
	    }

	    if (curveNames_OIS == "")
	    {
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_OIS, marketName_OIS);
	    }
	    else
	    {
		    // New code for dual bootstrapping - ensure the implicit curve name is one of the curve index names
		    LAString indexNames = curveNames_OIS;
		    if (indexNames.findString(marketName_OIS) == -1)
		    {
			    indexNames = marketName_OIS + LAString(MULTI_STATIC_DATA_DELIMITER) + indexNames;
		    }

		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_OIS, indexNames);
	    }

	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix_prop_OIS, "DAILYCOMPOUNDING");

	    // Set curve type
	    //	LAString curveType = CURVETYPE_OIS;
	    //	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_OIS, curveType);

	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffix_prop_OIS);

	    MLIB_2D_MATRIX_CHECK( generateProp_OIS, "Invalid OIS Curve Properties / GenerateProps" )
	    for (size_t i = 0; i<generateProp_OIS.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.generator." + generateProp_OIS[i][0] + suffix_prop_OIS;
		    key.toLower();
		    LAString data = generateProp_OIS[i][1];
		    if (key.findString("dfcurvename") == -1) data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

        // *** Important *** Must remove keys that can accept blank values, since these will not be removed by default
	    irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffix_prop_OIS );
        irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix_prop_OIS );
        irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix_prop_OIS );
        irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD + suffix_prop_OIS );
		irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD_ARROIS + suffix_prop_OIS);
		irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD + suffix_prop_OIS );
        irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD + suffix_prop_OIS );
		irStaticData.removeStaticData( currency + STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE + suffix_prop_OIS);

	    MLIB_2D_MATRIX_CHECK( oisConv_OIS, "Invalid OIS Conventions" )
	    for (size_t i = 0; i<oisConv_OIS.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.ois." + oisConv_OIS[i][0] + suffix_prop_OIS;
		    key.toLower();
		    LAString data = oisConv_OIS[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    LAString oisFile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix_prop_OIS);
	    if (oisFile == AQ_NO_DATA)
	    {
		    oisFile = LAString("data/in/") + currency + LAString("_yield_ois_oiscurve.csv");
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix_prop_OIS, oisFile);
	    }

	    LAString fedFundFutureFile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffix_prop_OIS);
	    if (fedFundFutureFile == AQ_NO_DATA)
	    {
		    fedFundFutureFile = LAString("data/in/") + currency + LAString("_yield_fffuture.csv");
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffix_prop_OIS, fedFundFutureFile);
	    }

	    LAString histFile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix_prop_OIS);
	    if (histFile == AQ_NO_DATA)
	    {
		    histFile = LAString("data/in/") + currency + LAString("_yield_historical_ois_oiscurve.csv");
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix_prop_OIS, histFile);
	    }

	    LAString oisStream;
	    LAString fedFundFutureStream;
	    LAString usegrid_ois = "";
	    LAString usegrid_fffuture = "";
	
	    MLIB_2D_MATRIX_CHECK( oisRates_OIS, "Invalid OIS Rates" )
	    for (size_t i = 0; i<oisRates_OIS.size(); i++)
	    {
		    if (oisRates_OIS[i][0].findString("FF") != -1)
		    {
			    fedFundFutureStream += oisRates_OIS[i][0];

			    if (oisRates_OIS[i].size() != 3 && oisRates_OIS[i].size() != 5)
				    throw LACoreInvalidData("FF input size error", __FILE__, __LINE__);

			    //in case of FF, quoted value is price
			    double oisRate = oisRates_OIS[i][1].getDoubleValue();
			    fedFundFutureStream += "," + LAString(oisRate);

			    if (oisRates_OIS[i].size() == 5)
			    {
				    const LADate& startdate = stringToDate(oisRates_OIS[i][2]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    fedFundFutureStream += "," + startdate_str;
				    const LADate& enddate = stringToDate(oisRates_OIS[i][3]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    fedFundFutureStream += "," + enddate_str;
			    }

			    fedFundFutureStream += LF;

			    if (oisRates_OIS[i].size() == 5)
			    {
				    LAString useGridFrag = oisRates_OIS[i][4]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fffuture += oisRates_OIS[i][0] + ":";
				    else usegrid_fffuture += "NONE:";
			    }
			    else if (oisRates_OIS[i].size() == 3)
			    {
				    LAString useGridFrag = oisRates_OIS[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fffuture += oisRates_OIS[i][0] + ":";
				    else usegrid_fffuture += "NONE:";
			    }
		    }
		    else
		    {
			    oisStream += oisRates_OIS[i][0];

			    // Allow Central Bank Swap instruments to be specified in the OIS curve for all markets ...
			    if ( oisRates_OIS[i][0].findString("BOJ") != -1         // Bank of Japan
				    || oisRates_OIS[i][0].findString("EUSF") != -1      // European Central Bank (Bloomberg Ticker)
				    || oisRates_OIS[i][0].findString("ECB") != -1       // European Central Bank
				    || oisRates_OIS[i][0].findString("BOE") != -1       // Bank of England
				    || oisRates_OIS[i][0].findString("CB") != -1        // *** GENERIC *** Central Bank Swaps
                    || oisRates_OIS[i][0].findString("MPC") != -1 )     // *** GENERIC *** Monetary Policy Committee Swaps
			    {
				    if (oisRates_OIS[i].size() < 4)
					    throw LACoreInvalidData("short term market needs StartDate and EndDate", __FILE__, __LINE__);

				    double oisRate = oisRates_OIS[i][1].getDoubleValue() * 100.0;
				    oisStream += "," + LAString(oisRate);

				    const LADate& startdate = stringToDate(oisRates_OIS[i][2]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    oisStream += "," + startdate_str;
				    const LADate& enddate = stringToDate(oisRates_OIS[i][3]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    oisStream += "," + enddate_str;
			    }
			    else //normal case
			    {
				    double oisRate = oisRates_OIS[i][1].getDoubleValue() * 100.0;
				    oisStream += "," + LAString(oisRate);
			    }

			    oisStream += LF;

			    if (oisRates_OIS[i].size() == 5)
			    {
				    LAString useGridFrag = oisRates_OIS[i][4]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_ois += oisRates_OIS[i][0] + ":";
				    else usegrid_ois += "NONE:";
			    }
			    else if (oisRates_OIS[i].size() == 3)
			    {
				    LAString useGridFrag = oisRates_OIS[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_ois += oisRates_OIS[i][0] + ":";
				    else usegrid_ois += "NONE:";
			    }
		    }
	    }
	    std::istringstream *pOISStream = new std::istringstream(oisStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(oisFile), pOISStream);
	    std::istringstream *pFedFundFutureStream = new std::istringstream(fedFundFutureStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fedFundFutureFile), pFedFundFutureStream);


	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix_prop_OIS);
	    int find;
	    if (usegrid_ois != "")
	    {
		    usegrid_ois = usegrid_ois.subString(0, usegrid_ois.size() - 2);
		    while ((find = usegrid_ois.findString("NONE:")) != -1) usegrid_ois.remove(find, 5);
		    if ((find = usegrid_ois.findString(":NONE")) != -1) usegrid_ois.remove(find, 5);
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix_prop_OIS, usegrid_ois);
	    }
	    if (usegrid_fffuture != "")
	    {
		    usegrid_fffuture = usegrid_fffuture.subString(0, usegrid_fffuture.size() - 2);
		    while ((find = usegrid_fffuture.findString("NONE:")) != -1) usegrid_fffuture.remove(find, 5);
		    if ((find = usegrid_fffuture.findString(":NONE")) != -1) usegrid_fffuture.remove(find, 5);
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + suffix_prop_OIS, usegrid_fffuture);
	    }

	    LAString histStream;
	    MLIB_2D_MATRIX_CHECK( histRates_OIS, "Invalid OIS Fixing and Reset Rates" )
	    for (size_t i = 0; i<histRates_OIS.size(); i++)
	    {
		    const LADate& histdate = stringToDate(histRates_OIS[i][0]);
		    LAString histdate_str = histdate.stringWithFormat("YYYYMMDD");
		    histStream += histdate_str;
		    double histRate = histRates_OIS[i][1].getDoubleValue() * 100.0;
		    histStream += "," + LAString(histRate);
		    histStream += LF;
	    }
	    std::istringstream *pHISTStream = new std::istringstream(histStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(histFile), pHISTStream);

	    MLIB_2D_MATRIX_CHECK( lobasisConv_OIS, "Invalid Libor-OIS Basis Conventions" )
	    for (size_t i = 0; i<lobasisConv_OIS.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.basis." + lobasisConv_OIS[i][0] + suffix_prop_OIS;
		    key.toLower();
		    LAString data = lobasisConv_OIS[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    LAString lobasisfile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffix_prop_OIS);
	    if (lobasisfile == AQ_NO_DATA)
	    {
		    lobasisfile = LAString("data/in/") + currency + LAString("_yield_lobasis") + suffix_prop_OIS + LAString(".csv");
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffix_prop_OIS, lobasisfile);
	    }
	    LAString lobasisstream;
	    MLIB_2D_MATRIX_CHECK( lobasisRates_OIS, "Invalid Libor-OIS Basis Rates" )
		const size_t lobasisColumnSize = ( lobasisRates_OIS.size() > 0 ) ? lobasisRates_OIS[0].size() : 0;

	    for (size_t i = 0; i<lobasisRates_OIS.size(); i++)
	    {
			// Libor-OIS Rates Columns: Term, Rate, LiborType (Optional), UseMarketData (Optional)
		    lobasisstream += lobasisRates_OIS[i][0];
		    const double lobrate = lobasisRates_OIS[i][1].getDoubleValue() * 100.0;
		    lobasisstream += "," + LAString(lobrate);

			// Optional Column 3: LiborType
			if ( lobasisColumnSize > 2 )
			{
				const LAString liborType = lobasisRates_OIS[i][2];
				lobasisstream += "," + liborType;
			}

			// Optional Column 4: UseMarketData
			if ( lobasisColumnSize > 3 )
			{
				const LAString useMarketData = lobasisRates_OIS[i][3];
				lobasisstream += "," + useMarketData;
			}

			lobasisstream += LF;
	    }
	    std::istringstream *plobasisstream = new std::istringstream(lobasisstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(lobasisfile), plobasisstream);

	    MLIB_2D_MATRIX_CHECK( swapConv_OIS, "Invalid Swap Conventions" )
	    for (size_t i = 0; i<swapConv_OIS.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.swap." + swapConv_OIS[i][0] + suffix_prop_OIS;
		    key.toLower();
		    LAString data = swapConv_OIS[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    // Remove swap par rate market data from OIS curve
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_prop_OIS);

	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_OIS_ISDUALBOOTSTRAPPING + suffix_prop_OIS, LAString("TRUE"));

	    // Record more curve info
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_OIS, CURVETYPE_OIS);
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffix_prop_OIS, useMarkets_OIS);
    }


	// =====================================================================================================================
    void LAUpdateStaticDataManager::populateStaticDataManagerForTenorBasisCurve(LAStaticData &irStaticData, 
																				const LAString& curveName,
																				const LAString& curveNames,
																				const LAString& currency,
																				const LAStringMatrix& basisRates,
																				const LAStringMatrix& basisConv,
																				const LAStringMatrix& fwdFXs,
																				const LAStringMatrix& fwdConv,
																				const LAStringMatrix& spotFXs,
																				const LAStringMatrix& generateProp,
																				const LAStringMatrix& moneyConv,
																				const LAStringMatrix& fraConv,
																				const LAStringMatrix& fraRates,
																				const LAStringMatrix& liborConv,
																				const LAStringMatrix& liborRates)
    {
	    LAString staticDataSuffix;
	    LAString suffix_data;
	    staticDataSuffix = "." + curveName;
	    staticDataSuffix.toLower();
	    suffix_data = "_" + curveName;
	    suffix_data.toLower();

	    // Clear certain control parameters first
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_FIXINGSOURCE	+ staticDataSuffix);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE		+ staticDataSuffix);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDBASIS		+ staticDataSuffix);	
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD	+ staticDataSuffix);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + staticDataSuffix);
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_ISARBFREE);

	    LAString useMarkets = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	    LAStringVector tmpUseMarkets = useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (useMarkets == AQ_NO_DATA)
	    {
		    useMarkets = "";
	    }
	    LAString tmpCurveName = curveName;
	    tmpCurveName.toUpper();

	    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(), tmpUseMarkets.end(), tmpCurveName))
	    {
		    if (useMarkets == "")
		    {
			    useMarkets = curveName;
		    }
		    else
		    {
			    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + curveName;
		    }
	    }
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS, useMarkets);

	    if (curveNames == "")
	    {
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveName);
	    }
	    else
	    {
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveNames);
	    }

	    LAString curveType = CURVETYPE_BASIS;
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + staticDataSuffix, curveType);

	    // By default, set the "isfwdinter" property using false.
	    // If the user has specified the property in the basisConv info, allow the user setting to override
	    // this default in the following "basis info" code block.
	    {
		    LAString key = currency + STATIC_DATA_KEY_YIELD_BASIS_ISFWDINTER + staticDataSuffix;
		    key.toLower();
		    LAString data = "false";
		    irStaticData.setStaticData(key, data);
	    }

	    //basis info
	    MLIB_2D_MATRIX_CHECK( basisConv, "Invalid Basis Conventions" )
	    for (size_t i = 0; i<basisConv.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.basis." + basisConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = basisConv[i][1];
		    if (key.findString("discount") == -1 && key.findString("forecast") == -1)
		    {
			    data.toLower();
		    }
		    irStaticData.setStaticData(key, data);
	    }
	    //generator info
	    MLIB_2D_MATRIX_CHECK( generateProp, "Invalid Curve Properties or generateProp Conventions" )
	    for (size_t i = 0; i<generateProp.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.generator." + generateProp[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = generateProp[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	    //money info
	    MLIB_2D_MATRIX_CHECK( moneyConv, "Invalid Money Market Conventions" )
	    for (size_t i = 0; i<moneyConv.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.moneymarket." + moneyConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = moneyConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	    //FRA info
	    MLIB_2D_MATRIX_CHECK( fraConv, "Invalid FRA Conventions" )
	    for (size_t i = 0; i<fraConv.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.fra." + fraConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = fraConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	    //fwdfx info
	    MLIB_2D_MATRIX_CHECK( fwdConv, "Invalid Forward FX Conventions" )
	    for (size_t i = 0; i<fwdConv.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.basis.fwdfx." + fwdConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = fwdConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	    // Libor market convention
	    MLIB_2D_MATRIX_CHECK( liborConv, "Invalid Libor Fixings and Reset Conventions" )
	    for (size_t i = 0; i<liborConv.size(); i++)
	    {
		    LAString key = currency + ".sde.yield.libor." + liborConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = liborConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    //set Libor data file
	    LAString liborUseGrid = "";
	    LAString isLiborProvided_str = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_FIXINGSOURCE + staticDataSuffix);
	    if (isLiborProvided_str != AQ_NO_DATA)
	    {
		    if (isLiborProvided_str.toUpper() == ITSELF)
		    {
			    LAString liborEntityName = currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix;

			    LAString liborfile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix);
			    if (liborfile == AQ_NO_DATA)
			    {
				    liborfile = LAString("data/in/") + currency + LAString("_yield_libor") + suffix_data + LAString(".csv");
				    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + staticDataSuffix, liborfile);
			    }
			    LAString liborstream;
			    MLIB_2D_MATRIX_CHECK( liborRates, "Invalid Libor Fixing and Reset Rates" )
			    for (size_t i = 0; i<liborRates.size(); i++)
			    {
				    liborstream += liborRates[i][0];
				    double lrate = liborRates[i][1].getDoubleValue() * 100.0;
				    liborstream += "," + LAString(lrate) + LF;

				    if (liborRates[i].size() == 3)
				    {
					    LAString useGridFrag = liborRates[i][2]; upper(useGridFrag);
					    if (useGridFrag == "TRUE")
					    {
						    liborUseGrid += liborRates[i][0] + ":";
					    }
					    else
					    {
						    liborUseGrid += "NONE:";
					    }
				    }
			    }
			    std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);
		    }
	    }

	    //set FRA file
	    bool isFRAUse = false;
	    LAString fraUseGrid = "";
	    LAString isFRAUse_str = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + staticDataSuffix);
	    LADataBool tmpAttrB;
	    if (isFRAUse_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFRAUse_str);
		    isFRAUse = tmpAttrB.get();
	    }

	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix);
	    if (isFRAUse)
	    {
            AQ_REQUIRE( fraConv.size() > 0, "FRA Conventions Block Missing - Must provide FRA convention settings when IsFRAUse = True" )
            AQ_REQUIRE( fraRates.size() > 0, "FRA Rates Block Missing - Must provide FRA instrument data when IsFRAUse = True" )

		    //set fwd swap
		    bool areSwapsForwardStarting = false;
		    LAString isFwdSwap_str = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + staticDataSuffix);
		    if (isFwdSwap_str != AQ_NO_DATA)
		    {
			    tmpAttrB.convertFromString(isFwdSwap_str);
			    areSwapsForwardStarting = tmpAttrB.get();
		    }

		    LAString fraFile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix);
		    if (fraFile == AQ_NO_DATA)
		    {
			    fraFile = LAString("data/in/") + currency + LAString("_yield_fra") + suffix_data + LAString(".csv");
			    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_FILE + staticDataSuffix, fraFile);
		    }

		    LAString fraStream = etrading::buildFRAMarketDataFile(fraFile, fraRates, areSwapsForwardStarting, fraUseGrid);
		    std::istringstream *pfraStream = new std::istringstream(fraStream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfraStream);
	    }

	    //set fwd basis
	    bool isFwdBasis = false;
	    LAString isFwdBasis_str = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDBASIS + staticDataSuffix);
	    if (isFwdBasis_str.toUpper() == "TRUE")
	    {
		    isFwdBasis = true;
	    }

	    //basis file
	    LAString basisfile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix);
	    if (basisfile == AQ_NO_DATA)
	    {
		    basisfile = LAString("data/in/") + currency + LAString("_yield_basisswap") + suffix_data + LAString(".csv");
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FILE + staticDataSuffix, basisfile);
	    }
	    LAString basisstream;
	    LAString usegrid = "";
	    MLIB_2D_MATRIX_CHECK( basisRates, "Invalid Basis Rates" )
	    for (unsigned int i = 0; i < basisRates.size(); i++)
	    {
		    basisstream += basisRates[i][0];
		    const double brate = basisRates[i][1].getDoubleValue() * 10000.0;
		    basisstream += "," + LAString(brate);
		    // for fwd basis
		    if (isFwdBasis)
		    {
			    if (basisRates[i].size() < 5)
			    {
				    throw LACoreInvalidData("#Error: Invalid Tenor Basis market data, wrong number of columns. Market Data should consist of 5 columns specifiying forward dates when the 'isFwdBasis' flag is set to true.", __FILE__, __LINE__);
			    }

			    LAString isDate_str = basisRates[i][2];
			    basisstream += "," + isDate_str.toUpper();
			    if (isDate_str == "TRUE")
			    {
				    const LADate& startdate = stringToDate(basisRates[i][3]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    basisstream += "," + startdate_str;
				    const LADate& enddate = stringToDate(basisRates[i][4]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    basisstream += "," + enddate_str;
			    }
			    else
			    {
				    LAString startterm_str = basisRates[i][3];
				    basisstream += "," + startterm_str.toUpper();
				    LAString tenor_str = basisRates[i][4];
				    basisstream += "," + tenor_str.toUpper();
			    }
			    // set use grid
			    if (basisRates[i].size() == 6)
			    {
				    LAString useGridFrag = basisRates[i][5]; upper(useGridFrag);
				    if (useGridFrag == "TRUE")
				    {
					    usegrid += basisRates[i][0] + ":";
				    }
				    else
				    {
					    usegrid += "NONE:";
				    }
			    }
		    }
		    else if (basisRates[i].size() == 3)
		    {
			    LAString useGridFrag = basisRates[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE")
			    {
				    usegrid += basisRates[i][0] + ":";
			    }
			    else
			    {
				    usegrid += "NONE:";
			    }
		    }
		    basisstream += LF;
	    }
	    std::istringstream *pbasisstream = new std::istringstream(basisstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(basisfile), pbasisstream);

	    //fwdfx file
	    LAString fwdfile = irStaticData.getStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_FILE + staticDataSuffix);
	    LAString fwdstream;
	    LAString fwdusegrid = "";
	    MLIB_2D_MATRIX_CHECK( fwdFXs, "Invalid Forward FX Rates" )
	    for (unsigned int i = 0; i < fwdFXs.size(); i++)
	    {
		    fwdstream += fwdFXs[i][0];
		    const double fwdfx = fwdFXs[i][1].getDoubleValue();
		    fwdstream += "," + LAString(fwdfx) + LF;

		    if (fwdFXs[i].size() == 3)
		    {
			    LAString useGridFrag = fwdFXs[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE")
			    {
				    fwdusegrid += fwdFXs[i][0] + ":";
			    }
			    else
			    {
				    fwdusegrid += "NONE:";
			    }
		    }
	    }
	    std::istringstream *pfwdstream = new std::istringstream(fwdstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fwdfile), pfwdstream);

	    // Set up UseGrids
	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix);
	    int find;
	    if (liborUseGrid != "")
	    {
		    liborUseGrid = liborUseGrid.subString(0, liborUseGrid.size() - 2);
		    while ((find = liborUseGrid.findString("NONE:")) != -1)
		    {
			    liborUseGrid.remove(find, 5);
		    }

		    if ((find = liborUseGrid.findString(":NONE")) != -1)
		    {
			    liborUseGrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + staticDataSuffix, liborUseGrid);
	    }

	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + staticDataSuffix);
	    if (fraUseGrid != "")		// FRA UseGrid
	    {
		    fraUseGrid = fraUseGrid.subString(0, fraUseGrid.size() - 2);
		    while ((find = fraUseGrid.findString("NONE:")) != -1)
		    {
			    fraUseGrid.remove(find, 5);
		    }

		    if ((find = fraUseGrid.findString(":NONE")) != -1)
		    {
			    fraUseGrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + staticDataSuffix, fraUseGrid);
	    }

	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + staticDataSuffix);
	    if (usegrid != "")			// Basis UseGrid
	    {
		    usegrid = usegrid.subString(0, usegrid.size() - 2);
		    while ((find = usegrid.findString("NONE:")) != -1)
		    {
			    usegrid.remove(find, 5);
		    }

		    if ((find = usegrid.findString(":NONE")) != -1)
		    {
			    usegrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + staticDataSuffix, usegrid);
	    }

	    irStaticData.removeStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + staticDataSuffix);
	    if (fwdusegrid != "")		// Forward UseGrid
	    {
		    fwdusegrid = fwdusegrid.subString(0, fwdusegrid.size() - 2);
		    while ((find = fwdusegrid.findString("NONE:")) != -1)
		    {
			    fwdusegrid.remove(find, 5);
		    }

		    if ((find = fwdusegrid.findString(":NONE")) != -1)
		    {
			    fwdusegrid.remove(find, 5);
		    }
		    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID + staticDataSuffix, fwdusegrid);
	    }

	
	


	    //-----------------------------------------------------------------------------------------
	
	
	    // Record more curve info	
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + staticDataSuffix, CURVETYPE_BASIS);
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_USEMAKETS			+ staticDataSuffix, useMarkets);
	    irStaticData.setStaticData(currency + STATIC_DATA_KEY_YIELD_MARKETTYPE			+ staticDataSuffix, MARKETTYPE_BASIS);
    }


    void LAUpdateStaticDataManager::setUpGlobalEngineCurves(LADataInstance* dataInstance,
													        const LAString& engineName,
													        const LAString& curveCollectionID,
													        const LAStringMatrix& commonParams,
													        const std::vector<etrading::CurveObjectDataPtr>& curveDataCollection)
    {
	    LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
	    LAObjectPool &objPool = dataInstance->getObjectPool();
	    LAObject* pyld = NULL;

	    //======================================================
	    // Get currency
        AQ_REQUIRE( curveDataCollection.size() > 0, "Unable to set-up Global Engine Curves - No Curve Data has been provided" )
	    LAStringMatrix tmpInfo = curveDataCollection[0]->curveConvLVB_.toLAStringMatrix(); // Conventions are stored as LVBs
	    upper(tmpInfo);
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveCollectionID, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {
		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if (dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo, CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo, CURVEINPUT_CURRENCY, 1);
			    }
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo, CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo, CURVEINPUT_CURRENCY, 1);
		    }
	    }
	    LAString tmpCurrency = currency;
	    tmpCurrency.toLower();

	    //======================================================
	    // Miscallaneous

	    LAString staticDataSuffix("");
	    LAString suffix_data("");
	    LAString generateCurveName = engineName;
	    generateCurveName.toUpper();
	    if (generateCurveName != STD)
	    {
		    staticDataSuffix = "." + generateCurveName;
		    staticDataSuffix.toLower();
		    suffix_data = "_" + generateCurveName;
	    }

	    // The useMarkets string is a list of default market names for a particular currency in the form 
	    // of '1M3MBasis:3M6MBasis:SWAP:XCCYBasis' for example. This is loaded directly from the ir.properties file
	    LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	    LAStringVector useMarkets_vector = useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (useMarkets == AQ_NO_DATA)
	    {
		    useMarkets = "";
	    }

	    // Add common params to Property Manager
	    MLIB_2D_MATRIX_CHECK( commonParams, "Invalid CommonParameters Data Block" )
	    for (size_t i = 0; i<commonParams.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.globalenginecurves." + commonParams[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = commonParams[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    //======================================================
	    // Loop through each curve's data
	    LAString allCurveNames("");
	    LAString allCurveTypes("");
	    for (unsigned int i = 0; i < curveDataCollection.size(); ++i)
	    {
		    etrading::CurveTypeEnum curveType = curveDataCollection[i]->getCurveType();
		    LAStringMatrix curveConv = curveDataCollection[i]->curveConvLVB_.toLAStringMatrix(); // Conventions as LVBs;
		    LAString curveName = curveDataCollection[i]->curveName_;
		    LAString curveIndex = curveDataCollection[i]->curveIndex_;

		    allCurveNames += (i == 0 ? "" : LAString(MULTI_STATIC_DATA_DELIMITER)) + curveName;

		    if (curveType == etrading::OIS_CURVETYPE)
		    {
			    OISCurveObjectData curveData = *dynamic_cast<OISCurveObjectData*>(curveDataCollection[i].get());
			
			    LAString marketName = curveName;
			    if (marketName.size() == 0)
			    {
				    marketName = engineName + "OIS";
			    }

			    LAString generateCurveName = marketName;
			    generateCurveName.toUpper();

			    // Set as-of date from OIS curve
			    LAStringMatrix tmpInfo = curveData.curveConvLVB_.toLAStringMatrix(); // Curve Conventions are LVBs
			    upper(tmpInfo);
			    LADate asofdate = stringToDate(chgrow(tmpInfo, CURVEINPUT_ASOFDATE, 1));
			    LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
			
			    // Populate all relevant parameters and market data to property manager
			    populateStaticDataManagerForOISCurve(irStaticData,
										             useMarkets,
										             tmpCurrency,
										             curveIndex,
										             marketName,
										             generateCurveName,
										             curveData.curveConvLVB_.toLAStringMatrix(),    // Conventions as LVBs
										             curveData.oisRates_,
										             curveData.oisConvLVB_.toLAStringMatrix(),      // Conventions as LVBs
										             curveData.histRates_,
										             curveData.loBasisRates_,
										             curveData.loBasisConvLVB_.toLAStringMatrix(),  // Conventions as LVBs
										             curveData.swapConvLVB_.toLAStringMatrix() );   // Conventions as LVBs
			
			    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME + staticDataSuffix);
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME + staticDataSuffix, generateCurveName);

			    allCurveTypes += (i == 0 ? "" : LAString(MULTI_STATIC_DATA_DELIMITER)) + "OIS";
		    }
		    else if (curveType == etrading::SWAP_CURVETYPE)
		    {
			    SwapCurveObjectData curveData = *dynamic_cast<SwapCurveObjectData*>(curveDataCollection[i].get());

			    LAString marketName = curveName;
			    if (marketName.size() == 0)
			    {
				    marketName = engineName + "STD";
			    }

			    LAString generateCurveName = marketName;
			    generateCurveName.toUpper();

			    // Populate all relevant parameters and market data to property manager
			    populateStaticDataManagerForSwapCurve(irStaticData,
											          useMarkets,
											          tmpCurrency,
											          curveIndex,
											          marketName,
											          generateCurveName,
											          curveData.curveConvLVB_.toLAStringMatrix(),           // Conventions as LVBs
											          curveData.moneyMarketConvLVB_.toLAStringMatrix(),     // Conventions as LVBs
											          curveData.liborRates_,
											          curveData.liborConvLVB_.toLAStringMatrix(),           // Conventions as LVBs
											          curveData.swapRates_,
											          curveData.swapConvLVB_.toLAStringMatrix(),            // Conventions as LVBs
											          curveData.fra3mRates_,
											          curveData.fra3mRates_,
											          curveData.fra3mRates_,
											          curveData.futureRates_,
											          curveData.futureConvLVB_.toLAStringMatrix(),          // Conventions as LVBs
											          curveData.basisAdjConvLVB_.toLAStringMatrix(),        // Conventions as LVBs
											          curveData.basisAdjRates_);

			    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + staticDataSuffix);
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + staticDataSuffix, generateCurveName);

			    allCurveTypes += (i == 0 ? "" : LAString(MULTI_STATIC_DATA_DELIMITER)) + "SWAP";
		    }
		    else if (curveType == etrading::TENORBASIS_CURVETYPE)
		    {
			    TenorBasisCurveObjectData curveData = *dynamic_cast<TenorBasisCurveObjectData*>(curveDataCollection[i].get());

			    LAString marketName = curveName;
			    if (marketName.size() == 0)
			    {
				    marketName = engineName + "TENORBASIS";
			    }

			    LAString generateCurveName = marketName;
			    generateCurveName.toUpper();

			    populateStaticDataManagerForTenorBasisCurve(irStaticData,
												            marketName,
												            curveIndex,
												            tmpCurrency,
												            curveData.basisRates_,
												            curveData.basisConvLVB_.toLAStringMatrix(),         // Conventions as LVBs
												            curveData.fxFwdRates_,
												            curveData.fxFwdConvLVB_.toLAStringMatrix(),         // Conventions as LVBs
												            curveData.spotFxRates_,
												            curveData.curveConvLVB_.toLAStringMatrix(),         // Conventions as LVBs
												            curveData.moneyMarketConvLVB_.toLAStringMatrix(),   // Conventions as LVBs
												            curveData.fraConvLVB_.toLAStringMatrix(),           // Conventions as LVBs
												            curveData.fraConvLVB_.toLAStringMatrix(),           // Conventions as LVBs
												            curveData.liborConvLVB_.toLAStringMatrix(),         // Conventions as LVBs
												            curveData.liborRates_);

			    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + staticDataSuffix);
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + staticDataSuffix, generateCurveName);

			    allCurveTypes += (i == 0 ? "" : LAString(MULTI_STATIC_DATA_DELIMITER)) + "TENORBASIS";
		    }
		    else
		    {
			    AQ_THROW("Un-supported curve type is detected in multi-curve solving engine. Only support OIS, Swap and Tenor Basis curves.");
		    }		
	    }

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, generateCurveName.toUpper());
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveCollectionID);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVENAMES + staticDataSuffix, allCurveNames);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GLOBALENGINECURVES_ALLCURVETYPES + staticDataSuffix, allCurveTypes);
	
        //
        // ============================== CURVE CALIBRATION ========================================================

	    std::unique_ptr<LAObjectPoolBase> objectPoolManager(new UpdateObjectPoolForSDEsAndCurves(currency));
	    objectPoolManager->loadGlobalCurveDataAndCalibrate(currency, *dataInstance);

    }


	// ============================ DUAL-BOOTSTRAP CURVE ==============================================================================
    void LAUpdateStaticDataManager::setUpDualBootstrapCurve(LADataInstance* dataInstance,									 
									                        const LAString& curveID,
									                        const LAString& curveName_db,
									                        const LAString& curveName_OIS,
									                        const LAString& curveName_swap,
									                        const LAString& curveNames_OIS,
									                        const LAString& curveNames_swap,	 
									                        const LAStringMatrix& commonParams,
									                        const LAStringMatrix& generateProp_OIS, 
									                        const LAStringMatrix& oisRates_OIS, 
									                        const LAStringMatrix& oisConv_OIS,
									                        const LAStringMatrix& histRates_OIS,
									                        const LAStringMatrix& lobasisRates_OIS, 
									                        const LAStringMatrix& lobasisConv_OIS, 
									                        const LAStringMatrix& swapConv_OIS,
									                        const LAStringMatrix& generateProp_swap, 
									                        const LAStringMatrix& moneyConv_swap,
									                        const LAStringMatrix& liborRates_swap, 
									                        const LAStringMatrix& liborConv_swap,
									                        const LAStringMatrix& swapRates_swap, 
									                        const LAStringMatrix& swapConv_swap,
									                        const LAStringMatrix& fra3mRates_swap,
									                        const LAStringMatrix& fra6mRates_swap,
									                        const LAStringMatrix& fraConv_swap,
									                        const LAStringMatrix& futureRates_swap, 
									                        const LAStringMatrix& futureConv_swap,
									                        const LAStringMatrix& adjustSwapConv_swap,
									                        const LAStringMatrix& adjustSwapRates_swap )
    {
	    LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();


	    LAObjectPool &objPool = dataInstance->getObjectPool();
	    LAObject* pyld = NULL;

	    LAStringMatrix tmpInfo = generateProp_swap;
	    upper(tmpInfo);
	    LADate asofdate	= stringToDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
	    LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveID, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {
		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; 
	    tmpCurrency.toLower();

	    LAString marketName_swap = curveName_swap;
	    if (marketName_swap.size() == 0)
	    {
		    marketName_swap	= curveName_db + "STD";
	    }

	    LAString generateCurveName_swap = marketName_swap;
	    generateCurveName_swap.toUpper();
	
	    LAString marketName_OIS = curveName_OIS;
	    if (marketName_OIS.size() == 0)
	    {
		    marketName_OIS	= curveName_db + "OIS";
	    }

	    LAString generateCurveName_OIS = marketName_OIS;
	    generateCurveName_OIS.toUpper();

	    LAString staticDataSuffix("");
	    LAString suffix_data("");
	    LAString generateCurveName = curveName_db;
	    generateCurveName.toUpper();
	    if (generateCurveName != STD)
	    {
		    staticDataSuffix = "." + generateCurveName;
		    staticDataSuffix.toLower();
		    suffix_data = "_" + generateCurveName;
	    }
	
	    // The useMarkets string is a list of default market names for a particular currency in the form 
	    // of '1M3MBasis:3M6MBasis:SWAP:XCCYBasis' for example. This is loaded directly from the ir.properties file
	    LAString useMarkets = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS).toUpper();
	    LAStringVector tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
	    if (useMarkets == AQ_NO_DATA) 
	    {
		    useMarkets = "";
	    }
	
	    // Add common params to Property Manager
	    MLIB_2D_MATRIX_CHECK( commonParams, "Invalid CommonParameters Data Block" )
	    for (size_t i = 0; i<commonParams.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.dualbootstrap." + commonParams[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = commonParams[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }


	    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Begin - SWAP >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	    LAString suffix_prop_swap("");
	    LAString suffix_data_swap("");
	
	    // If the user-given market name is not STD, concatenate it to useMarkets 
	    LAString useMarkets_swap = useMarkets;
	    if (generateCurveName_swap != STD)
	    {
		    suffix_data_swap = "_" + generateCurveName_swap;

		    suffix_prop_swap = "." + generateCurveName_swap;
		    suffix_prop_swap.toLower();

		    LAString tmpCurveName = generateCurveName_swap; 
		    tmpCurveName.toUpper();
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		    {
			    if (useMarkets_swap == "") 
			    {
				    useMarkets_swap = generateCurveName_swap;
			    }
			    else 
			    {
				    useMarkets_swap += LAString(MULTI_STATIC_DATA_DELIMITER) + generateCurveName_swap;
			    }
		    }	
	    }
	    else
	    {	
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
		    {
			    if (useMarkets_swap == "") 
			    {
				    useMarkets_swap = SWAP;
			    }
			    else 
			    {
				    useMarkets_swap += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
			    }
		    }
	    }

	    if (curveNames_swap == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_swap, marketName_swap);
	    }
	    else
	    {
		    // New code for dual bootstrapping - ensure the implicit curve name is one of the curve index names
		    LAString indexNames = curveNames_swap;
		    if (indexNames.findString(marketName_swap) == -1)
		    {
			    indexNames = marketName_swap + LAString(MULTI_STATIC_DATA_DELIMITER) + indexNames;
		    }

		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_swap, indexNames);
	    }

	    // Set curve type
	    LAString curveType = CURVETYPE_SWAP;
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_swap, curveType);

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ISARBFREE);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISSWAPTENORADJUST + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_BASEFREQUENCYFLOAT + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_GENERATEFORWARDSFROMSWAPSONLY + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_OPTIMIZEPERFORMANCE + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_FASTREBUILD + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_INTERPOLATIONJOINDATE + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ALWAYSCALCJOINDATE + suffix_prop_swap);

	    bool isswaptenoradjust = false;
	    MLIB_2D_MATRIX_CHECK( generateProp_swap, "Invalid CurveProperties or generatorProp Conventions" )
	    for(size_t i=0; i<generateProp_swap.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp_swap[i][0] + suffix_prop_swap;
		    key.toLower();
		    LAString data = generateProp_swap[i][1];
		    if (key.findString("dfcurvename") == -1) 
		    {
			    data.toLower();
		    }
		    irStaticData.setStaticData(key, data);

		    LAString tmpProp = generateProp_swap[i][0];
		    tmpProp.toLower();
		    if (tmpProp == "isswaptenoradjust")
		    {
			    key = generateProp_swap[i][1];
			    isswaptenoradjust = (key.toUpper() == "TRUE");
		    }
	    }
	
	    MLIB_2D_MATRIX_CHECK( moneyConv_swap, "Invalid Money Market Conventions" )
	    for(size_t i=0; i<moneyConv_swap.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.moneymarket." + moneyConv_swap[i][0] + suffix_prop_swap;
		    key.toLower(); 
		    LAString data = moneyConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	
	    MLIB_2D_MATRIX_CHECK( liborConv_swap, "Invalid Libor Fixing and Reset Conventions" )
	    for(size_t i=0; i<liborConv_swap.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.libor." + liborConv_swap[i][0] + suffix_prop_swap;
		    key.toLower(); 
		    LAString data = liborConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
	
	    MLIB_2D_MATRIX_CHECK( swapConv_swap, "Invalid Libor Fixing and Reset Conventions" )
	    for(size_t i=0; i<swapConv_swap.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.swap." + swapConv_swap[i][0] + suffix_prop_swap;
		    key.toLower(); 
		    LAString data = swapConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( fraConv_swap, "Invalid FRA Conventions" )
	    for(size_t i=0; i<fraConv_swap.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.fra." + fraConv_swap[i][0] + suffix_prop_swap;
		    key.toLower(); 
		    LAString data = fraConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

	    MLIB_2D_MATRIX_CHECK( futureConv_swap, "Invalid Futures Conventions" )
	    for(size_t i=0; i<futureConv_swap.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.future." + futureConv_swap[i][0] + suffix_prop_swap;
		    key.toLower(); 
		    LAString data = futureConv_swap[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }
		
	    //swap adjust mode
	    LAString tmpBasisCurveName;
	    if (isswaptenoradjust)
	    {
            AQ_REQUIRE( adjustSwapConv_swap.size() > 0, "Adjustment Basis Swap Conventions Block Missing - Must provide Basis Swap convention settings when isSwapTenorAdjust = True" )
            AQ_REQUIRE( adjustSwapRates_swap.size() > 0, "Adjustment Basis Swap Rates Block Missing - Must provide Basis Swap rates when isSwapTenorAdjust = True" )

		    tmpBasisCurveName = THREESIXBASIS;
		    LAString adjsuffix_prop;
		    LAString adjsuffix_data;
		    if (tmpBasisCurveName != STD)
		    {
			    adjsuffix_prop = "." + tmpBasisCurveName;
			    adjsuffix_prop.toLower();
			    adjsuffix_data = "_" + tmpBasisCurveName;
			    adjsuffix_data.toLower();
		    }
		
		    MLIB_2D_MATRIX_CHECK( adjustSwapConv_swap, "Invalid Basis Swap or AdjustSwapConv Conventions" )
		    for(size_t i=0; i<adjustSwapConv_swap.size(); i++)
		    {
			    LAString key = tmpCurrency + ".sde.yield.basis." + adjustSwapConv_swap[i][0] + adjsuffix_prop;
			    key.toLower(); 
			    LAString data = adjustSwapConv_swap[i][1];
			    if (key.findString("discount") == -1 && key.findString("forecast") == -1) 
			    {
				    data.toLower();
			    }
			    irStaticData.setStaticData(key, data);
		    }
	
		    tmpBasisCurveName.toLower();
		    LAString basisEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + tmpBasisCurveName;

		    //swapfile
		    LAString adjfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + adjsuffix_prop);
		    if (adjfile == AQ_NO_DATA)
		    {
			    adjfile = LAString("data/in/") + tmpCurrency + LAString("_yield_basisswap") + adjsuffix_data + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_FILE + "." + adjsuffix_prop, adjfile);
		    }

		    LAString adjstream;
		    LAString usegrid = "";
		    MLIB_2D_MATRIX_CHECK( adjustSwapConv_swap, "Invalid Basis Swap Rates or AdjustSwapRates Rates" )
		    for(size_t i=0; i<adjustSwapRates_swap.size(); i++)
		    {
			    adjstream += adjustSwapRates_swap[i][0];
			    double adjrate = adjustSwapRates_swap[i][1].getDoubleValue() * 10000.0;
			    adjstream += "," + LAString(adjrate) + LF;

			    if (adjustSwapRates_swap[i].size() == 3)
			    {
				    LAString useGridFrag = adjustSwapRates_swap[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid += adjustSwapRates_swap[i][0] + ":";
				    else usegrid += "NONE:";
			    }
		    }
		    std::istringstream *padjstream = new std::istringstream(adjstream.getCString());
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(adjfile), padjstream);

		    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName);
		    int find;
		    if (usegrid != "") 
		    {
			    usegrid = usegrid.subString(0, usegrid.size() - 2);
			    while ((find = usegrid.findString("NONE:")) != -1) usegrid.remove(find, 5);
			    if ((find = usegrid.findString(":NONE")) != -1) usegrid.remove(find, 5);
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_USEGRID + "." + tmpBasisCurveName, usegrid);
		    }

		    LAString tmpCurveName = tmpBasisCurveName; tmpCurveName.toUpper();
		    tmpUseMarkets =  useMarkets.toToken(MULTI_STATIC_DATA_DELIMITER);
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		    { 
			    useMarkets += LAString(MULTI_STATIC_DATA_DELIMITER) + tmpBasisCurveName;
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_MARKETTYPE + "." + tmpBasisCurveName, MARKETTYPE_BASIS);
	    }

	    LAString usegrid_libor = "",usegrid_swap = "",usegrid_fra = "",usegrid_future = "";

	    //set Libor Object;
	    LAString liborEntityName = tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + suffix_prop_swap;

	    LAString liborfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + suffix_prop_swap);
	    if (liborfile == AQ_NO_DATA)
	    {
		    liborfile = LAString("data/in/") + tmpCurrency + LAString("_yield_libor") + suffix_data_swap + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_FILE + suffix_prop_swap, liborfile);
	    }
	    LAString liborstream;
	    MLIB_2D_MATRIX_CHECK( liborRates_swap, "Invalid Libor Fixing and Reset Rates" )
	    for(size_t i=0; i<liborRates_swap.size(); i++)
	    {
		    liborstream += liborRates_swap[i][0];
		    double lrate = liborRates_swap[i][1].getDoubleValue() * 100.0;
		    liborstream += "," + LAString(lrate) + LF;

		    if (liborRates_swap[i].size() == 3)
		    {
			    LAString useGridFrag = liborRates_swap[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") 
			    {
				    usegrid_libor += liborRates_swap[i][0] + ":";
			    }
			    else 
			    {
				    usegrid_libor += "NONE:";
			    }
		    }
	    }
	    std::istringstream *pliborstream = new std::istringstream(liborstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(liborfile), pliborstream);

	    //set fwd swap
	    bool areSwapsForwardStarting = false;
	    LAString isFwdSwap_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFWDSWAP + suffix_prop_swap);
	    LADataBool tmpAttrB;
	    if (isFwdSwap_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFwdSwap_str);
		    areSwapsForwardStarting = tmpAttrB.get();
	    }

	    //set Swap Object;
	    LAString swapfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_prop_swap);
	    if (swapfile == AQ_NO_DATA)
	    {
		    swapfile = LAString("data/in/") + tmpCurrency + LAString("_yield_swap") + suffix_data_swap + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_prop_swap, swapfile);
	    }
	    LAString swapstream;
	    MLIB_2D_MATRIX_CHECK( swapRates_swap, "Invalid Swap Rates" )
	    for(size_t i=0; i<swapRates_swap.size(); i++)
	    {
		    swapstream += swapRates_swap[i][0];
		    double srate = swapRates_swap[i][1].getDoubleValue() * 100.0;
		    swapstream += "," + LAString(srate);
		    // for fwd swap
		    if (areSwapsForwardStarting)
		    {
			    if (swapRates_swap[i].size() < 5)
			    {
                    throw LACoreInvalidData("FwdSwap size error",__FILE__,__LINE__);
			    }

			    LAString isDate_str = swapRates_swap[i][2];
			    swapstream += "," + isDate_str.toUpper();
			    if (isDate_str == "TRUE") 
			    {
				    const LADate& startdate = stringToDate(swapRates_swap[i][3]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    swapstream += "," + startdate_str;
				    const LADate& enddate = stringToDate(swapRates_swap[i][4]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    swapstream += "," + enddate_str;
			    }
			    else
			    {
				    LAString startterm_str = swapRates_swap[i][3];
				    swapstream += "," + startterm_str.toUpper();
				    LAString tenor_str = swapRates_swap[i][4];
				    swapstream += "," + tenor_str.toUpper();
			    }
			    // set use grid
			    if (swapRates_swap[i].size() == 6)
			    {
				    LAString useGridFrag = swapRates_swap[i][5]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") 
				    {
					    usegrid_swap += swapRates_swap[i][0] + ":";
				    }
				    else 
				    {
					    usegrid_swap += "NONE:";
				    }
			    }
		    }
		    // set use grid
		    else if (swapRates_swap[i].size() == 3)
		    {
			    LAString useGridFrag = swapRates_swap[i][2]; upper(useGridFrag);
			    if (useGridFrag == "TRUE") 
			    {
				    usegrid_swap += swapRates_swap[i][0] + ":";
			    }
			    else 
			    {
				    usegrid_swap += "NONE:";
			    }
		    }
		    swapstream += LF;
	    }
	    std::istringstream *pswapstream = new std::istringstream(swapstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(swapfile), pswapstream);

	    //set FRA Object;
	    bool isFRAUse = false;
	    LAString isFRAUse_str = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFRAUSE + suffix_prop_swap);
	    if (isFRAUse_str != AQ_NO_DATA)
	    {
		    tmpAttrB.convertFromString(isFRAUse_str);
		    isFRAUse = tmpAttrB.get();
	    }
	
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + suffix_prop_swap);

	    if (isFRAUse)
	    {
            AQ_REQUIRE( fraConv_swap.size() > 0, "FRA Conventions Block Missing - Must provide FRA convention settings when IsFRAUse = True" )
            AQ_REQUIRE( fra3mRates_swap.size() > 0 || fra6mRates_swap.size() > 0 , "FRA Rates Block Missing - Must provide FRA instrument data when IsFRAUse = True" )

		    LAString fraFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + suffix_prop_swap);
		    if (fraFile == AQ_NO_DATA)
		    {
			    fraFile = LAString("data/in/") + tmpCurrency + LAString("_yield_fra") + suffix_data_swap + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_FILE + suffix_prop_swap, fraFile);
		    }

		    if (fra3mRates_swap.size() != 0)
		    {
			    LAString fra3mstream = etrading::buildFRAMarketDataFile(fraFile, fra3mRates_swap, areSwapsForwardStarting, usegrid_fra);	

			    std::istringstream *pfra3mstream = new std::istringstream(fra3mstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfra3mstream);
		    }
		    else if (fra6mRates_swap.size() != 0)
		    {
			    LAString fra6mstream = etrading::buildFRAMarketDataFile(fraFile, fra6mRates_swap, areSwapsForwardStarting, usegrid_fra);

			    std::istringstream *pfra6mstream = new std::istringstream(fra6mstream.getCString());
			    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fraFile), pfra6mstream);
		    }
		    else
		    {
			    throw LACoreInvalidData("#Error - No FRA market data is provided",__FILE__,__LINE__);
		    }
	    }

	    //set future Object;
	    bool isFutureUse = false;
	    LAString tmpFutureStr = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_ISFUTUREUSE + suffix_prop_swap);
	    if (tmpFutureStr != AQ_NO_DATA)
	    {
		    LADataBool tmpAttrB;
		    tmpAttrB.convertFromString(tmpFutureStr);
		    isFutureUse = tmpAttrB.get();
	    }
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + suffix_prop_swap);
	    if (isFutureUse)
	    {
            AQ_REQUIRE( futureConv_swap.size() > 0, "Futures Conventions Block Missing - Must provide futures convention settings when IsFutureUse = True" )
            AQ_REQUIRE( futureRates_swap.size() > 0, "Futures Rates Block Missing - Must provide futures instrument data when IsFutureUse = True" )

		    LAString futureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + suffix_prop_swap);
		    if (futureFile == AQ_NO_DATA)
		    {
			    futureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_future") + suffix_data_swap + LAString(".csv");
			    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_FILE + suffix_prop_swap, futureFile);
		    }
		    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(futureFile), createFutureStream(futureRates_swap, usegrid_future));
	    }

	    ///////////////
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_3MFRA_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_6MFRA_USEGRID + suffix_prop_swap);
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + suffix_prop_swap);
	    int find;
	    if (usegrid_libor != "") 
	    {
		    usegrid_libor = usegrid_libor.subString(0, usegrid_libor.size() - 2);
		    while ((find = usegrid_libor.findString("NONE:")) != -1) 
		    {
			    usegrid_libor.remove(find, 5);
		    }

		    if ((find = usegrid_libor.findString(":NONE")) != -1) 
		    {
			    usegrid_libor.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LIBOR_USEGRID + suffix_prop_swap, usegrid_libor);
	    }
	    if (usegrid_swap != "") 
	    {
		    usegrid_swap = usegrid_swap.subString(0, usegrid_swap.size() - 2);
		    while ((find = usegrid_swap.findString("NONE:")) != -1) 
		    {
			    usegrid_swap.remove(find, 5);
		    }

		    if ((find = usegrid_swap.findString(":NONE")) != -1) 
		    {
			    usegrid_swap.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_USEGRID + suffix_prop_swap, usegrid_swap);
	    }
	    if (usegrid_fra != "") 
	    {
		    usegrid_fra = usegrid_fra.subString(0, usegrid_fra.size() - 2);
		    while ((find = usegrid_fra.findString("NONE:")) != -1) 
		    {
			    usegrid_fra.remove(find, 5);
		    }

		    if ((find = usegrid_fra.findString(":NONE")) != -1) 
		    {
			    usegrid_fra.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FRA_USEGRID + suffix_prop_swap, usegrid_fra);
	    }
	    if (usegrid_future != "") 
	    {
		    usegrid_future = usegrid_future.subString(0, usegrid_future.size() - 2);
		    while ((find = usegrid_future.findString("NONE:")) != -1) 
		    {
			    usegrid_future.remove(find, 5);
		    }

		    if ((find = usegrid_future.findString(":NONE")) != -1) 
		    {
			    usegrid_future.remove(find, 5);
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FUTURE_USEGRID + suffix_prop_swap, usegrid_future);
	    }
	
	    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< End - SWAP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



	    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Begin - OIS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	    LAString suffix_prop_OIS("");
	    LAString suffix_data_OIS("");

	    // If the user-given market name is not STD, concatenate it to useMarkets 
	    LAString useMarkets_OIS = useMarkets;
	    if (generateCurveName_OIS != STD)
	    {
		    suffix_data_OIS = "_" + generateCurveName_OIS;

		    suffix_prop_OIS = "." + generateCurveName_OIS;
		    suffix_prop_OIS.toLower();

		    LAString tmpCurveName = generateCurveName_OIS; 
		    tmpCurveName.toUpper();
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),tmpCurveName)) 
		    {
			    if (useMarkets_OIS == "") 
			    {
				    useMarkets_OIS = generateCurveName_OIS;
			    }
			    else 
			    {
				    useMarkets_OIS += LAString(MULTI_STATIC_DATA_DELIMITER) + generateCurveName_OIS;
			    }
		    }	
	    }
	    else
	    {	
		    if (tmpUseMarkets.end() == std::find(tmpUseMarkets.begin(),tmpUseMarkets.end(),SWAP)) 
		    {
			    if (useMarkets_OIS == "") 
			    {
				    useMarkets_OIS = SWAP;
			    }
			    else 
			    {
				    useMarkets_OIS += LAString(MULTI_STATIC_DATA_DELIMITER) + SWAP;
			    }
		    }
	    }

	    if (curveNames_OIS == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_OIS, marketName_OIS);
	    }
	    else
	    {
		    // New code for dual bootstrapping - ensure the implicit curve name is one of the curve index names
		    LAString indexNames = curveNames_OIS;
		    if (indexNames.findString(marketName_OIS) == -1)
		    {
			    indexNames = marketName_OIS + LAString(MULTI_STATIC_DATA_DELIMITER) + indexNames;
		    }

		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_ASSIGNEDCURVE + suffix_prop_OIS, indexNames);
	    }

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + suffix_prop_OIS, "DAILYCOMPOUNDING");

	    // Set curve type
    //	LAString curveType = CURVETYPE_OIS;
    //	irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_OIS, curveType);
	
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DFCURVENAME + suffix_prop_OIS);
	
	    MLIB_2D_MATRIX_CHECK( generateProp_OIS, "Invalid CurveProperties or generateProp Conventions" )
	    for(size_t i=0; i<generateProp_OIS.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + generateProp_OIS[i][0] + suffix_prop_OIS;
		    key.toLower();
		    LAString data = generateProp_OIS[i][1];
		    if (key.findString("dfcurvename") == -1) data.toLower();
		    irStaticData.setStaticData(key, data);
	    }

        // *** Important *** Must remove keys that can accept blank values, since these will not be removed by default
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_SHORTTERMCONVENTION + suffix_prop_OIS );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMCONVENTION + suffix_prop_OIS );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERM + suffix_prop_OIS );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD + suffix_prop_OIS );
		irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_LONGTERMGENMETHOD_ARROIS + suffix_prop_OIS);
		irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDMETHOD + suffix_prop_OIS );
        irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_COMPOUNDINGMETHOD + suffix_prop_OIS );
		irStaticData.removeStaticData( tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_INTERPOLATIONJOINDATE + suffix_prop_OIS);

	    MLIB_2D_MATRIX_CHECK( oisConv_OIS, "Invalid OIS Swap Conventions" )	
	    for(size_t i=0; i<oisConv_OIS.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.ois." + oisConv_OIS[i][0] + suffix_prop_OIS;
		    key.toLower(); 
		    LAString data = oisConv_OIS[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }	
    
	    LAString oisFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix_prop_OIS);
	    if (oisFile == AQ_NO_DATA)
	    {
		    oisFile = LAString("data/in/") + tmpCurrency + LAString("_yield_ois_oiscurve.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_FILE + suffix_prop_OIS, oisFile);
	    }

	    LAString fedFundFutureFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffix_prop_OIS);
	    if (fedFundFutureFile == AQ_NO_DATA)
	    {
		    fedFundFutureFile = LAString("data/in/") + tmpCurrency + LAString("_yield_fffuture.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_FILE + suffix_prop_OIS, fedFundFutureFile);
	    }

	    LAString histFile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix_prop_OIS);
	    if (histFile == AQ_NO_DATA)
	    {
		    histFile = LAString("data/in/") + tmpCurrency + LAString("_yield_historical_ois_oiscurve.csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_HISTORICAL_OIS_FILE + suffix_prop_OIS, histFile);
	    }

	    LAString oisStream;
	    LAString fedFundFutureStream;
	    LAString usegrid_ois = "";
	    LAString usegrid_fffuture = "";

	    MLIB_2D_MATRIX_CHECK( oisRates_OIS, "Invalid OIS Swap Rates" )	
	    for(size_t i=0; i<oisRates_OIS.size(); i++)
	    {
		    if (oisRates_OIS[i][0].findString("FF") != -1)
		    {
			    fedFundFutureStream += oisRates_OIS[i][0];

			    if (oisRates_OIS[i].size() != 3 && oisRates_OIS[i].size() != 5)
				    throw LACoreInvalidData("FF input size error",__FILE__,__LINE__);

			    //in case of FF, quoted value is price
			    double oisRate = oisRates_OIS[i][1].getDoubleValue();
			    fedFundFutureStream += "," + LAString(oisRate);

			    if (oisRates_OIS[i].size() == 5)
			    {
				    const LADate& startdate = stringToDate(oisRates_OIS[i][2]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    fedFundFutureStream += "," + startdate_str;
				    const LADate& enddate = stringToDate(oisRates_OIS[i][3]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    fedFundFutureStream += "," + enddate_str;
			    }

			    fedFundFutureStream += LF;
			
			    if (oisRates_OIS[i].size() == 5)
			    {
				    LAString useGridFrag = oisRates_OIS[i][4]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fffuture += oisRates_OIS[i][0] + ":";
				    else usegrid_fffuture += "NONE:";
			    }
			    else if (oisRates_OIS[i].size() == 3)
			    {
				    LAString useGridFrag = oisRates_OIS[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_fffuture += oisRates_OIS[i][0] + ":";
				    else usegrid_fffuture += "NONE:";
			    }
		    }
		    else
		    {
			    oisStream += oisRates_OIS[i][0];

                // Allow Central Bank Swap instruments to be specified in the OIS curve for all markets ...
                if (oisRates_OIS[i][0].findString("BOJ") != -1          // Bank of Japan
                    || oisRates_OIS[i][0].findString("EUSF") != -1      // European Central Bank (Bloomberg Ticker)
                    || oisRates_OIS[i][0].findString("ECB") != -1       // European Central Bank
                    || oisRates_OIS[i][0].findString("BOE") != -1       // Bank of England
                    || oisRates_OIS[i][0].findString("CB") != -1        // *** GENERIC *** Central Bank Swaps
                    || oisRates_OIS[i][0].findString("MPC") != -1)      // *** GENERIC *** Monetary Policy Committee Swaps
			    {
				    if (oisRates_OIS[i].size() < 4)
					    throw LACoreInvalidData("short term market needs StartDate and EndDate",__FILE__,__LINE__);

				    double oisRate = oisRates_OIS[i][1].getDoubleValue() * 100.0;
				    oisStream += "," + LAString(oisRate);

				    const LADate& startdate = stringToDate(oisRates_OIS[i][2]);
				    LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
				    oisStream += "," + startdate_str;
				    const LADate& enddate = stringToDate(oisRates_OIS[i][3]);
				    LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
				    oisStream += "," + enddate_str;
			    }
			    else //normal case
			    {
				    double oisRate = oisRates_OIS[i][1].getDoubleValue() * 100.0;
				    oisStream += "," + LAString(oisRate);
			    }
			
			    oisStream += LF;
			
			    if (oisRates_OIS[i].size() == 5)
			    {
				    LAString useGridFrag = oisRates_OIS[i][4]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_ois += oisRates_OIS[i][0] + ":";
				    else usegrid_ois += "NONE:";
			    }
			    else if (oisRates_OIS[i].size() == 3)
			    {
				    LAString useGridFrag = oisRates_OIS[i][2]; upper(useGridFrag);
				    if (useGridFrag == "TRUE") usegrid_ois += oisRates_OIS[i][0] + ":";
				    else usegrid_ois += "NONE:";
			    }
		    }
	    }
 	    std::istringstream *pOISStream = new std::istringstream(oisStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(oisFile), pOISStream);
	    std::istringstream *pFedFundFutureStream = new std::istringstream(fedFundFutureStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fedFundFutureFile), pFedFundFutureStream);


	    ///////////////
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveID);

	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix_prop_OIS);
	    if (usegrid_ois != "") 
	    {
		    usegrid_ois = usegrid_ois.subString(0, usegrid_ois.size() - 2);
		    while ((find = usegrid_ois.findString("NONE:")) != -1) usegrid_ois.remove(find, 5);
		    if ((find = usegrid_ois.findString(":NONE")) != -1) usegrid_ois.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_USEGRID + suffix_prop_OIS, usegrid_ois);
	    }
	    if (usegrid_fffuture != "") 
	    {
		    usegrid_fffuture = usegrid_fffuture.subString(0, usegrid_fffuture.size() - 2);
		    while ((find = usegrid_fffuture.findString("NONE:")) != -1) usegrid_fffuture.remove(find, 5);
		    if ((find = usegrid_fffuture.findString(":NONE")) != -1) usegrid_fffuture.remove(find, 5);
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_FFFUTURE_USEGRID + suffix_prop_OIS, usegrid_fffuture);
	    }

	    LAString histStream;
	    MLIB_2D_MATRIX_CHECK( histRates_OIS, "Invalid OIS Fixing and Reset Rates" )
	    for(size_t i=0; i<histRates_OIS.size(); i++)
	    {
		    const LADate& histdate = stringToDate(histRates_OIS[i][0]);
		    LAString histdate_str = histdate.stringWithFormat("YYYYMMDD");
		    histStream += histdate_str;
		    double histRate = histRates_OIS[i][1].getDoubleValue() * 100.0;
		    histStream += "," + LAString(histRate);
		    histStream += LF;
	    }
	    std::istringstream *pHISTStream = new std::istringstream(histStream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(histFile), pHISTStream);

	    MLIB_2D_MATRIX_CHECK( lobasisConv_OIS, "Invalid Libor-OIS Basis Conventions" )
	    for(size_t i=0; i<lobasisConv_OIS.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.basis." + lobasisConv_OIS[i][0] + suffix_prop_OIS;
		    key.toLower(); 
		    LAString data = lobasisConv_OIS[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }	
	
	    LAString lobasisfile = irStaticData.getStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffix_prop_OIS);
	    if (lobasisfile == AQ_NO_DATA)
	    {
		    lobasisfile = LAString("data/in/") + tmpCurrency + LAString("_yield_lobasis") + suffix_prop_OIS + LAString(".csv");
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_LOBASIS_FILE + suffix_prop_OIS, lobasisfile);
	    }
	    LAString lobasisstream;
	    MLIB_2D_MATRIX_CHECK( lobasisRates_OIS, "Invalid Libor-OIS Basis Rates" )
		const size_t lobasisColumnSize = ( lobasisRates_OIS.size() > 0 ) ? lobasisRates_OIS[0].size() : 0;

	    for(size_t i=0; i<lobasisRates_OIS.size(); i++)
	    {
			// Libor-OIS Rates Columns: Term, Rate, LiborType (Optional), UseMarketData (Optional)
		    lobasisstream += lobasisRates_OIS[i][0];
		    const double lobrate = lobasisRates_OIS[i][1].getDoubleValue() * 100.0;
		    lobasisstream += "," + LAString(lobrate);

			// Optional Column 3: LiborType
			if ( lobasisColumnSize > 2 )
			{
				const LAString liborType = lobasisRates_OIS[i][2];
				lobasisstream += "," + liborType;
			}

			// Optional Column 4: UseMarketData
			if ( lobasisColumnSize > 3 )
			{
				const LAString useMarketData = lobasisRates_OIS[i][3];
				lobasisstream += "," + useMarketData;
			}

			lobasisstream += LF;
	    }
	    std::istringstream *plobasisstream = new std::istringstream(lobasisstream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(lobasisfile), plobasisstream);

	    MLIB_2D_MATRIX_CHECK( swapConv_OIS, "Invalid Swap Conventions" )
	    for(size_t i=0; i<swapConv_OIS.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.swap." + swapConv_OIS[i][0] + suffix_prop_OIS;
		    key.toLower(); 
		    LAString data = swapConv_OIS[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key, data);
	    }	

	    // Remove swap par rate market data from OIS curve
	    irStaticData.removeStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_SWAP_FILE + suffix_prop_OIS);
	
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_OIS_ISDUALBOOTSTRAPPING + suffix_prop_OIS, LAString("TRUE"));

	    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< End - OIS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, generateCurveName.toUpper());

	    // Record curve types
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_swap, CURVETYPE_SWAP);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + suffix_prop_OIS, CURVETYPE_OIS);

	    // Record main curve names
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_SWAPCURVENAME + staticDataSuffix, generateCurveName_swap);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_DUALBOOTSTRAP_OISCURVENAME + staticDataSuffix, generateCurveName_OIS);

	    // Record other curve names
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffix_prop_swap, useMarkets_swap);
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_USEMAKETS + suffix_prop_OIS, useMarkets_OIS);


        // ============================== CURVE CALIBRATION ========================================================


        std::unique_ptr<LAObjectPoolBase> objectPoolManager( new UpdateObjectPoolForSDEsAndCurves(currency) );
	    objectPoolManager->loadDualBootstrapCurveDataAndCalibrate(currency, *dataInstance);
        

        // =========================================================================================================
    }


    // Set-up for irserver, create property file stream
    void LAUpdateStaticDataManager::setUpForIRServer()
    {
	    LACoreDataService::initialize();
	    LACoreDataService::setContext(CONTEXT_KEY_ISPRICER, "TRUE");
	    LACoreDataService::setContext(CONTEXT_KEY_ISEXCELREQUEST, "TRUE");
	    LACoreDataService::setContext(ARG_KEY_FILENUM, "");

	    // create property stream
	    std::istringstream *pPropSf = new std::istringstream();
	    std::istringstream *pRPropSf = new std::istringstream();
	    std::istringstream *pGpropSf = new std::istringstream();
	    std::istringstream *pCprofSf = new std::istringstream();
	    // set prop
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName("ir.properties"), pPropSf);
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName("grid.properties"), pGpropSf);
	    // set risk prop
	    LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
		staticData.setStaticData(KEY_RISK_SCENARIO_FILE, "risk.properties");
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName("risk.properties"), pRPropSf);
	    // set max leg num
	    staticData.setStaticData(KEY_DEAL_LEG_MAXNUM, "2");
	    // set polynomial dimension
	    staticData.setStaticData(KEY_SIMULATION_LSMC_BASEFUNCDIM, "2");
	    //set calib prop
	    //this is for avoiding LACoreDataService::getContext(ARG_KEY_NOCALIBTHREAD) == AQ_NO_DATA
	    LACoreDataService::setContext(ARG_KEY_NOCALIBTHREAD, "tmp");
	    staticData.setStaticData(KEY_CALIB_SCENARIO_FILE, "calib.properties"); 
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName("calib.properties"), pCprofSf);
	    staticData.setStaticData(KEY_SDE_CALIB_MODEL, "hw:ptberg");
	    staticData.setStaticData(KEY_SDE_PV_CALIB_TARGET_CURRENCY, "ALL");

	    //set isrealcalibmode //default is false
	    LAStaticData &rprop = LACoreDataService::getStaticDataManager().getRiskStaticData();
	    rprop.setStaticData(RISK_KEY_CALIB_ISREALCALIB,"false");
	    //rprop.setStaticData(RISK_KEY_CALIB_ISREALCALIB,"true");
	
	    //this is for avoiding !LAMarketData::isCalibrateEnd(dIRModelName, objPool) ...
	    LAStaticData &cprop = LACoreDataService::getStaticDataManager().getCalibStaticData();
	    cprop.setStaticData(KEY_CALIB_WAIT_TIME, "10");
	    //seriarize set
	    cprop.setStaticData(KEY_CALIB_SERIALIZE_STATUS, "normal");
	
	    //testisadjusdf
	    LAString tmpstr = LAString("jpy"); 
	    staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	    tmpstr = "usd";
	    staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	    tmpstr = "aud";
	    staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	    tmpstr = "eur";
	    staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");
	    tmpstr = "gbp";
	    staticData.setStaticData(tmpstr + STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF,"false");

	    //digitalcoupon set
	    LAString tmpfxstr = LAString("jpy/usd");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	    tmpfxstr = LAString("jpy/aud");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	    tmpfxstr = LAString("jpy/eur");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");

	    tmpfxstr = LAString("jpy/gbp");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_ISCALLSPREAD, "true");
	    staticData.setStaticData(tmpfxstr + FX_KEY_DEAL_DIGITALCOUPON_CALLSPREADVALUE, "0.01");
    }

    void 
    LAUpdateStaticDataManager::
    setUpDefaultIRStaticData(LADataInstance& dataInstance)
    {
	    const LAString* filepath = etrading::FolderConfig::ir_prop_path();
    
        // Track if Property Files have been loaded
        LAStaticDataImport::getInstance().setIsStaticDataLoaded( ( filepath != NULL ) ? true : false );
        
        if(filepath==NULL) return;

	    std::ifstream fin;
	    fin.open(filepath->getCString());
	    if (!fin){
		    std::stringstream sst;
            sst << "#Error: Cannot open ir properties configuration file" << std::endl 
			    << filepath
			    ;
		    throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
	    }

        LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
	    std::string line;
	    size_t line_num = 0;
	    while (getline(fin, line)){
		    line_num++;
		    const char *c_line = line.c_str();
		    LAString tmpstr(c_line);
		    LAStringVector tmp = tmpstr.toToken('=');
		    if (tmp.size() != 2){
			    std::stringstream sst;
                sst << "#Error: ir properties file format is invalid" << std::endl
				    << "file : " << filepath << std::endl
				    << "line : " << line_num << std::endl
				    << "contents : " << line
				    ;
			    throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		    }

		    irStaticData.setStaticData(tmp[0],tmp[1]);
	    }
	    fin.close();
    }

    void 
    LAUpdateStaticDataManager::
    setUpDefaultIRStaticData(LADataInstance& dataInstance, const LAString& filepath)
    {
        etrading::FolderConfig::set_ir_prop_path(filepath);  // creates a copy so no need for a double copy
        setUpDefaultIRStaticData(dataInstance);
    }

    void 
    LAUpdateStaticDataManager::
    setUpDefaultCalibStaticData(LADataInstance& dataInstance)
    {
	    const LAString* filepath = etrading::FolderConfig::calib_prop_path();
    
        // Track if Property Files have been loaded
        LAStaticDataImport::getInstance().setIsStaticDataLoaded( ( filepath != NULL ) ? true : false );
    
        if(filepath==NULL) return;

	    std::ifstream fin;
	    fin.open(filepath->getCString());
	    if (!fin){
		    std::stringstream sst;
		    sst << "cannot open calib properties file" << std::endl 
			    << filepath
			    ;
		    throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
	    }

        LAStaticData &calibprop = LACoreDataService::getStaticDataManager().getCalibStaticData();
	    std::string line;
	    size_t line_num = 0;
	    while (getline(fin, line)){
		    line_num++;
		    const char *c_line = line.c_str();
		    LAString tmpstr(c_line);
		    LAStringVector tmp = tmpstr.toToken('=');
		    if (tmp.size() != 2){
			    std::stringstream sst;
			    sst << "calib proerties file format is invalid" << std::endl
				    << "file : " << filepath << std::endl
				    << "line : " << line_num << std::endl
				    << "contents : " << line
				    ;
			    throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		    }

		    calibprop.setStaticData(tmp[0],tmp[1]);
	    }
	    fin.close();
    }

    void 
    LAUpdateStaticDataManager::
    setUpDefaultCalibStaticData(LADataInstance& dataInstance, LAString filepath)
    {
        etrading::FolderConfig::set_calib_prop_path(filepath);
        setUpDefaultCalibStaticData(dataInstance);
    }
    /*!
        @brief setup for irserver

	    create property file stream

    */
    void
    LAUpdateStaticDataManager
    ::resetService()
    {
	    LACoreDataService::finalize();
	    setUpForIRServer();
    }

    std::istringstream* 
    LAUpdateStaticDataManager::createFutureStream(const LAStringMatrix& future_rates, LAString& usegrid_future)
    {
        LAString futureStream;
        size_t adjustDataMinimumColumns = 3;
        AQ_MATRIX_CHECK( future_rates, adjustDataMinimumColumns, "Invalid Futures Data" )
        for(size_t i=0; i<future_rates.size(); i++)
        {
            if (future_rates[i].size() <= 4)
            {
                if (future_rates[i][0].size() < 3)
                    throw LACoreInvalidData("Invalid Futures Data: TICKER code should have 4 characters.",__FILE__,__LINE__);

                futureStream += future_rates[i][0];
                double futureRate = future_rates[i][1].getDoubleValue();
                futureStream += "," + LAString(futureRate);
                double futureVol = future_rates[i][2].getDoubleValue();
                futureStream += "," + LAString(futureVol) + LF;

                if (future_rates[i].size() == 4)
                {
                    LAString includeFuture = future_rates[i][3]; upper(includeFuture);
					if ( includeFuture == "TRUE" )
					{
						usegrid_future += future_rates[i][0] + ":";
					}
					else if ( includeFuture == "FALSE" )
					{
						usegrid_future += "NONE:";
					}
					else
					{
						throw LACoreInvalidData("Invalid Futures Data: In 4-column format, expected a boolean flag in column 4. Expected format: TICKER  PRICE  VOL/CONV  USE", __FILE__, __LINE__);
					}
                }
            }
            else if (future_rates[i].size() <= 6)
            {
                futureStream += future_rates[i][0];
				const LADate& startdate = stringToDate(future_rates[i][1]);
                LAString startdate_str = startdate.stringWithFormat("YYYYMMDD");
                futureStream += "," + startdate_str;
				const LADate& enddate = stringToDate(future_rates[i][2]); 
                LAString enddate_str = enddate.stringWithFormat("YYYYMMDD");
                futureStream += "," + enddate_str;
                double futureRate = future_rates[i][3].getDoubleValue();
                futureStream += "," + LAString(futureRate);
                double futureVol = future_rates[i][4].getDoubleValue();
                futureStream += "," + LAString(futureVol) + LF;

                if (future_rates[i].size() == 6)
                {
                    LAString includeFuture = future_rates[i][5]; upper(includeFuture);
					if ( includeFuture == "TRUE" )
					{
						usegrid_future += future_rates[i][0] + ":";
					}
					else if ( includeFuture == "FALSE" )
					{
						usegrid_future += "NONE:";
					}
					else
					{
						throw LACoreInvalidData("Invalid Futures Data: In 6-column format, expected a boolean flag in column 6. Expected format: TICKER  STARTDATE  ENDDATE  PRICE  VOL/CONV  USE", __FILE__, __LINE__);
					}
                }
            }
        }
        return new std::istringstream(futureStream.getCString());
    }

    /*!
        @brief set value to property accessor

	    @param[in,out] prop
	    @param[in] key
	    @param[in] value
	    @param[in] override flag

    */
    void
    LAUpdateStaticDataManager::setStaticDataValue(LAStaticData &staticData, const LAString &key, const LAString &val, const bool is_override)
    {
	    if (is_override)
	    {
		    staticData.setStaticData(key , val);
	    }
	    else
	    {
		    const LAString orig = staticData.getStaticData(key);
		    if (orig == AQ_NO_DATA)
		    {
			    staticData.setStaticData(key , val);
		    }
	    }
    }

    void
    LAUpdateStaticDataManager::SetUpFundingSpread(LADataInstance* dataInstance, const LAStringMatrix &fundingSpread)
    {
	    // save in string stream
	    LAString stream;
	    MLIB_2D_MATRIX_CHECK( fundingSpread, "Invalid Funding Spreads")
	    for (unsigned int i = 0; i < fundingSpread.size(); ++i)
	    {
		    stream += fundingSpread[i][0];
		    stream += "," + fundingSpread[i][1] + LF;
	    }
	    LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	    LAString fdspdfile = staticData.getStaticData(KEY_FUNDINGSPREAD_FILE);
	    std::istringstream *pstream = new std::istringstream(stream.getCString());
	    LACoreDataService::setIStringStream(LAMarketData::getNumFileName(fdspdfile), pstream);
    }

    void
    LAUpdateStaticDataManager::CalcMeanAndCovariance(const DoubleMatrix& data, DoubleArray& mean, DoubleMatrix& covar)
    {
	    size_t ValNum = data.size();
	    size_t ObsNum = data[0].size();
	    mean.resize(ValNum);
	    for (size_t j = 0; j < ValNum; j++)
	    {
		    double tmp = 0.;
		    for(size_t k = 0; k < ObsNum; k++)
		    {
			    tmp += data[j][k];
		    }
		    mean[j] = tmp / static_cast<double>(ObsNum);
	    }

	    covar.resize(ValNum);
	    for (size_t i = 0; i < ValNum; i++)
	    {
		    covar[i].resize(ValNum);
		    for(size_t j = i; j < ValNum; j++)
		    {
			    double tmp = 0.;
			    for(size_t k = 0; k < ObsNum; k++)
			    {
				    tmp += (data[i][k] - mean[i]) * (data[j][k] - mean[j]);
			    }
			    covar[i][j] = tmp / static_cast<double>(ObsNum);
		    }
	    }

	    for (size_t i = 0; i < ValNum; i++)
		    for(size_t j = i; j < ValNum; j++)
			    covar[j][i] = covar[i][j];


    }

    void
    LAUpdateStaticDataManager::SetUpPCA(LADataInstance* dataInstance, const DoubleMatrix& corr, const size_t no_factors, const LAString& id)
    {

	    LAObjectPool& objPool = dataInstance->getObjectPool();
	    LAMathCorrelation* cor_obj = NULL;
	    LAString name = PREFIX_COR + id;
	    if(!objPool.getObject(name).isDefined())
	    {
		    cor_obj = new LAMathCorrelation(dataInstance);
		    objPool.set(name, cor_obj);
	    }
	    else
	    {
		    cor_obj = &dynamic_cast<LAMathCorrelation &>(objPool.getObject(name).get());
		    cor_obj->reset();
	    }

	    if (no_factors > corr.size())
		    throw LACoreInvalidData("factornumber is bigger than correlation size", __FILE__, __LINE__);

	    DoubleArray dummy_tgrid(corr.size());
	    for(size_t i = 0; i < corr.size(); i++)
	    {
		    dummy_tgrid[i] = static_cast<double>(i + 1); 
	    }
	    cor_obj->getTGrid().set(dummy_tgrid);

	    cor_obj->getIsMultiVol().set(false);

	    dynamic_cast<LADataInt&>(cor_obj->getData(IR_CALIBRATION_DATA_FACTORNUM_AFTER, ISDEFINED).get()).set(no_factors);
	    dynamic_cast<LADataBool&>(cor_obj->getData(IR_CALIBRATION_DATA_ISOPTIM, ISDEFINED).get()).set(false);

	    cor_obj->setCorrelation(corr);
	    cor_obj->calcFactorLoading();
    }

    DoubleMatrix
    LAUpdateStaticDataManager::GetPCAResult(LADataInstance* dataInstance, const LAString& type, const LAString& id)
    {
	    LAObjectPool& objPool = dataInstance->getObjectPool();
	    LAString name = PREFIX_COR + id;
	    LAMathCorrelation cor_obj = dynamic_cast<LAMathCorrelation& >(objPool.getObject(name, ENCHKTYPE_ISDEFINED).get());

	    DoubleMatrix ret;
	    if(type == LAString("EIGEN_VECTORS"))
	    {
		    ret = dynamic_cast<LADataDoubleMatrix& >(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVECTORS, ISNOTNULL).get()).get();
	    }
	    else if(type == LAString("EIGEN_VALUES"))
	    {
		    DoubleArray tmp = dynamic_cast<LADataDoubles&>(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVALUES, ISNOTNULL).get()).get();
		    ret.push_back(tmp);
	    }
	    else if (type == LAString("POV"))
	    {
		    DoubleArray tmp = dynamic_cast<LADataDoubles&>(cor_obj.getData(IR_CALIBRATION_DATA_EIGENVALUES, ISNOTNULL).get()).get();
		    double sum_tmp = 0.;
		    for(unsigned int i = 0; i < tmp.size(); i++)
			    sum_tmp += tmp[i];
		    for(unsigned int i = 0; i < tmp.size(); i++)
			    tmp[i] /= sum_tmp;
		    ret.push_back(tmp);
	    }
	    else
	    {
		    LAString msg = LAString("Unknown result type: ") + type;
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	    }

	    return ret;

    }

    /* @brief			Calibrate a cheapest-to-deliver curve
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		curveName			Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		collateralCurves	The group of collateral curves out of which the CTD curve is constructed
    */
    void LAUpdateStaticDataManager::setUpCheapestToDeliverCurve(LADataInstance* dataInstance,
												     const LAString& curveCollection, 
												     const LAString& curveName, 
												     const LAString& curveIndexes, 
												     const LAStringMatrix& curveConv,
												     const LAStringVector& collateralCurves)
    {
	    LAStaticData &irStaticData = LACoreDataService::getStaticDataManager().getStaticData();
	    LAObjectPool &objPool = dataInstance->getObjectPool();

	    LAStringMatrix tmpInfo = curveConv;
	    upper(tmpInfo);

	    LADate asofdate = stringToDate(chgrow(tmpInfo,CURVEINPUT_ASOFDATE,1));
        LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofdate.stringWithFormat());
	
	    LAString currency;
	    LAObjectHolder objHolder = objPool.getObject(curveCollection, ENCHKTYPE_NOCHECK);
	    if (objHolder.isDefined())
	    {
		    LAObject& yldEntity = objHolder.get();
		    const LADataHolder* dh = &(yldEntity.getData(IR_CALIBRATION_DATA_CURRENCY));
		    if(dh->isDefined() && !dh->isNull())
		    {
			    currency = (dynamic_cast<const LADataString&> (dh->get())).get();
		    }
		    else
		    {
			    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
			    {
				    currency = LAString("DUMMY");
			    }
			    else
			    {
				    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
			    } 		
		    }
	    }
	    else
	    {
		    if (LAFunctionUtilities::findRowsNumber(tmpInfo,CURVEINPUT_CURRENCY) < 0)
		    {
			    currency = LAString("DUMMY");
		    }
		    else
		    {
			    currency = chgrow(tmpInfo,CURVEINPUT_CURRENCY,1);
		    } 	
	    }
	    LAString tmpCurrency = currency; 
	    tmpCurrency.toLower();
	
	    //set market rate	
	    if (curveName == "" || curveName == STD) 
	    {
		    throw LACoreInvalidData("#Error: Do not use STD or blank for CTD curve name!",__FILE__,__LINE__);
	    }

	    LAString staticDataSuffix;	
	    staticDataSuffix = "." + curveName;
	    staticDataSuffix.toLower();
		
	    if (curveIndexes == "")
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveName);
	    }
	    else
	    {
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_BASIS_ASSIGNEDCURVE + staticDataSuffix, curveIndexes);
	    }
	
	    // Set curve type
	    LAString curveType = CURVETYPE_CHEAPESTTODELIVER;
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_CURVETYPE + staticDataSuffix, curveType);

	    // general info
	    MLIB_2D_MATRIX_CHECK( curveConv, "Invalid Curve Properties or Curve Conventions")
	    for(size_t i=0; i<curveConv.size(); i++)
	    {
		    LAString key = tmpCurrency + ".sde.yield.generator." + curveConv[i][0] + staticDataSuffix;
		    key.toLower();
		    LAString data = curveConv[i][1];
		    data.toLower();
		    irStaticData.setStaticData(key,data);
	    }

	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATECURVEID, curveCollection);

	    // set collateral curves
	    if (collateralCurves.size() == 0)
	    {
		    throw LACoreInvalidData("#Error: Must provide at least one CSA curve",__FILE__,__LINE__);
	    }
	    else
	    {
		    LAString collCurves;
		    for (unsigned int i = 0; i < collateralCurves.size(); ++i)
		    {
			    collCurves += (i == 0? LAString("") : LAString(MULTI_STATIC_DATA_DELIMITER)) + collateralCurves[i];
		    }
		    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_CTD_COLLATERALCURVES + staticDataSuffix, collCurves);
	    }

	    // store market name
	    LAString tmpCurveName = curveName; 
	    tmpCurveName.toUpper();
	    irStaticData.setStaticData(tmpCurrency + STATIC_DATA_KEY_YIELD_GENERATOR_TARGET, tmpCurveName);	
	
	    LAUpdateCurveObject *entityPoolManager = new UpdateObjectPoolForSDEsAndCurves(currency);
	    entityPoolManager->loadCheapestToDeliverCurveDataAndCalibrate(currency, *dataInstance);
	    delete entityPoolManager;

	    //ylddata
	    LAString	CurveIDTool			= curveCollection + TOOL;
	    LAString	CurveIDManager		= curveCollection + curveName + MANAGER;
	    //curve entities manager (curve, grids, curveinformation entities)
	    LAObject* mae = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    mae = new LAObject;
		    objPool.set(CurveIDManager,mae);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    mae	= &objPool.getObject(CurveIDManager).get();
	    }
	    mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	    mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);
    }

}