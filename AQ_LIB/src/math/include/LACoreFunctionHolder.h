#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreFunctionBase.h"
#include <cstdio>

/*! 
    @brief Class to hold virtual base class for valuation, intepolation, procedure etc.

    This class is used to register Function class.
	If mDeleteFlag is true, it has ownership of Function object, and the default setting is true.
*/
class LACoreFunctionHolder 
{
public:
//	LIFECYCLE
    // default constructor
	explicit LACoreFunctionHolder(const LACoreFunctionBase* e = NULL, bool deleteFlag = false);
    // copy constructor
	LACoreFunctionHolder(const LACoreFunctionHolder& objectHolder);
    // destructor
	virtual ~LACoreFunctionHolder();

//  QUERY
   // return whether or not that holds the Function object
    /*!
        @retval true hold the Function object
        @retval false not hold the Function object
    */
	bool				isDefined() const {return mpFunction != NULL;}

	// return the delete flag of the Function object
    /*!
        @retval true hold ownership of the Function
        @retval false not hold ownership of the Function
    */
 	bool				isDeleteFlag() const {return mDeleteFlag;}

    // return Function object(behavior is undetermined when Function object is not defined)
    /*!
        @return Function
    */
	const LACoreFunctionBase& get(void) const {return *mpFunction;}

//  OPERATION
	// set Function object(copy the pointer to Function object)
	void                set(const LACoreFunctionBase* base, bool df=false);

    // set the delete flag of the Function object
	/*!
        @param[in] df delete flag (true or false)
    */
	void				setDeleteFlag(bool df) {mDeleteFlag = df;}

    // assignment operator
	LACoreFunctionHolder&	operator=(const LACoreFunctionHolder& e) {return copy(e);}

    // check whether this class derives from base class with type id
	virtual bool        isTypeOf(function_t id) const
							{return mpFunction->isTypeOf(id);}

    // return Function ID this Holder has
    /*!
        @return Function ID
    */
	virtual function_t	getType() const
							{return mpFunction->getType();}

    // relational operator thorough magnitude relationship of FunctionID
    // if the function object is not defined, the behavior is undetermined.
	bool                operator==(const LACoreFunctionHolder& objectHolder) const
							{return *mpFunction == *objectHolder.mpFunction;}
    // relational operator thorough magnitude relationship of FunctionID
    // if the function object is not defined, the behavior is undetermined.
	bool                operator < (const LACoreFunctionHolder& objectHolder) const
							{return *mpFunction < *objectHolder.mpFunction;}
protected:
	// shallow copy of the specified Function Holder object
	virtual LACoreFunctionHolder&   copy(const LACoreFunctionHolder& e);

private:
	const LACoreFunctionBase*    mpFunction;  // Function object
	bool					 mDeleteFlag; // flag whether or not to able to delete Function object
};
