#pragma once
#include <string>
#include <vector>

#include "SwigTypes.h"

/* @brief			swig interface for aqGeneratorList function
*  @param [in]		typeAsString	    One of SWAP_GENERATOR / BOND_GENERATOR / CURVE_GENERATOR
*  @return			the generator names found on disk for that type
*/
std::vector<std::string> aqGeneratorList( const std::string& typeAsString );

/* @brief			swig interface for aqGeneratorDisplay function
*  @param [in]		typeAsString	    One of SWAP_GENERATOR / BOND_GENERATOR / CURVE_GENERATOR
*  @param [in]		generatorName	    The generator name
*  @return			the generator's properties
*/
SWIG_STRINGMATRIX aqGeneratorDisplay( const std::string& typeAsString, const std::string& generatorName );

/* @brief			swig interface for aqGeneratorValidate function
*  @param [in]		typeAsString	    One of SWAP_GENERATOR / BOND_GENERATOR / CURVE_GENERATOR
*  @param [in]		generatorName	    The generator name
*  @return			"OK", or the reason the generator failed to load
*/
std::string aqGeneratorValidate( const std::string& typeAsString, const std::string& generatorName );
