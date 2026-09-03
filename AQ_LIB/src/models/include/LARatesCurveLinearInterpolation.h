#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesPEInterpolationBase.h"
#include "LAModelDynamicsCurve.h"


// ID for LARatesCurveLinearInterpolation
#define FN_CURVELINEARINTERPOLATION	1903 
// Function name for LARatesCurveLinearInterpolation
#define FN_CURVELINEARINTERPOLATION_STR	"fn_curve_linearinterpolation"

// ID for LARatesCurveForLinearInterpolation
#define PE_CURVEFORLINEARINTER		5 


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class of linear interpolation of curve path element

*/
class LARatesCurveLinearInterpolation : public LARatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesCurveLinearInterpolation();
	//	Copy constructor
	LARatesCurveLinearInterpolation(const LARatesCurveLinearInterpolation& v);
	// Destructor
	virtual ~LARatesCurveLinearInterpolation();

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


// OPERATION

private:
	/*! 
		@brief Class to return survival probability fanction value.
	*/
	class LARatesCurveForLinearInterpolation: public LARatesPathElementCurve
	{
	public:
		// Default constructor
		LARatesCurveForLinearInterpolation();
		// Destructor
		~LARatesCurveForLinearInterpolation();
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
		virtual	LARatesCurveForLinearInterpolation&
									operator = (const LARatesCurveForLinearInterpolation& a);
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
	friend class LARatesCurveLinearInterpolation;
		const LARatesPathElementCurve* mpCurve1;
		const LARatesPathElementCurve* mpCurve2;	
	};	
	
	LARatesCurveForLinearInterpolation		mValue;
protected:

};

