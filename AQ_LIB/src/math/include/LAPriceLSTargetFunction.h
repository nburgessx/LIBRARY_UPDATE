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

#define FN_LSTARGETFUNCTION	1250
#define FN_LSTARGETFUNCTION_STR	"fn_lstargetfunction"

class LAMathObjectValue;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class that calculates the sum of square error.

*/
template<class T>
class LAPriceLSTargetFunction : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAPriceLSTargetFunction();
	// destructor
	virtual ~LAPriceLSTargetFunction();


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

	// 20060929 override a virtual function of the base class
	virtual double				operator()(double x) const
								{
									return LAFunctionBase::operator()(x);
								};
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};

//  OPERATION
	/*!
		@brief set two lines of valuation functions
		@param[in] target vector of pointer pair of corresponding valuation object 
	*/
	void						set(const std::vector<std::pair<LAMathObjectValue*, LAMathObjectValue*> >& target)
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
	std::vector<std::pair<LAMathObjectValue*, LAMathObjectValue*> >	mTarget;

	// pointer to changing paramters
	T* mpVal;

	// Base Date
	LADate mBaseDate;

protected:
	

};


//================ LAPriceLSTargetFunction ===================================
/*!
	@brief default constructor
*/
template<class T>
LAPriceLSTargetFunction<T>::LAPriceLSTargetFunction() 
: LAFunctionBase(), mpVal(NULL)
{
}


/*!
	@brief destructor
*/
template<class T>
LAPriceLSTargetFunction<T>::~LAPriceLSTargetFunction() 
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
template<class T>
bool
LAPriceLSTargetFunction<T>::isTypeOf(function_t id) const
{
	return (id == FN_LSTARGETFUNCTION ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
template<class T>
LACoreFunctionBase*	
LAPriceLSTargetFunction<T>::clone() const
{
    try 
	{
		return new LAPriceLSTargetFunction<T>(*this);
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
LAPriceLSTargetFunction<T>::getType() const
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
LAPriceLSTargetFunction<T>::operator()(const DoubleArray& x) const
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
