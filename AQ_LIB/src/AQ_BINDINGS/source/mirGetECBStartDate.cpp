/* 
 * @brief			Swig interface to Java for mirGetECBStartDate methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

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