#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of AQL1DNormDist
#define FN_1DNORMDIST	1205
// Function Name of AQL1DNormDist
#define FN_1DNORMDIST_STR	"fn_1dnormdist"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class to represent normal distribution function

    This class derives from MMFunctioinBase

*/
class AQL1DNormDist : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQL1DNormDist();
	// constructor
	AQL1DNormDist(double myu, double sigma);
	// destructor
	virtual ~AQL1DNormDist();

//  QUERY
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
								// return distribution density
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return distribution density
	virtual double				operator()(double x) const;

	AQL1DNormDist & operator=( const AQL1DNormDist & ) { return *this; }

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
