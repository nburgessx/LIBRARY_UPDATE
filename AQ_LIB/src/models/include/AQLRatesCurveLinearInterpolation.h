#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesPEInterpolationBase.h"
#include "AQLModelDynamicsCurve.h"


// ID for AQLRatesCurveLinearInterpolation
#define FN_CURVELINEARINTERPOLATION	1903 
// Function name for AQLRatesCurveLinearInterpolation
#define FN_CURVELINEARINTERPOLATION_STR	"fn_curve_linearinterpolation"

// ID for AQLRatesCurveForLinearInterpolation
#define PE_CURVEFORLINEARINTER		5 


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class of linear interpolation of curve path element

*/
class AQLRatesCurveLinearInterpolation : public AQLRatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesCurveLinearInterpolation();
	//	Copy constructor
	AQLRatesCurveLinearInterpolation(const AQLRatesCurveLinearInterpolation& v);
	// Destructor
	virtual ~AQLRatesCurveLinearInterpolation();

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
	class AQLRatesCurveForLinearInterpolation: public AQLRatesPathElementCurve
	{
	public:
		// Default constructor
		AQLRatesCurveForLinearInterpolation();
		// Destructor
		~AQLRatesCurveForLinearInterpolation();
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
		virtual	AQLRatesCurveForLinearInterpolation&
									operator = (const AQLRatesCurveForLinearInterpolation& a);
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
	friend class AQLRatesCurveLinearInterpolation;
		const AQLRatesPathElementCurve* mpCurve1;
		const AQLRatesPathElementCurve* mpCurve2;	
	};	
	
	AQLRatesCurveForLinearInterpolation		mValue;
protected:

};

