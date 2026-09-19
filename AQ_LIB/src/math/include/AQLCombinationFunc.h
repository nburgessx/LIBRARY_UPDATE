#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// ID for AQLCombinationMethod
#define FN_COMBINATIONFUNC	1204 
// Function name for AQLCombinationMethod
#define FN_COMBINATIONFUNC_STR	"fn_combinationfunc" 


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent function sums and products

    This class derives from AQLFunctionBase

*/
class AQLCombinationMethod : public AQLFunctionBase
{
public:
	// constructor
	explicit AQLCombinationMethod(bool checkflag = false);
	// copy constructor
	AQLCombinationMethod(const AQLCombinationMethod& v);
	// destructor
	virtual ~AQLCombinationMethod();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;


	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return function value
	virtual double				operator()(double x) const;
	                            //==========================================
	                            // operator definition of "+"
	virtual AQLCombinationMethod	operator+(const AQLFunctionBase& method) const;
	                            //==========================================
	                            // operator definition of "+"
	virtual AQLCombinationMethod	operator+(double a) const;
	                            //==========================================
	                            // operator definition of "*"
	virtual AQLCombinationMethod	operator*(const AQLFunctionBase& method) const;
	                            //==========================================
	                            // operator definition of "*"
	virtual AQLCombinationMethod	operator*(double a) const;

	// 20060929 override a virtual function of the base class
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};

	                            //==========================================
	                            // return partial derivative result
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, DIFF_TYPE difftype = BOTHSIDE,double delta = 0.0001) const;

	                            //==========================================
	                            // return second partial derivative result
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype = DEFAULT, double delta = 0.0001) const;
	                            //==========================================
								// check x is in domain of this function or not.
	virtual	bool				isInDomain(const DoubleArray& x)const;
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos)const;
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi,unsigned int posj)const;


								//==========================================
	                            // set check flag
	virtual void				setCheckFlag(bool checkflag = true);

	// equal operator
	AQLCombinationMethod & operator = (const AQLCombinationMethod& v);


private:
protected:
friend class AQLFunctionBase;
	// vector of coefficient and function for constitution
	std::vector<std::pair<double,std::vector<AQLFunctionBase*> > > mFuncs;
	double mConst;	// constant part of function

};

