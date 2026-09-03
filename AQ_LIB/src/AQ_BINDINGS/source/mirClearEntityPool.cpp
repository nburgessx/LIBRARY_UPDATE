/* 
 * @brief			Swig interface to Java for mirClearEntityPool function
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "mirClearEntityPool.h"
#include "tryMirClearEntityPool.h"

/* @brief			swig interface for the mirClearEntityPool method
*  @return			A notification string
*/
std::string mirClearEntityPool()
{
	std::string ret("");
	try 
	{
		ret = validation_api::tryMirClearEntityPool().getCString();
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	return ret;
}

