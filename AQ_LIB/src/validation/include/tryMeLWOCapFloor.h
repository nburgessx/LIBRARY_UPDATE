#pragma once

#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "RateProviderExternal.h"


using etrading::LabelValueBlock; 

namespace validation
{

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

	/* @brief			validation interface for the meLWOCapFloorCreate method
	*  @param [in]		objectName	    CapFloor object name
	*  @param [in]		tradeLVB			    CapFloor Label Value Block
	*  @param [in]		validateKeys	    True to validate the all keys provided are valid. Default to True
	*  @return			CapFloor Handle
	*/
	std::string tryMeLWOCapFloorCreate( const std::string& objectName, const LabelValueBlock& tradeLVB, const bool& validateKeys=true );

    /* @brief			validation interface for the meLWOCapFloorDisplay method, which display the INPUT parameters of the cached option
	*  @param [in]		objectName		CapFloor object name
	*  @return			CapFloor display of the input parameters
	*/
	AnyTypeMatrix tryMeLWOCapFloorDisplay( const std::string& objectName );

	/* @brief			validation interface for the meLWOLegDisplayCashflows method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Leg display of leg output
	*/
	AnyTypeMatrix tryMeLWOCapFloorDisplayCashflows(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, const std::string& fixingTableName = "", const bool& showColumnHeaders = true, const std::vector<std::string>& columnList = std::vector<std::string>());

	/* @brief			validation interface for the meLWOCapFloorPV method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @return			CapFloor PV
	*/
	double tryMeLWOCapFloorPV(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, const std::string& fixingTableName = "");

	/* @brief			validation interface for the meLWOCapFloorGreeksAnalytical method, to calculate Analytical greeks
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryMeLWOCapFloorGreeksAnalytical(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, const std::string& fixingTableName = "", const bool& showColumnHeaders = true);

	/* @brief			validation interface for the meLWOCapFloorGreeks method, to calculate Numerical greeks
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		curveCollection Curve collection
	*  @param [in]		deltaBump		Delta bump size 
	*  @param [in]		gammaBump		Gamma bump size
	*  @param [in]		vegaBump		Vega bump size
	*  @param [in]		thetaBump		Theta bump size
	*  @param [in]		fixingTableName	    Fixing table object name
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryMeLWOCapFloorGreeks(const std::string& objectName, const std::string& volObjectName, const std::string& curveCollection, 
												const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump,
												const std::string& fixingTableName = "", const bool& showColumnHeaders = true);

	//----- test methods to use external libor rates/discount factors like BB so that we know we are matching their prices-- //

	/* @brief			validation interface for the meLWOCapFloorDisplayCashflowsFromRates method
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			columnList      Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			CapFloor cashflow display
	*/
	AnyTypeMatrix tryMeLWOCapFloorDisplayCashflowsFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData, const bool& showColumnHeaders = true, const std::vector<std::string>& columnList = std::vector<std::string>());

	/* @brief			validation interface for the meLWOCapFloorGreeksAnalyticalFromRates method, to calculate Analytical greeks
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryMeLWOCapFloorGreeksAnalyticalFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData, const bool& showColumnHeaders = true);

	/* @brief			validation interface for the meLWOCapFloorGreeksFromRates method, to calculate Numerical greeks
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @param [in]		deltaBump		Delta bump size
	*  @param [in]		gammaBump		Gamma bump size
	*  @param [in]		vegaBump		Vega bump size
	*  @param [in]		thetaBump		Theta bump size
	*  @param [in]		showColumnHeaders	showColumnHeaders
	*  @return			CapFloor Greeks' display
	*/
	AnyTypeMatrix tryMeLWOCapFloorGreeksFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData, 
														const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump,
														const bool& showColumnHeaders = true);

	/* @brief			validation interface for the meLWOCapFloorPVFromRate method, to calculate Numerical greeks
	*  @param [in]		objectName		Cap Floor object name
	*  @param [in]		volObjectName	Volatility object name
	*  @param [in]		rateData		External discountFactors and liborRates
	*  @return			CapFloor PV
	*/
	double tryMeLWOCapFloorPVFromRates(const std::string& objectName, const std::string& volObjectName, const DoubleMatrix& rateData);
}
