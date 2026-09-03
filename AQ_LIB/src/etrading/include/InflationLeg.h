/*
 * @brief			Class which defines the Inflation Leg of a Zero Coupon Inflation Swap
 * @Created:		19 May 2030
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "Leg.h"
#include "InflationSchedule.h"
#include "InflationCurve.h"
#include "CoreEnumerations.h"



namespace etrading
{
    class InflationLeg : public Leg
    {
    public:

		InflationLeg( const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={} );
        InflationLeg( const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule );
		InflationLeg( const InflationLeg& rhs );
		virtual ~InflationLeg() {}

		LegPtr clone();

		ScheduleTypeEnum getType() const;

		static std::vector<std::string> legLVBWithoutScheduleKeys()
		{
			std::vector<std::string> expectedKeys = Leg::lvbKeys();
			return expectedKeys;
		}

		static std::vector<std::string> lvbKeys()
		{
			std::vector<std::string> expectedKeys = InflationLeg::legLVBWithoutScheduleKeys();
			std::vector<std::string> scheduleKeys = InflationSchedule::lvbKeys();
			expectedKeys.insert(expectedKeys.end(), scheduleKeys.begin(), scheduleKeys.end());
			return expectedKeys;
		}

		/* @brief Updates the cashflow discount factors using the provided curveCollection
		*
		* @param[out]	dataProvider		A reference to the DataProvider object which will be populated
		* @param[in]	updateCurveData		Whether to update the Curve data in the dataProvider
		*/
		virtual void initializeDataProvider( DataProvider& dataProvider, bool updateCurveData=true);

		/* @brief Calculates the PV of the Inflation leg, given inflation Index fixing values at start and end of the leg
		*
		* @param[in]	dataProvider			The data provider which holds the valuation settings
		* @param[in]	baseIndex				The inflation level at the effective date of the leg
		* @param[in]	resetIndex				The inflation level at the maturity of the leg
		* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
		* @returns	The calculated PV value
		*/
		double pvFromInflationIndex( DataProvider& dataProvider, const double baseIndex, const double resetIndex, bool updateCurveData=true );

		/* @brief Calculates the PV of the Inflation leg, given an inflation curve
		*
		* @param[in]	dataProvider			The data provider which holds the valuation settings
		* @param[in]	inflationCurve			An inflation curve from which index levels can be found
		* @param[in]	updateCurveData			Whether to update the Curve details in the leg static data
		* @returns	The calculated PV value
		*/
		double pv( DataProvider& dataProvider, const InflationCurve& inflationCurve, bool updateCurveData = true );

		std::unordered_set<CashflowHeaderEnum, EnumClassHash> allowedColumns() const;

	private:

	};

}