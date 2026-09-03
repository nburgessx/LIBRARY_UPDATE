/*
 * @brief			Swap component curve used in the global curve calibration engine
 * @Created:		16 Jan 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "GlobalCalibrationComponentCurve.h"
#include "LAObject.h"
#include "LADataReference.h"
#include "CurveCalibrationData.h"
#include <map>

class LAInterpolationBase;
class LADate;

/*! 
    @brief Class of SwapComponentCurve
*/
class SwapComponentCurve : public GlobalCalibrationComponentCurve
{
public:
	
	// Constructor
	SwapComponentCurve(const LAString& curveName, const CurveCalibrationData& curveCalibrationData, const LADate& baseDate, const bool& fastRebuildRequested = false);

	// Destructor
	virtual ~SwapComponentCurve() {}
    
	// Initialisation
	virtual void initialise();
		
	// Calibration routine by pricing instruments
	virtual void priceCalibrationInstruments(DoubleArray& allPVs);
	
	// Post processing results once instruments have been consumed in calibration steps
	virtual void postProcessing(LAObject& yieldCurveProEntity);

private:

	// Start the calibration process by calibrating using simple cash and forward instruments
	void calibrateSwapCurveWithCashAndForwards();

	// Calculate the floating leg PV of a vanilla IRS
	double calcFloatLegPV(std::shared_ptr<LAInterpolationBase>& inter, 
					std::shared_ptr<LAInterpolationBase>& df_inter, 
					const DoubleArray &terms_grid, 
					const int cpd_times, 
					const double term_start,
					const DoubleArray& floatAccrualPeriods = DoubleVector(),
					const DoubleArray& fixingStarts = DoubleVector(),
					const DoubleArray& fixingEnds = DoubleVector(),
					const DoubleArray& fixingTaus = DoubleVector() ) const;

	/////////////////////////////////////////
		
	LADate spotDate_;
	
	std::vector<LAObject*> data_;

	LADataReference yieldDataRef_;

	/////////////////////////////////////////

	LAString interpolationStr_;
	LAString interpolationYGStr_;

	std::shared_ptr<LAInterpolationBase> pInter_;
	std::shared_ptr<LAInterpolationBase> pInter_yg_;
	std::shared_ptr<LAInterpolationBase> pInter_fw_;	

	double interpolationJoinDateAsDouble_;
	LADate interpolationJoinDate_;

	/////////////////////////////////////////

	bool isSwapTenorAdjust_;
	bool is_f_use_;
	bool is_fra_use_;
	bool is_fwdswap_;
	bool isMultiCurve_;	
	bool generateForwardsFromSwapsOnly_;

	/////////////////////////////////////////

	// Curve Result Placeholders
	DiscountFactors dfResults_;
	DoubleMatrix fwd_termsmtx_;
	DoubleArray fwds_;

	std::vector<DoubleArray> rate_tauVec_;
	std::vector<DoubleArray> rate_tauVec_float_;
	std::vector<DoubleArray> rate_tauVec_ts_;
	std::vector<DoubleArray> terms_gridVec_;
	std::vector<DoubleArray> terms_gridVec_float_;
	std::vector<DoubleArray> terms_gridVec_ts_;
	std::vector<DoubleArray> terms_interval_float_;
	std::vector<DoubleArray> fixingStarts_;
	std::vector<DoubleArray> fixingEnds_;
	std::vector<DoubleArray> fixingTaus_;
	std::vector<DateVector> fixingStartDates_;
	std::vector<DateVector> fixingEndDates_;
	std::vector<DateVector> datesVec_float_;
	std::vector<DateVector> datesVec_;
	DoubleArray terms_grid_s_float_;										

	std::vector<int> cpd_times_;	
	std::map< std::pair<LADate, LADate>, const LAObject* > moneyMarketDataMap_;
	std::shared_ptr<LAStringVector> pRatePriority_;

	LAPriceDataDayCount dc_Libor_;
	
};

// Custom object type when used with shared pointer
typedef std::shared_ptr<SwapComponentCurve> SwapComponentCurvePtr;


