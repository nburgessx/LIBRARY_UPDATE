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
		ret = validation::tryMirClearEntityPool().getCString();
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	return ret;
}

