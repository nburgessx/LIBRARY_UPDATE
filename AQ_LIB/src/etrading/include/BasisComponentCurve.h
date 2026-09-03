#pragma once

#include "GlobalCalibrationComponentCurve.h"
#include "AQLObject.h"
#include "AQLDataReference.h"
#include "CurveCalibrationData.h"
#include <map>

class AQLInterpolationBase;
class AQLDate;

/*! 
    @brief Class of BasisComponentCurve
*/
class BasisComponentCurve : public GlobalCalibrationComponentCurve
{
public:
	
	// Constructor
	BasisComponentCurve(const AQLString& curveName, CurveCalibrationData& curveCalibrationData, const AQLDate& baseDate, const bool& fastRebuildRequested = false);

	// Destructor
	virtual ~BasisComponentCurve() {}
    
	// Initialisation
	virtual void initialise();
		
	// Calibration routine by pricing instruments
	virtual void priceCalibrationInstruments(DoubleArray& allPVs);
	
	// Post processing results once instruments have been consumed in calibration steps
	virtual void postProcessing(AQLObject& yieldCurveProEntity);

	// Set target leg DF interpolator	
	void setTargetLegDFInterpolator(const std::shared_ptr<AQLInterpolationBase>& dfInterp);

	// Set against leg DF interpolator	
	void setAgainstLegDFInterpolator(const std::shared_ptr<AQLInterpolationBase>& dfInterp);

	// Set target leg Forecast interpolator	
	void setTargetLegFwdInterpolator(const std::shared_ptr<AQLInterpolationBase>& fwdInterp);

	// Set against leg Forecast interpolator	
	void setAgainstLegFwdInterpolator(const std::shared_ptr<AQLInterpolationBase>& fwdInterp);

private:

	std::shared_ptr<AQLInterpolationBase> pInter_fwd_againstLeg;
	std::shared_ptr<AQLInterpolationBase> pInter_df_againstLeg;

	std::shared_ptr<AQLInterpolationBase> pInter_fwd_targetLeg;
	std::shared_ptr<AQLInterpolationBase> pInter_df_targetLeg;

	/////////////////////////////////////////

	AQLDate spotDate_;

	AQLDate asOfDate_;

	std::vector<AQLObject*> data_;

	AQLDataReference yieldDataRef_;

	CurveCalibrationData& curveCalibrationData_;

	DoubleArray extrapolateTerms_;

	AQLString spotRateTerm_;

	AQLDate liborDate_;

	AQLString firstInstrumentTerm_;

	AQLDate firstInstrumentDate_;

	double spotTerm_;

	double a_spotTerm_;

	double fxfwd_spotdf_;

	double liborRate_;

	double liborTerm_;

	double dfAdj_againstLeg_;

	unsigned int swap_count_;

	unsigned int ndf_size_;

	unsigned int fxfwd_size_;

	AQLString curveNameCaseless_;

	/////////////////////////////////////////

	AQLString interpolationStr_;

	std::shared_ptr<AQLInterpolationBase> pInter_adj_;

	int interpType_;

	/////////////////////////////////////////

	bool isDiscount_;
	bool isFwdRenotional_;
	bool isUSDleg_;
	bool isSpreadOnAgainstLeg_;
	bool is_fra_use_;
	bool isYieldSpreadCalc_;
	bool isFWDInter_;
	bool isFwdBasis_;

	/////////////////////////////////////////

	DoubleArray spreadVec_;						// Swap basis inputs
	
	DoubleArray effectiveStartGridVec_;			// Date fraction between spot date and effective start date - target leg
	DoubleArray a_effectiveStartGridVec_;			// Date fraction between spot date and effective start date - against leg

	std::vector<DoubleArray> gridVec_;			// Date fraction between spot date and payment dates of every swap - target leg
	std::vector<DoubleArray> a_gridVec_;		// Date fraction between spot date and payment dates of every swap - against leg
	
	std::vector<DoubleArray> tauVec_;			// Date fraction of each payment period of every swap - target leg
	std::vector<DoubleArray> a_tauVec_;			// Date fraction of each payment period of every swap - against leg
	
	std::vector<DoubleMatrix> i_gridMatVec_;	// Date fraction between asof date and fixing dates of every swap - target leg
	std::vector<DoubleMatrix> a_i_gridMatVec_;	// Date fraction between asof date and fixing dates of every swap - against leg
	
	std::vector<DoubleMatrix> i_termMatVec_;	// Date fraction of each indexing period of every swap - target leg
	std::vector<DoubleMatrix> a_i_termMatVec_;	// Date fraction of each indexing period of every swap - against leg
	
	std::vector<DoubleMatrix> refCurve_logDF_;  // Log df of a reference curve on which basis spreads are added to yield the target curve
	
	int cpd_times_;		// The number of times compoundings happen with one accural period - target leg
	int a_cpd_times_;	// The number of times compoundings happen with one accural period - against leg
	
	/////////////////////////////////////////

	DoubleMatrix NDF_DFs_;
	DoubleMatrix FRA_DFs_;
	DoubleArray fxfwd_terms_;
	DoubleArray fxfwd_dfs_;

	/////////////////////////////////////////

	// calc target PV for Newton-Raphson method to generate Forecast Curve
	double calcTargetPV_calibFwdCurve(const double spread,
		const int cpd_times,
		const double swapSpotDateAsTerm,
		const DoubleArray &terms_grid,
		const DoubleArray &terms_interval,
		const DoubleMatrix &i_gridMat,
		const DoubleMatrix &i_termMat,
		const DoubleMatrix &b_yieldTimeMat,
		const double effectiveStartTerm);

	// calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)
	double calcTargetPV_calibDfCurve_MTM(const double spread,
		const int cpd_times,
		const double swapSpotDateAsTerm,
		const DoubleArray &terms_grid,
		const DoubleArray &terms_interval,
		const DoubleMatrix &i_gridMat,
		const DoubleMatrix &i_termMat,
		const DoubleMatrix &b_yieldTimeMat);


	// target PV for Newton-Raphson method to generate Discount Curve (spot renotional)
	double calcTargetPV_calibDfCurve(const double spread,
		const int cpd_times,
		const DoubleArray &terms_grid,
		const DoubleArray &terms_interval,
		const DoubleMatrix &i_gridMat,
		const DoubleMatrix &i_termMat,
		const DoubleMatrix &b_yieldTimeMat,
		const double effectiveStartTerm);

	// calc against PV (forward renotional)
	double calcAgainstPV_MTM(const double spread,
		const int cpd_times,
		const double swapSpotDateAsTerm,
		const double a_term_spot,
		const DoubleArray &terms_grid,
		const DoubleArray &terms_interval,
		const DoubleMatrix &i_gridMat,
		const DoubleMatrix &i_termMat,
		const double effectiveStartingTerm);

	// calc against PV (spot renotional)
	double calcAgainstPV(const double spread,
		const int cpd_times,
		const double swapSpotDateAsTerm,
		const DoubleArray &terms_grid,
		const DoubleArray &terms_interval,
		const DoubleMatrix &i_gridMat,
		const DoubleMatrix &i_termMat,
		const double effectiveStartingTerm);


};

// Custom object type when used with shared pointer
typedef std::shared_ptr<BasisComponentCurve> BasisComponentCurvePtr;


