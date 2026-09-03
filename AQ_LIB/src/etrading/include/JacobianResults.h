// JacobianResults.h

/*
 * @brief			Jacobian Results Class
 * @Created:		15th July 2017
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once
#include <vector>
#include <memory>

#include "LADate.h"
#include "CoreEnumerations.h"
#include "LAInterpolationBase.h"
#include "Variant.h"

// The Jacobian Builds and Extends upon the Curve Desccription Results & Discount Factor Results Classes
#include "CurveDescription.h"
#include "DiscountFactorResults.h"

namespace etrading
{
    // =============================== HELPER CLASS ==================================================================


    // Jacobian Data Class
    class JacobianData
    {
        public:

            JacobianData() {}
            ~JacobianData() {}

            // Main Constructor: Create Jacobian from Flat-Shifted Discount Factors
            JacobianData( const std::shared_ptr<DiscountFactorResults> & originalDFObject,
                          const std::shared_ptr<DiscountFactorResults> & flatShiftedDFObject,
						  const bool isFwdInter = false );
						  
			// Main Constructor: Create Jacobian from Perturbed Discount Factors
            JacobianData( const DoubleVector & perturbedMarketDataShiftSizes,
                          const StandardStringVector & perturbedInstrumentList,
						  const std::vector<bool> & perturbedInstrumentIsOutright,
						  const std::shared_ptr<DiscountFactorResults> & originalDFObject,
						  const std::vector<std::shared_ptr<DiscountFactorResults> > & perturbedDFObjects,
						  const bool isFwdInter = false );
						  
            DateVector paymentDates() const							{   return paymentDates_;            }
            DoubleVector originalDiscountFactors() const			{   return originalDiscountFactors_; }
            
			// Jacobian Data Results
			DoubleVector flatShiftJacobianByDiscountFactor() const;
			DoubleVector flatShiftJacobianByForwardRate() const;
			DoubleMatrix perturbedJacobianByDiscountFactor() const;
			DoubleMatrix perturbedJacobianByForwardRate() const;
			DoubleVector perturbedMarketDataShiftSizes() const;
			StandardStringVector perturbedInstrumentList() const;
			StandardStringVector perturbedInstrumentTenors() const;
			std::vector<bool> perturbedInstrumperturbedInstrumentIsOutright() const;

            // Static Helper Method to calculate the change in discount factors and validate inputs
            static DoubleVector changeInDiscountFactors( const std::shared_ptr<DiscountFactorResults> & originalDFObject,
                                                         const std::shared_ptr<DiscountFactorResults> & bumpedDFObject );

			// Static Helper Method to calculate the change in Forward Rates and validate inputs
            static DoubleVector changeInForwardRates( const std::shared_ptr<DiscountFactorResults> & originalDFObject,
													  const std::shared_ptr<DiscountFactorResults> & bumpedDFObject,
													  const bool isFwdInter = false );
													  
			bool isFwdInter() const		{ return isFwdInter_; }

        private:

			// Perturbed or Flat-Shift
			ShiftTypeEnum shiftTypeEnum_; 

			// Legacy Parameter: Controls if pseudo forward rates should be used with Discount Factor state variable
			bool isFwdInter_;

            DateVector paymentDates_;
            DoubleVector originalDiscountFactors_;
            
			DoubleVector perturbedMarketDataShiftSizes_;
			StandardStringVector perturbedInstrumentList_;
			StandardStringVector perturbedInstrumentTenors_;
			std::vector<bool> perturbedInstrumentIsOutright_; // i.e. Peturbed Spread Instruments should not be included in risk calculations, only outright instruments. This

			DoubleVector flatShiftJacobianByDiscountFactor_;
			DoubleVector flatShiftJacobianByForwardRate_;

			DoubleMatrix perturbedJacobianByDiscountFactor_;
			DoubleMatrix perturbedJacobianByForwardRate_;
    };

    // =============================== JACOBIAN RESULTS CLASS ==================================================================

    // Class to store and retrieve discount factors
    class JacobianResults
    {

    public:
        JacobianResults() {};
        ~JacobianResults() {};

        // Main Constructors 
        // =====================================================================================================================

        // Flat-Shift Jacobian Constructor *** For Server APIs where Dates stored as Dates ***
        // ===============================
        // ShiftTypeEnum = FLAT_SHIFT
        // InstrumentVector not required since we are working on TOTALs with size 1
        // Jacobian result is a VECTOR ( n x 1 ), where n = number of dates in riskDateVector
        JacobianResults( const LADate & asOfDate,
                         const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                         const RiskTypeEnum & riskType,
                         const double & gradientShiftSize,
                         const DateVector & riskDateVector,             
                         const double & flatShiftMarketDataShiftSize,
                         const DoubleVector & flatShiftJacobian,
                         const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults );

        // Perturbed Jacobian Constructor *** For Server APIs where Dates stored as Dates ***
        // ===============================
        // ShiftTypeEnum = PERTURBED
        // InstrumentVector required for Pertubation buckets
        // Jacobian result is a MATRIX (n x m), where n = number of dates in riskDateVector and m = number of Instruments in instrumentVector
        JacobianResults( const LADate & asOfDate,
                         const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                         const RiskTypeEnum & riskType,   
                         const double & gradientShiftSize,
                         const DateVector & riskDateVector,             
                         const DoubleVector & perturbedMarketDataShiftSizes,
                         const StandardStringVector & perturbedInstruments,
						 const std::vector<bool> & perturbedInstrumentsIsOutright,
                         const DoubleMatrix & perturbedJacobian,
                         const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults );

        // Flat-Shift Jacobian Constructor *** For Object Pool APIs where Dates stored as Term Doubles ***
        // ===============================
        // ShiftTypeEnum = FLAT_SHIFT
        // InstrumentVector not required since we are working on TOTALs with size 1
        // Jacobian result is a VECTOR ( n x 1 ), where n = number of dates in riskDateVector
        JacobianResults( const LADate & asOfDate,
                         const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                         const RiskTypeEnum & riskType,
                         const double & gradientShiftSize,
                         const DoubleVector & riskDateVectorInTermFormat,             
                         const double & flatShiftMarketDataShiftSize,
                         const DoubleVector & flatShiftJacobian,
                         const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults );

        // Perturbed Jacobian Constructor *** For Object Pool APIs where Dates stored as Term Doubles ***
        // ===============================
        // ShiftTypeEnum = PERTURBED
        // InstrumentVector required for Pertubation buckets
        // Jacobian result is a MATRIX (n x m), where n = number of dates in riskDateVector and m = number of Instruments in instrumentVector
        JacobianResults( const LADate & asOfDate,
                         const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                         const RiskTypeEnum & riskType,
                         const double & gradientShiftSize,
                         const DoubleVector & riskDateVectorInTermFormat,             
                         const DoubleVector & perturbedMarketDataShiftSizes,
                         const StandardStringVector & perturbedInstruments,
						 const std::vector<bool> & perturbedInstrumentsIsOutright,
                         const DoubleMatrix & perturbedJacobian,
                         const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults );

        // =====================================================================================================================

        // Copy Constructor
        JacobianResults( const JacobianResults& rhs );

		// Clone
		std::shared_ptr<JacobianResults> clone() const;

        // Assignment Operator
        JacobianResults & operator=( const JacobianResults & rhs );

        // Jacobian Methods
        // =======================================================
        
        // Populate FLAT-SHIFT Jacobian from Server APIs *** using Dates ***
        static void populateFlatShiftJacobianFromStringMatrix( DateVector & riskDates, DoubleVector & flatShiftJacobian, const StandardStringMatrix & inputMatrix );
    
        // Populate FLAT-SHIFT Jacobian from Object Pool *** using Terms Doubles for Dates ***
        static void populateFlatShiftJacobianFromStringMatrix( DoubleVector & riskDatesAsDouble, DoubleVector & flatShiftJacobian, const StandardStringMatrix & inputMatrix );
    
        // Populate PERTURBED Jacobian from Server APIs *** using Dates ***
        static void populatePerturbedJacobianFromStringMatrix( DateVector & riskDates, StandardStringVector & perturbedInstruments, DoubleMatrix & perturbedJacobian, const StandardStringMatrix & inputMatrix );
    
        // Populate PERTURBED Jacobian from Object Pool *** using Terms Doubles for Dates ***
        static void populatePerturbedJacobianFromStringMatrix( DoubleVector & riskDatesAsDouble, StandardStringVector & perturbedInstruments, DoubleMatrix & perturbedJacobian, const StandardStringMatrix & inputMatrix );

        // Method to consolodate and display the Flat Shift Jacobian
        VariantMatrix displayFlatShiftJacobian() const;
        
        // Method to consolodate and display the Perturbed Jacobian
        VariantMatrix displayPerturbedJacobian() const;

        // Method to Calculate the Total Risk across all instruments for each Risk Date.
        DoubleVector calculateFlatShiftRiskTotals() const;

        // Method to Calculate the Total Risk across all instruments for each Risk Date.
        DoubleVector calculatePerturbedRiskTotals( const bool & useOutrightInstrumentsOnly = true ) const;

        // Method to Imply New Discount Factors
        DoubleVector implyNewDiscountFactors() const;
        
        // TODO: Placeholders for methods required
        void implyNewForwardRates() const {};
        void implyNewComoundRates() const {};

        // Jacobian Accessors
        // =======================================================

        std::shared_ptr<etrading::CurveDescription> curveDescription() const;
        std::shared_ptr<etrading::DiscountFactorResults> discountFactorResults() const;
        
        LADate asOfDate() const                                     { return asOfDate_;                         }
        RiskTypeEnum riskType() const                               { return riskType_;                         }
        ShiftTypeEnum shiftType() const                             { return shiftType_;                        }
        double gradientShiftSize() const                            { return gradientShiftSize_;                }
        DateVector riskDateVector() const                           { return riskDateVector_;                   }
        DoubleVector riskDateVectorInTermFormat() const             { return riskDateVectorInTermFormat_;       }
        double flatShiftMarketDataShiftSize() const                 { return flatShiftMarketDataShiftSize_;     }
        DoubleVector flatShiftJacobian() const                      { return flatShiftJacobian_;                }
        std::vector<double> perturbedMarketDataShiftSizes() const   { return perturbedMarketDataShiftSizes_;    }
        StandardStringVector perturbedInstruments() const           { return perturbedInstruments_;             }
		std::vector<bool> perturbedInstrumentsIsOutright() const	{ return perturbedInstrumentsIsOutright_;   }
        DoubleMatrix perturbedJacobian() const                      { return perturbedJacobian_;                }

    private:
        
        std::shared_ptr<etrading::CurveDescription>         curveDescription_;
        std::shared_ptr<etrading::DiscountFactorResults>    discountFactorResults_;

        LADate                  asOfDate_;
        RiskTypeEnum            riskType_;
        ShiftTypeEnum           shiftType_;
        double                  gradientShiftSize_;
        DateVector              riskDateVector_;
        DoubleVector            riskDateVectorInTermFormat_;
        double                  flatShiftMarketDataShiftSize_;
        DoubleVector            flatShiftJacobian_;
        std::vector<double>     perturbedMarketDataShiftSizes_;
        StandardStringVector    perturbedInstruments_;
		std::vector<bool>		perturbedInstrumentsIsOutright_; 
        DoubleMatrix            perturbedJacobian_;
    };

}


