#pragma once


#include "CoreEnumerations.h"

#include <string>
#include <vector>


namespace etrading
{

	class RepLine
	{
	public:
		RepLine( const std::string& name,
				 const double& proportion,
				 const double& term,
				 const double& balance,
				 const double& loanRate,
				 const double& defaultRate,
				 const double& severity,
				 const double& prepayment,
				 const double& riskWeight,
				 const double& kirb,
				 const LoanTypeEnum loanType,
				 int loanFrequency );

		// Allows the balance to be updated after construction
		void setBalance( const double& balance );

		// Various getter methods
		double getProportion() const;
		double getTerm() const;
		double getBalance() const;
		double getLoanRate() const;
		double getDefaultRate() const;
		double getSeverity() const;
		double getPrepayment() const;
		double getRiskWeight() const;
		double getKirb() const;
		LoanTypeEnum getLoanType() const;
		int getLoanFrequency() const;


	private:
		std::string name_;		// THe name of this RepLine
		double proportion_;		// Represents the proportion of the portfolio represented by this RepLine. i.e. the proportion of a reinvestment
		double term_;			// The maturity of the RepLine in months
		double balance_;		// The outstanding balance of the RepLine
		double loanRate_;		// The loan interest rate
		double defaultRate_;	// The loan default rate
		double severity_;		// The loan recovery rate
		double prepayment_;		// The loan prepayment rate
		double riskWeight_;		// The loan risk weight
		double kirb_;			// THe loan regulatory capital requirement
		LoanTypeEnum loanType_;	// The loan type i.e. Amortizing or bullet
		int loanFrequency_;		// The frequency of interest rate payments

	};

	
	


}

