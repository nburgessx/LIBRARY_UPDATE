// JacobianResults.cpp

/*
 * @brief			Jacobian Results Class
 * @Created:		15th July 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "JacobianResults.h"
#include "ExceptionMacros.h"
#include "CurveValidation.h"			        // Convert Dates to Terms and vice versa
#include "AQLDateScheduleHelpers.h"              // Convert Strings to Dates and vice versa
#include "CurveUtilities.h"				        // DateFromTenor methods
#include "AQLCurvePricingObject.h"	            // Methods to get the curve daycount conventions
#include "AQLEnumConversion.h"		            // Methods to convert enum values to legacy enums
#include "DataUtilities.h"                      // Methods to cast numbers to strings
#include <numeric>                              // For std::accumulate

namespace etrading
{

    // ======================================= DISCOUNT FACTOR JACOBIAN CLASS =====================================================


    // Main Constructor: Create Jacobian from Flat-Shifted Discount Factors
    JacobianData::JacobianData( const std::shared_ptr<DiscountFactorResults> & originalDFObject,
                                const std::shared_ptr<DiscountFactorResults> & flatShiftedDFObject,
								const bool isFwdInter )
    {
        shiftTypeEnum_ = FLAT_SHIFT_TYPE;
		isFwdInter_ = isFwdInter;

		paymentDates_               = originalDFObject->paymentDates();
        originalDiscountFactors_    = originalDFObject->discountFactors();
        
        // Change in Discount Factors
        flatShiftJacobianByDiscountFactor_.clear();
        flatShiftJacobianByDiscountFactor_ = changeInDiscountFactors( originalDFObject, flatShiftedDFObject );

		// Change in Forward Rates
		flatShiftJacobianByForwardRate_.clear();
		flatShiftJacobianByForwardRate_ = changeInForwardRates( originalDFObject, flatShiftedDFObject, isFwdInter );
    }

	// Main Constructor: Create Jacobian from Perturbed Discount Factors
	JacobianData::JacobianData( const DoubleVector & perturbedMarketDataShiftSizes,
								const StandardStringVector & perturbedInstrumentList,
								const std::vector<bool> & perturbedInstrumentIsOutright,
								const std::shared_ptr<DiscountFactorResults> & originalDFObject,
								const std::vector<std::shared_ptr<DiscountFactorResults> > & perturbedDFObjects,
								const bool isFwdInter )
	{
		shiftTypeEnum_ = PERTURBED_SHIFT_TYPE;
		isFwdInter_ = isFwdInter;

		AQ_REQUIRE( perturbedInstrumentList.size() == perturbedInstrumentIsOutright.size(), "Invalid Jacobian Data: The number of perturbed instruments must match the number of instrument isOutright vector elements" )
		AQ_REQUIRE( perturbedInstrumentList.size() == perturbedMarketDataShiftSizes.size(), "Invalid Jacobian Data: The number of perturbed instruments must match the number of perturbation shift sizes" )
		AQ_REQUIRE( perturbedInstrumentList.size() == perturbedDFObjects.size(), "Invalid Jacobian Data: the number of perturbed instruments must match the number of perturbed discount factor objects" )

		// Update Jacobian Non-Matrix Data
		paymentDates_					= originalDFObject->paymentDates();
        originalDiscountFactors_		= originalDFObject->discountFactors();
        perturbedMarketDataShiftSizes_	= perturbedMarketDataShiftSizes;
		perturbedInstrumentList_		= perturbedInstrumentList;
		perturbedInstrumentIsOutright_	= perturbedInstrumentIsOutright;

		// Clear Previous Jacobian Matrix Results (if any)
		perturbedJacobianByDiscountFactor_.clear();
		perturbedJacobianByForwardRate_.clear();

		// Reserve Jacobian Matrix Dimensions
		// -------------------------------------------
		// Jacobians are returned column by column, from which we need to populate the Jacobian Data Matrix by row
		const size_t nJacobianRows = paymentDates_.size();
		const size_t nJacobianCols = perturbedInstrumentList_.size();

		perturbedJacobianByDiscountFactor_.resize( nJacobianRows );
		perturbedJacobianByForwardRate_.resize( nJacobianRows );

		for( size_t row = 0; row < nJacobianRows; ++row )
		{
			perturbedJacobianByDiscountFactor_[row].resize( nJacobianCols );
			perturbedJacobianByForwardRate_[row].resize( nJacobianCols );
		}
		// -------------------------------------------

		// Populate Jacobian Matrix
		// Jacobians are returned column by column, from which we need to populate the Jacobian Data Matrix by row
		for ( size_t jacobianCol = 0; jacobianCol < nJacobianCols; ++jacobianCol )
		{
			// Change in Discount Factors & Forward Rates
			const DoubleVector jacobianColumnDataByDiscountFactor( changeInDiscountFactors( originalDFObject, perturbedDFObjects[ jacobianCol ] ) );
			const DoubleVector jacobianColumnDataByForwardRates( changeInForwardRates( originalDFObject, perturbedDFObjects[ jacobianCol ], isFwdInter ) );
		
			AQ_REQUIRE( jacobianColumnDataByDiscountFactor.size() == nJacobianRows, "Invalid Jacobian Data: The number of Jacobian Deltas by Discount Factor must match the number of discount factor payment dates" )
			AQ_REQUIRE( jacobianColumnDataByForwardRates.size() == nJacobianRows, "Invalid Jacobian Data: The number of Jacobian Deltas by Forward Rate must match the number of forward rate fixing dates" )

			for( size_t jacobianRow = 0; jacobianRow < nJacobianRows; ++jacobianRow )
			{
				perturbedJacobianByDiscountFactor_[ jacobianRow ][ jacobianCol ]	= jacobianColumnDataByDiscountFactor[ jacobianRow ];
				perturbedJacobianByForwardRate_[ jacobianRow ][ jacobianCol ]		= jacobianColumnDataByForwardRates[ jacobianRow ];
			}
		}

	}


	// Method to return flat-shift Jacobian results by discount factor
	DoubleVector JacobianData::flatShiftJacobianByDiscountFactor() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == FLAT_SHIFT_TYPE, "Invalid ShiftType used with Flat-Shift Jacobian Data" )
		return flatShiftJacobianByDiscountFactor_;
	}
	
	// Method to return flat-shift Jacobian results by forward rate
	DoubleVector JacobianData::flatShiftJacobianByForwardRate() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == FLAT_SHIFT_TYPE, "Invalid ShiftType used with Flat-Shift Jacobian Data" )
		return flatShiftJacobianByForwardRate_;
	}
	
	// Method to return perturbed Jacobian results by discount factor
	DoubleMatrix JacobianData::perturbedJacobianByDiscountFactor() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == PERTURBED_SHIFT_TYPE, "Invalid ShiftType used with Perturbed Jacobian Data" )
		return perturbedJacobianByDiscountFactor_;
	}
	
	// Method to return perturbed Jacobian results by forward rate
	DoubleMatrix JacobianData::perturbedJacobianByForwardRate() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == PERTURBED_SHIFT_TYPE, "Invalid ShiftType used with Perturbed Jacobian Data" )
		return perturbedJacobianByForwardRate_;
	}

	DoubleVector JacobianData::perturbedMarketDataShiftSizes() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == PERTURBED_SHIFT_TYPE, "Invalid ShiftType used with Perturbed Jacobian Data" )
		return perturbedMarketDataShiftSizes_;
	}

	StandardStringVector JacobianData::perturbedInstrumentList() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == PERTURBED_SHIFT_TYPE, "Invalid ShiftType used with Perturbed Jacobian Data" )
		return perturbedInstrumentList_;
	}

	StandardStringVector JacobianData::perturbedInstrumentTenors() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == PERTURBED_SHIFT_TYPE, "Invalid ShiftType used with Perturbed Jacobian Data" )
		return perturbedInstrumentTenors_;
	}

	std::vector<bool> JacobianData::perturbedInstrumperturbedInstrumentIsOutright() const
	{
		AQ_REQUIRE( shiftTypeEnum_ == PERTURBED_SHIFT_TYPE, "Invalid ShiftType used with Perturbed Jacobian Data" )
		return perturbedInstrumentIsOutright_;
	}

    // Static Helper Method to calculate the change in discount factors and validate inputs
    DoubleVector JacobianData::changeInDiscountFactors( const std::shared_ptr<DiscountFactorResults> & originalDFObject,
                                                        const std::shared_ptr<DiscountFactorResults> & shiftedDFObject )
    {
        // Dimension Validation
        const size_t nOriginalPaymentDates    = originalDFObject->paymentDates().size();
        const size_t nOriginalDFs             = originalDFObject->discountFactors().size();
        const size_t nShiftedPaymentDates     = shiftedDFObject->paymentDates().size();
        const size_t nShiftedDFs              = shiftedDFObject->discountFactors().size();

        AQ_REQUIRE( nOriginalPaymentDates == nOriginalDFs,	"Unable to calculate the Discount Factor Jacobian - Invalid 'OriginalDiscountFactors' - Number of Payment Dates must match the number of original Discount Factors" )
        AQ_REQUIRE( nShiftedPaymentDates == nShiftedDFs,		"Unable to calculate the Discount Factor Jacobian - Invalid 'ShiftedDiscountFactors' - Number of Payment Dates must match the number of original Discount Factors" )
        AQ_REQUIRE( nOriginalDFs == nShiftedDFs,				"Unable to calculate the Discount Factor Jacobian - Number of Original Discount Factors must match the number of Shifted Discount Factors" )

        // Change in Discount Factors
        DoubleVector changeInDiscountFactors( nOriginalPaymentDates, 0.0 );
        for ( size_t i = 0; i < nOriginalPaymentDates; ++i )
        {
            changeInDiscountFactors[i] = shiftedDFObject->discountFactors()[i] - originalDFObject->discountFactors()[i];
        }

        return changeInDiscountFactors;
    }

	// Static Helper Method to calculate the change in Forward Rates and validate inputs
    DoubleVector JacobianData::changeInForwardRates( const std::shared_ptr<DiscountFactorResults> & originalDFObject,
                                                     const std::shared_ptr<DiscountFactorResults> & shiftedDFObject,

												     const bool isFwdInter )
    {
        // Dimension Validation
        const size_t nOriginalPaymentDates    = originalDFObject->paymentDates().size();
        const size_t nOriginalDFs             = originalDFObject->discountFactors().size();
        const size_t nShiftedPaymentDates     = shiftedDFObject->paymentDates().size();
        const size_t nShiftedDFs              = shiftedDFObject->discountFactors().size();

        AQ_REQUIRE( nOriginalPaymentDates == nOriginalDFs,	"Unable to calculate the Forward Rate Jacobian - Invalid 'OriginalDiscountFactors' - Number of Payment Dates must match the number of original Discount Factors" )
        AQ_REQUIRE( nShiftedPaymentDates == nShiftedDFs,		"Unable to calculate the Forward Rate Jacobian - Invalid 'ShiftedDiscountFactors' - Number of Payment Dates must match the number of original Discount Factors" )
        AQ_REQUIRE( nOriginalDFs == nShiftedDFs,				"Unable to calculate the Forward Rate Jacobian - Number of Original Discount Factors must match the number of Shifted Discount Factors" )

		// Forward Rates
		const DoubleVector originalForwardRates		= originalDFObject->implyForwardRates( isFwdInter );
		const DoubleVector shiftedForwardRates		= shiftedDFObject->implyForwardRates( isFwdInter );

		AQ_REQUIRE( originalForwardRates.size() == shiftedForwardRates.size(), "Unable to calculate the Forward Rate Jacobian - Number of Original Forward Rates and Shifted Forward Rates must be the same" )

        // Change in Forward Rates
        DoubleVector changeInForwardRates( nOriginalPaymentDates, 0.0 );
        for ( size_t i = 0; i < nOriginalPaymentDates; ++i )
        {
            changeInForwardRates[i] = shiftedForwardRates[i] - originalForwardRates[i];
        }

        return changeInForwardRates;
    }

    // =============================== JACOBIAN RESULTS CLASS ==================================================================
    

    // Flat-Shift Jacobian Constructor - Server APIs - Dates stored as Dates
    // ===============================
    // ShiftTypeEnum = FLAT_SHIFT
    // InstrumentVector not required since we are working on TOTALs with size 1
    // Jacobian result is a VECTOR ( n x 1 ), where n = number of dates in riskDateVector
    JacobianResults::JacobianResults( const AQLDate & asOfDate,
                                      const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                                      const RiskTypeEnum & riskType,
                                      const double & gradientShiftSize,
                                      const DateVector & riskDateVector,                              
                                      const double & flatShiftMarketDataShiftSize,
                                      const DoubleVector & flatShiftJacobian,
                                      const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults )
        : asOfDate_( asOfDate ),
          curveDescription_(curveDescription),
          riskType_(riskType),
          shiftType_(FLAT_SHIFT_TYPE), // FLAT_SHIFT_TYPE
          gradientShiftSize_(gradientShiftSize),
          riskDateVector_(riskDateVector),
          flatShiftMarketDataShiftSize_(flatShiftMarketDataShiftSize),
          flatShiftJacobian_(flatShiftJacobian),
          discountFactorResults_(discountFactorResults),
		  perturbedInstrumentsIsOutright_(std::vector<bool>()) // null vector
    {
        // Dimension Check(s)
        AQ_REQUIRE( flatShiftJacobian.size() > 0, "Invalid Jacobian: The Jacobian is empty")
        AQ_REQUIRE( riskDateVector.size() == flatShiftJacobian.size(), "Invalid Jacobian Row Space: The number of riskDates must match the number of Jacobian rows" )
    }

    // Perturbed Jacobian Constructor - Server APIs - Dates stored as Dates
    // ===============================
    // ShiftTypeEnum = PERTURBED
    // InstrumentVector required for Pertubation buckets
    // Jacobian result is a MATRIX (n x m), where n = number of dates in riskDateVector and m = number of Instruments in instrumentVector
    JacobianResults::JacobianResults( const AQLDate & asOfDate,
                                      const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                                      const RiskTypeEnum & riskType,
                                      const double & gradientShiftSize,
                                      const DateVector & riskDateVector,                              
                                      const DoubleVector & perturbedMarketDataShiftSizes,
                                      const StandardStringVector & perturbedInstruments,
									  const std::vector<bool> & perturbedInstrumentsIsOutright,
                                      const DoubleMatrix & perturbedJacobian,
                                      const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults )
        : asOfDate_( asOfDate ),
          curveDescription_(curveDescription),
          riskType_(riskType),
          shiftType_(PERTURBED_SHIFT_TYPE), // PERTURBED_SHIFT_TYPE
          gradientShiftSize_(gradientShiftSize),
          riskDateVector_(riskDateVector),
          perturbedMarketDataShiftSizes_(perturbedMarketDataShiftSizes),
          perturbedInstruments_(perturbedInstruments),
		  perturbedInstrumentsIsOutright_(perturbedInstrumentsIsOutright),
          perturbedJacobian_(perturbedJacobian),
          discountFactorResults_(discountFactorResults)
    {
        // Dimension Check(s)
        AQ_REQUIRE( perturbedJacobian.size() > 0, "Invalid Jacobian: The Jacobian is empty")
        AQ_REQUIRE( riskDateVector.size() == perturbedJacobian.size(), "Invalid Jacobian Results Object: The number of riskDates must match the number of Jacobian rows" )
        AQ_REQUIRE( perturbedInstruments.size() == perturbedJacobian[0].size(), "Invalid Jacobian Column Space: The number of Instruments must match the number of Jacobian columns" )
		AQ_REQUIRE( perturbedInstrumentsIsOutright.size() == perturbedJacobian[0].size(), "Invalid Jacobian Column Space: The number of OutrightInstrument parameters must match the number of Jacobian columns" )
    }

    // Flat-Shift Jacobian Constructor - Object Pool APIs - Dates stored as Term Doubles
    // ===============================
    // ShiftTypeEnum = FLAT_SHIFT
    // InstrumentVector not required since we are working on TOTALs with size 1
    // Jacobian result is a VECTOR ( n x 1 ), where n = number of dates in riskDateVector
    JacobianResults::JacobianResults( const AQLDate & asOfDate,
                                      const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                                      const RiskTypeEnum & riskType,
                                      const double & gradientShiftSize,
                                      const DoubleVector & riskDateVectorInTermFormat,                              
                                      const double & flatShiftMarketDataShiftSize,
                                      const DoubleVector & flatShiftJacobian,
                                      const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults )
        : asOfDate_( asOfDate ),
          curveDescription_(curveDescription),
          riskType_(riskType),
          shiftType_(FLAT_SHIFT_TYPE),  // FLAT_SHIFT_TYPE
          gradientShiftSize_(gradientShiftSize),
          riskDateVectorInTermFormat_(riskDateVectorInTermFormat),
          flatShiftMarketDataShiftSize_(flatShiftMarketDataShiftSize),
          flatShiftJacobian_(flatShiftJacobian),
          discountFactorResults_(discountFactorResults),
		  perturbedInstrumentsIsOutright_(std::vector<bool>()) // null vector
    {
        // Dimension Check(s)
        AQ_REQUIRE( flatShiftJacobian.size() > 0, "Invalid Jacobian: The Jacobian is empty")
        AQ_REQUIRE( riskDateVectorInTermFormat.size() == flatShiftJacobian.size(), "Invalid Jacobian Row Space: The number of riskDates must match the number of Jacobian rows" )
    }

    // Perturbed Jacobian Constructor - Object Pool APIs - Dates stored as Term Doubles
    // ===============================
    // ShiftTypeEnum = PERTURBED
    // InstrumentVector required for Pertubation buckets
    // Jacobian result is a MATRIX (n x m), where n = number of dates in riskDateVector and m = number of Instruments in instrumentVector
    JacobianResults::JacobianResults( const AQLDate & asOfDate,
                                      const std::shared_ptr<etrading::CurveDescription> & curveDescription,
                                      const RiskTypeEnum & riskType,
                                      const double & gradientShiftSize,
                                      const DoubleVector & riskDateVectorInTermFormat,                              
                                      const DoubleVector & perturbedMarketDataShiftSizes,
                                      const StandardStringVector & perturbedInstruments,
									  const std::vector<bool> & perturbedInstrumentsIsOutright,
                                      const DoubleMatrix & perturbedJacobian,
                                      const std::shared_ptr<etrading::DiscountFactorResults> & discountFactorResults )
        : asOfDate_( asOfDate ),
          curveDescription_(curveDescription),  
          riskType_(riskType),
          shiftType_(PERTURBED_SHIFT_TYPE), // PERTURBED_SHIFT_TYPE
          gradientShiftSize_(gradientShiftSize),
          riskDateVectorInTermFormat_(riskDateVectorInTermFormat),
          perturbedMarketDataShiftSizes_(perturbedMarketDataShiftSizes),
          perturbedInstruments_(perturbedInstruments),
		  perturbedInstrumentsIsOutright_(perturbedInstrumentsIsOutright),
          perturbedJacobian_(perturbedJacobian),
          discountFactorResults_(discountFactorResults)
    {
        // Dimension Check(s)
        AQ_REQUIRE( perturbedJacobian.size() > 0, "Invalid Jacobian: The Jacobian is empty")
        AQ_REQUIRE( riskDateVectorInTermFormat.size() == perturbedJacobian.size(), "Invalid Jacobian Results Object: The number of riskDates must match the number of Jacobian rows" )
        AQ_REQUIRE( perturbedInstruments.size() == perturbedJacobian[0].size(), "Invalid Jacobian Column Space: The number of Instruments must match the number of Jacobian columns" )
		AQ_REQUIRE( perturbedInstrumentsIsOutright.size() == perturbedJacobian[0].size(), "Invalid Jacobian Column Space: The number of OutrightInstrument parameters must match the number of Jacobian columns" )
    }

    // =========================================================================================================================        
    
    // Copy Constructor
    JacobianResults::JacobianResults( const JacobianResults& rhs ) 
        : asOfDate_(rhs.asOfDate_),
          curveDescription_(rhs.curveDescription_),  // Shared Pointer
          riskType_(rhs.riskType_),
          shiftType_(rhs.shiftType_),
          gradientShiftSize_(rhs.gradientShiftSize_),
          riskDateVector_(rhs.riskDateVector_ ),
          riskDateVectorInTermFormat_( rhs.riskDateVectorInTermFormat_ ),
          flatShiftMarketDataShiftSize_(rhs.flatShiftMarketDataShiftSize_),
          flatShiftJacobian_(rhs.flatShiftJacobian_),  
          perturbedMarketDataShiftSizes_(rhs.perturbedMarketDataShiftSizes_),
          perturbedInstruments_(rhs.perturbedInstruments_),
		  perturbedInstrumentsIsOutright_(rhs.perturbedInstrumentsIsOutright_),
          perturbedJacobian_(rhs.perturbedJacobian_),
          discountFactorResults_(rhs.discountFactorResults_) // Shared Pointer
	{
	}

	// Clone
	std::shared_ptr<JacobianResults> JacobianResults::clone() const
	{
		return std::make_shared<JacobianResults>( JacobianResults( *this ) );
	}

    // Assignment Operator
    JacobianResults & JacobianResults::operator=( const JacobianResults & rhs )
    {
		// For Performance 
		if ( &rhs == this )
		{
			return *this;
		}

        // For Exception Safety
            
        // 1. Make a temp copy
        JacobianResults temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( asOfDate_,                       temp.asOfDate_ );
        std::swap( curveDescription_,               temp.curveDescription_ );
        std::swap( riskType_,                       temp.riskType_ );
        std::swap( shiftType_,                      temp.shiftType_ );
        std::swap( gradientShiftSize_,              temp.gradientShiftSize_ );
        std::swap( riskDateVector_,                 temp.riskDateVector_ );
        std::swap( riskDateVectorInTermFormat_,     temp.riskDateVectorInTermFormat_ );
        std::swap( flatShiftMarketDataShiftSize_,   temp.flatShiftMarketDataShiftSize_ );
        std::swap( flatShiftJacobian_,              temp.flatShiftJacobian_ );
        std::swap( perturbedMarketDataShiftSizes_,  temp.perturbedMarketDataShiftSizes_ );
        std::swap( perturbedInstruments_,           temp.perturbedInstruments_ );
		std::swap( perturbedInstrumentsIsOutright_, temp.perturbedInstrumentsIsOutright_ );
        std::swap( perturbedJacobian_,              temp.perturbedJacobian_ );
        std::swap( discountFactorResults_,          temp.discountFactorResults_ );
        
        return *this;
    }

    // ========= ACCESSORS ===========================================================================================================================
    std::shared_ptr<etrading::CurveDescription> JacobianResults::curveDescription() const
    {
        AQ_REQUIRE( curveDescription_ != nullptr, "Invalid Curve Results: CurveDescription does not exist" )
        return curveDescription_;
    }

    std::shared_ptr<etrading::DiscountFactorResults> JacobianResults::discountFactorResults() const
    {
        AQ_REQUIRE( discountFactorResults_ != nullptr, "Invalid Curve Results: discountFactorResults do not exist" )
        return discountFactorResults_;
    }

    // ========= STATIC HELPER METHODS ================================================================================================================   

    // Populate FLAT-SHIFT Jacobian from Server APIs *** using Dates ***
    void JacobianResults::populateFlatShiftJacobianFromStringMatrix( DateVector & riskDates, DoubleVector & flatShiftJacobian, const StandardStringMatrix & inputMatrix )
    {
        AQ_REQUIRE( !inputMatrix.empty(),         "Unble to create Jacobian - Input Matrix is empty" )
        AQ_REQUIRE( inputMatrix[0].size() == 2,   "Unble to create Jacobian - Input Matrix must consist of exactly 2 columns; a single column of risk dates and a single column of risk totals" )

        const size_t rowSizeWithoutHeader = inputMatrix.size() - 1;
        riskDates.reserve( rowSizeWithoutHeader );
        flatShiftJacobian.reserve( rowSizeWithoutHeader );

        // The input matrix contains a header, so start from row 1
        for ( size_t row = 1; row < inputMatrix.size(); ++row )
        {
            // Column 1: Risk Dates - Cast String to Date
            riskDates.push_back( etrading::AQLDateScheduleHelpers::getAQLDate( inputMatrix[row][0] ) );
            
            // Column 2: Risk Values - Cast from String to Double
            char * pFirstNonNumber;
            flatShiftJacobian.push_back( std::strtod( inputMatrix[row][1].c_str(), &pFirstNonNumber ) );
        }
    }

    // Populate FLAT-SHIFT Jacobian from Object Pool *** using Terms Doubles for Dates ***
    void JacobianResults::populateFlatShiftJacobianFromStringMatrix( DoubleVector & riskDatesAsDouble, DoubleVector & flatShiftJacobian, const StandardStringMatrix & inputMatrix )
    {
        AQ_REQUIRE( !inputMatrix.empty(),         "Unble to create Jacobian - Input Matrix is empty" )
        AQ_REQUIRE( inputMatrix[0].size() == 2,   "Unble to create Jacobian - Input Matrix must consist of exactly 2 columns; a single column of risk dates and a single column of risk totals" )

        const size_t rowSizeWithoutHeader = inputMatrix.size() - 1;
        riskDatesAsDouble.reserve( rowSizeWithoutHeader );
        flatShiftJacobian.reserve( rowSizeWithoutHeader );

        // The input matrix contains a header, so start from row 1
        for ( size_t row = 1; row < inputMatrix.size(); ++row )
        {
            // Column 1: Risk Dates - Cast String to Double (Date as Double)
            char * pFirstNonNumber1;
            riskDatesAsDouble.push_back( std::strtod( inputMatrix[row][0].c_str(), &pFirstNonNumber1 ) );
            
            // Column 2: Risk Values - Cast from String to Double
            char * pFirstNonNumber2;
            flatShiftJacobian.push_back( std::strtod( inputMatrix[row][1].c_str() , &pFirstNonNumber2 ) );
        }
    }

    // Populate PERTURBED Jacobian from Server APIs *** using Dates ***
    void JacobianResults::populatePerturbedJacobianFromStringMatrix( DateVector & riskDates, StandardStringVector & perturbedInstruments, DoubleMatrix & perturbedJacobian, const StandardStringMatrix & inputMatrix )
    {
        AQ_REQUIRE( !inputMatrix.empty(),         "Unble to create Jacobian - Input Matrix is empty" )
        AQ_REQUIRE( inputMatrix.size() >= 2,      "Unble to create Jacobian - Input Matrix must have 2 or more rows, a single row of instruments and one or more rows of risk totals" )
        AQ_REQUIRE( inputMatrix[0].size() >= 2,   "Unble to create Jacobian - Input Matrix must have 2 or more columns, a single column of risk dates and one or more columns of risk totals" )

        const size_t rowSizeWithoutHeader = inputMatrix.size() - 1;
        riskDates.reserve( rowSizeWithoutHeader );
        perturbedJacobian.reserve( rowSizeWithoutHeader );

        for ( size_t row = 0; row < inputMatrix.size(); ++row )
        {
            // Row 1: Contains the Perturbed Instruments
            if ( row == 0 )
            {
                // Note The first column is blank so start from index i = 1
                for ( size_t i = 1; i < inputMatrix[0].size(); ++i )
                {
                    perturbedInstruments.push_back( inputMatrix[0][i] );
                }
                continue;
            }

            // Column 1: Risk Dates - Cast String to Date
            riskDates.push_back( etrading::AQLDateScheduleHelpers::getAQLDate( inputMatrix[row][0] ) );
            
            // Column 2 Onwards: Risk Values - Cast from String to Double
            DoubleVector thisJacobianRowWithoutHeader( inputMatrix[row].size() - 1 );

            size_t columnIndex = 0;
            for ( size_t col = 1; col < inputMatrix[row].size(); ++col )
            {
                char * pFirstNonNumber;
                thisJacobianRowWithoutHeader[columnIndex] = std::strtod( inputMatrix[row][col].c_str(), &pFirstNonNumber );
                ++columnIndex;
            }
            perturbedJacobian.push_back( thisJacobianRowWithoutHeader );
        }
    }

    // Populate PERTURBED Jacobian from Object Pool *** using Terms Doubles for Dates ***
    void JacobianResults::populatePerturbedJacobianFromStringMatrix( DoubleVector & riskDatesAsDouble, StandardStringVector & perturbedInstruments, DoubleMatrix & perturbedJacobian, const StandardStringMatrix & inputMatrix )
    {
        AQ_REQUIRE( !inputMatrix.empty(),         "Unble to create Jacobian - Input Matrix is empty" )
        AQ_REQUIRE( inputMatrix.size() >= 2,      "Unble to create Jacobian - Input Matrix must have 2 or more rows, a single row of instruments and one or more rows of risk totals" )
        AQ_REQUIRE( inputMatrix[0].size() >= 2,   "Unble to create Jacobian - Input Matrix must have 2 or more columns, a single column of risk dates and one or more columns of risk totals" )
        
        const size_t rowSizeWithoutHeader = inputMatrix.size() - 1;
        riskDatesAsDouble.reserve( rowSizeWithoutHeader );
        perturbedJacobian.reserve( rowSizeWithoutHeader );

        for ( size_t row = 0; row < inputMatrix.size(); ++row )
        {
            // Row 1: Contains the Perturbed Instruments
            if ( row == 0 )
            {
                // Note The first column is blank so start from index i = 1
                for ( size_t i = 1; i < inputMatrix[0].size(); ++i )
                {
                    perturbedInstruments.push_back( inputMatrix[0][i] );
                }
                continue;
            }

            // Column 1: Risk Dates - Cast String to Double (Date as Double)
            char * pFirstNonNumber;
            riskDatesAsDouble.push_back(  std::strtod( inputMatrix[row][0].c_str(), &pFirstNonNumber ) );
            
            // Column 2 Onwards: Risk Values - Cast from String to Double
            DoubleVector thisJacobianRowWithoutHeader( inputMatrix[row].size() - 1 );
            size_t columnIndex = 0;
            for ( size_t col = 1; col < inputMatrix[row].size(); ++col )
            {
                char * pFirstNonNumber;
                thisJacobianRowWithoutHeader[columnIndex] = std::strtod( inputMatrix[row][col].c_str(), &pFirstNonNumber );
                ++columnIndex;
            }
            perturbedJacobian.push_back( thisJacobianRowWithoutHeader );
        }
    }

    // ================== HELPER METHOD =================================================================================================================

    // Method to consolodate and display the Flat Shift Jacobian
    VariantMatrix JacobianResults::displayFlatShiftJacobian() const
    {
        AQ_REQUIRE( shiftType_ == FLAT_SHIFT_TYPE, "Invalid Jacobian: The shiftType must be FLAT_SHIFT" )
        AQ_REQUIRE( flatShiftJacobian_.size() > 0, "Invalid Jacobian: The Jacobian is empty" )

        // 1.  *** CHECK AND SET DIMENSIONS ***
        // ------------------------------------
        size_t totalColumnSize  = 2;    // Columns: Dates, Values
        size_t headerSize       = 9;    // Headers: RiskType, ShiftType, GradientShiftSize, RowSpace, OutrightInstrument, MarketDataShiftSize, RowSpace, InstrumentHeader, InstumentList
        size_t totalRowSize     = flatShiftJacobian_.size() + headerSize;
            
        VariantMatrix flatShiftJacobianDisplay( totalRowSize );

        // 2.  *** CHECK AND SET RISK DATES ***
        // ------------------------------------
        // Risk Dates are in Double Format when populated by the object pool and in Date format otherwise
        // Therefore we must source the correct risk dates from the riskDateVector or the riskDateVectorInTermFormat
        AQ_REQUIRE( !riskDateVector_.empty() || !riskDateVectorInTermFormat_.empty(), "Invalid Jacobian: Invalid Risk Dates" )
        
        DateVector riskDatesToDisplay = riskDateVector_;
        if ( riskDatesToDisplay.empty() )
        {
            riskDatesToDisplay = etrading::convertCurveTermsToDates( curveDescription_->curveCollection(), riskDateVectorInTermFormat_ );
        }
        
        // Consistency Check & Prevent Access Violations
        AQ_REQUIRE( riskDatesToDisplay.size() == flatShiftJacobian_.size(), "Invalid Jacobian: Inconsistent Number of Risk Dates and Jacobian Matrix Rows" )
        AQ_REQUIRE( riskDatesToDisplay.size() < totalRowSize, "Invalid Jacobian: Inconsistent Number of Risk Dates" )

        // 3.  *** Construct the Jacobain Display ***
        // ------------------------------------------
        for ( size_t row = 0; row < totalRowSize; ++row )
        {
            // Blank Row by Default
            VariantVector thisRow( totalColumnSize, "" );

            if ( row < headerSize )
            {
                // =================== POPULATE HEADERS ==========================================================================

                switch( row )
                {
                    case 0:
                    {
                        // Header 1
                        thisRow[0] = "RiskType";                // Column 1
                        thisRow[1] = toString( riskType_ );     // Column 2
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 1:
                    {
                        // Header 2
                        thisRow[0] = "ShiftType";               // Column 1
                        thisRow[1] = toString( shiftType_ );    // Column 2
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 2:
                    {
                        // Header 3
                        thisRow[0] = "GradientShiftSize";       // Column 1
                        thisRow[1] = gradientShiftSize_;        // Column 2
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 3:
                    {
                        // Header 4 BlankRow
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
					case 4:
                    {
                        // Header 5
                        thisRow[0] = "OutrightInstrument";      // Column 1
                        thisRow[1] = true;						// Column 2
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 5:
                    {
                        // Header 6
                        thisRow[0] = "MarketDataShiftSize";               // Column 1
                        thisRow[1] = flatShiftMarketDataShiftSize_;       // Column 2
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 6:
                    {
                        // Header 7 BlankRow
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 7:
                    {
                        // Header 8
                        thisRow[0] = "";                        // Column 1
                        thisRow[1] = "Instrument";              // Column 2
                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 8:
                    {
                        // Header 9
                        
                        // Column 1
                        switch ( riskType_ )
                        {
                            case DISCOUNT_FACTOR_RISK_TYPE:
                            {
                                thisRow[0] = "PaymentDate";
                                break;
                            }
                            case FORWARD_RATE_RISK_TYPE:
                            {
                                thisRow[0] = "FixingDate";
                                break;
                            }
                            case COMPOUND_RATE_RISK_TYPE:
                            {
                                thisRow[0] = "FixingDate";
                                break;
                            }
                            default:
                            {
                                AQ_THROW( "Invalid Jacobian: Invalid Risk Type: " )
                            }
                        }

                        // Column 2
                        thisRow[1] = "Total";

                        flatShiftJacobianDisplay[row] = thisRow;
                        break;
                    }
                    default:
                    {
                        AQ_THROW( "Invalid Jacobian: Inconsistent Jacobian Headers" )
                    }
                }
            }
            else
            {
                // =================== POPULATE DATA ==========================================================================
                
                thisRow[0] = riskDatesToDisplay[row - headerSize];   // Column 1
                thisRow[1] = flatShiftJacobian_[row - headerSize];   // Column 2
                flatShiftJacobianDisplay[row] = thisRow;
            }
        }
        
        return flatShiftJacobianDisplay;
    }

    // Method to consolodate and display the Perturbed Jacobian
    VariantMatrix JacobianResults::displayPerturbedJacobian() const
    {
        AQ_REQUIRE( shiftType_ == PERTURBED_SHIFT_TYPE, "Invalid Jacobian: The shiftType must be 'PERTURBED'" )
        AQ_REQUIRE( perturbedJacobian_.size() > 0, "Invalid Jacobian: The Jacobian is empty" )

        // 1.  *** CHECK AND SET DIMENSIONS ***
        // ------------------------------------
        size_t headerSize       = 9;    // Headers: RiskType, ShiftType, GradientShiftSize, RowSpace, OutrightInstrument, MarketDataShiftSizes, RowSpace, InstrumentHeader, InstumentList
        size_t totalRowSize     = perturbedJacobian_.size() + headerSize;
        
        size_t extraColumns     = 1;     // Extra Column: RiskDates
        size_t totalColumnSize  = perturbedJacobian_[0].size() + extraColumns;

        VariantMatrix perturbedJacobianDisplay( totalRowSize );

        // 2.  *** CHECK AND SET RISK DATES ***
        // ------------------------------------
        // Risk Dates are in Double Format when populated by the object pool and in Date format otherwise
        // Therefore we must source the correct risk dates from the riskDateVector or the riskDateVectorInTermFormat
        AQ_REQUIRE( !riskDateVector_.empty() || !riskDateVectorInTermFormat_.empty(), "Invalid Jacobian: Invalid Risk Dates" )
        
        DateVector riskDatesToDisplay = riskDateVector_;
        if ( riskDatesToDisplay.empty() )
        {
            riskDatesToDisplay = etrading::convertCurveTermsToDates( curveDescription_->curveCollection(), riskDateVectorInTermFormat_ );
        }
        
        // Consistency Check & Prevent Access Violations
        AQ_REQUIRE( riskDatesToDisplay.size() == perturbedJacobian_.size(), "Invalid Jacobian: Inconsistent Number of Risk Dates and Jacobian Matrix Rows" )
        AQ_REQUIRE( riskDatesToDisplay.size() < totalRowSize, "Invalid Jacobian: Inconsistent Number of Risk Dates" )

        // 3.  *** Construct the Jacobain Display ***
        // ------------------------------------------
        for ( size_t row = 0; row < totalRowSize; ++row )
        {
            // Blank Row by Default
            VariantVector thisRow( totalColumnSize, "" );

            if ( row < headerSize )
            {
                // =================== POPULATE HEADERS ==========================================================================

                switch( row )
                {
                    case 0:
                    {
                        // Header 1
                        thisRow[0] = "RiskType";                // Column 1
                        thisRow[1] = toString( riskType_ );     // Column 2
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 1:
                    {
                        // Header 2
                        thisRow[0] = "ShiftType";               // Column 1
                        thisRow[1] = toString( shiftType_ );    // Column 2
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 2:
                    {
                        // Header 3
                        thisRow[0] = "GradientShiftSize";       // Column 1
                        thisRow[1] = gradientShiftSize_;        // Column 2
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 3:
                    {
                        // Header 4 BlankRow
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
					case 4:
                    {
                        // Header 5
                        thisRow[0] = "OutrightInstrument";				// Column 1

						// Column 2 Onwards: Instrument Is Outright?
						AQ_REQUIRE( perturbedInstrumentsIsOutright_.size() == totalColumnSize - extraColumns, "Invalid Jacobian: Number of Instrument IsOutright parameters must match the number of Jacobian Columns" )
                        for( size_t col = 0; col < perturbedInstrumentsIsOutright_.size(); ++col )
                        {
                            thisRow[col+extraColumns] = perturbedInstrumentsIsOutright_[col];    // Column 2 Onwards
                        }
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 5:
                    {
                        // Header 6
                        thisRow[0] = "MarketDataShiftSize"; // Column 1
                        
                        // Column 2 Onwards: Shift Sizes
                        AQ_REQUIRE( perturbedMarketDataShiftSizes_.size() == totalColumnSize - extraColumns, "Invalid Jacobian: Number of ShiftSizes must match the number of Jacobian Columns" )
                        for( size_t col = 0; col < perturbedMarketDataShiftSizes_.size(); ++col )
                        {
                            thisRow[col+extraColumns] = perturbedMarketDataShiftSizes_[col];    // Column 2 Onwards
                        }
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 6:
                    {
                        // Header 7 BlankRow
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 7:
                    {
                        // Header 8
                        thisRow[0] = "";                        // Column 1
                        thisRow[1] = "Instrument";              // Column 2
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    case 8:
                    {
                        // Header 9
                        
                        // Column 1
                        switch ( riskType_ )
                        {
                            case DISCOUNT_FACTOR_RISK_TYPE:
                            {
                                thisRow[0] = "PaymentDate";
                                break;
                            }
                            case FORWARD_RATE_RISK_TYPE:
                            {
                                thisRow[0] = "FixingDate";
                                break;
                            }
                            case COMPOUND_RATE_RISK_TYPE:
                            {
                                thisRow[0] = "FixingDate";
                                break;
                            }
                            default:
                            {
                                AQ_THROW( "Invalid Jacobian: Invalid Risk Type: " )
                            }
                        }

                        // Column 2 Onwards: Perturbed Instruments
                        AQ_REQUIRE( perturbedInstruments_.size() == totalColumnSize - extraColumns, "Invalid Jacobian: Number of Perturbed Instrument Buckets must match the number of Jacobian Columns" )
                        for( size_t col = 0; col < perturbedInstruments_.size(); ++col )
                        {
                            thisRow[col+extraColumns] = perturbedInstruments_[col];    // Column 2 Onwards
                        }
                        perturbedJacobianDisplay[row] = thisRow;
                        break;
                    }
                    default:
                    {
                        AQ_THROW( "Invalid Jacobian: Inconsistent Jacobian Headers" )
                    }
                }
            }
            else
            {
                // =================== POPULATE DATA ==========================================================================
                
                // Column 1: Risk Dates
                thisRow[0] = riskDatesToDisplay[row - headerSize];   // Column 1
                
                // Column 2 Onwards: Jacobian Data
                for( size_t col = 0; col < perturbedJacobian_[0].size(); ++col )
                {
                    thisRow[col+extraColumns] = perturbedJacobian_[row - headerSize][col];   // Column 2 Onwards
                }

                perturbedJacobianDisplay[row] = thisRow;
            }
        }
        
        return perturbedJacobianDisplay;
    }

    // Method to Calculate the Total Risk across all instruments for each Risk Date.
    DoubleVector JacobianResults::calculateFlatShiftRiskTotals() const
    {
        AQ_REQUIRE( shiftType_ == FLAT_SHIFT_TYPE, "Invalid Shift Type: The shiftType must be 'FLAT_SHIFT'." )
        AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( gradientShiftSize_ ), "Invalid Data: GradientShiftSize cannot be zero or less than 1.0e-14 in absolute terms" )

        size_t nRiskDates       = riskDateVector_.size();
        size_t nJacobianRows    = flatShiftJacobian_.size();
    
        AQ_REQUIRE( nJacobianRows > 0, "Invalid Jacobian: Jacobian is Empty - Null Row Space" )
        AQ_REQUIRE( nRiskDates == nJacobianRows, "Invalid Jacobian: The number of Risk Dates must match the number of Jacobian rows" )
        
        DoubleVector riskTotals( nRiskDates, 0.0 );
        for ( size_t i = 0; i < nRiskDates; ++i )
        {
            // Risk Total: ( Jacobian Change / gradientShiftSize ) * MarketDatashiftSize
            // Note: Divide by zero check is above
            riskTotals[i] = flatShiftJacobian_[i] / gradientShiftSize_ * flatShiftMarketDataShiftSize_;
        }

        return riskTotals;
    }

    // Method to Calculate the Total Risk across all instruments for each Risk Date.
    DoubleVector JacobianResults::calculatePerturbedRiskTotals( const bool & useOutrightInstrumentsOnly ) const
    {
        AQ_REQUIRE( shiftType_ == PERTURBED_SHIFT_TYPE, "Invalid Shift Type: The shiftType must be 'PERTURBED'." )
        AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( gradientShiftSize_ ), "Invalid Data: GradientShiftSize cannot be zero or less than 1.0e-14 in absolute terms" )

        size_t nRiskDates       = riskDateVector_.size();
        size_t nJacobianRows    = perturbedJacobian_.size();
    
        AQ_REQUIRE( nJacobianRows > 0, "Invalid Jacobian: Jacobian is Empty - Null Row Space" )
        AQ_REQUIRE( nRiskDates == nJacobianRows, "Invalid Jacobian: The number of Risk Dates must match the number of Jacobian rows" )
        
        size_t nShiftSizes      = perturbedMarketDataShiftSizes_.size();
        size_t nJacobianColumns = perturbedJacobian_[0].size();
    
        AQ_REQUIRE( nJacobianColumns > 0, "Invalid Jacobian: Jacobian is Empty - Null Column Space" )
        AQ_REQUIRE( nShiftSizes == nJacobianColumns, "Invalid Jacobian: ShiftSizeVector size must equal the number of Jacobian columns"  )

		// Use the perturbedInstrumentIsOutright vector if present, set all columns to isOutright = true otherwise
		// This vector tells us which instrument buckets to use for risk totals, outright instruments are to be included in risk totals and spread instruments excluded.
		std::vector<bool> isOutrightInstrument( nJacobianColumns, true );
		if ( perturbedInstrumentsIsOutright_.size() != 0 )
		{
			AQ_REQUIRE( perturbedInstrumentsIsOutright_.size() == nJacobianColumns, "Invalid Jacobian Data: Number if Jacobian columns must equal size of isOutrightInstrument vector")
			isOutrightInstrument = perturbedInstrumentsIsOutright_;
		};

        DoubleVector riskTotals( nRiskDates, 0.0 );
        for ( size_t row = 0; row < nRiskDates; ++row )
        {
            for( size_t col = 0; col < nJacobianColumns; ++col )
            {
				// Skip Jacobian columns that don't contain outright instruments, if requested.
				if( useOutrightInstrumentsOnly && !isOutrightInstrument[col] )
				{
					continue;
				}

				// Risk Total: ( Jacobian Change / gradientShiftSize ) * MarketDatashiftSize
                // Note: Divide by zero check is above
                riskTotals[row] += perturbedJacobian_[row][col] / gradientShiftSize_ * perturbedMarketDataShiftSizes_[col];
            }
        }

        return riskTotals;
    }

    // Method to Imply New Discount Factors
    DoubleVector JacobianResults::implyNewDiscountFactors() const
    {
        AQ_REQUIRE( discountFactorResults_ != nullptr, "Unable to imply new discount factors: Discount Factor results data is empty" )
        DoubleVector originalDiscountFactors    = discountFactorResults_->discountFactors();
        
        // Calculate the Risk Totals
        DoubleVector riskTotals;
        if ( shiftType_ == FLAT_SHIFT_TYPE )
        {
            riskTotals = calculateFlatShiftRiskTotals();
        }
        else if ( shiftType_ == PERTURBED_SHIFT_TYPE )
        {
            riskTotals = calculatePerturbedRiskTotals();
        }
        else
        {
            AQ_THROW( "Invalid Jacobian: ShiftType must be 'FLAT_SHIFT' or 'PERTURBED'." )
        }

        // Dimension Checks
        size_t nDiscountFactors                 = originalDiscountFactors.size();
        size_t nRiskTotals                      = riskTotals.size();
        AQ_REQUIRE( nDiscountFactors == nRiskTotals, "Invalid Jacobian: Number of Discount Factors must match the number of Risk Dates" )
        
        // Calculate New Discount Factors: Original DiscFact + Jacobian Total Change in DiscFact x ShiftSize
        DoubleVector newDiscountFactors         = originalDiscountFactors;
        for ( size_t i = 0; i < nDiscountFactors; ++i )
        {
            newDiscountFactors[i] += riskTotals[i];
        }

        return newDiscountFactors;            
    }
}
