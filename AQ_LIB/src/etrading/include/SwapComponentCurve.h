#pragma once

#include "GlobalCalibrationComponentCurve.h"
#include "AQLObject.h"
#include "AQLDataReference.h"
#include "CurveCalibrationData.h"
#include <map>

class AQLInterpolationBase;
class AQLDate;

/*! 
    @brief Class of SwapComponentCurve
*/
class SwapComponentCurve : public GlobalCalibrationComponentCurve
{
public:
	
	// Constructor
	SwapComponentCurve(const AQLString& curveName, const CurveCalibrationData& curveCalibrationData, const AQLDate& baseDate, const bool& fastRebuildRequested = false);

	// Destructor
	virtual ~SwapComponentCurve() {}
    
	// Initialisation
	virtual void initialise();
		
	// Calibration routine by pricing instruments
	virtual void priceCalibrationInstruments(DoubleArray& allPVs);
	
	// Post processing results once instruments have been consumed in calibration steps
	virtual void postProcessing(AQLObject& yieldCurveProEntity);

private:

	// Start the calibration process by calibrating using simple cash and forward instruments
	void calibrateSwapCurveWithCashAndForwards();

	// Calculate the floating leg PV of a vanilla IRS
	double calcFloatLegPV(std::shared_ptr<AQLInterpolationBase>& inter, 
					std::shared_ptr<AQLInterpolationBase>& df_inter, 
					const DoubleArray &terms_grid, 
					const int cpd_times, 
					const double term_start,
					const DoubleArray& floatAccrualPeriods = DoubleVector(),
					const DoubleArray& fixingStarts = DoubleVector(),
					const DoubleArray& fixingEnds = DoubleVector(),
					const DoubleArray& fixingTaus = DoubleVector() ) const;

	/////////////////////////////////////////
		
	AQLDate spotDate_;
	
	std::vector<AQLObject*> data_;

	AQLDataReference yieldDataRef_;

	/////////////////////////////////////////

	AQLString interpolationStr_;
	AQLString interpolationYGStr_;

	std::shared_ptr<AQLInterpolationBase> pInter_;
	std::shared_ptr<AQLInterpolationBase> pInter_yg_;
	std::shared_ptr<AQLInterpolationBase> pInter_fw_;	

	double interpolationJoinDateAsDouble_;
	AQLDate interpolationJoinDate_;

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
	std::map< std::pair<AQLDate, AQLDate>, const AQLObject* > moneyMarketDataMap_;
	std::shared_ptr<AQLStringVector> pRatePriority_;

	AQLPriceDataDayCount dc_Libor_;
	
};

// Custom object type when used with shared pointer
typedef std::shared_ptr<SwapComponentCurve> SwapComponentCurvePtr;


