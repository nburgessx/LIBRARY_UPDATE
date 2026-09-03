#include "mirGetNextCBDate.h"
#include "tryMirGetNextCBDate.h"


/* @brief			swig interface for the mirGetNextCBDate method
*  @param [in]		centralBankId	Central bank Id, e.g. ECB, BoE, Fed
*  @param [in]		baseDate		Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next central bank meeting on/after the base date
*/
std::string mirGetNextCBDate(const std::string& centralBankId, 
							const std::string& baseDate,
							bool strictlyAfter)
{

	std::string ret("");
	try 
	{
		// Input marshalling
		LAString cbId(centralBankId.c_str());
		LADate baseDt(baseDate.c_str(),"YYYYMMDD");

		LADate date = validation_api::tryMirGetNextCBDate(cbId, baseDt, strictlyAfter);
		
		ret = date.stringWithFormat("YYYYMMDD").getCString();

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}