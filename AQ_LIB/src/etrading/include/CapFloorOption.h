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


