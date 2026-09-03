#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


class AQLPriceIndexToolBase;
class AQLPriceEventBase;
class AQLPriceEventHolder;
class AQLDate;
class AQLObject;
class AQLFunctionBase;
class AQLPricePayOff;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of tool class of trigger.
*/
class AQLPriceEventTool
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceEventTool();
    // destructor	
	virtual ~AQLPriceEventTool();

	// get trigger action
	virtual	const std::vector<AQLPriceEventHolder*>&	
								getAction() {return mAction;} 
								
	
	// set up this class
	virtual	void				setUp(const AQLDate& basedate, 
										const AQLObject& trade,
										AQLObject& trigger,
										const AQLPricePayOff& payoff,
										unsigned int id,
										bool isCall = false,
										bool isLSMC = false);

	// set trigger effective
	/*!
		@brief set trigger effective
	*/
	void						setTriggerOn(void) {mEffectiveFlag = true;}
								
protected:

private:
	// clear this class members
	void						clear();
	
	bool	mEffectiveFlag;	// trigger effective flag
	AQLFunctionBase*	mpOperator;// operator of trigger reference index calculation
	std::vector<std::vector<AQLPriceIndexToolBase*> >		mIndex;// trigger refenrence index
	std::vector<AQLPriceEventBase*> mActionMaster;// trigger action master
	std::vector<AQLPriceEventHolder*> mAction;// trigger actions


};

