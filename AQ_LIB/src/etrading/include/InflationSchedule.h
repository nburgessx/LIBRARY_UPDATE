/*
 * @brief			Class the defines the Inflation Leg Schedule of a Zero Coupon Inflation Swap
 * @Created:		21 May 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "Schedule.h"


namespace etrading
{
    class InflationSchedule : public Schedule
    {
    public:
		InflationSchedule( const std::string& instanceName );
		InflationSchedule( const LabelValueBlock& scheduleLVB, const std::string& instanceName );
		InflationSchedule( const InflationSchedule& rhs );
		virtual ~InflationSchedule() {};
        SchedulePtr clone();

		/* @brief	Specifies the inflation index level at the start and end of the schedule
		*  @param	baseIndex	The base / reference inflation level at the start of the inflation schedule
		*  @param	resetIndex	The reset inflation level at the end of the inflation schedule
		*/
		void setInflationIndex( const double baseIndex, const double resetIndex );

		// Specify the columns which may appear in the cashflow table display
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;

		// @brief Calculate a one-period Inflation schedule
		virtual void calculateScheduleDates();

		// @brief Returns the inflationResetType: e.g. MonthlyInterpolation, DailyInterpolation
		InflationResetTypeEnum getInflationResetType() const;

   	protected:
		void populateNotionalAndPaymentFreqEnum( const LabelValueBlock& scheduleLVB=LabelValueBlock() );
        
		virtual void createCashflows();

	private:
		InflationResetTypeEnum inflationResetType_;

    };

}

