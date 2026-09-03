#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


class LAPriceIndexToolBase;
class LAPriceEventBase;
class LAPriceEventHolder;
class AQLDate;
class AQLObject;
class AQLFunctionBase;
class LAPricePayOff;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of tool class of trigger.
*/
class LAPriceEventTool
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceEventTool();
    // destructor	
	virtual ~LAPriceEventTool();

	// get trigger action
	virtual	const std::vector<LAPriceEventHolder*>&	
								getAction() {return mAction;} 
								
	
	// set up this class
	virtual	void				setUp(const AQLDate& basedate, 
										const AQLObject& trade,
										AQLObject& trigger,
										const LAPricePayOff& payoff,
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
	std::vector<std::vector<LAPriceIndexToolBase*> >		mIndex;// trigger refenrence index
	std::vector<LAPriceEventBase*> mActionMaster;// trigger action master
	std::vector<LAPriceEventHolder*> mAction;// trigger actions


};

