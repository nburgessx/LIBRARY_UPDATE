#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLObject.h"
#include "AQLDataInstance.h"
#include "AQLCoreValuation.h"


#define ENTITY_VENTITY  2

class AQLDataHolder;
class AQLDataString;
class AQLDataDate;


/*! 
    @brief Class to represent the Object with the evaluation, such as pricing.

    This class has Data(AQLDataValuation) as member variables
	to reprecent valuefunction such as pricing derived from AQLObject class.

	AQLMathObjectValue set Data name automatically as follows <br>
	1. "Name"<BR>
    2. "Value

*/
class AQLMathObjectValue : public AQLObject
{
public:
//	LIFECYCLE
    // default constructor
	AQLMathObjectValue(AQLDataInstance* dataInstance);
    // copy constructor
	AQLMathObjectValue(const AQLMathObjectValue& ve);
    // destructor
	virtual ~AQLMathObjectValue();

// QUERY

    // return object id
	virtual object_t	getType(void) const;

    // find out whether the object or the inherited object matches a specified object id or not
	virtual bool		isTypeOf(object_t id) const;

    // deep copy of this object
	virtual AQLObject*	clone() const;


    // get valuable function name
	const AQLDataString&	getName() const;
    // get valuable function name
	AQLDataString&		getName();

    // find out whether the function class that this class has or the inherited class matches a specified function id or not
	const bool          isMethodTypeOf(function_t id) const;

    // return Function ID this Holder has
    function_t			getMethodType(void) const;

// OPERATION
    // register the Data into the Object
    AQLDataHolder&		add(const AQLString& name);

	// return the result of evaluation of the Object
	double				value(const AQLDate& baseDate);

	// return the result of foreced re-evaluation of the Object
	double				revalue(void);

    // set function name and Function class to implement the valuation function
    virtual void		setValuationMethod(const AQLCoreValuation* a, const AQLString& name);	

    // set function name and Function class to implement the valuation function
    virtual void		setValuationMethod(const AQLString& name);

    // delete Data
    virtual void        remove(const AQLString& dataName);
	
	// Initialize this Object.
	virtual void		reset(void);

protected:
    // shallow copy of the object
    virtual AQLObject&   copy(const AQLObject& e);

private:
	AQLDataHolder*	mpName;   // valuable function name
	AQLDataHolder*	mpMethod; // pointer to Function class

	// set error message
	void			setErrMsgAttr(AQLCoreError &err);
};
