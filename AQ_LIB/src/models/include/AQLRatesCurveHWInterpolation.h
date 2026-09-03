#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ include +++++
#include "AQLRatesPEInterpolationBase.h"
#include "AQLModelDynamicsCurve.h"
#include <map>

//+++++ definition +++++
// ID for AQLRatesCurveHWInterpolation
#define FN_CURVEHWINTERPOLATION	10027 

// Function name for AQLRatesCurveHWInterpolation
#define FN_CURVEHWINTERPOLATION_STR	"fn_curve_hwinterpolation"

// ID for AQLRatesCurveForHWInterpolation
#define PE_CURVEFORHW		10028 

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde integral class

*/

using namespace std;

class AQLRatesCurveHWInterpolation : public AQLRatesPEInterpolationBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesCurveHWInterpolation();
	//	Copy constructor
	AQLRatesCurveHWInterpolation(const AQLRatesCurveHWInterpolation& v);
	// Destructor
	virtual ~AQLRatesCurveHWInterpolation();

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
								value( double t,	double t1, double t2,
								       const AQLRatesPathElementBase& val1,
									   const AQLRatesPathElementBase& val2
                                     );
                                //======================================
								// clear cache
	virtual void				init(void);

protected:

private:
	
    /*! 
		@brief Class to return survival probability fanction value.
	*/
	class AQLRatesCurveForHWInterpolation: public AQLRatesPathElementCurve
	{
	public:
		// Default constructor
		AQLRatesCurveForHWInterpolation();

        //AQLRatesCurveForHWInterpolation(const AQLRatesCurveForHWInterpolation& rhs); 
		// Destructor
		virtual ~AQLRatesCurveForHWInterpolation();
									//======================================
									// Check pathelement for this class ID
		virtual bool                isTypeOf(pathelement_t id) const;
									//======================================

		virtual AQLRatesPathElementBase*	
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
		virtual	AQLRatesCurveForHWInterpolation&
									operator = (const AQLRatesCurveForHWInterpolation& a);
									//======================================

									// get discount bond price
									/*!
										@param[in] T 
										@return discount bond price
									*/
		virtual double				getP (double T) const;
                                    //======================================

		mutable map<double, AQLRatesPathElementBase*> curve_cache;
		mutable map<double, bool>                 is_curve_cache;
		mutable map<double, double>				  initialF_cache;
    private:

        friend class                              AQLRatesCurveHWInterpolation;
        const AQLRatesPathElementCurve*               mpCurve1;
		const AQLRatesPathElementCurve*               mpCurve2;
		bool                                      is_cloned;
	};

    AQLRatesCurveForHWInterpolation     mValue;

};

