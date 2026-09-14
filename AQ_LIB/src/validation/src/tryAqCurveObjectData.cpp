#include "tryAqCurveObjectData.h"

#include <sstream>


#include "RecordMacros.h"
#include "CurveData.h"
#include "Environment.h"
#include "ContainerUtilities.h"
#include "ObjectUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "ScheduleValidation.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{

    etrading::VariantMatrix tryAqCurveObjectDataDisplay( const std::string& mdcName )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( mdcName.c_str(), "", mdcName );

        auto result = etrading::Environment::defaultEnv().accessObject<etrading::CurveData>(mdcName)->getVariantMatrix();
    
        AQ_RECORD_DECORATED_OUTPUTS( mdcName.c_str(), "", mdcName );

        return result;
    
        VALID_EXCEPTION_END
    
    };

	const std::string tryAqCurveObjectDataCreate (
		const std::string& mdcName,
		const std::string& currency,
		const std::string& tenorString,
		const std::string& swapType,
		const AQLStringMatrix& swapStringBlock,
		const std::string& toTenorString,
		const AQLStringMatrix& fraStringBlock,
		const AQLStringMatrix& irFuturesStringBlock,
		const std::string& centralBankTypeString,
		const AQLStringMatrix& centralBankStringBlock,
		const AQLStringMatrix& fxStringBlock,
		const std::string& unitCurrency,
		const bool isInvertedFX
		)
	{
        VALID_EXCEPTION_START

        // Recording of inputs for playback
		AQ_RECORD_DECORATED_INPUTS( mdcName.c_str(), "", mdcName, currency, tenorString, swapType, swapStringBlock, toTenorString, fraStringBlock, irFuturesStringBlock, centralBankTypeString, centralBankStringBlock, fxStringBlock, unitCurrency, isInvertedFX );

		auto ccy = etrading::toCCYEnum(currency);
		auto curveTenorEnum = etrading::toCurveTenorEnum(tenorString);

		etrading::CurveData mdc(mdcName, curveTenorEnum);
		if(swapType != "")
		{
			auto swapQuoteType = etrading::toSwapQuoteTypeEnum(swapType);
			auto swapStringBlockUsed = etrading::transpose(swapStringBlock);
			if(swapStringBlockUsed.size() >= 2)
			{
				if(swapQuoteType != etrading::BASIS_SWAPTYPE)
				{
					etrading::SwapQuotes swapQuotes(
						etrading::createStringVector(swapStringBlockUsed[0]),  
						etrading::createDoubleVector(swapStringBlockUsed[1]),
						swapQuoteType, ccy, curveTenorEnum);  
					mdc.setSwapQuotes(std::move(swapQuotes));
				} else
				{
					auto toTenorEnum = etrading::toCurveTenorEnum(toTenorString);
					etrading::BasisSwapQuotes basisSwapQuotes(
						etrading::createStringVector(swapStringBlockUsed[0]),  
						etrading::createDoubleVector(swapStringBlockUsed[1]),
						ccy, curveTenorEnum, toTenorEnum);  
					mdc.setSwapQuotes(std::move(basisSwapQuotes));
				}
			} 
			else
			{
			   AQ_THROW( "Swap Quotes column size must be 2" );
			}
		}

		if(fraStringBlock.size() >= 2)
		{
			auto fraPeriodEnum = etrading::toFRAPeriodEnum(curveTenorEnum);
			auto fraStringBlockUsed = etrading::transpose(fraStringBlock);
			etrading::FRAQuotes fraQuotes(
				etrading::createStringVector(fraStringBlockUsed[0]),  
				etrading::createDoubleVector(fraStringBlockUsed[1]),
				fraPeriodEnum, ccy);  
			mdc.setFRAQuotes(std::move(fraQuotes));
		}

		if(irFuturesStringBlock.size() >= 2)
		{
			auto irFuturesStringBlockUsed = etrading::transpose(irFuturesStringBlock);

			auto fraPeriodEnum = etrading::toFRAPeriodEnum(curveTenorEnum);
			std::vector<double> convextiyAdjustements = std::vector<double>(0u);
			if(irFuturesStringBlockUsed.size() >= 5)
			{
				convextiyAdjustements = etrading::createDoubleVector(irFuturesStringBlockUsed[4]);
			}

			etrading::IRFuturesQuotes irfuturesQuotes(
				etrading::createStringVector(irFuturesStringBlockUsed[0]),  
				etrading::createDoubleVector(irFuturesStringBlockUsed[1]),
				etrading::createDateVector(irFuturesStringBlockUsed[2]),
				etrading::createDateVector(irFuturesStringBlockUsed[3]), ccy, fraPeriodEnum, convextiyAdjustements); 
			mdc.setIRFuturesQuotes(std::move(irfuturesQuotes));
		}

		if(centralBankTypeString != "")
		{
			auto centralBankEnum = etrading::toCentralBankTypeEnum(centralBankTypeString);
			auto centralBankStringBlockUsed = etrading::transpose(centralBankStringBlock);
			if(centralBankStringBlockUsed.size() >= 4)
			{
				etrading::CentralBankSwapQuotes cbSwapQuotes(				
					etrading::createStringVector(centralBankStringBlockUsed[0]),  
					etrading::createDoubleVector(centralBankStringBlockUsed[1]),
					etrading::createDateVector(centralBankStringBlockUsed[2]),
					etrading::createDateVector(centralBankStringBlockUsed[3]),
					centralBankEnum, ccy,curveTenorEnum);
				mdc.setCBSwapQuotes(std::move(cbSwapQuotes));
			} 
		}

		if(unitCurrency != "")
		{
			auto unitCcy = etrading::toCCYEnum(unitCurrency);
			auto fxStringBlockUsed = etrading::transpose(fxStringBlock);

			if(fxStringBlockUsed.size() >= 2)
			{
				etrading::FXQuotes fxQuotes(				
					etrading::createStringVector(fxStringBlockUsed[0]),  
					etrading::createDoubleVector(fxStringBlockUsed[1]),
					ccy, unitCcy,isInvertedFX);
				mdc.setFXQuotes(std::move(fxQuotes));
			}
		}
		etrading::moveToCache(std::move(mdc));
		auto ptrToMDC = etrading::Environment::defaultEnv().accessObject<etrading::CurveData>( mdcName );

        if( ptrToMDC != nullptr )
        {
            AQ_RECORD_DECORATED_OUTPUTS( mdcName.c_str(), "", mdcName );
            return mdcName.c_str();
        }
        else
        {
            std::ostringstream errStream;
            errStream << "Unable to create AQObjCurveData named " << mdcName;
            std::string errString = errStream.str();
            AQ_RECORD_DECORATED_OUTPUTS( mdcName.c_str(), "", errString.c_str() );
            AQ_THROW( errString );
        }

		VALID_EXCEPTION_END

	}

}