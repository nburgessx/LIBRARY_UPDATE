#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include <new>
#include <vector>

#define FN_TARGETFUNCTION	1251
#define FN_TARGETFUNCTION_STR	"fn_targetfunction"

class AQLMathObjectValue;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class that calculates the sum of square error.

*/
template<class T>
class AQLPriceTargetFunction : public AQLFunctionBase
{
public:
	// constructor
	AQLPriceTargetFunction();
	// destructor
	virtual ~AQLPriceTargetFunction();


								//======================================
								// Check function for this class type
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;
								//======================================
								// Return this class type
	virtual function_t			getType() const;
	                            //==========================================
	                            // Calculate the summation of square error.
	virtual double				operator()(const DoubleArray& x) const;


	/*!
		@brief set pair of valuable object
		@param[in] target pointer pair of valuable object
	*/
	void						set(const std::pair<AQLMathObjectValue*, AQLMathObjectValue*>& target)
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
	void						setBaseDate(const AQLDate& date){mBaseDate = date;};

private:

	// vector of pointer pair of corresponding valuation object
	std::pair<AQLMathObjectValue*, AQLMathObjectValue*>	mTarget;

	// pointer to changing paramters
	T* mpVal;

	// Base Date
	AQLDate mBaseDate;

protected:
	

};


//================ AQLPriceTargetFunction ===================================
/*!
	@brief default constructor
*/
template<class T>
AQLPriceTargetFunction<T>::AQLPriceTargetFunction() 
: AQLFunctionBase(), mpVal(NULL)
{
}


/*!
	@brief destructor
*/
template<class T>
AQLPriceTargetFunction<T>::~AQLPriceTargetFunction() 
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
template<class T>
bool
AQLPriceTargetFunction<T>::isTypeOf(function_t id) const
{
	return (id == FN_TARGETFUNCTION ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
template<class T>
AQLCoreFunctionBase*	
AQLPriceTargetFunction<T>::clone() const
{
    try 
	{
		return new AQLPriceTargetFunction<T>(*this);
    }
	catch (std::bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief Return this function type
    @return function type
*/
template<class T>
function_t
AQLPriceTargetFunction<T>::getType() const
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
AQLPriceTargetFunction<T>::operator()(const DoubleArray& x) const
{
	(*mpVal) = x;
	mTarget.first->update();
	double val = mTarget.first->value(mBaseDate) - mTarget.second->value(mBaseDate);
   
	return val;
}

