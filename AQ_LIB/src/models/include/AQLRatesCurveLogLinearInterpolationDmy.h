#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesPEInterpolationBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLRatesCurveLogLinearInterpolation.h"

// ID for AQLRatesCurveLogLinearInterpolationDmy
#define FN_CURVELOGLINEARINTERPOLATIONDMY	10017 
// Function name for AQLRatesCurveLogLinearInterpolationDmy
#define FN_CURVELOGLINEARINTERPOLATIONDMY_STR	"fn_curve_loglinearinterpolationdmy"

// ID for AQLRatesCurveForLogLinearInterpolationDmy
#define PE_CURVEFORLOGLINEARINTERDMY		10018 





///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class of log linearinterpolation of curve path element(for only dmy sde)

*/
class AQLRatesCurveLogLinearInterpolationDmy : public AQLRatesCurveLogLinearInterpolation
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesCurveLogLinearInterpolationDmy();
	//	Copy constructor
	AQLRatesCurveLogLinearInterpolationDmy(const AQLRatesCurveLogLinearInterpolationDmy& v);
	// Destructor
	virtual ~AQLRatesCurveLogLinearInterpolationDmy();

								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Return interpolated value
	virtual const AQLRatesPathElementBase&
								value(double t,	double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2);

	void clearVal() { mValues.clear(); }

protected:
	/*! 
		@brief Class to return survival probability fanction value.
	*/
	class AQLRatesCurveForLogLinearInterpolationDmy: public AQLRatesCurveForLogLinearInterpolation
	{
	public:
		// Default constructor
		AQLRatesCurveForLogLinearInterpolationDmy();
		// Destructor
		~AQLRatesCurveForLogLinearInterpolationDmy();
		//	Copy constructor
		AQLRatesCurveForLogLinearInterpolationDmy(const AQLRatesCurveForLogLinearInterpolationDmy& v);
	
									//======================================
									// Check pathelement for this class ID
		virtual bool                isTypeOf(pathelement_t id) const;
									//======================================
									// Make copy(clone) of this class
									/*!
										@return Deep copy of this class
									*/
		virtual AQLRatesPathElementBase*	
									clone() const;// %%% COVARIANT RETURN %%%
									//======================================
									// Return this class ID
		virtual pathelement_t		getType() const;

		// equal operator
		/*!
			@param[in] a source object
			@return copy object
		*/
		virtual	AQLRatesCurveForLogLinearInterpolationDmy&
									operator = (const AQLRatesCurveForLogLinearInterpolationDmy& a);
									// get discount bond price
									/*!
										@param[in] T maturity
										@return discount bond price
									*/
		virtual double				getP (double T) const; 	

	private:
	friend class AQLRatesCurveLogLinearInterpolationDmy;
		mutable std::map<double, double> mPVal;
	};	
	
	mutable std::map<double, AQLRatesCurveForLogLinearInterpolationDmy>	 mValues;

};

