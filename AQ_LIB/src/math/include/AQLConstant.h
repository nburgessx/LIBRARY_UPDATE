#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of AQLConstant
#define FN_CONSTANT	1202
// Function Name of AQLConstant
#define FN_CONSTANT_STR	"fn_constant"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent constant function

    This class derives from AQLFunctionBase

*/
class AQLConstant : public AQLFunctionBase
{
public:
	// constructor
	AQLConstant();
	// constructor
	AQLConstant(double x);
	// destructor
	virtual ~AQLConstant();

	AQLConstant & operator=( const AQLConstant & ) { return *this; }

								//======================================
								// Check function for this class type
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
								// return function value
	virtual double				operator()(double x) const;

	// 20060929 override a virtual function of the base class
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};

	                            //==========================================
								// return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;

	                            //==========================================
								// return integral result
	virtual double				integral(double xl, double xu) const;
	
	// 20060929 override a virtual function of the base class
	virtual double				integral(const std::vector<std::pair<double,double> >& x,
										const AQLIntegralBase* pIntegral) const {
									return AQLFunctionBase::integral(x, pIntegral);
								};
	virtual double				integral(double xl, double xu,
										const AQL1DIntegral* pIntegral) const {
									return AQLFunctionBase::integral(xl, xu, pIntegral);
								};

	                            //==========================================
								// return partial derivative value
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, DIFF_TYPE difftype = BOTHSIDE,double delta = 0.0001) const;

	                            //==========================================
								// return second partial deribative value
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype = DEFAULT, double delta = 0.0001) const;


								//==========================================
								// check x is in domain of this function or not.
	virtual	bool				isInDomain(const DoubleArray& x)const { (void)x; return true;};
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos)const { (void)x; (void)pos;return true;};
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi,unsigned int posj)const { (void)x; (void)posi; (void)posj; return true;};


								//==========================================
	                            // set value	
	virtual void				set(double x);
								//==========================================
	                            // set parameters	
	virtual void				setParam(const DoubleArray& param);
								//==========================================
	                            // Assinment operator
	AQLConstant&					operator=(double x);

private:

protected:
	

};

