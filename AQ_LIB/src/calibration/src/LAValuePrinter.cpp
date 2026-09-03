/*! @file
    @brief Value print class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAValuePrinter.cpp
//
//  DESCRIPTION :      Value print class
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


#include <algorithm>
#include "LAValuePrinter.h"
#include "LADataInstance.h"
#include "LACoreFunctionHolder.h"
#include "LAMathCalendar.h"
#include "LAMathCalendarSet.h"
#include "LACoreTemplateType.h"
#include "LADataBasics.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataCalendar.h"
#include "LADataValuation.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LADataReference.h"
#include "LAMathDateCalculations.h"
#include "LAPricePortfolioValue.h"
#include "LAPriceTradeValue.h"
#include "LAMathPathEntity.h"
#include "LAPriceDataDayCount.h"
#include "LAMathValuableEntity.h"
#include "LAPriceAccruedInterest.h"
#include "LAObjectConfiguration.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LAStaticDataManager.h"
#include "LAMarketData.h"
#include "LAPriceLSMCTradeValue.h"
#include "LALogManager.h"
#include "LALogger.h"
#include "LALinearRatesOptionValue.h"
#include "LAPriceConvergenceValue.h"
#include "LAFileAccessor.h"
#include "LADataMatrix.h"

#include <numeric>

#include "LADefinitionsCalibration.h"

#define P_DBL_MAX 1.0e+44
using namespace std;


// constructor
/*!

*/
MAValuePrinter::MAValuePrinter(void)
{
}

// destructor
/*!

*/
MAValuePrinter::~MAValuePrinter(void)
{
}




// 
/*!
    @brief print cashflow

	if all success return 0 
	else if all false return 1 
	else return 2
	
	@param[in] mainTradeNameVec
	@param[in] vector of fileNum
	@param[in] vector of dataInstance pointer
	@param[in] vector of ccy
	@return int
*/
int 
MAValuePrinter::printCF(const vector<LAString> &mainTradeVec, const vector<LAString> &fileNumVec,
					    const vector<LADataInstance *> &rootVec, const vector<LAString> &ccyVec, LAString calc)
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString outFileName = staticData.getStaticData(KEY_DEAL_CF_FILE);
	LADataBool isDetail;

	// ! Check isDataOut
	if ( LACoreDataService::getContext( ARG_KEY_RESULTOUT ) == MLIB_NO_DATA )
	{
		return SUCCESS_CODE;
	}

	const bool flag_print_compounded_rate = LACoreDataService::getContext(ARG_KEY_COMPOUNDEDRATEOUT) != MLIB_NO_DATA;


	bool isvanilla = false;
	if (calc.size() != 0 && calc.toUpper() == "VANILLA" )
	{
		isvanilla = true;
		isDetail.set(false);
	}
	else
	{
		isDetail.set(false);
	}

	int e_spos = outFileName.findString('.');
	LAString extension = outFileName.subString(e_spos , outFileName.size() - 1);
	LAString outFileName_no_ex = outFileName.subString(0, e_spos - 1);

	const int portNum = mainTradeVec.size();
	for (int i = 0; i < portNum; ++i)
	{
		LACoreDataService::setContext(ARG_KEY_MAINTRADE, mainTradeVec[i]);
		LACoreDataService::setContext(ARG_KEY_CURRENCY, ccyVec[i]);

        if(LACoreDataService::getStaticDataManager().getStaticData().getStaticData(KEY_DEAL_FXOPT_BULK) != MLIB_NO_DATA)
        {
            MAValuePrinter::printFxOptionMatrix(rootVec[i], fileNumVec[i]);
            continue;
        }


		LAString outFileName_ = outFileName_no_ex + fileNumVec[i] + extension;
		ofstream outFile(outFileName_.getCString());

		LAObjectHolder objHolder = rootVec[i]->getObjectPool().getObject(mainTradeVec[i], ENCHKTYPE_ISDEFINED);
		if (dynamic_cast<const LADataValuation &>
			(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
		{
			// if portfolio value output each deal
			const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
				(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
			const int dealSize = unders.getSize();

			const LADataHolder* ah_c;
			//get summary property
			LAStaticData &irProp = LACoreDataService::getStaticDataManager().getStaticData();
			LAString portName = mainTradeVec[i];
			if (staticData.getStaticData(KEY_DEAL_SUMMARY_INFO + LAString(".")  + portName.toLower()) != MLIB_NO_DATA)
			{
				LAStringVector summaryInfo = staticData.getStaticData(KEY_DEAL_SUMMARY_INFO + LAString(".")  + portName.toLower()).toToken(';');
				LAStringVector tradeNames = irProp.getStaticData(KEY_DEAL_OUTPUT_NAME + LAString(".")  + portName.toLower()).toToken(';');
				if (summaryInfo.size() != tradeNames.size())
					throw LACoreInvalidData("MAValuePrinter::printValue failed! Sizes of summary information are inconsistent!",__FILE__,__LINE__);
				for (unsigned int j = 0; j < summaryInfo.size(); ++j)
				{
					LAStringVector summaryTrades = summaryInfo[j].toToken(':');
					outFile << tradeNames[j].getCString() << " PV break down" << endl;
					// print pvs for breaking down sum value
					LAString output_pv = "";
					output_pv += "TransID,";
					output_pv += "DirtyPrice,";
					output_pv += "PVCurrency,";
					output_pv += "Leg1PV,";
					output_pv += "Leg1PVCurrency,";
					output_pv += "Leg1TodayFX,";
					output_pv += "Leg2PV,";
					output_pv += "Leg2PVCurrency,";
					output_pv += "Leg2TodayFX,";
					outFile << output_pv.getCString() << endl;

					for (unsigned int k = 0; k < summaryTrades.size(); ++k)
					{
						const LAObject* e = &rootVec[i]->getObjectPool().getObject(summaryTrades[k], ENCHKTYPE_ISDEFINED).get();
						output_pv = "";
						//TransID
						output_pv += dynamic_cast<const LADataString &>(e->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
						output_pv += ",";
						//dirty price
						double dirtyPrice = dynamic_cast<const LADataDouble &>(e->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL).get()).get();
						output_pv += outStr(dirtyPrice);
						output_pv += ",";
						//pv currency
						LAString ccy_pv;
						ah_c = &e->getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK);
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							ccy_pv = dynamic_cast<const LADataString &>(ah_c->get()).get();
							
						}
						else
						{
							ccy_pv = dynamic_cast<const LADataString &>(e->getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
						}
						output_pv += ccy_pv;
						output_pv += ",";
						//leg info
						const LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
						const bool isMovePrefixingCF = staticData.getStaticData(KEY_DEAL_MTMCS_IS_MOVE_PREFIXING_PV).toLower() == "true";
						ah_c = &e->getData(PRICING_DATA_PV_LEG1, NOCHECK);
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							double pvLeg1 = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
							ah_c = &e->getData(PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT, NOCHECK);
							if (ah_c->isDefined() && !ah_c->isNull() && isMovePrefixingCF)
							{
								double pvLeg1PrefixingAdjustment = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
								pvLeg1 += pvLeg1PrefixingAdjustment;
							}
							output_pv += outStr(pvLeg1);
						}
						output_pv += ",";
						ah_c = &e->getData(PRICING_DATA_CURRENCY_LEG1, NOCHECK);
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							output_pv += dynamic_cast<const LADataString &>(ah_c->get()).get();
						}
						output_pv += ",";
						ah_c = &e->getData(PRICING_DATA_TODAYFX_LEG1CCY, NOCHECK);
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							output_pv += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
						}
						output_pv += ",";
						ah_c = &e->getData(PRICING_DATA_PV_LEG2, NOCHECK);
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							double pvLeg2 = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
							ah_c = &e->getData(PRICING_DATA_PV_LEG2_PREFIXING_ADJUSTMENT, NOCHECK);
							if (ah_c->isDefined() && !ah_c->isNull() && isMovePrefixingCF)
							{
								double pvLeg2PrefixingAdjustment = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
								pvLeg2 += pvLeg2PrefixingAdjustment;
							}
							output_pv += outStr(pvLeg2);
						}
						output_pv += ",";
						ah_c = &e->getData(PRICING_DATA_CURRENCY_LEG2, NOCHECK);
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							output_pv += dynamic_cast<const LADataString &>(ah_c->get()).get();
						}
						output_pv += ",";
						ah_c = &e->getData(PRICING_DATA_TODAYFX_LEG2CCY, NOCHECK);
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							output_pv += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
						}
						output_pv += ",";

						outFile << output_pv.getCString() << endl;
					}

					outFile << endl;
				}
			}

			for (int j = 0; j < dealSize; ++j)
			{
				const LAString &name = dynamic_cast<const LADataString &>( unders.get(j).getData( CALIBRATION_DATA_NAME, ISNOTNULL ).get() ).get();

				if ( isvanilla )
				{
					// ! print option parameter
					LAStringVector paramNames;
					ah_c = &( unders.get(j).getData( PRICING_DATA_ANALYTICPARAMNAME, NOCHECK ) );
					if ( ah_c->isDefined() && !ah_c->isNull() )
					{
						paramNames = dynamic_cast< const LADataStrings& >( ah_c->get() ).get();
					}

					DoubleVector params;
					ah_c = &( unders.get(j).getData( PRICING_DATA_ANALYTICPARAM, NOCHECK ) );
					if ( ah_c->isDefined() && !ah_c->isNull() )
					{
						params = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
					}

					LAString output_op = "";
					if ( paramNames.size() > 0 )
					{
						output_op += name + "_AnalyticParams";
						outFile << output_op << endl;
					}

					output_op = "";
					for ( size_t k = 0; k < paramNames.size(); k++ )
					{
						output_op += paramNames[k];
						output_op += ",";
					}
					outFile << output_op << endl;

					output_op = "";
					for ( size_t k = 0; k < params.size(); k++ )
					{
						output_op += outStr( params[k] );
						output_op += ",";
					}
					outFile << output_op << endl;
					outFile << endl;

					// ! print leg cashflow

					ah_c = &( unders.get(j).getData( CALIBRATION_DATA_UNDERLYINGS, NOCHECK ) );
					if ( !ah_c->isDefined() || ah_c->isNull() )
					{
						// ! if leg does not exist, continue
						continue;
					}
					const size_t legNum = dynamic_cast< const LADataMultiReference& >( ah_c->get() ).getSize();

					// ! loop for leg 1, 2,,
					for ( size_t k = 1; k < legNum + 1; k++ )
					{
						const LAObject* e;
						const int num = k;

						e = &( unders.get(j).get() );

						LAString dataName;

						// ! CFCalcStartDates
						DateVector cfStartDates;
						dataName = PRICING_DATA_CFCALCSTARTDATE_LEG + LAString( num ) ;

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfStartDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! CFCalcEndDates
						DateVector cfEndDates;
						dataName = PRICING_DATA_CFCALCENDDATE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfEndDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! CFPaymentDates
						DateVector cfPaymentDates;
						dataName = PRICING_DATA_PAYMENTDATE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfPaymentDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! PaymentTimings
						DoubleVector times;
						dataName = PRICING_DATA_CASHLETVALUETIME_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							times = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! CFs
						DoubleVector cfs;
						dataName = PRICING_DATA_CASHLETVALUE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! PVs
						DoubleVector pvs;
						dataName = PRICING_DATA_PVVALUE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							pvs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! DFs
						DoubleVector dfs;
						dataName = PRICING_DATA_DF_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							dfs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! FRADFs
						DoubleVector fradfs;
						dataName = PRICING_DATA_FRADF_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							fradfs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}
						
						// ! Gearings
						DoubleVector gearings;
						dataName = PRICING_DATA_GEARING_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							gearings = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! Forwards
						DoubleVector forwards;
						dataName = PRICING_DATA_FORWARD_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							forwards = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! Spreads
						DoubleVector spreads;
						dataName = PRICING_DATA_SPREAD_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							spreads = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! Strikes
						DoubleVector strikes1;
						dataName = PRICING_DATA_STRIKE_LEG + LAString( num ) + LAString("_") + LAString( 1 );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							strikes1 = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						DoubleVector strikes2;
						dataName = PRICING_DATA_STRIKE_LEG + LAString( num ) + LAString("_") + LAString( 2 );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							strikes2 = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! AccrualDays
						DoubleVector accrualDays;
						dataName = PRICING_DATA_ACCRUALDAYS_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							accrualDays = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! Notionals
						DoubleVector notionals;
						dataName = PRICING_CALIBRATION_DATAOTIONAL_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							notionals = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! Volatilities
						DoubleVector volatilities1;
						dataName = PRICING_DATA_VOLATILITY_LEG + LAString( num ) + LAString("_") + LAString( 1 );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							volatilities1 = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						DoubleVector volatilities2;
						dataName = PRICING_DATA_VOLATILITY_LEG + LAString( num ) + LAString("_") + LAString( 2 );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							volatilities2 = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! ExpiryTerms
						DoubleVector expiryTerms;
						dataName = PRICING_DATA_EXPIRYTERM_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							expiryTerms = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! ExtraCFTime
						DoubleVector extracfTimes;
						dataName = PRICING_DATA_EXTRACFVALUETIME_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							extracfTimes = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! ExtraCF
						DoubleVector extracfs;
						dataName = PRICING_DATA_EXTRACFVALUE_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							extracfs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! ExtraPV
						DoubleVector extrapvs;
						dataName = PRICING_DATA_EXTRACFPVVALUE_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							extrapvs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! FixingDates
						DateVector fixingDates;
						dataName = PRICING_DATA_FIXINGDATE_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							fixingDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! FixingFlags
						LAStringVector fixingFlags;
						dataName = PRICING_DATA_FIXINGFLAG_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							fixingFlags = dynamic_cast< const LADataStrings& >( ah_c->get() ).get();
						}

						// ! NotionalCF
						DoubleVector notionalcfs;
						dataName = PRICING_CALIBRATION_DATAOTIONALCF_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							notionalcfs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}
						// ! RenotionalFixingDates
						DateVector renotionalfixingDates;
						dataName = PRICING_DATA_RENOTIONALFIXINGDATE_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							renotionalfixingDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! ConvexityAdjusts
						DoubleVector convexityAdjusts;
						dataName = PRICING_DATA_CONVEXITYADJUST_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							convexityAdjusts = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! ConvexityAdjustVols
						DoubleVector convexityAdjustVols;
						dataName = PRICING_DATA_CONVEXITYADJUSTVOL_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							convexityAdjustVols = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! TodayFX from settlement currency to non-deliverable currency (used only for NDS)
						DoubleVector fx_settle_nondeliv_vec;
						dataName = DATAN_N_MV_IR_TODAYFX_SETTLEMENTTONONDELIVERABLE + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							fx_settle_nondeliv_vec = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! SettlementFixingDates
						DateVector settlefixingDates;
						dataName = PRICING_DATA_SETTLEMENTFIXINGDATE_LEG + LAString(num);
						ah_c = &(e->getData(dataName, NOCHECK));
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							settlefixingDates = dynamic_cast< const LADataDates& >(ah_c->get()).get();
						}

						// ! SettlementAdjustRatios
						DoubleVector settleAdjustRatios;
						dataName = PRICING_DATA_SETTLEMENTADJUSTRATIO_LEG + LAString(num);
						ah_c = &(e->getData(dataName, NOCHECK));
						if (ah_c->isDefined() && !ah_c->isNull())
						{
							settleAdjustRatios = dynamic_cast< const LADataDoubles& >(ah_c->get()).get();
						}

						// ! Output
						LAString output = "";
						if ( times.size() > 0 ) 
						{
							// ! output tag
							output = name + "_Cashflow_Leg" + LAString( num );
							//output += k;

							outFile << output.getCString() << endl;

							// ! output indices
							output = "";

							output += "CFCalcStartDate,";			
							output += "CFCalcEndDate,";	
							output += "AccrualDays,";	
							output += "Notional,";
							output += "FixingDate,";
							output += "FixingFlag,";
							if ( convexityAdjustVols.size() > 0 )
							{
								output += "ConvAdjVol,";					
								output += "ConvAdjust,";								
							}
							if ( gearings.size() > 0 )
							{
								output += "Gearing,";	
							}
							output += "Forward/FixedRate,";
							if ( spreads.size() > 0 )
							{
								output += "Spread,";
							}
							output += "PaymentDate,";					
							output += "PaymentTime,";					
							output += "Cashflow,";		

							if (settlefixingDates.size() > 0)
							{
								output += "SettlementFixingDate,";
							}

							if (settleAdjustRatios.size() > 0)
							{
								output += "SettlementAdjustRatio,";
							}

							output += "DF,";
							if ( fradfs.size() > 0 )
							{
								output += "FRADF,";
							}

							if ( fx_settle_nondeliv_vec.size() > 0 )
							{
								output += "PV_Settle,";
								output += "FX_SettleToNonDeliv,";
							}
							output += "PV,";

							if ( expiryTerms.size() > 0 )
							{
								output += "ExpiryTerm,";	
							}
							if ( volatilities1.size() > 0 )
							{
								output += "Volatility1,";
							}
							if ( volatilities2.size() > 0 )
							{
								output += "Volatility2,";
							}
							if ( strikes1.size() > 0 )
							{
								output += "Strike1,";
							}
							if ( strikes2.size() > 0 )
							{
								output += "Strike2,";
							}

							if ( extracfTimes.size() > 0 )
							{
								output += "ExtraCFTime,";					
								output += "ExtraCF,";					
								output += "ExtraCFPV,";
							}

							if (renotionalfixingDates.size() > 0)
							{
								output+= "RenotionalFixingDate,";
							}
							if (notionalcfs.size() > 0)
							{
								output+= "NotionalCF,";
							}

							outFile << output.getCString() << endl;
						}

						LADate d_date;
						// ! output contents	
						for ( size_t i = 0; i < times.size(); i++ )
						{
							output = "";

							if ( i < cfStartDates.size() && d_date != cfStartDates[i] )
								output += cfStartDates[i].stringWithFormat();
							output += ",";

							if ( i < cfEndDates.size() && d_date != cfEndDates[i] )
								output += cfEndDates[i].stringWithFormat();
							output += ",";

							if ( i < accrualDays.size() )
								output += outStr( accrualDays[i] );
							output += ",";

							if ( i < notionals.size() )
								output += outStr( notionals[i] );
							output += ",";

							if ( i < fixingDates.size() && d_date != fixingDates[i] )
								output += fixingDates[i].stringWithFormat();
							output += ",";

							if ( i < fixingFlags.size() )
								output += fixingFlags[i];
							output += ",";

							if ( convexityAdjustVols.size() > 0 )
							{
								if ( i < convexityAdjustVols.size() )
									output += outStr( convexityAdjustVols[i] );
								output += ",";			

								if ( i < convexityAdjusts.size() )
									output += outStr( convexityAdjusts[i] );
								output += ",";								
							}

							if ( gearings.size() > 0 )
							{
								if ( i < gearings.size() )
									output += outStr( gearings[i] );
								output += ",";
							}

							if ( i < forwards.size() )
								output += outStr( forwards[i] );
							output += ",";

							if ( spreads.size() > 0 )
							{
								if ( i < spreads.size() )
									output += outStr( spreads[i] );
								output += ",";	
							}

							if ( i < cfPaymentDates.size() && d_date != cfPaymentDates[i] )
								output += cfPaymentDates[i].stringWithFormat();
							output += ",";

							if ( i < times.size() )
								output += outStr( times[i] );
							output += ",";

							if (i < cfs.size())
							{
								if (settleAdjustRatios.size() > 0 && i < settleAdjustRatios.size())
									output += outStr(cfs[i]/settleAdjustRatios[i]);
								else
									output += outStr(cfs[i]);
							}
							output += ",";

							if (settlefixingDates.size() > 0)
							{
								if (i < settlefixingDates.size())
									output += settlefixingDates[i].stringWithFormat();
								output += ",";
							}

							if (settleAdjustRatios.size() > 0)
							{
								if (i < settleAdjustRatios.size())
									output += outStr(settleAdjustRatios[i]);
								output += ",";
							}

							if ( i < dfs.size() )
								output += outStr( dfs[i] );
							output += ",";

							if ( fradfs.size() > 0 )
							{
								if ( i < fradfs.size() )
									output += outStr( fradfs[i] );
								output += ",";	
							}

							if ( i < pvs.size() )
								output += outStr( pvs[i] );
							output += ",";											

							if ( fx_settle_nondeliv_vec.size() > 0 )
							{
								if ( i < fx_settle_nondeliv_vec.size() )
									output += outStr( fx_settle_nondeliv_vec[i] );
								output += ",";

								if ((i < fx_settle_nondeliv_vec.size()) && (i < pvs.size()))
									output += outStr( pvs[i] * fx_settle_nondeliv_vec[i] );
								output += ",";
							}

							if ( expiryTerms.size() > 0 )
							{
								if ( i < expiryTerms.size() )
									output += outStr( expiryTerms[i] );
								output += ",";
							}
							if ( volatilities1.size() > 0 )
							{
								if ( i < volatilities1.size() )
									output += outStr( volatilities1[i] );
								output += ",";								
							}
							if ( volatilities2.size() > 0 )
							{
								if ( i < volatilities2.size() )
									output += outStr( volatilities2[i] );
								output += ",";								
							}
							if ( strikes1.size() > 0 )
							{
								if ( i < strikes1.size() )
									output += outStr( strikes1[i] );
								output += ",";
							}
							if ( strikes2.size() > 0 )
							{
								if ( i < strikes2.size() )
									output += outStr( strikes2[i] );
								output += ",";
							}

							if ( extracfTimes.size() > 0 )
							{
								if ( i < extracfTimes.size() )
									output += outStr( extracfTimes[i] );
								output += ",";

								if ( i < extracfs.size() )
									output += outStr( extracfs[i] );
								output += ",";

								if ( i < extrapvs.size() )
									output += outStr( extrapvs[i] );
								output += ",";
							}

							if (renotionalfixingDates.size() > 0)
							{
								if ( i < renotionalfixingDates.size() && d_date != renotionalfixingDates[i] )
									output += renotionalfixingDates[i].stringWithFormat();
								output += ",";
							}
							if (notionalcfs.size() > 0)
							{
								if ( i < notionalcfs.size() )
									output += outStr( notionalcfs[i] );
								output += ",";
							}

							// ! output line
							outFile << output.getCString() << endl;

						}
						outFile << endl;
					}
				}
				else // isvanilla = false
				{
					// ! Call Probability
					ah_c = &( unders.get(j).getData( PRICING_DATA_CALLINFO, NOCHECK ) );

					DateVector callDates;
					DoubleVector callProbs;
					//double aveLife;
					if ( ah_c->isDefined() && !ah_c->isNull() )
					{
						const LADataReference& attr = dynamic_cast< const LADataReference& >( ah_c->get() );		
						const LAObject* callInfo = &attr.get().get();
						const LADataHolder* dh;

						dh = &( callInfo->getData( PRICING_DATA_ACTIONDATES, NOCHECK ) );
						if ( dh->isDefined() && !dh->isNull() )
							callDates = dynamic_cast< const LADataDates& >( dh->get() ).get();

						dh = &( callInfo->getData( PRICING_DATA_ACTIONPROBABILITIES, NOCHECK ) );
						if ( dh->isDefined() && !dh->isNull() )
							callProbs = dynamic_cast< const LADataDoubles& >( dh->get() ).get();

						/*dh = &( callInfo->getData( PRICING_DATA_AVERAGELIFE, NOCHECK ) );
						if ( dh->isDefined() && !dh->isNull() )
							aveLife = dynamic_cast< const LADataDouble& >( dh->get() ).get();*/
					}

					LAString output_op = "";
					for ( size_t l = 0; l < callDates.size(); l++ )
					{
						if ( l == 0 )
						{
							outFile << endl;
							LAString out_callName = name + "_CallInfo";
							outFile << out_callName << endl;
							//outFile << "ActionDate,Probability,AverageLife" << endl;
							outFile << "ActionDate,Probability" << endl;
						}
						LAString out_call  = "";
						out_call += callDates[l].stringWithFormat();
						out_call += ",";
						if ( l < callProbs.size() )
							out_call += outStr( callProbs[l] );
						
						/*if ( l == 0 )
						{
							out_call += ",";
							out_call += outStr( aveLife );
						}*/

						outFile << out_call << endl;
					}

					// ! Trigger Probability
					ah_c = &( unders.get(j).getData( PRICING_DATA_TRIGGERINFOS, NOCHECK ) );
					vector< LAObject* > triggerInfos;
					if ( ah_c->isDefined() && !ah_c->isNull() )
					{
						const LADataMultiReference& mr = dynamic_cast< const LADataMultiReference& >( ah_c->get() );
						triggerInfos.resize( mr.getSize() );
						for ( size_t l = 0; l < mr.getSize(); l++ )
							triggerInfos[l] = &( mr.get(l).get() );
					}

					for ( size_t l = 0; l < triggerInfos.size(); l++ )
					{
						DateVector triggerDates;
						DoubleVector triggerProbs;

						ah_c = &( triggerInfos[l]->getData( PRICING_DATA_ACTIONDATES, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
							triggerDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();

						ah_c = &( triggerInfos[l]->getData( PRICING_DATA_ACTIONPROBABILITIES, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
							triggerProbs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();

						for ( size_t m = 0; m < triggerDates.size(); m++ )
						{
							if ( m == 0 )
							{
								const int num_tri = l + 1;
								outFile << endl;
								LAString out_triName = name + "_TriggerInfo" + LAString( num_tri );
								outFile << out_triName << endl;
								outFile << "ActionDate,Probability" << endl;
							}
							LAString out_tri = "";
							out_tri += triggerDates[m].stringWithFormat();
							out_tri += ",";
							if ( m < triggerProbs.size() )
								out_tri += outStr( triggerProbs[m] );
							outFile << out_tri << endl;
						}
					}

					ah_c = &( unders.get(j).getData( CALIBRATION_DATA_UNDERLYINGS, NOCHECK ) );
					if ( !ah_c->isDefined() || ah_c->isNull() )
					{
						// ! if leg does not exist, continue
						continue;
					}
					const size_t legNum = dynamic_cast< const LADataMultiReference& >( ah_c->get() ).getSize();

					for ( size_t k = 1; k < legNum + 1; k++ )
					{
						const LAObject* e;
						const int num = k;

						e = &( unders.get(j).get() );

						LAString dataName;
						// ! CFCalcStartDates
						DateVector cfStartDates;
						dataName = PRICING_DATA_CFCALCSTARTDATE_LEG + LAString( num ) ;

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfStartDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! CFCalcEndDates
						DateVector cfEndDates;
						dataName = PRICING_DATA_CFCALCENDDATE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfEndDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! CFPaymentDates
						DateVector cfPaymentDates;
						dataName = PRICING_DATA_PAYMENTDATE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfPaymentDates = dynamic_cast< const LADataDates& >( ah_c->get() ).get();
						}

						// ! PaymentTimings
						DoubleVector times;
						dataName = PRICING_DATA_CASHLETVALUETIME_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							times = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! CFs
						DoubleVector cfs;
						dataName = PRICING_DATA_CASHLETVALUE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							cfs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! PVs
						DoubleVector pvs;
						dataName = PRICING_DATA_PVVALUE_LEG + LAString( num );

						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							pvs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						//// ! TriggerTime
						//DoubleVector triggerTimes;
						//dataName = PRICING_DATA_TRIGGERVALUETIME_LEG + LAString( num );
						//ah_c = &( e->getData( dataName, NOCHECK ) );
						//if ( ah_c->isDefined() && !ah_c->isNull() )
						//{
						//	triggerTimes = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						//}

						//// ! TriggerValue
						//DoubleVector triggers;
						//dataName = PRICING_DATA_TRIGGERVALUE_LEG + LAString( num );
						//ah_c = &( e->getData( dataName, NOCHECK ) );
						//if ( ah_c->isDefined() && !ah_c->isNull() )
						//{
						//	triggers = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						//}

						//// ! TriggerPVValue
						//DoubleVector triggerpvs;
						//dataName = PRICING_DATA_TRIGGERPVVALUE_LEG + LAString( num );
						//ah_c = &( e->getData( dataName, NOCHECK ) );
						//if ( ah_c->isDefined() && !ah_c->isNull() )
						//{
						//	triggerpvs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						//}

						// ! ExtraCFTime
						DoubleVector extracfTimes;
						dataName = PRICING_DATA_EXTRACFVALUETIME_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							extracfTimes = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! ExtraCF
						DoubleVector extracfs;
						dataName = PRICING_DATA_EXTRACFVALUE_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							extracfs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}

						// ! ExtraPV
						DoubleVector extrapvs;
						dataName = PRICING_DATA_EXTRACFPVVALUE_LEG + LAString( num );
						ah_c = &( e->getData( dataName, NOCHECK ) );
						if ( ah_c->isDefined() && !ah_c->isNull() )
						{
							extrapvs = dynamic_cast< const LADataDoubles& >( ah_c->get() ).get();
						}


						// ! Output
						LAString output = "";
						if ( times.size() > 0 ) 
						{
							outFile << endl;
							// ! output tag
							output = name + "_Cashflow_Leg" + LAString( num );
							//output += k;

							outFile << output.getCString() << endl;

							// ! output indices
							output = "";

							output += "CFCalcStartDate,";			
							output += "CFCalcEndDate,";					
							output += "PaymentDate,";								
							output += "PaymentTime,";								
							output += "UnderlyingCF,";					
							output += "UnderlyingPV,";					

							//if ( triggerTimes.size() > 0 )
							//{
							//	output += "TriggerTime,";					
							//	output += "TriggerCF,";					
							//	output += "TriggerPV,";					
							//}

							if ( extracfTimes.size() > 0 )
							{
								output += "ExtraCFTime,";					
								output += "ExtraCF,";					
								output += "ExtraCFPV,";
							}

							outFile << output.getCString() << endl;
						}

						LADate d_date;
						// ! output contents	
						for ( size_t i = 0; i < times.size(); i++ )
						{
							output = "";
							if ( i < cfStartDates.size() && d_date != cfStartDates[i] )
								output += cfStartDates[i].stringWithFormat();
							output += ",";

							if ( i < cfEndDates.size() && d_date != cfEndDates[i] )
								output += cfEndDates[i].stringWithFormat();
							output += ",";

							if ( i < cfPaymentDates.size() && d_date != cfPaymentDates[i] )
								output += cfPaymentDates[i].stringWithFormat();
							output += ",";

							if ( i < times.size() )
								output += outStr( times[i] );
							output += ",";

							if ( i < cfs.size() )
								output += outStr( cfs[i] );
							output += ",";

							if ( i < pvs.size() )
								output += outStr( pvs[i] );
							output += ",";

							if ( extracfTimes.size() > 0 )
							{
								if ( i < extracfTimes.size() )
									output += outStr( extracfTimes[i] );
								output += ",";

								if ( i < extracfs.size() )
									output += outStr( extracfs[i] );
								output += ",";

								if ( i < extrapvs.size() )
									output += outStr( extrapvs[i] );
								output += ",";
							}

							// ! output line
							outFile << output.getCString() << endl;

						}
						outFile << endl;
					}

				}

				if(flag_print_compounded_rate) printCompoundedRates(unders.get(j).get(), fileNumVec[i], rootVec[i], ccyVec[i], outFile, calc);
				
			}
		}
		outFile.close();
	}
	return SUCCESS_CODE;
}

// 
/*!
@brief print fee cash flow

@param[in] trade object
@param[in] file stream
@side-effect export fee cash flow to outFile
*/
void
MAValuePrinter::printFeeCF(LAObjectHolder& tradeEntity, ofstream& outFile)
{
	// ! print fee value
	LADataHolder* dh = &(tradeEntity.getData(PRICING_DATA_PVVALUE_FEE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const DoubleArray feeValues = dynamic_cast< const LADataDoubles& >(dh->get()).get();

		dh = &tradeEntity.getData(CALIBRATION_DATA_FEEPAYMENTDATES, ISNOTNULL);
		const DateVector& feePaymentDates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &tradeEntity.getData(CALIBRATION_DATA_FEEAMOUNTS, ISNOTNULL);
		const DoubleArray& feeAmounts = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		dh = &tradeEntity.getData(CALIBRATION_DATA_FEECURRENCIES, ISNOTNULL);
		const LAStringVector& feeCurrencies = dynamic_cast<const LADataStrings&>(dh->get()).get();
		dh = &tradeEntity.getData(PRICING_DATA_DF_FEE, ISNOTNULL);
		const DoubleArray& feeDiscountFactors = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		dh = &tradeEntity.getData(PRICING_DATA_TODAYFX_FEE, ISNOTNULL);
		const DoubleArray& feeTodayFXRates = dynamic_cast<const LADataDoubles&>(dh->get()).get();

		const LAString &name = dynamic_cast<const LADataString &>(tradeEntity.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		LAString outputTag = name + "_Cashflow_FeeValue";
		outFile << outputTag.getCString() << endl;

		LAString outputIndices = "";
		outputIndices += "FeePaymentDate,";
		outputIndices += "FeeAmount,";
		outputIndices += "FeeCurrency,";
		outputIndices += "DF,";
		outputIndices += "PV";
		outFile << outputIndices.getCString() << endl;

		for (size_t k = 0; k < feeValues.size(); ++k)
		{
			LAString outputValues = "";
			outputValues += feePaymentDates[k].stringWithFormat();
			outputValues += ",";
			outputValues += outStr(feeAmounts[k]);
			outputValues += ",";
			outputValues += feeCurrencies[k];
			outputValues += ",";
			outputValues += outStr(feeDiscountFactors[k]);
			outputValues += ",";
			outputValues += outStr(feeValues[k] != DBL_MAX ? feeValues[k] * feeTodayFXRates[k] : DBL_MAX);
			outFile << outputValues.getCString() << endl;
		}
		outFile << endl;
	}
}


// 
/*!
    @brief print value

	if all success return 0 
	else if all false return 1 
	else return 2
	
	@param[in] mainTradeNameVec
	@param[in] vector of fileNum
	@param[in] vector of dataInstance pointer
	@param[in] vector of ccy
	@return int
*/
int 
MAValuePrinter::printValue(const vector<LAString> &mainTradeVec, const vector<LAString> &fileNumVec,
						   const vector<LADataInstance *> &rootVec, const vector<LAString> &ccyVec, LAString calc)
{
	vector<bool> sFlags;
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString outFileName = staticData.getStaticData(KEY_DEAL_PV_FILE);
	LADataBool isDetail;

	bool isvanilla = false;
	if (calc.size() != 0 && calc.toUpper() == "VANILLA")
	{
		isvanilla = true;
		isDetail.set(false);
	}
	else
	{
		LAString tmpstr = staticData.getStaticData(KEY_SIMULATION_ISDETAILOUTPUT);
		if (tmpstr == MLIB_NO_DATA)
			isDetail.convertFromString("FALSE");
		else
			isDetail.convertFromString(staticData.getStaticData(KEY_SIMULATION_ISDETAILOUTPUT));
	}
	//isDetail.convertFromString(staticData.getStaticData(KEY_SIMULATION_ISDETAILOUTPUT));

	int e_spos = outFileName.findString('.');
	LAString extension = outFileName.subString(e_spos , outFileName.size() - 1);
	LAString outFileName_no_ex = outFileName.subString(0, e_spos - 1);

	const int portNum = mainTradeVec.size();
	for (unsigned int i = 0; i < portNum; ++i)
	{
		LACoreDataService::setContext(ARG_KEY_MAINTRADE, mainTradeVec[i]);
		LACoreDataService::setContext(ARG_KEY_CURRENCY, ccyVec[i]);
		LAString outFileName_ = outFileName_no_ex + fileNumVec[i] + extension;
		ofstream outFile(outFileName_.getCString());

		LAObjectHolder objHolder = rootVec[i]->getObjectPool().getObject(mainTradeVec[i], ENCHKTYPE_ISDEFINED);
		// flags to determine whether to print or not
		bool isPVPrint = false;
		bool isRiskPrint = false;
		// names and entities of deals which are printed actually
		LAStringVector tradeNames;
		vector<vector<const LAObject*> > tradeEntities;
		// flag to determine whether to sum results or not
		bool isSummary = false; 
		// risk information
		const LADataHolder* attrRisk;
		if (dynamic_cast<const LADataValuation &>
			(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
		{
			// if portfolio value output each deal
			const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
												(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
			LAString portName = mainTradeVec[i];
			LAStaticData &irProp = LACoreDataService::getStaticDataManager().getStaticData();
			// to sum results
			if (irProp.getStaticData(KEY_DEAL_OUTPUT_NAME + LAString(".") + portName.toLower()) != MLIB_NO_DATA)
			{				
				isSummary = true;
				tradeNames = irProp.getStaticData(KEY_DEAL_OUTPUT_NAME + LAString(".")  + portName.toLower()).toToken(';');
				//get summary information
				if (staticData.getStaticData(KEY_DEAL_SUMMARY_INFO + LAString(".")  + portName.toLower()) == MLIB_NO_DATA)
					throw LACoreInvalidData("MAValuePrinter::printValue failed! No summary information!",__FILE__,__LINE__);
				LAStringVector summaryInfo = staticData.getStaticData(KEY_DEAL_SUMMARY_INFO + LAString(".")  + portName.toLower()).toToken(';');
				if (summaryInfo.size() != tradeNames.size())
					throw LACoreInvalidData("MAValuePrinter::printValue failed! Sizes of summary information are inconsistent!",__FILE__,__LINE__);
				for (unsigned int j = 0; j < summaryInfo.size(); ++j)
				{
					LAStringVector summaryTrades = summaryInfo[j].toToken(':');
					// set trade entities
					vector<const LAObject*> summaryTradeEntities;
					for (unsigned int k = 0; k < summaryTrades.size(); ++k)
					{
						const LAObject* e = &rootVec[i]->getObjectPool().getObject(summaryTrades[k], ENCHKTYPE_ISDEFINED).get();
						summaryTradeEntities.push_back(e);
					}
					tradeEntities.push_back(summaryTradeEntities);
				}
			}
			// not to sum results
			else
			{	
				for (unsigned int j = 0; j < unders.getSize(); ++j)
				{
					// set trade names
					const LAString &name = dynamic_cast<const LADataString &>(unders.get(j).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
					tradeNames.push_back(name);
					// set trade object
					const LAObject* e = &rootVec[i]->getObjectPool().getObject(name, ENCHKTYPE_ISDEFINED).get();
					vector<const LAObject*> tradeEntitiy(1, e);
					tradeEntities.push_back(tradeEntitiy);
				}
			}

			// get risk info
			attrRisk = &objHolder.getData(PRICING_DATA_RISKCALCINFOS, NOCHECK);
			if (!attrRisk->isDefined() || attrRisk->isNull())
			{
				isPVPrint = true;
				isRiskPrint = false;
			}
			else
			{
				// get value type
				const LADataHolder &attrValueType = objHolder.getData(PRICING_DATA_VALUETYPE, NOCHECK);

				if (!attrValueType.isDefined() || attrValueType.isNull())
				{
					isPVPrint = true;
					isRiskPrint = true; 
				}
				else
				{
					LAString valueType = dynamic_cast<const LADataString&>(attrValueType.get()).get();
					valueType.toUpper();
					if (valueType == "PV")
					{
						isPVPrint = true;
						isRiskPrint = false; 
					}
					else if (valueType == "PVANDRISK")
					{
						isPVPrint = true;
						isRiskPrint = true; 
					}
					else
					{
						isPVPrint = false;
						isRiskPrint = true; 
					}
				}
			}
		}
		// not portfolio value
		else
		{
			// set trade name
			const LAString &name = dynamic_cast<const LADataString &>(objHolder.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			tradeNames.push_back(name);
			// set trade object
			const LAObject* e = &rootVec[i]->getObjectPool().getObject(name, ENCHKTYPE_ISDEFINED).get();
			vector<const LAObject*> tradeEntitiy(1, e);
			tradeEntities.push_back(tradeEntitiy);

			isPVPrint = true;
			isRiskPrint = false;
		}

		bool switchesToIMM = false;
		if (isRiskPrint)
		{
			for (unsigned int j = 0; j < tradeEntities.size(); ++j)
			{
				const LADataMultiReference &riskRef = dynamic_cast<const LADataMultiReference &>(attrRisk->get());
				const int riskNum = riskRef.getSize();
				for (unsigned int k = 0; k < riskNum; ++k)
				{
					LADataHolder &attrImmFwdRateTerm = riskRef.get(k).get().getData(PRICING_DATA_IMMFWDRATETERM, NOCHECK);
					if (attrImmFwdRateTerm.isDefined())
					{
						switchesToIMM = true;
						break;
					}
				}
				if (switchesToIMM)
				{
					isPVPrint = false;
					break;
				}
			}
		}

		for (unsigned int j = 0; j < tradeEntities.size(); ++j)
		{
			if (isPVPrint)
			{
				if (j == 0)
				{
					// set data name
					sFlags.push_back(printAttr(rootVec[i], outFile, ccyVec[i], isDetail.get(), isvanilla, isSummary));
				}

				sFlags.push_back(printPV(tradeNames[j], tradeEntities[j], outFile, ccyVec[i], isDetail.get(), isvanilla));
			}

			if (isRiskPrint)
			{
				// check 
				const LADataMultiReference &riskRef = dynamic_cast<const LADataMultiReference &>(attrRisk->get());
				const int riskNum = riskRef.getSize();
				for (unsigned int k = 0; k < riskNum; ++k)
				{
					LADataHolder &attrBOutputname = riskRef.get(k).getData(PRICING_DATA_BASEOUTPUTNAME, NOCHECK);
					if (attrBOutputname.isDefined() && !attrBOutputname.isNull())
					{
						const LAString &b_outputname = dynamic_cast<const LADataString &>(attrBOutputname.get()).get();
						if (!switchesToIMM)
						{
							sFlags.push_back(printRisk(tradeNames[j], b_outputname, tradeEntities[j], riskRef.get(k).get(),outFile, true));
						}
					}

					const LAString &outputname = dynamic_cast<const LADataString &>
						(riskRef.get(k).getData(PRICING_DATA_OUTPUTNAME, ISNOTNULL).get()).get();
					if (!switchesToIMM)
					{
						sFlags.push_back(printRisk(tradeNames[j], outputname, tradeEntities[j], riskRef.get(k).get(), outFile));
					}
					else
					{
						sFlags.push_back(printIMMFwdRisk(tradeNames[j], outputname, tradeEntities[j], riskRef.get(k).get(), outFile));
					}
					
					LADataHolder &attrOutputname2 = riskRef.get(k).getData(PRICING_DATA_OUTPUTNAME2, NOCHECK);
					if (attrOutputname2.isDefined() && !attrOutputname2.isNull())
					{
						const LAString &outputname2 = dynamic_cast<const LADataString &>(attrOutputname2.get()).get();
						if (!switchesToIMM)
						{
							sFlags.push_back(printRisk(tradeNames[j], outputname2, tradeEntities[j], riskRef.get(k).get(), outFile));
						}
						else
						{
							sFlags.push_back(printIMMFwdRisk(tradeNames[j], outputname2, tradeEntities[j], riskRef.get(k).get(), outFile));
						}
				}
			}
		}

		}
		outFile.close();
	}
	//Error log
	for (int i = 0; i < portNum; ++i)
	{
		LAObjectHolder objHolder = rootVec[i]->getObjectPool().getObject(mainTradeVec[i], ENCHKTYPE_ISDEFINED);
		//Error log for portfolio
		if (dynamic_cast<const LADataValuation &>
			(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)
		{
			const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
												(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
			const int dealSize = unders.getSize();
			for (int j = 0; j < dealSize; ++j)
			{
				// if portfolio value output each deal
				LADataHolder &dh = unders.get(j).getData(CALIBRATION_DATA_ERRORMESSAGES, NOCHECK);
				if (dh.isDefined() && !dh.isNull())
				{
					MALogger &logger = LACoreDataService::getLogManager().getLogger();
					const LAString &name = dynamic_cast<const LADataString &>(unders.get(j).getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
					LAString msg = " Trade Name = " + name + ", File Num = " + fileNumVec[i] + ", Messages are below.";
					logger.error(msg, __FILE__, __LINE__);
					const LADataStrings &errMsgs = dynamic_cast<const LADataStrings &>(dh.get());
					const unsigned int msgSize = errMsgs.getSize();
					for (unsigned int k = 0; k < msgSize; ++k)
					{
						logger.error(errMsgs[k], __FILE__, __LINE__);
					}
				}
			}
		}
		else
		{
			// if portfolio value output each deal
			const LADataHolder &dh = objHolder.getData(CALIBRATION_DATA_ERRORMESSAGES, NOCHECK);
			if (dh.isDefined() && !dh.isNull())
			{
				MALogger &logger =  LACoreDataService::getLogManager().getLogger();
				const LAString &name = dynamic_cast<const LADataString &>(objHolder.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
				LAString msg = " Trade Name = " + name + ", File Num = " + fileNumVec[i] + ", Messages are below";
				logger.error(msg, __FILE__, __LINE__);
				const LADataStrings &errMsgs = dynamic_cast<const LADataStrings &>(dh.get());
				const unsigned int msgSize = errMsgs.getSize();
				for (unsigned int j = 0; j < msgSize; ++j)
				{
					logger.error(errMsgs[j], __FILE__, __LINE__);
				}
			}
		}
	}
	const int sSize = sFlags.size();
	const int trueSize =  static_cast<int>(count(sFlags.begin(), sFlags.end(), true));

	if (sSize == trueSize)
	{
		// all success
		return SUCCESS_CODE;
	}
	else if (trueSize == 0)
	{
		// all false
		return ALL_ERROR_CODE;
	}
	else
	{
		return SOME_ERROR_CODE;
	}
}


// 
/*!
@brief print value

if all success return 0
else if all false return 1
else return 2

@param[in] mainTradeNameVec
@param[in] vector of fileNum
@param[in] vector of dataInstance pointer
@return int
*/
int
MAValuePrinter::printFeeValue(const vector<LAString> &mainTradeVec, const vector<LAString> &fileNumVec,
	const vector<LADataInstance *> &rootVec)
{
	vector<bool> sFlags;

	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString outFileName = staticData.getStaticData(KEY_DEAL_FEEPV_FILE);
	int e_spos = outFileName.findString('.');
	LAString extension = outFileName.subString(e_spos, outFileName.size() - 1);
	LAString outFileName_no_ex = outFileName.subString(0, e_spos - 1);

	const int portNum = mainTradeVec.size();
	for (size_t i = 0; i < portNum; ++i)
	{
		// Use void constuctor here not to create file when fee value is not calculated. 
		ofstream outFile;
		LAString outFileFullName = outFileName_no_ex + fileNumVec[i] + extension;

		bool isSuccess = true;

		LAObjectHolder objHolder = rootVec[i]->getObjectPool().getObject(mainTradeVec[i], ENCHKTYPE_ISDEFINED);
		const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
			(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
		bool isTagPrint = true;
		for (size_t j = 0; j < unders.getSize(); ++j)
		{
			LAObjectHolder& tradeEntity = unders.get(j);

			LADataHolder* dh = &(tradeEntity.getData(PRICING_DATA_PVVALUE_FEE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				if (!outFile.is_open())
				{
					outFile.open(outFileFullName.getCString());
				}

				const DoubleArray feeValues = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				dh = &tradeEntity.getData(CALIBRATION_DATA_FEECURRENCIES, ISNOTNULL);
				const LAStringVector& feeCurrencies = dynamic_cast<const LADataStrings&>(dh->get()).get();
				dh = &tradeEntity.getData(PRICING_DATA_DF_FEE, ISNOTNULL);
				const DoubleArray& feeDiscountFactors = dynamic_cast<const LADataDoubles&>(dh->get()).get();


				LAStringVector uniqueFeeCurrencies(feeCurrencies);
				sort(uniqueFeeCurrencies.begin(), uniqueFeeCurrencies.end());
				uniqueFeeCurrencies.erase(unique(uniqueFeeCurrencies.begin(), uniqueFeeCurrencies.end()), uniqueFeeCurrencies.end());

				map<LAString, double>  feePVTable;
				for (size_t k = 0; k < uniqueFeeCurrencies.size(); ++k)
				{
					feePVTable.insert({ uniqueFeeCurrencies[k] , 0. });
				}

				for (size_t k = 0; k < min(feeValues.size(), feeCurrencies.size()); ++k)
				{
					if (feePVTable[feeCurrencies[k]] == DBL_MAX || feeValues[k] == DBL_MAX)
					{
						feePVTable[feeCurrencies[k]] = DBL_MAX;
						isSuccess = false;
						continue;
					}

					feePVTable[feeCurrencies[k]] += feeValues[k];
				}

				if (isTagPrint)
				{
					// print tag name
					LAString outputValues = "";
					outputValues += "TransID,";
					outputValues += "PV,";
					outputValues += "PVCurrency";
					outFile << outputValues << endl;
					isTagPrint = false;
				}

				//print fee pv grouped by currency
				for (const pair<LAString, double> it : feePVTable)
				{
					LAString outputValues = "";
					outputValues += tradeEntity.getName();
					outputValues += ",";
					outputValues += outStr(it.second);
					outputValues += ",";
					outputValues += it.first;
					outFile << outputValues.getCString() << endl;
				}
			}
		}
		sFlags.push_back(isSuccess);
	}

	const int sSize = sFlags.size();
	const int trueSize = static_cast<int>(count(sFlags.begin(), sFlags.end(), true));

	if (sSize == trueSize)
	{
		// all success
		return SUCCESS_CODE;
	}
	else if (trueSize == 0)
	{
		// all false
		return ALL_ERROR_CODE;
	}
	else
	{
		return SOME_ERROR_CODE;
	}
}


// 
/*!
    @brief print pv value
	
	@param[in] name
	@param[in] target trade entitys
	@param[in] file
	@param[in] ccys
	@param[in] isDetail
	@return bool
*/
bool 
MAValuePrinter::printPV(const LAString &name, const vector<const LAObject*> &tradeEntities, ofstream &file, const LAString &ccys, bool isDetail, bool isvanilla)
{
	const LADataHolder* ah_c;	
	// size check
	if (tradeEntities.size() == 0)
	{
		throw LACoreInvalidData("MAValuePrinter::printPV failed! The number of trade is zero!",__FILE__,__LINE__);
	}
	else if (tradeEntities.size() > 1)
	{
		LAString output = "";
		if (isvanilla)
		{
			double value = 0.;
			std::vector<double > legValue(2, 0.);
			LAString ccy;
			std::vector<LAString > legCcy(2, "");
			std::vector<double > legTodayFx(2, 1.);
			
			bool IsOutputByLeg(true);
			std::vector<LAString> attrNameLegPV(2);
			attrNameLegPV[0] = PRICING_DATA_PV_LEG1; attrNameLegPV[1] = PRICING_DATA_PV_LEG2; 
			std::vector<LAString> attrNameLegCcy(2);
			attrNameLegCcy[0] = PRICING_DATA_CURRENCY_LEG1; attrNameLegCcy[1] = PRICING_DATA_CURRENCY_LEG2; 
			std::vector<LAString> attrNameLegTodayFX(2);
			attrNameLegTodayFX[0] = PRICING_DATA_TODAYFX_LEG1CCY; attrNameLegTodayFX[1] = PRICING_DATA_TODAYFX_LEG2CCY; 
			LAStringVector attrNameLegPVPrefixingAdjustment = { PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT, PRICING_DATA_PV_LEG2_PREFIXING_ADJUSTMENT };

			for (unsigned int i = 0; i < tradeEntities.size(); ++i)
			{
				value += dynamic_cast<const LADataDouble &>(tradeEntities[i]->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL).get()).get();
				
				LAString ccy_last = ccy;
				ah_c = &tradeEntities[i]->getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK);
				if (ah_c->isDefined() && !ah_c->isNull())
				{
					ccy = dynamic_cast<const LADataString &>(ah_c->get()).get();
					
				}
				else
				{
					ccy = dynamic_cast<const LADataString &>(tradeEntities[i]->getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
				}
				if (i != 0 && ccy_last != ccy)
					throw LACoreInvalidData("Currencies are not same in risk summary!",__FILE__,__LINE__);

				// summary by leg when all of trades contain leg dataValues
				bool IsContainLegAttribute(false);
				for (unsigned int j = 0; j < 2; ++j)
				{
					ah_c = &tradeEntities[i]->getData(attrNameLegPV[j], NOCHECK);
					if (ah_c->isDefined() && !ah_c->isNull())
					{
						IsContainLegAttribute = true;
						//leg pv
						legValue[j] += dynamic_cast<const LADataDouble& >(ah_c->get()).get();
						// add prefixing cf for MTMCS
						ah_c = &tradeEntities[i]->getData(PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT, NOCHECK);
						LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
						const bool isMovePrefixingCF = staticData.getStaticData(KEY_DEAL_MTMCS_IS_MOVE_PREFIXING_PV).toLower() == "true";
						if (ah_c->isDefined() && !ah_c->isNull() && isMovePrefixingCF)
						{
							double pvLeg1PrefixingAdjustment = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
							legValue[j] += pvLeg1PrefixingAdjustment;
						}
						//leg currency
						LAString legCcy_last = legCcy[j];
						legCcy[j] = dynamic_cast<const LADataString& >(tradeEntities[i]->getData(attrNameLegCcy[j], ISNOTNULL).get()).get();
						if (legCcy_last != "" && legCcy_last != legCcy[j])
							throw LACoreInvalidData("Currencies are not same in risk summary!",__FILE__,__LINE__);
						//leg today fx rate
						double legTodayFx_last = legTodayFx[j];
						legTodayFx[j] = dynamic_cast<const LADataDouble& >(tradeEntities[i]->getData(attrNameLegTodayFX[j], ISNOTNULL).get()).get();
						if (legTodayFx_last != 1. && legTodayFx_last != legTodayFx[j])
							throw LACoreInvalidData("TodayFXs are not same in risk summary!",__FILE__,__LINE__);
					}
				}
				IsOutputByLeg = IsOutputByLeg & IsContainLegAttribute;

			}
			output += name;
			output += ",";
			output += "PV,";
			output += outStr(value);
			output += ",";
			output += ccy;
			if(IsOutputByLeg)
			{
				//leg pv, currency, today fx
				for (unsigned int j = 0; j < 2; ++j)
				{
					output += "," + outStr(legValue[j]);
					output += "," + legCcy[j];
					output += "," + outStr(legTodayFx[j]);
				}
				//dirtyprice in leg1 currecy, dirtyprice in leg2 currency
				for (unsigned int j = 0; j < 2; ++j)
				{
					output += "," + outStr(value / legTodayFx[j]);
				}
			}
		}			
		else
		{
			double value = 0.;
			LAString ccy;
			for (unsigned int i = 0; i < tradeEntities.size(); ++i)
			{
				value += dynamic_cast<const LADataDouble &>(tradeEntities[i]->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL).get()).get();
				
				LAString ccy_last = ccy;
				ccy = dynamic_cast<const LADataString &>(tradeEntities[i]->getData(PRICING_DATA_PVCURRENCY, ISNOTNULL).get()).get();
				if (i != 0 && ccy_last != ccy)
					throw LACoreInvalidData("Currencies are not same in risk summary!",__FILE__,__LINE__);
			}
			output += name;
			output += ",";
			output += outStr(value);
			output += ",";
			output += ccy;
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_FEE_EXCLUDED_PV, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += ",";
			output += "FeeExcludedPV,";
			output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
		}

		file << output.getCString() << endl;
		return true;
	}

	if (isvanilla)
	{
		//phase6.0
		const LADataHolder* ah_c;	
		LAString output = name;
		output += ",";
		output += "PV,";
		output += outStr(dynamic_cast<const LADataDouble &>(tradeEntities[0]->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL).get()).get());
		output += ",";
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_VOLATILITYRESULTOFPREMIUM, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += "Volatility,";
			output += outStr(100.0 * dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_PREMIUMCURRENCY, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += dynamic_cast<const LADataString &>(ah_c->get()).get();
			output += ",";
		}
		else
		{
			output += dynamic_cast<const LADataString &>(tradeEntities[0]->getData(PRICING_DATA_CURRENCY, ISNOTNULL).get()).get();
			output += ",";
		}

		ah_c = &tradeEntities[0]->getData(PRICING_DATA_PV_LEG1, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			double pvLeg1 = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
			ah_c = &tradeEntities[0]->getData(PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT, NOCHECK);
			LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
			LAString isMovePrefixingCF_str = staticData.getStaticData(KEY_DEAL_MTMCS_IS_MOVE_PREFIXING_PV).toLower();
			const bool isMovePrefixingCF = isMovePrefixingCF_str == "true";
			if (ah_c->isDefined() && !ah_c->isNull() && isMovePrefixingCF)
			{
				double pvLeg1PrefixingAdjustment = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
				pvLeg1 += pvLeg1PrefixingAdjustment;
			}
			output += outStr(pvLeg1);
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_CURRENCY_LEG1, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += dynamic_cast<const LADataString &>(ah_c->get()).get();
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_TODAYFX_LEG1CCY, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_PV_LEG2, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			double pvLeg2 = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
			ah_c = &tradeEntities[0]->getData(PRICING_DATA_PV_LEG2_PREFIXING_ADJUSTMENT, NOCHECK);
			LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
			const bool isMovePrefixingCF = staticData.getStaticData(KEY_DEAL_MTMCS_IS_MOVE_PREFIXING_PV).toLower() == "true";
			if (ah_c->isDefined() && !ah_c->isNull() && isMovePrefixingCF)
			{
				double pvLeg2PrefixingAdjustment = dynamic_cast<const LADataDouble &>(ah_c->get()).get();
				pvLeg2 += pvLeg2PrefixingAdjustment;
			}
			output += outStr(pvLeg2);
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_CURRENCY_LEG2, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += dynamic_cast<const LADataString &>(ah_c->get()).get();
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_TODAYFX_LEG2CCY, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_DIRTYPRICE_LEG1CCY, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output +=  outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_DIRTYPRICE_LEG2CCY, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output +=  outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_OPTIONVALUE, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += "OptionPV,";
			output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_PREMIUMVALUE, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += "PremiumPV,";
			output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}
		ah_c = &tradeEntities[0]->getData(PRICING_DATA_FEE_EXCLUDED_PV, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output += "FeeExcludedPV,";
			output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			output += ",";
		}

		file << output.getCString() << endl;

		ah_c = &tradeEntities[0]->getData(PRICING_DATA_CONVERGENCETARGET, NOCHECK);
		if (ah_c->isDefined() && !ah_c->isNull())
		{
			output = name;
			output += ",";
			output += dynamic_cast<const LADataString &>(ah_c->get()).get();
			output += ",";
			ah_c = &tradeEntities[0]->getData(PRICING_DATA_CONVERGENCEVALUE, NOCHECK);
			if (ah_c->isDefined() && !ah_c->isNull())
			{
				output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
			}
			file << output.getCString() << endl;
		}

		return true;
	}
	
	double acc_tmp = 0;
	const LADataHolder* dh;	
	// check accruedinterest reference rate (spot or forward)
	//leg object
	LAObjectPool& objPool = tradeEntities[0]->getDataInstance()->getObjectPool();
	const LAString portName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(portName, ENCHKTYPE_ISDEFINED);

	// for portfolio
	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
										(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	LADate today;
	LADate settledate;
	bool isSpot = false;
	
	bool ret = true;
	LAString output = name;
	output += ",";
	//output += "PV,";
	output += outStr(dynamic_cast<const LADataDouble &>(tradeEntities[0]->getData(PRICING_DATA_CLEANPRICE, ISNOTNULL).get()).get());
	output += ",";
	output += outStr(dynamic_cast<const LADataDouble &>(tradeEntities[0]->getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL).get()).get());
	output += ",";
	if (isSpot)
	{
		// accrued interest with spot rate
		output += outStr(acc_tmp);
	}
	else
	{
		output += outStr(dynamic_cast<const LADataDouble &>(tradeEntities[0]->getData(PRICING_DATA_ACCRUEDINTEREST, ISNOTNULL).get()).get());
	}	
	
	// ! Call info
	output += ",";
	/*ah_c = &( tradeEntities[0]->getData( PRICING_DATA_CALLINFO, NOCHECK ) );
	if ( ah_c->isDefined() && !ah_c->isNull() )
	{*/
	dh = &( tradeEntities[0]->getData( PRICING_DATA_AVERAGELIFE, NOCHECK ) );
	if ( dh->isDefined() && !dh->isNull() )
	{
		double aveLife = dynamic_cast< const LADataDouble& >( dh->get() ).get();
		output += outStr(aveLife);
	}
	//}

	/*if (e.getData(PRICING_DATA_CLEANPRICESQUARE).isDefined())
	{
		output += ",";
		output += outStr(dynamic_cast<const LADataDouble &>(e.getData(PRICING_DATA_CLEANPRICESQUARE, ISNOTNULL).get()).get());
	}*/
	/*output += ",";
	output += outStr(dynamic_cast<const LADataDouble &>(e.getData(PRICING_DATA_CLEANPRICESQUARE, ISNOTNULL).get()).get());*/
	if (isDetail)
	{
		output += ",";
		const LADataHolder &attrCallTrig = tradeEntities[0]->getData(PRICING_DATA_CALLTRIGGERVALUE, NOCHECK);
		if (attrCallTrig.isDefined() && !attrCallTrig.isNull())
		{
			output += outStr(dynamic_cast<const LADataDouble &>(attrCallTrig.get()).get());
		}

		output += ",";
		const LADataHolder &attrCleanCallTrig = tradeEntities[0]->getData(PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER, NOCHECK);
		if (attrCleanCallTrig.isDefined() && !attrCleanCallTrig.isNull())
		{
			output += outStr(dynamic_cast<const LADataDouble &>(attrCleanCallTrig.get()).get());

		}
	}

	//print base currency
	output += ",";
	output += dynamic_cast<const LADataString &>(tradeEntities[0]->getData(PRICING_DATA_PVCURRENCY, ISNOTNULL).get()).get();

	//file << output.getCString() << endl;

	LAStringVector ccyVec = ccys.toToken(MULTI_STATIC_DATA_DELIMITER);
	const int ccySize = ccyVec.size();
	for (int i = 0; i < ccySize; ++i)
	{
		if (ccyVec[i].findString(FX_DELIMITER) >= 0)
		{
			break;
		}
		// cash print
		//LAString output = name;
		//output += ",";
		LAString cAttrStr = ccyVec[i].toUpper() + "_" + PRICING_DATA_CASH;
		//output += cAttrStr;
		output += ",";
		const LADataHolder &ahC = tradeEntities[0]->getData(cAttrStr);
		if (ahC.isDefined() && !ahC.isNull())
		{
			output += outStr(dynamic_cast<const LADataDouble &>(ahC.get()).get());
		}
		else
		{
			output += outStr(0.0);
		}
	}

	//print funding spread
	output += ",";
	dh = &(tradeEntities[0]->getData(PRICING_DATA_FUNDINGSPREADENTITY, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAObject& e = dynamic_cast<const LADataReference &>(dh->get()).get().get();
		const double spd = dynamic_cast<const LADataDouble &>(e.getData(PRICING_DATA_FUNDINGSPREAD, ISNOTNULL).get()).get();
		output += outStr(spd * 10000);
	}
	else
	{
		output += outStr(0.0);
	}

	// print PV without fee value
	output += ",";
	ah_c = &tradeEntities[0]->getData(PRICING_DATA_FEE_EXCLUDED_PV, NOCHECK);
	if (ah_c->isDefined() && !ah_c->isNull())
	{
		output += outStr(dynamic_cast<const LADataDouble &>(ah_c->get()).get());
	}

	file << output.getCString() << endl;

	if (dynamic_cast<const LADataDouble &>(tradeEntities[0]->getData(PRICING_DATA_CLEANPRICE, ISNOTNULL).get()).get()
		== DBL_MAX)
	{
		ret = false;
	}

	return ret;
}


// 
/*!
    @brief print summary risk value
	
	@param[in] trade name
	@param[in] risk name
	@param[in] target trade entitys
	@param[in] file
	@param[in] isBase
	@return bool
*/
bool 
MAValuePrinter::printRisk(const LAString &name, const LAString &riskName, const vector<const LAObject*> &tradeEntities, const LAObject &riske, ofstream &file, bool isBase)
{
	// size check
	if (tradeEntities.size() == 0)
		throw LACoreInvalidData("MAValuePrinter::printRisk failed! The number of trade is zero!",__FILE__,__LINE__);
	bool ret = true;
	//const LADataHolder &attrRisk = e.getData(riskName, ISNOTNULL);
	//const LADataHolder &attrRisk = e.getData(riskName, NOCHECK);

	// set base currency
	const LADataHolder* dh;
	LAString basecur;	
	for (unsigned int i = 0; i < tradeEntities.size(); ++i)
	{
		LAString basecur_last = basecur;

		dh = &(tradeEntities[i]->getData(PRICING_DATA_CURRENCY));
		if (dh->isDefined() && !dh->isNull())
		{
			basecur = dynamic_cast<const LADataString &>(dh->get()).get();
		}
		else 
		{
			dh = &(tradeEntities[i]->getData(PRICING_DATA_PREMIUMCURRENCY, ISNOTNULL));
			basecur = dynamic_cast<const LADataString &>(dh->get()).get();
		}

		dh = &(riske.getData(PRICING_DATA_RISKOUTPUTCURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			basecur = dynamic_cast<const LADataString &>(dh->get()).get();
		}

		if (i != 0 && basecur != basecur_last)
			throw LACoreInvalidData("Currencies are not same in risk summary!",__FILE__,__LINE__);
	}
	//set curve type
	LAString curvetype = "BaseCurve";
	dh = &(riske.getData(PRICING_DATA_RISKCURVETYPENAME, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAString tmpname = dynamic_cast<const LADataString &>(dh->get()).get();
		if (tmpname != "STD")
			curvetype = tmpname;
	}

	//set base shift curve type
	LAString baseShiftCurveType = "";
	dh = &(riske.getData(PRICING_DATA_RISKBASESHIFTCURVETYPENAME, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAString tmpname = dynamic_cast<const LADataString &>(dh->get()).get();
		if (tmpname == "STD")
		{
			baseShiftCurveType = "BaseCurve";
		}
		else
		{
			baseShiftCurveType = tmpname;
		}
	}

	// set vega type
	LAString vType = "";
	dh = &(riske.getData(AP_CALIBRATION_DATA_VEGATYPE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		vType = dynamic_cast<const LADataString &>(dh->get()).get();
		vType = "," + vType.toLower();
	}

	//phase6.0
	//RISK_KEY_OMIT_DIGITNUMBER
	LAStaticData &rprop = LACoreDataService::getStaticDataManager().getRiskStaticData();
	LAString digit = rprop.getStaticData(RISK_KEY_OMIT_DIGITNUMBER);
	digit.toUpper();
	bool isomit = (digit != MLIB_NO_DATA);
	unsigned int digitnum = 0;
	if (isomit)
		digitnum = static_cast<unsigned int>(digit.getIntValue());
	double omitborder = LAMath::pow(0.1,digitnum);


	if (isBase) //PV&PL
	{
		//double value = dynamic_cast<const LADataDouble &>(attrRisk.get()).get();
		double value = 0.;
		for (unsigned int i = 0; i < tradeEntities.size(); ++i)
		{
			const LADataHolder &attrRisk = tradeEntities[i]->getData(riskName, NOCHECK);
			if (attrRisk.isDefined() && !attrRisk.isNull()) 
			{
				value += dynamic_cast<const LADataDouble &>(attrRisk.get()).get();
			}
			else
			{
				value = DBL_MAX;
				break;
			}
		}
		// parallel shift only
		LAString output = name;
		output += ",";
		output += riskName;
		output += ",";
		
		output += outStr(value);
		//print basecur
		output += ",";
		output += basecur; 
		//print curvetype
		output += ",";
		output += curvetype;
		//print base shift curvetype
		output += ",";
		output += baseShiftCurveType;
		file << output.getCString() << endl;
		
		//calc base shift PV - original PV 
		//double value_PL = dynamic_cast<const LADataDouble &>(e.getData(riskName + "_PL", ISNOTNULL).get()).get();
		double value_PL = 0.; 
		if (value != DBL_MAX)
		{
			for (unsigned int i = 0; i < tradeEntities.size(); ++i)
			{
				value_PL += dynamic_cast<const LADataDouble &>(tradeEntities[i]->getData(riskName + "_PL", ISNOTNULL).get()).get();
			}
		}
		else
		{
			value_PL = DBL_MAX;
		}
		const LAString &outputname = dynamic_cast<const LADataString &> (riske.getData(PRICING_DATA_OUTPUTNAME, ISNOTNULL).get()).get();
		
		output = name;
		output += ",";

		LAString dtype = "";
		dh = &(riske.getData(AP_CALIBRATION_DATA_DELTATYPE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			dtype = "_" + dynamic_cast<const LADataString &>(dh->get()).get();
		}
		output += (outputname + dtype + "_PL");

		output += ",";
		output += outStr(value_PL);
		//print basecur
		output += ",";
		output += basecur;
		//print curvetype
		output += ",";
		output += curvetype;
		//print base shift curvetype
		output += ",";
		output += baseShiftCurveType;
		//print vegatype
		output += vType;
		file << output.getCString() << endl;

		// check DBL_MAX
		if (value == DBL_MAX)
		{
			ret = false;
		}
		return ret;
	}

	bool isRiskAttrExist = false;
	const LADataHolder* attrRisk;
	for (unsigned int i = 0; i < tradeEntities.size(); ++i)
	{
		attrRisk = &tradeEntities[i]->getData(riskName, NOCHECK);
		if (attrRisk->isDefined() && !attrRisk->isNull())
		{
			isRiskAttrExist = true;
			break;
		}
	}
	// if more than one trade object has a risk data
	if (isRiskAttrExist)
	{
		if (attrRisk->getType() == DATA_DOUBLES) //isGridSensitivity=TRUE
		{
			// grid out put
			LAString tmpRiskName = riskName;
			DoubleArray valueArray;
			int size = 0;
			LAStringVector gridArray;
			bool isParallel = false;
			for (unsigned int i = 0; i < tradeEntities.size(); ++i)
			{
				attrRisk = &tradeEntities[i]->getData(riskName, NOCHECK);
				if (!attrRisk->isDefined() || attrRisk->isNull()) continue;
				
				//DoubleArray valueArray_last = valueArray;
				int size_last = size;
				LAStringVector gridArray_last = gridArray;
				bool isParallel_last = isParallel;

				DoubleArray valueArray_oneTrade = dynamic_cast<const LADataDoubles &>(tradeEntities[i]->getData(riskName, ISNOTNULL).get()).get();
				size = valueArray_oneTrade.size();
				dh = &tradeEntities[i]->getData(tmpRiskName.toUpper() + AP_CALIBRATION_DATA_RISK_GRID, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					gridArray = dynamic_cast<const LADataStrings &>(dh->get()).get();
				}
				isParallel = dynamic_cast<const LADataBool &>(tradeEntities[i]->getData(tmpRiskName.toUpper() + AP_CALIBRATION_DATA_ISPARALLELSHIFT, ISNOTNULL).get()).get();
				
				if (size_last == 0)
				{
					valueArray = valueArray_oneTrade;
				}
				else
				{
					if (size_last != size || gridArray_last != gridArray || isParallel_last != isParallel)
						throw LACoreInvalidData("Summary information is inconsistent!",__FILE__,__LINE__);
				
					for (unsigned int j = 0; j < size; ++j)
					{
						if (valueArray[j] != DBL_MAX && valueArray_oneTrade[j] != DBL_MAX) 
						{
							valueArray[j] += valueArray_oneTrade[j];
						}
						else
						{
							valueArray[j] = DBL_MAX;
						}
					}
				}
			}

			unsigned int refAddIndex = 0;
			if (isParallel)
			{
				refAddIndex = 1;
				// parallel shift and grid risk
				// first elemet is parallel shift
				LAString output = name;
				output += ",";
		
				LAString dtype = "";
				dh = &(riske.getData(AP_CALIBRATION_DATA_DELTATYPE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					dtype = "_" + dynamic_cast<const LADataString &>(dh->get()).get();
				}
				output += (riskName + dtype + "_PARALLEL");

				output += ",";
				output += outStr(valueArray[0]);
				//print basecur
				output += ",";
				output += basecur;
				//print curvetype
				output += ",";
				output += curvetype;
				//print base shift curvetype
				output += ",";
				output += baseShiftCurveType;
				//print vegatype
				output += vType;
				file << output.getCString() << endl;
				// check DBL_MAX
				if (valueArray[0] == DBL_MAX)
				{
					ret = false;
				}
			}

			bool isOmitStart = false;
			bool isOmitEnd = false;
			LAString isOmitStart_str = LACoreDataService::getContext(CONTEXT_KEY_RISKGRID_ISOMITSTART).toUpper();
			LAString isOmitEnd_str = LACoreDataService::getContext(CONTEXT_KEY_RISKGRID_ISOMITEND).toUpper();
			if (isOmitStart_str == "TRUE")
			{
				isOmitStart = true;
			}
			if (isOmitEnd_str == "TRUE")
			{
				isOmitEnd = true;
			}
			dh = &riske.getData("Name", ISNOTNULL);
			LAStringVector riskNameVector = dynamic_cast<const LADataString &>(dh->get()).get().toToken('_');
			DoubleArray forwardFXs,fxVolatilities,forwardSwapRates, swaptionValatilities, optionMaturities, settleTerms;
			bool isPrintFXAdditionalInfo = riskNameVector.size() > 0 && riskNameVector[1] == "FXVEGA";
			bool isPrintIRAdditionalInfo = riskNameVector.size() > 0 && riskNameVector[1] == "IRVEGA";
			if (isPrintFXAdditionalInfo)
			{
				dh = &tradeEntities[0]->getData(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDFX_FORPRINT);
				if (!dh->isDefined() || dh->isNull())
				{
					isPrintFXAdditionalInfo = false;
				}
				else
				{
					forwardFXs = dynamic_cast<const LADataDoubles &>(dh->get()).get();
					dh = &tradeEntities[0]->getData(riskNameVector[0] + "_" + CALIBRATION_DATA_FXVOL_FORPRINT);
					fxVolatilities = dynamic_cast<const LADataDoubles &>(dh->get()).get();
				}
			}
			if (isPrintIRAdditionalInfo)
			{
				dh = &tradeEntities[0]->getData(riskNameVector[0] + "_" + CALIBRATION_DATA_FORWARDSWAPRATE_FORPRINT);
				if (!dh->isDefined() || dh->isNull())
				{
					isPrintIRAdditionalInfo = false;
				}
				else
				{
					forwardSwapRates = dynamic_cast<const LADataDoubles &>(dh->get()).get();
					dh = &tradeEntities[0]->getData(riskNameVector[0] + "_" + CALIBRATION_DATA_SWAPTIONVOL_FORPRINT);
					swaptionValatilities = dynamic_cast<const LADataDoubles &>(dh->get()).get();
					dh = &tradeEntities[0]->getData(riskNameVector[0] + "_" + CALIBRATION_DATA_OPTION_MATURITY_FORPRINT);
					optionMaturities = dynamic_cast<const LADataDoubles &>(dh->get()).get();
					dh = &tradeEntities[0]->getData(riskNameVector[0] + "_" + CALIBRATION_DATA_SETTLE_TERM_FORPRINT);
					settleTerms = dynamic_cast<const LADataDoubles &>(dh->get()).get();
				}
			}

			for (unsigned int i = refAddIndex; i < size; ++i)
			{
				size_t i_0 = i - refAddIndex;
				//phase6.0
				if (isomit && LAMath::abs(valueArray[i]) < omitborder || (isOmitStart && i == refAddIndex) || (isOmitEnd && i == size - 1))
					continue;

				LAString output = name;
				output += ",";
				output += (riskName + "_" +  gridArray[i - refAddIndex]);
				output += ",";
				output += outStr(valueArray[i]);
				//print basecur
				output += ",";
				output += basecur;
				//print curvetype
				output += ",";
				output += curvetype;
				//print base shift curvetype
				output += ",";
				output += baseShiftCurveType;
				//print vegatype
				output += vType;
				if (isPrintFXAdditionalInfo)
				{
					output += ",";
					output += outStr(forwardFXs[i_0]);
					output += ",";
					output += outStr(fxVolatilities[i_0]);
				}
				if (isPrintIRAdditionalInfo)
				{
					output += ",";
					output += outStr(forwardSwapRates[i_0]);
					output += ",";
					output += outStr(swaptionValatilities[i_0]);
					output += ",";
					output += outStr(optionMaturities[i_0]);
					output += ",";
					output += outStr(settleTerms[i_0]);
				}
				file << output.getCString() << endl;
				// check DBL_MAX
				if (valueArray[i] == DBL_MAX)
				{
					ret = false;
				}
			}
		}
		else //isGridSensitivity=FALSE
		{
			double value = 0.;
			for (unsigned int i = 0; i < tradeEntities.size(); ++i)
			{
				double value_oneTrade = dynamic_cast<const LADataDouble &>(tradeEntities[i]->getData(riskName, ISNOTNULL).get()).get();
				if (value != DBL_MAX && value_oneTrade != DBL_MAX)
				{
					value += value_oneTrade;
				}
				else
				{
					value = DBL_MAX;
				}
			}
			// parallel shift only
			LAString output = name;
			output += ",";

			bool isanalyticalrisk = false;
			dh = &(tradeEntities[0]->getData(LAString(PRICING_DATA_TERMFORRISK) + "_" + riskName));
			isanalyticalrisk = (dh->isDefined() && !dh->isNull());

			LAString dtype = "";
			dh = &(riske.getData(AP_CALIBRATION_DATA_DELTATYPE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				dtype = "_" + dynamic_cast<const LADataString &>(dh->get()).get();
			}

			if (isanalyticalrisk)
			{
				LAString matterm = dynamic_cast<const LADataString &>(dh->get()).get();
				output += (riskName + dtype + "_" + matterm);
			}
			else
			{
				output += (riskName + dtype + "_PARALLEL");
			}

			output += ",";
			output += outStr(value);
			//print basecur
			output += ",";
			output += basecur;
			//print curvetype
			output += ",";
			output += curvetype;
			//print base shift curvetype
			output += ",";
			output += baseShiftCurveType;
			//print vegatype
			output += vType;
			file << output.getCString() << endl;
			// check DBL_MAX
			if (value ==DBL_MAX)
			{
				ret = false;
			}
		}
	}
	else
	{
		double value = DBL_MAX;
		// parallel shift only
		LAString output = name;
		output += ",";

		bool isanalyticalrisk = false;
		dh = &(tradeEntities[0]->getData(LAString(PRICING_DATA_TERMFORRISK) + "_" + riskName));
		isanalyticalrisk = (dh->isDefined() && !dh->isNull());

		LAString dtype = "";
		dh = &(riske.getData(AP_CALIBRATION_DATA_DELTATYPE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			dtype = "_" + dynamic_cast<const LADataString &>(dh->get()).get();
		}

		if (isanalyticalrisk)
		{
			LAString matterm = dynamic_cast<const LADataString &>(dh->get()).get();
			output += (riskName + dtype + "_" + matterm);
		}
		else
		{
			output += (riskName + dtype + "_PARALLEL");
		}

		output += ",";
		output += outStr(value);
		//print basecur
		output += ",";
		output += basecur;
		//print curvetype
		output += ",";
		output += curvetype;
		//print base shift curvetype
		output += ",";
		output += baseShiftCurveType;
		//print vegatype
		output += vType;
		file << output.getCString() << endl;
		// check DBL_MAX
		if (value == DBL_MAX)
		{
			ret = false;
		}
	}

	return ret;
}

bool
MAValuePrinter::printIMMFwdRisk(const LAString &name, const LAString &riskName, const vector<const LAObject*> &tradeEntities, const LAObject &riske, ofstream &file)
{
	// size check
	if (tradeEntities.size() == 0)
		throw LACoreInvalidData("MAValuePrinter::printIMMFwdRisk failed! The number of trade is zero!",__FILE__,__LINE__);
	bool ret = true;
	const LADataHolder *dh;

	// set base currency
	LAString basecur;
	for (unsigned int i = 0; i < tradeEntities.size(); ++i)
	{
		LAString basecur_last = basecur;

		dh = &(tradeEntities[i]->getData(PRICING_DATA_CURRENCY));
		if (dh->isDefined() && !dh->isNull())
		{
			basecur = dynamic_cast<const LADataString &>(dh->get()).get();
		}
		else
		{
			dh = &(tradeEntities[i]->getData(PRICING_DATA_PREMIUMCURRENCY, ISNOTNULL));
			basecur = dynamic_cast<const LADataString &>(dh->get()).get();
		}

		dh = &(riske.getData(PRICING_DATA_RISKOUTPUTCURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			basecur = dynamic_cast<const LADataString &>(dh->get()).get();
		}

		if (i != 0 && basecur != basecur_last)
		{
			throw LACoreInvalidData("Currencies are not same in risk summary!",__FILE__,__LINE__);
		}
	}

	//set curve type
	LAString curvetype = "BaseCurve";
	dh = &(riske.getData(PRICING_DATA_RISKCURVETYPENAME, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAString tmpname = dynamic_cast<const LADataString &>(dh->get()).get();
		if (tmpname != STD)
			curvetype = tmpname;
	}

	LAString riskNameU(riskName);
	riskNameU.toUpper();

	for (std::vector<const LAObject*>::const_iterator i = tradeEntities.begin(), iend = tradeEntities.end(); i != iend; ++i)
	{
		const LAObject &tradeEntity = **i;
		const LADataHolder *dh = &tradeEntity.getData(riskNameU + "IMMFwd");
		if (dh->isDefined())
		{
			LAString output = name;
			output += ",";
			output += riskName;
			output += ",";
			output += basecur;
			output += ",";
			output += curvetype;
			const DoubleArray &risks = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			for (DoubleArray::const_iterator j = risks.begin(), jend = risks.end(); j != jend; ++j)
			{
				double value = *j;
				output += ",";
				output += outStr(value);
				if (value == DBL_MAX)
				{
					ret = false;
				}
			}
			file << output.getCString() << endl;
		}
	}

	return ret;
}

// 
/*!
    @brief print data name
	
	@param[in] object
	@param[in] file
	@param[in] ccys
	@param[in] isDetail
	@return bool
*/
bool 
MAValuePrinter::printAttr(LADataInstance *dataInstance, ofstream &file, const LAString &ccys, bool isDetail, bool isvanilla, bool isSummary)
{
	
	if (isvanilla)
	{
		return true;
	}

	if (isSummary)
	{
		LAString output = "";
		output += "TransID,";
		output += "DirtyPrice,";
		output += "PVCurrency,";

		output = output.subString(0, output.size() - 2);
		file << output.getCString() << endl;

		return true;
	}
	
	double acc_tmp = 0;
	LADataHolder* dh;	
	// check accruedinterest reference rate (spot or forward)
	//leg object
	LAObjectPool& objPool = dataInstance->getObjectPool();
	const LAString portName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObjectHolder objHolder = objPool.getObject(portName, ENCHKTYPE_ISDEFINED);

	// for portfolio
	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
										(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
	
	bool ret = true;
	LAString output = "";
	output += "TransID,";
	output += "CleanPrice,";
	output += "DirtyPrice,";
	output += "AccruedInterest,";
	output += "AverageLife,";

	if (isDetail)
	{
		output += "CallTriggerValue,";
		output += "CleanPriceWithoutCallTriggerValue,";
	}

	//print base currency
	output += "PVCurrency,";

	LAStringVector ccyVec = ccys.toToken(MULTI_STATIC_DATA_DELIMITER);
	const int ccySize = ccyVec.size();
	for (int i = 0; i < ccySize; ++i)
	{
		if (ccyVec[i].findString(FX_DELIMITER) >= 0)
		{
			break;
		}
		// cash print
		LAString cAttrStr = ccyVec[i].toUpper() + "_" + PRICING_DATA_CASH;
		output += cAttrStr;
		output += ",";
	}

	//print funding spread
	output += "FundingSpread,";
	//print fee excluded pv
	output += "FeeExcludedPV,";

	output = output.subString(0, output.size() - 2);

	file << output.getCString() << endl;

	return ret;
}

// 
/*!
    @brief return value string
	
	@param[in] name
	@return LAString
*/
LAString
MAValuePrinter::outStr(double value)
{
	if (LAMath::abs(value) < P_DBL_MAX)
	{
		return LAString(value);
	}
	else
	{
		char str[20];
		sprintf(str, "%e", value);
		return LAString(str);
	}
}

void
MAValuePrinter::changeSemiAnalyticResult(LADataInstance& dataInstance,const LAString& infileName, std::ofstream &file)
{
	
	// asOfDate
	LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData(); 

	//KEY_SEMIANALYTIC_ OMIT_DIGITNUMBER
	LAString digit = staticData.getStaticData(KEY_SEMIANALYTIC_OMIT_DIGITNUMBER);
	digit.toUpper();
	bool isomit = (digit != MLIB_NO_DATA);
	unsigned int digitnum = 0;
	if (isomit)
		digitnum = static_cast<unsigned int>(digit.getIntValue());
	double omitborder = LAMath::pow(0.1,digitnum);

	MAFileAccessor zeroFile(infileName);
	LAStringMatrix zeroData;
	zeroFile.readAllData(MARKET_DATA_DELIMITER, zeroData);
	zeroFile.close();
	if (zeroData.empty() || zeroData[0].size() < 2)
	{
		throw LACoreInvalidData("SemiAnalytic Error",__FILE__,__LINE__);
	}

	LAString curveType = LACoreDataService::getContext(ARG_KEY_CURVETYPE);
	std::vector<LAString> curveTypes = curveType.toToken(',');
	LAStringMatrix keymat(curveTypes.size());
	LAStringVector curvenames(curveTypes.size());
	LAStringVector curvesuffixs(curveTypes.size());
	LAStringVector ccys(curveTypes.size());
	
	for (unsigned int k = 0; k < keymat.size(); k++)
	{
		keymat[k] = curveTypes[k].toToken('_');
		if (keymat[k].size() != 2)
				throw LACoreInvalidData("CurveTypes Error",__FILE__,__LINE__);

		ccys[k] = keymat[k][0];
		ccys[k].toLower();
		curvenames[k] = keymat[k][1];
		/*if (keymat[k][1] == "STD")
			curvenames[k] = "BaseCurve";*/

		
		curvesuffixs[k] = "." +keymat[k][1];
		curvesuffixs[k].toLower();
		if (keymat[k][1] == "STD")
			curvesuffixs[k] = "";
	}
	
	//zerodata[0]... tradeid
	//zerodata[1]... scenario
	//zerodata[2]... riskval
	//zerodata[3]... outputccy
	//zerodata[4]... curvename
	std::map<LAString, LAStringVector> pvoutmap;
	std::map<LAString, LAStringVector> paraoutmap;
	std::map<LAString, LAStringVector> gridtermmap;
	std::map<LAString, DoubleVector> gridriskmap;
	std::map<LAString, LAStringVector> gridmatoutmap;
	LAStringVector termgrids;
	DoubleVector termvals;
	DoubleVector riskvals;
	//LAString tmptradeid = zeroData[0][0];
	for (unsigned int i = 0; i < zeroData.size(); i++)
	{
		LAString tradeid = zeroData[i][0];
		if (zeroData[i][1] == "PV" && pvoutmap.find(tradeid) == pvoutmap.end())
		{
			pvoutmap[tradeid] = zeroData[i];
			continue;
		}

		/*if (tmptradeid != tradeid)
		{
			tmptradeid = zeroData[i][0];
			continue;
		}*/

		for (unsigned int k = 0; k < keymat.size(); k++)
		{

			if (curvenames[k] != "STD" && zeroData[i][4] != curvenames[k])
				continue;

			if (curvenames[k] == "STD" && zeroData[i][4] != "BaseCurve")
				continue;
			
			LAString inputriskname = staticData.getStaticData(ccys[k] + CCY_SEMIANALYTIC_INPUTRISKNAME + curvesuffixs[k]);
			
			if (zeroData[i][1].findString(inputriskname) != -1)
			{
				LAStringVector riskdetails = zeroData[i][1].toToken('_');
				if (riskdetails.back() == "PARALLEL")
				{
					LAString outputriskname = staticData.getStaticData(ccys[k] + CCY_SEMIANALYTIC_OUTPUTRISKNAME + curvesuffixs[k]);
					LAString outputccy = zeroData[i][3];
					paraoutmap[tradeid + ";" + outputriskname + ";" + outputccy + ";" + curvenames[k] + ";" + ccys[k]] = zeroData[i];
					continue;

				}
				else
				{
					LAString termstr = zeroData[i][1].toToken('_').back();
					double riskval = zeroData[i][2].getDoubleValue();
					LAString outputriskname = staticData.getStaticData(ccys[k] + CCY_SEMIANALYTIC_OUTPUTRISKNAME + curvesuffixs[k]);
					LAString outputccy = zeroData[i][3];
					
					gridtermmap[tradeid + ";" + outputriskname + ";" + outputccy + ";" + curvenames[k] + ";" + ccys[k]].push_back(termstr);
					gridriskmap[tradeid + ";" + outputriskname + ";" + outputccy + ";" + curvenames[k] + ";" + ccys[k]].push_back(riskval);

					
				}
			}
		}
	}

	std::map<LAString, LAStringVector>::iterator it =  gridtermmap.begin();
	for (it =  gridtermmap.begin(); it != gridtermmap.end(); ++it)
	{
		LAString key = it->first;
		LAStringVector termstrs = it->second;
		DoubleVector riskvals = gridriskmap[key];
		DoubleVector termvals(termstrs.size());

		for (unsigned int i = 0; i < termstrs.size(); i++)
		{
			termvals[i] = LAMarketData::getCalendarTime(asOfDate,termstrs[i]);
		}

		LAStringVector keyvec = key.toToken(';');
		if (keyvec.size() !=5)
			throw LACoreInvalidData("SemiAnalytic Error",__FILE__,__LINE__);


		LAString keyccy = keyvec[4];
		keyccy.toUpper();
		LAString proName = LAMarketData::getBaseYieldProName(keyccy);
		LAMathYieldCurvePro& yldpro = dynamic_cast<LAMathYieldCurvePro &>(dataInstance.getObjectPool().getObject(proName, ENCHKTYPE_ISDEFINED).get());

		LAString curveType = keyvec[3];
		LAStringVector marketterms;
		DoubleVector marketvals;
		yldpro.changeZeroRiskIntoMarketRisk(curveType,termvals,riskvals,marketterms,marketvals);

		LAString tradeid = keyvec[0];
		LAString outputrisk = keyvec[1];
		
		LAString outputccy = keyvec[2];
		outputccy.toUpper();

		LAStringVector outvec;
		for (unsigned int i = 0; i < marketterms.size(); i++)
		{
			if (isomit && LAMath::abs(marketvals[i]) < omitborder)
					continue;

			LAString valstr;

			valstr = tradeid + ",";
			valstr += outputrisk + "_" + marketterms[i] + ",";
			valstr += LAString(marketvals[i]) + ",";
			valstr += outputccy + ",";
			valstr += (curveType != "STD") ? curveType : "BaseCurve";
			outvec.push_back(valstr);
		}
		gridmatoutmap[key] = outvec;
		
		//change ParallelValue
		if (paraoutmap.find(key) != paraoutmap.end())
		{
			LAStringVector chgvec = paraoutmap[key];
			chgvec[1] = outputrisk + "_" + "PARALLEL";
			chgvec[2] = LAString(std::accumulate(marketvals.begin(), marketvals.end(), 0.0));
			paraoutmap[key] = chgvec;
		}
	
	}


	///output
	std::map<LAString, LAStringVector>::iterator itout = gridmatoutmap.begin();
	for (itout = gridmatoutmap.begin(); itout != gridmatoutmap.end(); ++itout)
	{
		LAString tradeid = itout->first.toToken(';')[0];
		if (!pvoutmap[tradeid].empty())
		{
			LAString output;
			LAStringVector vec = pvoutmap[tradeid];
			for (unsigned int i = 0; i < vec.size(); i++)
			{
				output += vec[i] + ",";
			}
			output.subString(0,output.size()-2);
			file << output.getCString() << endl;
		}

		LAString key = itout->first;

		if (!paraoutmap[key].empty())
		{
			LAString output;
			LAStringVector vec = paraoutmap[key];
			for (unsigned int i = 0; i < vec.size(); i++)
			{
				output += vec[i] + ",";
			}
			output.subString(0,output.size()-2);
			file << output.getCString() << endl;
		}

		
		for (unsigned int i = 0; i < itout->second.size(); i++)
			file << itout->second[i].getCString() << endl;
	}

	return;
}

int MAValuePrinter::printEntityDump(const std::vector< LAString >& fileNumVec,
                                    const std::vector< LADataInstance* >& rootVec
                                    )
{
    int ret = SUCCESS_CODE;

    try
    {
        LAString fnameBase = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(KEY_ENTITY_DUMP_FILE);
        if(fnameBase == MLIB_NO_DATA)
        {
            throw LACoreInvalidData("object dump file name is not defined.", __FILE__, __LINE__);
        }
        const int dotPos = fnameBase.findString('.');
        LAString fnameBody = fnameBase.subString(0, dotPos-1);
        LAString fnameExt = fnameBase.subString(dotPos, fnameBase.size()-1);
        for(unsigned int i = 0; i < fileNumVec.size(); ++i)
        {
            std::ofstream ofs((fnameBody + fileNumVec[i] + fnameExt).getCString());
            char* buf = NULL;
            unsigned long len;
            rootVec[i]->serialize(buf, len);
            ofs << buf;
            delete[] buf; buf = NULL;
        }
    }
    catch(LACoreError& e)
    {
        LAString msg = "Error in object dump:";
        msg += e.getMsg();
        LACoreDataService::getLogManager().getLogger().error(msg.getCString(), __FILE__, __LINE__);
        std::cerr << msg.getCString() << std::endl;
        ret = ALL_ERROR_CODE;
    }
    catch(...)
    {
        LAString msg = "Error in object dump:";
        LACoreDataService::getLogManager().getLogger().error(msg.getCString(), __FILE__, __LINE__);
        std::cerr << msg.getCString() << std::endl;
        ret = ALL_ERROR_CODE;
    }

    return ret;
}
int MAValuePrinter::printCompoundedRates(const LAObject& trade,
						                 const LAString& fileNum,
										 const LADataInstance* dataInstance,
										 const LAString& ccy,
										 std::ostream& dest,
										 LAString calc)
{
	int ret = SUCCESS_CODE;

	try{
		printCompoundedRates(1, trade, fileNum, dataInstance, ccy, dest, calc);
		printCompoundedRates(2, trade, fileNum, dataInstance, ccy, dest, calc);
	}
	catch(LACoreError& e){
		LAString msg = "Error in printCompoundedRates:";
		msg += e.getMsg();
        LACoreDataService::getLogManager().getLogger().error(msg.getCString(), __FILE__, __LINE__);
        std::cerr << msg.getCString() << std::endl;
        ret = ALL_ERROR_CODE;
	}
	catch(...){
		LAString msg = "Error in printCompoundedRates:";
        LACoreDataService::getLogManager().getLogger().error(msg.getCString(), __FILE__, __LINE__);
        std::cerr << msg.getCString() << std::endl;
        ret = ALL_ERROR_CODE;
	}

	return ret;
}

void MAValuePrinter::printCompoundedRates(const int leg_num,
	 					                  const LAObject& trade,
										  const LAString& fileNum,
										  const LADataInstance* dataInstance,
										  const LAString& ccy,
										  std::ostream& dest,
										  LAString calc)
{
	const LADataHolder* dh;
	dh = &trade.getData(LAString(PRICING_DATA_COMPOUNDING_STARTDATE) + "_Leg_" + LAString(leg_num), NOCHECK);
	if(!dh->isDefined() || dh->isNull()) return;

    const LAString& name = dynamic_cast<const LADataString&>(trade.getData(CALIBRATION_DATA_NAME).get()).get();

	dest 
        << name.getCString()
		<< "_CompoundedRate_Leg" << leg_num << endl
		<< "CFCalcStartDate,CFCalcEndDate,Term,FixingDate,FixingFlag,Forward/FixedRate" << endl;
	DateVector start, end, fixd;
    LAStringVector fixf;
	DoubleVector term, rate;

	start = dynamic_cast<const LADataDates&>(dh->get()).get();
	dh = &trade.getData(LAString(PRICING_DATA_COMPOUNDING_ENDDATE) + "_Leg_" + LAString(leg_num), ISNOTNULL);
	end = dynamic_cast<const LADataDates&>(dh->get()).get();
	dh = &trade.getData(LAString(PRICING_DATA_COMPOUNDING_FIXDATE) + "_Leg_" + LAString(leg_num), ISNOTNULL);
	fixd = dynamic_cast<const LADataDates&>(dh->get()).get();
    dh = &trade.getData(LAString(PRICING_DATA_COMPOUNDING_FIXFLAG) + "_Leg_" + LAString(leg_num), ISNOTNULL);
    fixf = dynamic_cast<const LADataStrings&>(dh->get()).get();
	dh = &trade.getData(LAString(PRICING_DATA_COMPOUNDEDTERM) + "_Leg_" + LAString(leg_num), ISNOTNULL);
	term = dynamic_cast<const LADataDoubles&>(dh->get()).get();
	dh = &trade.getData(LAString(PRICING_DATA_COMPOUNDEDRATE) + "_Leg_" + LAString(leg_num), ISNOTNULL);
	rate = dynamic_cast<const LADataDoubles&>(dh->get()).get();

	for(size_t i = 0; i < start.size(); i++){
        LAString output;
        output += LADataDate(start.at(i)).convertToString() + ",";
        output += LADataDate(end.at(i)).convertToString() + ",";
        output += outStr(term.at(i)) + ",";
        output += LADataDate(fixd.at(i)).convertToString() + ",";
        output += fixf.at(i) + ",";
        output += outStr(rate.at(i));
        dest << output.getCString() << endl;
	}
	dest << endl;
}


void MAValuePrinter::printFxOptionMatrix(LADataInstance* dataInstance, const LAString& filenum)
{
    const LAString fname_base = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(KEY_DEAL_FXOPT_BULK_OUTFILE);
    if(fname_base == MLIB_NO_DATA) return;


    const unsigned int dot_pos = fname_base.findString(".");
    const LAString fname_body = fname_base.subString(0, dot_pos-1);
    const LAString fname_ext = fname_base.subString(dot_pos, fname_base.size()-1);
    std::ofstream ofs((fname_body + filenum + fname_ext).getCString());



    LADataStringMatrix temp; temp.convertFromString(LACoreDataService::getStaticDataManager().getStaticData().getStaticData(KEY_DEAL_FXOPT_BULK));
    const LAStringMatrix opt_mat = temp.get();
    const size_t nrow = opt_mat.size();
    const size_t ncol = opt_mat[0].size();

    LAObjectPool& objPool = dataInstance->getObjectPool();






    LADataStrings temp2; temp2.convertFromString(LACoreDataService::getStaticDataManager().getStaticData().getStaticData(KEY_DEAL_FXOPT_BULK_LABEL));
    if(temp2.getSize() != ncol) throw LACoreInvalidData("fx option bulk label and matrix are inconsistent.", __FILE__, __LINE__);
    ofs << "Maturity,Forward,";
    for(size_t j = 0; j < ncol; j++){
        ofs << temp2.get().at(j) << " PV," << temp2.get().at(j) << " Volatility";
        if(j != ncol-1) ofs << ",";
    }
    ofs << endl;


    LAObjectHolder objHolder;
    LADataHolder dh;
    for(size_t i = 0; i < nrow; i++){
        for(size_t j = 0; j < ncol; j++){
            objHolder = objPool.getObject(opt_mat[i][j], ENCHKTYPE_ISDEFINED);

            const LAStringVector& names = dynamic_cast<const LADataStrings&>(objHolder.getData(PRICING_DATA_ANALYTICPARAMNAME, ISNOTNULL).get()).get();
            const DoubleVector& vals  = dynamic_cast<const LADataDoubles&>(objHolder.getData(PRICING_DATA_ANALYTICPARAM, ISNOTNULL).get()).get();
            const double* forward;
            for(size_t k = 0; k < names.size(); k++){
                if(names[k] == "Forward") {
                    forward = &vals[k];
                    break;
                }
            }
            if(j==0){
                dh = objHolder.getData(PRICING_DATA_EXPIRYDATE, ISNOTNULL);
                ofs << dh.convertToString().getCString() << ",";
                ofs << *forward << ",";
            }



            dh = objHolder.getData(PRICING_DATA_DIRTYPRICE, ISNOTNULL);
            ofs << outStr(dynamic_cast<const LADataDouble&>(dh.get()).get()) << ",";
            dh = objHolder.getData(PRICING_DATA_VOLATILITYRESULTOFPREMIUM, ISNOTNULL);
            ofs << outStr(dynamic_cast<const LADataDouble&>(dh.get()).get());
            if(j != ncol-1) ofs << ",";
        }
        ofs << endl;
    }
}
