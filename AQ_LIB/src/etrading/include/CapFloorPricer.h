/*
* @brief			Class the defines the CapFloor Pricer
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#pragma once

#include "Option.h"
#include "Volatility.h"
#include "RateProvider.h"
#include "OptionCashflow.h"
#include "ScheduleValidation.h"
#include "OptionCashflow.h"
#include "BlackScholes.h"

namespace etrading
{
    class CapFloorPricer 
    {
    public:

		CapFloorPricer(const std::shared_ptr<Option>& option, const std::shared_ptr<Volatility>& volProvider, const std::shared_ptr<RateProvider>& rateProvider);
        CapFloorPricer(const CapFloorPricer& rhs);
		virtual ~CapFloorPricer() {}

		const double pv() const;
		const BlackScholesGreeks calculateGreeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump) const;

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;
		const AnyTypeMatrix view(const bool& showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const;
		const AnyTypeMatrix viewGreeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, const bool& showColumnHeaders) const;

	private:

		std::shared_ptr<Option> option_;
		std::shared_ptr<Volatility> volProvider_;
		std::shared_ptr<RateProvider> rateProvider_;
		std::vector<std::shared_ptr<OptionCashflow>> cashflows_;

		void createCashflows();


	};

}


