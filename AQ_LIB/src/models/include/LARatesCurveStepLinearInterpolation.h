#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesPEInterpolationBase.h"
#include "LAModelDynamicsCurve.h"


// ID for LARatesCurveStepLinearInterpolation
#define FN_CURVESTEPLINEARINTERPOLATION	1905 
// Function name for LARatesCurveStepLinearInterpolation
#define FN_CURVESTEPLINEARINTERPOLATION_STR	"fn_curve_steplinearinterpolation"

// ID for LARatesCurveForLinearInterpolation
#define PE_CURVEFORSTEPLINEARINTER		12 

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde integral class

*/
class LARatesCurveStepLinearInterpolation : public LARatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesCurveStepLinearInterpolation();
	//	Copy constructor
	LARatesCurveStepLinearInterpolation(const LARatesCurveStepLinearInterpolation& v);
	// Destructor
	virtual ~LARatesCurveStepLinearInterpolation();

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
	virtual const LARatesPathElementBase&
								value(double t,	double t1, double t2,
										const LARatesPathElementBase& val1,
										const LARatesPathElementBase& val2);


// OPERATION

private:
	/*! 
		@brief Class to return survival probability fanction value.
	*/
	class LARatesCurveForStepLinearInterpolation: public LARatesPathElementCurve
	{
	public:
		// Default constructor
		LARatesCurveForStepLinearInterpolation();
		// Destructor
		~LARatesCurveForStepLinearInterpolation();
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
		virtual	LARatesCurveForStepLinearInterpolation&
									operator = (const LARatesCurveForStepLinearInterpolation& a);
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
	friend class LARatesCurveStepLinearInterpolation;
		const LARatesPathElementCurve* mpCurve1;
			
	};	
	
	LARatesCurveForStepLinearInterpolation		mValue;
protected:

};

