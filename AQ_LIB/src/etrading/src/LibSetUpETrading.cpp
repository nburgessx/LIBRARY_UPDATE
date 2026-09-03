#define _HAS_STD_BYTE 0

/*
 * @brief			Methods that assist in library initialization
 *					It was ported over from InitializeMLibVanilla.h/.cpp.
 * @Created:		14 November 2016
 * @Author:			
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


///Hishida 0313 unsolved problem /////////////////////////
#ifndef PRICING_DATA_SPOTDATE
#define PRICING_DATA_SPOTDATE				"SpotDate"
#endif
#ifndef CALIBRATION_DATA_MV_DFS
#define CALIBRATION_DATA_MV_DFS					"DiscountFactors"
#endif

#include "LibSetUpETrading.h"
#include "LACoreSystemError.h"
#include "LADate.h"
#include "LADataInstance.h"
#include "LAObjectHolder.h"
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataProcedure.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"
#include "LAPriceDataManager.h"
#include "LADataValuation.h"
#include "LADataBasics.h"
#include "LADataMultiReference.h"
#include "LADataReference.h"
#include "LAFunctionManager.h"
#include "LADataMatrix.h"

#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathCalendarSet.h"
#include "LAMathValuableEntity.h"
#include "LAPriceDataInterpolation.h"
#include "LAMathValuableEntity.h"
#include "LAPriceDataRand.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataConvention.h"
#include "LAMathDefine.h"
#include "LAPriceDataFunction.h"
#include "LAPriceDataFunctions.h"
#include "LAMathPathEntity.h"
#include "LAMathIndexEntity.h"
#include "LAMathFXEntity.h"
#include <LAPriceArbFreeGenerator.h>
#include "LAMathCentralBank.h"
#include "ExceptionMacros.h"

#include "LABasic.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAConstrainedSplineInterpolation.h"
#include "LAParabolicInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "LAMonotoneConvexInterpolation.h"
#include "LAMonotoneSplineInterpolation.h"
#include "LA1DDataSet.h"
#include "LAConstant.h"
#include "LAGaussLegendre.h"
#include "LARomberg.h"
#include "LALinearFunc.h"
#include "LAMaxFunc.h"
#include "LAMinFunc.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAPricePayOff.h"
#include "LAPriceCouponRainbow.h"
#include "LADataCSVFileLoader.h"
#include "LACurvePricingObject.h"
#include "LAString.h"
#include <iomanip>

#include <istream>
#include <fstream>
#include <sstream>
#include <cassert>
#include "ConstantDeclarations.h"

#include "LAMathPlainVanillaEntity.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LAPriceConvergenceValue.h"
#include "LACompoundingFunc.h"

#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LACoreComponentManager.h"

// fukui
#include "LAPriceAccruedInterest.h"
#include "LAPriceTradeValue.h"
#include "LAStaticDataImport.h"

#include "CurveCalibrationData.h"
#include "CurveCalibration.h"
#include "FolderConfig.h"

using namespace std;

namespace 
{
	void readFileOfCalendarFormat(ifstream& fin, LAStringVector& namevec, vector<DateVector>& dvec)
	{
		char c[2048];
		fin.getline(c,INT_MAX,'\n');
		LAString name(c);
		namevec = name.toToken(',');
		const int N = namevec.size();
		dvec.clear();
		dvec.reserve(N);
		for (int i = 0; i < N; ++i)
			dvec.push_back(DateVector());
	
		while(fin.getline(c,INT_MAX,'\n'))
		{
			LAString str(c);
			LAStringVector vec = str.toToken(',');
			const int N = vec.size();
				for(int i=0;i<N;i++)
				{
					if (!vec[i].size())
					{
						continue;
					}
					LADate tmp(vec[i].getCString());
					dvec[i].push_back(tmp);
				}
		}
	}
}

//namespace etrading
//{
	void libSetUpETrading(LADataInstance* dataInstance, const bool checkIfCalendarLoaded)
	{
		//set Data
		LAPriceDataManager &dm = dataInstance->getDataMaster();
		dm.setData(new LADataBool());
		dm.setData(new LADataString());
		dm.setData(new LADataInt());
		dm.setData(new LADataDouble());
		dm.setData(new LADataDate());
		dm.setData(new LADataDateTime());
		dm.setData(new LADataBools());
		dm.setData(new LADataStrings());
		dm.setData(new LADataInts());
		dm.setData(new LADataDoubles());
		dm.setData(new LADataDates());
		dm.setData(new LADataDateTimes());
		dm.setData(new LADataDoubleMatrix());
		dm.setData(new LADataReference());
		dm.setData(new LADataMultiReference());
		dm.setData(new LADataValuation());
		dm.setData(new LADataProcedure());
		dm.setData(new LAPriceDataCalendar());
		dm.setData(new LAPriceDataDayCount());
		dm.setData(new LAPriceDataConvention());
		dm.setData(new LAPriceDataSlidingRule());
		dm.setData(new LAPriceDataInterpolation());
		dm.setData(new LAPriceDataRand());
		dm.setData(new LAPriceDataFunction());
		dm.setData(new LAPriceDataFunctions());

		dm.setData(CALIBRATION_DATA_MV_DFS,			DATA_DOUBLES);
		dm.setData(PRICING_DATA_SPOTDATE,			DATA_DATE);
		dm.setData(PRICING_DATA_SPREAD,				DATA_DOUBLES);
		dm.setData(CALIBRATION_DATA_RATE,			DATA_DOUBLE);
		dm.setData(IR_CALIBRATION_DATA_DATATYPE,	DATA_STRING);
		dm.setData(IR_CALIBRATION_DATA_TERM,		DATA_STRING);
		dm.setData(PRICING_DATA_FREQUENCY,			DATA_STRING);
		dm.setData(PRICING_DATA_PRICE,				DATA_DOUBLE);

		dataInstance->getObjectMaster().setEntity(new LAMathYieldCurve(dataInstance));
		dataInstance->getObjectMaster().setEntity(new CurveCalibrationData(dataInstance));

		//set Function
		LAFunctionManager& fm = dataInstance->getFunctionMaster();
		//Yield Generator
		CurveCalibration* pCalibrationEngine = new CurveCalibration();
		fm.setFunction(pCalibrationEngine,FN_IRYIELDGENERATOR_STR);
		//Arbitrage Free Curve Generator
		LAPriceArbFreeGenerator* afyld = new LAPriceArbFreeGenerator();
		fm.setFunction(afyld,FN_IRARBFREEGENERATOR_STR);

		// *** Interpolation Factory ***
		fm.setFunction(new LASplineInterpolation(),								FN_SPLINEINTERPOLATION_STR);            // Defaults to Natural Spline
        fm.setFunction(new LASplineInterpolation(true),							FN_NATURALSPLINEINTERPOLATION_STR);     // Use Natural Spline = true
        fm.setFunction(new LASplineInterpolation(false),						FN_CLAMPEDSPLINEINTERPOLATION_STR);     // Use Clamped Spline = false
		fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_MONOTONESPLINEINTERPOLATION_STR);     // Monotone Cubic
		fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_MONOTONEPARABOLICINTERPOLATION_STR);  // Monotone Parabolic
        fm.setFunction(new LAParabolicInterpolation(),							FN_PARABOLICINTERPOLATION_STR);
        fm.setFunction(new LALinearSplineInterpolation(),						FN_LINEARSPLINEINTERPOLATION_STR);
		fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_LINEARMONOTONESPLINEINTERPOLATION_STR);
		fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_LINEARMONOTONEPARABOLICINTERPOLATION_STR);
        fm.setFunction(new LALinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	FN_LINEARINTERPOLATION_STR);
		fm.setFunction(new LAStepInterpolation(),								FN_STEPINTERPOLATION_STR);
		fm.setFunction(new LAStepInterpolation(StepType::RIGHT_CONTINUOUS),		FN_RIGHTCONTINUOUS_INTERPOLATION_STR);
		fm.setFunction(new LAStepInterpolation(StepType::LEFT_CONTINUOUS),		FN_LEFTCONTINUOUS_INTERPOLATION_STR);
		fm.setFunction(new LAConstrainedSplineInterpolation(),					FN_CONSTRAINEDSPLINEINTERPOLATION_STR);
		fm.setFunction(new LAMonotoneConvexInterpolation(),						FN_MONOTONECONVEXINTERPOLATION_STR);

		// *** Interpolation Factory Using Alias Names ***
		fm.setFunction(new LASplineInterpolation(),								SPLINE_INTERP );             // Defaults to Natural Spline
        fm.setFunction(new LASplineInterpolation(true),							NATURAL_SPLINE_INTERP );     // Use Natural Spline = true
        fm.setFunction(new LASplineInterpolation(false),						CLAMPED_SPLINE_INTERP );     // Use Clamped Spline = false
		fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		MONOTONE_SPLINE_INTERP );     // Monotone Cubic
		fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	MONOTONE_PARABOLIC_INTERP );  // Monotone Parabolic
        fm.setFunction(new LAParabolicInterpolation(),							PARABOLIC_INTERP );
        fm.setFunction(new LALinearSplineInterpolation(),						LINEAR_SPLINE_INTERP );
		fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		LINEAR_MONOTONE_SPLINE_INTERP );
		fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	LINEAR_MONOTONE_PARABOLIC_INTERP );
        fm.setFunction(new LALinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	LINEAR_INTERP );
		fm.setFunction(new LAStepInterpolation(),								STEP_INTERP );
		fm.setFunction(new LAStepInterpolation(StepType::RIGHT_CONTINUOUS),		RIGHT_CONTINUOUS_INTERP );
		fm.setFunction(new LAStepInterpolation(StepType::LEFT_CONTINUOUS),		LEFT_CONTINUOUS_INTERP );
		fm.setFunction(new LAConstrainedSplineInterpolation(),					CONSTRAINED_SPLINE_INTERP );
		fm.setFunction(new LAMonotoneConvexInterpolation(),						MONOTONE_CONVEX_INTERP );

		fm.setFunction(new LALinearMethod(), FN_LINEAR_STR);
		fm.setFunction(new LAConstant(), FN_CONSTANT_STR);
		fm.setFunction(new LAMaxMethod(), FN_MAX_STR);
		fm.setFunction(new LAMinMethod(), FN_MIN_STR);
		fm.setFunction(new LAMathBasisFunction(), FN_BASISFUNC1_STR);
		fm.setFunction(new LAMathBasisFunction2(), FN_BASISFUNC2_STR);

		dataInstance->getObjectMaster().setEntity(new LAMathPlainVanillaEntity(dataInstance));
		LALinearRatesSwapTradeValue* psval = new LALinearRatesSwapTradeValue(new LAPriceAccruedInterest());

		psval->registerData(dm);
		fm.setFunction(psval, FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);

		LAPriceCashFlowGenerator* pcf = new LAPriceCashFlowGenerator();
		pcf->registerData(dm);
		fm.setFunction(pcf, FN_IR_CASHFLOWGENERATOR_STR);

		LAPriceConvergenceValue* pconv = new LAPriceConvergenceValue();
		pconv->registerData(dm);
		fm.setFunction(pconv, FN_IR_CONVERGENCEVALUE_STR);

		fm.setFunction(new LACompoundMethod1, FN_COMPOUNDING1_STR);
		fm.setFunction(new LACompoundMethod8, FN_COMPOUNDING8_STR);
		fm.setFunction(new LACompoundMethod10, FN_COMPOUNDING10_STR);

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
			dm.setData(PRICING_DATA_CFCALCSTARTDATE_LEG + LAString( num ), DATA_DATES);
			dm.setData(PRICING_DATA_CFCALCENDDATE_LEG + LAString( num ), DATA_DATES);
			dm.setData(PRICING_DATA_PAYMENTDATE_LEG + LAString( num ), DATA_DATES);
			dm.setData(PRICING_DATA_CASHLETVALUETIME_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_CASHLETVALUE_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_PVVALUE_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_DF_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_FORWARD_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_SPREAD_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_ACCRUALDAYS_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_CALIBRATION_DATAOTIONAL_LEG + LAString( num ), DATA_DOUBLES);
			dm.setData(PRICING_DATA_FIXINGDATE_LEG + LAString( num ), DATA_DATES);
		}

        // Load Calendar Files and Throw on Failure
        // ----------------------------------------
		
        // Mandatory - typically we throw if the calendar is not loaded
        setupCalendarETrading( nullptr, checkIfCalendarLoaded ); // nullptr = don't override filepath, use preset filepath,
		
        // Optional - Don't Throw if not loaded
        setupCBScheduleETrading( nullptr, false ); // nullptr = don't override filepath, use preset filepath, enable throw = false
	}

	void setupCalendarETrading(const LAString* file_path, const bool enableThrow)
	{
		if(file_path!=NULL) etrading::FolderConfig::set_calendar_path(*file_path);
		const LAString* fname = etrading::FolderConfig::calendar_path();
		
        if( enableThrow )
        {
            MLIB_REQUIRE( fname != NULL, "Calendar file not loaded; invalid filepath provided" );
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
			throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		}
		LAStringVector namevec;
		std::vector<DateVector> dvec;
		readFileOfCalendarFormat(fin, namevec, dvec);
		const int N = namevec.size();

		for(int i=0; i<N;i++)
		{
			LAMathCalendar cal;
			cal.setWeekly(SUN);
			cal.setWeekly(SAT);
			cal.setDate(dvec[i]);
			LAMathCalendarSet calSet;
			calSet.setCalendarData(namevec[i],cal);
		}
		fin.close();
	}

	void setupCBScheduleETrading(const LAString* file_path, const bool enableThrow)
	{
		if(file_path!=NULL) etrading::FolderConfig::set_cbschedule_path(*file_path);
		const LAString* fname = etrading::FolderConfig::cbschedule_path();
		
        if( enableThrow )
        {
            MLIB_REQUIRE( fname != NULL, "Central Bank Calendar file not loaded; invalid filepath provided" );
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
			throw LACoreInvalidData(sst.str().c_str(), __FILE__, __LINE__);
		}

		LAStringVector namevec;
		std::vector<DateVector> dvec;
		readFileOfCalendarFormat(fin, namevec, dvec);

		assert(namevec.size() == dvec.size());

		LAMathCentralBank::Schedules& scheds = LAMathCentralBank::schedules();
		scheds.clear();
		for (size_t j = 0; j != namevec.size(); ++j)
        {
			scheds[namevec[j].toUpper()] = dvec[j];
		}

		fin.close();	// not actually necessary...
	}
//}