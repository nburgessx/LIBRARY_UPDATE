#include "tryMeCashflowClient.h"
#include "CashflowModel.h"
#include "LoanCalculations.h"
#include "LabelValueBlockValidation.h"
#include "ExceptionMacros.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"
#include "DataUtilities.h"					// For AQ_TO_STRING macros

namespace validation_api
{
	namespace
	{
		// Helper functions to assist with loading loan CSV files

		/* @brief Converts a CSV delimited line of text into tokens
		*  @param[in]	line	A comma delimited line of text
		*  @returns		The CSV line converted broken into string tokens
		*/
		std::vector<std::string> parseCSVLine( const std::string& line )
		{
			std::stringstream ss( line );
			std::vector<std::string> tokens;
		
			std::string token;
			while ( getline( ss,  token, ',' ))
			{
				tokens.push_back( token );
			}
			return tokens;
		}

		/* @brief	Given a vector of column names, returns the index position of the required column name in the vector
		*  @param[in]	fileColumnHeadings	A string vector containing a list of column headings
		*  @param[in]	requiredColumnName	A string containing the column names to search for in fileColumnHeadings
		*  @returns	The index position of requiredColumnName in the fileColumnHeadings
		*/
		int getColumnIndexForColumnName( const std::vector<std::string>&  fileColumnHeadings, const std::string& requiredColumnName )
		{
			auto it = std::find( fileColumnHeadings.begin(), fileColumnHeadings.end(), requiredColumnName );
			if ( it == fileColumnHeadings.end() )
			{
				AQ_THROW( "Could not find column " + requiredColumnName + " in the CSV file headings." );
			}		 

			const int index = std::distance( fileColumnHeadings.begin(), it );
			return index;
		}

		/* @brief	For each columnName in requiredColumnNames, finds the corresponding column index inside fileColumnHeadings.
		*  @param[in]	fileColumnHeadings	A string vector containing a list of column headings
		*  @param[in]	requiredColumnNames	A string vector containing column names to search for in fileColumnHeadings
		*  @returns	A vector of column indices
		*/
		std::vector<int> getColumnIndices( const std::vector<std::string>& fileColumnHeadings, const etrading::LabelValueBlock& requiredColumnNames )
		{
			const std::string collectionName( "RequiredCsvColumnNames" );
			
			std::vector<int> indices;
			indices.push_back( getColumnIndexForColumnName( fileColumnHeadings, requiredColumnNames.getCompulsoryValueAsString( etrading::STRUCTURED_CREDIT_CSV_KEYS::EXPOSURE_AT_DEFAULT, collectionName ) ) );
			indices.push_back( getColumnIndexForColumnName( fileColumnHeadings, requiredColumnNames.getCompulsoryValueAsString( etrading::STRUCTURED_CREDIT_CSV_KEYS::RISK_WEIGHT, collectionName ) ) );
			indices.push_back( getColumnIndexForColumnName( fileColumnHeadings, requiredColumnNames.getCompulsoryValueAsString( etrading::STRUCTURED_CREDIT_CSV_KEYS::KIRB, collectionName ) ) );
			
			return indices;
		}

		/* @brief	Given a row of tokens, returns a double vector containing the token values specified by requiredColumnIndices
		*  @param[in]	tokens	A vector of values corresponding to values in a CSV file
		*  @param[in]	requiredColumnIndices	Specifies the subset of token columns we are interested in
		*  @returns	a vector of token values corresponding to requiredColumnIndices
		*/
		DoubleVector extractColumnValues( const std::vector<std::string>& tokens, const std::vector<int>& requiredColumnIndices )
		{
			DoubleVector values;
			for ( size_t i=0; i<requiredColumnIndices.size(); i++ )
			{
				const size_t columnIndex = requiredColumnIndices[ i ];
				const std::string& token = tokens[ columnIndex ];
				char * pFirstNonNumber;
                const double value = std::strtod( token.c_str(), &pFirstNonNumber );
				values.push_back( value );
			}
			return values;
		}

		/* @brief	Given the filename of a CSV file with column headings, reads in the file and populates a DoubleMatrix with the values specified by requiredColumnNames
		*  @param[in]	filename			A fully qualified filename i.e. the full path to the CSV file
		*  @param[in]	requiredColumnNames	A string vector containing the column names of the CSV file that are required
		*  @returns		A DoubleMatrix containing the subset of values from the CSV file corresponding to requiredColumnNames
		*/
		DoubleMatrix readColumnsFromCSVFile( const std::string& filename, const LabelValueBlock& requiredColumnNames )
		{
			std::ifstream infile( filename.c_str() );
			if ( ! infile )
			{
				AQ_THROW( "Unable to open file: " + filename );
			}
		
			std::string line;
			std::getline( infile, line );
			const std::vector<std::string> fileColumnHeadings = parseCSVLine( line );
		
			const std::vector<int> requiredColumnIndices = getColumnIndices( fileColumnHeadings, requiredColumnNames );

			DoubleMatrix matrixValues;
			while (std::getline( infile, line ))
			{			
				std::vector<std::string> tokens = parseCSVLine( line );
				DoubleVector values = extractColumnValues( tokens, requiredColumnIndices );
				matrixValues.push_back( values );
			}

			return matrixValues;
		}

		// --------------------------------------

		/* @brief Private helper method which calculates the monthly cashflow breakdown for a loan.
		*		  Originally: loopLoans
		*  @param[in] originalBalance	The initial loan amount
		*  @param[in] riskWeight		The riskweighting of the loan
		*  @param[in] kirb				The capital requirement
		*  @param[in] term				The length of the loan in months
		*/
		std::vector<etrading::LoanCashflowBreakdown> processSingleLoan( const double& originalBalance, const double& riskWeight, const double& kirb, const int& term = 120 )
		{

			const double loanRate = 0.05; // Question for StructuredCredit team: Why is the loanRate hard coded?

			const size_t nVectorElements = term;
			const std::vector<double> defaultVector( nVectorElements, 0.0 );
			const std::vector<double> severityVector( nVectorElements, 0.5 ); // Question for StructuredCredit team: Why is the loanRate hard coded?
			const std::vector<double> prepaymentVector( nVectorElements, 0.0 );
			const std::vector<double> riskWeightVector( nVectorElements, riskWeight );
			const std::vector<double> kirbVector( nVectorElements, kirb );
			const etrading::LoanTypeEnum loanTypeEnum = etrading::AMORTIZING_LOAN_TYPE;

			return etrading::loanAmortizationDefaultPrepayment( term,
																loanRate,
																originalBalance,
																defaultVector,
																severityVector,
																prepaymentVector,
																riskWeightVector,
																kirbVector,
																loanTypeEnum );
		}

		/* @brief	Computes the amortized cashflow breakdown of a portfolio of loans
		*			Originally: "loanByLoan"
		*  @param[in]	loanMatrix	Each row contains a loan summarized with the three quantities:  "EAD", "RW", "KIRB"
		*  @returns		A structure containing the monthly cashflows and default amounts of the loan portfolio
		*/
		std::vector<etrading::LoanCashflowBreakdown> processMultipleLoans( const DoubleMatrix& loanMatrix )
		{
			const size_t nRows = loanMatrix.size();

			std::vector<etrading::LoanCashflowBreakdown> total;
			for ( size_t row=0; row< nRows; row++ )
			{
				const DoubleVector loanParameters = loanMatrix[ row ];
				const double originalBalance = loanParameters[0];
				const double riskWeight      = loanParameters[1];
				const double kirb            = loanParameters[2];

				const std::vector<etrading::LoanCashflowBreakdown> loanCashflows = processSingleLoan( originalBalance, riskWeight, kirb );

				// Accumulate each column of loanCashflows into total
				if ( row == 0 )
				{
					total = loanCashflows;
				}
				else
				{
					for ( size_t period = 0; period < loanCashflows.size(); period++ )
					{
						total[ period ].accumulate( loanCashflows[ period ] );
					}
				}
			}
			return total;
		}

		/* @brief Populates a LAStringMatrix from a vector of LoanCashflowBreakdown struct
		*  @param[in]	cashflowBreakdown	a vector of LoanCashflowBreakdown ( a struct containing various loan cashflow parameters )
		*  @param[in]	includeCashflowColumnHeaders	Specifies whether to label the output LAStringMatrix with column headings
		*  @returns		A LAStringMatrix containing all the values from cashflowBreakdown, optionally annotated with column headings.
		*/
		LAStringMatrix cashflowBreakdownToMatrix( const std::vector<etrading::LoanCashflowBreakdown> cashflowBreakdown, const bool& includeCashflowColumnHeaders )
		{
			LAStringMatrix cashflowMatrix;

			if ( includeCashflowColumnHeaders )
			{
				LAStringVector header = { "period", "balanceStart", "defaultBalance", "lossGivenLoss", "severityRecovered", "prepayment", "balanceAfterDefaultAndPrepayment", "loanPayment", "interestComponent", "principalComponent", "balanceEnd", "riskWeight", "kirbAmount" };
				cashflowMatrix.push_back( header );
			}

			const size_t nCashflows = cashflowBreakdown.size();
			for (size_t i=0; i<nCashflows; i++ )
			{
				const etrading::LoanCashflowBreakdown& cashflow = cashflowBreakdown[ i ];
				LAStringVector row;

				row.push_back( AQ_TO_STRING_FROM_INT( cashflow.period ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.balanceStart ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.defaultBalance ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.lossGivenLoss ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.severityRecovered ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.prepayment ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.balanceAfterDefaultAndPrepayment ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.loanPayment ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.interestComponent ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.principalComponent ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.balanceEnd ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.riskWeight ).c_str() );
				row.push_back( AQ_TO_STRING_FROM_DOUBLE( cashflow.kirbAmount ).c_str() );

				cashflowMatrix.push_back( row );
			}

			return cashflowMatrix;
		}
	}

	/* @brief Projects the monthly cashflows of an amortizing loan
	* @param[in]	term				The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate			The annualized loan rate
	* @param[in]	originalBalance		The loan amount
	* @param[in]	defaultVector		Specifies the default rate in each period of the loan
	* @param[in]	severityVector		Specifies the percentage loss, if a default occurs
	* @param[in]	prepaymentVector	Specifies the prepayment rate in each period of the loan
	* @param[in]	riskWeightVector	Specifies the risk-weight in each period of the loan
	* @param[in]	kirbVector			Specifies the capital requirement in each period fo a loan 
	* @param[in]	loanType			Whether  the loan is amortizing or has bullet repayment.
	*
	* @returns	A structure  containing the monthly loan cashflows and default amounts
	*/
	std::vector<etrading::LoanCashflowBreakdown> tryMeSRTAmortizeSingleLoan( const int& term,
																			 const double& loanRate,
																			 const double& originalBalance,
																			 const std::vector<double>& defaultVector,
																			 const std::vector<double>& severityVector,
																			 const std::vector<double>& prepaymentVector,
																			 const std::vector<double>& riskWeightVector,
																			 const std::vector<double>& kirbVector,
																			 const std::string& loanType )
	{
		VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( term, loanRate, originalBalance, defaultVector, severityVector, prepaymentVector, riskWeightVector, kirbVector, loanType );

		const etrading::LoanTypeEnum loanTypeEnum = etrading::toLoanTypeEnum( loanType );
		return etrading::loanAmortizationDefaultPrepayment( term,
															loanRate,
															originalBalance,
															defaultVector,
															severityVector,
															prepaymentVector,
															riskWeightVector,
															kirbVector,
															loanTypeEnum );
		VALID_EXCEPTION_END
	}

	/* @brief	Projects the monthly cashflows of an amortizing loan. Similar to the function above
	*			except that the results are presented in an Excel-friendly matrix.
	* @param[in]	term				The term of the loan, in months. i.e. 5 years = 60
	* @param[in]	loanRate			The annualized loan rate
	* @param[in]	originalBalance		The loan amount
	* @param[in]	defaultVector		Specifies the default rate in each period of the loan
	* @param[in]	severityVector		Specifies the percentage loss, if a default occurs
	* @param[in]	prepaymentVector	Specifies the prepayment rate in each period of the loan
	* @param[in]	riskWeightVector	Specifies the risk-weight in each period of the loan
	* @param[in]	kirbVector			Specifies the capital requirement in each period fo a loan 
	* @param[in]	loanType			Whether  the loan is amortizing or has bullet repayment.
	*  @param[in]	includeCashflowColumnHeaders	Whether to annotate the LAStringMatrix with column headers
	*
	* @returns	A matrix containing the cashflows and default amounts
	*/
	LAStringMatrix tryMeSRTAmortizeSingleLoanAsMatrix( const int& term,
													 const double& loanRate,
													 const double& originalBalance,
													 const std::vector<double>& defaultVector,
													 const std::vector<double>& severityVector,
													 const std::vector<double>& prepaymentVector,
													 const std::vector<double>& riskWeightVector,
													 const std::vector<double>& kirbVector,
													 const std::string& loanType,
													 const bool& includeCashflowColumnHeaders )
	{
		VALID_EXCEPTION_START

		const std::vector<etrading::LoanCashflowBreakdown> cashflowBreakdown = tryMeSRTAmortizeSingleLoan(  term,
																											loanRate,
																											originalBalance,
																											defaultVector,
																											severityVector,
																											prepaymentVector,
																											riskWeightVector,
																											kirbVector,
																											loanType );

		const LAStringMatrix cashflowMatrix = cashflowBreakdownToMatrix( cashflowBreakdown, includeCashflowColumnHeaders );
		return cashflowMatrix;

		VALID_EXCEPTION_END
	}

	/* @brief	Reads a CSV file containing a portfolio of loans and computes the cashflow amortization of the portfolio
	*  @param[in]	loanCsvFilename	The CSV file containing the loan portfolio details.
	*  @param[in]	csvColumnNames	The CSV file can contain many columns; this data lists the column names corresponding to "EAD", "RW", "KIRB"
	*
	*  @returns	A structure containing the monthly cashflows and default amounts of the loan portfolio
	*/
	std::vector<etrading::LoanCashflowBreakdown> tryMeSRTAmortizeLoanPortfolio( const std::string& loanCsvFilename, const LabelValueBlock& csvColumnNames )
	{
		VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( loanCsvFilename, csvColumnNames );

		const bool validateKeys = true;
		const std::string lvbName( "ColumnNames" );
		std::vector<std::string> expectedKeys = LoanPortfolio::lvbKeys();
		etrading::validateKeysForLVB( expectedKeys, csvColumnNames.getKeys(), validateKeys, lvbName );
				
		DoubleMatrix loanMatrix = readColumnsFromCSVFile( loanCsvFilename, csvColumnNames );

		return processMultipleLoans( loanMatrix );

		VALID_EXCEPTION_END
	}

	/* @brief	Reads a CSV file containing a portfolio of loans and computes the cashflow amortization of the portfolio
	*  @param[in]	loanCsvFilename	The CSV file containing the loan portfolio details.
	*  @param[in]	csvColumnNames	The CSV file can contain many columns; this data lists the column names corresponding to "EAD", "RW", "KIRB"
	*  @param[in]	includeCashflowColumnHeaders	Specifies whether to annotate the LAStringMatrix with column headings.
	*
	*  @returns	A LAStringMatrix containing the monthly cashflows and default amounts of the loan portfolio
	*/
	LAStringMatrix tryMeSRTAmortizeLoanPortfolioAsMatrix( const std::string& loanCsvFilename, const etrading::LabelValueBlock& csvColumnNames, const bool& includeCashflowColumnHeaders )
	{
		VALID_EXCEPTION_START

		const std::vector<etrading::LoanCashflowBreakdown> cashflowBreakdown = tryMeSRTAmortizeLoanPortfolio( loanCsvFilename, csvColumnNames );
		const LAStringMatrix cashflowMatrix = cashflowBreakdownToMatrix( cashflowBreakdown, includeCashflowColumnHeaders );

		return cashflowMatrix;

		VALID_EXCEPTION_END
	}

	/* @brief CashflowModel: This function projects the input loan-cashflows through a capital structure
	*		  Note: This is the R-library exposed function.
	*  @param[in]	trancheStructure	This vector describes the tranches that make up the capital structure
	*  @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
	*  @param[in]	elgd				The exposure-weighted average loss given default: The share of an asset that is lost if a borrower defaults
	*  @param[in]	trancheMaturity		The tranches' remaining effective maturity in years.
	*  @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
	*  @param[in]	supervisoryType		The regulatory rules to follow: LA or IRBA ( internal ratings-based approach )
	*  @param[in]	poolType			The type of the pool ("Wholesale" or "Retail")
	*/
	std::vector<etrading::TrancheCashflow> tryMeSRTCalculateCashflow( const std::vector<etrading::LoanCashflowBreakdown>& loanCashflows,
																	  const std::vector<etrading::TrancheDefinition>& trancheStructure,
																	  const double& kirb,
																	  const double& elgd,
																	  const double& trancheMaturity,
																	  const int& nEffectiveExposures,
																	  const std::string& supervisoryType,
																	  const std::string& poolType )
	{
		VALID_EXCEPTION_START

		const etrading::SupervisoryTypeEnum supervisoryTypeEnum = etrading::toSupervisoryTypeEnum( supervisoryType );
		const etrading::PoolTypeEnum poolTypeEnum = etrading::toPoolTypeEnum( poolType );
		return etrading::cashflowModel( loanCashflows, trancheStructure, kirb, elgd, trancheMaturity, nEffectiveExposures, supervisoryTypeEnum, poolTypeEnum );

		VALID_EXCEPTION_END
	}

}

