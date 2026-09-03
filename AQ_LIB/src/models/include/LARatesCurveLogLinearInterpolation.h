#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesPEInterpolationBase.h"
#include "LAModelDynamicsCurve.h"

// ID for LARatesCurveLogLinearInterpolation
#define FN_CURVELOGLINEARINTERPOLATION	1904 
// Function name for LARatesCurveLogLinearInterpolation
#define FN_CURVELOGLINEARINTERPOLATION_STR	"fn_curve_loglinearinterpolation"

// ID for LARatesCurveForLogLinearInterpolation
#define PE_CURVEFORLOGLINEARINTER		11 





///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde integral class

*/
class LARatesCurveLogLinearInterpolation : public LARatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesCurveLogLinearInterpolation();
	//	Copy constructor
	LARatesCurveLogLinearInterpolation(const LARatesCurveLogLinearInterpolation& v);
	// Destructor
	virtual ~LARatesCurveLogLinearInterpolation();

//  QUERY
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
	void setVolForInterpolation(const DoubleMatrix &mat){mVol = mat;}
	// set start time of this curve
	/*virtual void				set_t(double t); */
	
// OPERATION

protected:
	/*! 
		@brief Class to return survival probability fanction value.
	*/
	class LARatesCurveForLogLinearInterpolation: public LARatesPathElementCurve
	{
	public:
		// Default constructor
		LARatesCurveForLogLinearInterpolation();
		// Destructor
		~LARatesCurveForLogLinearInterpolation();
		//	Copy constructor
		LARatesCurveForLogLinearInterpolation(const LARatesCurveForLogLinearInterpolation& v);
	
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
		virtual	LARatesCurveForLogLinearInterpolation&
									operator = (const LARatesCurveForLogLinearInterpolation& a);
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
	friend class LARatesCurveLogLinearInterpolation;
		const LARatesPathElementCurve* mpCurve1;
		const LARatesPathElementCurve* mpCurve2;
		DoubleVector* mpDiscount1for;
		DoubleVector  mSpreads;
		DoubleMatrix volmat;
		unsigned int mfixpos;
		unsigned int mSpreadsID;
	};	
	
	LARatesCurveForLogLinearInterpolation		mValue;
	DoubleMatrix mVol;
	
protected:

};

