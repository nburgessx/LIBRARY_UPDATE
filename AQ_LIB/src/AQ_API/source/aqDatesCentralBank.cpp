#include "aqDatesCentralBank.h"
#include "tryAqDatesCentralBank.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"

/* @brief			swig interface for the aqDatesCentralBank method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		date			A base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string aqDatesCentralBank(const std::string& centralBankId, 
							const std::string& date,
							bool strictlyAfter)
{
    AQ_API_START
	std::string ret("");

    // Input marshalling
	AQLString cbId(centralBankId.c_str());
	AQLDate baseDt( etrading::stringToDate( date ) );

	AQLDate date = validation::tryAqDatesCentralBank(cbId, baseDt, strictlyAfter);
		
	ret = date.stringWithFormat("YYYYMMDD").getCString();
	
	return ret;
    AQ_API_END
}

/* @brief			swig interface for the aqDatesECB method
*  @param [in]		date			Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string aqDatesECB(const std::string& date, bool strictlyAfter)
{
    AQ_API_START
	std::string ret("");
	
	// Input marshalling
	AQLDate baseDt( etrading::stringToDate( date ) );

	AQLDate date = validation::tryAqDatesECB(baseDt, strictlyAfter);
		
	ret = date.stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interfac for the aqDatesECBSwapStart method
*  @param [in]		date	An input date
*  @return			The start date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDatesECBSwapStart(const std::string& date)
{
    AQ_API_START
	std::string ret("");

    // Input marshalling
	AQLDate tmp_date( etrading::stringToDate( date ) );
    AQLDate date = validation::tryAqDatesECBSwapStart(tmp_date);
	
    ret = date.stringWithFormat("YYYYMMDD").getCString();
    return ret;
    AQ_API_END
}

/* @brief			swig interfac for the aqDatesECBSwapEnd method
*  @param [in]		date	An input date
*  @return			The end date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDatesECBSwapEnd(const std::string& date)
{
    AQ_API_START
	std::string ret("");

    // Input marshalling
	AQLDate tmp_date( etrading::stringToDate( date ) );
    AQLDate date = validation::tryAqDatesECBSwapEnd(tmp_date);

	ret = date.stringWithFormat("YYYYMMDD").getCString();
    return ret;
    AQ_API_END
}
