// aqBondObjectCreate.h

/* 
 * @brief			Swig interface for aqBondObjectCreate... functions
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for aqBondObjectCreate function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		bondLVB	            Bond Definition Label Value Block (LVB) - A key-value matrix representing the bond definition
*  @param [in]		scheduleLVB     	Schedule Definition Label Value Block (LVB) - A key-value matrix representing the schedule definition
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the bond object handle name
*/
const std::string aqBondObjectCreate( const std::string& bondObjectName,
                                   const SWIG_STRINGMATRIX & bondLVB,
                                   const SWIG_STRINGMATRIX & scheduleLVB,
                                   const bool validateKeys );

/* @brief			swig interface for aqBondObjectCreateFromGenerator function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		bondGeneratorName   Bond Generator Name
*  @param [in]		bondExpressionLVB	Bond Expression Label Value Block (LVB) - A key-value matrix representing the bond definition
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the bond object handle name
*/
const std::string aqBondObjectCreateFromGenerator( const std::string& bondObjectName,
                                                const std::string& bondGeneratorName,
                                                const SWIG_STRINGMATRIX & bondExpressionLVB,
                                                const bool validateKeys );

/* @brief			swig interface for aqBondObjectCreateFromLVB function
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		bondLVB	            Bond Definition Label Value Block (LVB), single key/value matrix
*  @param [in]		validateKeys	    Validate Bond LVB keys: true or false - true will throw if a key is misspecified
*  @param [out]		Returns the bond object handle name
*/
const std::string aqBondObjectCreateFromLVB( const std::string& bondObjectName,
                                          const SWIG_STRINGMATRIX & bondLVB,
                                          const bool validateKeys );

/* @brief			swig interface for aqBondObjectCreateAUDNotionalBond function. Creates the notional bond underlying an AUD bond future.
*  @param [in]		bondObjectName		Bond Object Name
*  @param [in]		settleDate			Settle date
*  @param [in]		maturityDate		Maturity date or tenor
*  @param [in]		couponRate			Notional bond coupon rate
*  @param [in]		payReceive			Pay/receive flag
*  @param [in]		calendar			Calendar
*  @param [in]		frequency			Coupon frequency
*  @param [in]		dayCount			Day count
*  @param [in]		bondQuoteConvention	Bond quote convention
*  @param [out]		Returns the bond object handle name
*/
const std::string aqBondObjectCreateAUDNotionalBond( const std::string& bondObjectName,
                                                  const std::string& settleDate,
                                                  const std::string& maturityDate,
                                                  const std::string& couponRate,
                                                  const std::string& payReceive,
                                                  const std::string& calendar,
                                                  const std::string& frequency,
                                                  const std::string& dayCount,
                                                  const std::string& bondQuoteConvention );

/* @brief			swig interface for aqBondObjectDisplay function, displays the INPUT parameters of the cached bond
*  @param [in]		bondObjectName		Bond Object Name
*  @param [out]		Bond display of the input parameters
*/
SWIG_STRINGMATRIX aqBondObjectDisplay( const std::string& bondObjectName );

/* @brief			swig interface for aqBondGeneratorCreate function. Creates a bond generator (static-data template) from one or two named data blocks.
*  @param [in]		bondGeneratorName	Bond Generator Name
*  @param [in]		key1				Name of the first data block
*  @param [in]		dataBlock1			First data block
*  @param [in]		key2				Optional. Name of the second data block; pass empty string / empty matrix to omit
*  @param [in]		dataBlock2			Optional. Second data block
*  @param [out]		Returns the bond generator object handle name
*/
std::string aqBondGeneratorCreate( const std::string& bondGeneratorName,
                                    const std::string& key1,
                                    const std::vector<std::vector<std::string> >& dataBlock1,
                                    const std::string& key2,
                                    const std::vector<std::vector<std::string> >& dataBlock2 );

/* @brief			swig interface for aqBondGeneratorDisplay function
*  @param [in]		bondGeneratorName	Bond Generator Name
*  @param [in]		propertyName		The configuration block to display; blank returns all blocks
*  @param [out]		Bond generator display of the requested block(s)
*/
SWIG_STRINGMATRIX aqBondGeneratorDisplay( const std::string& bondGeneratorName, const std::string& propertyName );
