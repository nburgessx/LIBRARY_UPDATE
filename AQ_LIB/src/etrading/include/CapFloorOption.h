/*
* @brief			Class the defines the CapFloor Option
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/
#pragma once

#include "LACoreTemplateType.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "LabelValueBlock.h"
#include "Option.h"
#include "FloatLeg.h"

namespace etrading
{
	class CapFloorOption : public Option
	{
	public:
		CapFloorOption(const std::string& objectName, const LabelValueBlock& dealLVB);
		CapFloorOption(const CapFloorOption& rhs);
		virtual ~CapFloorOption() {};

		//Override
		std::shared_ptr<Option> clone();

	};

}


