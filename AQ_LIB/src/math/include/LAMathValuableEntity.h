#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAObject.h"
#include "LADataInstance.h"
#include "LACoreValuation.h"


#define ENTITY_VENTITY  2

class LADataHolder;
class LADataString;
class LADataDate;


/*! 
    @brief Class to represent the Object with the evaluation, such as pricing.

    This class has Data(LADataValuation) as member variables
	to reprecent valuefunction such as pricing derived from LAObject class.

	LAMathObjectValue set Data name automatically as follows <br>
	1. "Name"<BR>
    2. "Value

*/
class LAMathObjectValue : public LAObject
{
public:
//	LIFECYCLE
    // default constructor
	LAMathObjectValue(LADataInstance* dataInstance);
    // copy constructor
	LAMathObjectValue(const LAMathObjectValue& ve);
    // destructor
	virtual ~LAMathObjectValue();

// QUERY

    // return object id
	virtual object_t	getType(void) const;

    // find out whether the object or the inherited object matches a specified object id or not
	virtual bool		isTypeOf(object_t id) const;

    // deep copy of this object
	virtual LAObject*	clone() const;


    // get valuable function name
	const LADataString&	getName() const;
    // get valuable function name
	LADataString&		getName();

    // find out whether the function class that this class has or the inherited class matches a specified function id or not
	const bool          isMethodTypeOf(function_t id) const;

    // return Function ID this Holder has
    function_t			getMethodType(void) const;

// OPERATION
    // register the Data into the Object
    LADataHolder&		add(const LAString& name);

	// return the result of evaluation of the Object
	double				value(const LADate& baseDate);

	// return the result of foreced re-evaluation of the Object
	double				revalue(void);

    // set function name and Function class to implement the valuation function
    virtual void		setValuationMethod(const LACoreValuation* a, const LAString& name);	

    // set function name and Function class to implement the valuation function
    virtual void		setValuationMethod(const LAString& name);

    // delete Data
    virtual void        remove(const LAString& dataName);
	
	// Initialize this Object.
	virtual void		reset(void);

protected:
    // shallow copy of the object
    virtual LAObject&   copy(const LAObject& e);

private:
	LADataHolder*	mpName;   // valuable function name
	LADataHolder*	mpMethod; // pointer to Function class

	// set error message
	void			setErrMsgAttr(LACoreError &err);
};
