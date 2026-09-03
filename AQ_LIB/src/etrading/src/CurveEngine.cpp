// CurveEngine.cpp

/*
 * @brief			OIS Curve Calibration Class
 * @Created:		3rd April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#include "CurveEngine.h"
#include "LAUpdateStaticDataManager.h"      // LA Curve Calibration
#include "ParameterValidation.h"            // etrading::getDataInstance()
#include "CurveValidation.h"                // Get Curve Information e.g. AsOfDate, Interpolation et al.
#include "CurveUtilities.h"                 // Curve Interpolation Join Date
#include "LADataVector.h"                   // Needed to get Terms and Discount Factors for curve from Object Pool "LADataDoubles" object class
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results
#include <boost/algorithm/string.hpp>       // boost::iequals
#include "JacobianResults.h"                // JacobianResults Helper Methods
#include "DataUtilities.h"					// for MLIB_TO_STRING_FROM_SIZE_T macro

namespace etrading
{
    // ======================================= CURVE ENGINE CLASS =====================================================
    
    // Copy Constructor
    CurveEngine::CurveEngine( const CurveEngine& rhs ) 
        :   curveDescription_(rhs.curveDescription_),
            curveResults_(rhs.curveResults_),
			curveConventionsAndMarketData_(rhs.curveConventionsAndMarketData_),
            oisCurveData_(rhs.oisCurveData_),
            arrCurveData_(rhs.arrCurveData_),
		    swapCurveData_(rhs.swapCurveData_),
            tenorBasisCurveData_(rhs.tenorBasisCurveData_),
            xccyBasisCurveData_(rhs.xccyBasisCurveData_),
            fxFwdConstantCurveData_(rhs.fxFwdConstantCurveData_)
	{
	}

    // Assignment Operator
    CurveEngine & CurveEngine::operator=( const CurveEngine & rhs )
    {
		// For Performance 
		if ( &rhs == this )
		{
			return *this;
		}

        // For Exception Safety
            
        // 1. Make a temp copy
        CurveEngine temp( rhs );

        // 2. Swap Data Members with the temp copy
		std::swap( curveDescription_,					temp.curveDescription_ );
        std::swap( curveResults_,						temp.curveResults_ );
		std::swap( curveConventionsAndMarketData_,      temp.curveConventionsAndMarketData_ );
        std::swap( oisCurveData_,						temp.oisCurveData_ );
        std::swap( arrCurveData_,						temp.arrCurveData_ );
        std::swap( swapCurveData_,						temp.swapCurveData_ );
        std::swap( tenorBasisCurveData_,				temp.tenorBasisCurveData_ );
        std::swap( xccyBasisCurveData_,					temp.xccyBasisCurveData_ );
        std::swap( fxFwdConstantCurveData_,				temp.fxFwdConstantCurveData_ );
        
        return *this;
    }

    // ================CURVE ENGINE PRIVATE MEMBER ACCESSORS - WITH NULLPTR CHECK ============================

	std::shared_ptr<CurveDescription> CurveEngine::curveDescription() const
    { 
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        return curveDescription_; 
    }
    
    
    std::shared_ptr<CurveResults> CurveEngine::curveResults() const
    { 
        MLIB_REQUIRE( curveResults_ != nullptr, "Curve Results are Missing" )
        return curveResults_;
    }
    
	std::shared_ptr<CurveConventionsAndMarketData> CurveEngine::curveConventionsAndMarketData() const
	{
		MLIB_REQUIRE( curveConventionsAndMarketData_ != nullptr, "Curve Static Data Conventions and Market Data are Missing" )
        return curveConventionsAndMarketData_;
	}

    std::shared_ptr<OISCurveObjectData> CurveEngine::oisCurveData() const
    { 
        MLIB_REQUIRE( oisCurveData_ != nullptr, "OIS Curve Market Data is Missing" )
        return oisCurveData_; 
    }

	std::shared_ptr<ARRCurveObjectData> CurveEngine::arrCurveData() const
    { 
        MLIB_REQUIRE( arrCurveData_ != nullptr, "ARR Curve Market Data is Missing" )
        return arrCurveData_;
    }

	std::shared_ptr<SwapCurveObjectData> CurveEngine::swapCurveData() const
    {
        MLIB_REQUIRE( swapCurveData_ != nullptr, "Swap Curve Market Data is Missing" )
        return swapCurveData_;
    }
    
    std::shared_ptr<TenorBasisCurveObjectData> CurveEngine::tenorBasisCurveData() const
    { 
        MLIB_REQUIRE( tenorBasisCurveData_ != nullptr, "Tenor Basis Curve MarketData is Missing" )
        return tenorBasisCurveData_;
    }
    
    std::shared_ptr<XccyBasisCurveObjectData> CurveEngine::xccyBasisCurveData() const
    {
        MLIB_REQUIRE( xccyBasisCurveData_ != nullptr, "Xccy Basis Curve Market Data is Missing" )
        return xccyBasisCurveData_;
    }
    
    std::shared_ptr<FwdConstantCurveObjectData> CurveEngine::fxFwdConstantCurveData() const
    {
        MLIB_REQUIRE( fxFwdConstantCurveData_ != nullptr, "FX Fwd Constant Curve Market Data is Missing" )
        return fxFwdConstantCurveData_;
    }


    // -------------------------------- SINGLE CURVE CONSTRUCTORS --------------------------------------------------

    // Curve Info Constructor
    CurveEngine::CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription )
        : curveDescription_(curveDescription)
    {
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )

        // 1. Curve Description
        const std::string curveCollection           = curveDescription_->curveCollection();
        const std::string objectPoolLookupTable     = curveDescription_->objectPoolLookupTable();
        
		// 2. Populate Calibration Results Object
		curveResults_ = std::make_shared<CurveResults>( curveDescription );

		// 3. Persist curveResults in Singleton Curve Results Container for the Index and any alias Indices
		const LAStringVector curveIndexAliasList = etrading::curveIndexAliasList( curveCollection.c_str(), objectPoolLookupTable.c_str() );
		for( auto curveIndex : curveIndexAliasList )
		{
			CurveResultsContainer::getInstance().addCurveResults( curveCollection, curveIndex.getCString(), curveResults_ );
		}
    }

    // Single Curve Constructor - ARR Curve
	CurveEngine::CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
							  const std::shared_ptr<ARRCurveObjectData> & arrCurveData)
		: curveDescription_(curveDescription), arrCurveData_(arrCurveData) /* curveConventionsAndMarketData_ in body */
	{
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( arrCurveData_ != nullptr, "ARR Curve Market Data is Missing" )
		
		// LEGACY OBJECT POOL CALIBRATION ONLY
		// -------------------------------------------
		if( !isEnabledCurveResults() )
		{
			calibrateARRCurve();
			return;
		}

		// OBJECT POOL AND CURVE RESULTS CALIBRATION
		// -------------------------------------------

		// Create and Store the Curve Conventions & Market Data Object
		curveConventionsAndMarketData_ = std::shared_ptr< etrading::CurveConventionsAndMarketData>( new CurveConventionsAndMarketData( arrCurveData ) );
		
        // This method checks what curve build type is required and takes into account if JacobianBuildFrequency 'ALWAYS', 'ONCE' etc ...
        const ShiftTypeEnum curveBuildType = curveRiskMetricsRequired();
        
        switch ( curveBuildType )
        {
            case NONE_SHIFT_TYPE:
            {
                // 1. Calibrate ARR Curve
                calibrateARRCurve();

                // 2. Update Curve Results Object - We also preserve the Jacobian if already built
				updateCurveResults();
                
                break;
            }
            case FLAT_SHIFT_TYPE:
            {
				// Get Jacobian Parameters
                const double gradientShiftSize		= arrCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE, DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= arrCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE, DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate ARR Curve and Compute the Flat-Shift Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateARRCurveAndCalculateFlatShiftJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndFlatShiftJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            case PERTURBED_SHIFT_TYPE:
            {
                // Get Jacobian Parameters
                const double gradientShiftSize		= arrCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE, DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= arrCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE, DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate ARR Curve and Compute the Perturbed Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateARRCurveAndCalculatePerturbedJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndPerturbedJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve JacobianShiftType: Must be 'FLAT_SHIFT', 'PERTURBED' or 'NONE'")
                break;
            }
        }

        // Store the Curve Results for the Curve Index and any alias Indices
        storeCurveResults();
	}

	// Single Curve Constructor - OIS Curve
    CurveEngine::CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
							  const std::shared_ptr<OISCurveObjectData> & oisCurveData )
        : curveDescription_(curveDescription), oisCurveData_(oisCurveData) /* curveConventionsAndMarketData_ in body */
    {
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
		MLIB_REQUIRE( oisCurveData_ != nullptr, "OIS Curve Market Data is Missing" )

		// LEGACY OBJECT POOL CALIBRATION ONLY
		// -------------------------------------------
		if( !isEnabledCurveResults() )
		{
			calibrateOISCurve();
			return;
		}

		// OBJECT POOL AND CURVE RESULTS CALIBRATION
		// -------------------------------------------

		// Create and Store the Curve Conventions & Market Data Object
		curveConventionsAndMarketData_ = std::shared_ptr< etrading::CurveConventionsAndMarketData>( new CurveConventionsAndMarketData( oisCurveData ) );

        // This method checks what curve build type is required and takes into account if JacobianBuildFrequency 'ALWAYS', 'ONCE' etc ...
        const ShiftTypeEnum curveBuildType = curveRiskMetricsRequired();
        
        switch ( curveBuildType )
        {
            case NONE_SHIFT_TYPE:
            {
                // 1. Calibrate OIS Curve
                calibrateOISCurve();

                // 2. Update Curve Results Object - We also preserve the Jacobian if already built
				updateCurveResults();
                
                break;
            }
            case FLAT_SHIFT_TYPE:
            {
				// Get Jacobian Parameters
                const double gradientShiftSize		= oisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,		DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= oisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE,	DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")
				
				// 1. Calibrate OIS Curve and Compute the Flat-Shift Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateOISCurveAndCalculateFlatShiftJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndFlatShiftJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );
				
                break;
            }
            case PERTURBED_SHIFT_TYPE:
            {
                // Get Jacobian Parameters
                const double gradientShiftSize		= oisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,		DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= oisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE,	DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")
				
				// 1. Calibrate OIS Curve and Compute the Perturbed Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateOISCurveAndCalculatePerturbedJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndPerturbedJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve JacobianShiftType: Must be 'FLAT_SHIFT', 'PERTURBED' or 'NONE'")
                break;
            }
        }

        // Store the Curve Results for the Curve Index and any alias Indices
        storeCurveResults();
    }

	// Single Curve Constructor - Swap Curve
    CurveEngine::CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
							  const std::shared_ptr<SwapCurveObjectData> & swapCurveData )
        : curveDescription_(curveDescription), swapCurveData_(swapCurveData) /* curveConventionsAndMarketData_ in body */
    {
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( swapCurveData_ != nullptr, "Swap Curve Market Data is Missing" )
		
		// LEGACY OBJECT POOL CALIBRATION ONLY
		// -------------------------------------------
		if( !isEnabledCurveResults() )
		{
			calibrateSwapCurve();
			return;
		}

		// OBJECT POOL AND CURVE RESULTS CALIBRATION
		// -------------------------------------------

		// Create and Store the Curve Conventions & Market Data Object
		curveConventionsAndMarketData_ = std::shared_ptr< etrading::CurveConventionsAndMarketData>( new CurveConventionsAndMarketData( swapCurveData ) );
		
        // This method checks what curve build type is required and takes into account if JacobianBuildFrequency 'ALWAYS', 'ONCE' etc ...
        const ShiftTypeEnum curveBuildType = curveRiskMetricsRequired();
        
        switch ( curveBuildType )
        {
            case NONE_SHIFT_TYPE:
            {
                // 1. Calibrate SWAP Curve
                calibrateSwapCurve();

                // 2. Update Curve Results Object - We also preserve the Jacobian if already built
				updateCurveResults();

                break;
            }
            case FLAT_SHIFT_TYPE:
            {
				// Get Jacobian Parameters
                const double gradientShiftSize		= swapCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,		DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= swapCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE,	DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate Swap Curve and Compute the Flat-Shift Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateSwapCurveAndCalculateFlatShiftJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndFlatShiftJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            case PERTURBED_SHIFT_TYPE:
            {
                // Get Jacobian Parameters
                const double gradientShiftSize		= swapCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,		DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= swapCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE,	DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate Swap Curve and Compute the Perturbed Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateSwapCurveAndCalculatePerturbedJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndPerturbedJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve JacobianShiftType: Must be 'FLAT_SHIFT', 'PERTURBED' or 'NONE'")
                break;
            }
        }

        // Store the Curve Results for the Curve Index and any alias Indices
        storeCurveResults();

    }

    // Single Curve Constructor - Tenor Basis Curve
    CurveEngine::CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
							  const std::shared_ptr<TenorBasisCurveObjectData> & tenorBasisCurveData )
        : curveDescription_(curveDescription), tenorBasisCurveData_(tenorBasisCurveData) /* curveConventionsAndMarketData_ in body */
    {
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( tenorBasisCurveData_ != nullptr, "Tenor Basis Curve Market Data is Missing" )
		
		// LEGACY OBJECT POOL CALIBRATION ONLY
		// -------------------------------------------
		if( !isEnabledCurveResults() )
		{
			calibrateTenorBasisCurve();
			return;
		}

		// OBJECT POOL AND CURVE RESULTS CALIBRATION
		// -------------------------------------------

		// Create and Store the Curve Conventions & Market Data Object
		curveConventionsAndMarketData_ = std::shared_ptr< etrading::CurveConventionsAndMarketData>( new CurveConventionsAndMarketData( tenorBasisCurveData ) );

        // This method checks what curve build type is required and takes into account if JacobianBuildFrequency 'ALWAYS', 'ONCE' etc ...
        const ShiftTypeEnum curveBuildType = curveRiskMetricsRequired();
        
        switch ( curveBuildType )
        {
            case NONE_SHIFT_TYPE:
            {
                // 1. Calibrate Tenor Basis Curve
                calibrateTenorBasisCurve();

                // 2. Update Curve Results Object - We also preserve the Jacobian if already built
				updateCurveResults();

                break;
            }
            case FLAT_SHIFT_TYPE:
            {
				// Get Jacobian Parameters
                const double gradientShiftSize		= tenorBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,		DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= tenorBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE,	DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")
                
				// 1. Calibrate Swap Curve and Compute the Flat-Shift Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateTenorBasisCurveAndCalculateFlatShiftJacobian( gradientShiftSize );
            
               // 2. Update Curve Results and Jacobian
				updateCurveResultsAndFlatShiftJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            case PERTURBED_SHIFT_TYPE:
            {
                // Get Jacobian Parameters
                const double gradientShiftSize		= tenorBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,		DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= tenorBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE,	DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")
                
				// 1. Calibrate Swap Curve and Compute the Perturbed Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateTenorBasisCurveAndCalculatePerturbedJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndPerturbedJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve JacobianShiftType: Must be 'FLAT_SHIFT', 'PERTURBED' or 'NONE'")
                break;
            }
        }

        // Store the Curve Results for the Curve Index and any alias Indices
        storeCurveResults();
    }

    // Single Curve Constructor - Xccy Curve (USD CSA Curves)
    CurveEngine::CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
							  const std::shared_ptr<XccyBasisCurveObjectData> & xccyBasisCurveData )
        : curveDescription_(curveDescription), xccyBasisCurveData_(xccyBasisCurveData) /* curveConventionsAndMarketData_ in body */
    {
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( xccyBasisCurveData_ != nullptr, "Xccy Basis Curve Market Data is Missing" )
		
		// LEGACY OBJECT POOL CALIBRATION ONLY
		// -------------------------------------------
		if( !isEnabledCurveResults() )
		{
			calibrateXccyBasisCurve();
			return;
		}

		// OBJECT POOL AND CURVE RESULTS CALIBRATION
		// -------------------------------------------

		// Create and Store the Curve Conventions & Market Data Object
		curveConventionsAndMarketData_ = std::shared_ptr< etrading::CurveConventionsAndMarketData>( new CurveConventionsAndMarketData( xccyBasisCurveData ) );
		
        // This method checks what curve build type is required and takes into account if JacobianBuildFrequency 'ALWAYS', 'ONCE' etc ...
        const ShiftTypeEnum curveBuildType = curveRiskMetricsRequired();
        
        switch ( curveBuildType )
        {
            case NONE_SHIFT_TYPE:
            {
                // 1. Calibrate Xccy Basis Curve
                calibrateXccyBasisCurve();

               // 2. Update Curve Results Object - We also preserve the Jacobian if already built
				updateCurveResults();

                break;
            }
            case FLAT_SHIFT_TYPE:
            {
				// Get Jacobian Parameters
                const double gradientShiftSize		= xccyBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,	DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= xccyBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE, DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate Swap Curve and Compute the Flat-Shift Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateXccyBasisCurveAndCalculateFlatShiftJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndFlatShiftJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            case PERTURBED_SHIFT_TYPE:
            {
                // Get Jacobian Parameters
                const double gradientShiftSize		= xccyBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,	DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= xccyBasisCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE, DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate Swap Curve and Compute the Perturbed Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateXccyBasisCurveAndCalculatePerturbedJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndPerturbedJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve JacobianShiftType: Must be 'FLAT_SHIFT', 'PERTURBED' or 'NONE'")
                break;
            }
        }

        // Store the Curve Results for the Curve Index and any alias Indices
        storeCurveResults();
	}

	// Single Curve Constructor - FX Forward Constant Curve (Non-USD CSA Curves)
	CurveEngine::CurveEngine( const std::shared_ptr<CurveDescription> & curveDescription,
							  const std::shared_ptr<FwdConstantCurveObjectData> & fxFwdConstantCurveData )
		: curveDescription_( curveDescription ), fxFwdConstantCurveData_( fxFwdConstantCurveData ) /* curveConventionsAndMarketData_ in body */
	{
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( fxFwdConstantCurveData_ != nullptr, "FX Forward Constant Curve Market Data is Missing" )
		
		// LEGACY OBJECT POOL CALIBRATION ONLY
		// -------------------------------------------
		if( !isEnabledCurveResults() )
		{
			calibrateFwdFXConstantCurve();
			return;
		}

		// OBJECT POOL AND CURVE RESULTS CALIBRATION
		// -------------------------------------------

		// Create and Store the Curve Conventions & Market Data Object
		curveConventionsAndMarketData_ = std::shared_ptr< etrading::CurveConventionsAndMarketData>( new CurveConventionsAndMarketData( fxFwdConstantCurveData ) );
		
		// This method checks what curve build type is required and takes into account if JacobianBuildFrequency 'ALWAYS', 'ONCE' etc ...
        const ShiftTypeEnum curveBuildType = curveRiskMetricsRequired();
        
        switch ( curveBuildType )
        {
            case NONE_SHIFT_TYPE:
            {
                // 1. Calibrate the FX Forward Constant Curve
                calibrateFwdFXConstantCurve();

                // 2. Update Curve Results Object - We also preserve the Jacobian if already built
				updateCurveResults();

                break;
            }
            case FLAT_SHIFT_TYPE:
            {
				// Get Jacobian Parameters
                const double gradientShiftSize		= fxFwdConstantCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,	DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= fxFwdConstantCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE, DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate Swap Curve and Compute the Flat-Shift Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateFxForwardConstantCurveAndCalculateFlatShiftJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndFlatShiftJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            case PERTURBED_SHIFT_TYPE:
            {
                // Get Jacobian Parameters
                const double gradientShiftSize		= fxFwdConstantCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_GRADIENT_SHIFT_SIZE,	DEFAULT_GRADIENT_SHIFT_SIZE );
				const double marketDatetShiftSize	= fxFwdConstantCurveData_->curveConvLVB_.getOptionalValueAsDouble( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_MARKET_DATA_SHIFT_SIZE, DEFAULT_MARKET_DATA_SHIFT_SIZE );
				MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Curve Input: 'JacobianGradientShiftSize' parameter must not be set to zero")

                // 1. Calibrate Swap Curve and Compute the Perturbed Jacobian
				std::shared_ptr<JacobianData> jacobianData = calibrateFxForwardConstantCurveAndCalculatePerturbedJacobian( gradientShiftSize );
            
                // 2. Update Curve Results and Jacobian
				updateCurveResultsAndPerturbedJacobian( gradientShiftSize, marketDatetShiftSize, jacobianData );

                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve JacobianShiftType: Must be 'FLAT_SHIFT', 'PERTURBED' or 'NONE'")
                break;
            }
        }

        // Store the Curve Results for the Curve Index and any alias Indices
        storeCurveResults();
	}


    // ----------------------------------- CALIBRATION HELPER METHODS --------------------------------------------------
    
    // Method to get the Jacobian Results, will return nullptr if they don't exist
    std::shared_ptr<etrading::JacobianResults> CurveEngine::getJacobianResultsByDiscountFactor() const
    {
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )

        std::shared_ptr<etrading::JacobianResults> result = nullptr;
        if( etrading::doesExistCurveResultsJacobianByDiscountFactor( curveDescription_->curveCollection(), curveDescription_->objectPoolLookupTable() ) )
        {
            result = CurveResultsContainer::getInstance().getCurveResults( curveDescription_->curveCollection(), curveDescription_->objectPoolLookupTable() )->jacobianResultsByDiscountFactor();
        }
        return result;
    }
	
	// Method to get the Jacobian Results, will return nullptr if they don't exist
    std::shared_ptr<etrading::JacobianResults> CurveEngine::getJacobianResultsByForwardRate() const
    {
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )

        std::shared_ptr<etrading::JacobianResults> result = nullptr;
        if( etrading::doesExistCurveResultsJacobianByForwardRate( curveDescription_->curveCollection(), curveDescription_->objectPoolLookupTable() ) )
        {
            result = CurveResultsContainer::getInstance().getCurveResults( curveDescription_->curveCollection(), curveDescription_->objectPoolLookupTable() )->jacobianResultsByForwardRate();
        }
        return result;
    }

	// Method to get the Jacobian Results, will return nullptr if they don't exist
    std::shared_ptr<etrading::JacobianResults> CurveEngine::getJacobianResultsByCompoundRate() const
    {
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )

        std::shared_ptr<etrading::JacobianResults> result = nullptr;
        if( etrading::doesExistCurveResultsJacobianByCompoundRate( curveDescription_->curveCollection(), curveDescription_->objectPoolLookupTable() ) )
        {
            result = CurveResultsContainer::getInstance().getCurveResults( curveDescription_->curveCollection(), curveDescription_->objectPoolLookupTable() )->jacobianResultsByCompoundRate();
        }
        return result;
    }

    // Method to evaluate which curve risk metrics to calcuate as part of the curve build
    ShiftTypeEnum CurveEngine::curveRiskMetricsRequired() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )

        std::string jacobianBuildFrequency;
        std::string jacobianShiftType;

        // 1. Get the Jacobian Parameters from the appropriate curve and respective convention LVB
        // ================================================================================================================
        switch ( curveDescription_->curveTypeEnum() )
        {
            case OIS_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( oisCurveData_ != nullptr, "OIS Curve Market Data is Missing" )

                jacobianBuildFrequency    = oisCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_BUILD_FREQUENCY,  "NEVER" );
                jacobianShiftType         = oisCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_SHIFT_TYPE,       "NONE" );
                break;
            }
            case ARR_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( arrCurveData_ != nullptr, "ARR Curve Market Data is Missing" )

                jacobianBuildFrequency    = arrCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_BUILD_FREQUENCY,  "NEVER" );
                jacobianShiftType         = arrCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_SHIFT_TYPE,       "NONE" );
                break;
            }
            case SWAP_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( swapCurveData_ != nullptr, "Swap Curve Market Data is Missing" )

                jacobianBuildFrequency    = swapCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_BUILD_FREQUENCY,  "NEVER" );
                jacobianShiftType         = swapCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_SHIFT_TYPE,       "NONE" );
                break;
            }
            case TENORBASIS_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( tenorBasisCurveData_ != nullptr, "Tenor Basis Curve Market Data is Missing" )

                jacobianBuildFrequency    = tenorBasisCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_BUILD_FREQUENCY,  "NEVER" );
                jacobianShiftType         = tenorBasisCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_SHIFT_TYPE,       "NONE" );
                break;
            }
            case XCCYBASIS_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( xccyBasisCurveData_ != nullptr, "Xccy Basis Curve Market Data is Missing" )

                jacobianBuildFrequency    = xccyBasisCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_BUILD_FREQUENCY,  "NEVER" );
                jacobianShiftType         = xccyBasisCurveData_->curveConvLVB_.getOptionalValueAsString( CURVEGENERATOR_CURVEPROPERTIES_KEY::JACOBIAN_SHIFT_TYPE,       "NONE" );
                break;
            }
            case FWDFXCONST_CURVETYPE:
            {
                // No need to bump the FWDFXCONST Curve
                return NONE_SHIFT_TYPE;
                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve Type '" + toString( curveDescription_->curveTypeEnum() ) + "'")
                break;
            }
        }

        // 2. Validate Inputs
        // ================================================================================================================
        
        const ShiftTypeEnum jacobianShiftTypeEnum               = toShiftTypeEnum( jacobianShiftType ); // Non-Const - This is our result, which we may override in this method
        const BuildFrequencyEnum jacobianBuildFrequencyEnum     = toBuildFrequencyEnum( jacobianBuildFrequency );

		MLIB_REQUIRE( jacobianBuildFrequencyEnum != TOLERANCE_BUILD_FREQUENCY, "Invalid JacobianBuildFrequency: 'TOLERANCE' method not supported")

        // 3. Case when no need to build the curve risks
        // ================================================================================================================
        
        if ( jacobianShiftTypeEnum == NONE_SHIFT_TYPE || jacobianBuildFrequencyEnum == NEVER_BUILD_FREQUENCY )
        {
            return NONE_SHIFT_TYPE;
        }

        // 4. Check the Risk Metrics Shift-Type
        // ================================================================================================================
        // Here we set which (if any) Curve Risk Metrics to build, based on the chosen Shift Type and the Build Frequency. We only build the
        // risk metrics once unless we are asked to always build them, so we check if they already exist. Therefore we get the default Shift-Type
        // above and set to NONE_SHIFT_TYPE if risk has already been calculated or if it is not required.
        
        switch ( jacobianBuildFrequencyEnum )
        {
            case ALWAYS_BUILD_FREQUENCY:
            {
                return jacobianShiftTypeEnum;
                break;
            }
            case ONCE_BUILD_FREQUENCY:
            {
                // Return NONE_SHIFT_TYPE if the risk jacobian exists and we asked to build it 'ONCE'
                const bool jacobianExists = doesExistCurveResultsJacobianByDiscountFactor( curveDescription_->curveCollection(), curveDescription_->objectPoolLookupTable() );
                if ( jacobianExists )
                {
                    return NONE_SHIFT_TYPE;
                }
                else
                {
                    return jacobianShiftTypeEnum;
                }
                break;
            }
            // TODO: Implement this method
            case TOLERANCE_BUILD_FREQUENCY:
            default:
            {
                MLIB_THROW("Invalid JacobianBuildFrequency: Only 'NEVER', 'ALWAYS', 'ONCE' JacobianBuildFrequency supported")
            }
        }
        
        // We should never reach here
        return jacobianShiftTypeEnum;
    }

	// FLAT-SHIFT JACOBIANS
	// ====================

    // Method to calibrate the ARR curve and calculate the flat-shift Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateARRCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize ) const
    {
        // 1. Bump ARR Market Data
        arrCurveData()->applyFlatShift( gradientShiftSize );

        // 2. Calibrate Bumped Curve and Extract Discount Factors
        calibrateARRCurve();
        std::shared_ptr<DiscountFactorResults> flatShiftedDFObject = createDiscountFactorResultsObject();
        
        // 3. Reset ARR Market Data to Original Unbumped Data
        arrCurveData()->restoreDataAndClearFlatShift();

        // 4. Calibrate Original Curve and Extract Discount Factors
        calibrateARRCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();
        
        // 5.  Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( originalDFObject, flatShiftedDFObject, false ) );
        return curveJacobianData;
    }

	// Method to calibrate the OIS curve and calculate the flat-shift Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateOISCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize ) const
    {
        // 1. Bump OIS Market Data
        oisCurveData()->applyFlatShift( gradientShiftSize );

        // 2. Calibrate Bumped Curve and Extract Discount Factors
        calibrateOISCurve();
        std::shared_ptr<DiscountFactorResults> flatShiftedDFObject = createDiscountFactorResultsObject();
        
        // 3. Reset OIS Market Data to Original Unbumped Data
        oisCurveData()->restoreDataAndClearFlatShift();

        // 4. Calibrate Original Curve and Extract Discount Factors
        calibrateOISCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();
        
        // 5.  Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( originalDFObject, flatShiftedDFObject, false ) );
        return curveJacobianData;
    }

    // Method to calibrate the Swap curve and calculate the flat-shift Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateSwapCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize ) const
    {
        // 1. Bump Swap Market Data
        swapCurveData()->applyFlatShift( gradientShiftSize );

        // 2. Calibrate Bumped Curve and Extract Discount Factors
        calibrateSwapCurve();
        std::shared_ptr<DiscountFactorResults> flatShiftedDFObject = createDiscountFactorResultsObject();
        
        // 3. Reset Swap Market Data to Original Unbumped Data
        swapCurveData()->restoreDataAndClearFlatShift();

        // 4. Calibrate Original Curve and Extract Discount Factors
        calibrateSwapCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();
        
        // 5.  Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( originalDFObject, flatShiftedDFObject, false ) );
        return curveJacobianData;
    }
    
    // Method to calibrate the Tenor Basis curve and calculate the flat-shift Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateTenorBasisCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize ) const
    {
        // 1. Bump Tenor Basis Market Data
        tenorBasisCurveData()->applyFlatShift( gradientShiftSize );

        // 2. Calibrate Bumped Curve and Extract Discount Factors
        calibrateTenorBasisCurve();
        std::shared_ptr<DiscountFactorResults> flatShiftedDFObject = createDiscountFactorResultsObject();
        
        // 3. Reset Tenor Basis Market Data to Original Unbumped Data
        tenorBasisCurveData()->restoreDataAndClearFlatShift();

        // 4. Calibrate Original Curve and Extract Discount Factors
        calibrateTenorBasisCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();
        
        // 5.  Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( originalDFObject, flatShiftedDFObject, false ) );
        return curveJacobianData;
    }

    // Method to calibrate the Xccy Basis curve and calculate the flat-shift Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateXccyBasisCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize ) const
    {
        // 1. Bump Xccy Basis Market Data
        xccyBasisCurveData()->applyFlatShift( gradientShiftSize );

        // 2. Calibrate Bumped Curve and Extract Discount Factors
        calibrateXccyBasisCurve();
        std::shared_ptr<DiscountFactorResults> flatShiftedDFObject = createDiscountFactorResultsObject();
        
        // 3. Reset Xccy Basis Market Data to Original Unbumped Data
        xccyBasisCurveData()->restoreDataAndClearFlatShift();

        // 4. Calibrate Original Curve and Extract Discount Factors
        calibrateXccyBasisCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();
        
        // 5.  Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( originalDFObject, flatShiftedDFObject, false ) );
        return curveJacobianData;
    }

    // Method to calibrate the FX Forward Constant curve and calculate the flat-shift Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateFxForwardConstantCurveAndCalculateFlatShiftJacobian( const double gradientShiftSize ) const
    {
        // 1. Bump FX Forward Constant Market Data
        fxFwdConstantCurveData()->applyFlatShift( gradientShiftSize );

        // 2. Calibrate Bumped Curve and Extract Discount Factors
        calibrateFwdFXConstantCurve();
        std::shared_ptr<DiscountFactorResults> flatShiftedDFObject = createDiscountFactorResultsObject();
        
        // 3. Reset FX Forward Constant Market Data to Original Unbumped Data
        fxFwdConstantCurveData()->restoreDataAndClearFlatShift();

        // 4. Calibrate Original Curve and Extract Discount Factors
        calibrateFwdFXConstantCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();
        
        // 5.  Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( originalDFObject, flatShiftedDFObject ) );
        return curveJacobianData;
    }


	// PERTURBED JACOBIANS
	// ====================

	// Method to calibrate the ARR curve and calculate the Perturbed Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateARRCurveAndCalculatePerturbedJacobian( const double gradientShiftSize ) const
    {
		// 1. Calibrate Original Discount Factors
		calibrateARRCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();

		// 2. Create Vector of Perturbed Instruments Discount Factor Objects
		StandardStringVector perturbedInstrumentList;
		std::vector<bool> perturbedInstrumentIsOutright;
		StandardStringVector perturbedInstrumentTenors;
		DoubleVector perturbedMarketDataShiftSizes;
		std::vector<std::shared_ptr<DiscountFactorResults> > perturbedDFObjects;

		// 3. Perturb OIS Swaps
		// ==========================
		const size_t nOISInstruments = arrCurveData_->oisRates_.size();
		const StandardStringVector perturbedOISTenors = arrCurveData_->instrumentTenors( OIS_MARKETDATA );
		MLIB_REQUIRE( perturbedOISTenors.size() == nOISInstruments, "Invalid Market Data: Number of OIS Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nOISInstruments; ++i )
		{
			// i. Perturb Market Data
			arrCurveData_->perturb( OIS_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateARRCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("OIS_" + perturbedOISTenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			arrCurveData_->restoreDataAndClearPerturbation();
		}
		
		// 4. Perturb Libor-OIS Swaps
		// ==========================
		const size_t nLiborOISInstruments = arrCurveData_->loBasisRates_.size();
		const StandardStringVector perturbedLiborOISTenors = arrCurveData_->instrumentTenors( LIBOR_OIS_BASISSPREAD_MARKETDATA );
		MLIB_REQUIRE( perturbedLiborOISTenors.size() == nLiborOISInstruments, "Invalid Market Data: Number of Libor-OIS Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nLiborOISInstruments; ++i )
		{
			// i. Perturb Market Data
			arrCurveData_->perturb( LIBOR_OIS_BASISSPREAD_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateARRCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("LIBOROIS_" + perturbedLiborOISTenors[i] );
			perturbedInstrumentIsOutright.push_back( false );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			arrCurveData_->restoreDataAndClearPerturbation();
		}

		// 5. Perturb Swaps
		// ==========================
		const size_t nSwapInstruments = arrCurveData_->swapRates_.size();
		const StandardStringVector perturbedSwapTenors = arrCurveData_->instrumentTenors( SWAP_MARKETDATA );
		MLIB_REQUIRE( perturbedSwapTenors.size() == nSwapInstruments, "Invalid Market Data: Number of Swap Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nSwapInstruments; ++i )
		{
			// i. Perturb Market Data
			arrCurveData_->perturb( SWAP_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateARRCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("SWAP_" + perturbedSwapTenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			arrCurveData_->restoreDataAndClearPerturbation();
		}

        // 7. Restore and Recalibrate Original Curve
        arrCurveData_->restoreDataAndClearPerturbation();
		calibrateARRCurve();
        
        // 8. Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( perturbedMarketDataShiftSizes, perturbedInstrumentList, perturbedInstrumentIsOutright, originalDFObject, perturbedDFObjects ) );
        
		return curveJacobianData;
    }

	// Method to calibrate the OIS curve and calculate the Perturbed Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateOISCurveAndCalculatePerturbedJacobian( const double gradientShiftSize ) const
    {
		// 1. Calibrate Original Discount Factors
		calibrateOISCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();

		// 2. Create Vector of Perturbed Instruments Discount Factor Objects
		StandardStringVector perturbedInstrumentList;
		std::vector<bool> perturbedInstrumentIsOutright;
		DoubleVector perturbedMarketDataShiftSizes;
		std::vector<std::shared_ptr<DiscountFactorResults> > perturbedDFObjects;

		// 3. Perturb OIS Swaps
		// ==========================
		const size_t nOISInstruments = oisCurveData_->oisRates_.size();
		const StandardStringVector perturbedOISTenors = oisCurveData_->instrumentTenors( OIS_MARKETDATA );
		MLIB_REQUIRE( perturbedOISTenors.size() == nOISInstruments, "Invalid Market Data: Number of OIS Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nOISInstruments; ++i )
		{
			// i. Perturb Market Data
			oisCurveData_->perturb( OIS_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateOISCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("OIS_" + perturbedOISTenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			oisCurveData_->restoreDataAndClearPerturbation();
		}
		
		// 4. Perturb Libor-OIS Swaps
		// ==========================
		const size_t nLiborOISInstruments = oisCurveData_->loBasisRates_.size();
		const StandardStringVector perturbedLiborOISTenors = oisCurveData_->instrumentTenors( LIBOR_OIS_BASISSPREAD_MARKETDATA );
		MLIB_REQUIRE( perturbedLiborOISTenors.size() == nLiborOISInstruments, "Invalid Market Data: Number of Libor-OIS Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nLiborOISInstruments; ++i )
		{
			// i. Perturb Market Data
			oisCurveData_->perturb( LIBOR_OIS_BASISSPREAD_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateOISCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("LIBOROIS_" + perturbedLiborOISTenors[i] );
			perturbedInstrumentIsOutright.push_back( false );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			oisCurveData_->restoreDataAndClearPerturbation();
		}

		// 5. Perturb Swaps
		// ==========================
		const size_t nSwapInstruments = oisCurveData_->swapRates_.size();
		const StandardStringVector perturbedSwapTenors = oisCurveData_->instrumentTenors( SWAP_MARKETDATA );
		MLIB_REQUIRE( perturbedSwapTenors.size() == nSwapInstruments, "Invalid Market Data: Number of Swap Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nSwapInstruments; ++i )
		{
			// i. Perturb Market Data
			oisCurveData_->perturb( SWAP_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateOISCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("SWAP_" + perturbedSwapTenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			oisCurveData_->restoreDataAndClearPerturbation();
		}

        // 7. Restore and Recalibrate Original Curve
        oisCurveData_->restoreDataAndClearPerturbation();
		calibrateOISCurve();
        
        // 8. Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( perturbedMarketDataShiftSizes, perturbedInstrumentList, perturbedInstrumentIsOutright, originalDFObject, perturbedDFObjects ) );
        
		return curveJacobianData;
    }

    // Method to calibrate the Swap curve and calculate the Perturbed Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateSwapCurveAndCalculatePerturbedJacobian( const double gradientShiftSize ) const
    {
		// 1. Calibrate Original Discount Factors
		calibrateSwapCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();

		// 2. Create Vector of Perturbed Instruments Discount Factor Objects
		StandardStringVector perturbedInstrumentList;
		std::vector<bool> perturbedInstrumentIsOutright;
		DoubleVector perturbedMarketDataShiftSizes;
		std::vector<std::shared_ptr<DiscountFactorResults> > perturbedDFObjects;

		// 3. Perturb LIBOR_FIXINGS
		// ==========================
		const size_t nLiborFixings = swapCurveData_->liborRates_.size();
		const StandardStringVector perturbedLiborFixingTenors = swapCurveData_->instrumentTenors( LIBOR_FIXING_TABLE );
		MLIB_REQUIRE( perturbedLiborFixingTenors.size() == nLiborFixings, "Invalid Market Data: Number of Libor Fixing Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nLiborFixings; ++i )
		{
			// i. Perturb Market Data
			swapCurveData_->perturb( LIBOR_FIXING_TABLE, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateSwapCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("LIBORFIXING_" + perturbedLiborFixingTenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			swapCurveData_->restoreDataAndClearPerturbation();
		}
		
		// 4. Perturb FUTURES
		// ==========================
		const size_t nFuturesInstruments = swapCurveData_->futureRates_.size();
		const StandardStringVector perturbedFuturesTenors = swapCurveData_->instrumentTenors( FUTURES_MARKETDATA );
		MLIB_REQUIRE( perturbedFuturesTenors.size() == nFuturesInstruments, "Invalid Market Data: Number of Futures Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nFuturesInstruments; ++i )
		{
			// i. Perturb Market Data
			swapCurveData_->perturb( FUTURES_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateSwapCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back( perturbedFuturesTenors[i] ); // No need to write "Future" in front of tenor since already present in market data block
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			swapCurveData_->restoreDataAndClearPerturbation();
		}

		// 5. Perturb FRA_MARKETDATA
		// ==========================

		// We can have either 3M or 6M FRAs, but not both, use whichever is bigger
		const size_t nFRA3MInstruments = swapCurveData_->fra3mRates_.size();
		const size_t nFRA6MInstruments = swapCurveData_->fra6mRates_.size();
		const size_t nFRAInstruments = nFRA3MInstruments > nFRA6MInstruments ? nFRA3MInstruments : nFRA6MInstruments;
		const StandardStringVector perturbedFRATenors = swapCurveData_->instrumentTenors( FRA_MARKETDATA );
		MLIB_REQUIRE( perturbedFRATenors.size() == nFRAInstruments, "Invalid Market Data: Number of FRA Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nFRAInstruments; ++i )
		{
			// i. Perturb Market Data
			swapCurveData_->perturb( FRA_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateSwapCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("FRA_" + perturbedFRATenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			swapCurveData_->restoreDataAndClearPerturbation();
		}

		// 6. Perturb SWAP_MARKETDATA
		// ==========================
		const size_t nSwapInstruments = swapCurveData_->swapRates_.size();
		const StandardStringVector perturbedSwapTenors = swapCurveData_->instrumentTenors( SWAP_MARKETDATA );
		MLIB_REQUIRE( perturbedSwapTenors.size() == nSwapInstruments, "Invalid Market Data: Number of Swap Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nSwapInstruments; ++i )
		{
			// i. Perturb Market Data
			swapCurveData_->perturb( SWAP_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateSwapCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("SWAP_" + perturbedSwapTenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			swapCurveData_->restoreDataAndClearPerturbation();
		}

		// 7. Perturb BASIS_SWAP_MARKETDATA
		// ==========================
		const size_t nBasisInstruments = swapCurveData_->basisAdjRates_.size();
		const StandardStringVector perturbedBasisTenors = swapCurveData_->instrumentTenors( BASIS_SWAP_MARKETDATA );
		MLIB_REQUIRE( perturbedBasisTenors.size() == nBasisInstruments, "Invalid Market Data: Number of Basis Swap Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nBasisInstruments; ++i )
		{
			// i. Perturb Market Data
			swapCurveData_->perturb( BASIS_SWAP_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateSwapCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("BASIS_" + perturbedBasisTenors[i] );
			perturbedInstrumentIsOutright.push_back( false );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			swapCurveData_->restoreDataAndClearPerturbation();
		}

        // 8. Restore and Recalibrate Original Curve
        swapCurveData_->restoreDataAndClearPerturbation();
		calibrateSwapCurve();
        
        // 9. Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( perturbedMarketDataShiftSizes, perturbedInstrumentList, perturbedInstrumentIsOutright, originalDFObject, perturbedDFObjects ) );
        
		return curveJacobianData;
    }
    
    // Method to calibrate the Tenor Basis curve and calculate the Perturbed Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateTenorBasisCurveAndCalculatePerturbedJacobian( const double gradientShiftSize ) const
    {
		// 1. Calibrate Original Discount Factors
		calibrateTenorBasisCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();

		// 2. Create Vector of Perturbed Instruments Discount Factor Objects
		StandardStringVector perturbedInstrumentList;
		std::vector<bool> perturbedInstrumentIsOutright;
		DoubleVector perturbedMarketDataShiftSizes;
		std::vector<std::shared_ptr<DiscountFactorResults> > perturbedDFObjects;

		// 3. Perturb LIBOR_FIXING_TABLE
		// ==========================
		const size_t nLiborFixings = tenorBasisCurveData_->liborRates_.size();
		const StandardStringVector perturbedLiborFixingTenors = tenorBasisCurveData_->instrumentTenors( LIBOR_FIXING_TABLE );
		MLIB_REQUIRE( perturbedLiborFixingTenors.size() == nLiborFixings, "Invalid Market Data: Number of Libor Fixing Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nLiborFixings; ++i )
		{
			// i. Perturb Market Data
			tenorBasisCurveData_->perturb( LIBOR_FIXING_TABLE, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateTenorBasisCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("LIBORFIXING_" + perturbedLiborFixingTenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			tenorBasisCurveData_->restoreDataAndClearPerturbation();
		}
		
		// 4. Perturb FRA_MARKETDATA
		// ==========================

		const size_t nFRAInstruments = tenorBasisCurveData_->fraRates_.size();
		const StandardStringVector perturbedFRATenors = tenorBasisCurveData_->instrumentTenors( FRA_MARKETDATA );
		MLIB_REQUIRE( perturbedFRATenors.size() == nFRAInstruments, "Invalid Market Data: Number of FRA Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nFRAInstruments; ++i )
		{
			// i. Perturb Market Data
			tenorBasisCurveData_->perturb( FRA_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateTenorBasisCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("FRA_" + perturbedFRATenors[i] );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			tenorBasisCurveData_->restoreDataAndClearPerturbation();
		}

		// 5. Perturb BASIS_SWAP_MARKETDATA
		// ==========================
		const size_t nBasisInstruments = tenorBasisCurveData_->basisRates_.size();
		const StandardStringVector perturbedBasisTenors = tenorBasisCurveData_->instrumentTenors( BASIS_SWAP_MARKETDATA );
		MLIB_REQUIRE( perturbedBasisTenors.size() == nBasisInstruments, "Invalid Market Data: Number of Basis Swap Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nBasisInstruments; ++i )
		{
			// i. Perturb Market Data
			tenorBasisCurveData_->perturb( BASIS_SWAP_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateTenorBasisCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("BASIS_" + perturbedBasisTenors[i] );
			perturbedInstrumentIsOutright.push_back( false );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			tenorBasisCurveData_->restoreDataAndClearPerturbation();
		}

        // 6. Restore and Recalibrate Original Curve
        tenorBasisCurveData_->restoreDataAndClearPerturbation();
		calibrateTenorBasisCurve();
        
        // 7. Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( perturbedMarketDataShiftSizes, perturbedInstrumentList, perturbedInstrumentIsOutright, originalDFObject, perturbedDFObjects ) );
        
		return curveJacobianData;
    }

    // Method to calibrate the Xccy Basis curve and calculate the Perturbed Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateXccyBasisCurveAndCalculatePerturbedJacobian( const double gradientShiftSize ) const
    {
		// 1. Calibrate Original Discount Factors
		calibrateXccyBasisCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();

		// 2. Create Vector of Perturbed Instruments Discount Factor Objects
		StandardStringVector perturbedInstrumentList;
		std::vector<bool> perturbedInstrumentIsOutright;
		DoubleVector perturbedMarketDataShiftSizes;
		std::vector<std::shared_ptr<DiscountFactorResults> > perturbedDFObjects;

		// 3. Check if we are using FX Forwards as calibration instruments get the FX PipSize and FX Shift Size
		// FXForwards are used if Curve Market Data CURVEPROPERTIES data block 'isFwdFX' parameter is set to true
		// The FX PipSize stored in the Curve Market Data FXFWDS data block as the 'denomintor' parameter, where 10,000 indicates a PipSize of 1/10000 = 0.0001
		
		const size_t nFXSpotInstruments = xccyBasisCurveData_->spotFxRates_.size();
		MLIB_REQUIRE( nFXSpotInstruments == 1, "Invalid Xccy Curve Market Data: Exactly one FX Spot Rate is required" )
		const bool useFXForwardInstruments = xccyBasisCurveData_->curveConvLVB_.getOptionalValueAsBool( CURVEGENERATOR_CURVEPROPERTIES_KEY::IS_FWD_FX, false );

		// FX Parameter PlaceHolders
		double pipSize = 1.0;
		double fxSpotRate = 1.0;
		double fxSpotShiftSize = 1.0;
		double fxForwardPointsShiftSize = 1.0;

		if( useFXForwardInstruments )
		{
			const double pipSizeDenominator = xccyBasisCurveData_->fxFwdConvLVB_.getCompulsoryValueAsDouble( CURVEGENERATOR_FXFWDS_KEY::DENOMINATOR );
			
			const size_t fxSpotRateColumn = 2; // i.e. Column 3 of the FX Spot Market Data Block
			fxSpotRate = xccyBasisCurveData_->spotFxRates_[0][fxSpotRateColumn].getDoubleValue();
			
			pipSize = 1.0 / pipSizeDenominator;
			fxSpotShiftSize = fxSpotRate * gradientShiftSize; // gradientShiftSize typically 1 bps
			fxForwardPointsShiftSize = fxSpotShiftSize / pipSize;
		}
			   
		// 4. Perturb FXSPOT_MARKETDATA
		// ==========================
		for( size_t i = 0; i < nFXSpotInstruments; ++i )
		{
			// i. Perturb Market Data
			xccyBasisCurveData_->perturb( FXSPOT_MARKETDATA, i, fxSpotShiftSize ); // fxSpotShiftSize = FXSpotRate x gradientShiftSize
			
			// ii. Calibrate and Extract DF Results Object
			calibrateXccyBasisCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();
		
			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("FXSPOT" );
			perturbedInstrumentIsOutright.push_back( true );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			xccyBasisCurveData_->restoreDataAndClearPerturbation();
		}

		// 5. Perturb FXFWD_MARKETDATA
		// ==========================
		if( useFXForwardInstruments )
		{
			const size_t nFXForwards = xccyBasisCurveData_->fxFwdRates_.size();
			MLIB_REQUIRE( nFXForwards > 0, "Invalid Xccy Curve Market Data: Missing FXForward market data, which is required when parameter 'isFWDFX' is set to True" )
		
			const StandardStringVector perturbedFXForwardTenors = xccyBasisCurveData_->instrumentTenors( FXFWD_MARKETDATA );
			MLIB_REQUIRE( perturbedFXForwardTenors.size() == nFXForwards, "Invalid Market Data: Number of FX Forwards Instruments must match number of Instrument Tenors" )

			for( size_t i = 0; i < nFXForwards; ++i )
			{
				// i. Perturb Market Data
				xccyBasisCurveData_->perturb( FXFWD_MARKETDATA, i, fxForwardPointsShiftSize ); // fxForwardPointsShiftSize = FXSpotRate x gradientShiftSize / pipSize
			
				// ii. Calibrate and Extract DF Results Object
				calibrateXccyBasisCurve();
				std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();
		
				// iii. Update Pertubation Parameters & Discount Factors
				perturbedInstrumentList.push_back("FXFORWARD_" + perturbedFXForwardTenors[i] );
				perturbedInstrumentIsOutright.push_back( true );
				perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
				perturbedDFObjects.push_back( purturbedDFObject );
					
				// iv. Reset Market Data
				xccyBasisCurveData_->restoreDataAndClearPerturbation();
			}
		}

		// 6. Perturb XCCY_SWAP_MARKETDATA
		// ==========================
		const size_t nXccyBasisInstruments = xccyBasisCurveData_->basisRates_.size();
		const StandardStringVector perturbedXccyBasisTenors = xccyBasisCurveData_->instrumentTenors( XCCY_SWAP_MARKETDATA );
		MLIB_REQUIRE( perturbedXccyBasisTenors.size() == nXccyBasisInstruments, "Invalid Market Data: Number of Xccy Basis Instruments must match number of Instrument Tenors" )

		for( size_t i = 0; i < nXccyBasisInstruments; ++i )
		{
			// i. Perturb Market Data
			xccyBasisCurveData_->perturb( XCCY_SWAP_MARKETDATA, i, gradientShiftSize );
			
			// ii. Calibrate and Extract DF Results Object
			calibrateXccyBasisCurve();
			std::shared_ptr<DiscountFactorResults> purturbedDFObject = createDiscountFactorResultsObject();

			// iii. Update Pertubation Parameters & Discount Factors
			perturbedInstrumentList.push_back("XCCY_" + perturbedXccyBasisTenors[i] );
			perturbedInstrumentIsOutright.push_back( false );
			perturbedMarketDataShiftSizes.push_back( gradientShiftSize );
			perturbedDFObjects.push_back( purturbedDFObject );
					
			// iv. Reset Market Data
			xccyBasisCurveData_->restoreDataAndClearPerturbation();
		}

        // 7. Restore and Recalibrate Original Curve
        xccyBasisCurveData_->restoreDataAndClearPerturbation();
		calibrateXccyBasisCurve();
        
        // 8. Create and Return Jacobian Data
        std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( perturbedMarketDataShiftSizes, perturbedInstrumentList, perturbedInstrumentIsOutright, originalDFObject, perturbedDFObjects ) );
        
		return curveJacobianData;
    }

    // Method to calibrate the FX Forward Constant curve and calculate the Perturbed Jacobian
    std::shared_ptr<JacobianData> CurveEngine::calibrateFxForwardConstantCurveAndCalculatePerturbedJacobian( const double gradientShiftSize ) const
    {
		// Nothing to do here - This is a placeholder to create the null jacobian for this curve type

		// 1. Get the Original Discount Factor Results Object
        calibrateFwdFXConstantCurve();
        std::shared_ptr<DiscountFactorResults> originalDFObject = createDiscountFactorResultsObject();
        
        // 2.  Create and Return Jacobian Data
		StandardStringVector perturbedInstrumentList = { "FXFwdConstant" };
		std::vector<bool> perturbedInstrumentIsOutright = { true };
		DoubleVector perturbedMarketDataShiftSizes = { gradientShiftSize };
		std::vector<std::shared_ptr<DiscountFactorResults> > perturbedDFObjects( perturbedInstrumentList.size(), originalDFObject );
		std::shared_ptr<JacobianData> curveJacobianData = std::shared_ptr<JacobianData>( new JacobianData( perturbedMarketDataShiftSizes, perturbedInstrumentList, perturbedInstrumentIsOutright, originalDFObject, perturbedDFObjects ) );
        
		return curveJacobianData;
    }

	// ====================
	
	// Method to Update Curve Results and the Jacobian if Present
	void CurveEngine::updateCurveResults()
	{
		// 1. Curve Results: Discount Factors
        std::shared_ptr<DiscountFactorResults> discountFactorResults = createDiscountFactorResultsObject();
                
        // 2. Get and Keep the Jacobian Results if they Exist
        std::shared_ptr<etrading::JacobianResults> jacobianResultsByDiscountFactor	= getJacobianResultsByDiscountFactor();
        std::shared_ptr<etrading::JacobianResults> jacobianResultsByForwardRate		= getJacobianResultsByForwardRate();
		std::shared_ptr<etrading::JacobianResults> jacobianResultsByCompoundRate	= getJacobianResultsByCompoundRate();

        // 3. Update Curve Results Object
		if( jacobianResultsByDiscountFactor == nullptr )
		{
			curveResults_ = std::shared_ptr<CurveResults>( new CurveResults( curveDescription_,
																			 curveConventionsAndMarketData_,
																			 discountFactorResults ) );
		}
		else
		{
			curveResults_ = std::shared_ptr<CurveResults>( new CurveResults( curveDescription_,
																			 curveConventionsAndMarketData_,
																			 discountFactorResults,
																			 jacobianResultsByDiscountFactor,
																			 jacobianResultsByForwardRate,
																			 jacobianResultsByCompoundRate ) );
		}
		return;
	}

	// Method to Update Curve Results and the Jacobian if Present
	void CurveEngine::updateCurveResultsAndFlatShiftJacobian( const double & gradientShiftSize, const double & marketDataShiftSize, const std::shared_ptr<JacobianData> & jacobianData )
	{
		// 1. Curve Results: Discount Factors + Jacobian
		std::shared_ptr<DiscountFactorResults> discountFactorResults		= createDiscountFactorResultsObject();
        std::shared_ptr<JacobianResults> jacobianResultsByDiscountFactor    = createFlatShiftJacobianResults( jacobianData, discountFactorResults, DISCOUNT_FACTOR_RISK_TYPE, marketDataShiftSize );
		std::shared_ptr<JacobianResults> jacobianResultsByForwardRate		= createFlatShiftJacobianResults( jacobianData, discountFactorResults, FORWARD_RATE_RISK_TYPE, marketDataShiftSize );
		std::shared_ptr<JacobianResults> jacobianResultsByCompoundRate		= nullptr; // TODO: Complete Me!!!

        // 2. Update Curve Results Object
        curveResults_ = std::shared_ptr<CurveResults>( new CurveResults( curveDescription_,
																	     curveConventionsAndMarketData_,
																	     discountFactorResults,
																	     jacobianResultsByDiscountFactor,
																	     jacobianResultsByForwardRate,
																	     jacobianResultsByCompoundRate ) );
		return;
	}
	

	// Method to Update Curve Results and the Jacobian if Present
	void CurveEngine::updateCurveResultsAndPerturbedJacobian( const double & gradientShiftSize, const double & marketDataShiftSize, const std::shared_ptr<JacobianData> & jacobianData )
	{
		// 1. Curve Results: Discount Factors + Jacobian
		std::shared_ptr<DiscountFactorResults> discountFactorResults		= createDiscountFactorResultsObject();
        std::shared_ptr<JacobianResults> jacobianResultsByDiscountFactor    = createPerturbedJacobianResults( jacobianData, discountFactorResults, DISCOUNT_FACTOR_RISK_TYPE, marketDataShiftSize );
		std::shared_ptr<JacobianResults> jacobianResultsByForwardRate		= createPerturbedJacobianResults( jacobianData, discountFactorResults, FORWARD_RATE_RISK_TYPE, marketDataShiftSize );
		std::shared_ptr<JacobianResults> jacobianResultsByCompoundRate		= nullptr; // TODO: Complete Me!!!

        // 2. Update Curve Results Object
        curveResults_ = std::shared_ptr<CurveResults>( new CurveResults( curveDescription_,
																	     curveConventionsAndMarketData_,
																	     discountFactorResults,
																	     jacobianResultsByDiscountFactor,
																	     jacobianResultsByForwardRate,
																	     jacobianResultsByCompoundRate ) );
		return;
	}


	// Method to Calibrate an ARR Curve ( Alternative Reference Rate aka Risk-Free Rate )
    void CurveEngine::calibrateARRCurve() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( arrCurveData_ != nullptr, "ARR Curve Market Data is Missing" )

		// 1. Curve Static ARR Data
		etrading::LAUpdateStaticDataManager::loadStaticDataOISCurve( getDataInstance(),
																	 curveDescription_->curveCollection().c_str(),
																	 curveDescription_->objectPoolLookupTable().c_str(),
																	 arrCurveData_->curveConvLVB_.toLAStringMatrix(),      // Use LVB for Convention Data Lookups
																	 arrCurveData_->oisRates_,
																	 arrCurveData_->oisConvLVB_.toLAStringMatrix(),        // Use LVB for Convention Data Lookups
																	 curveDescription_->curveIndexList().c_str(),
																	 arrCurveData_->histRates_,
																	 arrCurveData_->loBasisRates_,
																	 arrCurveData_->loBasisConvLVB_.toLAStringMatrix(),    // Use LVB for Convention Data Lookups
																	 arrCurveData_->swapRates_,
																	 arrCurveData_->swapConvLVB_.toLAStringMatrix() );
		// 2. Calibrate ARR Curve
		LAUpdateStaticDataManager::calibrateOISCurve( getDataInstance(),
													  curveDescription_->curveCollection().c_str(),
													  curveDescription_->objectPoolLookupTable().c_str(),
													  arrCurveData_->curveConvLVB_.toLAStringMatrix() );
    }


    // Method to Calibrate an OIS Curve
    void CurveEngine::calibrateOISCurve() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( oisCurveData_ != nullptr, "OIS Curve Market Data is Missing" )
		
        // 1. Curve Static OIS Data
		etrading::LAUpdateStaticDataManager::loadStaticDataOISCurve( getDataInstance(),
																	 curveDescription_->curveCollection().c_str(),
																	 curveDescription_->objectPoolLookupTable().c_str(),
																	 oisCurveData_->curveConvLVB_.toLAStringMatrix(),      // Use LVB for Convention Data Lookups
																	 oisCurveData_->oisRates_,
																	 oisCurveData_->oisConvLVB_.toLAStringMatrix(),        // Use LVB for Convention Data Lookups
																	 curveDescription_->curveIndexList().c_str(),
																	 oisCurveData_->histRates_,
																	 oisCurveData_->loBasisRates_,
																	 oisCurveData_->loBasisConvLVB_.toLAStringMatrix(),    // Use LVB for Convention Data Lookups
																	 oisCurveData_->swapRates_,
																	 oisCurveData_->swapConvLVB_.toLAStringMatrix() );
		// 2. Calibrate OIS Curve
		LAUpdateStaticDataManager::calibrateOISCurve( getDataInstance(),
													  curveDescription_->curveCollection().c_str(),
													  curveDescription_->objectPoolLookupTable().c_str(),
													  oisCurveData_->curveConvLVB_.toLAStringMatrix() );
	}
	

    // Method to Calibrate a Swap Curve
    void CurveEngine::calibrateSwapCurve() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( swapCurveData_ != nullptr, "Swap Curve Market Data is Missing" )

        // 1. Load Curve Static Data
        LAUpdateStaticDataManager::loadStaticDataSwapCurve( etrading::getDataInstance(),
															curveDescription_->curveCollection().c_str(),
															curveDescription_->objectPoolLookupTable().c_str(),
															swapCurveData_->curveConvLVB_.toLAStringMatrix(),        // Use LVB for Convention Data Lookups
															swapCurveData_->moneyMarketConvLVB_.toLAStringMatrix(),  // Use LVB for Convention Data Lookups
															swapCurveData_->liborRates_,
															swapCurveData_->liborConvLVB_.toLAStringMatrix(),        // Use LVB for Convention Data Lookups
															swapCurveData_->swapRates_,
															swapCurveData_->swapConvLVB_.toLAStringMatrix(),         // Use LVB for Convention Data Lookups
															swapCurveData_->fra3mRates_,
															swapCurveData_->fra6mRates_,
															swapCurveData_->fraConvLVB_.toLAStringMatrix(),          // Use LVB for Convention Data Lookups
															swapCurveData_->futureRates_,
															swapCurveData_->futureConvLVB_.toLAStringMatrix(),       // Use LVB for Convention Data Lookups
															swapCurveData_->basisAdjConvLVB_.toLAStringMatrix(),     // Use LVB for Convention Data Lookups
															swapCurveData_->basisAdjRates_,
															curveDescription_->curveIndexList().c_str(),
															"" ); // curveName_DF2
		// 2. Calibrate Curve
		LAUpdateStaticDataManager::calibrateSwapCurve( etrading::getDataInstance(),
													   curveDescription_->curveCollection().c_str(),
													   curveDescription_->objectPoolLookupTable().c_str(),
													   swapCurveData_->curveConvLVB_.toLAStringMatrix() );
	}

    // Method to Calibrate a Tenor Basis Curve
    void CurveEngine::calibrateTenorBasisCurve() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( tenorBasisCurveData_ != nullptr, "Tenor Basis Curve Market Data is Missing" )

        // 1. Load Curve Static Data
        LAUpdateStaticDataManager::loadStaticDataBasisCurve( etrading::getDataInstance(),
															 curveDescription_->curveCollection().c_str(),
															 curveDescription_->objectPoolLookupTable().c_str(),
															 tenorBasisCurveData_->basisRates_,
															 tenorBasisCurveData_->basisConvLVB_.toLAStringMatrix(),         // Use LVB for Convention Data Lookups
															 tenorBasisCurveData_->fxFwdRates_,
															 tenorBasisCurveData_->fxFwdConvLVB_.toLAStringMatrix(),         // Use LVB for Convention Data Lookups
															 tenorBasisCurveData_->spotFxRates_,
															 tenorBasisCurveData_->curveConvLVB_.toLAStringMatrix(),         // Use LVB for Convention Data Lookups
															 tenorBasisCurveData_->moneyMarketConvLVB_.toLAStringMatrix(),   // Use LVB for Convention Data Lookups
															 curveDescription_->curveIndexList().c_str(),
															 tenorBasisCurveData_->fraConvLVB_.toLAStringMatrix(),           // Use LVB for Convention Data Lookups
															 tenorBasisCurveData_->fraRates_,
															 tenorBasisCurveData_->liborConvLVB_.toLAStringMatrix(),         // Use LVB for Convention Data Lookups
															 tenorBasisCurveData_->liborRates_ );
		// 2. Calibrate Curve
		LAUpdateStaticDataManager::calibrateBasisCurve( etrading::getDataInstance(),
														curveDescription_->curveCollection().c_str(),
														curveDescription_->objectPoolLookupTable().c_str(),
														tenorBasisCurveData_->curveConvLVB_.toLAStringMatrix() );
    }

    // Method to Calibrate a Xccy Basis Curve to calculate USD CSA discount factors
    void CurveEngine::calibrateXccyBasisCurve() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( xccyBasisCurveData_ != nullptr, "Xccy Basis Curve Market Data is Missing" )

        // 1. Load Curve Static Data
        LAUpdateStaticDataManager::loadStaticDataBasisCurve( etrading::getDataInstance(),
															 curveDescription_->curveCollection().c_str(), // domestic curve
															 curveDescription_->objectPoolLookupTable().c_str(),
															 xccyBasisCurveData_->basisRates_,
															 xccyBasisCurveData_->basisConvLVB_.toLAStringMatrix(),          // Use LVB for Convention Data Lookups
															 xccyBasisCurveData_->fxFwdRates_,
															 xccyBasisCurveData_->fxFwdConvLVB_.toLAStringMatrix(),          // Use LVB for Convention Data Lookups
															 xccyBasisCurveData_->spotFxRates_,
															 xccyBasisCurveData_->curveConvLVB_.toLAStringMatrix(),          // Use LVB for Convention Data Lookups
															 xccyBasisCurveData_->moneyMarketConvLVB_.toLAStringMatrix(),    // Use LVB for Convention Data Lookups
															 curveDescription_->curveIndexList().c_str(),
															 xccyBasisCurveData_->fraConvLVB_.toLAStringMatrix(),            // Use LVB for Convention Data Lookups
															 xccyBasisCurveData_->fraRates_ );
		// 2. Calibrate Curve
        LAUpdateStaticDataManager::calibrateBasisCurve( etrading::getDataInstance(),
													    curveDescription_->curveCollection().c_str(), // domestic curve
													    curveDescription_->objectPoolLookupTable().c_str(),
													    xccyBasisCurveData_->curveConvLVB_.toLAStringMatrix() );
	}

    // Method to Calibrate a Forward FX Constant Curve to calculate Non-USD CSA discount factors
    void CurveEngine::calibrateFwdFXConstantCurve() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
		MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )
        MLIB_REQUIRE( fxFwdConstantCurveData_ != nullptr, "FX Forward Constant Curve Market Data is Missing" )

		// 1. Load Curve Static Data
		LAUpdateStaticDataManager::loadStaticDataFwdFXConstantCurve( etrading::getDataInstance(),
														             curveDescription_->curveCollection().c_str(), // domestic curve
														             curveDescription_->objectPoolLookupTable().c_str(),
														             fxFwdConstantCurveData_->fwdfxconstConvLVB_.toLAStringMatrix(),     // Use LVB for Convention Data Lookups
														             fxFwdConstantCurveData_->curveConvLVB_.toLAStringMatrix(),          // Use LVB for Convention Data Lookups
														             curveDescription_->curveIndexList().c_str() );
		// 2. Calibrate Curve
		LAUpdateStaticDataManager::calibrateFwdFXConstantCurve( etrading::getDataInstance(),
														        curveDescription_->curveCollection().c_str(), // domestic curve
														        curveDescription_->objectPoolLookupTable().c_str(),
														        fxFwdConstantCurveData_->curveConvLVB_.toLAStringMatrix() );         // Use LVB for Convention Data Lookups
    }


    // Method to Store the Curve Results for the Curve Index and any alias Indices in the CurveResultsContainer object
    void CurveEngine::storeCurveResults() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )

        // Persist curveResults in Singleton Curve Results Container for the Index and any alias Indices
        const LAStringVector curveIndexAliasList = etrading::curveIndexAliasList( curveDescription_->curveCollection().c_str(),
                                                                                  curveDescription_->objectPoolLookupTable().c_str() );
        for( auto curveIndex : curveIndexAliasList )
        {
            CurveResultsContainer::getInstance().addCurveResults( curveDescription_->curveCollection(), curveIndex.c_str(), curveResults_ );
        }
    }
    

    // Method to create Curve Discount Factor Results
    std::shared_ptr<DiscountFactorResults> CurveEngine::createDiscountFactorResultsObject() const
    {
        // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
        MLIB_REQUIRE( curveDescription_ != nullptr, "Curve Description Static Data is Missing" )

        // Curve Description Parameters
        const CurveTypeEnum curveTypeEnum                       = curveDescription_->curveTypeEnum();
        const CurveTenorEnum curveTenorEnum                     = curveDescription_->curveTenorEnum();
        const std::string curveCollection                       = curveDescription_->curveCollection();
        const std::string objectPoolLookupTable                 = curveDescription_->objectPoolLookupTable();

        // Fixing Information needed to Imply Forwards
        std::string fixingCalendar;
        std::string fixingBusDayAdj;
        BusinessDayAdjustmentEnum fixingBusDayAdjEnum;
		StandardStringMatrix forwardAdjustments;

        // Get Fixing Information from the correct Curve Type
        switch ( curveTypeEnum )
        {
            case OIS_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( oisCurveData_ != nullptr, "OIS Curve Market Data is Missing" )

                fixingCalendar                                      = oisCurveData_->oisConvLVB_.getOptionalValueAsString(CURVEGENERATOR_OIS_KEY::CALENDAR, "NO_CHANGE");
                fixingBusDayAdj                                     = oisCurveData_->oisConvLVB_.getOptionalValueAsString(CURVEGENERATOR_OIS_KEY::SLIDING_RULE, "NO_CHANGE");
                fixingBusDayAdjEnum									= toBusinessDayAdjustmentEnum(fixingBusDayAdj);
				forwardAdjustments									= oisCurveData_->forwardAdjustments_;
                break;
            }
            case ARR_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( arrCurveData_ != nullptr, "ARR Curve Market Data is Missing" )

                fixingCalendar                                      = arrCurveData_->oisConvLVB_.getOptionalValueAsString(CURVEGENERATOR_OIS_KEY::CALENDAR, "NO_CHANGE");
                fixingBusDayAdj                                     = arrCurveData_->oisConvLVB_.getOptionalValueAsString(CURVEGENERATOR_OIS_KEY::SLIDING_RULE, "NO_CHANGE");
                fixingBusDayAdjEnum									= toBusinessDayAdjustmentEnum(fixingBusDayAdj);
				forwardAdjustments									= arrCurveData_->forwardAdjustments_;
                break;
            }
            case SWAP_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( swapCurveData_ != nullptr, "Swap Curve Market Data is Missing" )

                fixingCalendar                                      = swapCurveData_->swapConvLVB_.getOptionalValueAsString( CURVEGENERATOR_SWAPS_KEY::CALENDAR, "NO_CHANGE" );
                fixingBusDayAdj                                     = swapCurveData_->swapConvLVB_.getOptionalValueAsString( CURVEGENERATOR_SWAPS_KEY::SLIDING_RULE, "NO_CHANGE" );
                fixingBusDayAdjEnum									= toBusinessDayAdjustmentEnum(fixingBusDayAdj);
				forwardAdjustments									= swapCurveData_->forwardAdjustments_;
                break;
            }
            case TENORBASIS_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( tenorBasisCurveData_ != nullptr, "Tenor Basis Curve Market Data is Missing" )

                const bool isTargetLeg1 = isBasisTargetLeg1( curveTypeEnum, tenorBasisCurveData_->basisConvLVB_ );
                if ( isTargetLeg1 )
                {
                    // Use Leg1
                    fixingCalendar          = tenorBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_CALENDAR, "Tenor Basis Conventions" );    
                    fixingBusDayAdj         = tenorBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_SLIDING_RULE, "Tenor Basis Conventions" );
                    fixingBusDayAdjEnum     = toBusinessDayAdjustmentEnum( fixingBusDayAdj );
					forwardAdjustments		= tenorBasisCurveData_->forwardAdjustments_;
                }
                else
                {
                    // Use Leg2
                    fixingCalendar          = tenorBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_CALENDAR, "Tenor Basis Conventions" );
                    fixingBusDayAdj         = tenorBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SLIDING_RULE, "Tenor Basis Conventions" );
                    fixingBusDayAdjEnum     = toBusinessDayAdjustmentEnum( fixingBusDayAdj );
					forwardAdjustments		= tenorBasisCurveData_->forwardAdjustments_;
                }
                break;
            }
            case XCCYBASIS_CURVETYPE:
            {
                // Note getting private shared pointer member data has no nullptr check, so we check here to prevent structured execptions
                MLIB_REQUIRE( xccyBasisCurveData_ != nullptr, "Xccy Basis Curve Market Data is Missing" )

                const bool isTargetLeg1 = isBasisTargetLeg1( curveTypeEnum, xccyBasisCurveData_->basisConvLVB_ );
                if ( isTargetLeg1 )
                {
                    // Use Leg1
                    fixingCalendar          = xccyBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_CALENDAR, "Xccy Basis Conventions" );    
                    fixingBusDayAdj         = xccyBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG1_CASHLET_SLIDING_RULE, "Xccy Basis Conventions" );
                    fixingBusDayAdjEnum     = toBusinessDayAdjustmentEnum( fixingBusDayAdj );
					forwardAdjustments		= xccyBasisCurveData_->forwardAdjustments_;
                }
                else
                {
                    // Use Leg2
                    fixingCalendar          = xccyBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_CALENDAR, "Xccy Basis Conventions" );
                    fixingBusDayAdj         = xccyBasisCurveData_->basisConvLVB_.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::LEG2_CASHLET_SLIDING_RULE, "Xccy Basis Conventions" );
                    fixingBusDayAdjEnum     = toBusinessDayAdjustmentEnum( fixingBusDayAdj );
					forwardAdjustments		= xccyBasisCurveData_->forwardAdjustments_;
                }
                break;
            }
            case FWDFXCONST_CURVETYPE:
            {
                // TODO:    Fixing Information needed to Imply Forwards
                // It is not clear what conventions to use for FX Forward Curves nor how to get the fixing information . It appears we have to
                // get that information from the target basis curve. Use UNADJUSTED or NO_CHANGE as the business day adjustment for now ...
                fixingCalendar			= "";
                fixingBusDayAdj			= "";
                fixingBusDayAdjEnum		= NONE_BUSINESS_DAY_ADJ;
				forwardAdjustments		= fxFwdConstantCurveData_->forwardAdjustments_;
                break;
            }
            default:
            {
                MLIB_THROW("Invalid Curve Type '" + toString( curveTypeEnum ) + "'. Unable to Create Discount Factors Results.")
                break;
            }
        }
		
        // Get the Discount Factor Results from the Object Pool
        std::shared_ptr<DiscountFactorResults> discountFactorResults  = getDiscountFactorResultsFromObjectPool( curveTypeEnum, curveTenorEnum, curveCollection, objectPoolLookupTable, fixingBusDayAdjEnum, fixingCalendar, forwardAdjustments );

		return discountFactorResults;
    }

    // Method to create a Flat-Shift Jacobian Results Object given the Jacobian Data
    std::shared_ptr<JacobianResults> CurveEngine::createFlatShiftJacobianResults( const std::shared_ptr<JacobianData> & jacobianData,
                                                                                  const std::shared_ptr<DiscountFactorResults> & discountFactorResults,
                                                                                  const RiskTypeEnum & riskType,
																			      const double gradientShiftSize,
                                                                                  const double marketDataShiftSizeForFlatShift ) const
    {
        MLIB_REQUIRE( jacobianData != nullptr,			"Jacobian Data is Missing" )
        MLIB_REQUIRE( discountFactorResults != nullptr, "Discount Factor Results Data is Missing" )
        
        // Flat-Shift Risk Parameters
        const LADate asOfDate                       = discountFactorResults->asOfDate();
        const DateVector riskDateVector             = jacobianData->paymentDates();
		
		DoubleVector flatShiftJacobian;
		switch( riskType )
		{
			case DISCOUNT_FACTOR_RISK_TYPE:
			{
				flatShiftJacobian = jacobianData->flatShiftJacobianByDiscountFactor();
				break;
			}
			case FORWARD_RATE_RISK_TYPE:
			{
				flatShiftJacobian = jacobianData->flatShiftJacobianByForwardRate();
				break;
			}
			default:
			{
				MLIB_THROW("Invalid Jacobian Risk Type: Only DiscountFactor and ForwardRate Risk Types are supported")
			}
		}

        // Jacobian Results Constructor for Flat-Shift Jacobian
        std::shared_ptr<JacobianResults> jacobianResults = std::shared_ptr<JacobianResults>( new JacobianResults( asOfDate, 
                                                                                                                  curveDescription(),
                                                                                                                  riskType,
                                                                                                                  gradientShiftSize,                // default gradient shift size 1e-4 
                                                                                                                  riskDateVector,
                                                                                                                  marketDataShiftSizeForFlatShift,  // default market data shift size 1e-4 i.e. 1 basis point
                                                                                                                  flatShiftJacobian,
                                                                                                                  discountFactorResults ) ); 
        return jacobianResults;
    }

	// Method to create a Perturbed Jacobian Results Object given the Jacobian Data
	std::shared_ptr<JacobianResults> CurveEngine::createPerturbedJacobianResults( const std::shared_ptr<JacobianData> & jacobianData,
																				  const std::shared_ptr<DiscountFactorResults> & discountFactorResults,
																				  const RiskTypeEnum & riskType,
																				  const double gradientShiftSize ) const
	{
		MLIB_REQUIRE( jacobianData != nullptr,			"Jacobian Data is Missing" )
        MLIB_REQUIRE( discountFactorResults != nullptr, "Discount Factor Results Data is Missing" )
        
        // Perturbed Risk Parameters
        const LADate asOfDate									= discountFactorResults->asOfDate();
        const DateVector riskDateVector							= jacobianData->paymentDates();
		const StandardStringVector perturbedInstrumentList		= jacobianData->perturbedInstrumentList();
		const std::vector<bool> perturbedInstrumentIsOutright	= jacobianData->perturbedInstrumperturbedInstrumentIsOutright();
		const DoubleVector perturbedMarketDataShiftSizes		= jacobianData->perturbedMarketDataShiftSizes();
		
		DoubleMatrix perturbedJacobian;
		switch( riskType )
		{
			case DISCOUNT_FACTOR_RISK_TYPE:
			{
				perturbedJacobian = jacobianData->perturbedJacobianByDiscountFactor();
				break;
			}
			case FORWARD_RATE_RISK_TYPE:
			{
				perturbedJacobian = jacobianData->perturbedJacobianByForwardRate();
				break;
			}
			default:
			{
				MLIB_THROW("Invalid Jacobian Risk Type: Only DiscountFactor and ForwardRate Risk Types are supported")
			}
		}

        // Jacobian Results Constructor - For Perturbed Jacobian
        std::shared_ptr<JacobianResults> jacobianResults = std::shared_ptr<JacobianResults>( new JacobianResults( asOfDate, 
                                                                                                                  curveDescription(),
                                                                                                                  riskType,
                                                                                                                  gradientShiftSize,                // default gradient shift size 1e-4 
                                                                                                                  riskDateVector,
                                                                                                                  perturbedMarketDataShiftSizes,	// default market data shift size 1e-4 i.e. 1 basis point
                                                                                                                  perturbedInstrumentList,			// instrument names	
																												  perturbedInstrumentIsOutright,	// vector to indicate which risk buckets to use for risk totals i.e. use the outrights and not the spread instruments
																												  perturbedJacobian,				// delta matrix
                                                                                                                  discountFactorResults ) ); 
        return jacobianResults;
	}

	// -------------------------------------------------------------------------------------------------------------

	// Helper Method to Get the Daycount from Curve Data
	DayCountEnum CurveEngine::getDayCountFromCurveData( const CurveTypeEnum & curveType,
                                                        const std::string & curveCollection ) const
	{
		DayCountEnum daycount = NONE_DAYCOUNT;

		switch ( curveType )
		{
			case OIS_CURVETYPE:
			{
				// Get OIS Swap Float Leg Daycount with mullptr check
				std::shared_ptr<OISCurveObjectData> curveData = oisCurveData();
				daycount = toDayCountEnum( curveData->oisConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::DAYCOUNT, CURVE_ENGINE_KEYS::OIS_CURVE_CONVENTIONS ) );
				break;
			}
			case ARR_CURVETYPE:
			{
				// Get ARR Swap Float Leg Daycount with mullptr check
				std::shared_ptr<ARRCurveObjectData> curveData = arrCurveData();
				daycount = toDayCountEnum(curveData->oisConvLVB_.getCompulsoryValueAsString(CURVE_ENGINE_KEYS::DAYCOUNT, CURVE_ENGINE_KEYS::OIS_CURVE_CONVENTIONS));
				break;
			}
			case SWAP_CURVETYPE:
			{
				// Get Swap Float Leg Daycount with mullptr check
				std::shared_ptr<SwapCurveObjectData> curveData = swapCurveData();
				daycount = toDayCountEnum( curveData->swapConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::DAYCOUNT_FLOAT, CURVE_ENGINE_KEYS::SWAP_CURVE_CONVENTIONS  ) );
				break;
			}
			case TENORBASIS_CURVETYPE:
			{
				// Get Swap Float Leg Daycount with mullptr check
				std::shared_ptr<TenorBasisCurveObjectData> curveData = tenorBasisCurveData();

				// *** FORECAST CURVE TARGET ***
				// The Forecast Target for Tenor Basis Swaps is defined as "LEG1FORECAST" or "LEG2FORECAST"
				std::string forecastTargetLeg = curveData->basisConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::TARGET, CURVE_ENGINE_KEYS::TENOR_BASIS_CURVE_CONVENTIONS , true ); // Uppercase = true
				bool isTargetLeg1 = ( forecastTargetLeg == CURVE_ENGINE_KEYS::LEG1_FORECAST_RATES );

				if ( isTargetLeg1 )
				{
					daycount = toDayCountEnum( curveData->basisConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::LEG1_DAYCOUNT, CURVE_ENGINE_KEYS::TENOR_BASIS_CURVE_CONVENTIONS ) );
				}
				else
				{
					daycount = toDayCountEnum( curveData->basisConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::LEG2_DAYCOUNT, CURVE_ENGINE_KEYS::TENOR_BASIS_CURVE_CONVENTIONS ) );
				}

                break;
			}
            case XCCYBASIS_CURVETYPE:
			{
                // Get Xccy Curve Data with nullptr check
				std::shared_ptr<XccyBasisCurveObjectData> curveData = xccyBasisCurveData();
				
				// *** DISCOUNT CURVE TARGET ***
				// The Discount Target for Xccy Basis Swaps is defined as "LEG1DISCOUNT" or "LEG2DISCOUNT"
				std::string discountTargetLeg = curveData->basisConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::TARGET, CURVE_ENGINE_KEYS::XCCY_BASIS_CURVE_CONVENTIONS, true ); // Uppercase = true
				bool isTargetLeg1 = ( discountTargetLeg == CURVE_ENGINE_KEYS::LEG1_DISCOUNT_FACTORS );

				if ( isTargetLeg1 )
				{
					daycount = toDayCountEnum( curveData->basisConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::LEG1_DAYCOUNT, CURVE_ENGINE_KEYS::XCCY_BASIS_CURVE_CONVENTIONS ) );
				}
				else
				{
					daycount = toDayCountEnum( curveData->basisConvLVB_.getCompulsoryValueAsString( CURVE_ENGINE_KEYS::LEG2_DAYCOUNT, CURVE_ENGINE_KEYS::XCCY_BASIS_CURVE_CONVENTIONS ) );
				}

                break;
			}
            case FWDFXCONST_CURVETYPE:
			{
				// FX Forward Curve ... market convention for FX Forward Curves is to match the daycount USD leg of the XCCY Basis
				daycount = ACT_360_DAYCOUNT;
				break;
			}
            default:
			{
				MLIB_THROW("Invalid Curve Type: " + toString(curveType) + ", Supported types include OIS, SWAP, TENORBASIS, XCCY and FWDFXCONST curve types")
                break;
			}
		}

		return daycount;
	}
		
	// Helper Method to get the Daycount from Curve Data as a std::string
	std::string CurveEngine::getDayCountFromCurveDataAsString( const CurveTypeEnum & curveType,
                                                               const std::string & curveCollection ) const
	{
		std::string daycount = toString( getDayCountFromCurveData( curveType, curveCollection ) );
		return daycount;
	}

    // Helper Method to Update Discount Factor Results from Object Pool
    std::shared_ptr<DiscountFactorResults> CurveEngine::getDiscountFactorResultsFromObjectPool( const CurveTypeEnum & curveType,
																								const CurveTenorEnum & curveTenorEnum,
                                                                                                const std::string & curveCollection,
																								const std::string & objectPoolLookupTable,
                                                                                                const BusinessDayAdjustmentEnum & fixingBusDayAdj,
                                                                                                const std::string & fixingCalendar,
																								const StandardStringMatrix & forwardAdjustments ) const
    {
        // Extract results from Object Pool
        const LAObject& entityPoolYieldCurve                    = getDataInstance()->getObjectPool().getObject( curveCollection.c_str(), ENCHKTYPE_ISDEFINED ).get();
        const std::string entityPoolCurveSuffix                 = ( objectPoolLookupTable == STD ) ? "" : "_" + objectPoolLookupTable;
        
        std::string TERMS_ENTITY_POOL_SEARCH_KEY                = OBJECT_POOL_KEYS::TERMS + entityPoolCurveSuffix;                  // CALIBRATION_DATA_TERMS = "Terms"
        std::string DISCOUNT_FACTORS_ENTITY_POOL_SEARCH_KEY     = OBJECT_POOL_KEYS::DISCOUNT_FACTORS + entityPoolCurveSuffix;       // IR_CALIBRATION_DATA_DFS = DiscountFactors"
        
        const VectorDouble paymentDatesInTermsFormat            = dynamic_cast<const LADataDoubles&>(entityPoolYieldCurve.getData(TERMS_ENTITY_POOL_SEARCH_KEY.c_str(), ISDEFINED).get()).get();
        const VectorDouble discountFactors                      = dynamic_cast<const LADataDoubles& >(entityPoolYieldCurve.getData(DISCOUNT_FACTORS_ENTITY_POOL_SEARCH_KEY.c_str(), ISDEFINED).get()).get();
        
        const LADate asOfDate                                   = getCurveAsOfDate( curveCollection.c_str() );
        const InterpolationEnum interpolationEnum               = toInterpolationEnum( getCurveInterpolation( curveCollection.c_str(), objectPoolLookupTable.c_str() ).getCString() );
        
		// IMPORTANT NOTE: We need the daycount to imply forwards rates from discount factors. In the case of Xccy and FXForwardConstant curves we do not need to 
        // do this since forwards are already known. Hence Daycount is not required for Xccy FXForwardConstant curves and we pass a dummy daycount in this case.
		const DayCountEnum daycount		= getDayCountFromCurveData( curveType, curveCollection );

        double joinDateForHybridInterpolation = 0.0;
        if ( interpolationEnum == LINEARSPLINE_INTERPOLATION )
        {
            // Must use objectPoolLookupTable name for the curve index, since
            joinDateForHybridInterpolation = getLinearSplineJoinDateAsDouble( curveCollection.c_str(), objectPoolLookupTable.c_str() );
        }

        const std::shared_ptr<DiscountFactorResults> discountFactorResults( new DiscountFactorResults( curveTenorEnum,
                                                                                                       asOfDate,
																									   curveCollection,
																									   objectPoolLookupTable,
                                                                                                       interpolationEnum,
                                                                                                       paymentDatesInTermsFormat,
                                                                                                       discountFactors,
																									   daycount,
                                                                                                       fixingBusDayAdj,
                                                                                                       fixingCalendar,
                                                                                                       joinDateForHybridInterpolation,
																									   forwardAdjustments ) );
        return discountFactorResults;
    }

    // Helper Method to Identify the Target Leg of a Xccy Basis Swap
    bool CurveEngine::isBasisTargetLeg1( const CurveTypeEnum curveType, const LabelValueBlock & curveConventionLVB ) const
    {
        switch ( curveType )
        {
            case TENORBASIS_CURVETYPE:
            {
                // Tenor Basis: Search for Leg1Forecast Target
                const std::string targetLeg = curveConventionLVB.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::TARGET, "Tenor Basis Conventions" );
                if ( boost::iequals( targetLeg, "LEG1FORECAST" ) )
                {
                    return true;
                }
                else if ( boost::iequals( targetLeg, "LEG2FORECAST" ) )
                {
                    return false;
                }
                MLIB_THROW( "Invalid Tenor Basis Curve Config: Target must me 'Leg1Forecast' or 'Leg2Forecast' rates." )
            }
            case XCCYBASIS_CURVETYPE:
            {
                // Xccy Basis: Search for Leg1Discount
                const std::string targetLeg = curveConventionLVB.getCompulsoryValueAsString( CURVEGENERATOR_BASISSWAPS_KEY::TARGET, "Xccy Basis Conventions" );
                if ( boost::iequals( targetLeg, "LEG1DISCOUNT" ) )
                {
                    return true;
                }
                else if ( boost::iequals( targetLeg, "LEG2DISCOUNT" ) )
                {
                    return false;
                }
                MLIB_THROW( "Invalid Xccy Basis Curve Config: Target must me 'Leg1Discount' or 'Leg2Discount' rates." )
            }
            default:
            {
                MLIB_THROW( "Unable to find the basis curve target leg: Invalid Curve Type: Must be a TenorBasis or XccyBasis Curve." )
            }
        }
    }
   
}
