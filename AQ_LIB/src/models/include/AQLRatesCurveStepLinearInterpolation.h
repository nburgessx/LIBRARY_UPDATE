#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesPEInterpolationBase.h"
#include "AQLModelDynamicsCurve.h"


// ID for AQLRatesCurveStepLinearInterpolation
#define FN_CURVESTEPLINEARINTERPOLATION	1905 
// Function name for AQLRatesCurveStepLinearInterpolation
#define FN_CURVESTEPLINEARINTERPOLATION_STR	"fn_curve_steplinearinterpolation"

// ID for AQLRatesCurveForLinearInterpolation
#define PE_CURVEFORSTEPLINEARINTER		12 

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde integral class

*/
class AQLRatesCurveStepLinearInterpolation : public AQLRatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesCurveStepLinearInterpolation();
	//	Copy constructor
	AQLRatesCurveStepLinearInterpolation(const AQLRatesCurveStepLinearInterpolation& v);
	// Destructor
	virtual ~AQLRatesCurveStepLinearInterpolation();

//  QUERY
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


// OPERATION

private:
	/*! 
		@brief Class to return survival probability fanction value.
	*/
	class AQLRatesCurveForStepLinearInterpolation: public AQLRatesPathElementCurve
	{
	public:
		// Default constructor
		AQLRatesCurveForStepLinearInterpolation();
		// Destructor
		~AQLRatesCurveForStepLinearInterpolation();
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
		virtual	AQLRatesCurveForStepLinearInterpolation&
									operator = (const AQLRatesCurveForStepLinearInterpolation& a);
									//======================================
									// get value 
									/*!
										@return NULL
										@note this method does not support
									*/
//		virtual const void*			get(void) const {return NULL;} 
									// get discount bond price
									/*!
										@param[in] T maturity
										@return discount bond price
									*/
		virtual double				getP (double T) const; 	
									// set value
									/*!									
										@note this method does not support
									*/
//		virtual void				set(const void* pdata){pdata;} 

	private:
	friend class AQLRatesCurveStepLinearInterpolation;
		const AQLRatesPathElementCurve* mpCurve1;
			
	};	
	
	AQLRatesCurveForStepLinearInterpolation		mValue;
protected:

};

