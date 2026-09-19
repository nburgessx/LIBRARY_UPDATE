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

#define FN_LSTARGETFUNCTION	1250
#define FN_LSTARGETFUNCTION_STR	"fn_lstargetfunction"

class AQLMathObjectValue;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class that calculates the sum of square error.

*/
template<class T>
class AQLPriceLSTargetFunction : public AQLFunctionBase
{
public:
	// constructor
	AQLPriceLSTargetFunction();
	// destructor
	virtual ~AQLPriceLSTargetFunction();


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

	// 20060929 override a virtual function of the base class
	virtual double				operator()(double x) const
								{
									return AQLFunctionBase::operator()(x);
								};
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};

	/*!
		@brief set two lines of valuation functions
		@param[in] target vector of pointer pair of corresponding valuation object 
	*/
	void						set(const std::vector<std::pair<AQLMathObjectValue*, AQLMathObjectValue*> >& target)
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
	std::vector<std::pair<AQLMathObjectValue*, AQLMathObjectValue*> >	mTarget;

	// pointer to changing paramters
	T* mpVal;

	// Base Date
	AQLDate mBaseDate;

protected:
	

};


//================ AQLPriceLSTargetFunction ===================================
/*!
	@brief default constructor
*/
template<class T>
AQLPriceLSTargetFunction<T>::AQLPriceLSTargetFunction() 
: AQLFunctionBase(), mpVal(NULL)
{
}


/*!
	@brief destructor
*/
template<class T>
AQLPriceLSTargetFunction<T>::~AQLPriceLSTargetFunction() 
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
template<class T>
bool
AQLPriceLSTargetFunction<T>::isTypeOf(function_t id) const
{
	return (id == FN_LSTARGETFUNCTION ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
template<class T>
AQLCoreFunctionBase*	
AQLPriceLSTargetFunction<T>::clone() const
{
    try 
	{
		return new AQLPriceLSTargetFunction<T>(*this);
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
AQLPriceLSTargetFunction<T>::getType() const
{
	return FN_LSTARGETFUNCTION;
}

/*!
    @brief Calculate the summation of square error
	@param[in] x paramters
    @return summation of square error
*/
template<class T>
double
AQLPriceLSTargetFunction<T>::operator()(const DoubleArray& x) const
{
	(*mpVal) = x;
	double ret=0.0;
	for(unsigned int i = 0; i < mTarget.size(); i++)
	{
		mTarget[i].first->update();
		double val = mTarget[i].first->value(mBaseDate) - mTarget[i].second->value(mBaseDate);
        ret += val*val; 
	}
	return ret;
}
