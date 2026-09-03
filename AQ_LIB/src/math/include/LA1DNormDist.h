#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// Funciton ID of LA1DNormDist
#define FN_1DNORMDIST	1205
// Function Name of LA1DNormDist
#define FN_1DNORMDIST_STR	"fn_1dnormdist"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class to represent normal distribution function

    This class derives from MMFunctioinBase

*/
class LA1DNormDist : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LA1DNormDist();
	// constructor
	LA1DNormDist(double myu, double sigma);
	// destructor
	virtual ~LA1DNormDist();

//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	                            //==========================================
								// return distribution density
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return distribution density
	virtual double				operator()(double x) const;

	LA1DNormDist & operator=( const LA1DNormDist & ) { return *this; }

	// 20060929 override a virtual function of the base class
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};

	                            //==========================================
								// return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;
	                            //==========================================
								// return integral result
	virtual double				integral(double xl, double xu) const;
	
	// 20060929 override a virtual function of the base class
	virtual double				integral(const std::vector<std::pair<double,double> >& x,
										const LAIntegralBase* pIntegral) const
								{
									return LAFunctionBase::integral(x, pIntegral);
								};
	virtual double				integral(double xl, double xu,
										const LA1DIntegral* pIntegral) const
								{
									return LAFunctionBase::integral(xl, xu, pIntegral);
								};

	                            //==========================================
								// return partial derivative value
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, DIFF_TYPE difftype = BOTHSIDE,double delta = 0.0001) const;

	                            //==========================================
								// return second partial deribative value
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi,unsigned int posj,
												CALC_TYPE calctype = DEFAULT, double delta = 0.0001) const;

	                            //==========================================
								// check x is in domain of this function or not.
	virtual	bool				isInDomain(const DoubleArray& x) const { (void)x; return true;};
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos)const { (void)x; (void)pos; return true;};
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj)const { (void)x; (void)posi; (void)posj; return true;};


//  OPERATION
	                            //==========================================
								// set distribution parameter
	virtual void				setParam(const DoubleArray& param);

private:

protected:
	

};
