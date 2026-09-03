#include "mirGetNextECBDate.h"
#include "tryMirGetNextECBDate.h"


/* @brief			swig interface for the mirGetNextECBDate method
*  @param [in]		baseDate		Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string mirGetNextECBDate(const std::string& baseDate,
							bool strictlyAfter)
{

	std::string ret("");
	try 
	{
		// Input marshalling
		AQLDate baseDt(baseDate.c_str(),"YYYYMMDD");

		AQLDate date = validation::tryMirGetNextECBDate(baseDt, strictlyAfter);
		
		ret = date.stringWithFormat("YYYYMMDD").getCString();

	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}