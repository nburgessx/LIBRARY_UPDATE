#pragma once

/*! @file
    @brief Class declaration to perform linear-spline interpolation for the yield curve.

    In this class, based on the data set of n pieces of data to be interpolated n \n
	returns the value of one-dimensional. Data every time you set the data, \
	the data you set just before is destroyed.

*/
//  3rd April 2017 Miuhzo Interational London

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLInterpolationBase.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLCoreAppError.h"
#include "AQLDataValuation.h"
#include "AQLCoreTemplateType.h"
#include <vector>

// AQLLinearInterpolatiaon Function ID
#define FN_LINEARSPLINEINTERPOLATION	    1010

// AQLLinearInterpolatiaon Function Name
#define FN_LINEARSPLINEINTERPOLATION_STR	"fn_linearsplineinterpolation"

// Alias Name
#define LINEAR_SPLINE_INTERP				"linearspline"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class of Linear-Spline interpolation for the yield curve.

    In this class, based on the data set of n pieces of data to be interpolated n \n
	returns the value of one-dimensional. Data every time you set the data, \
	the data you set just before is destroyed. \n
	Outside is constant interpolation.
*/
class AQLLinearSplineInterpolation : public AQLInterpolationBase
{
public:

    // default constructor
    AQLLinearSplineInterpolation(); 

    // alternative constructor
    // FLAT_EXTRAPOLATION = Piecewise-Constant Extrapolation
	// LINEAR_EXTRAPOLATION = Linear Extrapolation
	explicit AQLLinearSplineInterpolation( ExtrapolationType extrapolationType, const bool useNaturalSpline); 
    
    // destructor
	virtual ~AQLLinearSplineInterpolation();
	
    // copy constructor
	AQLLinearSplineInterpolation( const AQLLinearSplineInterpolation& v );

    // check whether this class derives from base class with type id
	bool		                isTypeOf(function_t id) const;
	
    // deep copy of this object
	virtual AQLCoreFunctionBase*		clone() const;
	
    // get function type
	virtual function_t			getType() const;

	// return a value of one-dimensional complement of the curve
	virtual double				value( const double x1 ) const; 

	// Differentiation & Integration
	virtual double differentiate( const double & x1 ) const; 
	virtual double integrate(const double & lowerBound, const double & upperBound ) const;
	
	// set the information of one-dimensional curve

	// Must set the joinDateAsDouble before setting the index and value parameters
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double& joinDateAsDouble );

	// Must set the joinDateAsDouble before setting the index and value parameters
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble );
	
	// Must set the joinDateAsDouble before setting the index and value parameters
	virtual void				set( const DoubleArray& index, const DoubleArray& value );
	
	virtual const double        getJoinDateAsDouble() const;
    virtual bool                isHybrid() const { return true;  };
    
    // assignment operator
	// This is a faulty implmentation of assignment operation. Should be fixed.
	AQLLinearSplineInterpolation & operator=( const AQLLinearSplineInterpolation & ) { return *this; }

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;


protected:

	// Base Class Method Implementations
	// ---------------------------------------------------
	
	// *** Virtual Base Methods Not Required for this Class, since using the methods from the Linear and Spline Sub-Classes ***
	// virtual double intervalSlope( const Interval& interval ) const;
	// virtual double intervalArea( const size_t & endIndex , const double & lowerBound, const double & upperBound ) const;

    std::shared_ptr<AQLLinearInterpolation>  linearInterpolation_;
    std::shared_ptr<AQLSplineInterpolation>  splineInterpolation_;
    ExtrapolationType       extrapolationType_;
    
    double                  joinDateAsDouble_;
    bool                    isJoinDateSet_;

    DoubleArray             indices_;
    DoubleArray             values_;

	bool	linearInitialised_;
	bool	splineInitialised_;

};
