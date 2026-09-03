/*
* @brief			Class the defines the Swaption Trade Lightweight Object
* @Created:			18 July 2018
* @Author:			Ian Castleton
* @Department:		Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/


#pragma once

#include "LACoreTemplateType.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "LabelValueBlock.h"
#include "Option.h"
#include "Swap.h"

namespace etrading
{
	class SwaptionTrade : public Option
	{
	public:
		// Constructors
		SwaptionTrade( const std::string& objectName, const LabelValueBlock& dealLVB );
		SwaptionTrade( const SwaptionTrade& rhs );
		virtual ~SwaptionTrade() {};

		std::shared_ptr<Option> clone();

		// Get Accessor Methods
        OptionStyleEnum optionStyle() const             { return optionStyle_;          };
        LongShortPositionEnum longShort() const         { return longShort_;            };
        PayerReceiverSwaptionEnum payerReceiver() const { return payerReceiver_;        };
        SettlementTypeEnum settlementType() const       { return settlementType_;       };
        
        double strike() const                           { return strike_;               };
        LADate effectiveDate() const                    { return effectiveDate_;        };
        LADate adjustedOptionExpiryDate() const         { return adjustedOptionExpiryDate_;  };
        std::string tenorDescription() const            { return tenorDescription_;     };
        DayCountEnum optionDayCount() const             { return optionDayCount_;       };
        std::string notificationDays() const            { return notificationDays_;     };

        CCY currency() const                            { return currency_;             };
		CCY valuationCurrency() const                   { return valuationCurrency_;    };
        double notional() const                         { return notional_;             };
        double leverage() const                         { return leverage_;             };

        std::string swapGeneratorString() const         { return swapGeneratorString_;  };
        SwapPtr underlyingSwap() const                  { return underlyingSwap_;       };
        LADate swapStartDate() const                    { return swapStartDate_;        };
        LADate swapMaturityDate() const                 { return swapMaturityDate_;     };

        BusinessDayAdjustmentEnum busDayAdj() const     { return busDayAdj_;            };
        std::string calendar() const                    { return calendar_;             };
        DayCountEnum daycount() const                   { return daycount_;             };

        PayReceiveEnum feePayReceive() const            { return feePayReceive_;        };  
        double feePayReceiveIndicator() const           { return feePayReceiveIndicator_; };  
        double fee() const                              { return fee_;                  };
        LADate feeDate() const                          { return feeDate_;              };

	private:

		/* @brief Populates a default swap expression label value block, used for building the Swap instrument underlier for the swaption
		* @param [in]   asOfDate		The asOf / valuation date
        * @param [in]   swapStartDate   The Swap Start date. We explicitly provide the date, because the underlying swap cannot imply the start date if provided as a tenor, since it is the base / reference date.
        * @param [in]   deaLVB	        The deal LVB used for building the Swap
		*/
		LabelValueBlock setupSwapExpressionLVBforSwaption( const LADate& swapStartDate, const LabelValueBlock& dealLVB ) const;

		// Member data
		OptionStyleEnum optionStyle_;					// European, Bermudan, American
		LongShortPositionEnum longShort_;       
        PayerReceiverSwaptionEnum payerReceiver_;
        SettlementTypeEnum settlementType_;				// Price Cash / Price Yield / Physical Bilateral / Physical Cleared
        
        double strike_;
        LADate effectiveDate_;
        LADate adjustedOptionExpiryDate_;               // Adjusted for Notification Lag
        std::string tenorDescription_;                  // Optional Swaption Description
		std::string notificationDays_;                  // This is actually a tenor string, which is typically a number of days
        DayCountEnum optionDayCount_;	                // Used to calculate year fraction from asOf date to maturity
        
		CCY currency_;
		CCY valuationCurrency_;
		double notional_;
		double leverage_;

        std::string swapGeneratorString_;
		SwapPtr underlyingSwap_;
		LADate swapStartDate_;
		LADate swapMaturityDate_;

        BusinessDayAdjustmentEnum busDayAdj_;
        std::string calendar_;
        DayCountEnum daycount_;

        PayReceiveEnum feePayReceive_;
        double feePayReceiveIndicator_;
        double fee_;
        LADate feeDate_;
	};

}


