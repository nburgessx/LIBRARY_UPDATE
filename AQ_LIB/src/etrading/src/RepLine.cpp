/*
 * @brief			Structured Credit Representative Line (Rep Line) Item
 *					In Rep Line analysis, a large portfolio of loans is represented
 *					by a small number of hypothetical loans. Each Rep Line represents
 *					a subset of the portfolio with the statistical characteristics of that group.
 *					Each loan within the group is assumed to be homogeneous and to have the
 *					same behaviour as the Rep Line.
 *
 *					This code was ported from MGEN MHI R analytics library
 *					Author Andrew Friend
 *
 * @Created:		3rd Dec 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#include "RepLine.h"



// Private helper functions
namespace
{


}

namespace etrading
{

	// Main constructor
	RepLine::RepLine( const std::string& name,
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
					  int loanFrequency )
					:	name_( name ),
						proportion_( proportion ),
						term_( term ),
						balance_( balance ),
						loanRate_( loanRate ),
						defaultRate_( defaultRate ),
						severity_( severity ),
						prepayment_( prepayment ),
						riskWeight_( riskWeight ),
						kirb_( kirb ),
						loanType_( loanType ),
						loanFrequency_( loanFrequency )
	{
	}

	// Allows the balance to be updated after construction
	void RepLine::setBalance( const double& balance )
	{
		balance_ = balance;
	}

	double RepLine::getProportion() const
	{
		return proportion_;
	}

	double RepLine::getTerm() const
	{
		return term_;
	}

	double RepLine::getBalance() const
	{
		return balance_;
	}

	double RepLine::getLoanRate() const
	{
		return loanRate_;
	}

	double RepLine::getDefaultRate() const
	{
		return defaultRate_;
	}

	double RepLine::getSeverity() const
	{
		return severity_;
	}

	double RepLine::getPrepayment() const
	{
		return prepayment_;
	}

	double RepLine::getRiskWeight() const
	{
		return riskWeight_;
	}

	double RepLine::getKirb() const
	{
		return kirb_;
	}

	LoanTypeEnum RepLine::getLoanType() const
	{
		return loanType_;
	}

	int RepLine::getLoanFrequency() const
	{
		return loanFrequency_;
	}

}

