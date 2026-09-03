// CurveEngine.h

/*
 * @brief			OIS Curve Caalibration Class
 * @Created:		3rd April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once
#include <memory>
#include "CurveObject.h"
#include "CurveResults.h"
#include "CoreEnumerations.h"

namespace etrading
{
    // Anonymous Namespace Definitons to limit scope to this translation unit
    namespace
    {
        const double DEFAULT_GRADIENT_SHIFT_SIZE		= 1.0e-4;	// i.e. 1 Basis Point
        const double DEFAULT_MARKET_DATA_SHIFT_SIZE		= 1.0e-4;   // i.e. 1 Basis Point
    }


    // =============================== CURVE ENGINE CLASS ==================================================================

    // Class to Calibrate the OIS Curve. This class will be used to call legacy code and store model inputs and results as class members
    class CurveEngine
    {

    public:

        // Constructor
        CurveEngine() {};
        
        // Destructor
        ~CurveEngine() {};

        // Copy Constructor
        CurveEngine( const CurveEngine& rhs );

        // Assignment Operator
        CurveEngine & operator=( const CurveEngine & rhs );

        // Curve Info Constructor
        CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription );

        // -------------------------------- SINGLE CURVE CONSTRUCTORS --------------------------------------------------

        // Single Curve Constructor - ARR Curve
		CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
					 const std::shared_ptr<ARRCurveObjectData> & arrCurveData );
		
		// Single Curve Constructor - OIS Curve
        CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
					 const std::shared_ptr<OISCurveObjectData> & oisCurveData );

		// Single Curve Constructor - Swap Curve
        CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
					 const std::shared_ptr<SwapCurveObjectData> & swapCurveData );

        // Single Curve Constructor - Tenor Basis Curve
        CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
					 const std::shared_ptr<TenorBasisCurveObjectData> & tenorBasisCurveData );

        // Single Curve Constructor - Xccy Curve (USD CSA Curves)
        CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
					 const std::shared_ptr<XccyBasisCurveObjectData> & xccyBasisCurveData );

        // Single Curve Constructor - FX Forward Constant Curve (Non-USD CSA Curves)
        CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
					 const std::shared_ptr<FwdConstantCurveObjectData> & fxFwdConstantCurveData );

        // -------------------------------------------------------------------------------------------------------------

        // Curve Output Accessors with nullptr check
		std::shared_ptr<CurveDescription>               curveDescription() const;
        std::shared_ptr<CurveResults>                   curveResults() const;
		std::shared_ptr<CurveConventionsAndMarketData>	curveConventionsAndMarketData() const;
        
        // Curve Input Accessors with nullptr check
        std::shared_ptr<OISCurveObjectData>             oisCurveData() const;
		std::shared_ptr<ARRCurveObjectData>             arrCurveData() const;
		std::shared_ptr<SwapCurveObjectData>            swapCurveData() const;
        std::shared_ptr<TenorBasisCurveObjectData>      tenorBasisCurveData() const;
        std::shared_ptr<XccyBasisCurveObjectData>       xccyBasisCurveData() const;
        std::shared_ptr<FwdConstantCurveObjectData>     fxFwdConstantCurveData() const;
    
    private:

        // ----------------------------------- CALIBRATION HELPER METHODS --------------------------------------------------

        // Method to get the Jacobian Results, will return nullptr if they don't exist
        std::shared_ptr<etrading::JacobianResults> getJacobianResultsByDiscountFactor() const;
        
		// Method to get the Jacobian Results, will return nullptr if they don't exist
        std::shared_ptr<etrading::JacobianResults> getJacobianResultsByForwardRate() const;

		// Method to get the Jacobian Results, will return nullptr if they don't exist
        std::shared_ptr<etrading::JacobianResults> getJacobianResultsByCompoundRate() const;

        // Method to evaluate which curve risk metrics to calcuate as part of the curve build
        ShiftTypeEnum curveRiskMetricsRequired() const;

		// FLAT-SHIFT JACOBIANS
		// ====================

        // Method to calibrate the ARR curve and calculate the flat-shift Jacobian
        std::shared_ptr<JacobianData> calibrateARRCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

		// Method to calibrate the OIS curve and calculate the flat-shift Jacobian
        std::shared_ptr<JacobianData> calibrateOISCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the Swap curve and calculate the flat-shift Jacobian
        std::shared_ptr<JacobianData> calibrateSwapCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the Tenor Basis curve and calculate the flat-shift Jacobian
        std::shared_ptr<JacobianData> calibrateTenorBasisCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the Xccy Basis curve and calculate the flat-shift Jacobian
        std::shared_ptr<JacobianData> calibrateXccyBasisCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the FX Forward Constant curve and calculate the flat-shift Jacobian
        std::shared_ptr<JacobianData> calibrateFxForwardConstantCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

		// PERTURBED JACOBIANS
		// ====================

		// Method to calibrate the ARR curve and calculate the Perturbed Jacobian
        std::shared_ptr<JacobianData> calibrateARRCurveAndCalculatePerturbedJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

		// Method to calibrate the OIS curve and calculate the Perturbed Jacobian
        std::shared_ptr<JacobianData> calibrateOISCurveAndCalculatePerturbedJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the Swap curve and calculate the Perturbed Jacobian
        std::shared_ptr<JacobianData> calibrateSwapCurveAndCalculatePerturbedJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the Tenor Basis curve and calculate the Perturbed Jacobian
        std::shared_ptr<JacobianData> calibrateTenorBasisCurveAndCalculatePerturbedJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the Xccy Basis curve and calculate the Perturbed Jacobian
        std::shared_ptr<JacobianData> calibrateXccyBasisCurveAndCalculatePerturbedJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // Method to calibrate the FX Forward Constant curve and calculate the Perturbed Jacobian
        std::shared_ptr<JacobianData> calibrateFxForwardConstantCurveAndCalculatePerturbedJacobian( const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

		// ===================

		// Method to Update Curve Results and keep the existing Jacobian if Present
		void updateCurveResults();

		// Method to Update Curve Results and Flat-Shift Jacobian
		void updateCurveResultsAndFlatShiftJacobian( const double & gradientShiftSize, const double & marketDataShiftSize, const std::shared_ptr<JacobianData> & jacobianData );

		// Method to Update Curve Results and Perturbed Jacobian
		void updateCurveResultsAndPerturbedJacobian( const double & gradientShiftSize, const double & marketDataShiftSize, const std::shared_ptr<JacobianData> & jacobianData );

        // Method to Calibrate an ARR Curve ( Alternative Reference Rate aka Risk-Free Rate )
        void calibrateARRCurve() const;

		// Method to Calibrate an OIS Curve
        void calibrateOISCurve() const;

        // Method to Calibrate a Swap Curve
        void calibrateSwapCurve() const;

        // Method to Calibrate a Tenor Basis Curve
        void calibrateTenorBasisCurve() const;

        // Method to Calibrate a Xccy Basis Curve to calculate USD CSA discount factors
        void calibrateXccyBasisCurve() const;

        // Method to Calibrate a Forward FX Constant Curve to calculate Non-USD CSA discount factors
        void calibrateFwdFXConstantCurve() const;

        // Method to Store the Curve Results for the Curve Index and any alias Indices in the CurveResultsContainer object
        void storeCurveResults() const;

        // Method to create Curve Discount Factor Results
        std::shared_ptr<DiscountFactorResults> createDiscountFactorResultsObject() const;

        // Method to create a Flat-Shift Jacobian Results Object given the Jacobian Data
        std::shared_ptr<JacobianResults> createFlatShiftJacobianResults( const std::shared_ptr<JacobianData> & jacobianData,
                                                                         const std::shared_ptr<DiscountFactorResults> & discountFactorResults,
                                                                         const RiskTypeEnum & riskType,
																	     const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE,
                                                                         const double marketDataShiftSizeForFlatShift = DEFAULT_MARKET_DATA_SHIFT_SIZE ) const;

		// Method to create a Perturbed Jacobian Results Object given the Jacobian Data
        std::shared_ptr<JacobianResults> createPerturbedJacobianResults( const std::shared_ptr<JacobianData> & jacobianData,
                                                                         const std::shared_ptr<DiscountFactorResults> & discountFactorResults,
                                                                         const RiskTypeEnum & riskType,
																	     const double gradientShiftSize = DEFAULT_GRADIENT_SHIFT_SIZE ) const;

        // ------------------------------------- HELPER METHODS -----------------------------------------------------
        
        // Helper Method to get the Daycount from Curve Data
		DayCountEnum getDayCountFromCurveData( const CurveTypeEnum & curveType,
                                               const std::string & curveCollection ) const;
		
		// Helper Method to get the Daycount from Curve Data as a std::string
		std::string getDayCountFromCurveDataAsString( const CurveTypeEnum & curveType,
                                                      const std::string & curveCollection ) const;

        // Helper Method to Update Discount Factor Results from Object Pool
        std::shared_ptr<DiscountFactorResults> getDiscountFactorResultsFromObjectPool( const CurveTypeEnum & curveType,
																					   const CurveTenorEnum & curveTenorEnum,
                                                                                       const std::string & curveCollection,
                                                                                       const std::string & objectPoolLookupTable,
                                                                                       const BusinessDayAdjustmentEnum & fixingBusDayAdj,
                                                                                       const std::string & fixingCalendar,
																					   const StandardStringMatrix & forwardAdjustments ) const;
		
        // Helper Method to Identify the Target Leg of a Tenor or Xccy Basis Swap
        bool isBasisTargetLeg1( const CurveTypeEnum curveType, const LabelValueBlock & curveConventionLVB ) const;

        // Output Data Members
		std::shared_ptr<CurveDescription>               curveDescription_;
        std::shared_ptr<CurveResults>                   curveResults_;
        std::shared_ptr<CurveConventionsAndMarketData>	curveConventionsAndMarketData_;

        // Input Data Members
		std::shared_ptr<OISCurveObjectData>             oisCurveData_;
		std::shared_ptr<ARRCurveObjectData>             arrCurveData_;
		std::shared_ptr<SwapCurveObjectData>            swapCurveData_;
        std::shared_ptr<TenorBasisCurveObjectData>      tenorBasisCurveData_;
        std::shared_ptr<XccyBasisCurveObjectData>       xccyBasisCurveData_;
        std::shared_ptr<FwdConstantCurveObjectData>     fxFwdConstantCurveData_;
    };

}
