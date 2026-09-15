#pragma once

#include "Variant.h"

#include <string>
#include <vector>

namespace validation
{
    /* @brief Lists the generator names available on disk for a generator type, scanning
     *        $(AQ)/resources/config/<type> directly - a generator need not already be
     *        loaded into the AQObj cache to be listed.
     * @param [in] typeAsString     One of SWAP_GENERATOR / BOND_GENERATOR / CURVE_GENERATOR
     * @param [out]                 The generator names found (file names, extension stripped)
     */
    std::vector<std::string> tryAqGeneratorList( const std::string& typeAsString );

    /* @brief Reads one generator's JSON file from disk and returns its properties.
     * @param [in] typeAsString     One of SWAP_GENERATOR / BOND_GENERATOR / CURVE_GENERATOR
     * @param [in] generatorName    The generator name (file name, extension optional)
     * @param [out]                 A VariantMatrix containing the generator's properties
     */
    const etrading::VariantMatrix tryAqGeneratorDisplay( const std::string& typeAsString,
                                                           const std::string& generatorName );

    /* @brief Checks that a generator's JSON file parses and satisfies its schema, by
     *        attempting to load it through the same path AQObjectLoad uses.
     * @param [in] typeAsString     One of SWAP_GENERATOR / BOND_GENERATOR / CURVE_GENERATOR
     * @param [in] generatorName    The generator name (file name, extension optional)
     * @param [out]                 "OK", or the reason the generator failed to load
     */
    std::string tryAqGeneratorValidate( const std::string& typeAsString,
                                         const std::string& generatorName );
}
