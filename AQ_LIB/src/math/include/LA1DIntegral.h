#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAIntegralBase.h"
#include "LAFunctionBase.h"


// ID for LA1DIntegral
#define FN_1DINTEGRAL     1402
// Function name for LA1DIntegral
#define FN_1DINTEGRAL_STR	"fn_1dintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class for Gaussian quadrature integral methods

    This class itself derives from abstract class "LAIntegralBase". This class has pure
	virtual method "get".

*/
class LA1DIntegral : public LAIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	LA1DIntegral();
	// Destructor
	virtual ~LA1DIntegral();

//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const=0;
								//======================================
								// Return this class type
    virtual function_t          getType() const;


	//======================================
	// method for integration, which is inherited from base class
	virtual double	integrate(const LAFunctionBase& f,
								const std::vector<std::pair<double,double> >& x) const;
	//======================================
	// method for 1-dim integration
	virtual double	integrate(const LAFunctionBase& f, double xl, double xu) const = 0;
	//======================================
	// method for 1-dim integration
	virtual double	integrate(const LAFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const;
    //======================================
    // Integrate on each interval of vector samples using segment integration
    double IntegrateBySampling(const LAFunctionBase& f, const std::vector<double>& samples) const;
    //======================================
    // Integrate on nSplits equal intervals between lower and upper bounds
    double IntegrateByEqualIntervals(const LAFunctionBase& f, double xl, double xu, int nSplits) const;

	LA1DIntegral & operator=( const LA1DIntegral & ) { return *this; }

private:
	// member class to represent inner function
	/*! 
    @brief Declaration and definition of inner class to represent inner function

	*/
	class MMInnerFunction : public LAFunctionBase
	{
	public:
		// Default constructor
		explicit MMInnerFunction(const LA1DIntegral* pIntegral);
		// Destructor
		~MMInnerFunction();
		// Copy constructor
		MMInnerFunction(const MMInnerFunction& v);
									//======================================
									// Make copy(clone) of this class
		virtual LACoreFunctionBase*		clone() const;
									//==========================================
									// Return y(=f(x))
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

									//==========================================
									// set function
		void						setFunc(const LAFunctionBase& method);
									//==========================================
									// set integral region
		void						setIntegralRegion(double xl, double xu) {mXl=xl; mXu=xu;};

		MMInnerFunction & operator=( const MMInnerFunction & ) { return *this; }

	private:
		LAFunctionBase* mpFunc;				// pointer to function
		double mXl;							// lower side of intergral region
		double mXu;							// upper side of integral region
		const LA1DIntegral*		mpIntegral;	// pointer to integral method
	};
protected:

};

