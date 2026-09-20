/*! 
	@file
    @brief Class declaration to Generate ArbitrageFreeCurve.
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLNl2sol.h"

#include "AQLPriceYieldGenerator.h"
#include "AQLPriceArbFreeGenerator.h"
#include "AQLLinearInterpolation.h"

#include <algorithm>
#include "AQLMathYieldCurvePro.h"
#include "AQLDateCalculations.h"
#include "AQLAlgorithm.h"
#include "AQLDataMatrix.h"

#ifndef DUMMY
#define DUMMY "DUMMY"
#endif

#ifndef AF3ML 
#define AF3ML "AF3ML"
#endif
#ifndef AF6ML
#define AF6ML "AF6ML"
#endif
#ifndef AFDF
#define AFDF "AFDF"
#endif

#define ROLLCONV_NORMAL "NORMAL"
#define ROLLCONV_EOM "EOM"

#define MIN_DF 1E-10
// for calculation error of double
double eps = 1E-10;

/*!
    @brief  constructor
*/
AQLPriceArbFreeGenerator::AQLPriceArbFreeGenerator()
{
}
/*!
    @brief  destructor
*/
AQLPriceArbFreeGenerator::~AQLPriceArbFreeGenerator()
{
}
/*!
    @brief  Check function for this class ID

    @param[in] id type of class(function_t)
    @return true or false
*/
bool
AQLPriceArbFreeGenerator::isTypeOf(function_t id) const
{
    return (id == FN_IRARBFREEGENERATOR ? true :
                        AQLCoreProcedure::isTypeOf(id));
}
/*!
    @brief  Make copy(clone) of this class

    @return copy object
*/
AQLCoreFunctionBase*     
AQLPriceArbFreeGenerator::clone() const
{
    try 
    {
        return new AQLPriceArbFreeGenerator();
    }
    catch (bad_alloc & e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }   
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLPriceArbFreeGenerator::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
    dm.setData(CALIBRATION_DATA_TERMS, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_DFS, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_CALENDAR, DATA_CALENDAR);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_STARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ENDDATE, DATA_DATE);
	dm.setData(IR_CALIBRATION_DATA_DATATYPE, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_TERM, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_FREQUENCY, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_SPOTDATE, DATA_DATE);
	dm.setData(CALIBRATION_DATA_RATE, DATA_DOUBLE);
	dm.setData(IR_CALIBRATION_DATA_DAYCOUNT, DATA_DAYCOUNT);
}

/*!
    @brief  Return this class type
    @return Function Type(FN_IRYIELDGENERATOR)
*/
function_t          
AQLPriceArbFreeGenerator::getType() const
{
    return FN_IRARBFREEGENERATOR;
}

/*!
    @brief  function of generating yieldcurve
            get market data ,and caclation DF yield by moving function "calcDiscountFactor"
    @param[in] basedate spotdate
	@param[in,out] Reference of AQLMathYieldCurve Object 
    @param[in] att Data of Estimate Procedure (we don't use for this method)

*/
void AQLPriceArbFreeGenerator::calibrateModel( const AQLDate& basedate, 
										      AQLObject& object, 
										      const AQLDataProcedure& att ) const
{
    (void)att;
    const AQLMathYieldCurvePro& yg = dynamic_cast<AQLMathYieldCurvePro&>(object);

    // valarray of result
    DoubleArray dfTerms,dfCurve, 
                threeMLRate, threeMLTerms_DF, threeMLDF,
                sixMLRate, sixMLTerms_DF, sixMLDF, std_DF,
                floaterTerms, floaterPrices;
	DoubleMatrix threeMLTermsMtx_Rate, sixMLTermsMtx_Rate; 
	unsigned int threeMLSPos, sixMLSPos;

   // get curve ID and currency
	const AQLDataReference& ref = yg.getYieldData();
	AQLObjectPool& objPool = object.getDataInstance()->getObjectPool();
	AQLObjectHolder objHolder = ref.get();
    
    // foreign currency data
	const AQLDataReference& ref_fy = yg.getForeignYieldData();
	const AQLObject* fYieldData = NULL;
	if (!ref_fy.isNull())
	{	
		fYieldData= &ref_fy.get().get();
	}
	// get market date
	vector<AQLObject*> data;
    const AQLDataMultiReference& mr = yg.getMarketData();
    for(unsigned int i = 0; i < mr.getSize(); i++)
	{
		data.push_back(&mr.get(i).get());
	}
	//xccybasis
	const AQLDataHolder *dh = &yg.getData(CALIBRATION_DATA_MARKETDATA + AQLString("_") + AQLString(XCCYBASIS) ,NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLDataMultiReference& mr_xccy = dynamic_cast<const AQLDataMultiReference&> (dh->get());
		for(unsigned int i = 0; i < mr_xccy.getSize(); i++)
		{
			data.push_back(&mr_xccy.get(i).get());
		}
	}
	//3m6mbasis
	const AQLDataMultiReference& mr_3m6m = 
		dynamic_cast<const AQLDataMultiReference&> (yg.getData(CALIBRATION_DATA_MARKETDATA + AQLString("_") + AQLString(THREESIXBASIS), ISNOTNULL).get());
	for(unsigned int i = 0; i < mr_3m6m.getSize(); i++)
	{
		data.push_back(&mr_3m6m.get(i).get());
	}

    bool isFRAUse = dynamic_cast<const AQLDataBool& > ((yg.getData(IR_CALIBRATION_DATA_ISFRAUSE, ISNOTNULL)).get());
	bool isFutureUse = dynamic_cast<const AQLDataBool&>(yg.getIsFutureUse()).get();
	bool isRenAdj = dynamic_cast<const AQLDataBool& > ((yg.getData(IR_CALIBRATION_DATA_ISRENOTIONALADJUST,
																  IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, ISNOTNULL)).get()); // Alias Method: First Parameter Takes Priortity
    
    AQLInterpolationBase* pInter = dynamic_cast<AQLInterpolationBase*>(yg.getInterpolation().getMethod().clone());
	AQLCoreFunctionHolder fh(pInter, true);

	// data check
	bool isCheckCurves = false;
	dh = &yg.getData(IR_CALIBRATION_DATA_ISCURVEEXISTCHECK, NOCHECK);
	if (dh->isDefined() && !dh->isNull()) isCheckCurves = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	bool isCurveAttrExist = yg.checkCurveAttr(AF6ML) && yg.checkCurveAttr(AFDF) && yg.checkCurveAttr(AF3ML) && yg.checkCurveAttr(SWAP);
	if (!isCheckCurves || !isCurveAttrExist)
	{
		const AQLString& currency = dynamic_cast<AQLDataString& > ((objHolder.get().getData(IR_CALIBRATION_DATA_CURRENCY, ISNOTNULL)).get());
		if( currency == CURRENCY_USD )
		{
			generateUSDCurve( basedate, data, fYieldData, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF,
							  sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, threeMLSPos, sixMLSPos, isFRAUse, isFutureUse, isRenAdj, pInter );
		}
		else
		{
			generateCurve( basedate, data, fYieldData, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF,
						   sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, threeMLSPos, sixMLSPos, isFRAUse, isFutureUse, isRenAdj, pInter );
		}

		// check term
		DoubleArray::const_iterator it = min_element(dfTerms.begin(), dfTerms.end());
		if (!dfTerms.empty() && *it < 0.0)
		{
			throw AQLCoreInvalidData("dfTerms, term must be positive.", __FILE__, __LINE__);
		}
		it = min_element(threeMLTerms_DF.begin(), threeMLTerms_DF.end());
		if (!threeMLTerms_DF.empty() && *it < 0.0)
		{
			throw AQLCoreInvalidData("threeMLTerms_DF, term must be positive.", __FILE__, __LINE__);
		}
		it = min_element(sixMLTerms_DF.begin(), sixMLTerms_DF.end());
		if (!sixMLTerms_DF.empty() && *it < 0.0)
		{
			throw AQLCoreInvalidData("sixMLTerms_DF, term must be positive.", __FILE__, __LINE__);
		}
		if (!threeMLTermsMtx_Rate.empty() && !threeMLTermsMtx_Rate[0].empty())
		{
			it = min_element(threeMLTermsMtx_Rate[0].begin(), threeMLTermsMtx_Rate[0].end());
			if (*it < 0.0)
			{
				throw AQLCoreInvalidData("threeMLTermsMtx_Rate, term must be positive.", __FILE__, __LINE__);
			}
		}
		if (!sixMLTermsMtx_Rate.empty() && !sixMLTermsMtx_Rate[0].empty())
		{
			it = min_element(sixMLTermsMtx_Rate[0].begin(), sixMLTermsMtx_Rate[0].end());
			if (*it < 0.0)
			{
				throw AQLCoreInvalidData("sixMLTermsMtx_Rate, term must be positive.", __FILE__, __LINE__);
			}
		}
		//set DF Curve
		AQLStringVector curveNames_6ML;
		AQLStringVector curveNames_DF;
		AQLStringVector curveNames_3ML;
		dh = &objHolder.get().getData(IR_CALIBRATION_DATA_6MLCURVENAMES, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			curveNames_6ML = dynamic_cast<const AQLDataStrings& > (dh->get()).get();
		}
		dh = &objHolder.get().getData(IR_CALIBRATION_DATA_DFCURVENAMES, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			curveNames_DF = dynamic_cast<const AQLDataStrings& > (dh->get()).get();
		}
		dh = &objHolder.get().getData(IR_CALIBRATION_DATA_3MLCURVENAMES, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{
			curveNames_3ML = dynamic_cast<const AQLDataStrings& > (dh->get()).get();
		}
		objHolder.remove(CALIBRATION_DATA_ASOFDATE);
		objHolder.remove(CALIBRATION_DATA_TERMS);
		objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX);
		objHolder.remove(IR_CALIBRATION_DATA_DFS);
		objHolder.remove(IR_CALIBRATION_DATA_DFS2);
		objHolder.remove(CALIBRATION_DATA_INTERPOLATION);
		objHolder.add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(basedate));
		objHolder.add(CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation(yg.getInterpolation()));

		for (size_t i=0; i<curveNames_DF.size(); i++)
		{
			if(curveNames_DF[i]==STD)
			{
				throw AQLCoreInvalidData("ArbFree DF Curve Name is not STD!", __FILE__, __LINE__);	
			}
			objHolder.remove(CALIBRATION_DATA_TERMS + AQLString("_") + curveNames_DF[i]);
			objHolder.remove(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveNames_DF[i]);	
			objHolder.add(CALIBRATION_DATA_TERMS + AQLString("_") + curveNames_DF[i] , new AQLDataDoubles(dfTerms));
			objHolder.add(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveNames_DF[i], new AQLDataDoubles(dfCurve));
		}

		bool isSTDExist = false;
		for (size_t i=0; i<curveNames_6ML.size(); i++)
		{
			if(curveNames_6ML[i]==STD)
			{
				objHolder.add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(sixMLTerms_DF));
				objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new AQLDataDoubleMatrix(sixMLTermsMtx_Rate));
				objHolder.add(IR_CALIBRATION_DATA_DFS, new AQLDataDoubles(sixMLDF));
				isSTDExist = true;
			}
			else
			{
				objHolder.remove(CALIBRATION_DATA_TERMS + AQLString("_") + curveNames_6ML[i]);
				objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + AQLString("_") + curveNames_6ML[i]);
				objHolder.remove(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveNames_6ML[i]);
				objHolder.add(CALIBRATION_DATA_TERMS + AQLString("_") + curveNames_6ML[i] , new AQLDataDoubles(sixMLTerms_DF));
				objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + AQLString("_") + curveNames_6ML[i] , new AQLDataDoubleMatrix(sixMLTermsMtx_Rate));
				objHolder.add(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveNames_6ML[i], new AQLDataDoubles(sixMLDF));
			}
		}

		for (size_t i=0; i<curveNames_3ML.size(); i++)
		{
			if(curveNames_3ML[i]==STD)
			{
				// error check
				if(isSTDExist)
				{
					throw AQLCoreInvalidData("there exist more than two STD Curve name!", __FILE__, __LINE__);
				}

				objHolder.add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(threeMLTerms_DF));
				objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new AQLDataDoubleMatrix(threeMLTermsMtx_Rate));
				objHolder.add(IR_CALIBRATION_DATA_DFS, new AQLDataDoubles(threeMLDF));

				isSTDExist = true;
			}
			else
			{
				objHolder.remove(CALIBRATION_DATA_TERMS + AQLString("_") + curveNames_3ML[i]);
				objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + AQLString("_") + curveNames_3ML[i]);
				objHolder.remove(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveNames_3ML[i]);
				objHolder.add(CALIBRATION_DATA_TERMS + AQLString("_") + curveNames_3ML[i] , new AQLDataDoubles(threeMLTerms_DF));
				objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX + AQLString("_") + curveNames_3ML[i] , new AQLDataDoubleMatrix(threeMLTermsMtx_Rate));
				objHolder.add(IR_CALIBRATION_DATA_DFS + AQLString("_") + curveNames_3ML[i], new AQLDataDoubles(threeMLDF));
			}
		}
		
		if (!isSTDExist)
		{
			vector<AQLObject*> data_swap;
			for(unsigned i = 0; i < data.size(); i++)
			{
				// check use grid
				const AQLDataHolder *dh = &data[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
				if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

				AQLString dataType = dynamic_cast<const AQLDataString&> ((data[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
				dataType.toUpper();

				if (dataType == PAR) data_swap.push_back(data[i]);//swap case
			}

			if (data_swap.size() == 0)
			{
				throw AQLCoreInvalidData("No Swap Market Object", __FILE__, __LINE__);
			}
			else
			{
				const AQLString& freq_Float = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, ISNOTNULL)).get());
				if ( freq_Float == SEMI_ANNUAL )
				{
					objHolder.add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(sixMLTerms_DF));
					objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new AQLDataDoubleMatrix(sixMLTermsMtx_Rate));
					objHolder.add(IR_CALIBRATION_DATA_DFS, new AQLDataDoubles(sixMLDF));
				}
				else if ( freq_Float == QUARTERLY )
				{
					objHolder.add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(threeMLTerms_DF));
					objHolder.add(CALIBRATION_DATA_FWDTERMSMATRIX, new AQLDataDoubleMatrix(threeMLTermsMtx_Rate));
					objHolder.add(IR_CALIBRATION_DATA_DFS, new AQLDataDoubles(threeMLDF));
				}
				else
				{
					throw AQLCoreInvalidData("swap floating leg frequecy is not supported.", __FILE__, __LINE__);
				}
			}
		}
		
		setCurveConvention(objHolder, data, curveNames_3ML, curveNames_6ML);
	}
}

/*
    @brief function to calclation DiscountFactor from market data
    
    @param[in] basedate                 basedate
	@param[in] mktData                  input market data
    @param[out] dfTerms		            output terms
    @param[out] dfCurve		            output discount factor at terms
    @param[out] threeMLTerms_Rate       output terms as rate of 3m Libor
    @param[out] threeMLRate	            output forward 3m Libor at terms
    @param[out] threeMLTerms_Rate       output terms as DF of 3m Libor
    @param[out] threeMLDF	            output DF base forward 3m Libor at terms
    @param[out] sixMLTerms_Rate         output terms as rate of 6m Libor 
    @param[out] sixMLRate		        output forward 6m Libor at terms
    @param[out] sixMLTerms_DF           output terms as DF of 6m Libor 
    @param[out] sixMLDF 		        output DF base forward 6m Libor at terms
	@param[out] threeMLSPos             output swap start pos of 3m Libor
	@param[out] sixMLSPos               output swap start pos of 6m Libor

    Suppose that
    data[0] money market
    data[1] libor market
    data[2] swap market
    data[3] currency basis market
    data[4] 3L6L basis market   
    data[5] 3LFRA
    data[6] 6LFRA
*/
void 
AQLPriceArbFreeGenerator::
generateCurve( const AQLDate& basedate,
			   const std::vector<AQLObject*>& mktData,
			   const AQLObject* fYieldData,
               DoubleArray& dfTerms,
			   DoubleArray& dfCurve,
               DoubleMatrix& threeMLTermsMtx_Rate,
               DoubleArray& threeMLRate,
               DoubleArray& threeMLTerms_DF,
               DoubleArray& threeMLDF,
               DoubleMatrix& sixMLTermsMtx_Rate,
               DoubleArray& sixMLRate,
               DoubleArray& sixMLTerms_DF,
               DoubleArray& sixMLDF,
			   unsigned int& threeMLSPos,
			   unsigned int& sixMLSPos,
               bool isFRAUse,
			   bool isFutureUse, 
			   bool isRenAdj,
			   AQLInterpolationBase* pInter_DF )
{
	if (isFutureUse) throw AQLCoreInvalidData("Furue is not supported!",__FILE__,__LINE__);

    dfTerms.clear();
    dfCurve.clear();
    threeMLTermsMtx_Rate.clear();
	threeMLTermsMtx_Rate.resize(2);
    threeMLRate.clear();
    threeMLTerms_DF.clear();
    threeMLDF.clear();
    sixMLTermsMtx_Rate.clear();
	sixMLTermsMtx_Rate.resize(2);
    sixMLRate.clear();
    sixMLTerms_DF.clear();
    sixMLDF.clear();
	threeMLSPos = 0;
	sixMLSPos = 0;

    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	vector<const AQLObject*> data_mon, data_libor, data_fra6m, data_fra3m, data_swap, data_3m6m, data_xccy;
	unsigned int size_data = mktData.size();
	AQLString dataType, dataName;
	for(unsigned i = 0; i < size_data; i++)
	{
		// check use grid
		const AQLDataHolder *dh = &mktData[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

		dataType = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		dataType.toUpper();

		dataName = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL)).get()).get();
		dataName.toUpper();

		if (dataType == ZERO) data_libor.push_back(mktData[i]);//libor case
		else if (dataType == PAR) data_swap.push_back(mktData[i]);//swap case
		else if (dataType == FRA6M) data_fra6m.push_back(mktData[i]);//6mfra case
		else if (dataType == FRA3M) data_fra3m.push_back(mktData[i]);//3mfra case
		else if (dataType == BASIS && dataName.findString(THREESIXBASIS) != -1) data_3m6m.push_back(mktData[i]);//3m6m basis swap
		else if (dataType == BASIS && dataName.findString(XCCYBASIS) != -1) data_xccy.push_back(mktData[i]);//xccy basis swap
		else if (dataType == O_N) data_mon.push_back(mktData[i]);//on case
		else if (dataType == T_N) data_mon.push_back(mktData[i]);//tn case
	}

	sort(data_libor.begin(), data_libor.end(), Comp_term());
	sort(data_swap.begin(), data_swap.end(), Comp_term());
	sort(data_3m6m.begin(), data_3m6m.end(), Comp_term());
	sort(data_xccy.begin(), data_xccy.end(), Comp_term());
	sort(data_fra3m.begin(), data_fra3m.end(), Comp_term());
	sort(data_fra6m.begin(), data_fra6m.end(), Comp_term());
	
    const MoneyMarket monMkt(data_mon);
    const LiborMarket libMkt(data_libor);
    const SwapMarket swapMkt(data_swap,data_3m6m,libMkt,basedate);
    const XCCYBasisMarket xccyBasisMkt(data_xccy,fYieldData,swapMkt,isRenAdj,false,basedate);
    const LiborBasisMarket libBasisMkt(data_3m6m,swapMkt,basedate);
	const FRAMarket fra3MLMkt(data_fra3m,isFRAUse);
	const FRAMarket fra6MLMkt(data_fra6m,isFRAUse);

	if (isFRAUse)
	{
		if (fra3MLMkt.map_term_rate.find("3M") == fra3MLMkt.map_term_rate.end()) throw AQLCoreInvalidData("3X6 FRA No Data!", __FILE__, __LINE__);
		if (fra3MLMkt.map_term_rate.find("6M") == fra3MLMkt.map_term_rate.end()) throw AQLCoreInvalidData("6X9 FRA No Data!", __FILE__, __LINE__);
	}
    
	if( libMkt.dc == swapMkt.dc_Float ) ;
    else throw AQLCoreInvalidData("a daycount of Libor is different from a daycount of swap!", __FILE__, __LINE__);

    //set term grid. use swap convention
    DoubleArray termGrid_3MRoll;
    DoubleArray termGrid_6MRoll;
	DoubleArray termGrid_12MRoll;
    DateVector dateGrid_3MRoll;
    DateVector dateGrid_6MRoll;
	DateVector dateGrid_12MRoll;

	generateSchedule(swapMkt.spotDate, swapMkt.spotDate, swapMkt.endDate, 3, swapMkt.pCal, swapMkt.sld, dateGrid_3MRoll, termGrid_3MRoll, swapMkt.isEOMRoll);  
    generateSchedule(swapMkt.spotDate, swapMkt.spotDate, swapMkt.endDate, 6, swapMkt.pCal, swapMkt.sld, dateGrid_6MRoll, termGrid_6MRoll, swapMkt.isEOMRoll);  
	generateSchedule(swapMkt.spotDate, swapMkt.spotDate, swapMkt.endDate, 12, swapMkt.pCal, swapMkt.sld, dateGrid_12MRoll, termGrid_12MRoll, swapMkt.isEOMRoll);  
    size_t dateSize_3M = termGrid_3MRoll.size();
    size_t dateSize_6M = termGrid_6MRoll.size();
	size_t dateSize_12M = termGrid_12MRoll.size();


    //interpolate market data
    //3L6L basis
    DoubleArray threeSixBasisGrid;
    for(size_t j = 1; j<dateSize_6M; j++)
    {
        threeSixBasisGrid.push_back( libBasisMkt.pInter->value(swapMkt.termGrid_6MRoll[j]) );
    }
    //Currency basis
    DoubleArray currBasisGrid;
    for(size_t i = 1; i<dateSize_3M; i++)
    {
        currBasisGrid.push_back( xccyBasisMkt.pInter->value(swapMkt.termGrid_3MRoll[i]) );
    }
    //Swap
    DoubleArray swapRateGrid;
	if (swapMkt.freq_Fix == ANNUAL)
	{
		for(size_t j = 1; j< dateSize_12M; j++)
		{
			swapRateGrid.push_back( swapMkt.pInter->value(swapMkt.termGrid_12MRoll[j]) );
		}
	}
	else if (swapMkt.freq_Fix == SEMI_ANNUAL || swapMkt.freq_Fix == QUARTERLY)
	{
		for(size_t j = 1; j< dateSize_6M; j++)
		{
			swapRateGrid.push_back( swapMkt.pInter->value(swapMkt.termGrid_6MRoll[j]) );
		}
	}   
	else
	{
		throw AQLCoreInvalidData("Frequency is not supported.", __FILE__, __LINE__);
	}

	double ANN_Swap=0.,ANN_Swap_6M=0.,ANN_Swap_3M=0.,ANN_3L6L_3L=0.,ANN_3L6L_6L=0.,ANN_Curr=0.,PV_Swap=0.,PV_Swap_3L=0.,PV_3L6L_3L=0.,PV_3L6L_6L=0.,PV_Curr=0.;
    double spotAdjust = getSpotAdjust(basedate, swapMkt.spotDate,monMkt);
	double spotTerm = dc_act365.getTerm(basedate, swapMkt.spotDate);
	double I = 1.;

	if (swapMkt.freq_Fix == ANNUAL)
	{
		for(size_t j=0; j<dateSize_12M-1; j++)
		{
			if (swapMkt.freq_Float == SEMI_ANNUAL)
			{
				calcCurve_Annu_Semi(basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, sixMLRate,
					sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, fra3MLMkt, fra6MLMkt, swapRateGrid,
					currBasisGrid, threeSixBasisGrid, dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll, 
					PV_Swap, PV_3L6L_6L, PV_3L6L_3L, PV_Curr, ANN_Swap, ANN_3L6L_6L, ANN_3L6L_3L, ANN_Curr, I, spotAdjust, spotTerm, j);
			}
			else if (swapMkt.freq_Float == QUARTERLY)
			{
				calcCurve_Annu_Quar(basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, sixMLRate,
					sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, fra3MLMkt, fra6MLMkt, swapRateGrid,
					currBasisGrid, threeSixBasisGrid, dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll, 
					PV_Swap, PV_3L6L_6L, PV_3L6L_3L, PV_Curr, ANN_Swap, ANN_3L6L_6L, ANN_3L6L_3L, ANN_Curr, I, spotAdjust, spotTerm, j);
			}
			else
			{
				throw AQLCoreInvalidData("Frequency is not supported.", __FILE__, __LINE__);
			}
		}
	}
	else
	{
		for(size_t j=0; j<dateSize_6M-1; j++)
		{
			AQLString frequencyFix = getSwapFixFrequency(6*(j+1), swapMkt);
			if (frequencyFix == SEMI_ANNUAL && swapMkt.freq_Float == SEMI_ANNUAL)
			{
				calcCurve_Semi_Semi(basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, sixMLRate,
					sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, fra3MLMkt, fra6MLMkt, swapRateGrid,
					currBasisGrid, threeSixBasisGrid, dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll, 
					PV_Swap, PV_3L6L_6L, PV_3L6L_3L, PV_Curr, ANN_Swap_6M, ANN_Swap_3M, ANN_3L6L_6L, ANN_3L6L_3L, ANN_Curr, I, spotAdjust, spotTerm, j);
			}
			else if (frequencyFix == QUARTERLY && swapMkt.freq_Float == QUARTERLY)
			{
				calcCurve_Quar_Quar(basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, sixMLRate,
					sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, fra3MLMkt, fra6MLMkt, swapRateGrid,
					currBasisGrid, threeSixBasisGrid, dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll, 
					PV_Swap, PV_3L6L_6L, PV_3L6L_3L, PV_Curr, ANN_Swap_6M, ANN_Swap_3M, ANN_3L6L_6L, ANN_3L6L_3L, ANN_Curr, I, spotAdjust, spotTerm, j);
			}
			else if (frequencyFix == QUARTERLY && swapMkt.freq_Float == SEMI_ANNUAL)
			{
				calcCurve_Quar_Semi(basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, sixMLRate,
					sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, fra3MLMkt, fra6MLMkt, swapRateGrid,
					currBasisGrid, threeSixBasisGrid, dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll, 
					PV_Swap, PV_3L6L_6L, PV_3L6L_3L, PV_Curr, ANN_Swap_6M, ANN_Swap_3M, ANN_3L6L_6L, ANN_3L6L_3L, ANN_Curr, I, spotAdjust, spotTerm, j);
			}
			else if (frequencyFix == SEMI_ANNUAL && swapMkt.freq_Float == QUARTERLY)
			{
				calcCurve_Semi_Quar(basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, sixMLRate,
					sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, fra3MLMkt, fra6MLMkt, swapRateGrid,
					currBasisGrid, threeSixBasisGrid, dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll, 
					PV_Swap, PV_3L6L_6L, PV_3L6L_3L, PV_Curr, ANN_Swap_6M, ANN_Swap_3M, ANN_3L6L_6L, ANN_3L6L_3L, ANN_Curr, I, spotAdjust, spotTerm, j);
			}
			else
			{
				throw AQLCoreInvalidData("Frequency is not supported.", __FILE__, __LINE__);
			}
		}
	}

    dfTerms.insert( dfTerms.begin(), 0. );
    dfCurve.insert( dfCurve.begin(), 1. );
    threeMLTerms_DF.insert( threeMLTerms_DF.begin(), 0. );
    threeMLDF.insert( threeMLDF.begin(), 1. );
    sixMLTerms_DF.insert( sixMLTerms_DF.begin(), 0. );
    sixMLDF.insert( sixMLDF.begin(), 1. );

    if( isFRAUse )
    {
        insertFRA( swapMkt.spotDate, fra3MLMkt, fra6MLMkt, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF,
                   threeMLDF, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, threeMLSPos, sixMLSPos, pInter_DF );
    }

    baseDateAdjust( basedate, swapMkt.spotDate, monMkt, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF,
                    threeMLDF, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF );

	if (swapMkt.spotDate != libMkt.spotDate) 
	{
		liborDateAdjust( basedate, libMkt, swapMkt, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, 
						 sixMLRate, sixMLTerms_DF, sixMLDF, pInter_DF );
	}

	for (unsigned int i = 0; i < dfCurve.size(); ++i)
	{
		dfCurve[i] = AQLMath::max(dfCurve[i], MIN_DF);
	}
	for (unsigned int i = 0; i < threeMLDF.size(); ++i)
	{
		threeMLDF[i] = AQLMath::max(threeMLDF[i], MIN_DF);
	}
	for (unsigned int i = 0; i < sixMLDF.size(); ++i)
	{
		sixMLDF[i] = AQLMath::max(sixMLDF[i], MIN_DF);
	}
	if (swapMkt.optimizeMethod == NR)
	{
		calcCurve_NewtonRaphson(basedate, dfTerms, dfCurve, threeMLTerms_DF, threeMLDF, sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, 
			libMkt, swapMkt, xccyBasisMkt, libBasisMkt, swapMkt.dateGrid_3MRoll, swapMkt.termGrid_3MRoll, swapMkt.dateGrid_6MRoll, swapMkt.termGrid_6MRoll,
			swapMkt.dateGrid_12MRoll, swapMkt.termGrid_12MRoll, pInter_DF, false);
	}
	else if (swapMkt.optimizeMethod == NL2SOLALGO)
	{
		calcCurve_NL2SOL(basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, 
			sixMLRate, sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, pInter_DF, false);
	}
}

/*
    @brief function to calclation DiscountFactor from market data
    
    @param[in] basedate                 basedate
	@param[in] mktData                  input market data
    @param[out] dfTerms		            output terms
    @param[out] dfCurve		            output discount factor at terms
    @param[out] threeMLTerms_Rate       output terms as rate of 3m Libor
    @param[out] threeMLRate	            output forward 3m Libor at terms
    @param[out] threeMLTerms_Rate       output terms as DF of 3m Libor
    @param[out] threeMLDF	            output DF base forward 3m Libor at terms
    @param[out] sixMLTerms_Rate         output terms as rate of 6m Libor 
    @param[out] sixMLRate		        output forward 6m Libor at terms
    @param[out] sixMLTerms_DF           output terms as DF of 6m Libor 
    @param[out] sixMLDF 		        output DF base forward 6m Libor at terms]
	@param[out] threeMLSPos             output swap start pos of 3m Libor
	@param[out] sixMLSPos               output swap start pos of 6m Libor

    Suppose that
    data[0] money market
    data[1] libor market
    data[2] swap market
    data[3] currency basis market
    data[4] 3L6L basis market
    data[5] 3LFRA
    data[6] 6LFRA
*/
void 
AQLPriceArbFreeGenerator::
generateUSDCurve( const AQLDate& basedate,
				  const std::vector<AQLObject*>& mktData,
				  const AQLObject* fYieldData,
                  DoubleArray& dfTerms,
				  DoubleArray& dfCurve,
                  DoubleMatrix& threeMLTermsMtx_Rate,
                  DoubleArray& threeMLRate,
                  DoubleArray& threeMLTerms_DF,
                  DoubleArray& threeMLDF,
                  DoubleMatrix& sixMLTermsMtx_Rate,
                  DoubleArray& sixMLRate,
                  DoubleArray& sixMLTerms_DF,
                  DoubleArray& sixMLDF,
				  unsigned int& threeMLSPos,
				  unsigned int& sixMLSPos,
                  bool isFRAUse,
				  bool isFutureUse, 
				  bool isRenAdj,
				  AQLInterpolationBase* pInter_DF )
{
	if (isFRAUse && isFutureUse) 
	{
		throw AQLCoreInvalidData("We can not use fra and future at a same time!", __FILE__, __LINE__);
	}

    dfTerms.clear();
    dfCurve.clear();
    threeMLTermsMtx_Rate.clear();
	threeMLTermsMtx_Rate.resize(2);
    threeMLRate.clear();
    threeMLTerms_DF.clear();
    threeMLDF.clear();
    sixMLTermsMtx_Rate.clear();
	sixMLTermsMtx_Rate.resize(2);
    sixMLRate.clear();
    sixMLTerms_DF.clear();
    sixMLDF.clear();
	threeMLSPos = 0;
	sixMLSPos = 0;

    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    
	vector<const AQLObject*> data_mon, data_libor, data_fra6m, data_fra3m, data_swap, data_3m6m, data_xccy, data_future;
	unsigned int size_data = mktData.size();
	AQLString dataType, dataName;
	for(unsigned i = 0; i < size_data; i++)
	{
		// check use grid
		const AQLDataHolder *dh = &mktData[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

		dataType = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		dataType.toUpper();

		dataName = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL)).get()).get();
		dataName.toUpper();

		if (dataType == ZERO) data_libor.push_back(mktData[i]);//libor case
		else if (dataType == PAR) data_swap.push_back(mktData[i]);//swap case
		else if (dataType == FRA6M) data_fra6m.push_back(mktData[i]);//6mfra case
		else if (dataType == FRA3M) data_fra3m.push_back(mktData[i]);//3mfra case
		else if (dataType == FUTURE) data_future.push_back(mktData[i]);//future case
		else if (dataType == BASIS && dataName.findString(THREESIXBASIS) != -1) data_3m6m.push_back(mktData[i]);//3m6m basis swap
		else if (dataType == BASIS && dataName.findString(XCCYBASIS) != -1) data_xccy.push_back(mktData[i]);//xccy basis swap
		else if (dataType == O_N) data_mon.push_back(mktData[i]);//on case
		else if (dataType == T_N) data_mon.push_back(mktData[i]);//tn case
	}

	sort(data_libor.begin(), data_libor.end(), Comp_term());
	sort(data_swap.begin(), data_swap.end(), Comp_term());
	sort(data_3m6m.begin(), data_3m6m.end(), Comp_term());
	sort(data_xccy.begin(), data_xccy.end(), Comp_term());
	sort(data_fra3m.begin(), data_fra3m.end(), Comp_term());
	sort(data_fra6m.begin(), data_fra6m.end(), Comp_term());
	sort(data_future.begin(), data_future.end(), Comp_term());
	
    const MoneyMarket monMkt(data_mon);
    const LiborMarket libMkt(data_libor);
    const SwapMarket swapMkt(data_swap,data_3m6m,libMkt,basedate);
    const XCCYBasisMarket xccyBasisMkt(data_xccy,fYieldData,swapMkt,isRenAdj,true,basedate);
    const LiborBasisMarket libBasisMkt(data_3m6m,swapMkt,basedate);
	const FRAMarket fra3MLMkt(data_fra3m,isFRAUse);
	const FRAMarket fra6MLMkt(data_fra6m,isFRAUse);
	const FutureMarket futureMkt(data_future,swapMkt,isFutureUse,basedate);

	if (isFRAUse)
	{
		if (fra3MLMkt.map_term_rate.find("3M") == fra3MLMkt.map_term_rate.end()) throw AQLCoreInvalidData("3X6 FRA No Data!", __FILE__, __LINE__);
		if (fra3MLMkt.map_term_rate.find("6M") == fra3MLMkt.map_term_rate.end()) throw AQLCoreInvalidData("6X9 FRA No Data!", __FILE__, __LINE__);
	}

    unsigned int span;
    if (swapMkt.freq_Fix == ANNUAL) span = 12;
	else if (swapMkt.freq_Fix == SEMI_ANNUAL) span = 6;
	else
	{
		//error
		throw AQLCoreInvalidData("Input freq type is not supported.", __FILE__, __LINE__);
	}

	if( libMkt.dc == swapMkt.dc_Float ) ;
    else throw AQLCoreInvalidData("a daycount of Libor is different from a daycount of swap!", __FILE__, __LINE__);

    DoubleArray df_tmp,dfTerms_tmp;
    DateVector dfDates_tmp;
    generateSchedule(swapMkt.spotDate, swapMkt.spotDate, swapMkt.endDate, span, swapMkt.pCal, swapMkt.sld, dfDates_tmp, dfTerms_tmp, swapMkt.isEOMRoll);
    df_tmp.push_back(1.);

    //set term grid. use swap convention
    DoubleArray termGrid_3MRoll;
    DoubleArray termGrid_6MRoll;
	DoubleArray termGrid_12MRoll;
    DateVector dateGrid_3MRoll;
    DateVector dateGrid_6MRoll;
	DateVector dateGrid_12MRoll;
    generateSchedule(swapMkt.spotDate, swapMkt.spotDate, swapMkt.endDate, 3, swapMkt.pCal, swapMkt.sld, dateGrid_3MRoll, termGrid_3MRoll, swapMkt.isEOMRoll);  
    generateSchedule(swapMkt.spotDate, swapMkt.spotDate, swapMkt.endDate, 6, swapMkt.pCal, swapMkt.sld, dateGrid_6MRoll, termGrid_6MRoll, swapMkt.isEOMRoll);  
    generateSchedule(swapMkt.spotDate, swapMkt.spotDate, swapMkt.endDate, 12, swapMkt.pCal, swapMkt.sld, dateGrid_12MRoll, termGrid_12MRoll, swapMkt.isEOMRoll);  
    size_t dateSize_3M = termGrid_3MRoll.size();
    size_t dateSize_6M = termGrid_6MRoll.size();
	size_t dateSize_12M = termGrid_12MRoll.size();
    
    //Swap
    DoubleArray swapRateGrid;
	double spotTerm = dc_act365.getTerm(basedate, swapMkt.spotDate);
    for(size_t i=0; i<dfTerms_tmp.size()-1; i++)
    {
        swapRateGrid.push_back( swapMkt.pInter->value(dfTerms_tmp[i+1] + spotTerm) );
    }

    double ANN=0.,term_accru_Swap_Fix,floater,term_f;
	double rate,PV=0,term_accru_Curr,term_accru_Curr2,df_3ML;
	double spotAdjust = getSpotAdjust(basedate, swapMkt.spotDate, monMkt);
	double spotAdjust_baseccy;
	spotAdjust_baseccy = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, swapMkt.spotDate));
	map<AQLString, double >::const_iterator it;
	if(!isRenAdj)
	{
		for(size_t j=0; j<dfTerms_tmp.size()-1; j++)
		{
			term_f = dc_act365.getTerm(basedate, dfDates_tmp[j+1]);
			term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dfDates_tmp[j], dfDates_tmp[j+1], false);
			floater = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
			if (span == 12)
			{
				df_tmp.push_back( ( floater - ANN * swapRateGrid[j] ) / ( 1. + term_accru_Swap_Fix * swapRateGrid[j] ) );
			}
			else if (span == 6)
			{
				if( isFRAUse && j==0 )
				{
					double term_accru_Swap_Float_3ML = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[j], dateGrid_3MRoll[j+1], false);
					double term_accru_Swap_Float_6ML = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[j+1], dateGrid_3MRoll[j+2], false);
					double term_f_3ML = dc_act365.getTerm(basedate, termGrid_3MRoll[1]);
					double floater_3ML = xccyBasisMkt.pInter_fPrices->value(term_f_3ML) / spotAdjust_baseccy;
					df_3ML = floater_3ML / (1. + term_accru_Swap_Float_3ML * libMkt.threeMLibor);
					it = fra3MLMkt.map_term_rate.find("3M");
					df_tmp.push_back( (floater - df_3ML * term_accru_Swap_Float_3ML * libMkt.threeMLibor) / 
										(1. + term_accru_Swap_Float_6ML * it->second) );
				}
				else
				{
					df_tmp.push_back( ( floater - ANN * swapRateGrid[j] ) / ( 1. + term_accru_Swap_Fix * swapRateGrid[j] ) );
				}
			}
			ANN += term_accru_Swap_Fix * df_tmp[j+1];
		}

		if (isFRAUse && span == 6)
		{
			dfTerms_tmp.insert(dfTerms_tmp.begin()+1,termGrid_3MRoll[1]);
			df_tmp.insert(df_tmp.begin()+1,df_3ML);
		}
		pInter_DF->set(dfTerms_tmp,df_tmp);
		for(size_t i=0; i<dateSize_3M-1; i++)
		{
			dfTerms.push_back( termGrid_3MRoll[i+1] );
			dfCurve.push_back( pInter_DF->value( termGrid_3MRoll[i+1] ) );
		}   
	
		for(int i=0; i<dateSize_3M-1; i++)
		{
			term_accru_Curr = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[i], dateGrid_3MRoll[i+1], false);
			if( i == 0 )
			{
				insertRate( libMkt.threeMLibor, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
							termGrid_3MRoll[i], termGrid_3MRoll[i+1], dateGrid_3MRoll[i], dateGrid_3MRoll[i+1], libMkt.dc );
				term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[i+1]);
				floater = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
				dfCurve[i] = ( (floater - PV) / ( 1. + term_accru_Curr * threeMLRate[i] ) );
			}
			else if( isFRAUse && i<3 )
			{
				AQLString month = AQLString( 3 * i ) + AQLString("M");
				it = fra3MLMkt.map_term_rate.find(month);
				rate = it->second;

				insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
							termGrid_3MRoll[i], termGrid_3MRoll[i+1], dateGrid_3MRoll[i], dateGrid_3MRoll[i+1], libMkt.dc );
				term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[i+1]);
				floater = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
				dfCurve[i] = ( (floater - PV) / ( 1. + term_accru_Curr * threeMLRate[i] ) );
			}
			else
			{
				term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[i+1]);
				floater = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
				rate = (floater - dfCurve[i] - PV) / term_accru_Curr / dfCurve[i];

				insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
							termGrid_3MRoll[i], termGrid_3MRoll[i+1], dateGrid_3MRoll[i], dateGrid_3MRoll[i+1], libMkt.dc );
			}
			PV += threeMLRate[i] * term_accru_Curr * dfCurve[i];
		}        
	}
	else
	{
		if (swapMkt.freq_Fix == ANNUAL)
		{
			double A1,A2,A3,B1,B2,B3,C1,C2,C3,D1,D2,D3,E1,E2,E3,F1,F2,F3,e1,e2,e3,e4,f1,f2,f3,f4;
			double baseccyDF0,baseccyDF1,baseccyDF2,baseccyDF3,floater0,floater1,floater2,floater3,floater4;
			double term_accru_Curr3,term_accru_Curr4;
			for(unsigned int j=0; 2*j<dateSize_6M-1; j++)
			{
				term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[4*j], dateGrid_3MRoll[4*j+4], false);
				term_accru_Curr = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[4*j], dateGrid_3MRoll[4*j+1], false);
				term_accru_Curr2 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[4*j+1], dateGrid_3MRoll[4*j+2], false);
				term_accru_Curr3 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[4*j+2], dateGrid_3MRoll[4*j+3], false);
				term_accru_Curr4 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[4*j+3], dateGrid_3MRoll[4*j+4], false);
				floater0 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j])) / spotAdjust_baseccy;
				floater1 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j+1])) / spotAdjust_baseccy;
				floater2 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j+2])) / spotAdjust_baseccy;
				floater3 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j+3])) / spotAdjust_baseccy;
				floater4 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j+4])) / spotAdjust_baseccy;
				baseccyDF0 = xccyBasisMkt.pInter_baseccydf->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j])) / spotAdjust_baseccy;
				baseccyDF1 = xccyBasisMkt.pInter_baseccydf->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j+1])) / spotAdjust_baseccy;
				baseccyDF2 = xccyBasisMkt.pInter_baseccydf->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j+2])) / spotAdjust_baseccy;
				baseccyDF3 = xccyBasisMkt.pInter_baseccydf->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[4*j+3])) / spotAdjust_baseccy;
				if(j==0)
				{
					dfTerms.push_back( termGrid_3MRoll[4*j+1] );
					dfCurve.push_back( floater1 / (1. + term_accru_Curr * libMkt.threeMLibor) );
					insertRate( libMkt.threeMLibor, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[4*j], termGrid_3MRoll[4*j+1], dateGrid_3MRoll[4*j], dateGrid_3MRoll[4*j+1], libMkt.dc );
					PV += threeMLRate[4*j] * term_accru_Curr * dfCurve[4*j];

					if(isFRAUse)
					{
						it = fra3MLMkt.map_term_rate.find("3M");
						rate = it->second;
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[4*j+1], termGrid_3MRoll[4*j+2], dateGrid_3MRoll[4*j+1], dateGrid_3MRoll[4*j+2], libMkt.dc );
						dfTerms.push_back( termGrid_3MRoll[4*j+2] );
						dfCurve.push_back( (dfCurve[4*j] / baseccyDF1 * (floater2 - floater1) + dfCurve[4*j]) / 
										   (1. + term_accru_Curr2 * threeMLRate[4*j+1]) );
						PV += threeMLRate[4*j+1] * term_accru_Curr2 * dfCurve[4*j+1];

						it = fra3MLMkt.map_term_rate.find("6M");
						rate = it->second;
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[4*j+2], termGrid_3MRoll[4*j+3], dateGrid_3MRoll[4*j+2], dateGrid_3MRoll[4*j+3], libMkt.dc );
						dfTerms.push_back( termGrid_3MRoll[4*j+3] );
						dfCurve.push_back( (dfCurve[4*j+1] / baseccyDF2 * (floater3 - floater2) + dfCurve[4*j+1]) / 
										   (1. + term_accru_Curr3 * threeMLRate[4*j+2]));
						PV += threeMLRate[4*j+2] * term_accru_Curr3 * dfCurve[4*j+2];

						dfTerms.push_back( termGrid_3MRoll[4*j+4] );
						dfCurve.push_back( (dfCurve[4*j+2] / baseccyDF3 * (floater4 - floater3) + dfCurve[4*j+2] + PV) / 
										   (1. +  term_accru_Swap_Fix * swapRateGrid[j]) );
						rate = (swapRateGrid[j] * term_accru_Swap_Fix * dfCurve[4*j+3] - PV) / (term_accru_Curr4 * dfCurve[4*j+3]);
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[4*j+3], termGrid_3MRoll[4*j+4], dateGrid_3MRoll[4*j+3], dateGrid_3MRoll[4*j+4], libMkt.dc );
						PV += threeMLRate[4*j+3] * term_accru_Curr4 * dfCurve[4*j+3];
					}
					else
					{
						A2 = 1. / ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] );
						B2 = - ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] + termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+1] ) / 
							   ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] ) / ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] );
						C2 = ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] + termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j] ) / 
							 ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] ) / ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] );
						D2 = - 1. / ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] );
						E2 = - B2 / A2;
						F2 = - (C2 * dfCurve[4*j] + D2) / A2;

						A3 = 1. / ( termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+3] );
						B3 = - ( termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+3] + termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+2] ) / 
							   ( termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+3] ) / ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] );
						C3 = ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] + termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+1] ) / 
							 ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] ) / ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] );
						D3 = - 1. / ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] );
						E3 = - (B3 * E2 + C3) / A3;
						F3 = - (B3 * F2 + D3 * dfCurve[4*j]) / A3;

						e2 = - 1.;
						f2 = dfCurve[4*j] / baseccyDF1 *  (floater2 - floater1) + dfCurve[4*j];
						e3 = 1. / baseccyDF2 * (floater3 - floater2) + 1. - E2;
						f3 = - F2;
						e4 = E2 / baseccyDF3 * (floater4 - floater3) + E2 - E3;
						f4 = F2 / baseccyDF3 * (floater4 - floater3) + F2 - F3;

						dfTerms.push_back( termGrid_3MRoll[4*j+2] );
						dfCurve.push_back( (PV + f2+ f3 + f4 - swapRateGrid[j] * term_accru_Swap_Fix * F3) / 
										   (swapRateGrid[j] * term_accru_Swap_Fix * E3 - e2 - e3 - e4) );
						rate = (e2 * dfCurve[4*j+1] + f2) / (term_accru_Curr2 * dfCurve[4*j+1]);
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[4*j+1], termGrid_3MRoll[4*j+2], dateGrid_3MRoll[4*j+1], dateGrid_3MRoll[4*j+2], libMkt.dc );
						PV += threeMLRate[4*j+1] * term_accru_Curr2 * dfCurve[4*j+1];

						dfTerms.push_back( termGrid_3MRoll[4*j+3] );
						dfCurve.push_back(E2 * dfCurve[4*j+1] + F2);
						rate = (e3 * dfCurve[4*j+1] + f3) / (term_accru_Curr3 * dfCurve[4*j+2]);
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[4*j+2], termGrid_3MRoll[4*j+3], dateGrid_3MRoll[4*j+2], dateGrid_3MRoll[4*j+3], libMkt.dc );
						PV += threeMLRate[4*j+2] * term_accru_Curr3 * dfCurve[4*j+2];

						dfTerms.push_back( termGrid_3MRoll[4*j+4] );
						dfCurve.push_back(E3 * dfCurve[4*j+1] + F3);
						rate = (e4 * dfCurve[4*j+1] + f4) / (term_accru_Curr4 * dfCurve[4*j+3]);
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[4*j+3], termGrid_3MRoll[4*j+4], dateGrid_3MRoll[4*j+3], dateGrid_3MRoll[4*j+4], libMkt.dc );
						PV += threeMLRate[4*j+3] * term_accru_Curr4 * dfCurve[4*j+3];
					}
				}
				else
				{
					A1 = 1. / ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] );
					B1 = - ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] + termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j] ) / 
						   ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] ) / ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] );
					C1 = ( termGrid_3MRoll[4*j] - termGrid_3MRoll[4*j-1] + termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j-1] ) / 
						 ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] ) / ( termGrid_3MRoll[4*j] - termGrid_3MRoll[4*j-1] );
					D1 = - 1. / ( termGrid_3MRoll[4*j] - termGrid_3MRoll[4*j-1] );
					E1 = - B1 / A1;
					F1 = - (C1 * dfCurve[4*j-1] + D1 * dfCurve[4*j-2]) / A1;

					A2 = 1. / ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] );
					B2 = - ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] + termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+1] ) / 
						   ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] ) / ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] );
					C2 = ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] + termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j] ) / 
						 ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] ) / ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] );
					D2 = - 1. / ( termGrid_3MRoll[4*j+1] - termGrid_3MRoll[4*j] );
					E2 = - (B2 * E1 + C2) / A2;
					F2 = - (B2 * F1 + D2 * dfCurve[4*j-1]) / A2;

					A3 = 1. / ( termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+3] );
					B3 = - ( termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+3] + termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+2] ) / 
						   ( termGrid_3MRoll[4*j+4] - termGrid_3MRoll[4*j+3] ) / ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] );
					C3 = ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] + termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+1] ) / 
						 ( termGrid_3MRoll[4*j+3] - termGrid_3MRoll[4*j+2] ) / ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] );
					D3 = - 1. / ( termGrid_3MRoll[4*j+2] - termGrid_3MRoll[4*j+1] );
					E3 = - (B3 * E2 + C3 * E1 + D3) / A3;
					F3 = - (B3 * F2 + C3 * F1) / A3;

					e1 = - 1.;
					f1 = dfCurve[4*j-1] / baseccyDF0 * (floater1 - floater0) + dfCurve[4*j-1];
					e2 = 1. / baseccyDF1 * (floater2 - floater1) + 1. - E1;
					f2 = - F1;
					e3 = E1 / baseccyDF2 * (floater3 - floater2) + E1 - E2;
					f3 = F1 / baseccyDF2 * (floater3 - floater2) + F1 - F2;
					e4 = E2 / baseccyDF3 * (floater4 - floater3) + E2 - E3;
					f4 = F2 / baseccyDF3 * (floater4 - floater3) + F2 - F3;

					dfTerms.push_back( termGrid_3MRoll[4*j+1] );
					dfCurve.push_back( (PV + f1 + f2+ f3 + f4 - swapRateGrid[j] * (ANN + term_accru_Swap_Fix * F3)) / 
									   (swapRateGrid[j] * term_accru_Swap_Fix * E3 - e1 - e2 - e3 - e4) );
					rate = (e1 * dfCurve[4*j] + f1) / (term_accru_Curr * dfCurve[4*j]);
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[4*j], termGrid_3MRoll[4*j+1], dateGrid_3MRoll[4*j], dateGrid_3MRoll[4*j+1], libMkt.dc );
					PV += threeMLRate[4*j] * term_accru_Curr * dfCurve[4*j];

					dfTerms.push_back( termGrid_3MRoll[4*j+2] );
					dfCurve.push_back( E1 * dfCurve[4*j] + F1 );
					rate = (e2 * dfCurve[4*j] + f2) / (term_accru_Curr2 * dfCurve[4*j+1]);
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[4*j+1], termGrid_3MRoll[4*j+2], dateGrid_3MRoll[4*j+1], dateGrid_3MRoll[4*j+2], libMkt.dc );
					PV += threeMLRate[4*j+1] * term_accru_Curr2 * dfCurve[4*j+1];

					dfTerms.push_back( termGrid_3MRoll[4*j+3] );
					dfCurve.push_back(E2 * dfCurve[4*j] + F2);
					rate = (e3 * dfCurve[4*j] + f3) / (term_accru_Curr3 * dfCurve[4*j+2]);
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[4*j+2], termGrid_3MRoll[4*j+3], dateGrid_3MRoll[4*j+2], dateGrid_3MRoll[4*j+3], libMkt.dc );
					PV += threeMLRate[4*j+2] * term_accru_Curr3 * dfCurve[4*j+2];

					dfTerms.push_back( termGrid_3MRoll[4*j+4] );
					dfCurve.push_back(E3 * dfCurve[4*j] + F3);
					rate = (e4 * dfCurve[4*j] + f4) / (term_accru_Curr4 * dfCurve[4*j+3]);
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[4*j+3], termGrid_3MRoll[4*j+4], dateGrid_3MRoll[4*j+3], dateGrid_3MRoll[4*j+4], libMkt.dc );
					PV += threeMLRate[4*j+3] * term_accru_Curr4 * dfCurve[4*j+3];
				}
				ANN += term_accru_Swap_Fix * dfCurve[4*j+3];
			}	
		}
		else if (swapMkt.freq_Fix == SEMI_ANNUAL)
		{
			double baseccyDF0,baseccyDF1,baseccyDF2,floater0,floater1,floater2;
			double PV_Curr=0.;
			for(unsigned int j=0; j<dateSize_6M-1; j++)
			{
				term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*j], dateGrid_3MRoll[2*j+2], false);
				term_accru_Curr = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*j], dateGrid_3MRoll[2*j+1], false);
				term_accru_Curr2 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j+2], false);
				floater0 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[2*j])) / spotAdjust_baseccy;
				floater1 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[2*j+1])) / spotAdjust_baseccy;
				floater2 = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[2*j+2])) / spotAdjust_baseccy;
				baseccyDF0 = xccyBasisMkt.pInter_baseccydf->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[2*j])) / spotAdjust_baseccy;
				baseccyDF1 = xccyBasisMkt.pInter_baseccydf->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[2*j+1])) / spotAdjust_baseccy;
				baseccyDF2 = xccyBasisMkt.pInter_baseccydf->value(dc_act365.getTerm(basedate, dateGrid_3MRoll[2*j+2])) / spotAdjust_baseccy;
				if(j == 0)
				{
					dfTerms.push_back( termGrid_3MRoll[2*j+1] );
					dfCurve.push_back( floater1 / (1. + term_accru_Curr * libMkt.threeMLibor) );
					insertRate( libMkt.threeMLibor, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[2*j], termGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j], dateGrid_3MRoll[2*j+1], libMkt.dc );
					PV += threeMLRate[2*j] * term_accru_Curr * dfCurve[2*j];
					if(isFRAUse)
					{
						it = fra3MLMkt.map_term_rate.find("3M");
						rate = it->second;
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[2*j+1], termGrid_3MRoll[2*j+2], dateGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j+2], libMkt.dc );
						dfTerms.push_back( termGrid_3MRoll[2*j+2] );
						dfCurve.push_back( (dfCurve[2*j] / baseccyDF1 * (floater2 - floater1) + dfCurve[2*j]) / (1. + term_accru_Curr2 * threeMLRate[2*j+1]) );
						ANN += term_accru_Swap_Fix * dfCurve[2*j+1];
					}
					else
					{
						dfTerms.push_back( termGrid_3MRoll[2*j+2] );
						dfCurve.push_back( (dfCurve[2*j] / baseccyDF1 * (floater2 - floater1) + dfCurve[2*j] + PV) / 
										   (1. + term_accru_Swap_Fix * swapRateGrid[j]) );
						ANN += term_accru_Swap_Fix * dfCurve[2*j+1];
						rate = (swapRateGrid[j] * ANN - PV) / term_accru_Curr2 / dfCurve[2*j+1];
						insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
									termGrid_3MRoll[2*j+1], termGrid_3MRoll[2*j+2], dateGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j+2], libMkt.dc );
					}
					PV += threeMLRate[2*j+1] * term_accru_Curr2 * dfCurve[2*j+1];
					PV_Curr +=	  term_accru_Curr * threeMLRate[2*j] * dfCurve[2*j] 
								+ term_accru_Curr2 * threeMLRate[2*j+1] * dfCurve[2*j+1] * baseccyDF1 / dfCurve[2*j]
								+ (1. - baseccyDF1 / dfCurve[2*j]) * dfCurve[2*j]
								+ (baseccyDF1 / dfCurve[2*j] - baseccyDF2 / dfCurve[2*j+1]) * dfCurve[2*j+1];
				}
				else if(j==1 && isFRAUse)
				{
					it = fra3MLMkt.map_term_rate.find("6M");
					rate = it->second;
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[2*j], termGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j], dateGrid_3MRoll[2*j+1], libMkt.dc );
					dfTerms.push_back( termGrid_3MRoll[2*j+1] );
					dfCurve.push_back( dfCurve[2*j-1] / baseccyDF0 * (floater1 - PV_Curr) / (1. + term_accru_Curr * threeMLRate[2*j]) );
					PV += threeMLRate[2*j] * term_accru_Curr * dfCurve[2*j];
					PV_Curr +=	  term_accru_Curr * threeMLRate[2*j] * dfCurve[2*j] * baseccyDF0 / dfCurve[2*j-1]
								+ (baseccyDF0 / dfCurve[2*j-1] - baseccyDF1 / dfCurve[2*j]) * dfCurve[2*j];

					dfTerms.push_back( termGrid_3MRoll[2*j+2] );
					dfCurve.push_back( (dfCurve[2*j] / baseccyDF1 * (floater2 - PV_Curr) + PV - swapRateGrid[j] * ANN) / 
									   (1. + term_accru_Swap_Fix * swapRateGrid[j]) );
					ANN += term_accru_Swap_Fix * dfCurve[2*j+1];
					rate = (swapRateGrid[j] * ANN - PV) / term_accru_Curr2 / dfCurve[2*j+1];
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[2*j+1], termGrid_3MRoll[2*j+2], dateGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j+2], libMkt.dc );
					PV += threeMLRate[2*j+1] * term_accru_Curr2 * dfCurve[2*j+1];
					PV_Curr +=	  term_accru_Curr2 * threeMLRate[2*j+1] * dfCurve[2*j+1] * baseccyDF1 / dfCurve[2*j]
								+ (baseccyDF1 / dfCurve[2*j] - baseccyDF2 / dfCurve[2*j+1]) * dfCurve[2*j+1];
				}
				else
				{
					double A1,B1,C1,D1,E1,F1,e1,f1,e2,f2;
					A1 = 1 / ( termGrid_3MRoll[2*j+2] - termGrid_3MRoll[2*j+1] );
					B1 = - ( termGrid_3MRoll[2*j+2] - termGrid_3MRoll[2*j+1] + termGrid_3MRoll[2*j+2] - termGrid_3MRoll[2*j] ) / 
						   ( termGrid_3MRoll[2*j+2] - termGrid_3MRoll[2*j+1] ) / ( termGrid_3MRoll[2*j+1] - termGrid_3MRoll[2*j] );
					C1 = ( termGrid_3MRoll[2*j] - termGrid_3MRoll[2*j-1] + termGrid_3MRoll[2*j+1] - termGrid_3MRoll[2*j-1] ) / 
								( termGrid_3MRoll[2*j+1] - termGrid_3MRoll[2*j] ) / ( termGrid_3MRoll[2*j] - termGrid_3MRoll[2*j-1] );
					D1 = -1. / ( termGrid_3MRoll[2*j] - termGrid_3MRoll[2*j-1] );
					E1 = - B1 / A1;
					F1 = - (C1 * dfCurve[2*j-1] + D1 * dfCurve[2*j-2]) / A1;
					e1 = - 1.;
					f1 = dfCurve[2*j-1] / baseccyDF0 * (floater1 - PV_Curr);
					e2 = (floater2 - floater1) / baseccyDF1 + 1. - E1;
					f2 = - F1;

					dfTerms.push_back( termGrid_3MRoll[2*j+1] );
					dfCurve.push_back( (f1 + f2 + PV - swapRateGrid[j] * term_accru_Swap_Fix * F1 - swapRateGrid[j] * ANN) / 
									   (swapRateGrid[j] * term_accru_Swap_Fix * E1 - e1 - e2) );
					dfTerms.push_back( termGrid_3MRoll[2*j+2] );
					dfCurve.push_back( E1 * dfCurve[2*j] + F1);

					rate = (e1 * dfCurve[2*j] + f1) / term_accru_Curr / dfCurve[2*j];
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[2*j], termGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j], dateGrid_3MRoll[2*j+1], libMkt.dc );
					rate = (e2 * dfCurve[2*j] + f2) / term_accru_Curr2 / dfCurve[2*j+1];
					insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
								termGrid_3MRoll[2*j+1], termGrid_3MRoll[2*j+2], dateGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j+2], libMkt.dc );
					PV_Curr +=	  term_accru_Curr * threeMLRate[2*j] * dfCurve[2*j] * baseccyDF0 / dfCurve[2*j-1]
								+ term_accru_Curr2 * threeMLRate[2*j+1] * dfCurve[2*j+1] * baseccyDF1 / dfCurve[2*j]
								+ (baseccyDF0 / dfCurve[2*j-1] - baseccyDF1 / dfCurve[2*j]) * dfCurve[2*j]
								+ (baseccyDF1 / dfCurve[2*j] - baseccyDF2 / dfCurve[2*j+1]) * dfCurve[2*j+1];
					PV += threeMLRate[2*j] * term_accru_Curr * dfCurve[2*j];
					PV += threeMLRate[2*j+1] * term_accru_Curr2 * dfCurve[2*j+1];
					ANN += term_accru_Swap_Fix * dfCurve[2*j+1];
				}
			}
		}
	}

    dfTerms.insert( dfTerms.begin(), 0. );
    dfCurve.insert( dfCurve.begin(), 1. );
    threeMLTerms_DF.insert( threeMLTerms_DF.begin(), 0. );
    threeMLDF.insert( threeMLDF.begin(), 1. );
   
    //interpolate market data
    //3L6L basis
    DoubleArray threeSixBasisGrid;
    for(size_t j = 1; j<dateSize_6M; j++)
    {
        threeSixBasisGrid.push_back( libBasisMkt.pInter->value(termGrid_6MRoll[j]) );
    }

    //calculate discount factor, forward 3M Libor and forward 6M Libor
    double term_accru_3L6L_3L1,term_accru_3L6L_3L2,term_accru_3L6L_6L;
    double ANN_3L6L_3L=0.,PV_3L6L_3L=0.,PV_3L6L_6L=0.;

    for(size_t j=0; j<dateSize_6M-1; j++)
    {
        term_accru_3L6L_3L1 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*j], dateGrid_3MRoll[2*j+1], false);
        term_accru_3L6L_3L2 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*j+1], dateGrid_3MRoll[2*j+2], false);
        term_accru_3L6L_6L = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[j], dateGrid_6MRoll[j+1], false);
        ANN_3L6L_3L += term_accru_3L6L_3L1 * dfCurve[2*j+1] + term_accru_3L6L_3L2 * dfCurve[2*j+2];
        PV_3L6L_3L += term_accru_3L6L_3L1 * threeMLRate[2*j] * dfCurve[2*j+1] + 
					  term_accru_3L6L_3L2 * threeMLRate[2*j+1] * dfCurve[2*j+2];
        if(j==0)
        {
            insertRate( libMkt.sixMLibor, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                        termGrid_6MRoll[j], termGrid_6MRoll[j+1], dateGrid_6MRoll[j], dateGrid_6MRoll[j+1], libMkt.dc );
        }
        else
        {                
            rate = (PV_3L6L_3L + ANN_3L6L_3L * threeSixBasisGrid[j] - PV_3L6L_6L) / (term_accru_3L6L_6L * dfCurve[2*j+2]);
            insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                        termGrid_6MRoll[j], termGrid_6MRoll[j+1], dateGrid_6MRoll[j], dateGrid_6MRoll[j+1], libMkt.dc );
        }
        
        PV_3L6L_6L += term_accru_3L6L_6L * sixMLRate[j] * dfCurve[2*j+2];
    }
    sixMLTerms_DF.insert( sixMLTerms_DF.begin(), 0. );
    sixMLDF.insert( sixMLDF.begin(), 1. );

	if( isFutureUse )
    {
        insertFuture( swapMkt.spotDate, futureMkt, swapMkt, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, threeMLSPos, pInter_DF );
    }

    if( isFRAUse )
    {
        insertFRA( swapMkt.spotDate, fra3MLMkt, fra6MLMkt, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF,
                   threeMLDF, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, threeMLSPos, sixMLSPos, pInter_DF );
    }

    baseDateAdjust( basedate, swapMkt.spotDate, monMkt, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF,
                    threeMLDF, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF );

	for (unsigned int i = 0; i < dfCurve.size(); ++i)
	{
		dfCurve[i] = AQLMath::max(dfCurve[i], MIN_DF);
	}
	for (unsigned int i = 0; i < threeMLDF.size(); ++i)
	{
		threeMLDF[i] = AQLMath::max(threeMLDF[i], MIN_DF);
	}
	for (unsigned int i = 0; i < sixMLDF.size(); ++i)
	{
		sixMLDF[i] = AQLMath::max(sixMLDF[i], MIN_DF);
	}    
	if (swapMkt.optimizeMethod == NR)
	{
		calcCurve_NewtonRaphson( basedate, dfTerms, dfCurve, threeMLTerms_DF, threeMLDF, sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, 
			libMkt, swapMkt, xccyBasisMkt, libBasisMkt, swapMkt.dateGrid_3MRoll, swapMkt.termGrid_3MRoll, swapMkt.dateGrid_6MRoll, 
			swapMkt.termGrid_6MRoll, swapMkt.dateGrid_12MRoll, swapMkt.termGrid_12MRoll, pInter_DF, true );
	}
	else if (swapMkt.optimizeMethod == NL2SOLALGO)
	{
		calcCurve_NL2SOL( basedate, dfTerms, dfCurve, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, sixMLTermsMtx_Rate, 
			sixMLRate, sixMLTerms_DF, sixMLDF, isFRAUse, isRenAdj, libMkt, swapMkt, xccyBasisMkt, libBasisMkt, pInter_DF, true );
	}        
}

/*
    @brief function to calculate iscount factor, forward 3M Libor and forward 6M Libor
		when a frequency of swap floating leg is Semi-annual and a frequency of swap fixed leg is Semi-annual
*/
void 
AQLPriceArbFreeGenerator::calcCurve_Semi_Semi
(	const AQLDate& basedate, 
	DoubleArray& dfTerms,
	DoubleArray& dfCurve,
	DoubleMatrix& threeMLTermsMtx_Rate,
	DoubleArray& threeMLRate,
	DoubleArray& threeMLTerms_DF,
	DoubleArray& threeMLDF,
	DoubleMatrix& sixMLTermsMtx_Rate,
	DoubleArray& sixMLRate,
	DoubleArray& sixMLTerms_DF,
	DoubleArray& sixMLDF,
	bool isFRAUse,
	bool isRenAdj, 
	const LiborMarket& libMkt,
	const SwapMarket& swapMkt,
	const XCCYBasisMarket& xccyBasisMkt,
	const LiborBasisMarket& libBasisMkt,
	const FRAMarket& fra3MLMkt,
	const FRAMarket& fra6MLMkt,
	const DoubleArray& swapRateGrid,
	const DoubleArray& currBasisGrid,
	const DoubleArray& threeSixBasisGrid,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	double& PV_Swap,
	double& PV_3L6L_6L,
	double& PV_3L6L_3L,
	double& PV_Curr,
	double& ANN_Swap_6M,
	double& ANN_Swap_3M,
	double& ANN_3L6L_6L,
	double& ANN_3L6L_3L,
	double& ANN_Curr,
	double& I,
	double spotAdjust,
	double spotTerm,
	unsigned int pos )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	double spotAdjust_dol,spotAdjust_baseccy,term_f,H1, H2;
	if(isRenAdj)  spotAdjust_dol = xccyBasisMkt.pInter_usd->value(spotTerm);
	else  spotAdjust_baseccy = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, swapMkt.spotDate));

	map<AQLString, double >::const_iterator it;

	double rate;
	double term_accru_Curr1 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_Curr2 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_3L1 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_3L6L_3L2 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_6L = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Float = swapMkt.dc_Float.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix_3L1 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Float_3L1 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Fix_3L2 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
    double term_accru_Swap_Float_3L2 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);

	if(!isRenAdj)
	{
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+1]);
		H1 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+2]);
		H2 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
	}
	else
	{
		if(pos==0)
		{
			I += xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				 xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 1.;
		}
		else
		{
			I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol) *
				  dfCurve[2*pos-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol);
		}
	}

	if(pos==0)
	{
		if(isFRAUse)
		{
			it = fra3MLMkt.map_term_rate.find("3M");

			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[1] );
			if(!isRenAdj) dfCurve.push_back( H1 / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	
			else dfCurve.push_back( I / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

			dfTerms.push_back( termGrid_3MRoll[2] );
			if(!isRenAdj) 
			{
				dfCurve.push_back( (H2 - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
			else 
			{
				dfCurve.push_back( (I - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
		}
		else
		{
			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			if(!isRenAdj) dfCurve.push_back( H1 / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );
			else dfCurve.push_back( I / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

			double tmp_a,tmp_ad,tmp_b,tmp_bd,tmp_c,tmp_cd;
			tmp_a = term_accru_Curr2 * currBasisGrid[1] + 1.;
			tmp_b = term_accru_Curr2;
			if(!isRenAdj) tmp_c = H2 - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];
			else tmp_c = I - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];

			tmp_ad = term_accru_3L6L_6L * libMkt.sixMLibor - term_accru_3L6L_3L2 * threeSixBasisGrid[0];
			tmp_bd = - term_accru_3L6L_3L2;
			tmp_cd = term_accru_3L6L_3L1 * ( libMkt.threeMLibor + threeSixBasisGrid[0] ) * dfCurve[0];

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_bd*tmp_c - tmp_b*tmp_cd) / (tmp_a*tmp_bd - tmp_ad*tmp_b) ); 

			rate = (tmp_cd*tmp_a - tmp_c*tmp_ad) / (tmp_bd*tmp_c - tmp_b*tmp_cd);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );
		}
	}
	else
	{
		if( isFRAUse && pos == 1 )
		{
			it = fra3MLMkt.map_term_rate.find("6M");

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			double tmp_a_Swap,tmp_c_Swap,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,tmp_c_3L6L,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_6L * term_accru_Swap_Fix / term_accru_Swap_Float * swapRateGrid[pos];
			tmp_c_Swap = term_accru_3L6L_6L / term_accru_Swap_Float * ( swapRateGrid[pos] * ANN_Swap_6M - PV_Swap );
			tmp_a_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * term_accru_Curr1 * ( currBasisGrid[2*pos+1] + threeMLRate[2*pos] );
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( H2 - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr );
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * (- tmp + term_accru_Curr1 * (currBasisGrid[2*pos+1] + threeMLRate[2*pos]));
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( I - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr );
			}

			tmp_c_3L6L = PV_3L6L_6L - PV_3L6L_3L - threeSixBasisGrid[pos] * ANN_3L6L_3L;
			tmp_A = tmp_a_Swap - tmp_a_Curr2 - threeSixBasisGrid[pos] * term_accru_3L6L_3L2;
			tmp_B = - ( threeMLRate[2*pos] + threeSixBasisGrid[pos] ) * term_accru_3L6L_3L1 - tmp_b_Curr2;
			tmp_C = tmp_c_Swap + tmp_c_3L6L - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
			tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
					( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
			tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
					( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
					dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );
			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );

			rate = (tmp_c_Swap + tmp_a_Swap * dfCurve[2*pos+1]) / (term_accru_3L6L_6L * dfCurve[2*pos+1]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[2*pos] + tmp_a_Curr2 * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
		}
		else
		{
			double tmp_a_Swap,tmp_c_Swap,tmp_b_Curr1,tmp_c_Curr1,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,
				tmp_c_3L6L,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_6L * term_accru_Swap_Fix / term_accru_Swap_Float * swapRateGrid[pos];
			tmp_c_Swap = term_accru_3L6L_6L / term_accru_Swap_Float * ( swapRateGrid[pos] * ANN_Swap_6M - PV_Swap );
			tmp_b_Curr1 = - term_accru_3L6L_3L1 / term_accru_Curr1 * ( 1. + term_accru_Curr1 * currBasisGrid[2*pos] );
            
			if(!isRenAdj)
			{
				tmp_c_Curr1 = term_accru_3L6L_3L1 / term_accru_Curr1 * ( H1 - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}
			else
			{
				tmp_c_Curr1 = term_accru_3L6L_3L1 / term_accru_Curr1 * ( I - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}

			tmp_a_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ((H2-H1) - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( tmp - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}

			tmp_c_3L6L = PV_3L6L_6L - PV_3L6L_3L - threeSixBasisGrid[pos] * ANN_3L6L_3L;
			tmp_A = tmp_a_Swap - tmp_a_Curr2 - threeSixBasisGrid[pos] * term_accru_3L6L_3L2;
			tmp_B = - ( tmp_b_Curr1 + tmp_b_Curr2 + threeSixBasisGrid[pos] * term_accru_3L6L_3L1 );
			tmp_C = tmp_c_Swap + tmp_c_3L6L - tmp_c_Curr1 - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
			tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
					( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
			tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
					( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
					dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
   
			rate = (tmp_c_Swap + tmp_a_Swap * dfCurve[2*pos+1]) / (term_accru_3L6L_6L * dfCurve[2*pos+1]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			rate = (tmp_c_Curr1 + tmp_b_Curr1 * dfCurve[2*pos] ) / (term_accru_3L6L_3L1 * dfCurve[2*pos]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[2*pos] + tmp_a_Curr2 * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );			
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
		}
	}

	ANN_Swap_6M += term_accru_Swap_Fix * dfCurve[2*pos+1];
	ANN_Swap_3M += term_accru_Swap_Fix_3L1 * dfCurve[2*pos] + term_accru_Swap_Fix_3L2 * dfCurve[2*pos+1];
	ANN_Curr += term_accru_Curr1 * dfCurve[2*pos] + term_accru_Curr2 * dfCurve[2*pos+1];
	ANN_3L6L_3L += term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
	ANN_3L6L_6L += term_accru_3L6L_6L * dfCurve[2*pos+1];
	PV_Swap += term_accru_Swap_Float * sixMLRate[pos] * dfCurve[2*pos+1];
	PV_Curr += term_accru_Curr1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
				term_accru_Curr2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_3L += term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
					term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_6L += term_accru_3L6L_6L * sixMLRate[pos] * dfCurve[2*pos+1];
}

/*
    @brief function to calculate iscount factor, forward 3M Libor and forward 6M Libor
		when a frequency of swap floating leg is Quarterly and a frequency of swap fixed leg is Quarterly
*/
void 
AQLPriceArbFreeGenerator::calcCurve_Quar_Quar
(	const AQLDate& basedate, 
	DoubleArray& dfTerms,
	DoubleArray& dfCurve,
	DoubleMatrix& threeMLTermsMtx_Rate,
	DoubleArray& threeMLRate,
	DoubleArray& threeMLTerms_DF,
	DoubleArray& threeMLDF,
	DoubleMatrix& sixMLTermsMtx_Rate,
	DoubleArray& sixMLRate,
	DoubleArray& sixMLTerms_DF,
	DoubleArray& sixMLDF,
	bool isFRAUse,
	bool isRenAdj, 
	const LiborMarket& libMkt,
	const SwapMarket& swapMkt,
	const XCCYBasisMarket& xccyBasisMkt,
	const LiborBasisMarket& libBasisMkt,
	const FRAMarket& fra3MLMkt,
	const FRAMarket& fra6MLMkt,
	const DoubleArray& swapRateGrid,
	const DoubleArray& currBasisGrid,
	const DoubleArray& threeSixBasisGrid,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	double& PV_Swap,
	double& PV_3L6L_6L,
	double& PV_3L6L_3L,
	double& PV_Curr,
	double& ANN_Swap_6M,
	double& ANN_Swap_3M,
	double& ANN_3L6L_6L,
	double& ANN_3L6L_3L,
	double& ANN_Curr,
	double& I,
	double spotAdjust,
	double spotTerm,
	unsigned int pos )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	double spotAdjust_dol,spotAdjust_baseccy,term_f,H1, H2;
	if(isRenAdj)  spotAdjust_dol = xccyBasisMkt.pInter_usd->value(spotTerm);
	else  spotAdjust_baseccy = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, swapMkt.spotDate));

	map<AQLString, double >::const_iterator it;

	double rate;
	double term_accru_Curr1 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_Curr2 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_3L1 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_3L6L_3L2 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_6L = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Float = swapMkt.dc_Float.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix_3L1 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Float_3L1 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Fix_3L2 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
    double term_accru_Swap_Float_3L2 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);

	if(!isRenAdj)
	{
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+1]);
		H1 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+2]);
		H2 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
	}
	else
	{
		if(pos==0)
		{
			I += xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				 xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 1.;
		}
		else
		{
			I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol) *
				  dfCurve[2*pos-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol);
		}
	}

	if(pos==0)
    {
        if( isFRAUse )
        {
            it = fra3MLMkt.map_term_rate.find("3M");
            rate = libMkt.threeMLibor;
            insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
                        termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

            rate = it->second;
            insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
                        termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );

            rate = libMkt.sixMLibor;
            insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                        termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

            dfTerms.push_back( termGrid_3MRoll[1] );
			if(!isRenAdj) dfCurve.push_back( H1 / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	
			else dfCurve.push_back( I / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

            dfTerms.push_back( termGrid_3MRoll[2] );
			if(!isRenAdj) 
			{
				dfCurve.push_back( (H2 - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
			else 
			{
				dfCurve.push_back( (I - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
        }
        else
        {
            rate = libMkt.threeMLibor;
            insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
                        termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

            dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
            if(!isRenAdj) dfCurve.push_back( H1 / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );
			else dfCurve.push_back( I / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

            double tmp_a,tmp_ad,tmp_b,tmp_bd,tmp_c,tmp_cd;
            tmp_a = term_accru_Curr2 * currBasisGrid[1] + 1.;
            tmp_b = term_accru_Curr2;
			if(!isRenAdj) tmp_c = H2 - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];
			else tmp_c = I - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];

            tmp_ad = term_accru_3L6L_6L * (libMkt.sixMLibor + threeSixBasisGrid[0]);
            tmp_bd = - term_accru_3L6L_3L2;
            tmp_cd = term_accru_3L6L_3L1 * libMkt.threeMLibor * dfCurve[0];               

            rate = libMkt.sixMLibor;
            insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                        termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

            dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
            dfCurve.push_back( (tmp_bd*tmp_c - tmp_b*tmp_cd) / (tmp_a*tmp_bd - tmp_ad*tmp_b) ); 

            rate = (tmp_cd*tmp_a - tmp_c*tmp_ad) / (tmp_bd*tmp_c - tmp_b*tmp_cd);
            insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
                        termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );
        }
    }
    else
    {
        if( isFRAUse && pos == 1 )
        {
            it = fra3MLMkt.map_term_rate.find("6M");
            rate = it->second;
            insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
                        termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

            double tmp_a_Swap,tmp_b_Swap,tmp_c_Swap,tmp_a_Curr,tmp_b_Curr,tmp_c_Curr,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;

            tmp_a_Swap = term_accru_3L6L_3L2 / term_accru_Swap_Float_3L2 * swapRateGrid[pos] * term_accru_Swap_Fix_3L2;
            tmp_b_Swap = term_accru_3L6L_3L2 / term_accru_Swap_Float_3L2 * 
                            (swapRateGrid[pos] * term_accru_Swap_Fix_3L1 - threeMLRate[2*pos] * term_accru_Swap_Float_3L1);
            tmp_c_Swap = term_accru_3L6L_3L2 / term_accru_Swap_Float_3L2 * 
                            (swapRateGrid[pos] * ANN_Swap_3M - PV_Swap);

            tmp_a_Curr = - term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if (!isRenAdj)
			{
				tmp_b_Curr = - term_accru_3L6L_3L2 / term_accru_Curr2 * term_accru_Curr1 * ( currBasisGrid[2*pos+1] + threeMLRate[2*pos] );
				tmp_c_Curr = term_accru_3L6L_3L2 / term_accru_Curr2 * ( H2 - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr );
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr = - term_accru_3L6L_3L2 / term_accru_Curr2 * (- tmp + term_accru_Curr1 * (currBasisGrid[2*pos+1] + threeMLRate[2*pos]));
				tmp_c_Curr = term_accru_3L6L_3L2 / term_accru_Curr2 * (I - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr);
			}

            tmp_A = tmp_a_Curr - tmp_a_Swap;
            tmp_B = tmp_b_Curr - tmp_b_Swap;
            tmp_C = tmp_c_Curr - tmp_c_Swap;

            tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
            tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
                    ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
            tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
                    ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
                    dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

            dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
            dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
            dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
            dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );

            rate = (tmp_c_Curr + tmp_b_Curr * dfCurve[2*pos] + tmp_a_Curr * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
            insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
                        termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );

            double tmp_PV3L = PV_3L6L_3L + term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
                                        term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
            double tmp_ANN3L = ANN_3L6L_3L + term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
            double tmp_ANN6L = ANN_3L6L_6L + term_accru_3L6L_6L * dfCurve[2*pos+1];

            rate = -(PV_3L6L_6L - tmp_ANN3L * threeSixBasisGrid[pos] - tmp_PV3L)/(term_accru_3L6L_6L * dfCurve[2*pos+1]);
            insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                        termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
        }
        else
        {
            double tmp_a_Swap,tmp_b_Swap,tmp_c_Swap,tmp_b_Curr1,tmp_c_Curr1,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,
				tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_Swap_Fix_3L2 * swapRateGrid[pos];
			tmp_b_Swap = term_accru_Swap_Fix_3L1 * swapRateGrid[pos];
			tmp_c_Swap = swapRateGrid[pos] * ANN_Swap_3M - PV_Swap;
			tmp_b_Curr1 = - term_accru_Swap_Float_3L1 / term_accru_Curr1 * ( 1. + term_accru_Curr1 * currBasisGrid[2*pos] );
            
			if(!isRenAdj)
			{
				tmp_c_Curr1 = term_accru_Swap_Float_3L1 / term_accru_Curr1 * ( H1 - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}
			else
			{
				tmp_c_Curr1 = term_accru_Swap_Float_3L1 / term_accru_Curr1 * ( I - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}

			tmp_a_Curr2 = - term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( 1. - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ((H2-H1) - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( tmp - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}

			tmp_A = tmp_a_Swap - tmp_a_Curr2;
			tmp_B = - (tmp_b_Curr1 + tmp_b_Curr2 - tmp_b_Swap);
			tmp_C = tmp_c_Swap - tmp_c_Curr1 - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
			tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
					( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
			tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
					( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
					dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );

			rate = (tmp_c_Curr1 + tmp_b_Curr1 * dfCurve[2*pos] ) / (term_accru_3L6L_3L1 * dfCurve[2*pos]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[2*pos] + tmp_a_Curr2 * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );			
			
			double tmp_PV3L = PV_3L6L_3L + term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
                                           term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
            double tmp_ANN3L = ANN_3L6L_3L + term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
            double tmp_ANN6L = ANN_3L6L_6L + term_accru_3L6L_6L * dfCurve[2*pos+1];
			rate = -(PV_3L6L_6L - tmp_ANN3L * threeSixBasisGrid[pos] - tmp_PV3L)/(term_accru_3L6L_6L * dfCurve[2*pos+1]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
        }
	}
	
	ANN_Swap_6M += term_accru_Swap_Fix * dfCurve[2*pos+1];
	ANN_Swap_3M += term_accru_Swap_Fix_3L1 * dfCurve[2*pos] + term_accru_Swap_Fix_3L2 * dfCurve[2*pos+1];
	ANN_Curr += term_accru_Curr1 * dfCurve[2*pos] + term_accru_Curr2 * dfCurve[2*pos+1];
	ANN_3L6L_3L += term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
	ANN_3L6L_6L += term_accru_3L6L_6L * dfCurve[2*pos+1];
    PV_Swap += threeMLRate[2*pos] * term_accru_Swap_Float_3L1 * dfCurve[2*pos] + threeMLRate[2*pos+1] * term_accru_Swap_Float_3L2 * dfCurve[2*pos+1];
	PV_Curr += term_accru_Curr1 * threeMLRate[2*pos] * dfCurve[2*pos] + term_accru_Curr2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_3L += term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_6L += term_accru_3L6L_6L * sixMLRate[pos] * dfCurve[2*pos+1];
}

/*
    @brief function to calculate iscount factor, forward 3M Libor and forward 6M Libor
		when a frequency of swap floating leg is Semi-Annual and a frequency of swap fixed leg is Quarterly
*/
void 
AQLPriceArbFreeGenerator::calcCurve_Quar_Semi
(	const AQLDate& basedate, 
	DoubleArray& dfTerms,
	DoubleArray& dfCurve,
	DoubleMatrix& threeMLTermsMtx_Rate,
	DoubleArray& threeMLRate,
	DoubleArray& threeMLTerms_DF,
	DoubleArray& threeMLDF,
	DoubleMatrix& sixMLTermsMtx_Rate,
	DoubleArray& sixMLRate,
	DoubleArray& sixMLTerms_DF,
	DoubleArray& sixMLDF,
	bool isFRAUse,
	bool isRenAdj, 
	const LiborMarket& libMkt,
	const SwapMarket& swapMkt,
	const XCCYBasisMarket& xccyBasisMkt,
	const LiborBasisMarket& libBasisMkt,
	const FRAMarket& fra3MLMkt,
	const FRAMarket& fra6MLMkt,
	const DoubleArray& swapRateGrid,
	const DoubleArray& currBasisGrid,
	const DoubleArray& threeSixBasisGrid,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	double& PV_Swap,
	double& PV_3L6L_6L,
	double& PV_3L6L_3L,
	double& PV_Curr,
	double& ANN_Swap_6M,
	double& ANN_Swap_3M,
	double& ANN_3L6L_6L,
	double& ANN_3L6L_3L,
	double& ANN_Curr,
	double& I,
	double spotAdjust,
	double spotTerm,
	unsigned int pos )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	double spotAdjust_dol,spotAdjust_baseccy,term_f,H1, H2;
	if(isRenAdj)  spotAdjust_dol = xccyBasisMkt.pInter_usd->value(spotTerm);
	else  spotAdjust_baseccy = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, swapMkt.spotDate));

	map<AQLString, double >::const_iterator it;

	double rate;
	double term_accru_Curr1 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_Curr2 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_3L1 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_3L6L_3L2 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_6L = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Float = swapMkt.dc_Float.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix_3L1 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Float_3L1 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Fix_3L2 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
    double term_accru_Swap_Float_3L2 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);

	if(!isRenAdj)
	{
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+1]);
		H1 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+2]);
		H2 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
	}
	else
	{
		if(pos==0)
		{
			I += xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				 xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 1.;
		}
		else
		{
			I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol) *
				  dfCurve[2*pos-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol);
		}
	}

	if(pos==0)
	{
		if(isFRAUse)
		{
			it = fra3MLMkt.map_term_rate.find("3M");

			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[1] );
			if(!isRenAdj) dfCurve.push_back( H1 / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	
			else dfCurve.push_back( I / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

			dfTerms.push_back( termGrid_3MRoll[2] );
			if(!isRenAdj) 
			{
				dfCurve.push_back( (H2 - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
			else 
			{
				dfCurve.push_back( (I - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
		}
		else
		{
			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			if(!isRenAdj) dfCurve.push_back( H1 / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );
			else dfCurve.push_back( I / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

			double tmp_a,tmp_ad,tmp_b,tmp_bd,tmp_c,tmp_cd;
			tmp_a = term_accru_Curr2 * currBasisGrid[1] + 1.;
			tmp_b = term_accru_Curr2;
			if(!isRenAdj) tmp_c = H2 - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];
			else tmp_c = I - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];

			tmp_ad = term_accru_3L6L_6L * libMkt.sixMLibor - term_accru_3L6L_3L2 * threeSixBasisGrid[0];
			tmp_bd = - term_accru_3L6L_3L2;
			tmp_cd = term_accru_3L6L_3L1 * ( libMkt.threeMLibor + threeSixBasisGrid[0] ) * dfCurve[0];

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_bd*tmp_c - tmp_b*tmp_cd) / (tmp_a*tmp_bd - tmp_ad*tmp_b) ); 

			rate = (tmp_cd*tmp_a - tmp_c*tmp_ad) / (tmp_bd*tmp_c - tmp_b*tmp_cd);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );
		}
	}
	else
	{
		if( isFRAUse && pos == 1 )
		{
			it = fra3MLMkt.map_term_rate.find("6M");

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			double tmp_a_Swap,tmp_b_Swap,tmp_c_Swap,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,tmp_c_3L6L,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_6L * term_accru_Swap_Fix_3L2 / term_accru_Swap_Float * swapRateGrid[pos];
			tmp_b_Swap = term_accru_3L6L_6L * term_accru_Swap_Fix_3L1 / term_accru_Swap_Float * swapRateGrid[pos];
			tmp_c_Swap = term_accru_3L6L_6L / term_accru_Swap_Float * ( swapRateGrid[pos] * ANN_Swap_3M - PV_Swap );
			tmp_a_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * term_accru_Curr1 * ( currBasisGrid[2*pos+1] + threeMLRate[2*pos] );
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( H2 - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr );
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * (- tmp + term_accru_Curr1 * (currBasisGrid[2*pos+1] + threeMLRate[2*pos]));
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( I - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr );
			}

			tmp_c_3L6L = PV_3L6L_6L - PV_3L6L_3L - threeSixBasisGrid[pos] * ANN_3L6L_3L;
			tmp_A = tmp_a_Swap - tmp_a_Curr2 - threeSixBasisGrid[pos] * term_accru_3L6L_3L2;
			tmp_B = - ( threeMLRate[2*pos] + threeSixBasisGrid[pos] ) * term_accru_3L6L_3L1 - tmp_b_Curr2 + tmp_b_Swap;
			tmp_C = tmp_c_Swap + tmp_c_3L6L - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
			tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
					( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
			tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
					( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
					dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );
			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );

			rate = (tmp_c_Swap + tmp_b_Swap * dfCurve[2*pos] + tmp_a_Swap * dfCurve[2*pos+1]) / (term_accru_3L6L_6L * dfCurve[2*pos+1]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[2*pos] + tmp_a_Curr2 * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
		}
		else
		{
			double tmp_a_Swap,tmp_b_Swap,tmp_c_Swap,tmp_b_Curr1,tmp_c_Curr1,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,
				tmp_c_3L6L,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_6L * term_accru_Swap_Fix_3L2 / term_accru_Swap_Float * swapRateGrid[pos];
			tmp_b_Swap = term_accru_3L6L_6L * term_accru_Swap_Fix_3L1 / term_accru_Swap_Float * swapRateGrid[pos];
			tmp_c_Swap = term_accru_3L6L_6L / term_accru_Swap_Float * ( swapRateGrid[pos] * ANN_Swap_3M - PV_Swap );
			tmp_b_Curr1 = - term_accru_3L6L_3L1 / term_accru_Curr1 * ( 1. + term_accru_Curr1 * currBasisGrid[2*pos] );
            
			if(!isRenAdj)
			{
				tmp_c_Curr1 = term_accru_3L6L_3L1 / term_accru_Curr1 * ( H1 - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}
			else
			{
				tmp_c_Curr1 = term_accru_3L6L_3L1 / term_accru_Curr1 * ( I - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}

			tmp_a_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ((H2-H1) - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( tmp - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}

			tmp_c_3L6L = PV_3L6L_6L - PV_3L6L_3L - threeSixBasisGrid[pos] * ANN_3L6L_3L;
			tmp_A = tmp_a_Swap - tmp_a_Curr2 - threeSixBasisGrid[pos] * term_accru_3L6L_3L2;
			tmp_B = - ( tmp_b_Curr1 + tmp_b_Curr2 + threeSixBasisGrid[pos] * term_accru_3L6L_3L1 - tmp_b_Swap);
			tmp_C = tmp_c_Swap + tmp_c_3L6L - tmp_c_Curr1 - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
			tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
					( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
			tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
					( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
					dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
   
			rate = (tmp_c_Swap + tmp_b_Swap * dfCurve[2*pos] + tmp_a_Swap * dfCurve[2*pos+1]) / (term_accru_3L6L_6L * dfCurve[2*pos+1]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			rate = (tmp_c_Curr1 + tmp_b_Curr1 * dfCurve[2*pos] ) / (term_accru_3L6L_3L1 * dfCurve[2*pos]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[2*pos] + tmp_a_Curr2 * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );			
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
		}
	}

	ANN_Swap_6M += term_accru_Swap_Fix * dfCurve[2*pos+1];
	ANN_Swap_3M += term_accru_Swap_Fix_3L1 * dfCurve[2*pos] + term_accru_Swap_Fix_3L2 * dfCurve[2*pos+1];
	ANN_Curr += term_accru_Curr1 * dfCurve[2*pos] + term_accru_Curr2 * dfCurve[2*pos+1];
	ANN_3L6L_3L += term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
	ANN_3L6L_6L += term_accru_3L6L_6L * dfCurve[2*pos+1];
	PV_Swap += term_accru_Swap_Float * sixMLRate[pos] * dfCurve[2*pos+1];
	PV_Curr += term_accru_Curr1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
				term_accru_Curr2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_3L += term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
					term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_6L += term_accru_3L6L_6L * sixMLRate[pos] * dfCurve[2*pos+1];
}

/*
    @brief function to calculate iscount factor, forward 3M Libor and forward 6M Libor
		when a frequency of swap floating leg is Quarterly and a frequency of swap fixed leg is Semi-Annual
*/
void 
AQLPriceArbFreeGenerator::calcCurve_Semi_Quar
(	const AQLDate& basedate, 
	DoubleArray& dfTerms,
	DoubleArray& dfCurve,
	DoubleMatrix& threeMLTermsMtx_Rate,
	DoubleArray& threeMLRate,
	DoubleArray& threeMLTerms_DF,
	DoubleArray& threeMLDF,
	DoubleMatrix& sixMLTermsMtx_Rate,
	DoubleArray& sixMLRate,
	DoubleArray& sixMLTerms_DF,
	DoubleArray& sixMLDF,
	bool isFRAUse,
	bool isRenAdj, 
	const LiborMarket& libMkt,
	const SwapMarket& swapMkt,
	const XCCYBasisMarket& xccyBasisMkt,
	const LiborBasisMarket& libBasisMkt,
	const FRAMarket& fra3MLMkt,
	const FRAMarket& fra6MLMkt,
	const DoubleArray& swapRateGrid,
	const DoubleArray& currBasisGrid,
	const DoubleArray& threeSixBasisGrid,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	double& PV_Swap,
	double& PV_3L6L_6L,
	double& PV_3L6L_3L,
	double& PV_Curr,
	double& ANN_Swap_6M,
	double& ANN_Swap_3M,
	double& ANN_3L6L_6L,
	double& ANN_3L6L_3L,
	double& ANN_Curr,
	double& I,
	double spotAdjust,
	double spotTerm,
	unsigned int pos )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	double spotAdjust_dol,spotAdjust_baseccy,term_f,H1, H2;
	if(isRenAdj)  spotAdjust_dol = xccyBasisMkt.pInter_usd->value(spotTerm);
	else  spotAdjust_baseccy = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, swapMkt.spotDate));

	map<AQLString, double >::const_iterator it;

	double rate;
	double term_accru_Curr1 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_Curr2 = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_3L1 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
	double term_accru_3L6L_3L2 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
	double term_accru_3L6L_6L = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Float = swapMkt.dc_Float.getTerm(dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], false);
	double term_accru_Swap_Fix_3L1 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Float_3L1 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], false);
    double term_accru_Swap_Fix_3L2 = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);
    double term_accru_Swap_Float_3L2 = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], false);

	if(!isRenAdj)
	{
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+1]);
		H1 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
		term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[2*pos+2]);
		H2 = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
	}
	else
	{
		if(pos==0)
		{
			I += xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				 xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 1.;
		}
		else
		{
			I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol) *
				  dfCurve[2*pos-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos] + spotTerm) / spotAdjust_dol);
		}
	}

	if(pos==0)
	{
		if(isFRAUse)
		{
			it = fra3MLMkt.map_term_rate.find("3M");

			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[1] );
			if(!isRenAdj) dfCurve.push_back( H1 / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	
			else dfCurve.push_back( I / (term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

			dfTerms.push_back( termGrid_3MRoll[2] );
			if(!isRenAdj) 
			{
				dfCurve.push_back( (H2 - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
			else 
			{
				dfCurve.push_back( (I - term_accru_Curr1 * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr2 * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
		}
		else
		{
			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			if(!isRenAdj) dfCurve.push_back( H1 / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );
			else dfCurve.push_back( I / (1. + term_accru_Curr1 * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}

			double tmp_a,tmp_ad,tmp_b,tmp_bd,tmp_c,tmp_cd;
			tmp_a = term_accru_Curr2 * currBasisGrid[1] + 1.;
			tmp_b = term_accru_Curr2;
			if(!isRenAdj) tmp_c = H2 - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];
			else tmp_c = I - term_accru_Curr1 * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];

			tmp_ad = term_accru_3L6L_6L * libMkt.sixMLibor - term_accru_3L6L_3L2 * threeSixBasisGrid[0];
			tmp_bd = - term_accru_3L6L_3L2;
			tmp_cd = term_accru_3L6L_3L1 * ( libMkt.threeMLibor + threeSixBasisGrid[0] ) * dfCurve[0];

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_bd*tmp_c - tmp_b*tmp_cd) / (tmp_a*tmp_bd - tmp_ad*tmp_b) ); 

			rate = (tmp_cd*tmp_a - tmp_c*tmp_ad) / (tmp_bd*tmp_c - tmp_b*tmp_cd);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );
		}
	}
	else
	{
		if( isFRAUse && pos == 1 )
		{
			it = fra3MLMkt.map_term_rate.find("6M");

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			double tmp_a_Swap,tmp_b_Swap,tmp_c_Swap,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_3L2 / term_accru_Swap_Float_3L2 * term_accru_Swap_Fix * swapRateGrid[pos];
			tmp_b_Swap = - term_accru_3L6L_3L2 / term_accru_Swap_Float_3L2 * term_accru_Swap_Float_3L1 * threeMLRate[pos*2];
			tmp_c_Swap = term_accru_3L6L_3L2 / term_accru_Swap_Float_3L2 * ( swapRateGrid[pos] * ANN_Swap_6M - PV_Swap );
			tmp_a_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * term_accru_Curr1 * ( currBasisGrid[2*pos+1] + threeMLRate[2*pos] );
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( H2 - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr );
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = - term_accru_3L6L_3L2 / term_accru_Curr2 * (- tmp + term_accru_Curr1 * (currBasisGrid[2*pos+1] + threeMLRate[2*pos]));
				tmp_c_Curr2 = term_accru_3L6L_3L2 / term_accru_Curr2 * ( I - currBasisGrid[2*pos+1] * ANN_Curr - PV_Curr );
			}

			tmp_A = tmp_a_Swap - tmp_a_Curr2;
			tmp_B = tmp_b_Swap - tmp_b_Curr2;
			tmp_C = tmp_c_Swap - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
			tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
					( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
			tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
					( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
					dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );
			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[2*pos] + tmp_a_Curr2 * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );
			
			double tmp_PV3L = PV_3L6L_3L + term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
                                           term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
            double tmp_ANN3L = ANN_3L6L_3L + term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
            double tmp_ANN6L = ANN_3L6L_6L + term_accru_3L6L_6L * dfCurve[2*pos+1];
			rate = -(PV_3L6L_6L - tmp_ANN3L * threeSixBasisGrid[pos] - tmp_PV3L)/(term_accru_3L6L_6L * dfCurve[2*pos+1]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );			
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
		}
		else
		{
			double tmp_a_Swap,tmp_c_Swap,tmp_b_Curr1,tmp_c_Curr1,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,
				tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_Swap_Fix * swapRateGrid[pos];
			tmp_c_Swap = swapRateGrid[pos] * ANN_Swap_6M - PV_Swap;
			tmp_b_Curr1 = - term_accru_Swap_Float_3L1 / term_accru_Curr1 * ( 1. + term_accru_Curr1 * currBasisGrid[2*pos] );
            
			if(!isRenAdj)
			{
				tmp_c_Curr1 = term_accru_Swap_Float_3L1 / term_accru_Curr1 * ( H1 - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}
			else
			{
				tmp_c_Curr1 = term_accru_Swap_Float_3L1 / term_accru_Curr1 * ( I - currBasisGrid[2*pos] * ANN_Curr - PV_Curr );
			}

			tmp_a_Curr2 = - term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( 1. + currBasisGrid[2*pos+1] * term_accru_Curr2 );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( 1. - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ((H2-H1) - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( tmp - term_accru_Curr1 * ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) );
				tmp_c_Curr2 = term_accru_Swap_Float_3L2 / term_accru_Curr2 * ( - ( currBasisGrid[2*pos+1] - currBasisGrid[2*pos] ) * ANN_Curr);
			}

			tmp_A = tmp_a_Swap - tmp_a_Curr2;
			tmp_B = - (tmp_b_Curr1 + tmp_b_Curr2);
			tmp_C = tmp_c_Swap - tmp_c_Curr1 - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] );
			tmp_Bd = - ( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] + termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos] ) / 
					( termGrid_3MRoll[2*pos+2] - termGrid_3MRoll[2*pos+1] ) / ( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] );
			tmp_Cd = ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] + termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos-1] ) / 
					( termGrid_3MRoll[2*pos+1] - termGrid_3MRoll[2*pos] ) / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] ) * dfCurve[2*pos-1] -
					dfCurve[2*pos-2] / ( termGrid_3MRoll[2*pos] - termGrid_3MRoll[2*pos-1] );

			dfTerms.push_back( termGrid_3MRoll[2*pos+1] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
			dfTerms.push_back( termGrid_3MRoll[2*pos+2] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );

			rate = (tmp_c_Curr1 + tmp_b_Curr1 * dfCurve[2*pos] ) / (term_accru_3L6L_3L1 * dfCurve[2*pos]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos], termGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos], dateGrid_3MRoll[2*pos+1], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[2*pos] + tmp_a_Curr2 * dfCurve[2*pos+1]) / (term_accru_3L6L_3L2 * dfCurve[2*pos+1]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[2*pos+1], termGrid_3MRoll[2*pos+2], dateGrid_3MRoll[2*pos+1], dateGrid_3MRoll[2*pos+2], libMkt.dc );			
			
			double tmp_PV3L = PV_3L6L_3L + term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
                                           term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
            double tmp_ANN3L = ANN_3L6L_3L + term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
            double tmp_ANN6L = ANN_3L6L_6L + term_accru_3L6L_6L * dfCurve[2*pos+1];
			rate = -(PV_3L6L_6L - tmp_ANN3L * threeSixBasisGrid[pos] - tmp_PV3L)/(term_accru_3L6L_6L * dfCurve[2*pos+1]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[pos], termGrid_6MRoll[pos+1], dateGrid_6MRoll[pos], dateGrid_6MRoll[pos+1], libMkt.dc );
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[2*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[2*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[2*pos+1] + spotTerm) / spotAdjust_dol);
			}
		}
	}

	ANN_Swap_6M += term_accru_Swap_Fix * dfCurve[2*pos+1];
	ANN_Swap_3M += term_accru_Swap_Fix_3L1 * dfCurve[2*pos] + term_accru_Swap_Fix_3L2 * dfCurve[2*pos+1];
	ANN_Curr += term_accru_Curr1 * dfCurve[2*pos] + term_accru_Curr2 * dfCurve[2*pos+1];
	ANN_3L6L_3L += term_accru_3L6L_3L1 * dfCurve[2*pos] + term_accru_3L6L_3L2 * dfCurve[2*pos+1];
	ANN_3L6L_6L += term_accru_3L6L_6L * dfCurve[2*pos+1];
	PV_Swap += threeMLRate[2*pos] * term_accru_Swap_Float_3L1 * dfCurve[2*pos] + threeMLRate[2*pos+1] * term_accru_Swap_Float_3L2 * dfCurve[2*pos+1];
	PV_Curr += term_accru_Curr1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
				term_accru_Curr2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_3L += term_accru_3L6L_3L1 * threeMLRate[2*pos] * dfCurve[2*pos] + 
					term_accru_3L6L_3L2 * threeMLRate[2*pos+1] * dfCurve[2*pos+1];
	PV_3L6L_6L += term_accru_3L6L_6L * sixMLRate[pos] * dfCurve[2*pos+1];
}

/*
    @brief function to calculate iscount factor, forward 3M Libor and forward 6M Libor
		when a frequency of swap floating leg is Annual and a frequency of swap fixed leg is Semi-Annual
*/
void 
AQLPriceArbFreeGenerator::calcCurve_Annu_Semi
(	const AQLDate& basedate,
	DoubleArray& dfTerms,
	DoubleArray& dfCurve,
	DoubleMatrix& threeMLTermsMtx_Rate,
	DoubleArray& threeMLRate,
	DoubleArray& threeMLTerms_DF,
	DoubleArray& threeMLDF,
	DoubleMatrix& sixMLTermsMtx_Rate,
	DoubleArray& sixMLRate,
	DoubleArray& sixMLTerms_DF,
	DoubleArray& sixMLDF,
	bool isFRAUse,
	bool isRenAdj, 
	const LiborMarket& libMkt,
	const SwapMarket& swapMkt,
	const XCCYBasisMarket& xccyBasisMkt,
	const LiborBasisMarket& libBasisMkt,
	const FRAMarket& fra3MLMkt,
	const FRAMarket& fra6MLMkt,
	const DoubleArray& swapRateGrid,
	const DoubleArray& currBasisGrid,
	const DoubleArray& threeSixBasisGrid,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	double& PV_Swap,
	double& PV_3L6L_6L,
	double& PV_3L6L_3L,
	double& PV_Curr,
	double& ANN_Swap,
	double& ANN_3L6L_6L,
	double& ANN_3L6L_3L,
	double& ANN_Curr,
	double& I,
	double spotAdjust,
	double spotTerm,
	unsigned int pos )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	double spotAdjust_dol,spotAdjust_baseccy,term_f;
	DoubleArray H(4);
	if(isRenAdj)  spotAdjust_dol = xccyBasisMkt.pInter_usd->value(spotTerm);
	else  spotAdjust_baseccy = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, swapMkt.spotDate));

	map<AQLString, double >::const_iterator it;

	double rate;
	DoubleArray term_accru_Curr(4),term_accru_3L6L_3L(4);
	for (size_t i=0; i<4; i++)
	{
		term_accru_Curr[i] = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[4*pos+i], dateGrid_3MRoll[4*pos+i+1], false);
		term_accru_3L6L_3L[i] = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[4*pos+i], dateGrid_3MRoll[4*pos+i+1], false);
	}

	DoubleArray term_accru_3L6L_6L(2),term_accru_Swap_Float(2);
	for (size_t i=0; i<2; i++)
	{
		term_accru_Swap_Float[i] = swapMkt.dc_Float.getTerm(dateGrid_6MRoll[2*pos+i], dateGrid_6MRoll[2*pos+i+1], false);
		term_accru_3L6L_6L[i] = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[2*pos+i], dateGrid_6MRoll[2*pos+i+1], false);
	}
	double term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+2], false);

	if(!isRenAdj)
	{
		for (size_t i=0; i<4; i++)
		{
			term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[4*pos+i+1]);
			H[i] = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
		}
	}
	else
	{
		if(pos==0)
		{
			I += xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) * 
				 xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol - 1.;
		}
		else
		{
			I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos] + spotTerm) / spotAdjust_dol) *
				  dfCurve[4*pos-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos] + spotTerm) / spotAdjust_dol);
		}
	}

	if(pos==0)
	{
		if(isFRAUse)
		{
			it = fra3MLMkt.map_term_rate.find("3M");

			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos], termGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos], dateGrid_3MRoll[4*pos+1], libMkt.dc );

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+1], termGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos+2], libMkt.dc );

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos], termGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[1] );
			if(!isRenAdj) dfCurve.push_back( H[0] / (term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	
			else dfCurve.push_back( I / (term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );

			dfTerms.push_back( termGrid_3MRoll[2] );
			if(!isRenAdj) 
			{
				dfCurve.push_back( (H[1] - term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr[1] * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
			else 
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol);

				dfCurve.push_back( (I - term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr[1] * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );				

				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol) *
				  dfCurve[4*pos+1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol);
			}

			double tmp_ANN_3L6L_3L = term_accru_3L6L_3L[0] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * dfCurve[4*pos+1];
			double tmp_ANN_3L6L_6L = term_accru_3L6L_6L[0] * dfCurve[4*pos+1];
			double tmp_ANN_Curr = term_accru_Curr[0] * dfCurve[4*pos] + term_accru_Curr[1] * dfCurve[4*pos+1];
			double tmp_PV_Swap = term_accru_Swap_Float[0] * sixMLRate[2*pos] * dfCurve[4*pos+1];
			double tmp_PV_Curr = term_accru_Curr[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_Curr[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_3L = term_accru_3L6L_3L[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_6L = term_accru_3L6L_6L[0] * sixMLRate[2*pos] * dfCurve[4*pos+1];
			
			it = fra3MLMkt.map_term_rate.find("6M");

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+2], termGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+3], libMkt.dc );

			double tmp_a_Swap,tmp_c_Swap,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,tmp_c_3L6L,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_6L[1] / term_accru_Swap_Float[1] * term_accru_Swap_Fix * swapRateGrid[pos];
			tmp_c_Swap = - term_accru_3L6L_6L[1] / term_accru_Swap_Float[1] * tmp_PV_Swap;
			tmp_a_Curr2 = - term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( 1. + currBasisGrid[4*pos+3] * term_accru_Curr[3] );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = - term_accru_3L6L_3L[3] / term_accru_Curr[3] * term_accru_Curr[2] * ( currBasisGrid[4*pos+3] + threeMLRate[4*pos+2] );
				tmp_c_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( H[3] - currBasisGrid[4*pos+3] * tmp_ANN_Curr - tmp_PV_Curr );
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol - 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = - term_accru_3L6L_3L[3] / term_accru_Curr[3] * (- tmp + term_accru_Curr[2] * (currBasisGrid[4*pos+3] + threeMLRate[4*pos+2]));
				tmp_c_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( I - currBasisGrid[4*pos+3] * tmp_ANN_Curr - tmp_PV_Curr );
			}

			tmp_c_3L6L = tmp_PV_3L6L_6L - tmp_PV_3L6L_3L - threeSixBasisGrid[2*pos+1] * tmp_ANN_3L6L_3L;
			tmp_A = tmp_a_Swap - tmp_a_Curr2 - threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[3];
			tmp_B = - ( threeMLRate[4*pos+2] + threeSixBasisGrid[2*pos+1] ) * term_accru_3L6L_3L[2] - tmp_b_Curr2;
			tmp_C = tmp_c_Swap + tmp_c_3L6L - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] );
			tmp_Bd = - ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] + termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+2] ) / 
					( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] ) / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
			tmp_Cd = ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
					( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) * dfCurve[4*pos+1] -
					dfCurve[4*pos] / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );

			dfTerms.push_back( termGrid_3MRoll[4*pos+3] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );
			dfTerms.push_back( termGrid_3MRoll[4*pos+4] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );

			rate = (tmp_c_Swap + tmp_a_Swap * dfCurve[4*pos+3]) / (term_accru_3L6L_6L[1] * dfCurve[4*pos+3]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos+1], termGrid_6MRoll[2*pos+2], dateGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos+2], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[4*pos+2] + tmp_a_Curr2 * dfCurve[4*pos+3]) / (term_accru_3L6L_3L[3] * dfCurve[4*pos+3]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+3], termGrid_3MRoll[4*pos+4], dateGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+4], libMkt.dc );
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos+2] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);
			}
		}
		else
		{
			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos], termGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos], dateGrid_3MRoll[4*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[4*pos+1] );
			if(!isRenAdj) dfCurve.push_back( H[0] / (1. + term_accru_Curr[0] * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );
			else dfCurve.push_back( I / (1. + term_accru_Curr[0] * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol);
			}

			double tmp_a,tmp_ad,tmp_b,tmp_bd,tmp_c,tmp_cd;
			tmp_a = term_accru_Curr[1] * currBasisGrid[1] + 1.;
			tmp_b = term_accru_Curr[1];
			if(!isRenAdj) tmp_c = H[1] - term_accru_Curr[0] * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];
			else tmp_c = I - term_accru_Curr[0] * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];

			tmp_ad = term_accru_3L6L_6L[0] * libMkt.sixMLibor - term_accru_3L6L_3L[1] * threeSixBasisGrid[0];
			tmp_bd = - term_accru_3L6L_3L[1];
			tmp_cd = term_accru_3L6L_3L[0] * ( libMkt.threeMLibor + threeSixBasisGrid[0] ) * dfCurve[0];

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos], termGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[4*pos+2] );
			dfCurve.push_back( (tmp_bd*tmp_c - tmp_b*tmp_cd) / (tmp_a*tmp_bd - tmp_ad*tmp_b) ); 

			rate = (tmp_cd*tmp_a - tmp_c*tmp_ad) / (tmp_bd*tmp_c - tmp_b*tmp_cd);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+1], termGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos+2], libMkt.dc );
			
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol) *
				  dfCurve[4*pos+1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol);
			}

			double tmp_ANN_3L6L_3L = term_accru_3L6L_3L[0] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * dfCurve[4*pos+1];
			double tmp_ANN_3L6L_6L = term_accru_3L6L_6L[0] * dfCurve[4*pos+1];
			double tmp_ANN_Curr = term_accru_Curr[0] * dfCurve[4*pos] + term_accru_Curr[1] * dfCurve[4*pos+1];
			double tmp_PV_Swap = term_accru_Swap_Float[0] * sixMLRate[2*pos] * dfCurve[4*pos+1];
			double tmp_PV_Curr = term_accru_Curr[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_Curr[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_3L = term_accru_3L6L_3L[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_6L = term_accru_3L6L_6L[0] * sixMLRate[2*pos] * dfCurve[4*pos+1];

			double tmp_a_Swap,tmp_c_Swap,tmp_b_Curr1,tmp_c_Curr1,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,
				tmp_c_3L6L,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_6L[1] / term_accru_Swap_Float[1] * term_accru_Swap_Fix * swapRateGrid[pos];
			tmp_c_Swap = - term_accru_3L6L_6L[1] / term_accru_Swap_Float[1] * tmp_PV_Swap;
			tmp_b_Curr1 = - term_accru_3L6L_3L[2] / term_accru_Curr[2] * ( 1. + term_accru_Curr[2] * currBasisGrid[4*pos+2] );
            
			if(!isRenAdj)
			{
				tmp_c_Curr1 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * ( H[2] - currBasisGrid[4*pos+2] * tmp_ANN_Curr - tmp_PV_Curr );
			}
			else
			{
				tmp_c_Curr1 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * ( I - currBasisGrid[4*pos+2] * tmp_ANN_Curr - tmp_PV_Curr );
			}

			tmp_a_Curr2 = - term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( 1. + currBasisGrid[4*pos+3] * term_accru_Curr[3] );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( 1. - term_accru_Curr[2] * ( currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2] ) );
				tmp_c_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ((H[3]-H[2]) - ( currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2] ) * tmp_ANN_Curr);
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( tmp - term_accru_Curr[2] * ( currBasisGrid[4*pos+2] - currBasisGrid[4*pos+2] ) );
				tmp_c_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( - ( currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2] ) * tmp_ANN_Curr);
			}

			tmp_c_3L6L = tmp_PV_3L6L_6L - tmp_PV_3L6L_3L - threeSixBasisGrid[2*pos+1] * tmp_ANN_3L6L_3L;
			tmp_A = tmp_a_Swap - tmp_a_Curr2 - threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[3];
			tmp_B = - ( tmp_b_Curr1 + tmp_b_Curr2 + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[2] );
			tmp_C = tmp_c_Swap + tmp_c_3L6L - tmp_c_Curr1 - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] );
			tmp_Bd = - ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] + termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+2] ) / 
					( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] ) / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
			tmp_Cd = ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
					( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) * dfCurve[4*pos+1] -
					dfCurve[4*pos] / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );

			dfTerms.push_back( termGrid_3MRoll[4*pos+3] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
			dfTerms.push_back( termGrid_3MRoll[4*pos+4] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
   
			rate = (tmp_c_Swap + tmp_a_Swap * dfCurve[4*pos+3]) / (term_accru_3L6L_6L[1] * dfCurve[4*pos+3]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos+1], termGrid_6MRoll[2*pos+2], dateGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos+2], libMkt.dc );

			rate = (tmp_c_Curr1 + tmp_b_Curr1 * dfCurve[4*pos+2] ) / (term_accru_3L6L_3L[2] * dfCurve[4*pos+2]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+2], termGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+3], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[4*pos+2] + tmp_a_Curr2 * dfCurve[4*pos+3]) / (term_accru_3L6L_3L[3] * dfCurve[4*pos+3]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+3], termGrid_3MRoll[4*pos+4], dateGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+4], libMkt.dc );			
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos+2] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);
			}
		}
	}
	else
	{
		double A1,A2,A3,B1,B2,B3,C1,C2,C3,D1,D2,D3,E1,E2,E3,F1,F2,F3,e1,e2,e3,e4,f1,f2,f3,f4,J1,J2,K1,K2;

		A1 = 1. / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		B1 = - ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos] ) / 
			   ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) / ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] );
		C1 = ( termGrid_3MRoll[4*pos] - termGrid_3MRoll[4*pos-1] + termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos-1] ) / 
			 ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] ) / ( termGrid_3MRoll[4*pos] - termGrid_3MRoll[4*pos-1] );
		D1 = - 1. / ( termGrid_3MRoll[4*pos] - termGrid_3MRoll[4*pos-1] );
		E1 = - B1 / A1;
		F1 = - (C1 * dfCurve[4*pos-1] + D1 * dfCurve[4*pos-2]) / A1;

		A2 = 1. / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
		B2 = - ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
			   ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		C2 = ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] + termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos] ) / 
			 ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) / ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] );
		D2 = - 1. / ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] );
		E2 = - (B2 * E1 + C2) / A2;
		F2 = - (B2 * F1 + D2 * dfCurve[4*pos-1]) / A2;

		A3 = 1. / ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] );
		B3 = - ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] + termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+2] ) / 
			   ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] ) / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
		C3 = ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
			 ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		D3 = - 1. / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		E3 = - (B3 * E2 + C3 * E1 + D3) / A3;
		F3 = - (B3 * F2 + C3 * F1) / A3;

		e1 = - term_accru_3L6L_3L[0] / term_accru_Curr[0] * (1. + currBasisGrid[4*pos] * term_accru_Curr[0]);
		if (isRenAdj)
		{
			DoubleArray g(4);
			for (size_t i=1; i<4; i++)
			{
				g[i] = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+i] + spotTerm) 
						/ xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1+i] + spotTerm) 
						* xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i+1] + spotTerm) / spotAdjust_dol 
						- xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol) 
						  / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol);
			}

			f1 = term_accru_3L6L_3L[0] / term_accru_Curr[0] * (I - PV_Curr - currBasisGrid[4*pos] * ANN_Curr);
			e2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				(1. - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * term_accru_Curr[0] 
					- (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * E1 + g[1]);
			f2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				( - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * ANN_Curr
				  - (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * F1);
			e3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				(- (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[0]
				 + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * E1
					- (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * E2 + g[2] * E1);
			f3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				( - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * ANN_Curr
				  + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * F1
				  - (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * F2 + g[2] * F1);
			e4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				(- (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[0]
				 - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * E1
				 + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * E2
					- (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * E3 + g[3] * E2);
			f4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				( - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * ANN_Curr
				  - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * F1
				  + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * F2
				  - (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * F3 + g[3] * F2);
		}
		else
		{
			f1 = term_accru_3L6L_3L[0] / term_accru_Curr[0] * (H[0] - PV_Curr - currBasisGrid[4*pos] * ANN_Curr);
			e2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				(1. - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * term_accru_Curr[0] 
					- (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * E1);
			f2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				( H[1] - H[0] - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * ANN_Curr
				  - (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * F1);
			e3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				(- (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[0]
				 + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * E1
					- (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * E2);
			f3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				( H[2] - H[1] - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * ANN_Curr
				  + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * F1
				  - (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * F2);
			e4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				(- (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[0]
				 - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * E1
				 + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * E2
					- (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * E3);
			f4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				( H[3] - H[2] - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * ANN_Curr
				  - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * F1
				  + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * F2
				  - (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * F3);
		}
		
		J1 = term_accru_Swap_Float[0] / term_accru_3L6L_6L[0] * 
			( e1 + e2 + threeSixBasisGrid[2*pos] * term_accru_3L6L_3L[0] + threeSixBasisGrid[2*pos] * term_accru_3L6L_3L[1] * E1);
		K1 = term_accru_Swap_Float[0] / term_accru_3L6L_6L[0] * 
			( - PV_3L6L_6L + PV_3L6L_3L + threeSixBasisGrid[2*pos] * ANN_3L6L_3L + f1 + f2 
			  + threeSixBasisGrid[2*pos] * term_accru_3L6L_3L[1] * F1 );
		J2 = term_accru_Swap_Float[1] / term_accru_3L6L_6L[1] * 
			( (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * term_accru_3L6L_3L[0] 
			  + (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * term_accru_3L6L_3L[1] * E1 + e3 + e4
			  + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[2] * E2 + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[3] * E3);
		K2 = term_accru_Swap_Float[1] / term_accru_3L6L_6L[1] * 
			( (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * ANN_3L6L_3L
			   + (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * term_accru_3L6L_3L[1] * F1 + f3 + f4 
			   + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[2] * F2 + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[3] * F3);


		dfTerms.push_back( termGrid_3MRoll[4*pos+1] );
		dfCurve.push_back( (swapRateGrid[pos] * ANN_Swap - PV_Swap + swapRateGrid[pos] * term_accru_Swap_Fix * F3 - K2 - K1) / 
						   (J1 + J2 - swapRateGrid[pos] * term_accru_Swap_Fix * E3) );
		rate = (e1 * dfCurve[4*pos] + f1) / (term_accru_3L6L_3L[0] * dfCurve[4*pos]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos], termGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos], dateGrid_3MRoll[4*pos+1], libMkt.dc );

		dfTerms.push_back( termGrid_3MRoll[4*pos+2] );
		dfCurve.push_back( E1 * dfCurve[4*pos] + F1 );
		rate = (e2 * dfCurve[4*pos] + f2) / (term_accru_3L6L_3L[1] * dfCurve[4*pos+1]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos+1], termGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos+2], libMkt.dc );

		dfTerms.push_back( termGrid_3MRoll[4*pos+3] );
		dfCurve.push_back(E2 * dfCurve[4*pos] + F2);
		rate = (e3 * dfCurve[4*pos] + f3) / (term_accru_3L6L_3L[2] * dfCurve[4*pos+2]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos+2], termGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+3], libMkt.dc );

		dfTerms.push_back( termGrid_3MRoll[4*pos+4] );
		dfCurve.push_back(E3 * dfCurve[4*pos] + F3);
		rate = (e4 * dfCurve[4*pos] + f4) / (term_accru_3L6L_3L[3] * dfCurve[4*pos+3]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos+3], termGrid_3MRoll[4*pos+4], dateGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+4], libMkt.dc );

		rate = (J1 * dfCurve[4*pos] + K1) / (term_accru_3L6L_6L[0] * dfCurve[4*pos+1]);
        insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                    termGrid_6MRoll[2*pos], termGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+1], libMkt.dc );

		rate = (J2 * dfCurve[4*pos] + K2) / (term_accru_3L6L_6L[1] * dfCurve[4*pos+3]);
        insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                    termGrid_6MRoll[2*pos+1], termGrid_6MRoll[2*pos+2], dateGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos+2], libMkt.dc );
		
		if (isRenAdj)
		{
			for (size_t i=1; i<4; i++)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+i] + spotTerm) 
					/ xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1+i] + spotTerm) 
					* xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i+1] + spotTerm) / spotAdjust_dol 
					- xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol) 
					* dfCurve[4*pos+i-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol);
			}
		}
	}

	ANN_Swap += term_accru_Swap_Fix * dfCurve[4*pos+3];
	for (size_t i=0; i<4; i++)
	{
		ANN_Curr += term_accru_Curr[i] * dfCurve[4*pos+i];
		ANN_3L6L_3L += term_accru_3L6L_3L[i] * dfCurve[4*pos+i];
		PV_Curr += term_accru_Curr[i] * threeMLRate[4*pos+i] * dfCurve[4*pos+i];
		PV_3L6L_3L += term_accru_3L6L_3L[i] * threeMLRate[4*pos+i] * dfCurve[4*pos+i];
	}
	for (size_t i=0; i<2; i++)
	{
		ANN_3L6L_6L += term_accru_3L6L_6L[i] * dfCurve[4*pos+1+2*i];
		PV_Swap += term_accru_Swap_Float[i] * sixMLRate[2*pos+i] * dfCurve[4*pos+1+2*i];
		PV_3L6L_6L += term_accru_3L6L_6L[i] * sixMLRate[2*pos+i] * dfCurve[4*pos+1+2*i];
	}
}

/*
    @brief function to calculate Discount factor, forward 3M Libor and forward 6M Libor
		when a frequency of swap floating leg is Annual and a frequency of swap fixed leg is Quarterly
*/
void 
AQLPriceArbFreeGenerator::calcCurve_Annu_Quar(	
	const AQLDate& basedate,
	DoubleArray& dfTerms,
	DoubleArray& dfCurve,
	DoubleMatrix& threeMLTermsMtx_Rate,
	DoubleArray& threeMLRate,
	DoubleArray& threeMLTerms_DF,
	DoubleArray& threeMLDF,
	DoubleMatrix& sixMLTermsMtx_Rate,
	DoubleArray& sixMLRate,
	DoubleArray& sixMLTerms_DF,
	DoubleArray& sixMLDF,
	bool isFRAUse,
	bool isRenAdj, 
	const LiborMarket& libMkt,
	const SwapMarket& swapMkt,
	const XCCYBasisMarket& xccyBasisMkt,
	const LiborBasisMarket& libBasisMkt,
	const FRAMarket& fra3MLMkt,
	const FRAMarket& fra6MLMkt,
	const DoubleArray& swapRateGrid,
	const DoubleArray& currBasisGrid,
	const DoubleArray& threeSixBasisGrid,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	double& PV_Swap,
	double& PV_3L6L_6L,
	double& PV_3L6L_3L,
	double& PV_Curr,
	double& ANN_Swap,
	double& ANN_3L6L_6L,
	double& ANN_3L6L_3L,
	double& ANN_Curr,
	double& I,
	double spotAdjust,
	double spotTerm,
	unsigned int pos )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	double spotAdjust_dol,spotAdjust_baseccy,term_f;
	DoubleArray H(4);
	if(isRenAdj)  spotAdjust_dol = xccyBasisMkt.pInter_usd->value(spotTerm);
	else  spotAdjust_baseccy = xccyBasisMkt.pInter_fPrices->value(dc_act365.getTerm(basedate, swapMkt.spotDate));

	map<AQLString, double >::const_iterator it;

	double rate;
	DoubleArray term_accru_Curr(4),term_accru_3L6L_3L(4),term_accru_Swap_Float(4);
	for (size_t i=0; i<4; i++)
	{
		term_accru_Curr[i] = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[4*pos+i], dateGrid_3MRoll[4*pos+i+1], false);
		term_accru_3L6L_3L[i] = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[4*pos+i], dateGrid_3MRoll[4*pos+i+1], false);
		term_accru_Swap_Float[i] = swapMkt.dc_Float.getTerm(dateGrid_3MRoll[4*pos+i], dateGrid_3MRoll[4*pos+i+1], false);
	}

	DoubleArray term_accru_3L6L_6L(2);
	for (size_t i=0; i<2; i++)
	{
		term_accru_3L6L_6L[i] = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[2*pos+i], dateGrid_6MRoll[2*pos+i+1], false);
	}
	double term_accru_Swap_Fix = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+2], false);

	if(!isRenAdj)
	{
		for (size_t i=0; i<4; i++)
		{
			term_f = dc_act365.getTerm(basedate, dateGrid_3MRoll[4*pos+i+1]);
			H[i] = xccyBasisMkt.pInter_fPrices->value(term_f) / spotAdjust_baseccy;
		}
	}
	else
	{
		if(pos==0)
		{
			I += xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) * 
				 xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol - 1.;
		}
		else
		{
			I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos] + spotTerm) / spotAdjust_dol) *
				  dfCurve[4*pos-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos] + spotTerm) / spotAdjust_dol);
		}
	}

	if(pos==0)
	{
		if(isFRAUse)
		{
			it = fra3MLMkt.map_term_rate.find("3M");

			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos], termGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos], dateGrid_3MRoll[4*pos+1], libMkt.dc );

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+1], termGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos+2], libMkt.dc );

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos], termGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[1] );
			if(!isRenAdj) dfCurve.push_back( H[0] / (term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );	
			else dfCurve.push_back( I / (term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[0] ) + 1.) );

			dfTerms.push_back( termGrid_3MRoll[2] );
			if(!isRenAdj) 
			{
				dfCurve.push_back( (H[1] - term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr[1] * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );
			}
			else 
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol);

				dfCurve.push_back( (I - term_accru_Curr[0] * ( threeMLRate[0] + currBasisGrid[1] ) * dfCurve[0]) /
								   (term_accru_Curr[1] * ( threeMLRate[1] + currBasisGrid[1] ) + 1.) );				

				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol) *
				  dfCurve[4*pos+1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol);
			}

			double tmp_ANN_3L6L_3L = term_accru_3L6L_3L[0] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * dfCurve[4*pos+1];
			double tmp_ANN_3L6L_6L = term_accru_3L6L_6L[0] * dfCurve[4*pos+1];
			double tmp_ANN_Curr = term_accru_Curr[0] * dfCurve[4*pos] + term_accru_Curr[1] * dfCurve[4*pos+1];
			double tmp_PV_Swap = term_accru_Swap_Float[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_Swap_Float[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_Curr = term_accru_Curr[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_Curr[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_3L = term_accru_3L6L_3L[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_6L = term_accru_3L6L_6L[0] * sixMLRate[2*pos] * dfCurve[4*pos+1];
			
			it = fra3MLMkt.map_term_rate.find("6M");

			rate = it->second;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+2], termGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+3], libMkt.dc );

			double tmp_a_Swap,tmp_b_Swap,tmp_c_Swap,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_3L6L_3L[3] / term_accru_Swap_Float[3] * term_accru_Swap_Fix * swapRateGrid[pos];
			tmp_b_Swap = - term_accru_3L6L_3L[3] / term_accru_Swap_Float[3] * term_accru_Swap_Float[2] * threeMLRate[4*pos+2];
			tmp_c_Swap = - term_accru_3L6L_3L[3] / term_accru_Swap_Float[3] * tmp_PV_Swap;
			tmp_a_Curr2 = - term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( 1. + currBasisGrid[4*pos+3] * term_accru_Curr[3] );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = - term_accru_3L6L_3L[3] / term_accru_Curr[3] * term_accru_Curr[2] * ( currBasisGrid[4*pos+3] + threeMLRate[4*pos+2] );
				tmp_c_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( H[3] - currBasisGrid[4*pos+3] * tmp_ANN_Curr - tmp_PV_Curr );
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol - 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = - term_accru_3L6L_3L[3] / term_accru_Curr[3] * (- tmp + term_accru_Curr[2] * (currBasisGrid[4*pos+3] + threeMLRate[4*pos+2]));
				tmp_c_Curr2 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * ( I - currBasisGrid[4*pos+3] * tmp_ANN_Curr - tmp_PV_Curr );
			}

			tmp_A = tmp_a_Swap - tmp_a_Curr2;
			tmp_B = tmp_b_Swap - tmp_b_Curr2;
			tmp_C = tmp_c_Swap - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] );
			tmp_Bd = - ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] + termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+2] ) / 
					( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] ) / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
			tmp_Cd = ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
					( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) * dfCurve[4*pos+1] -
					dfCurve[4*pos] / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );

			dfTerms.push_back( termGrid_3MRoll[4*pos+3] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );
			dfTerms.push_back( termGrid_3MRoll[4*pos+4] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / (tmp_A * tmp_Bd - tmp_Ad * tmp_B) );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[4*pos+2] + tmp_a_Curr2 * dfCurve[4*pos+3]) / (term_accru_3L6L_3L[3] * dfCurve[4*pos+3]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+3], termGrid_3MRoll[4*pos+4], dateGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+4], libMkt.dc );
			
			tmp_PV_3L6L_3L  += term_accru_3L6L_3L[2] * threeMLRate[4*pos+2] * dfCurve[4*pos+2] + 
								term_accru_3L6L_3L[3] * threeMLRate[4*pos+3] * dfCurve[4*pos+3];
            tmp_ANN_3L6L_3L += term_accru_3L6L_3L[2] * dfCurve[4*pos+2] + term_accru_3L6L_3L[3] * dfCurve[4*pos+3];
            tmp_ANN_3L6L_6L += term_accru_3L6L_6L[1] * dfCurve[4*pos+3];
			rate = -(tmp_PV_3L6L_6L - tmp_ANN_3L6L_3L * threeSixBasisGrid[2*pos+1] - tmp_PV_3L6L_3L)/(term_accru_3L6L_6L[1] * dfCurve[4*pos+3]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos+1], termGrid_6MRoll[2*pos+2], dateGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos+2], libMkt.dc );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos+2] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);
			}
		}
		else
		{
			rate = libMkt.threeMLibor;
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos], termGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos], dateGrid_3MRoll[4*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[4*pos+1] );
			if(!isRenAdj) dfCurve.push_back( H[0] / (1. + term_accru_Curr[0] * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );
			else dfCurve.push_back( I / (1. + term_accru_Curr[0] * ( currBasisGrid[0] + libMkt.threeMLibor ) ) );

			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+1] + spotTerm) / spotAdjust_dol);
			}

			double tmp_a,tmp_ad,tmp_b,tmp_bd,tmp_c,tmp_cd;
			tmp_a = term_accru_Curr[1] * currBasisGrid[1] + 1.;
			tmp_b = term_accru_Curr[1];
			if(!isRenAdj) tmp_c = H[1] - term_accru_Curr[0] * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];
			else tmp_c = I - term_accru_Curr[0] * ( libMkt.threeMLibor + currBasisGrid[1] ) * dfCurve[0];

			tmp_ad = term_accru_3L6L_6L[0] * libMkt.sixMLibor - term_accru_3L6L_3L[1] * threeSixBasisGrid[0];
			tmp_bd = - term_accru_3L6L_3L[1];
			tmp_cd = term_accru_3L6L_3L[0] * ( libMkt.threeMLibor + threeSixBasisGrid[0] ) * dfCurve[0];

			rate = libMkt.sixMLibor;
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos], termGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+1], libMkt.dc );

			dfTerms.push_back( termGrid_3MRoll[4*pos+2] );
			dfCurve.push_back( (tmp_bd*tmp_c - tmp_b*tmp_cd) / (tmp_a*tmp_bd - tmp_ad*tmp_b) ); 

			rate = (tmp_cd*tmp_a - tmp_c*tmp_ad) / (tmp_bd*tmp_c - tmp_b*tmp_cd);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+1], termGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos+2], libMkt.dc );
			
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+2] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) * 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol - 
				  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol) *
				  dfCurve[4*pos+1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+2] + spotTerm) / spotAdjust_dol);
			}

			double tmp_ANN_3L6L_3L = term_accru_3L6L_3L[0] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * dfCurve[4*pos+1];
			double tmp_ANN_3L6L_6L = term_accru_3L6L_6L[0] * dfCurve[4*pos+1];
			double tmp_ANN_Curr = term_accru_Curr[0] * dfCurve[4*pos] + term_accru_Curr[1] * dfCurve[4*pos+1];
			double tmp_PV_Swap = term_accru_Swap_Float[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_Swap_Float[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_Curr = term_accru_Curr[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_Curr[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_3L = term_accru_3L6L_3L[0] * threeMLRate[4*pos] * dfCurve[4*pos] + term_accru_3L6L_3L[1] * threeMLRate[4*pos+1] * dfCurve[4*pos+1];
			double tmp_PV_3L6L_6L = term_accru_3L6L_6L[0] * sixMLRate[2*pos] * dfCurve[4*pos+1];

			double tmp_a_Swap,tmp_c_Swap,tmp_b_Curr1,tmp_c_Curr1,tmp_a_Curr2,tmp_b_Curr2,tmp_c_Curr2,
				tmp_A,tmp_B,tmp_C,tmp_Ad,tmp_Bd,tmp_Cd;
            
			tmp_a_Swap = term_accru_Swap_Fix * swapRateGrid[pos];
			tmp_c_Swap = - tmp_PV_Swap;
			tmp_b_Curr1 = - term_accru_Swap_Float[2] / term_accru_Curr[2] * ( 1. + term_accru_Curr[2] * currBasisGrid[4*pos+2] );
            
			if(!isRenAdj)
			{
				tmp_c_Curr1 = term_accru_Swap_Float[2] / term_accru_Curr[2] * ( H[2] - currBasisGrid[4*pos+2] * tmp_ANN_Curr - tmp_PV_Curr );
			}
			else
			{
				tmp_c_Curr1 = term_accru_Swap_Float[2] / term_accru_Curr[2] * ( I - currBasisGrid[4*pos+2] * tmp_ANN_Curr - tmp_PV_Curr );
			}

			tmp_a_Curr2 = - term_accru_Swap_Float[3] / term_accru_Curr[3] * ( 1. + currBasisGrid[4*pos+3] * term_accru_Curr[3] );
			if(!isRenAdj)
			{
				tmp_b_Curr2 = term_accru_Swap_Float[3] / term_accru_Curr[3] * ( 1. - term_accru_Curr[2] * ( currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2] ) );
				tmp_c_Curr2 = term_accru_Swap_Float[3] / term_accru_Curr[3] * ((H[3]-H[2]) - ( currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2] ) * tmp_ANN_Curr);
			}
			else
			{
				double tmp = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol) *
							  1. / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);

				tmp_b_Curr2 = term_accru_Swap_Float[3] / term_accru_Curr[3] * ( tmp - term_accru_Curr[2] * ( currBasisGrid[4*pos+2] - currBasisGrid[4*pos+2] ) );
				tmp_c_Curr2 = term_accru_Swap_Float[3] / term_accru_Curr[3] * ( - ( currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2] ) * tmp_ANN_Curr);
			}

			tmp_A = tmp_a_Swap - tmp_a_Curr2;
			tmp_B = - (tmp_b_Curr1 + tmp_b_Curr2);
			tmp_C = tmp_c_Swap - tmp_c_Curr1 - tmp_c_Curr2;

			tmp_Ad = 1 / ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] );
			tmp_Bd = - ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] + termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+2] ) / 
					( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] ) / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
			tmp_Cd = ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
					( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) * dfCurve[4*pos+1] -
					dfCurve[4*pos] / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );

			dfTerms.push_back( termGrid_3MRoll[4*pos+3] );
			dfCurve.push_back( (tmp_Ad * tmp_C - tmp_A * tmp_Cd) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );
			dfTerms.push_back( termGrid_3MRoll[4*pos+4] );
			dfCurve.push_back( (tmp_B * tmp_Cd - tmp_Bd * tmp_C) / ( tmp_A * tmp_Bd - tmp_Ad * tmp_B ) );

			rate = (tmp_c_Curr1 + tmp_b_Curr1 * dfCurve[4*pos+2] ) / (term_accru_3L6L_3L[2] * dfCurve[4*pos+2]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+2], termGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+3], libMkt.dc );

			rate = (tmp_c_Curr2 + tmp_b_Curr2 * dfCurve[4*pos+2] + tmp_a_Curr2 * dfCurve[4*pos+3]) / (term_accru_3L6L_3L[3] * dfCurve[4*pos+3]);
			insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
						termGrid_3MRoll[4*pos+3], termGrid_3MRoll[4*pos+4], dateGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+4], libMkt.dc );			
			
			tmp_PV_3L6L_3L  += term_accru_3L6L_3L[2] * threeMLRate[4*pos+2] * dfCurve[4*pos+2] + 
								term_accru_3L6L_3L[3] * threeMLRate[4*pos+3] * dfCurve[4*pos+3];
            tmp_ANN_3L6L_3L += term_accru_3L6L_3L[2] * dfCurve[4*pos+2] + term_accru_3L6L_3L[3] * dfCurve[4*pos+3];
            tmp_ANN_3L6L_6L += term_accru_3L6L_6L[1] * dfCurve[4*pos+3];
			rate = -(tmp_PV_3L6L_6L - tmp_ANN_3L6L_3L * threeSixBasisGrid[2*pos+1] - tmp_PV_3L6L_3L)/(term_accru_3L6L_6L[1] * dfCurve[4*pos+3]);
			insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
						termGrid_6MRoll[2*pos+1], termGrid_6MRoll[2*pos+2], dateGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos+2], libMkt.dc );
			if(isRenAdj)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+3] + spotTerm) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+4] + spotTerm) * 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+4] + spotTerm) / spotAdjust_dol - 
					  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol) *
					  dfCurve[4*pos+2] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+3] + spotTerm) / spotAdjust_dol);
			}
		}
	}
	else
	{
		double A1,A2,A3,B1,B2,B3,C1,C2,C3,D1,D2,D3,E1,E2,E3,F1,F2,F3,e1,e2,e3,e4,f1,f2,f3,f4,J1,J2,K1,K2;

		A1 = 1. / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		B1 = - ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos] ) / 
			   ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) / ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] );
		C1 = ( termGrid_3MRoll[4*pos] - termGrid_3MRoll[4*pos-1] + termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos-1] ) / 
			 ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] ) / ( termGrid_3MRoll[4*pos] - termGrid_3MRoll[4*pos-1] );
		D1 = - 1. / ( termGrid_3MRoll[4*pos] - termGrid_3MRoll[4*pos-1] );
		E1 = - B1 / A1;
		F1 = - (C1 * dfCurve[4*pos-1] + D1 * dfCurve[4*pos-2]) / A1;

		A2 = 1. / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
		B2 = - ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
			   ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		C2 = ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] + termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos] ) / 
			 ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] ) / ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] );
		D2 = - 1. / ( termGrid_3MRoll[4*pos+1] - termGrid_3MRoll[4*pos] );
		E2 = - (B2 * E1 + C2) / A2;
		F2 = - (B2 * F1 + D2 * dfCurve[4*pos-1]) / A2;

		A3 = 1. / ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] );
		B3 = - ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] + termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+2] ) / 
			   ( termGrid_3MRoll[4*pos+4] - termGrid_3MRoll[4*pos+3] ) / ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] );
		C3 = ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] + termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+1] ) / 
			 ( termGrid_3MRoll[4*pos+3] - termGrid_3MRoll[4*pos+2] ) / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		D3 = - 1. / ( termGrid_3MRoll[4*pos+2] - termGrid_3MRoll[4*pos+1] );
		E3 = - (B3 * E2 + C3 * E1 + D3) / A3;
		F3 = - (B3 * F2 + C3 * F1) / A3;

		e1 = - term_accru_3L6L_3L[0] / term_accru_Curr[0] * (1. + currBasisGrid[4*pos] * term_accru_Curr[0]);
		if (isRenAdj)
		{
			DoubleArray g(4);
			for (size_t i=1; i<4; i++)
			{
				g[i] = (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+i] + spotTerm) 
						/ xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1+i] + spotTerm) 
						* xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i+1] + spotTerm) / spotAdjust_dol 
						- xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol) 
						  / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol);
			}

			f1 = term_accru_3L6L_3L[0] / term_accru_Curr[0] * (I - PV_Curr - currBasisGrid[4*pos] * ANN_Curr);
			e2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				(1. - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * term_accru_Curr[0] 
					- (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * E1 + g[1]);
			f2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				( - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * ANN_Curr
				  - (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * F1);
			e3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				(- (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[0]
				 + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * E1
					- (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * E2 + g[2] * E1);
			f3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				( - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * ANN_Curr
				  + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * F1
				  - (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * F2 + g[2] * F1);
			e4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				(- (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[0]
				 - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * E1
				 + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * E2
					- (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * E3 + g[3] * E2);
			f4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				( - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * ANN_Curr
				  - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * F1
				  + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * F2
				  - (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * F3 + g[3] * F2);
		}
		else
		{
			f1 = term_accru_3L6L_3L[0] / term_accru_Curr[0] * (H[0] - PV_Curr - currBasisGrid[4*pos] * ANN_Curr);
			e2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				(1. - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * term_accru_Curr[0] 
					- (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * E1);
			f2 = term_accru_3L6L_3L[1] / term_accru_Curr[1] * 
				( H[1] - H[0] - (currBasisGrid[4*pos+1] - currBasisGrid[4*pos]) * ANN_Curr
				  - (1. + term_accru_Curr[1] * currBasisGrid[4*pos+1]) * F1);
			e3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				(- (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[0]
				 + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * E1
					- (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * E2);
			f3 = term_accru_3L6L_3L[2] / term_accru_Curr[2] * 
				( H[2] - H[1] - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * ANN_Curr
				  + (1. - (currBasisGrid[4*pos+2] - currBasisGrid[4*pos+1]) * term_accru_Curr[1]) * F1
				  - (1. + term_accru_Curr[2] * currBasisGrid[4*pos+2]) * F2);
			e4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				(- (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[0]
				 - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * E1
				 + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * E2
					- (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * E3);
			f4 = term_accru_3L6L_3L[3] / term_accru_Curr[3] * 
				( H[3] - H[2] - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * ANN_Curr
				  - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[1] * F1
				  + (1. - (currBasisGrid[4*pos+3] - currBasisGrid[4*pos+2]) * term_accru_Curr[2]) * F2
				  - (1. + term_accru_Curr[3] * currBasisGrid[4*pos+3]) * F3);
		}
		
		J1 = e1 + e2 + threeSixBasisGrid[2*pos] * term_accru_3L6L_3L[0] + threeSixBasisGrid[2*pos] * term_accru_3L6L_3L[1] * E1;
		K1 = - PV_3L6L_6L + PV_3L6L_3L + threeSixBasisGrid[2*pos] * ANN_3L6L_3L + f1 + f2 + threeSixBasisGrid[2*pos] * term_accru_3L6L_3L[1] * F1;
		J2 = (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * term_accru_3L6L_3L[0] 
			  + (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * term_accru_3L6L_3L[1] * E1 + e3 + e4
			  + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[2] * E2 + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[3] * E3;
		K2 = (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * ANN_3L6L_3L
			   + (threeSixBasisGrid[2*pos+1] - threeSixBasisGrid[2*pos]) * term_accru_3L6L_3L[1] * F1 + f3 + f4 
			   + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[2] * F2 + threeSixBasisGrid[2*pos+1] * term_accru_3L6L_3L[3] * F3;

		dfTerms.push_back( termGrid_3MRoll[4*pos+1] );
		dfCurve.push_back( (swapRateGrid[pos] * ANN_Swap - PV_Swap + swapRateGrid[pos] * term_accru_Swap_Fix * F3 - f4 - f3 - f2- f1) / 
						   (e1 + e2 + e3 + e4 - swapRateGrid[pos] * term_accru_Swap_Fix * E3) );
		rate = (e1 * dfCurve[4*pos] + f1) / (term_accru_3L6L_3L[0] * dfCurve[4*pos]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos], termGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos], dateGrid_3MRoll[4*pos+1], libMkt.dc );

		dfTerms.push_back( termGrid_3MRoll[4*pos+2] );
		dfCurve.push_back( E1 * dfCurve[4*pos] + F1 );
		rate = (e2 * dfCurve[4*pos] + f2) / (term_accru_3L6L_3L[1] * dfCurve[4*pos+1]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos+1], termGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+1], dateGrid_3MRoll[4*pos+2], libMkt.dc );

		dfTerms.push_back( termGrid_3MRoll[4*pos+3] );
		dfCurve.push_back(E2 * dfCurve[4*pos] + F2);
		rate = (e3 * dfCurve[4*pos] + f3) / (term_accru_3L6L_3L[2] * dfCurve[4*pos+2]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos+2], termGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+2], dateGrid_3MRoll[4*pos+3], libMkt.dc );

		dfTerms.push_back( termGrid_3MRoll[4*pos+4] );
		dfCurve.push_back(E3 * dfCurve[4*pos] + F3);
		rate = (e4 * dfCurve[4*pos] + f4) / (term_accru_3L6L_3L[3] * dfCurve[4*pos+3]);
		insertRate( rate, threeMLTermsMtx_Rate, threeMLRate, threeMLTerms_DF, threeMLDF, 
					termGrid_3MRoll[4*pos+3], termGrid_3MRoll[4*pos+4], dateGrid_3MRoll[4*pos+3], dateGrid_3MRoll[4*pos+4], libMkt.dc );

		rate = (J1 * dfCurve[4*pos] + K1) / (term_accru_3L6L_6L[0] * dfCurve[4*pos+1]);
        insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                    termGrid_6MRoll[2*pos], termGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos], dateGrid_6MRoll[2*pos+1], libMkt.dc );

		rate = (J2 * dfCurve[4*pos] + K2) / (term_accru_3L6L_6L[1] * dfCurve[4*pos+3]);
        insertRate( rate, sixMLTermsMtx_Rate, sixMLRate, sixMLTerms_DF, sixMLDF, 
                    termGrid_6MRoll[2*pos+1], termGrid_6MRoll[2*pos+2], dateGrid_6MRoll[2*pos+1], dateGrid_6MRoll[2*pos+2], libMkt.dc );
		
		if (isRenAdj)
		{
			for (size_t i=1; i<4; i++)
			{
				I += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+i] + spotTerm) 
					/ xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[4*pos+1+i] + spotTerm) 
					* xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i+1] + spotTerm) / spotAdjust_dol 
					- xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol) 
					* dfCurve[4*pos+i-1] / (xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[4*pos+i] + spotTerm) / spotAdjust_dol);
			}
		}
	}

	ANN_Swap += term_accru_Swap_Fix * dfCurve[4*pos+3];
	for (size_t i=0; i<4; i++)
	{
		ANN_Curr += term_accru_Curr[i] * dfCurve[4*pos+i];
		ANN_3L6L_3L += term_accru_3L6L_3L[i] * dfCurve[4*pos+i];
		PV_Swap += term_accru_Swap_Float[i] * threeMLRate[4*pos+i] * dfCurve[4*pos+i];
		PV_Curr += term_accru_Curr[i] * threeMLRate[4*pos+i] * dfCurve[4*pos+i];
		PV_3L6L_3L += term_accru_3L6L_3L[i] * threeMLRate[4*pos+i] * dfCurve[4*pos+i];
	}
	for (size_t i=0; i<2; i++)
	{
		ANN_3L6L_6L += term_accru_3L6L_6L[i] * dfCurve[4*pos+1+2*i];
		PV_3L6L_6L += term_accru_3L6L_6L[i] * sixMLRate[2*pos+i] * dfCurve[4*pos+1+2*i];
	}
}

/*
    @brief function to calculate Discount factor, forward 3M Libor and forward 6M Libor
		by newton raphson method
*/
void 
AQLPriceArbFreeGenerator::calcCurve_NewtonRaphson(	
	const AQLDate&  basedate, 
	DoubleArray& dfTerms, 
	DoubleArray& dfCurve, 
	DoubleArray& threeMLTerms_DF, 
	DoubleArray& threeMLDF, 
	DoubleArray& sixMLTerms_DF, 
	DoubleArray& sixMLDF, 
	bool isFRAUse, 
	bool isRenAdj, 
	const LiborMarket& libMkt, 
	const SwapMarket& swapMkt, 
	const XCCYBasisMarket& xccyBasisMkt, 
	const LiborBasisMarket& libBasisMkt, 
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	const DateVector& dateGrid_12MRoll,
	const DoubleArray& termGrid_12MRoll,
	AQLInterpolationBase* pInter_yg,
	bool isUSD)
{
	DoubleArray dfTerms_ori = dfTerms; dfTerms.clear();
	DoubleArray dfCurve_ori = dfCurve; dfCurve.clear();
	DoubleArray threeMLTerms_DF_ori = threeMLTerms_DF; threeMLTerms_DF.clear(); 
	DoubleArray threeMLDF_ori = threeMLDF; threeMLDF.clear();
	DoubleArray sixMLTerms_DF_ori = sixMLTerms_DF; sixMLTerms_DF.clear();
	DoubleArray sixMLDF_ori = sixMLDF; sixMLDF.clear();
	AQLInterpolationBase* pInter_3ML = dynamic_cast<AQLInterpolationBase*>(pInter_yg->clone());
	AQLInterpolationBase* pInter_6ML = dynamic_cast<AQLInterpolationBase*>(pInter_yg->clone());
	AQLInterpolationBase* pInter_DF = dynamic_cast<AQLInterpolationBase*>(pInter_yg->clone());
	pInter_3ML->set(threeMLTerms_DF_ori, threeMLDF_ori);
	pInter_6ML->set(sixMLTerms_DF_ori, sixMLDF_ori);
	pInter_DF->set(dfTerms_ori, dfCurve_ori);

	for (size_t i=0; dfTerms_ori[i]<xccyBasisMkt.mktTerms[0]; i++)
	{
		dfTerms.push_back(dfTerms_ori[i]);
		dfCurve.push_back(dfCurve_ori[i]);
	}

	if (swapMkt.freq_Float == SEMI_ANNUAL)
	{
		for (size_t i=0; threeMLTerms_DF_ori[i]<libBasisMkt.mktTerms[0]; i++)
		{
			threeMLTerms_DF.push_back(threeMLTerms_DF_ori[i]);
			threeMLDF.push_back(threeMLDF_ori[i]);
		}

		for (size_t i=0; sixMLTerms_DF_ori[i]<swapMkt.mktTerms[0]; i++)
		{
			sixMLTerms_DF.push_back(sixMLTerms_DF_ori[i]);
			sixMLDF.push_back(sixMLDF_ori[i]);
		}
	}
	else if (swapMkt.freq_Float == QUARTERLY)
	{
		for (size_t i=0; threeMLTerms_DF_ori[i]<swapMkt.mktTerms[0]; i++)
		{
			threeMLTerms_DF.push_back(threeMLTerms_DF_ori[i]);
			threeMLDF.push_back(threeMLDF_ori[i]);
		}

		for (size_t i=0; sixMLTerms_DF_ori[i]<libBasisMkt.mktTerms[0]; i++)
		{
			sixMLTerms_DF.push_back(sixMLTerms_DF_ori[i]);
			sixMLDF.push_back(sixMLDF_ori[i]);
		}
	}
	
	size_t shortTermSize_DF = dfTerms.size();
	size_t shortTermSize_6ML = sixMLTerms_DF.size();
	size_t shortTermSize_3ML = threeMLTerms_DF.size();

	for (size_t i=0; i<xccyBasisMkt.mktTerms.size(); i++)
	{
		dfTerms.push_back(xccyBasisMkt.mktTerms[i]);
		dfCurve.push_back(pInter_DF->value(xccyBasisMkt.mktTerms[i]));
	}
	
	if (swapMkt.freq_Float == SEMI_ANNUAL)
	{
		for (size_t i=0; i<libBasisMkt.mktTerms.size(); i++)
		{
			threeMLTerms_DF.push_back(libBasisMkt.mktTerms[i]);
			threeMLDF.push_back(pInter_3ML->value(libBasisMkt.mktTerms[i]));
		}

		for (size_t i=0; i<swapMkt.mktTerms.size(); i++)
		{
			sixMLTerms_DF.push_back(swapMkt.mktTerms[i]);
			sixMLDF.push_back(pInter_6ML->value(swapMkt.mktTerms[i]));
		}
	}
	else if (swapMkt.freq_Float == QUARTERLY)
	{
		for (size_t i=0; i<swapMkt.mktTerms.size(); i++)
		{
			threeMLTerms_DF.push_back(swapMkt.mktTerms[i]);
			threeMLDF.push_back(pInter_3ML->value(swapMkt.mktTerms[i]));
		}

		for (size_t i=0; i<libBasisMkt.mktTerms.size(); i++)
		{
			sixMLTerms_DF.push_back(libBasisMkt.mktTerms[i]);
			sixMLDF.push_back(pInter_6ML->value(libBasisMkt.mktTerms[i]));
		}
	}	
	else
	{
		throw AQLCoreInvalidData("Swap floating frequency is not supported!", __FILE__, __LINE__); 
	}

	size_t size_all = swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size() + xccyBasisMkt.mktTerms.size();

	const double EPS_PV = 1.0e-15;
	const double MIN_VAL = 1.0e-12;
	const unsigned int MAX_LOOP = 150;
	DoubleArray valVec0,valVec1,val;
	DoubleArray swapValue;
	
	pInter_3ML->set(threeMLTerms_DF, threeMLDF);
	pInter_6ML->set(sixMLTerms_DF, sixMLDF);
	pInter_DF->set(dfTerms, dfCurve);
	swapValue = getSwapValue(libMkt,swapMkt,dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll,
		dateGrid_12MRoll, termGrid_12MRoll,pInter_3ML,pInter_6ML,pInter_DF);
	valVec1.insert(valVec1.begin(), swapValue.begin(), swapValue.end());
	swapValue = getLibBasisValue(libMkt,swapMkt,libBasisMkt,dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll,
		pInter_3ML,pInter_6ML,pInter_DF);
	valVec1.insert(valVec1.begin() + swapMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );
	swapValue = getXccyBasisValue(basedate,isRenAdj,libMkt,swapMkt,xccyBasisMkt,dateGrid_3MRoll, termGrid_3MRoll, 
		pInter_3ML,pInter_6ML,pInter_DF, isUSD);
	valVec1.insert(valVec1.begin() + swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );

	int pos = 0;

	DoubleArray sixMLDF_div, threeMLDF_div, dfCurve_div;
	int loopNum = MAX_LOOP;
	while (loopNum--)
	{
		bool isEnd = true;
		for (unsigned int i = 0; i < size_all; ++i)
		{
			//if (AQLMath::abs(valVec0[i] - valVec1[i]) >= EPS_PV)
			if (AQLMath::abs(valVec1[i]) > 1.0E-9)
			{
				isEnd = false;
				break;
			}
		}
		if (isEnd)
		{
			break;
		}

		AQLMatrix divMat(size_all, size_all);
		// create divMat
		sixMLDF_div = sixMLDF, threeMLDF_div = threeMLDF, dfCurve_div = dfCurve;
		for (unsigned int j = 0; j < size_all; ++j)
		{
			if ( j < swapMkt.mktTerms.size() )
			{
				if (swapMkt.freq_Float == SEMI_ANNUAL)
				{
					pos = shortTermSize_6ML + j;
					sixMLDF_div[pos] = sixMLDF[pos] + MIN_VAL;
				}
				else if (swapMkt.freq_Float == QUARTERLY)
				{
					pos = shortTermSize_3ML + j;
					threeMLDF_div[pos] = threeMLDF[pos] + MIN_VAL;
				}	
			}
			else if ( j < swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size())
			{
				if (swapMkt.freq_Float == SEMI_ANNUAL)
				{
					pos = shortTermSize_3ML + j - swapMkt.mktTerms.size();
					threeMLDF_div[pos] = threeMLDF[pos] + MIN_VAL;
				}
				else if (swapMkt.freq_Float == QUARTERLY)
				{
					pos = shortTermSize_6ML + j - swapMkt.mktTerms.size();
					sixMLDF_div[pos] = sixMLDF[pos] + MIN_VAL;
				}	
			}
			else
			{
				pos = shortTermSize_DF + j - swapMkt.mktTerms.size() - libBasisMkt.mktTerms.size();
				dfCurve_div[pos] = dfCurve[pos] + MIN_VAL;
			}	

			pInter_3ML->set(threeMLTerms_DF, threeMLDF_div);
			pInter_6ML->set(sixMLTerms_DF, sixMLDF_div);
			pInter_DF->set(dfTerms, dfCurve_div);
			val.clear();
			swapValue = getSwapValue(libMkt,swapMkt,dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll,
				dateGrid_12MRoll, termGrid_12MRoll,pInter_3ML,pInter_6ML,pInter_DF);
			val.insert(val.begin(), swapValue.begin(), swapValue.end());
			swapValue = getLibBasisValue(libMkt,swapMkt,libBasisMkt,dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll,
				pInter_3ML,pInter_6ML,pInter_DF);
			val.insert(val.begin() + swapMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );
			swapValue = getXccyBasisValue(basedate,isRenAdj,libMkt,swapMkt,xccyBasisMkt,dateGrid_3MRoll, termGrid_3MRoll, 
				pInter_3ML,pInter_6ML,pInter_DF, isUSD);
			val.insert(val.begin() + swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );

			for (unsigned int i = 0; i < size_all; ++i)
			{
				divMat.setValue(i, j, (val[i] - valVec1[i]) / MIN_VAL);			
			}
		}

		AQLMatrix invMat = divMat.inverseMatrix();
		AQLMatrix valMat(valVec1);
		AQLMatrix deltaMat = invMat * valMat;
		// plus delta
		for (unsigned int i = 0; i < size_all; ++i)
		{
			if ( i < swapMkt.mktTerms.size() )
			{
				if (swapMkt.freq_Float == SEMI_ANNUAL)
				{
					pos = shortTermSize_6ML + i;
					sixMLDF[pos] -= deltaMat.getValue(i, 0);
				}
				else if (swapMkt.freq_Float == QUARTERLY)
				{
					pos = shortTermSize_3ML + i;
					threeMLDF[pos] -= deltaMat.getValue(i, 0);
				}	
			}
			else if ( i < swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size())
			{
				if (swapMkt.freq_Float == SEMI_ANNUAL)
				{
					pos = shortTermSize_3ML + i - swapMkt.mktTerms.size();
					threeMLDF[pos] -= deltaMat.getValue(i, 0);
				}
				else if (swapMkt.freq_Float == QUARTERLY)
				{
					pos = shortTermSize_6ML + i - swapMkt.mktTerms.size();
					sixMLDF[pos] -= deltaMat.getValue(i, 0);
				}	
			}
			else
			{
				pos = shortTermSize_DF + i - swapMkt.mktTerms.size() - libBasisMkt.mktTerms.size();
				dfCurve[pos] -= deltaMat.getValue(i, 0);
			}	
		}

		pInter_3ML->set(threeMLTerms_DF, threeMLDF);
		pInter_6ML->set(sixMLTerms_DF, sixMLDF);
		pInter_DF->set(dfTerms, dfCurve);
		valVec1.clear();
		swapValue = getSwapValue(libMkt,swapMkt,dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll,
			dateGrid_12MRoll, termGrid_12MRoll,pInter_3ML,pInter_6ML,pInter_DF);
		valVec1.insert(valVec1.begin(), swapValue.begin(), swapValue.end());
		swapValue = getLibBasisValue(libMkt,swapMkt,libBasisMkt,dateGrid_3MRoll, termGrid_3MRoll, dateGrid_6MRoll, termGrid_6MRoll,
			pInter_3ML,pInter_6ML,pInter_DF);
		valVec1.insert(valVec1.begin() + swapMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );
		swapValue = getXccyBasisValue(basedate,isRenAdj,libMkt,swapMkt,xccyBasisMkt,dateGrid_3MRoll, termGrid_3MRoll, 
			pInter_3ML,pInter_6ML,pInter_DF, isUSD);
		valVec1.insert(valVec1.begin() + swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );
	}

	if (loopNum < 0)
	{
		for (unsigned int i = 0; i < size_all; ++i)
		{
			// check error is within 0.00001bp
			if (AQLMath::abs(valVec1[i]) > 1.0E-9)
			{
				throw AQLCoreInvalidData("Convergence error in DF calc (Newton Raphson)", __FILE__, __LINE__); 
			}
		}
	}

	pInter_3ML->set(threeMLTerms_DF, threeMLDF);
	pInter_6ML->set(sixMLTerms_DF, sixMLDF);
	pInter_DF->set(dfTerms, dfCurve);

	dfTerms.clear(); dfCurve.clear();
	for (size_t i=0; i<dfTerms_ori.size(); i++)
	{
		dfTerms.push_back(dfTerms_ori[i]);
		dfCurve.push_back(pInter_DF->value(dfTerms_ori[i]));
	}

	threeMLTerms_DF.clear(); threeMLDF.clear();
	for (size_t i=0; i<threeMLTerms_DF_ori.size(); i++)
	{
		threeMLTerms_DF.push_back(threeMLTerms_DF_ori[i]);
		threeMLDF.push_back(pInter_3ML->value(threeMLTerms_DF_ori[i]));
	}

	sixMLTerms_DF.clear(); sixMLDF.clear();
	for (size_t i=0; i<sixMLTerms_DF_ori.size(); i++)
	{
		sixMLTerms_DF.push_back(sixMLTerms_DF_ori[i]);
		sixMLDF.push_back(pInter_6ML->value(sixMLTerms_DF_ori[i]));
	}

	delete pInter_DF;
	delete pInter_6ML;
	delete pInter_3ML;
}

/*
    @brief function to calculate Discount factor, forward 3M Libor and forward 6M Libor
		by NL2SOL method
*/
void 
AQLPriceArbFreeGenerator::calcCurve_NL2SOL(	
	const AQLDate&  basedate, 
	DoubleArray& dfTerms, 
	DoubleArray& dfCurve, 
	DoubleMatrix& threeMLTermsMtx_Rate,
	DoubleArray& threeMLRate,
	DoubleArray& threeMLTerms_DF, 
	DoubleArray& threeMLDF, 
	DoubleMatrix& sixMLTermsMtx_Rate,
	DoubleArray& sixMLRate,
	DoubleArray& sixMLTerms_DF, 
	DoubleArray& sixMLDF, 
	bool isFRAUse, 
	bool isRenAdj, 
	const LiborMarket& libMkt, 
	const SwapMarket& swapMkt, 
	const XCCYBasisMarket& xccyBasisMkt, 
	const LiborBasisMarket& libBasisMkt, 
	AQLInterpolationBase* pInter_yg,
	bool isUSD)
{
	DoubleArray dfTerms_ori = dfTerms; dfTerms.clear();
	DoubleArray dfCurve_ori = dfCurve; dfCurve.clear();
	DoubleArray threeMLTerms_DF_ori = threeMLTerms_DF; threeMLTerms_DF.clear(); 
	DoubleArray threeMLDF_ori = threeMLDF; threeMLDF.clear();
	DoubleArray sixMLTerms_DF_ori = sixMLTerms_DF; sixMLTerms_DF.clear();
	DoubleArray sixMLDF_ori = sixMLDF; sixMLDF.clear();
	AQLInterpolationBase* pInter_3ML = dynamic_cast<AQLInterpolationBase*>(pInter_yg->clone());
	AQLInterpolationBase* pInter_6ML = dynamic_cast<AQLInterpolationBase*>(pInter_yg->clone());
	AQLInterpolationBase* pInter_DF = dynamic_cast<AQLInterpolationBase*>(pInter_yg->clone());
	pInter_3ML->set(threeMLTerms_DF_ori, threeMLDF_ori);
	pInter_6ML->set(sixMLTerms_DF_ori, sixMLDF_ori);
	pInter_DF->set(dfTerms_ori, dfCurve_ori);

	for (size_t i=0; dfTerms_ori[i]<xccyBasisMkt.mktTerms[0] - eps; i++)
	{
		dfTerms.push_back(dfTerms_ori[i]);
		dfCurve.push_back(dfCurve_ori[i]);
	}

	if (swapMkt.freq_Float == SEMI_ANNUAL)
	{
		for (size_t i=0; threeMLTerms_DF_ori[i]<libBasisMkt.mktTerms[0] - eps; i++)
		{
			threeMLTerms_DF.push_back(threeMLTerms_DF_ori[i]);
			threeMLDF.push_back(threeMLDF_ori[i]);
		}

		for (size_t i=0; sixMLTerms_DF_ori[i]<swapMkt.mktTerms[0] - eps; i++)
		{
			sixMLTerms_DF.push_back(sixMLTerms_DF_ori[i]);
			sixMLDF.push_back(sixMLDF_ori[i]);
		}
	}
	else if (swapMkt.freq_Float == QUARTERLY)
	{
		for (size_t i=0; threeMLTerms_DF_ori[i]<swapMkt.mktTerms[0] - eps; i++)
		{
			threeMLTerms_DF.push_back(threeMLTerms_DF_ori[i]);
			threeMLDF.push_back(threeMLDF_ori[i]);
		}

		for (size_t i=0; sixMLTerms_DF_ori[i]<libBasisMkt.mktTerms[0] - eps; i++)
		{
			sixMLTerms_DF.push_back(sixMLTerms_DF_ori[i]);
			sixMLDF.push_back(sixMLDF_ori[i]);
		}
	}
	
	size_t shortTermSize_DF = dfTerms.size();
	size_t shortTermSize_6ML = sixMLTerms_DF.size();
	size_t shortTermSize_3ML = threeMLTerms_DF.size();

	DoubleArray val;
	
	if (swapMkt.freq_Float == SEMI_ANNUAL)
	{
		for (size_t i=0; i<swapMkt.mktTerms.size(); i++)
		{
			sixMLTerms_DF.push_back(swapMkt.mktTerms[i]);
			val.push_back(AQLMath::max(pInter_6ML->value(swapMkt.mktTerms[i]), MIN_DF));
			sixMLDF.push_back(val[i]);
		}

		for (size_t i=0; i<libBasisMkt.mktTerms.size(); i++)
		{
			threeMLTerms_DF.push_back(libBasisMkt.mktTerms[i]);
			val.push_back(AQLMath::max(pInter_3ML->value(libBasisMkt.mktTerms[i]), MIN_DF));
			threeMLDF.push_back(val[i + swapMkt.mktTerms.size()]);
		}

	}
	else if (swapMkt.freq_Float == QUARTERLY)
	{
		for (size_t i=0; i<swapMkt.mktTerms.size(); i++)
		{
			threeMLTerms_DF.push_back(swapMkt.mktTerms[i]);
			val.push_back(AQLMath::max(pInter_3ML->value(swapMkt.mktTerms[i]), MIN_DF));
			threeMLDF.push_back(val[i]);
		}

		for (size_t i=0; i<libBasisMkt.mktTerms.size(); i++)
		{
			sixMLTerms_DF.push_back(libBasisMkt.mktTerms[i]);
			val.push_back(AQLMath::max(pInter_6ML->value(libBasisMkt.mktTerms[i]), MIN_DF));
			sixMLDF.push_back(val[i + swapMkt.mktTerms.size()]);
		}
	}	
	else
	{
		throw AQLCoreInvalidData("Swap floating frequency is not supported!", __FILE__, __LINE__); 
	}

	for (size_t i=0; i<xccyBasisMkt.mktTerms.size(); i++)
	{
		dfTerms.push_back(xccyBasisMkt.mktTerms[i]);
		val.push_back(AQLMath::max(pInter_DF->value(xccyBasisMkt.mktTerms[i]), MIN_DF));
		dfCurve.push_back(val[i + swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size()]);
	}

	//calibration
    AQLMathAFCurveCalibrator calibrator(basedate, threeMLTerms_DF, threeMLDF, sixMLTerms_DF, sixMLDF, dfTerms, dfCurve, isRenAdj, 
		libMkt, swapMkt, xccyBasisMkt, libBasisMkt, pInter_3ML, pInter_6ML, pInter_DF, shortTermSize_3ML, shortTermSize_6ML,
		shortTermSize_DF, isUSD);
    NL2SOL solver( calibrator );
    solver.tryToSolve( val );

	unsigned int pos = 0;

	for (size_t i = 0; i < val.size(); ++i)
	{
		if ( i < swapMkt.mktTerms.size() )
		{
			if (swapMkt.freq_Float == SEMI_ANNUAL)
			{
				pos = shortTermSize_6ML + i;
				sixMLDF[pos] = val[i];
			}
			else if (swapMkt.freq_Float == QUARTERLY)
			{
				pos = shortTermSize_3ML + i;
				threeMLDF[pos] = val[i];
			}	
		}
		else if ( i < swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size())
		{
			if (swapMkt.freq_Float == SEMI_ANNUAL)
			{
				pos = shortTermSize_3ML + i - swapMkt.mktTerms.size();
				threeMLDF[pos] = val[i];
			}
			else if (swapMkt.freq_Float == QUARTERLY)
			{
				pos = shortTermSize_6ML + i - swapMkt.mktTerms.size();
				sixMLDF[pos] = val[i];
			}	
		}
		else
		{
			pos = shortTermSize_DF + i - swapMkt.mktTerms.size() - libBasisMkt.mktTerms.size();
			dfCurve[pos] = val[i];
		}
	}

	pInter_3ML->set(threeMLTerms_DF, threeMLDF);
	pInter_6ML->set(sixMLTerms_DF, sixMLDF);
	pInter_DF->set(dfTerms, dfCurve);
	

	dfTerms.clear(); dfCurve.clear();
	threeMLTerms_DF.clear(); threeMLDF.clear();
	sixMLTerms_DF.clear(); sixMLDF.clear();
	for (size_t i=0; i<shortTermSize_DF; i++)
	{
		dfTerms.push_back(dfTerms_ori[i]);
		dfCurve.push_back(AQLMath::max(pInter_DF->value(dfTerms_ori[i]), MIN_DF));
	}
	for (size_t i=0; i<shortTermSize_3ML; i++)
	{
		threeMLTerms_DF.push_back(threeMLTerms_DF_ori[i]);
		threeMLDF.push_back(AQLMath::max(pInter_3ML->value(threeMLTerms_DF_ori[i]), MIN_DF));
	}
	for (size_t i=0; i<shortTermSize_6ML; i++)
	{
		sixMLTerms_DF.push_back(sixMLTerms_DF_ori[i]);
		sixMLDF.push_back(AQLMath::max(pInter_6ML->value(sixMLTerms_DF_ori[i]), MIN_DF));
	}

	double df, rate, accruTerm;
	//insert terms and dfs until the first term of swap
	if (swapMkt.freq_Float == SEMI_ANNUAL)
	{
		for (size_t i=0; i<libBasisMkt.termGrid_3MRoll.size(); i++)
		{
			if (threeMLTerms_DF_ori[shortTermSize_3ML] <= libBasisMkt.termGrid_3MRoll[i] + eps) continue;

			for (pos=1; pos<threeMLTerms_DF.size(); pos++)
			{
				if (libBasisMkt.termGrid_3MRoll[i] < threeMLTerms_DF[pos]) break;
			}

			if (AQLMath::abs(threeMLTerms_DF[pos-1] - libBasisMkt.termGrid_3MRoll[i]) > eps && 
				(pos>=threeMLTerms_DF.size() || AQLMath::abs(threeMLTerms_DF[pos] - libBasisMkt.termGrid_3MRoll[i]) > eps))
			{
				df = AQLMath::max(pInter_3ML->value(libBasisMkt.termGrid_3MRoll[i]), MIN_DF);
				threeMLTerms_DF.insert(threeMLTerms_DF.begin() + pos, libBasisMkt.termGrid_3MRoll[i]);
				threeMLDF.insert(threeMLDF.begin() + pos, df);

				accruTerm = libMkt.dc.getTerm(libBasisMkt.dateGrid_3MRoll[i], libBasisMkt.dateGrid_3MRoll[i+1], false);
				rate = (pInter_3ML->value(libBasisMkt.termGrid_3MRoll[i]) / pInter_3ML->value(libBasisMkt.termGrid_3MRoll[i+1]) - 1.) / accruTerm;
				if (!getPositionOfVector(libBasisMkt.termGrid_3MRoll[i], threeMLTermsMtx_Rate[0], eps, pos))
				{
					insertToVector(libBasisMkt.termGrid_3MRoll[i], rate, threeMLTermsMtx_Rate[0], threeMLRate, pos);
					threeMLTermsMtx_Rate[1].insert(threeMLTermsMtx_Rate[1].begin() + pos, libBasisMkt.termGrid_3MRoll[i+1]);
				}
			}
		}
		for (size_t i=0; i<swapMkt.termGrid_6MRoll.size(); i++)
		{
			if (sixMLTerms_DF_ori[shortTermSize_6ML] <= swapMkt.termGrid_6MRoll[i] + eps) continue;

			for (pos=1; pos<sixMLTerms_DF.size(); pos++)
			{
				if (swapMkt.termGrid_6MRoll[i] < sixMLTerms_DF[pos]) break;
			}

			if (AQLMath::abs(sixMLTerms_DF[pos-1] - swapMkt.termGrid_6MRoll[i]) > eps && 
				(pos>=sixMLTerms_DF.size() || AQLMath::abs(sixMLTerms_DF[pos] - swapMkt.termGrid_6MRoll[i]) > eps))
			{
				df = AQLMath::max(pInter_6ML->value(swapMkt.termGrid_6MRoll[i]), MIN_DF);
				sixMLTerms_DF.insert(sixMLTerms_DF.begin() + pos, swapMkt.termGrid_6MRoll[i]);
				sixMLDF.insert(sixMLDF.begin() + pos, df);

				accruTerm = libMkt.dc.getTerm(swapMkt.dateGrid_6MRoll[i], swapMkt.dateGrid_6MRoll[i+1], false);
				rate = (pInter_6ML->value(swapMkt.termGrid_6MRoll[i]) / pInter_6ML->value(swapMkt.termGrid_6MRoll[i+1]) - 1.) / accruTerm;
				if (!getPositionOfVector(swapMkt.termGrid_6MRoll[i], sixMLTermsMtx_Rate[0], eps, pos))
				{
					insertToVector(swapMkt.termGrid_6MRoll[i], rate, sixMLTermsMtx_Rate[0], sixMLRate, pos);
					sixMLTermsMtx_Rate[1].insert(sixMLTermsMtx_Rate[1].begin() + pos, swapMkt.termGrid_6MRoll[i+1]);
				}
			}
		}
	}
	else if (swapMkt.freq_Float == QUARTERLY)
	{
		for (size_t i=0; i<swapMkt.termGrid_3MRoll.size(); i++)
		{
			if (threeMLTerms_DF_ori[shortTermSize_3ML] <= swapMkt.termGrid_3MRoll[i] + eps) continue;

			for (pos=1; pos<threeMLTerms_DF.size(); pos++)
			{
				if (swapMkt.termGrid_3MRoll[i] < threeMLTerms_DF[pos]) break;
			}

			if (AQLMath::abs(threeMLTerms_DF[pos-1] - swapMkt.termGrid_3MRoll[i]) > eps && 
				(pos>=threeMLTerms_DF.size() || AQLMath::abs(threeMLTerms_DF[pos] - swapMkt.termGrid_3MRoll[i]) > eps))
			{
				df = AQLMath::max(pInter_3ML->value(swapMkt.termGrid_3MRoll[i]), MIN_DF);
				threeMLTerms_DF.insert(threeMLTerms_DF.begin() + pos, swapMkt.termGrid_3MRoll[i]);
				threeMLDF.insert(threeMLDF.begin() + pos, df);

				accruTerm = libMkt.dc.getTerm(swapMkt.dateGrid_3MRoll[i], swapMkt.dateGrid_3MRoll[i+1], false);
				rate = (pInter_3ML->value(swapMkt.termGrid_3MRoll[i]) / pInter_3ML->value(swapMkt.termGrid_3MRoll[i+1]) - 1.) / accruTerm;
				if (!getPositionOfVector(swapMkt.termGrid_3MRoll[i], threeMLTermsMtx_Rate[0], eps, pos))
				{
					insertToVector(swapMkt.termGrid_3MRoll[i], rate, threeMLTermsMtx_Rate[0], threeMLRate, pos);
					threeMLTermsMtx_Rate[1].insert(threeMLTermsMtx_Rate[1].begin() + pos, swapMkt.termGrid_3MRoll[i+1]);
				}
			}
		}
		for (size_t i=0; i<libBasisMkt.termGrid_6MRoll.size(); i++)
		{
			if (sixMLTerms_DF_ori[shortTermSize_6ML] <= libBasisMkt.termGrid_6MRoll[i] + eps) continue;

			for (pos=1; pos<sixMLTerms_DF.size(); pos++)
			{
				if (libBasisMkt.termGrid_6MRoll[i] < sixMLTerms_DF[pos]) break;
			}

			if (AQLMath::abs(sixMLTerms_DF[pos-1] - libBasisMkt.termGrid_6MRoll[i]) > eps && 
				(pos>=sixMLTerms_DF.size() || AQLMath::abs(sixMLTerms_DF[pos] - libBasisMkt.termGrid_6MRoll[i]) > eps))
			{
				df = AQLMath::max(pInter_6ML->value(libBasisMkt.termGrid_6MRoll[i]), MIN_DF);
				sixMLTerms_DF.insert(sixMLTerms_DF.begin() + pos, libBasisMkt.termGrid_6MRoll[i]);
				sixMLDF.insert(sixMLDF.begin() + pos, df);

				accruTerm = libMkt.dc.getTerm(libBasisMkt.dateGrid_6MRoll[i], libBasisMkt.dateGrid_6MRoll[i+1], false);
				rate = (pInter_6ML->value(libBasisMkt.termGrid_6MRoll[i]) / pInter_6ML->value(libBasisMkt.termGrid_6MRoll[i+1]) - 1.) / accruTerm;
				if (!getPositionOfVector(libBasisMkt.termGrid_6MRoll[i], sixMLTermsMtx_Rate[0], eps, pos))
				{
					insertToVector(libBasisMkt.termGrid_6MRoll[i], rate, sixMLTermsMtx_Rate[0], sixMLRate, pos);
					sixMLTermsMtx_Rate[1].insert(sixMLTermsMtx_Rate[1].begin() + pos, libBasisMkt.termGrid_6MRoll[i+1]);
				}
			}
		}
	}

	for (size_t i=0; i<xccyBasisMkt.termGrid_3MRoll.size(); i++)
	{
		if (dfTerms_ori[shortTermSize_DF-1] >= xccyBasisMkt.termGrid_3MRoll[i] - eps) continue;
		dfTerms.push_back(xccyBasisMkt.termGrid_3MRoll[i]);
		dfCurve.push_back(AQLMath::max(pInter_DF->value(xccyBasisMkt.termGrid_3MRoll[i]), MIN_DF));
	}
	if (swapMkt.freq_Float == SEMI_ANNUAL)
	{
		for (size_t i=0; i<libBasisMkt.termGrid_3MRoll.size(); i++)
		{
			if (threeMLTerms_DF_ori[shortTermSize_3ML] >= libBasisMkt.termGrid_3MRoll[i] + eps) continue;
			threeMLTerms_DF.push_back(libBasisMkt.termGrid_3MRoll[i]);
			threeMLDF.push_back(AQLMath::max(pInter_3ML->value(libBasisMkt.termGrid_3MRoll[i]), MIN_DF));
		}
		for (size_t i=0; i<swapMkt.termGrid_6MRoll.size(); i++)
		{
			if (sixMLTerms_DF_ori[shortTermSize_6ML] >= swapMkt.termGrid_6MRoll[i] + eps) continue;
			sixMLTerms_DF.push_back(swapMkt.termGrid_6MRoll[i]);
			sixMLDF.push_back(AQLMath::max(pInter_6ML->value(swapMkt.termGrid_6MRoll[i]), MIN_DF));
		}
	}
	else if (swapMkt.freq_Float == QUARTERLY)
	{
		for (size_t i=0; i<swapMkt.termGrid_3MRoll.size(); i++)
		{
			if (threeMLTerms_DF_ori[shortTermSize_3ML] >= swapMkt.termGrid_3MRoll[i] + eps) continue;
			threeMLTerms_DF.push_back(swapMkt.termGrid_3MRoll[i]);
			threeMLDF.push_back(AQLMath::max(pInter_3ML->value(swapMkt.termGrid_3MRoll[i]), MIN_DF));
		}
		for (size_t i=0; i<libBasisMkt.termGrid_6MRoll.size(); i++)
		{
			if (sixMLTerms_DF_ori[shortTermSize_6ML] >= libBasisMkt.termGrid_6MRoll[i] + eps) continue;
			sixMLTerms_DF.push_back(libBasisMkt.termGrid_6MRoll[i]);
			sixMLDF.push_back(AQLMath::max(pInter_6ML->value(libBasisMkt.termGrid_6MRoll[i]), MIN_DF));
		}
	}

	delete pInter_DF;
	delete pInter_6ML;
	delete pInter_3ML;
}

DoubleVector 
AQLPriceArbFreeGenerator::getSwapValue(	
	const LiborMarket& libMkt, 
	const SwapMarket& swapMkt, 
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	const DateVector& dateGrid_12MRoll,
	const DoubleArray& termGrid_12MRoll,
	AQLInterpolationBase* pInter_3ML,
	AQLInterpolationBase* pInter_6ML,
	AQLInterpolationBase* pInter_DF )
{
	size_t dateSize_3M = termGrid_3MRoll.size();
    size_t dateSize_6M = termGrid_6MRoll.size();
	size_t dateSize_12M = termGrid_12MRoll.size();
	double ANN_Swap=0.,ANN_Swap_irre=0.,PV_Swap=0.;
	double term_accru = 0.,term_accru_lib = 0.,forward = 0.,df = 0.;

	////set frequency information
	DateVector dates_float_payment, dates_float_index;
	DoubleArray terms_float_payment, terms_float_index;
	AQLInterpolationBase* pInter_float;
	//payment of floating leg
	if (swapMkt.freq_Float_Pay == ANNUAL)
	{
		dates_float_payment = dateGrid_12MRoll;
		terms_float_payment = termGrid_12MRoll;
	}
	else if (swapMkt.freq_Float_Pay == SEMI_ANNUAL)
	{
		dates_float_payment = dateGrid_6MRoll;
		terms_float_payment = termGrid_6MRoll;
	}
	else if (swapMkt.freq_Float_Pay == QUARTERLY)
	{
		dates_float_payment = dateGrid_3MRoll;
		terms_float_payment = termGrid_3MRoll;
	}
	//index of floating leg
	if (swapMkt.freq_Float == SEMI_ANNUAL)
	{
		dates_float_index = dateGrid_6MRoll;
		terms_float_index = termGrid_6MRoll;
		pInter_float = pInter_6ML;
	}
	else if (swapMkt.freq_Float == QUARTERLY)
	{
		dates_float_index = dateGrid_3MRoll;
		terms_float_index = termGrid_3MRoll;
		pInter_float = pInter_3ML;
	}

	DoubleVector ret(swapMkt.mktTerms.size());
	size_t pos_float = 0,pos_float_index = 0,pos_fix = 0,pos_fix_irre = 0;
	AQLString frequencyFix = "";
	for (unsigned int i = 0; i < swapMkt.mktTerms.size(); ++i)
	{
		AQLDate endDate = AQLDateCalculations::getDate(swapMkt.spotDate, swapMkt.mktTerms_str[i], swapMkt.sld, swapMkt.pCal, true, &swapMkt.roll_conv);

		for (; dates_float_payment[pos_float+1] <= endDate; ++pos_float)
		{
			double cashflow = 1.;
			for (; dates_float_index[pos_float_index+1] <= dates_float_payment[pos_float+1]; ++pos_float_index)
			{
				term_accru = swapMkt.dc_Float.getTerm(dates_float_index[pos_float_index], dates_float_index[pos_float_index+1], false);
				term_accru_lib = libMkt.dc.getTerm(dates_float_index[pos_float_index], dates_float_index[pos_float_index+1], false);
				forward = (pInter_float->value(terms_float_index[pos_float_index]) / pInter_float->value(terms_float_index[pos_float_index+1]) - 1.) / term_accru_lib;
				
				cashflow *= (1. + term_accru * forward);
				if (pos_float_index+2 == dates_float_index.size()) break;
			}
			cashflow -= 1.;
			df = pInter_DF->value(terms_float_payment[pos_float+1]);
			PV_Swap += cashflow * df;

			if (pos_float+2 == dates_float_payment.size()) break;
		}

		if (swapMkt.freq_Fix == SEMI_ANNUAL)
		{
			for (; dateGrid_6MRoll[pos_fix+1] <= endDate; ++pos_fix)
			{
				term_accru = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[pos_fix], dateGrid_6MRoll[pos_fix+1], false);
				df = pInter_DF->value(termGrid_6MRoll[pos_fix+1]);
				ANN_Swap += term_accru * df;

				if (pos_fix+2 == dateGrid_6MRoll.size()) break;
			}

			frequencyFix = getSwapFixFrequency(6*pos_fix, swapMkt);
		}
		else if (swapMkt.freq_Fix == QUARTERLY)
		{
			for (; dateGrid_3MRoll[pos_fix+1] <= endDate; ++pos_fix)
			{
				term_accru = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[pos_fix], dateGrid_3MRoll[pos_fix+1], false);
				df = pInter_DF->value(termGrid_3MRoll[pos_fix+1]);
				ANN_Swap += term_accru * df;

				if (pos_fix+2 == dateGrid_3MRoll.size()) break;
			}

			frequencyFix = getSwapFixFrequency(3*pos_fix, swapMkt);
		}		
		else if (swapMkt.freq_Fix == ANNUAL)
		{
			for (; dateGrid_12MRoll[pos_fix+1] <= endDate; ++pos_fix)
			{
				term_accru = swapMkt.dc_Fix.getTerm(dateGrid_12MRoll[pos_fix], dateGrid_12MRoll[pos_fix+1], false);
				df = pInter_DF->value(termGrid_12MRoll[pos_fix+1]);
				ANN_Swap += term_accru * df;

				if (pos_fix+2 == dateGrid_12MRoll.size()) break;
			}

			frequencyFix = getSwapFixFrequency(12*pos_fix, swapMkt);
		}				

		if (swapMkt.isSwapTenorAdjust)
		{
			if (frequencyFix == SEMI_ANNUAL)
			{
				for (; dateGrid_6MRoll[pos_fix_irre+1] <= endDate; ++pos_fix_irre)
				{
					term_accru = swapMkt.dc_Fix.getTerm(dateGrid_6MRoll[pos_fix_irre], dateGrid_6MRoll[pos_fix_irre+1], false);
					df = pInter_DF->value(termGrid_6MRoll[pos_fix_irre+1]);
					ANN_Swap_irre += term_accru * df;

					if (pos_fix_irre+2 == dateGrid_6MRoll.size()) break;
				}
			}
			else if (frequencyFix == QUARTERLY)
			{
				for (; dateGrid_3MRoll[pos_fix_irre+1] <= endDate; ++pos_fix_irre)
				{
					term_accru = swapMkt.dc_Fix.getTerm(dateGrid_3MRoll[pos_fix_irre], dateGrid_3MRoll[pos_fix_irre+1], false);
					df = pInter_DF->value(termGrid_3MRoll[pos_fix_irre+1]);
					ANN_Swap_irre += term_accru * df;

					if (pos_fix_irre+2 == dateGrid_3MRoll.size()) break;
				}
			}		
			else if (frequencyFix == ANNUAL)
			{
				for (; dateGrid_12MRoll[pos_fix_irre+1] <= endDate; ++pos_fix_irre)
				{
					term_accru = swapMkt.dc_Fix.getTerm(dateGrid_12MRoll[pos_fix_irre], dateGrid_12MRoll[pos_fix_irre+1], false);
					df = pInter_DF->value(termGrid_12MRoll[pos_fix_irre+1]);
					ANN_Swap_irre += term_accru * df;

					if (pos_fix_irre+2 == dateGrid_12MRoll.size()) break;
				}
			}
		}

		if (frequencyFix == swapMkt.freq_Fix)
		{
			ret[i] = ANN_Swap * swapMkt.mktRates[i] - PV_Swap;
		}
		else
		{
			ret[i] = ANN_Swap_irre * swapMkt.mktRates[i] - PV_Swap;
		}
	}

	return ret;
}

DoubleVector 
AQLPriceArbFreeGenerator::getXccyBasisValue(	
	const AQLDate&  basedate, 
	bool isRenAdj, 
	const LiborMarket& libMkt, 
	const SwapMarket& swapMkt, 
	const XCCYBasisMarket& xccyBasisMkt,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	AQLInterpolationBase* pInter_3ML,
	AQLInterpolationBase* pInter_6ML,
	AQLInterpolationBase* pInter_DF,
	bool isUSD )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	size_t dateSize_3M = termGrid_3MRoll.size();
	double PV_Curr=0.,ANN_Curr=0.,PV_USD=0.,PV_baseccy=0.;
	double term_accru = 0.,term_accru_lib = 0.,forward = 0.,df = 0.,todayFX = 1.;

	DoubleVector ret(xccyBasisMkt.mktTerms.size());
	size_t pos = 0;

	double spotTerm = dc_act365.getTerm(basedate, xccyBasisMkt.spotDate);
	if (isUSD)
	{
		if (isRenAdj)
		{
			todayFX = xccyBasisMkt.pInter_baseccydf->value(spotTerm) / pInter_DF->value(spotTerm);
		}
		else
		{
			todayFX = xccyBasisMkt.pInter_fPrices->value(spotTerm) / pInter_DF->value(spotTerm);
		}
	}
	else
	{
		if (isRenAdj)
		{
			todayFX = pInter_DF->value(spotTerm) / xccyBasisMkt.pInter_usd->value(spotTerm);
		}
	}

	if (isUSD)
	{
		for (unsigned int i = 0; i < xccyBasisMkt.mktTerms.size(); i++)
		{
			AQLDate endDate = AQLDateCalculations::getDate(xccyBasisMkt.spotDate, xccyBasisMkt.mktTerms_str[i], xccyBasisMkt.sld, xccyBasisMkt.pCal, true, &xccyBasisMkt.roll_conv);
			double floaterPrice = 0.;

			for (; dateGrid_3MRoll[pos+1]<=endDate; pos++)
			{
				if (isRenAdj)
				{
					if(pos==0)
					{
						PV_USD += pInter_3ML->value(termGrid_3MRoll[pos]) / pInter_3ML->value(termGrid_3MRoll[pos+1]) * 
							 pInter_DF->value(termGrid_3MRoll[pos+1]) - pInter_DF->value(spotTerm);
					}
					else
					{
						double amount = xccyBasisMkt.pInter_baseccydf->value(termGrid_3MRoll[pos]) / pInter_DF->value(termGrid_3MRoll[pos]);
						double adjustValue = 1.;
						if (xccyBasisMkt.pInter_adjust) adjustValue = xccyBasisMkt.pInter_adjust->value(termGrid_3MRoll[pos]);
						amount /= xccyBasisMkt.pInter_baseccydf->value(spotTerm) / pInter_DF->value(spotTerm);
						PV_USD += (pInter_3ML->value(termGrid_3MRoll[pos]) / pInter_3ML->value(termGrid_3MRoll[pos+1]) * 
							  pInter_DF->value(termGrid_3MRoll[pos+1]) - pInter_DF->value(termGrid_3MRoll[pos]) ) * amount * adjustValue;
							  
					}
				}
				else
				{
					PV_USD += (pInter_3ML->value(termGrid_3MRoll[pos]) / pInter_3ML->value(termGrid_3MRoll[pos+1]) * 
							  pInter_DF->value(termGrid_3MRoll[pos+1]) - pInter_DF->value(termGrid_3MRoll[pos]) );
				}
				floaterPrice = xccyBasisMkt.pInter_fPrices->value(termGrid_3MRoll[pos + 1]);

				if (pos+2 == dateGrid_3MRoll.size()) break;
			}

			PV_baseccy = floaterPrice - xccyBasisMkt.pInter_fPrices->value(spotTerm);

			ret[i] = PV_baseccy - PV_USD * todayFX;
		}
	}
	else
	{
		for (unsigned int i = 0; i < xccyBasisMkt.mktTerms.size(); i++)
		{
			AQLDate endDate = AQLDateCalculations::getDate(xccyBasisMkt.spotDate, xccyBasisMkt.mktTerms_str[i], xccyBasisMkt.sld, xccyBasisMkt.pCal, true, &xccyBasisMkt.roll_conv);

			for (; dateGrid_3MRoll[pos+1]<=endDate; pos++)
			{
				if (isRenAdj)
				{
					if(pos==0)
					{
						PV_USD += xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[pos]) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[pos+1]) * 
							 xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[pos+1]) - xccyBasisMkt.pInter_usd->value(spotTerm);
					}
					else
					{
						double amount = df / xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[pos]);
						amount /= pInter_DF->value(spotTerm) / xccyBasisMkt.pInter_usd->value(spotTerm);
						double adjustValue = 1.;
						if (xccyBasisMkt.pInter_adjust) adjustValue = xccyBasisMkt.pInter_adjust->value(termGrid_3MRoll[pos]);
						PV_USD += (xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[pos]) / xccyBasisMkt.pInter_usd_3ML->value(termGrid_3MRoll[pos+1]) * 
							  xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[pos+1]) - xccyBasisMkt.pInter_usd->value(termGrid_3MRoll[pos]) ) * amount * adjustValue;
					}
				}
				else
				{
					PV_USD = xccyBasisMkt.pInter_fPrices->value(termGrid_3MRoll[pos+1]) - xccyBasisMkt.pInter_fPrices->value(spotTerm);
				}

				term_accru = xccyBasisMkt.dc.getTerm(dateGrid_3MRoll[pos], dateGrid_3MRoll[pos+1], false);
				term_accru_lib = libMkt.dc.getTerm(dateGrid_3MRoll[pos], dateGrid_3MRoll[pos+1], false);
				forward = (pInter_3ML->value(termGrid_3MRoll[pos]) / pInter_3ML->value(termGrid_3MRoll[pos+1]) - 1.) / term_accru_lib;
				df = pInter_DF->value(termGrid_3MRoll[pos+1]);
				PV_Curr += term_accru * forward * df;
				ANN_Curr += term_accru * df;

				if (pos+2 == dateGrid_3MRoll.size()) break;
			}

			ret[i] = ANN_Curr * xccyBasisMkt.mktRates[i] + PV_Curr + df - pInter_DF->value(spotTerm) - PV_USD * todayFX;
		}
	}

	return ret;
}

DoubleVector 
AQLPriceArbFreeGenerator::getLibBasisValue(	
	const LiborMarket& libMkt, 
	const SwapMarket& swapMkt, 
	const LiborBasisMarket& libBasisMkt,
	const DateVector& dateGrid_3MRoll,
	const DoubleArray& termGrid_3MRoll,
	const DateVector& dateGrid_6MRoll,
	const DoubleArray& termGrid_6MRoll,
	AQLInterpolationBase* pInter_3ML,
	AQLInterpolationBase* pInter_6ML,
	AQLInterpolationBase* pInter_DF )
{
	size_t dateSize_3M = termGrid_3MRoll.size();
    size_t dateSize_6M = termGrid_6MRoll.size();
	double ANN_3L6L_3L = 0.,PV_3L6L_3L = 0.,PV_3L6L_6L = 0.;
	double term_accru = 0., term_accru_lib = 0., forward = 0., df = 0.;
	double term_accru2 = 0., term_accru_lib2 = 0., forward2 = 0., df2 = 0.;

	DoubleVector ret(libBasisMkt.mktTerms.size());
	size_t pos_3m = 0, pos_6m = 0;
	for (unsigned int i = 0; i < libBasisMkt.mktTerms.size(); ++i)
	{
		AQLDate endDate = AQLDateCalculations::getDate(libBasisMkt.spotDate, libBasisMkt.mktTerms_str[i], libBasisMkt.sld, libBasisMkt.pCal, true, &libBasisMkt.roll_conv);
		
		for (; dateGrid_6MRoll[pos_6m+1] <= endDate; ++pos_6m)
		{
			term_accru = libBasisMkt.dc_6L.getTerm(dateGrid_6MRoll[pos_6m], dateGrid_6MRoll[pos_6m+1], false);
			term_accru_lib = libMkt.dc.getTerm(dateGrid_6MRoll[pos_6m], dateGrid_6MRoll[pos_6m+1], false);
			forward = (pInter_6ML->value(termGrid_6MRoll[pos_6m]) / pInter_6ML->value(termGrid_6MRoll[pos_6m+1]) - 1.) / term_accru_lib;
			df = pInter_DF->value(termGrid_6MRoll[pos_6m+1]);
			PV_3L6L_6L += term_accru * forward * df;

			if (pos_6m+2 == dateGrid_6MRoll.size()) break;
		}

		if (libBasisMkt.freq_3L_pay == QUARTERLY)
		{
			for (; dateGrid_3MRoll[pos_3m+1] <= endDate; ++pos_3m)
			{
				term_accru = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[pos_3m], dateGrid_3MRoll[pos_3m+1], false);
				term_accru_lib = libMkt.dc.getTerm(dateGrid_3MRoll[pos_3m], dateGrid_3MRoll[pos_3m+1], false);
				forward = (pInter_3ML->value(termGrid_3MRoll[pos_3m]) / pInter_3ML->value(termGrid_3MRoll[pos_3m+1]) - 1.) / term_accru_lib;
				df = pInter_DF->value(termGrid_3MRoll[pos_3m+1]);
				PV_3L6L_3L += term_accru * forward * df;
				ANN_3L6L_3L += term_accru * df;

				if (pos_3m + 2 == dateGrid_3MRoll.size()) break;
			}
		}
		// compounding
		else if (libBasisMkt.freq_3L_pay == SEMI_ANNUAL)
		{
			for (; dateGrid_3MRoll[pos_3m+1] <= endDate; ++pos_3m)
			{
				term_accru = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[pos_3m], dateGrid_3MRoll[pos_3m+1], false);
				term_accru_lib = libMkt.dc.getTerm(dateGrid_3MRoll[pos_3m], dateGrid_3MRoll[pos_3m+1], false);
				forward = (pInter_3ML->value(termGrid_3MRoll[pos_3m]) / pInter_3ML->value(termGrid_3MRoll[pos_3m+1]) - 1.) / term_accru_lib;

				++pos_3m;

				term_accru2 = libBasisMkt.dc_3L.getTerm(dateGrid_3MRoll[pos_3m], dateGrid_3MRoll[pos_3m+1], false);
				term_accru_lib2 = libMkt.dc.getTerm(dateGrid_3MRoll[pos_3m], dateGrid_3MRoll[pos_3m+1], false);
				forward2 = (pInter_3ML->value(termGrid_3MRoll[pos_3m]) / pInter_3ML->value(termGrid_3MRoll[pos_3m+1]) - 1.) / term_accru_lib2;
				df2 = pInter_DF->value(termGrid_3MRoll[pos_3m+1]);
				
				PV_3L6L_3L += ((1. + term_accru * forward) * (1. + term_accru2 * forward2) - 1.) * df2;
				ANN_3L6L_3L += (term_accru2 + term_accru * (1. + term_accru2 * forward2)) * df2;

				if (pos_3m + 2 == dateGrid_3MRoll.size()) break;
			}
		}		

		ret[i] = ANN_3L6L_3L * libBasisMkt.mktRates[i] + PV_3L6L_3L - PV_3L6L_6L;
	}

	return ret;
}


/*!
	@brief generate a forecast rate curve adding basis from arbitrage free curve 
*/
void 
AQLPriceArbFreeGenerator::setForecastCurve(	AQLDataInstance* dataInstance,
                                        const AQLString& setUpCurveName)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

    AQLObjectPool& objPool	= dataInstance->getObjectPool();
    //Basis Swap Data
    AQLString marketDataName = setUpCurveName + "Market";
    AQLObject& marketData = objPool.getObject(marketDataName,ENCHKTYPE_ISDEFINED).get();
    AQLPriceDataCalendar& cal = dynamic_cast<AQLPriceDataCalendar&> ((marketData.getData(IR_CALIBRATION_DATA_CALENDARBASE, ISNOTNULL)).get());
    const AQLPriceDataSlidingRule& sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((marketData.getData(IR_CALIBRATION_DATA_SLIDINGRULEBASE, ISNOTNULL)).get());
    
    const AQLDate& spotDate = dynamic_cast<const AQLDataDate&> ((marketData.getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();
    const DoubleArray& basis_Market = dynamic_cast<const AQLDataDoubles&> ((marketData.getData("BasisRate", ISNOTNULL)).get()).get();
    double initialRate = dynamic_cast<const AQLDataDouble&> ((marketData.getData("InitialRate", ISNOTNULL)).get()).get();
    bool isIniRateUse = dynamic_cast<const AQLDataBool&> (marketData.getData("IsInitialRateUse", ISNOTNULL).get()).get();
    const AQLStringVector& terms_str = dynamic_cast<const AQLDataStrings&> ((marketData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();

    const AQLString& baseFreq = dynamic_cast<const AQLDataString&> ((marketData.getData("BaseCurveFrequency", ISNOTNULL)).get()).get();
    const AQLString& setFreq = dynamic_cast<const AQLDataString&> ((marketData.getData("SetUpCurveFrequency", ISNOTNULL)).get()).get();

	const AQLString& arbFreeCurveID = dynamic_cast<const AQLDataString&> ((marketData.getData("ArbFreeCurveID", ISNOTNULL)).get()).get();
    const AQLString& baseForecastCurveName = dynamic_cast<const AQLDataString&> ((marketData.getData("BaseForecastCurveName", ISNOTNULL)).get()).get();
    const AQLString& dfCurveName = dynamic_cast<const AQLDataString&> ((marketData.getData("DiscountCurveName", ISNOTNULL)).get()).get();

    const AQLPriceDataDayCount& bdc = dynamic_cast<const AQLPriceDataDayCount&> ((marketData.getData("BaseCurveDayCount", ISNOTNULL)).get());
    const AQLPriceDataDayCount& sdc = dynamic_cast<const AQLPriceDataDayCount&> ((marketData.getData("SetUpCurveDayCount", ISNOTNULL)).get());

    //Get Base Curve Data
    const AQLObject& arbFreeCurve = objPool.getObject(arbFreeCurveID,ENCHKTYPE_ISDEFINED).get();
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate&> ((arbFreeCurve.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
    const DoubleArray& DFTerms = dynamic_cast<const AQLDataDoubles& >(arbFreeCurve.getData(CALIBRATION_DATA_TERMS + AQLString("_") + dfCurveName,ISDEFINED).get()).get();
    const DoubleArray& DFs = dynamic_cast<const AQLDataDoubles& >(arbFreeCurve.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + dfCurveName,ISDEFINED).get()).get();

    AQLInterpolationBase* pInter_basis = dynamic_cast<AQLInterpolationBase*>
        (dynamic_cast<const AQLPriceDataInterpolation& >(arbFreeCurve.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	AQLCoreFunctionHolder fh_basis(pInter_basis, true);
    AQLInterpolationBase* pInter_BaseRate = dynamic_cast<AQLInterpolationBase*>
        (dynamic_cast<const AQLPriceDataInterpolation& >(arbFreeCurve.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	AQLCoreFunctionHolder fh_BaseRate(pInter_BaseRate, true);
    AQLInterpolationBase* pInter_DF = dynamic_cast<AQLInterpolationBase*>
        (dynamic_cast<const AQLPriceDataInterpolation& >(arbFreeCurve.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	AQLCoreFunctionHolder fh_DF(pInter_DF, true);
    pInter_DF->set(DFTerms,DFs);
    //Set Basis Market
    const AQLString& basisLeg = dynamic_cast<const AQLDataString&> ((marketData.getData("BasisLeg", ISNOTNULL)).get()).get();
    if( basis_Market.size() != terms_str.size() ) 
        throw AQLCoreInvalidData("size of swap market rates and size of swap market terms are not same.", __FILE__, __LINE__);
    size_t basisSize = basis_Market.size();
    DoubleArray basisTerms;
    for(size_t i=0; i < basisSize; i++)
    {
        AQLDate endDate = AQLDateCalculations::getDate(spotDate, terms_str[i], sld, &cal, true);
        basisTerms.push_back( dc_act365.getTerm(asOfDate, endDate) );
    }
    pInter_basis->set(basisTerms,basis_Market);   

    unsigned int setSpan,baseSpan;
    DoubleArray RateTerms,Rates;
    if (baseFreq == ANNUAL) baseSpan = 12;
	else if (baseFreq == SEMI_ANNUAL) baseSpan = 6;
	else if (baseFreq == QUARTERLY) baseSpan = 3;
	else if (baseFreq == MONTHLY) baseSpan = 1;
	else
	{
		//error
		throw AQLCoreInvalidData("Input freq type is not supported", __FILE__, __LINE__);
	}

    if (setFreq == ANNUAL) setSpan = 12;
	else if (setFreq == SEMI_ANNUAL) setSpan = 6;
	else if (setFreq == QUARTERLY) setSpan = 3;
	else if (setFreq == MONTHLY) setSpan = 1;
	else
	{
		//error
		throw AQLCoreInvalidData("Input freq type is not supported", __FILE__, __LINE__);
	}

    //AQLString baseCurveName = arbFreeCurveID;
    //const AQLObject& arbFreeCurve = objPool.getObject(arbFreeCurveID,ENCHKTYPE_ISDEFINED).get();
    //const AQLString& arbFreeCurveID_ = dynamic_cast<const AQLDataString& >(baseForecastCurve.getData("ArbFreeCurve",ISDEFINED).get()).get();
    //if( arbFreeCurveID_ != arbFreeCurveID ) throw AQLCoreInvalidData("arb free curves are inconsistent!", __FILE__, __LINE__);
    const DoubleArray& RateTerms_DF = dynamic_cast<const AQLDataDoubles& >(arbFreeCurve.getData(CALIBRATION_DATA_TERMS + AQLString("_") + baseForecastCurveName,ISDEFINED).get()).get();
    const DoubleArray& Rates_DF= dynamic_cast<const AQLDataDoubles& >(arbFreeCurve.getData(IR_CALIBRATION_DATA_DFS + AQLString("_") + baseForecastCurveName).get()).get();
    pInter_BaseRate->set(RateTerms_DF,Rates_DF);

    DoubleArray setTerms,setDFs,setRates,baseTerms,baseDFs,baseRates,basis,setRates_DF,setTerms_DF;
    setRates_DF.push_back( 1. );
    setTerms_DF.push_back( 0. );
    if( spotDate != asOfDate ) 
    {
        double spotTerm = dc_act365.getTerm(asOfDate, spotDate);
        setRates_DF.push_back( pInter_DF->value(spotTerm) );
        setTerms_DF.push_back( spotTerm );
    }

    DateVector setDates,baseDates;
    const AQLDate endDate = AQLDateCalculations::getDate(spotDate, terms_str[basisSize-1], true);

    generateSchedule(asOfDate, spotDate, endDate, setSpan, &cal, sld, setDates, setTerms,false);      
    for(size_t i=1; i<setTerms.size(); i++)
    {
        setDFs.push_back( pInter_DF->value( setTerms[i] ) );
        if(setSpan>=baseSpan)
        {
            basis.push_back( pInter_basis->value( setTerms[i] ) );
        }
    }

    generateSchedule(asOfDate, spotDate, endDate, baseSpan, &cal, sld, baseDates, baseTerms,false);   
    for(size_t i=1; i<baseTerms.size(); i++)
    {
        baseDFs.push_back( pInter_DF->value( baseTerms[i] ) );
        baseRates.push_back( (pInter_BaseRate->value( baseTerms[i-1] ) / pInter_BaseRate->value( baseTerms[i] ) - 1.) / 
							  bdc.getTerm(baseDates[i-1], baseDates[i]) );
        if(setSpan<baseSpan)
        {
            basis.push_back( pInter_basis->value( baseTerms[i] ) );
        }
    }
    
    double setPV=0.,setANN=0.,basePV=0.,baseANN=0.;
    if(setSpan>=baseSpan && basisLeg == baseForecastCurveName)
    {
        unsigned int oneRoll = setSpan/baseSpan;
        DoubleArray term_accru_base(oneRoll);
        double term_accru_set;
        for(size_t i=0; i<basis.size(); i++)
        {
            term_accru_set = 0.;
            for(size_t j=0; j<oneRoll; j++)
            {
                term_accru_base[j] = bdc.getTerm(baseDates[i*oneRoll+j], baseDates[i*oneRoll+j+1], false);
                term_accru_set += sdc.getTerm(baseDates[i*oneRoll+j], baseDates[i*oneRoll+j+1], false);
                baseANN += term_accru_base[j] * baseDFs[i*oneRoll+j];
                basePV += term_accru_base[j] * baseDFs[i*oneRoll+j] * baseRates[i*oneRoll+j];
            }
            if( isIniRateUse == true && i==0 )
            {
                setRates.push_back( initialRate );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates[i] * term_accru_set) );
                setTerms_DF.push_back( setTerms[i+1] );
            }
            else
            {
                setRates.push_back( (basePV + baseANN * basis[i] - setPV)/term_accru_set/setDFs[i] );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates[i] * term_accru_set) );
                setTerms_DF.push_back( setTerms[i+1] );
            }
            setPV += term_accru_set * setRates[i] * setDFs[i];           
        }
    }
    else if(setSpan>=baseSpan && basisLeg == setUpCurveName)
    {
        unsigned int oneRoll = setSpan/baseSpan;
        DoubleArray term_accru_base(oneRoll);
        double term_accru_set;
        for(size_t i=0; i<basis.size(); i++)
        {
            term_accru_set = 0.;
            for(size_t j=0; j<oneRoll; j++)
            {
                term_accru_base[j] = bdc.getTerm(baseDates[oneRoll*i+j], baseDates[oneRoll*i+j+1], false);
                term_accru_set += sdc.getTerm(baseDates[oneRoll*i+j], baseDates[oneRoll*i+j+1], false);
                basePV += term_accru_base[j] * baseDFs[i*oneRoll+j] * baseRates[i*oneRoll+j];
            }
            
            if( isIniRateUse == true && i==0 )
            {
                setRates.push_back( initialRate );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates[i] * term_accru_set) );
                setTerms_DF.push_back( setTerms[i+1] );
            }
            else
            {
                setRates.push_back( (basePV - basis[i] * setANN - setPV)/term_accru_set/setDFs[i] - basis[i] );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates[i] * term_accru_set) );
                setTerms_DF.push_back( setTerms[i+1] );
            }
            setANN += term_accru_set * setDFs[i];
            setPV += term_accru_set * setRates[i] * setDFs[i];
        }
    }
    else if(setSpan<baseSpan && basisLeg == baseForecastCurveName)
    {
        unsigned int oneRoll = baseSpan/setSpan;
        DoubleArray term_accru_set(oneRoll);
        double term_accru_base,term_accru_pre;
        double d,tmp,tmp1,tmp2;
        for(size_t i=0; i<basis.size(); i++)
        {
            term_accru_base = 0.;tmp=0.;
            for(size_t j=0; j<oneRoll; j++)
            {
                term_accru_set[j] = sdc.getTerm(setDates[oneRoll*i+j], setDates[oneRoll*i+j+1], false);
                term_accru_base += bdc.getTerm(setDates[oneRoll*i+j], setDates[oneRoll*i+j+1], false);
                tmp += term_accru_set[j] * setDFs[i*oneRoll+j];
            }
            basePV += term_accru_base * baseDFs[i] * baseRates[i];
            baseANN += term_accru_base * baseDFs[i];

            if( i==0 )
            {
                setRates.push_back( initialRate );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[0]) );
                setTerms_DF.push_back( setTerms[1] );

                setPV += term_accru_set[0] * setRates[0] * setDFs[0];

                tmp1=0.;tmp2=0.;
                for(size_t j=1; j<oneRoll; j++)
                {
                    tmp1 += term_accru_set[j-1];
                    tmp2 += tmp1 * term_accru_set[j] * setDFs[i*oneRoll+j];
                }
                d = (basePV + baseANN * basis[i] - initialRate * tmp) / tmp2;
                for(size_t j=1; j<oneRoll; j++)
                {
                    setRates.push_back( setRates[j-1] + d * term_accru_set[j-1] ); 
                    setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[j]) );
                    setTerms_DF.push_back( setTerms[j+1] );
                    setPV += term_accru_set[j] * setRates[j] * setDFs[j];
                }
            }
            else
            {
                term_accru_pre = sdc.getTerm(setDates[oneRoll*i-1], setDates[oneRoll*i], false);
                tmp1 = term_accru_pre;
                tmp2 = tmp1 * term_accru_set[0] * setDFs[i*oneRoll];
                for(size_t j=1; j<oneRoll; j++)
                {
                    tmp1 += term_accru_set[j-1];
                    tmp2 += tmp1 * term_accru_set[j] * setDFs[i*oneRoll+j];
                }
                d = (basePV + baseANN * basis[i] - setPV - setRates[oneRoll*i-1] * tmp) / tmp2;

                setRates.push_back( setRates[oneRoll*i-1] + d * term_accru_pre );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[0]) );
                setTerms_DF.push_back( setTerms[oneRoll*i+1] );
                setPV += term_accru_set[0]  * setRates[oneRoll*i] * setDFs[oneRoll*i];
                for(size_t j=1; j<oneRoll; j++)
                {
                    setRates.push_back( setRates[oneRoll*i-1+j] + d * term_accru_set[j-1] );
                    setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[j]) );
                    setTerms_DF.push_back( setTerms[oneRoll*i+j+1] );
                    setPV += term_accru_set[j] * setRates[oneRoll*i+j] * setDFs[oneRoll*i+j];
                }
            }            
        }
    }
    else if(setSpan<baseSpan && basisLeg == setUpCurveName)
    {
        unsigned int oneRoll = baseSpan/setSpan;
        DoubleArray term_accru_set(oneRoll);
        double term_accru_base,term_accru_pre;
        double d,tmp,tmp1,tmp2;
        for(size_t i=0; i<basis.size(); i++)
        {
            term_accru_base = 0.;tmp=0.;
            for(size_t j=0; j<oneRoll; j++)
            {
                term_accru_set[j] = sdc.getTerm(setDates[oneRoll*i+j], setDates[oneRoll*i+j+1], false);
                term_accru_base += bdc.getTerm(setDates[oneRoll*i+j], setDates[oneRoll*i+j+1], false);
                tmp += term_accru_set[j] * setDFs[i*oneRoll+j];
            }
            basePV += term_accru_base * baseDFs[i] * baseRates[i];

            if( i==0 )
            {
                setRates.push_back( initialRate );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[0]) );
                setTerms_DF.push_back( setTerms[1] );

                setPV += term_accru_set[0] * setRates[0] * setDFs[0];
                setANN += term_accru_set[0] * setDFs[0];

                tmp1=0.; tmp2=0.;
                for(size_t j=1; j<oneRoll; j++)
                {
                    tmp1 += term_accru_set[j-1];
                    tmp2 += tmp1 * term_accru_set[j] * setDFs[i*oneRoll+j];
                }
                d = (basePV - basis[i] * tmp - initialRate * tmp) / tmp2;
                for(size_t j=1; j<oneRoll; j++)
                {
                    setRates.push_back( setRates[j-1] + d * term_accru_set[j-1] );
                    setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[j]) );
                    setTerms_DF.push_back( setTerms[j+1] );
                    setPV += term_accru_set[j] * setRates[j] * setDFs[j];
                    setANN += term_accru_set[j] * setDFs[j];
                }
            }
            else
            {
                term_accru_pre = sdc.getTerm(setDates[oneRoll*i-1], setDates[oneRoll*i], false);
                tmp1 = term_accru_pre;
                tmp2 = tmp1 * term_accru_set[0] * setDFs[i*oneRoll];
                for(size_t j=1; j<oneRoll; j++)
                {
                    tmp1 += term_accru_set[j-1];
                    tmp2 += tmp1 * term_accru_set[j] * setDFs[i*oneRoll+j];
                }
                d = (basePV - setPV - setANN * basis[i] - basis[i] * tmp - setRates[oneRoll*i-1] * tmp) / tmp2;

                setRates.push_back( setRates[oneRoll*i-1] + d * term_accru_pre );
                setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[0]) );
                setTerms_DF.push_back( setTerms[oneRoll*i+1] );
                setPV += term_accru_set[0]  * setRates[oneRoll*i] * setDFs[oneRoll*i];
                setANN += term_accru_set[0] * setDFs[oneRoll*i];
                for(size_t j=1; j<oneRoll; j++)
                {
                    setRates.push_back( setRates[oneRoll*i-1+j] + d * term_accru_set[j-1] );
                    setRates_DF.push_back( setRates_DF.back() / (1. + setRates.back() * term_accru_set[j]) );
                    setTerms_DF.push_back( setTerms[oneRoll*i+j+1] );
                    setPV += term_accru_set[j] * setRates[oneRoll*i+j] * setDFs[oneRoll*i+j];
                    setANN += term_accru_set[j] * setDFs[oneRoll*i+j];
                }
            }            
        }
    }
    else
    {
        throw AQLCoreInvalidData("basis leg curve is not supported!", __FILE__, __LINE__);
    }

    setTerms.erase( setTerms.end()-1 );

    AQLObject* e = NULL;
    AQLString name = arbFreeCurveID + "_" + setUpCurveName;
	if(!objPool.getObject(name).isDefined())
	{
		e = new AQLObject;
		objPool.set(name,e);
	}
	else
	{
		objPool.getObject(name).get().clear();
		e = &objPool.getObject(name).get();
	}
    
    e->add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));
    e->add(CALIBRATION_DATA_NAME,	new AQLDataString(name) );
    e->add("Terms_Rate", new AQLDataDoubles(setTerms) );
    e->add("ForecastRates",	new AQLDataDoubles(setRates) );
	e->add("ArbFreeCurve", new AQLDataString(arbFreeCurveID));

    name = arbFreeCurveID;;
	if(!objPool.getObject(name).isDefined())
	{
		e = new AQLObject;
		objPool.set(name,e);
	}
	else
	{
		e = &objPool.getObject(name).get();
	}
	e->remove(CALIBRATION_DATA_TERMS + AQLString("_") + setUpCurveName);
	e->remove(IR_CALIBRATION_DATA_DFS + AQLString("_") + setUpCurveName);
    e->add(CALIBRATION_DATA_TERMS + AQLString("_") + setUpCurveName, new AQLDataDoubles(setTerms_DF));
	e->add(IR_CALIBRATION_DATA_DFS + AQLString("_") + setUpCurveName, new AQLDataDoubles(setRates_DF));
}

/*!
	@brief generate credit discount curve from forecast curve 
*/
void 
AQLPriceArbFreeGenerator::
generateCdtDFCurve( AQLDataInstance* dataInstance,
                    const AQLString& arbFreeCurveID,
                    const AQLString& forecastCurveID,
                    const AQLString& ctdCurveID,
                    const AQLDate& spotDate,
                    AQLPriceDataCalendar& cal, 
                    AQLPriceDataSlidingRule& sld, 
                    AQLString& freq, 
                    AQLPriceDataDayCount& dc,
                    double spread )
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool	= dataInstance->getObjectPool();
    AQLString curveName = arbFreeCurveID + "_" + forecastCurveID;
    const AQLObject& forecastCurve = objPool.getObject(curveName,ENCHKTYPE_ISDEFINED).get();
    const AQLString& arbFreeCurveID_ = dynamic_cast<const AQLDataString& >(forecastCurve.getData("ArbFreeCurve",ISDEFINED).get()).get();
    if( arbFreeCurveID_ != arbFreeCurveID ) throw AQLCoreInvalidData("arv free curves are inconsistent!", __FILE__, __LINE__);

    const DoubleArray& terms_rate = dynamic_cast<const AQLDataDoubles& >(forecastCurve.getData("Terms_Rate",ISDEFINED).get()).get();
    const DoubleArray& rates = dynamic_cast<const AQLDataDoubles& >(forecastCurve.getData("ForecastRates",ISDEFINED).get()).get();
    if( terms_rate.size() != rates.size() ) throw AQLCoreInvalidData("sizes of rates and terms are not same!", __FILE__, __LINE__);

    AQLObject& arbFreeCurve = objPool.getObject(arbFreeCurveID,ENCHKTYPE_ISDEFINED).get();
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(arbFreeCurve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get()).get();
    const DoubleArray& terms_df = dynamic_cast<const AQLDataDoubles& >(arbFreeCurve.getData(CALIBRATION_DATA_TERMS,ISDEFINED).get()).get();
    const DoubleArray& dfs = dynamic_cast<const AQLDataDoubles& >(arbFreeCurve.getData(IR_CALIBRATION_DATA_DFS,ISDEFINED).get()).get();
    if( terms_df.size() != dfs.size()  ) throw AQLCoreInvalidData("sizes of dfs and terms are not same!", __FILE__, __LINE__);

    AQLInterpolationBase* pInter_Rates = dynamic_cast<AQLInterpolationBase*>
        (dynamic_cast<const AQLPriceDataInterpolation& >(arbFreeCurve.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	AQLCoreFunctionHolder fh_Rates(pInter_Rates, true);
    AQLInterpolationBase* pInter_DF = dynamic_cast<AQLInterpolationBase*>
        (dynamic_cast<const AQLPriceDataInterpolation& >(arbFreeCurve.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	AQLCoreFunctionHolder fh_DF(pInter_DF, true);
    pInter_Rates->set(terms_rate, rates);
    pInter_DF->set(terms_df, dfs);

    DoubleArray dfs_ctd,terms_ctd;
    dfs_ctd.push_back(1.);
    terms_ctd.push_back(0.);
    double spotTerm = dc_act365.getTerm(asOfDate, spotDate);
    if( spotTerm != 0. ) 
    {
        dfs_ctd.push_back( pInter_DF->value(spotTerm) );
        terms_ctd.push_back( spotTerm );
    }

    unsigned int span;
    if (freq == ANNUAL) span = 12;
	else if (freq == SEMI_ANNUAL) span = 6;
    else if (freq == QUARTERLY) span = 3;
    else if (freq == MONTHLY) span = 1;
	else
	{
		//error
		throw AQLCoreInvalidData("Input freq type is not supported", __FILE__, __LINE__);
	}

    AQLDate date = spotDate;
    AQLDate date_sld = sld.getDate(date, cal);
    size_t n;
    double rate,term_sld=0.;
    AQLDate fixingDate;
    while(term_sld<=terms_df.back())
    {
        n = dfs_ctd.size();
        fixingDate = date_sld;
        rate = pInter_Rates->value(dc_act365.getTerm(asOfDate, fixingDate));

        date.addMonths(span);
        date_sld = sld.getDate(date, cal);
        term_sld = dc_act365.getTerm(asOfDate, date_sld);

        terms_ctd.push_back( term_sld );
        dfs_ctd.push_back( dfs_ctd[n-1] / (1. + (rate + spread) * dc.getTerm(fixingDate, date_sld, false) ) );   
    }
    
    AQLObject* e = NULL;
	if(!objPool.getObject(ctdCurveID).isDefined())
	{
		e = new AQLObject;
		objPool.set(ctdCurveID,e);
	}
	else
	{
		objPool.getObject(ctdCurveID).get().clear();
		e = &objPool.getObject(ctdCurveID).get();
	}
    
    e->add(CALIBRATION_DATA_NAME,	new AQLDataString(ctdCurveID));
    e->add(CALIBRATION_DATA_ASOFDATE,	new AQLDataDate(asOfDate));
    e->add(CALIBRATION_DATA_TERMS, new AQLDataDoubles(terms_ctd));
    e->add(IR_CALIBRATION_DATA_DFS, new AQLDataDoubles(dfs_ctd));
    e->add("ArbFreeCurve", new AQLDataString(arbFreeCurveID_));
}

void 
AQLPriceArbFreeGenerator::
generateSchedule( const AQLDate& asOfDate,
           const AQLDate& spotDate,
           const AQLDate& endDate,
           unsigned int span,
           const AQLPriceDataCalendar* pCal,
           const AQLPriceDataSlidingRule& sld,
           DateVector& dates,
           DoubleArray& terms,
		   bool isEOMRoll)
{
    dates.clear();
    terms.clear();
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

    if( asOfDate > spotDate ) throw AQLCoreInvalidData("as of date is before spot date!", __FILE__, __LINE__);

    dates.push_back( spotDate );
    terms.push_back( dc_act365.getTerm(asOfDate, spotDate) );
    AQLDate date = spotDate;
    date.addMonths(span);
    AQLDate date_sld;
	if (isEOMRoll)
	{
		date_sld = pCal->getEOMDay(date);
	}
	else
	{
		date_sld = sld.getDate(date, *pCal);
	}

	unsigned int count = 2; 
    while (date_sld.yearOfEra() * 12 + date_sld.monthOfYear() <= endDate.yearOfEra() * 12 + endDate.monthOfYear())
    {
        dates.push_back(date_sld);
        terms.push_back(dc_act365.getTerm(asOfDate, date_sld));

		AQLDate date = spotDate;
        date.addMonths(span * count);
        if (isEOMRoll)
		{
			date_sld = pCal->getEOMDay(date);
		}
		else
		{
			date_sld = sld.getDate(date, *pCal);
		}

		++count;
    }
}

void 
AQLPriceArbFreeGenerator::
insertRate( double rate, 
            DoubleMatrix& termsMtx, 
            DoubleArray& rates, 
            DoubleArray& df_terms, 
            DoubleArray& dfs,
            double staTerm, 
            double endTerm, 
            const AQLDate& staDate, 
            const AQLDate& endDate, 
            const AQLPriceDataDayCount& dc )
{
	if (termsMtx.size() != 2)
	{
		throw AQLCoreInvalidData("Invaild termsMtx", __FILE__, __LINE__);
	}
    termsMtx[0].push_back( staTerm );
	termsMtx[1].push_back( endTerm );
    rates.push_back( rate );

    double dfs_last;
    if(dfs.size() == 0) dfs_last = 1.;
    else dfs_last = dfs.back();

    df_terms.push_back( endTerm );
    dfs.push_back( dfs_last / ( 1. + dc.getTerm(staDate, endDate, false) * rate ) );
}

bool
AQLPriceArbFreeGenerator::
insertToVector( double in1,
				double in2,
				DoubleArray& array1, 
				DoubleArray& array2,
				unsigned int& pos)
{
	if (array1.size() != array2.size()) 
		throw AQLCoreInvalidData("sizes must be same!", __FILE__, __LINE__);

	if (array1.end() == std::find(array1.begin(), array1.end(), in1))
	{
		for (pos = 0; pos < array1.size(); pos++)
		{
			if (array1[pos] > in1)
			{
				array1.insert(array1.begin() + pos, in1);
				array2.insert(array2.begin() + pos, in2);
				return true;
			}
		}
	}
	return false;
}

bool
AQLPriceArbFreeGenerator::
getPositionOfVector( double target,
					 DoubleArray& vec, 
					 double error,
					 unsigned int& pos )
{
	for (pos = 0; pos < vec.size(); pos++)
	{
		if (AQLMath::abs(target - vec[pos]) < error) return true;
	}
	for (pos = 0; pos < vec.size(); pos++)
	{
		if (target < vec[pos]) break;
	}
	return false;

}

void 
AQLPriceArbFreeGenerator::
insertFRA( const AQLDate& basedate,   
		   const FRAMarket& fra3LMkt,
           const FRAMarket& fra6LMkt,
		   DoubleArray& dfTerms,
		   DoubleArray& dfCurve,
           DoubleMatrix& threeMLTermsMtx_Rate,
           DoubleArray& threeMLRate,
           DoubleArray& threeMLTerms_DF,
           DoubleArray& threeMLDF,
           DoubleMatrix& sixMLTermsMtx_Rate,
           DoubleArray& sixMLRate,
           DoubleArray& sixMLTerms_DF,
           DoubleArray& sixMLDF,
		   unsigned int& threeMLSPos,
		   unsigned int& sixMLSPos,
           AQLInterpolationBase* pInter )
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	map<AQLString, double >::const_iterator it;

    AQLDate date_sld,date,startDate;
	AQLInterpolationBase* pInter_DF = dynamic_cast<AQLInterpolationBase*>(pInter->clone());
    pInter_DF->set(dfTerms, dfCurve);
    AQLInterpolationBase* pInter_threeMLDF = dynamic_cast<AQLInterpolationBase*>(pInter->clone());
    pInter_threeMLDF->set(threeMLTerms_DF, threeMLDF);
    AQLCoreFunctionHolder fh_threeMLDF(pInter_threeMLDF, true);
    AQLInterpolationBase* pInter_sixMLDF = dynamic_cast<AQLInterpolationBase*>(pInter->clone());
    pInter_sixMLDF->set(sixMLTerms_DF, sixMLDF);
    AQLCoreFunctionHolder fh_sixMLDF(pInter_sixMLDF, true);   
    
	// 3M FRA
	double l_threeStartTerm = threeMLTermsMtx_Rate[0].front();
	unsigned int pos;
	for (int month=1; month<9; month++)
	{
		AQLString month_str = AQLString(month) + AQLString("M");
		it = fra3LMkt.map_term_rate.find(month_str);
		if( it == fra3LMkt.map_term_rate.end() ) continue;
		date = fra3LMkt.spotDate;

		date.addMonths(month);
		date_sld = fra3LMkt.sld.getDate( date, *fra3LMkt.pCal );
		if (fra3LMkt.isEOMRoll) date_sld = fra3LMkt.pCal->getEOMDay(date_sld);
		AQLDate startDate = date_sld;
		double startTerm = dc_act365.getTerm( basedate, date_sld );
		double startDF = pInter_threeMLDF->value(startTerm);
		insertToVector(startTerm, pInter_DF->value(startTerm), dfTerms, dfCurve, pos);
		insertToVector(startTerm, startDF, threeMLTerms_DF, threeMLDF, pos);

		date.addMonths(3);
		date_sld = fra3LMkt.sld.getDate( date, *fra3LMkt.pCal );
		if (fra3LMkt.isEOMRoll) date_sld = fra3LMkt.pCal->getEOMDay(date_sld);
		double endTerm = dc_act365.getTerm( basedate, date_sld );
		double endDF = startDF  / (1. + fra3LMkt.dc.getTerm(startDate, date_sld, false) * it->second);
		insertToVector(endTerm, pInter_DF->value(endTerm), dfTerms, dfCurve, pos);
		insertToVector(endTerm, endDF, threeMLTerms_DF, threeMLDF, pos);

		if (insertToVector(startTerm, it->second, threeMLTermsMtx_Rate[0], threeMLRate, pos))
		{
			threeMLTermsMtx_Rate[1].insert(threeMLTermsMtx_Rate[1].begin() + pos, endTerm);
		}
		l_threeStartTerm = startTerm;

		pInter_threeMLDF->set(threeMLTerms_DF, threeMLDF);
	}

	if (!AQLAlgorithm::find<DoubleArray, double>(threeMLTermsMtx_Rate[0], l_threeStartTerm, 0, threeMLTermsMtx_Rate[0].size() - 1, threeMLSPos))
	{
		throw AQLCoreInvalidData("threeMLTermsMtx_Rate is not consistent.", __FILE__, __LINE__);
	}
	++threeMLSPos;
	if (threeMLTermsMtx_Rate[0].size() <= threeMLSPos)
	{
		throw AQLCoreInvalidData("threeMLTermsMtx_Rate, swap term does not exist.", __FILE__, __LINE__);
	}

	// 6M FRA
	double l_sixStartTerm = sixMLTermsMtx_Rate[0].front();
	for (int month=1; month<6; month++)
	{
		AQLString month_str = AQLString(month) + AQLString("M");
		it = fra6LMkt.map_term_rate.find(month_str);
		if( it == fra6LMkt.map_term_rate.end() ) continue;
		date = fra6LMkt.spotDate;

		date.addMonths(month);
		date_sld = fra6LMkt.sld.getDate( date, *fra6LMkt.pCal );
		if (fra6LMkt.isEOMRoll) date_sld = fra6LMkt.pCal->getEOMDay(date_sld);
		AQLDate startDate = date_sld;
		double startTerm = dc_act365.getTerm( basedate, date_sld );
		double startDF = pInter_sixMLDF->value(startTerm);
		insertToVector(startTerm, startDF, sixMLTerms_DF, sixMLDF, pos);

		date.addMonths(6);
		date_sld = fra6LMkt.sld.getDate( date, *fra3LMkt.pCal );
		if (fra6LMkt.isEOMRoll) date_sld = fra6LMkt.pCal->getEOMDay(date_sld);
		double endTerm = dc_act365.getTerm( basedate, date_sld );
		double endDF = startDF  / (1. + fra6LMkt.dc.getTerm(startDate, date_sld, false) * it->second);
		insertToVector(endTerm, endDF, sixMLTerms_DF, sixMLDF, pos);

		if (insertToVector(startTerm, it->second, sixMLTermsMtx_Rate[0], sixMLRate, pos))
		{
			sixMLTermsMtx_Rate[1].insert(sixMLTermsMtx_Rate[1].begin() + pos, endTerm);
		}
		l_sixStartTerm = startTerm;
		pInter_sixMLDF->set(sixMLTerms_DF, sixMLDF);
	}
	if (!AQLAlgorithm::find<DoubleArray, double>(sixMLTermsMtx_Rate[0], l_sixStartTerm, 0, sixMLTermsMtx_Rate[0].size() - 1, sixMLSPos))
	{
		throw AQLCoreInvalidData("sixMLTermsMtx_Rate is not consistent.", __FILE__, __LINE__);
	}
	++sixMLSPos;
	if (sixMLTermsMtx_Rate[0].size() <= sixMLSPos)
	{
		throw AQLCoreInvalidData("sixMLTermsMtx_Rate, swap term does not exist.", __FILE__, __LINE__);
	}
}

void 
AQLPriceArbFreeGenerator::
insertFuture(	const AQLDate& basedate, 
				const FutureMarket& futureMkt,
				const SwapMarket& swapMkt,
				DoubleMatrix& threeMLTermsMtx_Rate,
				DoubleArray& threeMLRate,
				DoubleArray& threeMLTerms_DF,
				DoubleArray& threeMLDF,
				unsigned int& threeMLSPos,
				AQLInterpolationBase* pInter )
{
	if (futureMkt.mktRates.empty())
	{
		return;
	}
	if (threeMLTermsMtx_Rate.size() != 2 || threeMLTermsMtx_Rate[0].size() != threeMLTermsMtx_Rate[1].size())
	{
		throw AQLCoreInvalidData("Invalid threeMLTermsMtx_Rate", __FILE__, __LINE__);
	}
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
	AQLPriceDataConvention conv(futureMkt.dc.getDayCount(), rc);
	
	DoubleArray::iterator it;

	double futureStartTerm = dc_act365.getTerm(basedate, futureMkt.endDates.front());
	double swapStartTerm = dc_act365.getTerm(basedate, swapMkt.startDate);
	for (size_t i = 2; i < threeMLTerms_DF.size(); )
	{
		if (threeMLTerms_DF[i] >= swapStartTerm) break;
		if (threeMLTerms_DF[i] >= futureStartTerm)
		{
			it = threeMLTerms_DF.begin() + i;
			threeMLTerms_DF.erase(it);
			it = threeMLDF.begin() + i;
			threeMLDF.erase(it);
		}
		else
		{
			i++;
		}
	}
	for (size_t i = 1; i < threeMLTermsMtx_Rate[0].size(); )
	{
		if (threeMLTermsMtx_Rate[0][i] >= swapStartTerm) break;
		if (threeMLTermsMtx_Rate[0][i] >= futureStartTerm)
		{
			it = threeMLTermsMtx_Rate[0].begin() + i;
			threeMLTermsMtx_Rate[0].erase(it);
			it = threeMLTermsMtx_Rate[1].begin() + i;
			threeMLTermsMtx_Rate[1].erase(it);
			it = threeMLRate.begin() + i;
			threeMLRate.erase(it);
		}
		else
		{
			i++;
		}
	}

	AQLInterpolationBase* pInter_threeMLDF = dynamic_cast<AQLInterpolationBase*>(pInter->clone());
    pInter_threeMLDF->set(threeMLTerms_DF, threeMLDF);
    AQLCoreFunctionHolder fh_threeMLDF(pInter_threeMLDF, true);

	// 3M Future
	double l_threeStartTerm = threeMLTermsMtx_Rate[0].front();
	unsigned int pos;
	for (size_t i = 0; i < futureMkt.mktRates.size(); i++)
	{
		double startTerm = dc_act365.getTerm(basedate, futureMkt.startDates[i]);
		double endTerm = dc_act365.getTerm(basedate, futureMkt.endDates[i]);
		double startDF = pInter_threeMLDF->value(startTerm);
		double endDF = startDF * conv.getDF(futureMkt.mktRates[i], futureMkt.startDates[i], futureMkt.endDates[i]);
		insertToVector(startTerm, startDF, threeMLTerms_DF, threeMLDF, pos);
		insertToVector(endTerm, endDF, threeMLTerms_DF, threeMLDF, pos);

		if (insertToVector(startTerm, futureMkt.mktRates[i], threeMLTermsMtx_Rate[0], threeMLRate, pos))
		{
			threeMLTermsMtx_Rate[1].insert(threeMLTermsMtx_Rate[1].begin() + pos, endTerm);
		}
		l_threeStartTerm = startTerm; 

		pInter_threeMLDF->set(threeMLTerms_DF, threeMLDF);
	}
	if (!AQLAlgorithm::find<DoubleArray, double>(threeMLTermsMtx_Rate[0], l_threeStartTerm, 0, threeMLTermsMtx_Rate[0].size() - 1, threeMLSPos))
	{
		throw AQLCoreInvalidData("threeMLTermsMtx_Rate is not consistent.", __FILE__, __LINE__);
	}
	++threeMLSPos;
	if (threeMLTermsMtx_Rate[0].size() <= threeMLSPos)
	{
		throw AQLCoreInvalidData("threeMLTermsMtx_Rate, swap term does not exist.", __FILE__, __LINE__);
	}
}

double 
AQLPriceArbFreeGenerator::
getSpotAdjust( const AQLDate& basedate,
               const AQLDate& spotDate,
			   const MoneyMarket& monMkt )
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

    double term_O_N,term_accru_O_N,DF_O_N,term_T_N,term_accru_T_N,DF_T_N=1.;
    //spot date check
    if( spotDate < basedate ) throw AQLCoreInvalidData("spotDate of Swap Market is before base date.", __FILE__, __LINE__);
    //adjust from spot date of Swap to base date
    if( spotDate > basedate )
    {
	    AQLDate tmpDate = basedate; 
        tmpDate.addDays(1);
        const AQLDate endDate_O_N = monMkt.sld.getDate(tmpDate, *monMkt.pCal);
        if( endDate_O_N < basedate ) 
        {
            throw AQLCoreInvalidData("spot date of overnight is before asofdate.", __FILE__, __LINE__);
        }
        else if( endDate_O_N < spotDate )
        {
            term_O_N = dc_act365.getTerm(basedate, endDate_O_N);
            term_accru_O_N = monMkt.dc.getTerm(basedate, endDate_O_N, false);
            DF_O_N = 1. / (1. + monMkt.rates[0] * term_accru_O_N);

            term_T_N = term_O_N + dc_act365.getTerm(endDate_O_N, spotDate);
            term_accru_T_N = monMkt.dc.getTerm(endDate_O_N, spotDate, false);
            DF_T_N = DF_O_N / (1. + monMkt.rates[1] * term_accru_T_N);        
        }
        else if( endDate_O_N >= spotDate )
        {
            term_O_N = dc_act365.getTerm(basedate, spotDate);
            term_accru_O_N = monMkt.dc.getTerm(basedate, spotDate, false);
            DF_O_N = 1. / (1. + monMkt.rates[0] * term_accru_O_N);
        }
    }

    return DF_T_N;
}

void 
AQLPriceArbFreeGenerator::
baseDateAdjust( const AQLDate& basedate,
                const AQLDate& spotDate,
			    const MoneyMarket& monMkt,
                DoubleArray& dfTerms,
			    DoubleArray& dfCurve,
                DoubleMatrix& threeMLTermsMtx_Rate,
                DoubleArray& threeMLRate,
                DoubleArray& threeMLTerms_DF,
                DoubleArray& threeMLDF,
                DoubleMatrix& sixMLTermsMtx_Rate,
                DoubleArray& sixMLRate,
                DoubleArray& sixMLTerms_DF,
                DoubleArray& sixMLDF )
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	if (threeMLTermsMtx_Rate.size() != 2 || threeMLTermsMtx_Rate[0].size() != threeMLTermsMtx_Rate[1].size())
	{
		throw AQLCoreInvalidData("Invalid threeMLTermsMtx_Rate", __FILE__, __LINE__);
	}
	if (sixMLTermsMtx_Rate.size() != 2 || sixMLTermsMtx_Rate[0].size() != sixMLTermsMtx_Rate[1].size())
	{
		throw AQLCoreInvalidData("Invalid sixMLTermsMtx_Rate", __FILE__, __LINE__);
	}
    //spot date check
    if( spotDate < basedate ) throw AQLCoreInvalidData("spotDate of Swap Market is before base date.", __FILE__, __LINE__);
    //adjust from spot date of Swap to base date
    if( spotDate >= basedate )
    {
	    AQLDate tmpDate = basedate; 
        tmpDate.addDays(1);
        const AQLDate endDate_O_N = monMkt.sld.getDate(tmpDate, *monMkt.pCal);
		tmpDate = endDate_O_N;
		tmpDate.addDays(1);
        const AQLDate endDate_T_N = monMkt.sld.getDate(tmpDate, *monMkt.pCal);

		double term_O_N,term_accru_O_N,DF_O_N,
			   term_T_N,term_accru_T_N,DF_T_N,
			   term_Spot,term_accru_Spot,DF_Spot;

		term_O_N = dc_act365.getTerm(basedate, endDate_O_N);
        term_accru_O_N = monMkt.dc.getTerm(basedate, endDate_O_N, false);
        DF_O_N = 1. / (1. + monMkt.rates[0] * term_accru_O_N);

		term_T_N = term_O_N + dc_act365.getTerm(endDate_O_N, endDate_T_N);
        term_accru_T_N = monMkt.dc.getTerm(endDate_O_N, endDate_T_N, false);
        DF_T_N = DF_O_N / (1. + monMkt.rates[1] * term_accru_T_N);

        if( endDate_O_N < basedate ) 
        {
            throw AQLCoreInvalidData("spot date of overnight is before asofdate.", __FILE__, __LINE__);
        }
        else if( endDate_O_N < spotDate )
        {
            term_Spot = term_O_N + dc_act365.getTerm(endDate_O_N, spotDate);
            term_accru_Spot = monMkt.dc.getTerm(endDate_O_N, spotDate, false);
            DF_Spot = DF_O_N / (1. + monMkt.rates[1] * term_accru_Spot);

            for(size_t i=0; i<dfTerms.size(); i++)
            {
                dfTerms[i] += term_Spot;
            }
            for(size_t i=0; i<threeMLTermsMtx_Rate[0].size(); i++)
            {
                threeMLTermsMtx_Rate[0][i] += term_Spot;
				threeMLTermsMtx_Rate[1][i] += term_Spot;
            }
            for(size_t i=0; i<threeMLTerms_DF.size(); i++)
            {
                threeMLTerms_DF[i] += term_Spot;
            }
            for(size_t i=0; i<sixMLTermsMtx_Rate[0].size(); i++)
            {
                sixMLTermsMtx_Rate[0][i] += term_Spot;
				sixMLTermsMtx_Rate[1][i] += term_Spot;
            }
            for(size_t i=0; i<sixMLTerms_DF.size(); i++)
            {
                sixMLTerms_DF[i] += term_Spot;
            }

            for(size_t i=0; i<dfCurve.size(); i++)
            {
                dfCurve[i] *= DF_Spot;
            }
            for(size_t i=0; i<threeMLDF.size(); i++)
            {
                threeMLDF[i] *= DF_Spot;
            }
            for(size_t i=0; i<sixMLDF.size(); i++)
            {
                sixMLDF[i] *= DF_Spot;
            }

			if(spotDate < endDate_T_N)
			{
				dfTerms.insert(dfTerms.begin()+1, term_T_N);
				dfCurve.insert(dfCurve.begin()+1, DF_T_N);
				threeMLTerms_DF.insert(threeMLTerms_DF.begin()+1, term_T_N);
				threeMLDF.insert(threeMLDF.begin()+1, DF_T_N);
				sixMLTerms_DF.insert(sixMLTerms_DF.begin()+1, term_T_N);
				sixMLDF.insert(sixMLDF.begin()+1, DF_T_N);
			}
			else if(spotDate > endDate_T_N)
			{
				dfTerms.insert(dfTerms.begin(), term_T_N);
				dfCurve.insert(dfCurve.begin(), DF_T_N);
				threeMLTerms_DF.insert(threeMLTerms_DF.begin(), term_T_N);
				threeMLDF.insert(threeMLDF.begin(), DF_T_N);
				sixMLTerms_DF.insert(sixMLTerms_DF.begin(), term_T_N);
				sixMLDF.insert(sixMLDF.begin(), DF_T_N);
			}

            dfTerms.insert(dfTerms.begin(), term_O_N);
            dfCurve.insert(dfCurve.begin(), DF_O_N);
            threeMLTerms_DF.insert(threeMLTerms_DF.begin(), term_O_N);
            threeMLDF.insert(threeMLDF.begin(), DF_O_N);
            sixMLTerms_DF.insert(sixMLTerms_DF.begin(), term_O_N);
            sixMLDF.insert(sixMLDF.begin(), DF_O_N);

            dfTerms.insert(dfTerms.begin(), 0.);
            dfCurve.insert(dfCurve.begin(), 1.);
            threeMLTerms_DF.insert(threeMLTerms_DF.begin(), 0.);
            threeMLDF.insert(threeMLDF.begin(), 1.);
            sixMLTerms_DF.insert(sixMLTerms_DF.begin(), 0.);
            sixMLDF.insert(sixMLDF.begin(), 1.);
        }
        else if( endDate_O_N >= spotDate )
        {
            term_Spot = dc_act365.getTerm(basedate, spotDate);
            term_accru_Spot = monMkt.dc.getTerm(basedate, spotDate, false);
            DF_Spot = 1. / (1. + monMkt.rates[0] * term_accru_Spot);

            for(size_t i=0; i<dfTerms.size(); i++)
            {
                dfTerms[i] += term_Spot; 
            }
            for(size_t i=0; i<threeMLTermsMtx_Rate[0].size(); i++)
            {
                threeMLTermsMtx_Rate[0][i] += term_Spot;
				threeMLTermsMtx_Rate[1][i] += term_Spot;
            }
            for(size_t i=0; i<sixMLTermsMtx_Rate[0].size(); i++)
            {
                sixMLTermsMtx_Rate[0][i] += term_Spot;
				sixMLTermsMtx_Rate[1][i] += term_Spot;
            }
            for(size_t i=0; i<threeMLTerms_DF.size(); i++)
            {
                threeMLTerms_DF[i] += term_Spot;
            }
            for(size_t i=0; i<sixMLTerms_DF.size(); i++)
            {
                sixMLTerms_DF[i] += term_Spot;
            }
            for(size_t i=0; i<dfCurve.size(); i++)
            {
                dfCurve[i] *= DF_Spot;
            }
            for(size_t i=0; i<threeMLDF.size(); i++)
            {
                threeMLDF[i] *= DF_Spot;
            }
            for(size_t i=0; i<sixMLDF.size(); i++)
            {
                sixMLDF[i] *= DF_Spot;
            }

			dfTerms.insert(dfTerms.begin()+1, term_T_N);
			dfCurve.insert(dfCurve.begin()+1, DF_T_N);
			threeMLTerms_DF.insert(threeMLTerms_DF.begin()+1, term_T_N);
			threeMLDF.insert(threeMLDF.begin()+1, DF_T_N);
			sixMLTerms_DF.insert(sixMLTerms_DF.begin()+1, term_T_N);
			sixMLDF.insert(sixMLDF.begin()+1, DF_T_N);

			if( endDate_O_N > spotDate )
			{
				dfTerms.insert(dfTerms.begin()+1, term_O_N);
				dfCurve.insert(dfCurve.begin()+1, DF_O_N);
				threeMLTerms_DF.insert(threeMLTerms_DF.begin()+1, term_O_N);
				threeMLDF.insert(threeMLDF.begin()+1, DF_O_N);
				sixMLTerms_DF.insert(sixMLTerms_DF.begin()+1, term_O_N);
				sixMLDF.insert(sixMLDF.begin()+1, DF_O_N);
			}

			if( spotDate != basedate )
			{
				dfTerms.insert(dfTerms.begin(), 0.);
				dfCurve.insert(dfCurve.begin(), 1.);
				threeMLTerms_DF.insert(threeMLTerms_DF.begin(), 0.);
				threeMLDF.insert(threeMLDF.begin(), 1.);
				sixMLTerms_DF.insert(sixMLTerms_DF.begin(), 0.);
				sixMLDF.insert(sixMLDF.begin(), 1.);
			}
        }
    }
}

void 
AQLPriceArbFreeGenerator::
liborDateAdjust(	const AQLDate& basedate,
					const LiborMarket& libMkt, 
					const SwapMarket& swapMkt,
					DoubleMatrix& threeMLTermsMtx_Rate,
					DoubleArray& threeMLRate,
					DoubleArray& threeMLTerms_DF,
					DoubleArray& threeMLDF,
					DoubleMatrix& sixMLTermsMtx_Rate,
					DoubleArray& sixMLRate,
					DoubleArray& sixMLTerms_DF,
					DoubleArray& sixMLDF,
					AQLInterpolationBase* pInter )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	double term_spot_libor = dc_act365.getTerm(basedate, libMkt.spotDate);
	AQLDate threeMDate_libor = AQLDateCalculations::getDate(libMkt.spotDate, "3M", libMkt.sld, libMkt.pCal, true, &libMkt.roll_conv);
	double term_3M_libor = dc_act365.getTerm(basedate, threeMDate_libor);
	AQLDate sixMDate_libor = AQLDateCalculations::getDate(libMkt.spotDate, "6M", libMkt.sld, libMkt.pCal, true, &libMkt.roll_conv);
	double term_6M_libor = dc_act365.getTerm(basedate, sixMDate_libor);

	unsigned int pos_del = 0, pos_insert = 0;
	//3M rate
	if (!getPositionOfVector(swapMkt.termGrid_3MRoll[0], threeMLTermsMtx_Rate[0], eps, pos_del)) 
		throw AQLCoreInvalidData("spot term does not exist in threeMLTermsMtx_Rate.", __FILE__, __LINE__);
	threeMLTermsMtx_Rate[0].erase(threeMLTermsMtx_Rate[0].begin() + pos_del);
	threeMLTermsMtx_Rate[1].erase(threeMLTermsMtx_Rate[1].begin() + pos_del);
	if (!getPositionOfVector(term_spot_libor, threeMLTermsMtx_Rate[0], eps, pos_insert))
	{
		threeMLTermsMtx_Rate[0].insert(threeMLTermsMtx_Rate[0].begin() + pos_insert, term_spot_libor);	
		threeMLTermsMtx_Rate[1].insert(threeMLTermsMtx_Rate[1].begin() + pos_insert, term_3M_libor);
	}
	//6M rate
	if (!getPositionOfVector(swapMkt.termGrid_3MRoll[0], sixMLTermsMtx_Rate[0], eps, pos_del)) 
		throw AQLCoreInvalidData("spot term does not exist in sixMLTermsMtx_Rate.", __FILE__, __LINE__);
	sixMLTermsMtx_Rate[0].erase(sixMLTermsMtx_Rate[0].begin() + pos_del);
	sixMLTermsMtx_Rate[1].erase(sixMLTermsMtx_Rate[1].begin() + pos_del);
	if (!getPositionOfVector(term_spot_libor, sixMLTermsMtx_Rate[0], eps, pos_insert))
	{
		sixMLTermsMtx_Rate[0].insert(sixMLTermsMtx_Rate[0].begin() + pos_insert, term_spot_libor);
		sixMLTermsMtx_Rate[1].insert(sixMLTermsMtx_Rate[1].begin() + pos_insert, term_6M_libor);
	}
	//3M df
	pInter->set(threeMLTerms_DF, threeMLDF);
	if (!getPositionOfVector(swapMkt.termGrid_3MRoll[0], threeMLTerms_DF, eps, pos_del)) 
		throw AQLCoreInvalidData("spot term does not exist in threeMLTerms_DF.", __FILE__, __LINE__);
	threeMLTerms_DF.erase(threeMLTerms_DF.begin() + pos_del);
	threeMLDF.erase(threeMLDF.begin() + pos_del);
	if (!getPositionOfVector(term_spot_libor, threeMLTerms_DF, eps, pos_insert)) 
	{
		threeMLTerms_DF.insert(threeMLTerms_DF.begin() + pos_insert, term_spot_libor);
		threeMLDF.insert(threeMLDF.begin() + pos_insert, pInter->value(term_spot_libor));
	}
	if (!getPositionOfVector(swapMkt.termGrid_3MRoll[1], threeMLTerms_DF, eps, pos_del)) 
		throw AQLCoreInvalidData("spot term does not exist in threeMLTerms_DF.", __FILE__, __LINE__);
	threeMLTerms_DF.erase(threeMLTerms_DF.begin() + pos_del);
	threeMLDF.erase(threeMLDF.begin() + pos_del);
	if (!getPositionOfVector(term_3M_libor, threeMLTerms_DF, eps, pos_insert)) 
	{
		double accruTerm = libMkt.dc.getTerm(libMkt.spotDate, threeMDate_libor, false);
		double startDF = pInter->value(term_spot_libor);
		double df = startDF / (1. + accruTerm * libMkt.threeMLibor);
		threeMLTerms_DF.insert(threeMLTerms_DF.begin() + pos_insert, term_3M_libor);
		threeMLDF.insert(threeMLDF.begin() + pos_insert, df);
	}
	//6M df
	pInter->set(sixMLTerms_DF, sixMLDF);
	if (!getPositionOfVector(swapMkt.termGrid_3MRoll[0], sixMLTerms_DF, eps, pos_del)) 
		throw AQLCoreInvalidData("spot term does not exist in threeMLTerms_DF.", __FILE__, __LINE__);
	sixMLTerms_DF.erase(sixMLTerms_DF.begin() + pos_del);
	sixMLDF.erase(sixMLDF.begin() + pos_del);
	if (!getPositionOfVector(term_spot_libor, sixMLTerms_DF, eps, pos_insert)) 
	{
		sixMLTerms_DF.insert(sixMLTerms_DF.begin() + pos_insert, term_spot_libor);
		sixMLDF.insert(sixMLDF.begin() + pos_insert, pInter->value(term_spot_libor));
	}
	if (!getPositionOfVector(swapMkt.termGrid_3MRoll[2], sixMLTerms_DF, eps, pos_del)) 
		throw AQLCoreInvalidData("spot term does not exist in threeMLTerms_DF.", __FILE__, __LINE__);
	sixMLTerms_DF.erase(sixMLTerms_DF.begin() + pos_del);
	sixMLDF.erase(sixMLDF.begin() + pos_del);
	if (!getPositionOfVector(term_6M_libor, sixMLTerms_DF, eps, pos_insert)) 
	{
		double accruTerm = libMkt.dc.getTerm(libMkt.spotDate, sixMDate_libor, false);
		double startDF = pInter->value(term_spot_libor);
		double df = startDF / (1. + accruTerm * libMkt.sixMLibor);
		sixMLTerms_DF.insert(sixMLTerms_DF.begin() + pos_insert, term_6M_libor);
		sixMLDF.insert(sixMLDF.begin() + pos_insert, df);
	}

}

AQLString 
AQLPriceArbFreeGenerator::getSwapFixFrequency(unsigned int month, const SwapMarket& swapMkt)
{
	if (swapMkt.map_freq_Fix.size() == 0 || !swapMkt.isSwapTenorAdjust) 
	{
		return swapMkt.freq_Fix;
	}
	else
	{
		map<int, AQLString>::const_iterator it = swapMkt.map_freq_Fix.begin();
		while( it != swapMkt.map_freq_Fix.end() )
		{
			if ((*it).first >= month)  return (*it).second;
			++it;
		}
		
		return swapMkt.freq_Fix;
	}
}

AQLPriceArbFreeGenerator::MoneyMarket::
MoneyMarket( vector<const AQLObject* > mktData )
{
	if (mktData.size() != 2) throw AQLCoreInvalidData("No Money Market Object", __FILE__, __LINE__);

    pCal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
    sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
    dc = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	rates.resize(2); terms.resize(2);
	for (size_t i=0; i<2; i++)
	{
		double rate = dynamic_cast<const AQLDataDouble&> ((mktData[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLString dataType = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		if (dataType == O_N)
		{
			rates[0] = rate;
			terms[0] = "ON";
		}
		else if (dataType == T_N)
		{
			rates[1] = rate;
			terms[1] = "TN";
		}
		else
		{
			throw AQLCoreInvalidData("Money market term is ON or TN.", __FILE__, __LINE__);
		}
	}
}

AQLPriceArbFreeGenerator::MoneyMarket::~MoneyMarket(){}

AQLPriceArbFreeGenerator::LiborMarket::
LiborMarket( vector<const AQLObject* > mktData )
{
	if (mktData.size() == 0) throw AQLCoreInvalidData("No Libor Market Object", __FILE__, __LINE__);
	
	DoubleArray rates;
	AQLStringVector terms;
	for (size_t i=0; i<mktData.size(); i++)
	{
		double rate = dynamic_cast<const AQLDataDouble&> ((mktData[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLString term = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		rates.push_back(rate);
		terms.push_back(term);
		if (term == "6M") dc = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[i]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	}

	map<AQLString, double > liborLMarketMap;
    map<AQLString, double >::const_iterator it_libor;
	for(size_t i=0; i<terms.size(); i++)
    {
        liborLMarketMap.insert( map<AQLString, double >::value_type(terms[i], rates[i]) );
    }
	it_libor = liborLMarketMap.find("3M");
	if(it_libor == liborLMarketMap.end())
	{
		throw AQLCoreInvalidData("Both 3M Libor and 6M Libor are needed.", __FILE__, __LINE__);
	}
	else 
	{
		threeMLibor = it_libor->second;
	}
	it_libor = liborLMarketMap.find("6M");
	if(it_libor == liborLMarketMap.end())
	{
		throw AQLCoreInvalidData("Both 3M Libor and 6M Libor are needed.", __FILE__, __LINE__);
	}
	else 
	{
		sixMLibor = it_libor->second;
	}

	spotDate = dynamic_cast<const AQLDataDate&> ((mktData[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();
	pCal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
	isEOMRoll = false;
	const AQLDataHolder *dh = &(mktData[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	roll_conv = isEOMRoll ? ROLLCONV_EOM : ROLLCONV_NORMAL;
}

AQLPriceArbFreeGenerator::LiborMarket::~LiborMarket(){}

AQLPriceArbFreeGenerator::SwapMarket::
SwapMarket( vector<const AQLObject* > mktData, vector<const AQLObject* > tenorSwapMktData, const LiborMarket& libMkt, const AQLDate& basedate )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	if (mktData.size() == 0) throw AQLCoreInvalidData("No Swap Market Object", __FILE__, __LINE__);

	pCal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
    spotDate = dynamic_cast<const AQLDataDate&> ((mktData[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();
    dc_Float = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT, ISNOTNULL)).get());
    dc_Fix = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
	freq_Fix = dynamic_cast<const AQLDataString&> ((mktData[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FIX, ISNOTNULL)).get());
    freq_Float = dynamic_cast<const AQLDataString&> ((mktData[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, ISNOTNULL)).get());
	const AQLDataHolder *dh = &mktData[0]->getData(IR_CALIBRATION_DATA_FREQUENCY_COMPOUND, NOCHECK); 
	if (dh->isDefined() && !dh->isNull()) 
	{ 
		freq_Float_Pay = dynamic_cast<const AQLDataString&> (dh->get());
	}
	else
	{
		freq_Float_Pay = freq_Float;
	}
	int onePeriod_Float = AQLDateCalculations::getPeriodFrequencyInMonths(freq_Float);
	int onePeriod_Float_Pay = AQLDateCalculations::getPeriodFrequencyInMonths(freq_Float_Pay);
	if (onePeriod_Float_Pay % onePeriod_Float != 0)
		throw AQLCoreInvalidData("convention of compounding is not correct!", __FILE__, __LINE__);

	optimizeMethod = NONE;

	dh = &(mktData[0]->getData(IR_CALIBRATION_DATA_OPTIMIZEMETHOD, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		optimizeMethod = dynamic_cast<const AQLDataString &>(dh->get()).get();
	}

	isEOMRoll = false;
	dh = &(mktData[0]->getData(IR_CALIBRATION_DATA_ISEOMROLLSW, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	roll_conv = isEOMRoll ? ROLLCONV_EOM : ROLLCONV_NORMAL;

	DoubleArray rates;
	AQLStringVector terms_str;
	for (size_t i=0; i<mktData.size(); i++)
	{
		double rate = dynamic_cast<const AQLDataDouble&> ((mktData[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLString term = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		rates.push_back(rate);
		terms_str.push_back(term);

		AQLDate gridDate = AQLDateCalculations::getDate(spotDate, term, sld, pCal, true, &roll_conv);
        mktTerms.push_back( dc_act365.getTerm(basedate, gridDate) );
	}
	mktTerms_str = terms_str;
	mktRates = rates;

	startDate = AQLDateCalculations::getDate(spotDate, terms_str.front(), sld, pCal, true, &roll_conv);
    	
	int startPos = 0;
	if ( freq_Float == SEMI_ANNUAL )
	{
		if ( terms_str[0] != AQLString("6M") )
		{
			terms_str.insert(terms_str.begin(),AQLString("6M"));
			rates.insert(rates.begin(), libMkt.sixMLibor);
			++startPos;
			map_freq_Fix[6] = SEMI_ANNUAL;
		}
	}
	else if ( freq_Float == QUARTERLY )
	{
		if ( terms_str[0] != AQLString("3M") )
		{
			terms_str.insert(terms_str.begin(),AQLString("3M"));
			rates.insert(rates.begin(), libMkt.threeMLibor);
			++startPos;
			map_freq_Fix[3] = QUARTERLY;
		}
	}
	else
	{
		throw AQLCoreInvalidData("swap floating leg frequecy is not supported.", __FILE__, __LINE__);
	}

	isSwapTenorAdjust = false;
	dh = &mktData[0]->getData(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isSwapTenorAdjust = dynamic_cast<const AQLDataBool&> (dh->get()).get();
	}

	if (isSwapTenorAdjust)
	{
		if (tenorSwapMktData.size() == 0) throw AQLCoreInvalidData("No Tenor Swap Market Object", __FILE__, __LINE__);
		
		bool isAgtSpread = dynamic_cast<const AQLDataBool&> ((tenorSwapMktData[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL)).get());
		if ( freq_Float == SEMI_ANNUAL )
		{
			AQLPriceDataDayCount dc_Float_six;
			if (isAgtSpread)
			{
				dc_Float_six = dynamic_cast<const AQLPriceDataDayCount&> ((tenorSwapMktData[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL)).get());
			}
			else
			{
				dc_Float_six = dynamic_cast<const AQLPriceDataDayCount&> ((tenorSwapMktData[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL)).get());
			}
			if (!(dc_Float_six == dc_Float)) throw AQLCoreInvalidData("daycount is not consistent.", __FILE__, __LINE__);
		}
		else if ( freq_Float == QUARTERLY )
		{
			AQLPriceDataDayCount dc_Float_three;
			if (isAgtSpread)
			{
				dc_Float_three = dynamic_cast<const AQLPriceDataDayCount&> ((tenorSwapMktData[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL)).get());
			}
			else
			{
				dc_Float_three = dynamic_cast<const AQLPriceDataDayCount&> ((tenorSwapMktData[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL)).get());
			}
			if (!(dc_Float_three == dc_Float)) throw AQLCoreInvalidData("daycount is not consistent.", __FILE__, __LINE__);
		}
		DoubleArray basisRates;
		AQLStringVector terms_str_basis;
		for (size_t i=0; i<tenorSwapMktData.size(); i++)
		{
			double rate = dynamic_cast<const AQLDataDouble&> ((tenorSwapMktData[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			AQLString term = dynamic_cast<const AQLDataString&> ((tenorSwapMktData[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			basisRates.push_back(rate);
			terms_str_basis.push_back(term);
		}
		
		map<AQLString, double> map_36basis;
		if (basisRates.size() != terms_str_basis.size()) 
		{
			throw AQLCoreInvalidData("size of 36 basis rate and size of 36 basis term are not same.", __FILE__, __LINE__);
		}
		else
		{
			for (size_t i=0; i<terms_str_basis.size(); i++)
			{
				map_36basis[terms_str_basis[i]] = basisRates[i];
			}
		}
		
		int y,m,d,w;
		for (size_t i=startPos; i<terms_str.size(); i++)
		{
			AQLDateCalculations::termStrtoYMDW(terms_str[i],y,m,d,w);
			if (d!=0 || w!=0) throw AQLCoreInvalidData("a format of swap term is not supported.", __FILE__, __LINE__);

			dh = &mktData[i-startPos]->getData(IR_CALIBRATION_DATA_FREQUENCY, NOCHECK);
			if (dh->isDefined() && !dh->isNull())
			{
				map_freq_Fix[12*y + m] = dynamic_cast<const AQLDataString&> (dh->get()).get();
			}
			else
			{
				map_freq_Fix[12*y + m] = freq_Fix;
			}

			dh = &mktData[i-startPos]->getData(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, ISNOTNULL);
			const AQLString& freq_float_irregular = dynamic_cast<const AQLDataString&> (dh->get()).get();
			if (freq_float_irregular == freq_Float) continue;
			if (map_36basis.find(terms_str[i]) == map_36basis.end())
			{
				throw AQLCoreInvalidData("a term of swap is not in 36 basis terms in frequency change.", __FILE__, __LINE__);
			}

			if (freq_Float == SEMI_ANNUAL || freq_float_irregular == QUARTERLY)
			{
				rates[i] += map_36basis[terms_str[i]];
				mktRates[i-startPos] += map_36basis[terms_str[i]];
			}
			else if (freq_Float == QUARTERLY || freq_float_irregular == SEMI_ANNUAL)
			{
				rates[i] -= map_36basis[terms_str[i]];
				mktRates[i-startPos] -= map_36basis[terms_str[i]];
			}
		}
	}

	DoubleArray terms;
	for(size_t i=0; i < terms_str.size(); i++)
    {
        AQLDate gridDate = AQLDateCalculations::getDate(spotDate, terms_str[i], sld, pCal, true, &roll_conv);
        terms.push_back( dc_act365.getTerm(basedate, gridDate) );
    }
	pInter = dynamic_cast<AQLInterpolationBase*> ((dynamic_cast<const AQLPriceDataInterpolation&> 
		(mktData[0]->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get())).getMethod().clone());
	pInter->set(terms,rates);

	endTerm = terms_str.back();
	endDate = AQLDateCalculations::getDate(spotDate, terms_str.back(), sld, pCal, true, &roll_conv);

	generateSchedule(basedate, spotDate, endDate, 3, pCal, sld, dateGrid_3MRoll, termGrid_3MRoll, isEOMRoll);  
    generateSchedule(basedate, spotDate, endDate, 6, pCal, sld, dateGrid_6MRoll, termGrid_6MRoll, isEOMRoll);  
	generateSchedule(basedate, spotDate, endDate, 12, pCal, sld, dateGrid_12MRoll, termGrid_12MRoll, isEOMRoll);  
}

AQLPriceArbFreeGenerator::SwapMarket::SwapMarket(const SwapMarket& v)
:	mktTerms_str(v.mktTerms_str), mktTerms(v.mktTerms), mktRates(v.mktRates), spotDate(v.spotDate), startDate(v.startDate), 
	endDate(v.endDate), endTerm(v.endTerm), sld(v.sld), pCal(v.pCal), freq_Float(v.freq_Float), freq_Fix(v.freq_Fix), freq_Float_Pay(v.freq_Float_Pay),
	dc_Float(v.dc_Float), dc_Fix(v.dc_Fix), map_freq_Fix(v.map_freq_Fix), isEOMRoll(v.isEOMRoll), roll_conv(v.roll_conv), 
	optimizeMethod(v.optimizeMethod), isSwapTenorAdjust(v.isSwapTenorAdjust), dateGrid_3MRoll(v.dateGrid_3MRoll), 
	dateGrid_6MRoll(v.dateGrid_6MRoll), dateGrid_12MRoll(v.dateGrid_12MRoll), termGrid_3MRoll(v.termGrid_3MRoll), 
	termGrid_6MRoll(v.termGrid_6MRoll), termGrid_12MRoll(v.termGrid_12MRoll)
{
	pInter = dynamic_cast<AQLInterpolationBase*>( v.pInter->clone() );
}

AQLPriceArbFreeGenerator::SwapMarket::~SwapMarket()
{
	delete pInter;
}

AQLPriceArbFreeGenerator::XCCYBasisMarket::
XCCYBasisMarket( vector<const AQLObject* > mktData, const AQLObject* fYieldData, const SwapMarket& swapMkt, bool& isRenAdj, bool isUSD,
				 const AQLDate& basedate )
{
	pInter = NULL;
	pInter_usd = NULL;
	pInter_usd_3ML = NULL;
	pInter_fPrices = NULL;
	pInter_baseccydf = NULL;
	pInter_adjust = NULL;

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	double swapEndTerm = dc_act365.getTerm(swapMkt.spotDate, swapMkt.endDate);

	const AQLDataHolder* dh;
	if (!isUSD)
	{
		if (mktData.size() == 0) throw AQLCoreInvalidData("No Swap Market Object", __FILE__, __LINE__);

		pCal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL)).get());
		sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL)).get());
		spotDate = dynamic_cast<const AQLDataDate&> ((mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL)).get()).get();
		dc = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL)).get());

		isEOMRoll = false;
		dh = &(mktData[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
		roll_conv = isEOMRoll ? ROLLCONV_EOM : ROLLCONV_NORMAL;

		for (size_t i=0; i<mktData.size(); i++)
		{
			double rate = dynamic_cast<const AQLDataDouble&> ((mktData[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
			AQLString term = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			mktRates.push_back(rate);
			mktTerms_str.push_back(term);
		}
		
		for(size_t i=0; i < mktRates.size(); i++)
		{
			AQLDate gridDate = AQLDateCalculations::getDate(spotDate, mktTerms_str[i], sld, pCal, true, &roll_conv);
			mktTerms.push_back( dc_act365.getTerm(basedate, gridDate) );
		}
		pInter = dynamic_cast<AQLInterpolationBase*> ((dynamic_cast<const AQLPriceDataInterpolation&> 
			(mktData[0]->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get())).getMethod().clone());
		pInter->set(mktTerms,mktRates);

		endDate = AQLDateCalculations::getDate(spotDate, swapMkt.endTerm, sld, pCal, true, &roll_conv);

		dh = &(mktData[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& adjustValue_term = dynamic_cast<const AQLDataDoubles&> (dh->get()).get();
			const DoubleArray& adjustValue
				= dynamic_cast<const AQLDataDoubles&> ((mktData[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUE, ISNOTNULL)).get()).get();
			pInter_adjust = dynamic_cast<AQLInterpolationBase*> ((dynamic_cast<const AQLPriceDataInterpolation&> 
				(mktData[0]->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION, ISNOTNULL).get())).getMethod().clone());
			pInter_adjust->set(adjustValue_term,adjustValue);
		}

		AQLString suffix;
		AQLString fCurveName = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
		if (fCurveName.findString(DUMMY) >= 0)
		{
			isRenAdj = false;

			DoubleArray fTerms;
			DoubleArray fPrices;
			// floaterPrices is 0
			int y,m,d,w;
			AQLDateCalculations::termStrtoYMDW(swapMkt.endTerm,y,m,d,w);
			unsigned int endTerm = 12 * y + m;
			for (size_t i=0; i<endTerm+1; i++)
			{
				fTerms.push_back(static_cast<double >(i));
				fPrices.push_back(1.);
			}
			pInter_fPrices = new AQLLinearInterpolation(); 
			pInter_fPrices->set(fTerms, fPrices);
		}
		else
		{
			if (!fYieldData)
			{
				throw AQLCoreInvalidData("Foreign yield data is null!", __FILE__, __LINE__);
			}

			if (isRenAdj) 
			{	
				fCurveName = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_AGTDISCOUNT, ISNOTNULL).get()).get();

				if (fCurveName != STD)
				{
					suffix = AQLString("_") +  fCurveName;
				}
				else
				{
					suffix = "";
				}
				const DoubleArray& terms_usd = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
				const DoubleArray& dfs_usd = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
				pInter_usd = dynamic_cast<AQLInterpolationBase*>(dynamic_cast<const AQLPriceDataInterpolation& > 
					((fYieldData->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL)).get()).getMethod().clone());
				if (swapEndTerm > terms_usd.back())
				{
					DoubleArray tmpTerms_usd = terms_usd;
					DoubleArray tmpDfs_usd = dfs_usd;
					tmpTerms_usd.push_back(swapEndTerm);
					tmpDfs_usd.push_back(dfs_usd.back());
					pInter_usd->set(tmpTerms_usd,tmpDfs_usd);
				}
				else
				{
					pInter_usd->set(terms_usd, dfs_usd);
				}

				fCurveName = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
				if (fCurveName != STD)
				{
					suffix = AQLString("_") +  fCurveName;
				}
				else
				{
					suffix = "";
				}
				const DoubleArray& terms_usd_3ML = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
				const DoubleArray& dfs_usd_3ML = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
				pInter_usd_3ML = dynamic_cast<AQLInterpolationBase*>(dynamic_cast<const AQLPriceDataInterpolation& > 
					((fYieldData->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL)).get()).getMethod().clone());
				if (swapEndTerm > terms_usd_3ML.back())
				{
					DoubleArray tmpTerms_usd_3ML = terms_usd_3ML;
					DoubleArray tmpDfs_usd_3ML = dfs_usd_3ML;
					tmpTerms_usd_3ML.push_back(swapEndTerm);
					tmpDfs_usd_3ML.push_back(dfs_usd_3ML.back());
					pInter_usd_3ML->set(tmpTerms_usd_3ML, tmpDfs_usd_3ML);
				}
				else
				{
					pInter_usd_3ML->set(terms_usd_3ML, dfs_usd_3ML);
				}
			}
			else
			{
				fCurveName = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
				
				if (fCurveName != STD)
				{
					suffix = AQLString("_") +  fCurveName;
				}
				else
				{
					suffix = "";
				}
				const DoubleArray& fTerms = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
				const DoubleArray& fPrices = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
				pInter_fPrices = dynamic_cast<AQLInterpolationBase*>(dynamic_cast<const AQLPriceDataInterpolation& > 
					((fYieldData->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL)).get()).getMethod().clone()); 
				if (swapEndTerm > fTerms.back())
				{
					DoubleArray tmpFTerms = fTerms;
					DoubleArray tmpFPrices = fPrices;
					tmpFTerms.push_back(swapEndTerm);
					tmpFPrices.push_back(fPrices.back());
					pInter_fPrices->set(tmpFTerms,tmpFPrices);
				}
				else
				{
					pInter_fPrices->set(fTerms, fPrices);
				}
			}
		}
	}
	else
	{
		if (mktData.size() == 0) return;

		AQLString suffix;
		AQLString fCurveName;
		if (isRenAdj) 
		{
			if (!fYieldData)
			{
				throw AQLCoreInvalidData("Foreign yield data is null!", __FILE__, __LINE__);
			}

			fCurveName = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_AGTDISCOUNT, ISNOTNULL).get()).get();
			if (fCurveName == DUMMY)
			{
				throw AQLCoreInvalidData("do not set renotional adjust when the forecast curve is dummy!", __FILE__, __LINE__);
			}
			else if (fCurveName != STD)
			{
				suffix = AQLString("_") +  fCurveName;
			}
			else
			{
				suffix = "";
			}
			const DoubleArray& terms = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray& dfs = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
			pInter_baseccydf = dynamic_cast<AQLInterpolationBase*> (dynamic_cast<const AQLPriceDataInterpolation& > 
				((fYieldData->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL)).get()).getMethod().clone());
			if (swapEndTerm > terms.back())
			{
				DoubleArray tmpTerms = terms;
				DoubleArray tmpDs = dfs;
				tmpTerms.push_back(swapEndTerm);
				tmpDs.push_back(dfs.back());
				pInter_baseccydf->set(tmpTerms,tmpDs);
			}
			else
			{
				pInter_baseccydf->set(terms,dfs);
			}
		}
		
		fCurveName = dynamic_cast<const AQLDataString &>(mktData[0]->getData(IR_CALIBRATION_DATA_AGTFORECAST, ISNOTNULL).get()).get();
		if (fCurveName.findString(DUMMY) >= 0)
		{
			DoubleArray fTerms;
			DoubleArray fPrices;
			// floaterPrices is 0
			int y,m,d,w;
			AQLDateCalculations::termStrtoYMDW(swapMkt.endTerm,y,m,d,w);
			unsigned int endTerm = 12 * y + m;
			for (size_t i=0; i<endTerm+1; i++)
			{
				fTerms.push_back(static_cast<double >(i));
				fPrices.push_back(1.);
			}
			pInter_fPrices = new AQLLinearInterpolation(); 
			pInter_fPrices->set(fTerms, fPrices);

			pCal = swapMkt.pCal;
			sld = swapMkt.sld;
			spotDate = swapMkt.spotDate;
		}
		else
		{
			if (!fYieldData)
			{
				throw AQLCoreInvalidData("Foreign yield data is null!", __FILE__, __LINE__);
			}

			if (fCurveName != STD)
			{
				suffix = AQLString("_") +  fCurveName;
			}
			else
			{
				suffix = "";
			}
			const DoubleArray& fTerms = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(CALIBRATION_DATA_TERMS + suffix, ISNOTNULL)).get()).get();
			const DoubleArray& fPrices = dynamic_cast<const AQLDataDoubles& > ((fYieldData->getData(IR_CALIBRATION_DATA_DFS + suffix, ISNOTNULL)).get()).get();
			pInter_fPrices = dynamic_cast<AQLInterpolationBase*>(dynamic_cast<const AQLPriceDataInterpolation& > 
				((fYieldData->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL)).get()).getMethod().clone()); 
			if (swapEndTerm > fTerms.back())
			{
				DoubleArray tmpFTerms = fTerms;
				DoubleArray tmpFPrices = fPrices;
				tmpFTerms.push_back(swapEndTerm);
				tmpFPrices.push_back(fPrices.back());
				pInter_fPrices->set(tmpFTerms,tmpFPrices);
			}
			else
			{
				pInter_fPrices->set(fTerms, fPrices);
			}

			pCal = &dynamic_cast<const AQLPriceDataCalendar&> ((fYieldData->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR + suffix, ISNOTNULL)).get());
			sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((fYieldData->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE + suffix, ISNOTNULL)).get());
			spotDate = dynamic_cast<const AQLDataDate&> ((fYieldData->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE + suffix, ISNOTNULL)).get()).get();

			isEOMRoll = false;
			dh = &(fYieldData->getData(IR_CALIBRATION_DATA_ISEOMROLL + suffix, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
			}
			roll_conv = isEOMRoll ? ROLLCONV_EOM : ROLLCONV_NORMAL;

			mktTerms_str = dynamic_cast<const AQLDataStrings& > ((fYieldData->getData(IR_CALIBRATION_DATA_XCCYBASISTERM + suffix, ISNOTNULL)).get()).get();
			for (size_t i=0; i<mktTerms_str.size(); i++)
			{
				AQLDate gridDate = AQLDateCalculations::getDate(spotDate, mktTerms_str[i], sld, pCal, true, &roll_conv);
				mktTerms.push_back( dc_act365.getTerm(basedate, gridDate) );
			}

			const AQLDataHolder *dh = &(fYieldData->getData(IR_CALIBRATION_DATA_ADJUSTVALUETERM + suffix, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DoubleArray& adjustValue_term = dynamic_cast<const AQLDataDoubles&> (dh->get()).get();
				const DoubleArray& adjustValue
					= dynamic_cast<const AQLDataDoubles&> ((fYieldData->getData(IR_CALIBRATION_DATA_ADJUSTVALUE + suffix, ISNOTNULL)).get()).get();
				pInter_adjust = dynamic_cast<AQLInterpolationBase*> ((dynamic_cast<const AQLPriceDataInterpolation&> 
					(fYieldData->getData(IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION + suffix, ISNOTNULL).get())).getMethod().clone());
				pInter_adjust->set(adjustValue_term,adjustValue);
			}
		}

		endDate = AQLDateCalculations::getDate(spotDate, swapMkt.endTerm, sld, pCal, true, &roll_conv);
		dc = swapMkt.dc_Float;
	}

	generateSchedule(basedate, spotDate, endDate, 3, pCal, sld, dateGrid_3MRoll, termGrid_3MRoll, isEOMRoll);
}

AQLPriceArbFreeGenerator::XCCYBasisMarket::XCCYBasisMarket(const XCCYBasisMarket& v)
:	mktTerms_str(v.mktTerms_str), mktTerms(v.mktTerms), mktRates(v.mktRates), spotDate(v.spotDate), endDate(v.endDate), 
	dc(v.dc), sld(v.sld), pCal(v.pCal), isEOMRoll(v.isEOMRoll), roll_conv(v.roll_conv), dateGrid_3MRoll(v.dateGrid_3MRoll), 
	termGrid_3MRoll(v.termGrid_3MRoll)
{
	pInter = NULL;
	pInter_usd = NULL;
	pInter_usd_3ML = NULL;
	pInter_fPrices = NULL;
	pInter_baseccydf = NULL;
	pInter_adjust = NULL;

	if (v.pInter) pInter = dynamic_cast<AQLInterpolationBase*>( v.pInter->clone() );
	if (v.pInter_usd) pInter_usd = dynamic_cast<AQLInterpolationBase*>( v.pInter_usd->clone() );
	if (v.pInter_usd_3ML) pInter_usd_3ML = dynamic_cast<AQLInterpolationBase*>( v.pInter_usd_3ML->clone() );
	if (v.pInter_fPrices) pInter_fPrices = dynamic_cast<AQLInterpolationBase*>( v.pInter_fPrices->clone() );
	if (v.pInter_baseccydf) pInter_baseccydf = dynamic_cast<AQLInterpolationBase*>( v.pInter_baseccydf->clone() );
	if (v.pInter_adjust) pInter_adjust = dynamic_cast<AQLInterpolationBase*>( v.pInter_adjust->clone() );
}

AQLPriceArbFreeGenerator::XCCYBasisMarket::~XCCYBasisMarket()
{
	if (pInter) delete pInter;
	if (pInter_usd) delete pInter_usd;	
	if (pInter_usd_3ML) delete pInter_usd_3ML;
	if (pInter_fPrices) delete pInter_fPrices;
	if (pInter_baseccydf) delete pInter_baseccydf;
	if (pInter_adjust) delete pInter_adjust;
}

AQLPriceArbFreeGenerator::LiborBasisMarket::
LiborBasisMarket( vector<const AQLObject* > mktData, const SwapMarket& swapMkt, const AQLDate& basedate )
{
	if (mktData.size() == 0) throw AQLCoreInvalidData("No Swap Market Object", __FILE__, __LINE__);
	const AQLDataHolder *dh;

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	bool isAgtSpread = dynamic_cast<const AQLDataBool&> ((mktData[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL)).get());
	pCal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETCALENDAR, ISNOTNULL)).get());
	sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, ISNOTNULL)).get());
	spotDate = dynamic_cast<const AQLDataDate&> ((mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETSPOTDATE, ISNOTNULL)).get()).get();
	//check frequency
	AQLString freq = dynamic_cast<const AQLDataString&> ((mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCY, ISNOTNULL)).get()).get();
	AQLString freq_against = dynamic_cast<const AQLDataString&> ((mktData[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY, ISNOTNULL)).get()).get();
	freq_3L_pay = QUARTERLY;
	if (isAgtSpread)
	{
		if (freq != SEMI_ANNUAL || freq_against != QUARTERLY)
			throw AQLCoreInvalidData("ArbFree curve does not support a case of spread on the 6M leg of 36 basis swap!", __FILE__, __LINE__);
		dh = &mktData[0]->getData(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND, NOCHECK); 
		if (dh->isDefined() && !dh->isNull()) 
		{ 
			freq_3L_pay = dynamic_cast<const AQLDataString&> (dh->get());
		}
	}
	else
	{
		if (freq != QUARTERLY || freq_against != SEMI_ANNUAL)
			throw AQLCoreInvalidData("ArbFree curve does not support a case of spread on the 6M leg of 36 basis swap!", __FILE__, __LINE__);
		dh = &mktData[0]->getData(IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND, NOCHECK); 
		if (dh->isDefined() && !dh->isNull()) 
		{ 
			freq_3L_pay = dynamic_cast<const AQLDataString&> (dh->get());
		}
	}

	if (freq_3L_pay != QUARTERLY && freq_3L_pay != SEMI_ANNUAL)
		throw AQLCoreInvalidData("ArbFree curve supports a case that payment of the 3M leg of 36 basis swap is quarterly or semi-annual!", __FILE__, __LINE__);

	isEOMRoll = false;
	dh = &(mktData[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	roll_conv = isEOMRoll ? ROLLCONV_EOM : ROLLCONV_NORMAL;

    if (isAgtSpread)
	{
		dc_3L = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL)).get());
		dc_6L = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL)).get());
	}
	else
	{
		dc_3L = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL)).get());
		dc_6L = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL)).get());
	}

	for (size_t i = 0; i < mktData.size(); ++i)
	{
		double rate = dynamic_cast<const AQLDataDouble&> ((mktData[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLString term = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		mktRates.push_back(rate);
		mktTerms_str.push_back(term);
	}

    for(size_t i=0; i < mktRates.size(); ++i)
    {
		AQLDate gridDate = AQLDateCalculations::getDate(swapMkt.spotDate, mktTerms_str[i], swapMkt.sld, swapMkt.pCal, true, &roll_conv);
        mktTerms.push_back( dc_act365.getTerm(basedate, gridDate) );
    }
	pInter = dynamic_cast<AQLInterpolationBase*> ((dynamic_cast<const AQLPriceDataInterpolation&> 
		(mktData[0]->getData(CALIBRATION_DATA_INTERPOLATION, ISNOTNULL).get())).getMethod().clone());
	pInter->set(mktTerms,mktRates);

	endDate = AQLDateCalculations::getDate(spotDate, swapMkt.endTerm, swapMkt.sld, swapMkt.pCal, true, &roll_conv);

	generateSchedule(basedate, spotDate, endDate, 3, pCal, sld, dateGrid_3MRoll, termGrid_3MRoll, isEOMRoll);  
	generateSchedule(basedate, spotDate, endDate, 6, pCal, sld, dateGrid_6MRoll, termGrid_6MRoll, isEOMRoll);  
}

AQLPriceArbFreeGenerator::LiborBasisMarket::LiborBasisMarket(const LiborBasisMarket& v)
:	mktTerms_str(v.mktTerms_str), mktTerms(v.mktTerms), mktRates(v.mktRates), dc_3L(v.dc_3L), dc_6L(v.dc_6L),
	isEOMRoll(v.isEOMRoll), roll_conv(v.roll_conv), 
	dateGrid_3MRoll(v.dateGrid_3MRoll), dateGrid_6MRoll(v.dateGrid_6MRoll),	termGrid_3MRoll(v.termGrid_3MRoll), 
	termGrid_6MRoll(v.termGrid_6MRoll), sld(v.sld), pCal(v.pCal), spotDate(v.spotDate), endDate(v.endDate), freq_3L_pay(v.freq_3L_pay)
{
	pInter = dynamic_cast<AQLInterpolationBase*>( v.pInter->clone() );
}

AQLPriceArbFreeGenerator::LiborBasisMarket::~LiborBasisMarket()
{
	delete pInter;
}

AQLPriceArbFreeGenerator::FRAMarket::
FRAMarket( vector<const AQLObject* > mktData, bool isFRAUse )
{
	if (!isFRAUse || mktData.size() == 0) return;

	DoubleArray rates;
	AQLStringVector terms_str;
	for (size_t i=0; i<mktData.size(); i++)
	{
		double rate = dynamic_cast<const AQLDataDouble&> ((mktData[i]->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
		AQLString term = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
		rates.push_back(rate);
		terms_str.push_back(AQLPriceYieldGenerator::changeFRATermFormat(term));
	}
	//const DoubleArray& rates = dynamic_cast<const AQLDataDoubles&> ((mktData->getData(CALIBRATION_DATA_RATE, ISNOTNULL)).get()).get();
    //const AQLStringVector& terms = dynamic_cast<const AQLDataStrings&> ((mktData->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
    
	if( rates.size() != terms_str.size() )
    throw AQLCoreInvalidData("size of libor market rates and size of libor market terms are not same.", __FILE__, __LINE__);

	for(size_t i=0; i<rates.size(); i++)
    {
        map_term_rate.insert( map<AQLString, double >::value_type(terms_str[i], rates[i]) );
    }     
	
	spotDate = dynamic_cast<const AQLDataDate&> ((mktData[0]->getData(IR_CALIBRATION_DATA_SPOTDATE, ISNOTNULL)).get()).get();
	sld = dynamic_cast<const AQLPriceDataSlidingRule&> ((mktData[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());
    dc = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
    pCal = &dynamic_cast<const AQLPriceDataCalendar&> ((mktData[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	isEOMRoll = false;
	const AQLDataHolder *dh = &(mktData[0]->getData(IR_CALIBRATION_DATA_ISEOMROLL, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isEOMRoll = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	}
	roll_conv = isEOMRoll ? ROLLCONV_EOM : ROLLCONV_NORMAL;
}

AQLPriceArbFreeGenerator::FRAMarket::~FRAMarket(){}

AQLPriceArbFreeGenerator::FutureMarket::
FutureMarket( vector<const AQLObject* > mktData, const SwapMarket& swapMkt, bool isFutureUse, const AQLDate& basedate )
{
	if (!isFutureUse || mktData.size() == 0) return;

	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	AQLDate firstSwapDate = AQLDateCalculations::getDate(swapMkt.spotDate, swapMkt.mktTerms_str[0], swapMkt.sld, swapMkt.pCal, true, &swapMkt.roll_conv);

	AQLDate startDate, endDate;
	double rate;
	const AQLDataHolder* dh;
	for (size_t i=0; i<mktData.size(); i++)
	{
		startDate = dynamic_cast<const AQLDataDate&> ((mktData[i]->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
		endDate = dynamic_cast<const AQLDataDate&> ((mktData[i]->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
		if (endDate >= firstSwapDate) break;
		if (startDate <= swapMkt.spotDate) continue;

		startDates.push_back(startDate);
		endDates.push_back(endDate);

		dh = &(mktData[i]->getData(CALIBRATION_DATA_RATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) 
		{
			rate = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
		}
		else 
		{
			double price = dynamic_cast<const AQLDataDouble&> ((mktData[i]->getData(PRICING_DATA_PRICE, ISNOTNULL)).get()).get(); 
			rate = 1.0 - price * 0.01;
		}	

		// get spread
		double sp = 0.0;
		dh = &(mktData[i]->getData(PRICING_DATA_SPREAD, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) sp = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
		rate += sp;
		
		// get future volatility
		double f_vol = 0.0;
		dh = &(mktData[i]->getData(PRICING_DATA_FUTUREVOLATILITY, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) f_vol = dynamic_cast<const AQLDataDouble&> (dh->get()).get();
		
		RateConvention rc = AQLMathYieldCurve::setRC(SIMPLE);
		AQLPriceDataConvention conv(dc.getDayCount(), rc);
		if(f_vol > 0.0)
		{
			/*double start_term = dc_act365.getTerm(swapMkt.spotDate, startDate);
			double term = dc.getTerm(startDate, endDate, false);
			double term_rate = 1.0 / conv.getDF(rate, startDate, endDate) - 1.; 
			rate = (term_rate - 1) 
					+ AQLMath::sqrt((AQLMath::sqr(term_rate - 1) +
						   4 * term_rate * AQLMath::exp(AQLMath::sqr(f_vol) * start_term)));
			rate = rate / (2 * term * AQLMath::exp(AQLMath::sqr(f_vol) * start_term));*/
			//Convexity Adjust by Ho-Lee Model 
			double start_term = dc_act365.getTerm(basedate, startDate);
			double end_term = dc_act365.getTerm(basedate, endDate);
			rate -= 0.5 * start_term * end_term * f_vol * f_vol;
		}
		mktRates.push_back(rate);
	}

	dc  = dynamic_cast<const AQLPriceDataDayCount&> ((mktData[0]->getData(IR_CALIBRATION_DATA_DAYCOUNT, ISNOTNULL)).get());
}

AQLPriceArbFreeGenerator::FutureMarket::~FutureMarket(){}

AQLPriceArbFreeGenerator::AQLMathAFCurveCalibrator::AQLMathAFCurveCalibrator(
	const AQLDate& basedate_,
	const DoubleArray& threeMLTerms_DF_, 
	const DoubleArray& threeMLDF_, 
	const DoubleArray& sixMLTerms_DF_, 
	const DoubleArray& sixMLDF_, 
	const DoubleArray& dfTerms_, 
	const DoubleArray& dfCurve_, 
	bool isRenAdj_, 
	const LiborMarket& libMkt_, 
	const SwapMarket& swapMkt_, 
	const XCCYBasisMarket& xccyBasisMkt_, 
	const LiborBasisMarket& libBasisMkt_, 
	AQLInterpolationBase* pInter_3ML_, 
	AQLInterpolationBase* pInter_6ML_, 
	AQLInterpolationBase* pInter_DF_, 
	const size_t& shortTermSize_3ML_, 
	const size_t& shortTermSize_6ML_, 
	const size_t& shortTermSize_DF_, 
	bool isUSD_) :
		basedate(basedate_),
		threeMLTerms_DF(threeMLTerms_DF_), 
		threeMLDF(threeMLDF_), 
		sixMLTerms_DF(sixMLTerms_DF_), 
		sixMLDF(sixMLDF_), 
		dfTerms(dfTerms_), 
		dfCurve(dfCurve_), 
		isRenAdj(isRenAdj_),
		libMkt(libMkt_),
		swapMkt(swapMkt_),
		xccyBasisMkt(xccyBasisMkt_),
		libBasisMkt(libBasisMkt_),
		pInter_3ML(pInter_3ML_), 
		pInter_6ML(pInter_6ML_), 
		pInter_DF(pInter_DF_), 
		shortTermSize_3ML(shortTermSize_3ML_), 
		shortTermSize_6ML(shortTermSize_6ML_), 
		shortTermSize_DF(shortTermSize_DF_), 
		isUSD(isUSD_)
{}

void
AQLPriceArbFreeGenerator::AQLMathAFCurveCalibrator::operator()(DoubleArray& f, const DoubleArray& x)
{
	unsigned int pos = 0;

	for (size_t i = 0; i < x.size(); ++i)
	{
		if ( i < swapMkt.mktTerms.size() )
		{
			if (swapMkt.freq_Float == SEMI_ANNUAL)
			{
				pos = shortTermSize_6ML + i;
				sixMLDF[pos] = x[i];
			}
			else if (swapMkt.freq_Float == QUARTERLY)
			{
				pos = shortTermSize_3ML + i;
				threeMLDF[pos] = x[i];
			}	
		}
		else if ( i < swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size())
		{
			if (swapMkt.freq_Float == SEMI_ANNUAL)
			{
				pos = shortTermSize_3ML + i - swapMkt.mktTerms.size();
				threeMLDF[pos] = x[i];
			}
			else if (swapMkt.freq_Float == QUARTERLY)
			{
				pos = shortTermSize_6ML + i - swapMkt.mktTerms.size();
				sixMLDF[pos] = x[i];
			}	
		}
		else
		{
			pos = shortTermSize_DF + i - swapMkt.mktTerms.size() - libBasisMkt.mktTerms.size();
			dfCurve[pos] = x[i];
		}
	}

	pInter_3ML->set(threeMLTerms_DF, threeMLDF);
	pInter_6ML->set(sixMLTerms_DF, sixMLDF);
	pInter_DF->set(dfTerms, dfCurve);
	
	DoubleArray swapValue;
	f.clear();
	swapValue = getSwapValue(libMkt, swapMkt, swapMkt.dateGrid_3MRoll, swapMkt.termGrid_3MRoll, swapMkt.dateGrid_6MRoll, 
		swapMkt.termGrid_6MRoll, swapMkt.dateGrid_12MRoll, swapMkt.termGrid_12MRoll, pInter_3ML, pInter_6ML, pInter_DF);
	f.insert(f.begin(), swapValue.begin(), swapValue.end());
	swapValue = getLibBasisValue(libMkt, swapMkt, libBasisMkt, libBasisMkt.dateGrid_3MRoll, libBasisMkt.termGrid_3MRoll, 
		libBasisMkt.dateGrid_6MRoll, libBasisMkt.termGrid_6MRoll, pInter_3ML,pInter_6ML, pInter_DF);
	f.insert(f.begin() + swapMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );
	swapValue = getXccyBasisValue(basedate, isRenAdj, libMkt, swapMkt, xccyBasisMkt, xccyBasisMkt.dateGrid_3MRoll, 
		xccyBasisMkt.termGrid_3MRoll, pInter_3ML, pInter_6ML, pInter_DF, isUSD);
	f.insert(f.begin() + swapMkt.mktTerms.size() + libBasisMkt.mktTerms.size(), swapValue.begin(), swapValue.end() );
}

bool
AQLPriceArbFreeGenerator::AQLMathAFCurveCalibrator::constraintsAreViolated( const DoubleArray& x )
{
	for (size_t i=0; i<x.size(); i++)
	{
		if (x[i] < 0. || x[i] > 2.) return true;
	}

    return false;
}

/*
    @brief set a rate convention into a curve data object
    
    @param[in] objHolder	object holder
	@param[in] mktData	market object
	@param[in] curveName          
*/
void
AQLPriceArbFreeGenerator::setCurveConvention(	AQLObjectHolder& objHolder,
											std::vector<AQLObject*>& mktData,
											const AQLStringVector& curveNames_3ML,
											const AQLStringVector& curveNames_6ML)
{
	const AQLDataHolder *dh;

	vector<const AQLObject*> data_swap, data_libor, data_3m6m;
	for(unsigned i = 0; i < mktData.size(); i++)
	{
		// check use grid
		dh = &mktData[i]->getData(IR_CALIBRATION_DATA_GRIDUSEFLAG, NOCHECK);
		if (dh->isDefined() && !dh->isNull() && !dynamic_cast<const AQLDataBool &>(dh->get()).get()) continue;

		AQLString dataType = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		dataType.toUpper();

		AQLString dataName = dynamic_cast<const AQLDataString&> ((mktData[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL)).get()).get();
		dataName.toUpper();

		if (dataType == ZERO) data_libor.push_back(mktData[i]);//libor case
		else if (dataType == PAR) data_swap.push_back(mktData[i]);//swap case
		else if (dataType == BASIS && dataName.findString(THREESIXBASIS) != -1) data_3m6m.push_back(mktData[i]);//3m6m basis swap
	}

	if (data_libor.size() == 0) throw AQLCoreInvalidData("No Libor Market Object", __FILE__, __LINE__);
	if (data_swap.size() == 0) throw AQLCoreInvalidData("No Swap Market Object", __FILE__, __LINE__);
	if (data_3m6m.size() == 0) throw AQLCoreInvalidData("No 3M6M Basis Swap Market Object", __FILE__, __LINE__);

	AQLString baseFreq = dynamic_cast<const AQLDataString&> ((data_swap[0]->getData(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, ISNOTNULL)).get());
	const AQLPriceDataCalendar* cal = &dynamic_cast<const AQLPriceDataCalendar&> ((data_libor[0]->getData(CALIBRATION_DATA_CALENDAR, ISNOTNULL)).get());
	const AQLPriceDataSlidingRule* sld = &dynamic_cast<const AQLPriceDataSlidingRule&> ((data_libor[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());

	const AQLPriceDataDayCount* dc_3ml;
	const AQLPriceDataDayCount* dc_6ml;
	bool isAgtSpread = dynamic_cast<const AQLDataBool&> ((data_3m6m[0]->getData(IR_CALIBRATION_DATA_ISAGTSPREAD, ISNOTNULL)).get());
	if (isAgtSpread)
	{
		dc_3ml = &dynamic_cast<const AQLPriceDataDayCount&> ((data_3m6m[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL)).get());
		dc_6ml = &dynamic_cast<const AQLPriceDataDayCount&> ((data_3m6m[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL)).get());
	}
	else
	{
		dc_6ml = &dynamic_cast<const AQLPriceDataDayCount&> ((data_3m6m[0]->getData(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT, ISNOTNULL)).get());
		dc_3ml = &dynamic_cast<const AQLPriceDataDayCount&> ((data_3m6m[0]->getData(IR_CALIBRATION_DATA_INDEXDAYCOUNT, ISNOTNULL)).get());
	}
	//const AQLPriceDataSlidingRule* sld = &dynamic_cast<const AQLPriceDataSlidingRule&> ((data_swap[0]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL)).get());

	bool isSTDExist = false;
	AQLString suffix = "";
	for (size_t i = 0; i < curveNames_6ML.size(); i++)
	{	
		if (curveNames_6ML[i] == STD)
		{
			suffix = "";
			isSTDExist = true;
		}
		else
		{
			suffix = "_" + curveNames_6ML[i];
		}

		objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
		objHolder.remove(CALIBRATION_DATA_CALENDAR + suffix);
		objHolder.remove(CALIBRATION_DATA_SLIDINGRULE + suffix);
		objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
		objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
		
		objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new AQLDataString(SIMPLE));
		objHolder.add(CALIBRATION_DATA_CALENDAR + suffix, new AQLPriceDataCalendar(*cal));
		objHolder.add(CALIBRATION_DATA_SLIDINGRULE + suffix, new AQLPriceDataSlidingRule(*sld));
		objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT + suffix, new AQLPriceDataDayCount(*dc_6ml));
		objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new AQLDataString("6M"));
	}

	for (size_t i = 0; i < curveNames_3ML.size(); i++)
	{
		if(curveNames_3ML[i] == STD)
		{
			suffix = "";
			isSTDExist = true;
		}
		else
		{
			suffix = "_" + curveNames_3ML[i];
		}

		objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
		objHolder.remove(CALIBRATION_DATA_CALENDAR + suffix);
		objHolder.remove(CALIBRATION_DATA_SLIDINGRULE + suffix);
		objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
		objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
		
		objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new AQLDataString(SIMPLE));
		objHolder.add(CALIBRATION_DATA_CALENDAR + suffix, new AQLPriceDataCalendar(*cal));
		objHolder.add(CALIBRATION_DATA_SLIDINGRULE + suffix, new AQLPriceDataSlidingRule(*sld));
		objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT + suffix, new AQLPriceDataDayCount(*dc_3ml));
		objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new AQLDataString("3M"));
	}
	
	if (!isSTDExist)
	{
		if ( baseFreq == SEMI_ANNUAL )
		{
			objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY);
			objHolder.remove(CALIBRATION_DATA_CALENDAR);
			objHolder.remove(CALIBRATION_DATA_SLIDINGRULE);
			objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT);
			objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY);
			
			objHolder.add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString(SIMPLE));
			objHolder.add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(*cal));
			objHolder.add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule(*sld));
			objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount(*dc_6ml));
			objHolder.add(IR_CALIBRATION_DATA_ACCESSARY, new AQLDataString("6M"));
		}
		else if ( baseFreq == QUARTERLY )
		{
			objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY);
			objHolder.remove(CALIBRATION_DATA_CALENDAR);
			objHolder.remove(CALIBRATION_DATA_SLIDINGRULE);
			objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT);
			objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY);
			
			objHolder.add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString(SIMPLE));
			objHolder.add(CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(*cal));
			objHolder.add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule(*sld));
			objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount(*dc_3ml));
			objHolder.add(IR_CALIBRATION_DATA_ACCESSARY, new AQLDataString("3M"));
		}
		else
		{
			throw AQLCoreInvalidData("swap floating leg frequecy is not supported.", __FILE__, __LINE__);
		}
	}
}	