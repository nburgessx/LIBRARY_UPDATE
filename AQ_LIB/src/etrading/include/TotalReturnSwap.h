/*
 * @brief			Class which defines the Constant Maturity Swap product
 * @Created:		16 March 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>
#include "Swap.h"

#include "CreditModel.h"

namespace etrading
{

    class TotalReturnSwap : public Swap 
    {
	public:
		TotalReturnSwap( const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB );
   		TotalReturnSwap( const TotalReturnSwap& rhs );

		virtual ~TotalReturnSwap() {}
        
        std::shared_ptr<Swap> clone();

		/* @brief Calculates the total PV of all of the Credit Default Swap legs.
		*
		* @param[in]	creditModel				The calibrated credit model
		* @param[in]	fixingTableNames			A map of fixing table names, indexed by legName
		* @param[in]	legName					Optionally calculate the PV of the specified leg only.
		* @returns	The calculated PV value
		*/
		double pv( const std::string& creditModelName, const LabelValueBlock& fixingTableNames, const std::string& legName = "") const;

		/* @brief Calculates the par rate of the premium leg of the total return swap
		*
		* @param[in]	creditModel				The calibrated credit model
		* @param[in]	fixingTableNames			A map of fixing table names, indexed by legName
		* @returns		The par rate of the TRS premium leg	
		*/
		double parRate( const std::string& creditModelName, const LabelValueBlock& fixingTableNames ) const;

		/* @brief Calculates the par spread of the float leg of the total return swap
		*
		* @param[in]	creditModel				The calibrated credit model
		* @param[in]	fixingTableNames			A map of fixing table names, indexed by legName
		* @returns		The par spread of the TRS float leg	
		*/
		double spread( const std::string& creditModelName, const LabelValueBlock& fixingTableNames ) const;

		/* @brief Calculates the annuity of the specified Total Return Swap leg
		*
		* @param[in]	creditModelName		Credit Model object name
		* @param[in]	legName			    Mandatory, calculate the annuity of this leg.
		* @returns	The calculated annuity value
		*/
		double annuity( const std::string& creditModelName, const std::string& legName ) const;

	private:
		void validateCreditModel( const CreditModel& creditModel ) const;
	};

}

