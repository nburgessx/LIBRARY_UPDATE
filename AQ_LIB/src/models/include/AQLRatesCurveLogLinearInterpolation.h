#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesPEInterpolationBase.h"
#include "AQLModelDynamicsCurve.h"

// ID for AQLRatesCurveLogLinearInterpolation
#define FN_CURVELOGLINEARINTERPOLATION	1904 
// Function name for AQLRatesCurveLogLinearInterpolation
#define FN_CURVELOGLINEARINTERPOLATION_STR	"fn_curve_loglinearinterpolation"

// ID for AQLRatesCurveForLogLinearInterpolation
#define PE_CURVEFORLOGLINEARINTER		11 





///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde integral class

*/
class AQLRatesCurveLogLinearInterpolation : public AQLRatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesCurveLogLinearInterpolation();
	//	Copy constructor
	AQLRatesCurveLogLinearInterpolation(const AQLRatesCurveLogLinearInterpolation& v);
	// Destructor
	virtual ~AQLRatesCurveLogLinearInterpolation();

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
	void setVolForInterpolation(const DoubleMatrix &mat){mVol = mat;}
	// set start time of this curve
	/*virtual void				set_t(double t); */
	
// OPERATION

protected:
	/*! 
		@brief Class to return survival probability fanction value.
	*/
	class AQLRatesCurveForLogLinearInterpolation: public AQLRatesPathElementCurve
	{
	public:
		// Default constructor
		AQLRatesCurveForLogLinearInterpolation();
		// Destructor
		~AQLRatesCurveForLogLinearInterpolation();
		//	Copy constructor
		AQLRatesCurveForLogLinearInterpolation(const AQLRatesCurveForLogLinearInterpolation& v);
	
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
		virtual	AQLRatesCurveForLogLinearInterpolation&
									operator = (const AQLRatesCurveForLogLinearInterpolation& a);
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
		virtual void				set_t(double t);

	protected:
	friend class AQLRatesCurveLogLinearInterpolation;
		const AQLRatesPathElementCurve* mpCurve1;
		const AQLRatesPathElementCurve* mpCurve2;
		DoubleVector* mpDiscount1for;
		DoubleVector  mSpreads;
		DoubleMatrix volmat;
		unsigned int mfixpos;
		unsigned int mSpreadsID;
	};	
	
	AQLRatesCurveForLogLinearInterpolation		mValue;
	DoubleMatrix mVol;
	
protected:

};

