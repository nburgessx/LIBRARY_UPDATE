#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADate.h"
#include <new>
#include <vector>

#define FN_TARGETFUNCTION	1251
#define FN_TARGETFUNCTION_STR	"fn_targetfunction"

class LAMathObjectValue;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class that calculates the sum of square error.

*/
template<class T>
class LAPriceTargetFunction : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAPriceTargetFunction();
	// destructor
	virtual ~LAPriceTargetFunction();


//  QUERY
								//======================================
								// Check function for this class type
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;
								//======================================
								// Return this class type
	virtual function_t			getType() const;
	                            //==========================================
	                            // Calculate the summation of square error.
	virtual double				operator()(const DoubleArray& x) const;


//  OPERATION
	/*!
		@brief set pair of valuable object
		@param[in] target pointer pair of valuable object
	*/
	void						set(const std::pair<LAMathObjectValue*, LAMathObjectValue*>& target)
								{mTarget = target;}
	
	/*!
		@brief set calibration paramters
		@param[in] val reference to parameters 
	*/
	void						setVariable(T& val) {mpVal = &val;} 

	/*!
		@brief set base date
		@param[in] date base date 
	*/
	void						setBaseDate(const LADate& date){mBaseDate = date;};

private:

	// vector of pointer pair of corresponding valuation object
	std::pair<LAMathObjectValue*, LAMathObjectValue*>	mTarget;

	// pointer to changing paramters
	T* mpVal;

	// Base Date
	LADate mBaseDate;

protected:
	

};


//================ LAPriceTargetFunction ===================================
/*!
	@brief default constructor
*/
template<class T>
LAPriceTargetFunction<T>::LAPriceTargetFunction() 
: LAFunctionBase(), mpVal(NULL)
{
}


/*!
	@brief destructor
*/
template<class T>
LAPriceTargetFunction<T>::~LAPriceTargetFunction() 
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
template<class T>
bool
LAPriceTargetFunction<T>::isTypeOf(function_t id) const
{
	return (id == FN_TARGETFUNCTION ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
template<class T>
LACoreFunctionBase*	
LAPriceTargetFunction<T>::clone() const
{
    try 
	{
		return new LAPriceTargetFunction<T>(*this);
    }
	catch (std::bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief Return this function type
    @return function type
*/
template<class T>
function_t
LAPriceTargetFunction<T>::getType() const
{
	return FN_TARGETFUNCTION;
}

/*!
    @brief Calculate the difference
	@param[in] x paramters
    @return deifference between object value and target value
*/
template<class T>
double
LAPriceTargetFunction<T>::operator()(const DoubleArray& x) const
{
	(*mpVal) = x;
	mTarget.first->update();
	double val = mTarget.first->value(mBaseDate) - mTarget.second->value(mBaseDate);
   
	return val;
}

