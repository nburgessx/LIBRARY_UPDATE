#include "meDateCentralBank.h"
#include "tryMeDateCentralBank.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros
#include "ParameterValidation.h"

/* @brief			swig interface for the meDateCentralBank method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		date			A base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string meDateCentralBank(const std::string& centralBankId, 
							const std::string& date,
							bool strictlyAfter)
{
    MLIB_API_START
	std::string ret("");

    // Input marshalling
	LAString cbId(centralBankId.c_str());
	LADate baseDt( etrading::stringToDate( date ) );

	LADate date = validation_api::tryMeDateCentralBank(cbId, baseDt, strictlyAfter);
		
	ret = date.stringWithFormat("YYYYMMDD").getCString();
	
	return ret;
    MLIB_API_END
}

/* @brief			swig interface for the meDateECB method
*  @param [in]		date			Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string meDateECB(const std::string& date, bool strictlyAfter)
{
    MLIB_API_START
	std::string ret("");
	
	// Input marshalling
	LADate baseDt( etrading::stringToDate( date ) );

	LADate date = validation_api::tryMeDateECB(baseDt, strictlyAfter);
		
	ret = date.stringWithFormat("YYYYMMDD").getCString();
	return ret;
    MLIB_API_END
}


/* @brief			swig interfac for the meDateECBSwapStart method
*  @param [in]		date	An input date
*  @return			The start date of an ECB(European Central Bank) Swap based on the given date
*/
std::string meDateECBSwapStart(const std::string& date)
{
    MLIB_API_START
	std::string ret("");

    // Input marshalling
	LADate tmp_date( etrading::stringToDate( date ) );
    LADate date = validation_api::tryMeDateECBSwapStart(tmp_date);
	
    ret = date.stringWithFormat("YYYYMMDD").getCString();
    return ret;
    MLIB_API_END
}

/* @brief			swig interfac for the meDateECBSwapEnd method
*  @param [in]		date	An input date
*  @return			The end date of an ECB(European Central Bank) Swap based on the given date
*/
std::string meDateECBSwapEnd(const std::string& date)
{
    MLIB_API_START
	std::string ret("");

    // Input marshalling
	LADate tmp_date( etrading::stringToDate( date ) );
    LADate date = validation_api::tryMeDateECBSwapEnd(tmp_date);

	ret = date.stringWithFormat("YYYYMMDD").getCString();
    return ret;
    MLIB_API_END
}
