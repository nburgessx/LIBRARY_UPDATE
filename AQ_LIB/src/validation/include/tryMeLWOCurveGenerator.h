/*
 * @brief			validation interface for the meLWOCurveGenerator method
 * @Created:		24 Nov 2016
 * @Author:			Ian Castleton
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "Variant.h"
#include "LabelValueBlock.h"

#include <string>
#include <vector>


namespace validation_api
{
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

    /* @brief Creates a LWOCurveGenerator object, containing all of the curve properties.
	 * @param [in] objectName        The name of the Curve Configuration object
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryMeLWOCurveGeneratorCreate( const std::string& objectName,
                                              const std::vector<std::string>& propertyNames,
                                              const std::vector<TableInfo>& infoBlocks );

	/* @brief Displays the specified property of a LWOCurveGenerator. If propertyName is blank, all properties are returned.
	 * @param [in] objectName        The CurveGenerator object you wish to display
	 * @param [in] propertyName      The name of the property label-value block that you wish to display
	 * @param [out]                  A VariantMatrix containing a LabelValue block of properties
	 */
    const etrading::VariantMatrix  tryMeLWOCurveGeneratorDisplay( const std::string& objectName,
																  const std::string& propertyName );

	/* @brief Builds a new CurveGenerator from an existing base CurveGenerator, with modified properties.
	*  @param[in] newObjectName		The name to use for the new CurveGenerator object
	*  @param[in] baseObjectName	The name of the existing CurveGenerator object on which to base the new object
	*  @param[in] modifiedValues	A LabelValueBlock containing key/value pairs to update. Each key is specified in the format: PROPERTYNAME:KEYNAME.
	*/
	std::string tryMeLWOCurveGeneratorModify( const std::string& newObjectName,
											  const std::string& baseObjectName,
                                              const etrading::LabelValueBlock& modifiedValues );

     /* @brief Displays the specified property of an LWOCurveGenerator. If propertyName is blank, all properties are returned.
	 * @param [in] objectName        The CurveGenerator object you wish to display
	 * @param [in] propertyName      The name of the property label-value block that you wish to display.
	 * @param [out]                  A VariantMatrix containing a LabelValue block of properties.
	 */
	const etrading::VariantMatrix tryMeLWOCurveDisplayConventions(const std::string& objectName, const std::string& propertyKey );
}