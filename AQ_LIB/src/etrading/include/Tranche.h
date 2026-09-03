/*
 * @brief			Structured Credit Tranche specification. Used in the Capital Structure calculations
 *					The capital structure consists of one or more tranches of increasing seniority;
 *					Losses are applied to the least senior tranche first, once that tranche capacity
 *					has been exhausted the losses are applied to the next most senior tranche.
 *
 *					A tranche may pay coupons.
 *
 *					This code was ported from MGEN AlgoQuantHub R analytics library
 *					Author Andrew Friend
 *
 *
 * @Created:		11th Dec 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#pragma once

#include "DataFrame.h"
#include "LabelValueBlock.h"
#include "CoreEnumerations.h"
#include "CommonConstants.h"
#include "LADate.h"

#include <vector>
#include <string>


namespace etrading
{
	/* @brief	Specifies a Tranche configuration: At minimum, specifies the tranche size and the tranche payment type.
	*			Optionally also specifies coupon information, and curve information used to discount the coupon payments.
	*/
	struct TrancheDefinition
	{
	public:

		/* @brief	Construct a TrancheDefinition from a LabelValue block. of the tranche definition.
		* @param[in]	name			The name of this TrancheDefinition. e.g. Equity, Mezzanine
		* @param[in]	trancheLVB		The label-value block specifying tranche parameters
		* @param[in]	validateKeys	A boolean value specifying whether to validate the keys in the trancheLVB.
		* @returns	The name of the TrancheDefinition object
		*/
		TrancheDefinition( const std::string& name, const LabelValueBlock& trancheLVB, const bool validateKeys );

		/* @brief	Main constructor of the tranche definition.
		*  @param[in]	name			The name of this tranche ( for example "equity", "mezzanine" )
		*  @param[in]	trancheSize		The size of this tranche i.e. how much loss can this trache absorb.	
		*  @param[in]	payType			Specifies whether this tranche pays principal sequentially or pro-rata
		*  @param[in]	couponType		Optional parameter which specifies whether the coupons are fixed or floating
		*  @param[in]	couponRate		Optional coupon rate
		*  @param[in]	couponFloor		Optional coupon floor
		*  @param[in]	couponFrequency	Optional specifies how often coupons are paid
		*  @param[in]	curveCollection	Optional curve collection
		*  @param[in]	resetCurve		Optional curve index used to discount coupons
		*  @param[in]	discountMargin  Optional discount margin. i.e. the fixed spread on top of the forward rate used for discounting
		*  @param[in]	curveFixingTable Optional fixing table, for fixing dates in the past
		*/
		TrancheDefinition(  const std::string& name,
							const double& trancheSize,
							const TrancheTypeEnum& payType,
							const TrancheCouponTypeEnum couponType = NONE_COUPON_TYPE,
							const double& couponRate = std::numeric_limits<double>::quiet_NaN(),
							const BooleanEnum couponFloor = NONE_BOOL,
							const FrequencyEnum couponFrequency = MONTHLY_FREQUENCY,
							const std::string& curveCollection = "",
							const std::string& resetCurve = "",
							const double& discountMargin = std::numeric_limits<double>::quiet_NaN(),
							const std::string& curveFixingTable = "" );

		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
				TRANCHE_KEYS::TRANCHE_SIZE
				, TRANCHE_KEYS::SENIORITY_RANK
				, TRANCHE_KEYS::TRANCHE_PAY_TYPE
				, TRANCHE_KEYS::COUPON_TYPE
				, TRANCHE_KEYS::COUPON_RATE
				, TRANCHE_KEYS::COUPON_FLOOR
				, TRANCHE_KEYS::COUPON_FREQUENCY
				, TRANCHE_KEYS::CURVE_COLLECTION
				, TRANCHE_KEYS::RESET_CURVE
				, TRANCHE_KEYS::DISCOUNT_MARGIN
				, TRANCHE_KEYS::FIXING_TABLE
			};
			std::vector<std::string> expectedKeys(arr, arr + sizeof(arr) / sizeof(arr[0]));

			return expectedKeys;
		}

		std::string name_;
		double trancheSize_;
		TrancheTypeEnum payType_;
		double couponRate_;
		TrancheCouponTypeEnum couponType_;
		BooleanEnum couponFloor_;
		FrequencyEnum couponFrequency_;
		std::string curveCollection_;
		std::string resetCurve_;
		double discountMargin_;
		std::string curveFixingTable_;

	private:
		void validateInputs() const;
	};

	/* @brief The TrancheScheduleData shows the state of the tranche at each period of time
	*/
	class TrancheScheduleData
	{
	public:
		/* @brief	Main initializer
		*  @param	trancheName			The name of the tranche this TrancheSchedule data belongs to
		*  @param	nPeriods			Specifies the number of periods of schedule data to store
		*  @param	trancheDefinition	Specifies the Trache definition for this tranche schedule data
		*/
		void construct( const std::string& trancheName, size_t nPeriods, const std::shared_ptr<TrancheDefinition>& trancheDefinition );

		/* @brief	Calculate the PVs of all coupon flows, and store within the tranche schedule
		*/
		void calculatePVs();

		/* @brief Calculates the NPV of coupon flows at the specified time period and valuation date, using discount margin
		*  @param[in]	callPeriod		calculate the PV of coupons at this time period
		*  @param[in]	valuationDate	PV the coupons to this valuation date
		*  @returns		The PV of all coupons occurring at the specified time period
		*/
		double calculateNPVDiscountMargin( const size_t callPeriod, const LADate& valuationDate ) const;

		/* @brief Calculates the NPV of coupon flows at the specified time period and valuation date
		*  @param[in]	callPeriod		calculate the PV of coupons at this time period
		*  @param[in]	valuationDate	PV the coupons to this valuation date
		*  @returns		The PV of all coupons occurring at the specified time period
		*/
		double calculateNPV( const double& interestRate, const size_t callPeriod, const LADate& valuationDate ) const ;

		/* @brief Calculates the internal rate of return implied by the specified target price
		*  @param[in]	callPeriod		calculate the PV of coupons at this time period
		*  @param[in]	valuationDate	PV the coupons to this valuation date
		*  @param[in]	targetPrice		Calculate the internal rate of return such that the PV of coupons matches this targetPrice.
		*/
		double calculateIRR( const size_t callPeriod, const LADate& valuationDate, const double& targetPrice ) const;

		/* @brief	Used for output / visualization: Convert the tranche schedule information into a dataframe
		*/
		DataFrame outputToLabelledDataFrame() const;

		std::string trancheName_;

		std::vector<LADate> fixingDate_;
		std::vector<LADate> accrualStartDate_;
		std::vector<LADate> accrualEndDate_;
		std::vector<LADate> paymentDate_;

		std::vector<double> balanceStart_;
		std::vector<double> balanceEnd_;
		std::vector<double> lossAllocated_;
		std::vector<double> principalAllocated_;
		std::vector<double> principalPayment_;
		std::vector<double> attachmentPoint_;
		std::vector<double> detachmentPoint_;
		std::vector<double> coupon_;
		std::vector<double> couponRate_;

		std::vector<double> couponYearFraction_;
		std::vector<double> resetRate_;
		std::vector<LADate> resetDate_;   // <-- is this not the same as fixingDate?
		std::vector<double> discountFactorRiskless_;
		std::vector<double> discountFactorDiscountMargin_;
		std::vector<double> riskWeight_;
		std::vector<double> pvRiskless_;
		std::vector<double> pvDiscountMargin_;
		std::vector<double> averageBalance_;
		std::vector<TrancheTypeEnum> payType_;

		size_t nPeriods_;
	};

	// ---- The following are deprecated

	/* @brief	This structure is used by the output of cashflowModel to specify how funds are allocated to a tranche
	*/
	struct TrancheAllocation
	{
		double balanceStart;
		double balanceEnd;
		double lossAllocated;
		double principalAllocated;
		double principalPayment;
		double coupon;
		double riskWeight;
		double attachmentPoint;
		double detachmentPoint;
		TrancheTypeEnum trancheType;
	};

	/* @brief	Used by the output of the cashflowModel to specify the cashflow breakdown in each period
	*/
	struct TrancheCashflow
	{
		int period;
		double poolBalance;
		double principalComponent;
		double severityRecovered;
		double prepayment;
		double lossGivenLoss;
		double coupon;
		std::vector<TrancheAllocation> trancheAllocations;  // A TrancheAllocation per TrancheDefinition. e.g. Senior, Mezz, Equity
		std::vector<TrancheScheduleData> trancheScheduleData;
	};

	typedef std::vector<TrancheCashflow> TrancheCashflows;

}