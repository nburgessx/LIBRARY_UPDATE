#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"

// Funciton ID of AQLRatesCpnCapFunc
#define FN_CPNCAP	1218
// Function Name of AQLRatesCpnCapFunc
#define FN_CPNCAP_STR	"fn_cpncap"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent Coupon Cap function.

	This class derives from AQLFunctionBase

*/
class AQLRatesCpnCapFunc : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLRatesCpnCapFunc();
	// destructor
	virtual ~AQLRatesCpnCapFunc();

	//20061017--David--Remove warning:C4512
	AQLRatesCpnCapFunc & operator=( const AQLRatesCpnCapFunc & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return AQLFunctionBase::operator()(x);								
								}


	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};

	                            //==========================================
								// return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const
								{
									return AQLFunctionBase::integral(x);
								}

	                            //==========================================
								// return integral result
	virtual double				integral(double xl, double xu) const
								{
									return AQLFunctionBase::integral(xl, xu);
								}
	
	virtual double				integral(const std::vector<std::pair<double,double> >& x,
										const AQLIntegralBase* pIntegral) const
								{
									return AQLFunctionBase::integral(x, pIntegral);
								};
	virtual double				integral(double xl, double xu,
										const AQL1DIntegral* pIntegral) const
								{
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
	virtual	bool				isInDomain(const DoubleArray& x) const { (void)x; //20061017--David--Remove warning:C4100 20070411--Nagase--gcc
												return true;};
								//==========================================
	                            // check derivable or not.
                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos) const { (void)x; (void)pos; //20061017--David--Remove warning:C4100 20070411--Nagase--gcc
												return true;};
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) const { (void)x; (void)posi; (void)posj; //20061017--David--Remove warning:C4100 20070411--Nagase--gcc
												return true;};
//  OPERATION

private:

protected:
	

};

