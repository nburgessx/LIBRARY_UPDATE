#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ include +++++
#include "LARatesPEInterpolationBase.h"
#include "LAModelDynamicsCurve.h"
#include <map>

//+++++ definition +++++
// ID for LARatesCurveHWInterpolation
#define FN_CURVEHWINTERPOLATION	10027 

// Function name for LARatesCurveHWInterpolation
#define FN_CURVEHWINTERPOLATION_STR	"fn_curve_hwinterpolation"

// ID for LARatesCurveForHWInterpolation
#define PE_CURVEFORHW		10028 

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde integral class

*/

using namespace std;

class LARatesCurveHWInterpolation : public LARatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	LARatesCurveHWInterpolation();
	//	Copy constructor
	LARatesCurveHWInterpolation(const LARatesCurveHWInterpolation& v);
	// Destructor
	virtual ~LARatesCurveHWInterpolation();

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
								value( double t,	double t1, double t2,
								       const LARatesPathElementBase& val1,
									   const LARatesPathElementBase& val2
                                     );
                                //======================================
								// clear cache
	virtual void				init(void);

protected:

private:
	
    /*! 
		@brief Class to return survival probability fanction value.
	*/
	class LARatesCurveForHWInterpolation: public LARatesPathElementCurve
	{
	public:
		// Default constructor
		LARatesCurveForHWInterpolation();

        //LARatesCurveForHWInterpolation(const LARatesCurveForHWInterpolation& rhs); 
		// Destructor
		virtual ~LARatesCurveForHWInterpolation();
									//======================================
									// Check pathelement for this class ID
		virtual bool                isTypeOf(pathelement_t id) const;
									//======================================

		virtual LARatesPathElementBase*	
									clone() const;// %%% COVARIANT RETURN %%%
									//======================================
									// Return this class ID
		virtual pathelement_t		getType() const;
                                    //======================================

		// equal operator
		/*!
			@param[in] a source object
			@return copy object
		*/
		virtual	LARatesCurveForHWInterpolation&
									operator = (const LARatesCurveForHWInterpolation& a);
									//======================================

									// get discount bond price
									/*!
										@param[in] T 
										@return discount bond price
									*/
		virtual double				getP (double T) const;
                                    //======================================

		mutable map<double, LARatesPathElementBase*> curve_cache;
		mutable map<double, bool>                 is_curve_cache;
		mutable map<double, double>				  initialF_cache;
    private:

        friend class                              LARatesCurveHWInterpolation;
        const LARatesPathElementCurve*               mpCurve1;
		const LARatesPathElementCurve*               mpCurve2;
		bool                                      is_cloned;
	};

    LARatesCurveForHWInterpolation     mValue;

};

