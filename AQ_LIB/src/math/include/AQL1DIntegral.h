#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLIntegralBase.h"
#include "AQLFunctionBase.h"


// ID for AQL1DIntegral
#define FN_1DINTEGRAL     1402
// Function name for AQL1DIntegral
#define FN_1DINTEGRAL_STR	"fn_1dintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class for Gaussian quadrature integral methods

    This class itself derives from abstract class "AQLIntegralBase". This class has pure
	virtual method "get".

*/
class AQL1DIntegral : public AQLIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQL1DIntegral();
	// Destructor
	virtual ~AQL1DIntegral();

//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const=0;
								//======================================
								// Return this class type
    virtual function_t          getType() const;


	//======================================
	// method for integration, which is inherited from base class
	virtual double	integrate(const AQLFunctionBase& f,
								const std::vector<std::pair<double,double> >& x) const;
	//======================================
	// method for 1-dim integration
	virtual double	integrate(const AQLFunctionBase& f, double xl, double xu) const = 0;
	//======================================
	// method for 1-dim integration
	virtual double	integrate(const AQLFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const;
    //======================================
    // Integrate on each interval of vector samples using segment integration
    double IntegrateBySampling(const AQLFunctionBase& f, const std::vector<double>& samples) const;
    //======================================
    // Integrate on nSplits equal intervals between lower and upper bounds
    double IntegrateByEqualIntervals(const AQLFunctionBase& f, double xl, double xu, int nSplits) const;

	AQL1DIntegral & operator=( const AQL1DIntegral & ) { return *this; }

private:
	// member class to represent inner function
	/*! 
    @brief Declaration and definition of inner class to represent inner function

	*/
	class AQLInnerFunction : public AQLFunctionBase
	{
	public:
		// Default constructor
		explicit AQLInnerFunction(const AQL1DIntegral* pIntegral);
		// Destructor
		~AQLInnerFunction();
		// Copy constructor
		AQLInnerFunction(const AQLInnerFunction& v);
									//======================================
									// Make copy(clone) of this class
		virtual AQLCoreFunctionBase*		clone() const;
									//==========================================
									// Return y(=f(x))
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

									//==========================================
									// set function
		void						setFunc(const AQLFunctionBase& method);
									//==========================================
									// set integral region
		void						setIntegralRegion(double xl, double xu) {mXl=xl; mXu=xu;};

		AQLInnerFunction & operator=( const AQLInnerFunction & ) { return *this; }

	private:
		AQLFunctionBase* mpFunc;				// pointer to function
		double mXl;							// lower side of intergral region
		double mXu;							// upper side of integral region
		const AQL1DIntegral*		mpIntegral;	// pointer to integral method
	};
protected:

};

