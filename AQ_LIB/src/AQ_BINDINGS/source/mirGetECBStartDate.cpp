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
		LADate ecbDt(ecbDate.c_str(),"YYYYMMDD");

		LADate date = validation_api::tryMirGetECBStartDate(ecbDt);
		ret = date.stringWithFormat("YYYYMMDD").getCString();

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}