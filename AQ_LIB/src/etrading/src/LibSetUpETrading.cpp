#define _HAS_STD_BYTE 0

/*
 * @brief			Methods that assist in library initialization
 *					It was ported over from InitializeVanilla.h/.cpp.
 */


// TODO: unsolved problem
#ifndef PRICING_DATA_SPOTDATE
#define PRICING_DATA_SPOTDATE				"SpotDate"
#endif
#ifndef CALIBRATION_DATA_MV_DFS
#define CALIBRATION_DATA_MV_DFS					"DiscountFactors"
#endif

#include "LibSetUpETrading.h"
#include "AQLCoreSystemError.h"
#include "AQLDate.h"
#include "AQLDataInstance.h"
#include "AQLObjectHolder.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataProcedure.h"
#include "AQLDataValuation.h"
#include "AQLCoreTemplateType.h"
#include "AQLPriceDataManager.h"
#include "AQLDataValuation.h"
#include "AQLDataBasics.h"
#include "AQLDataMultiReference.h"
#include "AQLDataReference.h"
#include "AQLFunctionManager.h"
#include "AQLDataMatrix.h"

#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLCalendarSet.h"
#include "AQLMathValuableEntity.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLMathValuableEntity.h"
#include "AQLPriceDataRand.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataConvention.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataFunction.h"
#include "AQLPriceDataFunctions.h"
#include "AQLMathPathEntity.h"
#include "AQLMathIndexEntity.h"
#include "AQLMathFXEntity.h"
#include <AQLPriceArbFreeGenerator.h>
#include "AQLCentralBank.h"
#include "ExceptionMacros.h"

#include "AQLBasic.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLParabolicInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLMonotoneSplineInterpolation.h"
#include "AQL1DDataSet.h"
#include "AQLConstant.h"
#include "AQLGaussLegendre.h"
#include "AQLRomberg.h"
#include "AQLLinearFunc.h"
#include "AQLMaxFunc.h"
#include "AQLMinFunc.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLPricePayOff.h"
#include "AQLPriceCouponRainbow.h"
#include "AQLDataCSVFileLoader.h"
#include "AQLCurvePricingObject.h"
#include "AQLString.h"
#include <iomanip>

#include <istream>
#include <fstream>
#include <sstream>
#include <cassert>
#include "ConstantDeclarations.h"

#include "AQLMathPlainVanillaEntity.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLPriceConvergenceValue.h"
#include "AQLCompoundingFunc.h"

#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLCoreComponentManager.h"

// fukui
#include "AQLPriceAccruedInterest.h"
#include "AQLPriceTradeValue.h"
#include "AQLStaticDataImport.h"

#include "CurveCalibrationData.h"
#include "CurveCalibration.h"
#include "FolderConfig.h"

using namespace std;

namespace 
{
	void readFileOfCalendarFormat(ifstream& fin, AQLStringVector& namevec, vector<DateVector>& dvec)
	{
		char c[2048];
		fin.getline(c,INT_MAX,'\n');
		AQLString name(c);
		namevec = name.toToken(',');
		const int N = namevec.size();
		dvec.clear();
		dvec.reserve(N);
		for (int i = 0; i < N; ++i)
			dvec.push_back(DateVector());
	
		while(fin.getline(c,INT_MAX,'\n'))
		{
			AQLString str(c);
			AQLStringVector vec = str.toToken(',');
			const int N = vec.size();
				for(int i=0;i<N;i++)
				{
					if (!vec[i].size())
					{
						continue;
					}
					AQLDate tmp(vec[i].getCString());
					dvec[i].push_back(tmp);
				}
		}
	}
}

//namespace etrading
//{
	void libSetUpETrading(AQLDataInstance* dataInstance, const bool checkIfCalendarLoaded)
	{
		//set Data
		AQLPriceDataManager &dm = dataInstance->getDataMaster();
		dm.setData(new AQLDataBool());
		dm.setData(new AQLDataString());
		dm.setData(new AQLDataInt());
		dm.setData(new AQLDataDouble());
		dm.setData(new AQLDataDate());
		dm.setData(new AQLDataDateTime());
		dm.setData(new AQLDataBools());
		dm.setData(new AQLDataStrings());
		dm.setData(new AQLDataInts());
		dm.setData(new AQLDataDoubles());
		dm.setData(new AQLDataDates());
		dm.setData(new AQLDataDateTimes());
		dm.setData(new AQLDataDoubleMatrix());
		dm.setData(new AQLDataReference());
		dm.setData(new AQLDataMultiReference());
		dm.setData(new AQLDataValuation());
		dm.setData(new AQLDataProcedure());
		dm.setData(new AQLPriceDataCalendar());
		dm.setData(new AQLPriceDataDayCount());
		dm.setData(new AQLPriceDataConvention());
		dm.setData(new AQLPriceDataSlidingRule());
		dm.setData(new AQLPriceDataInterpolation());
		dm.setData(new AQLPriceDataRand());
		dm.setData(new AQLPriceDataFunction());
		dm.setData(new AQLPriceDataFunctions());

		dm.setData(CALIBRATION_DATA_MV_DFS,			DATA_DOUBLES);
		dm.setData(PRICING_DATA_SPOTDATE,			DATA_DATE);
		dm.setData(PRICING_DATA_SPREAD,				DATA_DOUBLES);
		dm.setData(CALIBRATION_DATA_RATE,			DATA_DOUBLE);
		dm.setData(IR_CALIBRATION_DATA_DATATYPE,	DATA_STRING);
		dm.setData(IR_CALIBRATION_DATA_TERM,		DATA_STRING);
		dm.setData(PRICING_DATA_FREQUENCY,			DATA_STRING);
		dm.setData(PRICING_DATA_PRICE,				DATA_DOUBLE);

		dataInstance->getObjectMaster().setEntity(new AQLMathYieldCurve(dataInstance));
		dataInstance->getObjectMaster().setEntity(new CurveCalibrationData(dataInstance));

		//set Function
		AQLFunctionManager& fm = dataInstance->getFunctionMaster();
		//Yield Generator
		CurveCalibration* pCalibrationEngine = new CurveCalibration();
		fm.setFunction(pCalibrationEngine,FN_IRYIELDGENERATOR_STR);
		//Arbitrage Free Curve Generator
		AQLPriceArbFreeGenerator* afyld = new AQLPriceArbFreeGenerator();
		fm.setFunction(afyld,FN_IRARBFREEGENERATOR_STR);

		// *** Interpolation Factory ***
		fm.setFunction(new AQLSplineInterpolation(),								FN_SPLINEINTERPOLATION_STR);            // Defaults to Natural Spline
        fm.setFunction(new AQLSplineInterpolation(true),							FN_NATURALSPLINEINTERPOLATION_STR);     // Use Natural Spline = true
        fm.setFunction(new AQLSplineInterpolation(false),						FN_CLAMPEDSPLINEINTERPOLATION_STR);     // Use Clamped Spline = false
		fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_MONOTONESPLINEINTERPOLATION_STR);     // Monotone Cubic
		fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_MONOTONEPARABOLICINTERPOLATION_STR);  // Monotone Parabolic
        fm.setFunction(new AQLParabolicInterpolation(),							FN_PARABOLICINTERPOLATION_STR);
        fm.setFunction(new AQLLinearSplineInterpolation(),						FN_LINEARSPLINEINTERPOLATION_STR);
		fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_LINEARMONOTONESPLINEINTERPOLATION_STR);
		fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_LINEARMONOTONEPARABOLICINTERPOLATION_STR);
        fm.setFunction(new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	FN_LINEARINTERPOLATION_STR);
		fm.setFunction(new AQLStepInterpolation(),								FN_STEPINTERPOLATION_STR);
		fm.setFunction(new AQLStepInterpolation(StepType::RIGHT_CONTINUOUS),		FN_RIGHTCONTINUOUS_INTERPOLATION_STR);
		fm.setFunction(new AQLStepInterpolation(StepType::LEFT_CONTINUOUS),		FN_LEFTCONTINUOUS_INTERPOLATION_STR);
		fm.setFunction(new AQLConstrainedSplineInterpolation(),					FN_CONSTRAINEDSPLINEINTERPOLATION_STR);
		fm.setFunction(new AQLMonotoneConvexInterpolation(),						FN_MONOTONECONVEXINTERPOLATION_STR);

		// *** Interpolation Factory Using Alias Names ***
		fm.setFunction(new AQLSplineInterpolation(),								SPLINE_INTERP );             // Defaults to Natural Spline
        fm.setFunction(new AQLSplineInterpolation(true),							NATURAL_SPLINE_INTERP );     // Use Natural Spline = true
        fm.setFunction(new AQLSplineInterpolation(false),						CLAMPED_SPLINE_INTERP );     // Use Clamped Spline = false
		fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		MONOTONE_SPLINE_INTERP );     // Monotone Cubic
		fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	MONOTONE_PARABOLIC_INTERP );  // Monotone Parabolic
        fm.setFunction(new AQLParabolicInterpolation(),							PARABOLIC_INTERP );
        fm.setFunction(new AQLLinearSplineInterpolation(),						LINEAR_SPLINE_INTERP );
		fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		LINEAR_MONOTONE_SPLINE_INTERP );
		fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	LINEAR_MONOTONE_PARABOLIC_INTERP );
        fm.setFunction(new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	LINEAR_INTERP );
		fm.setFunction(new AQLStepInterpolation(),								STEP_INTERP );
		fm.setFunction(new AQLStepInterpolation(StepType::RIGHT_CONTINUOUS),		RIGHT_CONTINUOUS_INTERP );
		fm.setFunction(new AQLStepInterpolation(StepType::LEFT_CONTINUOUS),		LEFT_CONTINUOUS_INTERP );
		fm.setFunction(new AQLConstrainedSplineInterpolation(),					CONSTRAINED_SPLINE_INTERP );
		fm.setFunction(new AQLMonotoneConvexInterpolation(),						MONOTONE_CONVEX_INTERP );

		fm.setFunction(new AQLLinearMethod(), FN_LINEAR_STR);
		fm.setFunction(new AQLConstant(), FN_CONSTANT_STR);
		fm.setFunction(new AQLMaxMethod(), FN_MAX_STR);
		fm.setFunction(new AQLMinMethod(), FN_MIN_STR);
		fm.setFunction(new AQLMathBasisFunction(), FN_BASISFUNC1_STR);
		fm.setFunction(new AQLMathBasisFunction2(), FN_BASISFUNC2_STR);

		dataInstance->getObjectMaster().setEntity(new AQLMathPlainVanillaEntity(dataInstance));
		AQLLinearRatesSwapTradeValue* psval = new AQLLinearRatesSwapTradeValue(new AQLPriceAccruedInterest());

		psval->registerData(dm);
		fm.setFunction(psval, FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);

		AQLPriceCashFlowGenerator* pcf = new AQLPriceCashFlowGenerator();
		pcf->registerData(dm);
		fm.setFunction(pcf, FN_IR_CASHFLOWGENERATOR_STR);

		AQLPriceConvergenceValue* pconv = new AQLPriceConvergenceValue();
		pconv->registerData(dm);
		fm.setFunction(pconv, FN_IR_CONVERGENCEVALUE_STR);

		fm.setFunction(new AQLCompoundMethod1, FN_COMPOUNDING1_STR);
		fm.setFunction(new AQLCompoundMethod8, FN_COMPOUNDING8_STR);
		fm.setFunction(new AQLCompoundMethod10, FN_COMPOUNDING10_STR);

		//add for MSUSA
		//this setting is for outputfunction in Excel
		dm.setData("PV", DATA_DOUBLE);
		dm.setData("Zero delta", DATA_DOUBLE);
		dm.setData("Source delta", DATA_DOUBLE);
		dm.setData("gamma", DATA_DOUBLE);
		dm.setData(PRICING_DATA_PV_LEG1, DATA_DOUBLE);
		dm.setData(PRICING_DATA_PV_LEG2, DATA_DOUBLE);
	
		for (int num = 1; num < 3; num++)
		{
			dm.setData(PRICING_DATA_CFCALCSTARTDATE_LEG + AQLString( num ), DATA_DATES);
			dm.setData(PRICING_DATA_CFCALCENDDATE_LEG + AQLString( num ), DATA_DATES);
			dm.setData(PRICING_DATA_PAYMENTDATE_LEG + AQLString( num ), DATA_DATES);
			dm.setData(PRICING_DATA_CASHLETVALUETIME_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_CASHLETVALUE_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_PVVALUE_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_DF_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_FORWARD_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_SPREAD_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_ACCRUALDAYS_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_CALIBRATION_DATAOTIONAL_LEG + AQLString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_FIXINGDATE_LEG + AQLString( num ), DATA_DATES);
		}

        // Load Calendar Files and Throw on Failure
        // ----------------------------------------
		
        // Mandatory - typically we throw if the calendar is not loaded
        setupCalendarETrading( nullptr, checkIfCalendarLoaded ); // nullptr = don't override filepath, use preset filepath,
		
        // Optional - Don't Throw if not loaded
        setupCBScheduleETrading( nullptr, false ); // nullptr = don't override filepath, use preset filepath, enable throw = false
	}

	void setupCalendarETrading(const AQLString* file_path, const bool enableThrow)
	{
		if(file_path!=NULL) etrading::FolderConfig::set_calendar_path(*file_path);
		const AQLString* fname = etrading::FolderConfig::calendar_path();
		
        if( enableThrow )
        {
            AQ_REQUIRE( fname != NULL, "Calendar file not loaded; invalid filepath provided" );
        }
        else
        {
            // Silent throw
            if (fname == NULL ) return;
        }

		ifstream fin;
		fin.open(fname->getCString());
		if(!fin.is_open()) 
		{
			std::stringstream sst;
            sst << "#Error: Cannot open calendar file: " << std::endl
				<< fname 
				;
			AQ_THROW( sst.str().c_str() );
		}
		AQLStringVector namevec;
		std::vector<DateVector> dvec;
		readFileOfCalendarFormat(fin, namevec, dvec);
		const int N = namevec.size();

		for(int i=0; i<N;i++)
		{
			AQLCalendar cal;
			cal.setWeekly(SUN);
			cal.setWeekly(SAT);
			cal.setDate(dvec[i]);
			AQLCalendarSet calSet;
			calSet.setCalendarData(namevec[i],cal);
		}
		fin.close();
	}

	void setupCBScheduleETrading(const AQLString* file_path, const bool enableThrow)
	{
		if(file_path!=NULL) etrading::FolderConfig::set_cbschedule_path(*file_path);
		const AQLString* fname = etrading::FolderConfig::cbschedule_path();
		
        if( enableThrow )
        {
            AQ_REQUIRE( fname != NULL, "Central Bank Calendar file not loaded; invalid filepath provided" );
        }
        else
        {
            // Silent throw
            if (fname == NULL ) return;
        }

		ifstream fin;
		fin.open(fname->getCString());
		if(!fin.is_open()) 
		{
			std::stringstream sst;
            sst << "#Error: Cannot open schedule file: " << std::endl
				<< fname;
			AQ_THROW( sst.str().c_str() );
		}

		AQLStringVector namevec;
		std::vector<DateVector> dvec;
		readFileOfCalendarFormat(fin, namevec, dvec);

		assert(namevec.size() == dvec.size());

		AQLCentralBank::Schedules& scheds = AQLCentralBank::schedules();
		scheds.clear();
		for (size_t j = 0; j != namevec.size(); ++j)
        {
			scheds[namevec[j].toUpper()] = dvec[j];
		}

		fin.close();	// not actually necessary...
	}
//}