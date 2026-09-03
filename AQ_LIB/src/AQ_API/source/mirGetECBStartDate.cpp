#include "mirGetECBStartDate.h"
#include "tryMirGetECBStartDate.h"


/* @brief			swig interfac for the mirGetECBStartDate method
*  @param [in]		ecbDate ECB(European Central Bank) date
*  @return			The start date of an ECB Swap base on the ecb date
*/
std::string mirGetECBStartDate(const std::string& ecbDate)
{

	std::string ret("");
	try 
	{
		// Input marshalling
		AQLDate ecbDt(ecbDate.c_str(),"YYYYMMDD");

		AQLDate date = validation::tryMirGetECBStartDate(ecbDt);
		ret = date.stringWithFormat("YYYYMMDD").getCString();

	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}