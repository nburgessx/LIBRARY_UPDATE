#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesPEInterpolationBase.h"
#include "LAModelDynamicsCurve.h"
#include "LARatesCurveLogLinearInterpolation.h"

// ID for LARatesCurveLogLinearInterpolationDmy
#define FN_CURVELOGLINEARINTERPOLATIONDMY	10017 
// Function name for LARatesCurveLogLinearInterpolationDmy
#define FN_CURVELOGLINEARINTERPOLATIONDMY_STR	"fn_curve_loglinearinterpolationdmy"

// ID for LARatesCurveForLogLinearInterpolationDmy
#define PE_CURVEFORLOGLINEARINTERDMY		10018 





///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class of log linearinterpolation of curve path element(for only dmy sde)

*/
class LARatesCurveLogLinearInterpolationDmy : public LARatesCurveLogLinearInterpolation
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesCurveLogLinearInterpolationDmy();
	//	Copy constructor
	LARatesCurveLogLinearInterpolationDmy(const LARatesCurveLogLinearInterpolationDmy& v);
	// Destructor
	virtual ~LARatesCurveLogLinearInterpolationDmy();

								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Return interpolated value
	virtual const LARatesPathElementBase&
								value(double t,	double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2);

	void clearVal() { mValues.clear(); }

protected:
	/*! 
		@brief Class to return survival probability fanction value.
	*/
	class LARatesCurveForLogLinearInterpolationDmy: public LARatesCurveForLogLinearInterpolation
	{
	public:
		// Default constructor
		LARatesCurveForLogLinearInterpolationDmy();
		// Destructor
		~LARatesCurveForLogLinearInterpolationDmy();
		//	Copy constructor
		LARatesCurveForLogLinearInterpolationDmy(const LARatesCurveForLogLinearInterpolationDmy& v);
	
									//======================================
									// Check pathelement for this class ID
		virtual bool                isTypeOf(pathelement_t id) const;
									//======================================
									// Make copy(clone) of this class
									/*!
										@return Deep copy of this class
									*/
		virtual LARatesPathElementBase*	
									clone() const;// %%% COVARIANT RETURN %%%
									//======================================
									// Return this class ID
		virtual pathelement_t		getType() const;

		// equal operator
		/*!
			@param[in] a source object
			@return copy object
		*/
		virtual	LARatesCurveForLogLinearInterpolationDmy&
									operator = (const LARatesCurveForLogLinearInterpolationDmy& a);
									// get discount bond price
									/*!
										@param[in] T maturity
										@return discount bond price
									*/
		virtual double				getP (double T) const; 	

	private:
	friend class LARatesCurveLogLinearInterpolationDmy;
		mutable std::map<double, double> mPVal;
	};	
	
	mutable std::map<double, LARatesCurveForLogLinearInterpolationDmy>	 mValues;

};

