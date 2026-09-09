#include "aqDateCentralBank.h"
#include "tryAqDateCentralBank.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"

/* @brief			swig interface for the aqDateCentralBank method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		date			A base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string aqDateCentralBank(const std::string& centralBankId, 
							const std::string& date,
							bool strictlyAfter)
{
    AQ_API_START
	std::string ret("");

    // Input marshalling
	AQLString cbId(centralBankId.c_str());
	AQLDate baseDt( etrading::stringToDate( date ) );

	AQLDate date = validation::tryAqDateCentralBank(cbId, baseDt, strictlyAfter);
		
	ret = date.stringWithFormat("YYYYMMDD").getCString();
	
	return ret;
    AQ_API_END
}

/* @brief			swig interface for the aqDateECB method
*  @param [in]		date			Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string aqDateECB(const std::string& date, bool strictlyAfter)
{
    AQ_API_START
	std::string ret("");
	
	// Input marshalling
	AQLDate baseDt( etrading::stringToDate( date ) );

	AQLDate date = validation::tryAqDateECB(baseDt, strictlyAfter);
		
	ret = date.stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interfac for the aqDateECBSwapStart method
*  @param [in]		date	An input date
*  @return			The start date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDateECBSwapStart(const std::string& date)
{
    AQ_API_START
	std::string ret("");

    // Input marshalling
	AQLDate tmp_date( etrading::stringToDate( date ) );
    AQLDate date = validation::tryAqDateECBSwapStart(tmp_date);
	
    ret = date.stringWithFormat("YYYYMMDD").getCString();
    return ret;
    AQ_API_END
}

/* @brief			swig interfac for the aqDateECBSwapEnd method
*  @param [in]		date	An input date
*  @return			The end date of an ECB(European Central Bank) Swap based on the given date
*/
std::string aqDateECBSwapEnd(const std::string& date)
{
    AQ_API_START
	std::string ret("");

    // Input marshalling
	AQLDate tmp_date( etrading::stringToDate( date ) );
    AQLDate date = validation::tryAqDateECBSwapEnd(tmp_date);

	ret = date.stringWithFormat("YYYYMMDD").getCString();
    return ret;
    AQ_API_END
}
