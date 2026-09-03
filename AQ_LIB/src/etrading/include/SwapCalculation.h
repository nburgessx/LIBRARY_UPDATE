/*
 * @brief			validation helper methods used within validation_api
 * @Created:		17 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "FixingTable.h"
#include "Leg.h"
#include "LegCollection.h"
#include "Bond.h"

namespace etrading
{
    std::pair<LegCollection, LegPtr> getBasisRefSpreadLegPair(const LegCollection& legs, const LAString& spreadLegName);

    std::pair<LegCollection, LegPtr> getXccyBasisRefSpreadLegPair(const LegCollection& legs, const LAString& notionalResetLegName, const LAString& spreadLegName);

	std::pair<bool, double> solveSpread( DataProvider& dataProviderSpreadLeg, double pv, double pvRefLeg, double deltaPV, double spread, double epsilonForPV, const LegPtr& spreadLeg );

    double calculateSwapSpread(const LegCollection& refLegs, const LegPtr& spreadLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread);
    
    double calculateXccySwapSpread(const LegCollection& legs, const LAString& spreadLegName, const LAString& notionalResetLegName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread);

    double calculateBasisSwapSpread(const LegCollection& legs, const LAString& spreadLegName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, bool isParSpread);

	/* @brief Calculate AsOfDateFxRate
	*
	* @param[in]	valuationSettingsLVB		valuationSettingsLVB
	* @param[in]	valuationCurrency			valuation currency chosed by user
	* @param[in]	leg1						First leg
	* @param[in]	leg2						Second leg
	* @returns		The asOfDateFxRate
	*/
	double calculateAsOfDateFxRate(const LabelValueBlock& valuationSettingsLVB, const CCY& valuationCurrency, const LegPtr& leg1, const LegPtr& leg2);

    void updateMTMXccyNotionalResetByFxLeg(const LabelValueBlock& valuationSettingsLVB, const LegPtr& leg1, const LegPtr& leg2, const LAString& notionalResetLegName, bool isMTM, const LAString& legName);

    double calculateParRate(const LabelValueBlock& valuationSettingsLVB, const LegCollection& legs, const LabelValueBlock& fixingTableNames);

    double calculateIRR(const LabelValueBlock& valuationSettingsLVB, const LegCollection& legs, const LabelValueBlock& fixingTableNames);
    
    double calculateNotionalFromFutureValueNotional(const LegPtr& leg);

    void setNotionalFromFutureValueNotional(const LegPtr& leg1, const LegPtr& leg2, const CCY& valuationCurrecy=NO_CCY, double spotFxRate=1.0);

	void setSwapLegSpreadOrFixedRate(DataProvider& dataProviderSpreadLeg, const ScheduleTypeEnum& legType, double spread);

}


