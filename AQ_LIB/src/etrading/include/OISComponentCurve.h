#pragma once

#include "GlobalCalibrationComponentCurve.h"
#include "AQLObject.h"
#include "AQLDataReference.h"
#include "CurveCalibrationData.h"
#include "CoreEnumerations.h"

class AQLInterpolationBase;
class AQLDate;

/*! 
    @brief Class of OISComponentCurve
*/
class OISComponentCurve : public GlobalCalibrationComponentCurve
{
public:
	
	// constructor
	OISComponentCurve(const AQLString& curveName, const CurveCalibrationData& curveCalibrationData, const AQLDate& baseDate, const bool& fastRebuildRequested = false);

	// destructor
	virtual ~OISComponentCurve() {}

	// Initialisation
	virtual void initialise();

	// Calibration routine by pricing instruments
	virtual void priceCalibrationInstruments(DoubleVector& allPVs);

	// Post processing results once instruments have been consumed in calibration steps
	virtual void postProcessing(AQLObject& yieldCurveProEntity);

	// Set the internal Libor rate interpolator
	void setLiborInterp(const std::shared_ptr<AQLInterpolationBase>& pInter_Libor);		

private:

	// Price the PV of a single OIS swap (either outright or Libor-OIS basis swap)
	double priceSingleOISSwapPV( double marketRate,
                                 double marketRate_s,
							     double& sumCF,
                                 double& sumT,
							     double& sumT_s,
                                 double _sumCF,
							     double _sumT,
                                 double _sumT_s,
							     size_t size_calc,
                                 size_t calced_size,			
							     size_t size_calc_s,
                                 size_t calced_size_s,
							     const DoubleVector& terms_grids,
                                 const DoubleVector& terms_intervals,
							     const DoubleVector& terms_grids_s,
                                 const DoubleVector& terms_intervals_s,
							     const DoubleVector& fixingTaus,
                                 const DateVector& fixingStartDates,
							     const DateVector& fixingEndDates, 
							     const etrading::OISCompoundingEnum& swapCompoundingMethodEnum,
                                 const etrading::OISLongTermInstrumentsEnum& longTermConveEnum,
                                 const AQLString& longTermGen,
							     const AQLPriceDataDayCount* dateCount,
                                 const AQLPriceDataCalendar* cal,
							     DoubleMatrix& startterms,
                                 DoubleMatrix& endterms,
							     bool isIRSParRateKnown = true,
                                 const DateVector& fixingStartDates_libor = DateVector(),
							     const DateVector& fixingEndDates_libor = DateVector(),
                                 const DoubleVector& fixingTaus_libor = DoubleVector(),
							     AQLInterpolationBase *libor_inter = nullptr ) const;
	
	AQLDate spotDate_;
	double spotterm_;
	
	/////////////////////////////////////////

	
	std::shared_ptr<AQLInterpolationBase> pInter_yg_;
	std::shared_ptr<AQLInterpolationBase> pInter_Libor_;	

	AQLString interpolationYGStr_;

	/////////////////////////////////////////
	
	std::vector<size_t> size_calcs_;
	std::vector<size_t> size_calcs_s_;
	std::vector<size_t> calced_sizes_;
	std::vector<size_t> calced_sizes_s_;
	DoubleMatrix terms_grids_;
	DoubleMatrix terms_grids_s_;
	DoubleMatrix terms_intervals_;
	DoubleMatrix terms_intervals_s_;
	std::vector<double> marketRates_;
	std::vector<double> marketRates_s_;
	std::vector<DateVector> fixingStartDates_ois_;
	std::vector<DateVector> fixingEndDates_ois_;
	std::vector<DoubleVector> fixingTaus_ois_;
	std::vector<DateVector> fixingStartDates_libor_;
	std::vector<DateVector> fixingEndDates_libor_;
	std::vector<DoubleVector> fixingTaus_libor_;
	std::vector<bool> isIRSParRateGiven_;

	std::vector<AQLString> term_strs_;
	std::vector<etrading::OISCompoundingEnum> swapCompoundingMethodEnums_;
	std::vector<etrading::OISLongTermInstrumentsEnum> longTermConvEnums_;
	std::vector<AQLString> longTermGens_;
	std::vector<bool> bLiborSwapCalcReset_;
	std::vector<bool> bOISSwapCalcReset_;
	std::vector<bool> bFullSigmaReset_;
	std::vector<const AQLPriceDataCalendar*> cals_;
	std::vector<const AQLPriceDataDayCount*> dateCounts_;

	std::vector<DoubleMatrix> dailyTerms_Start_;
	std::vector<DoubleMatrix> dailyTerms_End_;

	std::vector<AQLObject*> data_on_;
	std::vector<AQLObject*> data_tn_;
	std::vector<AQLObject*> data_;

	AQLDataReference yieldDataRef_;	
	
	/////////////////////////////////////////

};

// Custom object type when used with shared pointer
typedef std::shared_ptr<OISComponentCurve> OISComponentCurvePtr;