// ExtractCurveCalibrationData.cpp

/*
 * @brief			Helper methods to extract yield curve calibration inputs
 * @Created:		8th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "ExtractCurveCalibrationData.h"

namespace google_test
{

    // Function to get curve calibrationData
    etrading::VariantVector curveCalibrationData( const std::string & curveObject, const etrading::CurveMarketDataEnum & marketDataEnum, const unsigned int & columnNumber )
    {
        return validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( marketDataEnum ), columnNumber );
    }


    // Function to get calibration data from a curve - Basis Swap Tenors
    std::vector<std::string> curveCalibrationOisSwapTenors( const std::string & curveObject )
    {
        // Extract the Swap Terms from the LWO Curve Object
        etrading::VariantVector oisSwapTerms = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::OIS_MARKETDATA ), 1 ); // OIS_MARKETDATA Column 1

        // Convert from Variant
        std::vector<std::string> oisSwapTermsAsString( oisSwapTerms.size() );
        for ( unsigned int i = 0; i < oisSwapTerms.size(); ++i )
        {
            oisSwapTermsAsString[i] = oisSwapTerms[i].getValueAsString();
        }
        
        return oisSwapTermsAsString;
    }
        

    // Function to get calibration data from a curve - Basis Swap Spreads
    std::vector<double> curveCalibrationOisSwapParRates(const std::string & curveObject )
    {
        // Extract the Swap Par Rates from the LWO Curve Object
        etrading::VariantVector oisParRates = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::OIS_MARKETDATA ), 2 ); // OIS_MARKETDATA Column 2

        // Convert from Variant
        std::vector<double> oisParRatesAsDouble( oisParRates.size() );
        for ( unsigned int i = 0; i < oisParRates.size(); ++i )
        {
            oisParRatesAsDouble[i] = oisParRates[i].getValue<double>();
        }
        
        return oisParRatesAsDouble;
    }


    // Function to get calibration data from a curve - Basis Swap Tenors
    std::vector<std::string> curveCalibrationLiborOisTenors( const std::string & curveObject )
    {
        // Extract the Swap Terms from the LWO Curve Object
        etrading::VariantVector liborOisTerms = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::LIBOR_OIS_BASISSPREAD_MARKETDATA ), 1 ); // LIBOR_OIS_BASISSPREAD_MARKETDATA Column 1

        // Convert from Variant
        std::vector<std::string> liborOisTermsAsString( liborOisTerms.size() );
        for ( unsigned int i = 0; i < liborOisTerms.size(); ++i )
        {
            liborOisTermsAsString[i] = liborOisTerms[i].getValueAsString();
        }
        
        return liborOisTermsAsString;
    }
    

    // Function to get calibration data from a curve - Basis Swap Spreads
    std::vector<double> curveCalibrationLiborOisSpreads(const std::string & curveObject )
    {
        // Extract the Swap Terms from the LWO Curve Object
        etrading::VariantVector liborOisSpreads = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::LIBOR_OIS_BASISSPREAD_MARKETDATA ), 2 ); // LIBOR_OIS_BASISSPREAD_MARKETDATA Column 2

        // Convert from Variant
        std::vector<double> liborOisSpreadsAsDouble( liborOisSpreads.size() );
        for ( unsigned int i = 0; i < liborOisSpreads.size(); ++i )
        {
            liborOisSpreadsAsDouble[i] = liborOisSpreads[i].getValue<double>();
        }
        
        return liborOisSpreadsAsDouble;
    }


    // Function to get calibration data from a curve - Swap Tenors
    std::vector<std::string> curveCalibrationSwapTenors( const std::string & curveObject )
    {
        // Extract the Swap Terms from the LWO Curve Object
        etrading::VariantVector swapTerms = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::SWAP_MARKETDATA ), 1 ); // SWAP_MARKETDATA Column 1

        // Convert from Variant
        std::vector<std::string> swapTermsAsString( swapTerms.size() );
        for ( unsigned int i = 0; i < swapTerms.size(); ++i )
        {
            swapTermsAsString[i] = swapTerms[i].getValueAsString();
        }
        
        return swapTermsAsString;
    }


    // Function to get calibration data from a curve - Swap Par Rates
    std::vector<double> curveCalibrationSwapParRates(const std::string & curveObject )
    {
        // Extract the Swap Par Rates from the LWO Curve Object
        etrading::VariantVector parRates = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::SWAP_MARKETDATA ), 2 ); // SWAP_MARKETDATA Column 2

        // Convert from Variant
        std::vector<double> parRatesAsDouble( parRates.size() );
        for ( unsigned int i = 0; i < parRates.size(); ++i )
        {
            parRatesAsDouble[i] = parRates[i].getValue<double>();
        }
        
        return parRatesAsDouble;
    }


    // Function to get calibration data from a curve - Basis Swap Tenors
    std::vector<std::string> curveCalibrationBasisSwapTenors( const std::string & curveObject )
    {
        // Extract the Swap Terms from the LWO Curve Object
        etrading::VariantVector basisSwapTerms = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA ), 1 ); // BASIS_SWAP_MARKETDATA Column 1

        // Convert from Variant
        std::vector<std::string> basisSwapTermsAsString( basisSwapTerms.size() );
        for ( unsigned int i = 0; i < basisSwapTerms.size(); ++i )
        {
            basisSwapTermsAsString[i] = basisSwapTerms[i].getValueAsString();
        }
        
        return basisSwapTermsAsString;
    }
    

    // Function to get calibration data from a curve - Basis Swap Spreads
    std::vector<double> curveCalibrationBasisSwapSpreads(const std::string & curveObject )
    {
        // Extract the Swap Par Rates from the LWO Curve Object
        etrading::VariantVector basisSwapSpreads = validation::tryAqObjectsCurveMarketDataColumn( curveObject, toString( etrading::CurveMarketDataEnum::BASIS_SWAP_MARKETDATA ), 2 ); // BASIS_SWAP_MARKETDATA Column 2

        // Convert from Variant
        std::vector<double> basisSwapSpreadsAsDouble( basisSwapSpreads.size() );
        for ( unsigned int i = 0; i < basisSwapSpreads.size(); ++i )
        {
            basisSwapSpreadsAsDouble[i] = basisSwapSpreads[i].getValue<double>();
        }
        
        return basisSwapSpreadsAsDouble;
    }

}