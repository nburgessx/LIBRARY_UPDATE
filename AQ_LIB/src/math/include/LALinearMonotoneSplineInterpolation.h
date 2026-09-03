#pragma once

#include "LAInterpolationBase.h"
#include "LALinearInterpolation.h"
#include "LAMonotoneSplineInterpolation.h"
#include "LACoreAppError.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"
#include <vector>

// MMLinearMonotoneSplineInterpolation Function ID
#define FN_LINEARMONOTONESPLINEINTERPOLATION	    1012
#define FN_LINEARMONOTONEPARABOLICINTERPOLATION	    1015

// MMLinearMonotoneSplineInterpolatiaon Function Name
#define FN_LINEARMONOTONESPLINEINTERPOLATION_STR	"fn_linearmonotonesplineinterpolation"
#define FN_LINEARMONOTONEPARABOLICINTERPOLATION_STR	"fn_linearmonotoneparabolicinterpolation"

// Alias Names
#define LINEAR_MONOTONE_SPLINE_INTERP				"linearmonotonespline"
#define LINEAR_MONOTONE_PARABOLIC_INTERP			"linearmonotoneparabolic"

class LALinearMonotoneSplineInterpolation : public LAInterpolationBase
{
public:

    // @brief Default constructor
    LALinearMonotoneSplineInterpolation(); 

    /* @brief Alternative constructor
    *  FLAT_EXTRAPOLATION = Piecewise-Constant Extrapolation
	*  LINEAR_EXTRAPOLATION = Linear Extrapolation
	*/
	explicit LALinearMonotoneSplineInterpolation( const LAMonotoneSplineInterpolation::MonotoneInterpolationEnum monotoneInterpolationType, ExtrapolationType extrapolationType=LINEAR_EXTRAPOLATION_TYPE ); 
    
    // @brief Destructor
	virtual ~LALinearMonotoneSplineInterpolation();
	
    // @brief Copy constructor
	LALinearMonotoneSplineInterpolation( const LALinearMonotoneSplineInterpolation& v );
	
    // @brief  Returns whether this class derives from base class with specified type id
	bool isTypeOf(function_t id) const;
	
    // @brief  Deep copy of this object
	virtual LACoreFunctionBase*		clone() const;
	
    // @brief Returns the interpolator type
	virtual function_t			getType() const;

	/* @brief Returns the value interpolated at the specified point
	*  @param[in]	xValue	Specifies the point where to calculate an interpolated value
	*/
	virtual double				value( const double xValue ) const; 

	// Must set the joinDateAsDouble before setting the index and value parameters
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double& joinDateAsDouble );

    /* @brief	Initializes the interpolator with the specified vectors of X and Y values
	*  @param[in]	index	Vector of X values
	*  @param[in]	value	Vector of Y values
	*/
	virtual void				set( const DoubleArray& index, const DoubleArray& value );

	/* @brief	Returns the date (as a double) at which the Linear interpolator joins the MonotoneCubic interpolator
	*/
	virtual const double        getJoinDateAsDouble() const;

	/* @brief	Sets the date (as a double) at which the Linear interpolator joins the MonotoneCubic interpolator
	*/
    virtual void                setJoinDateAsDouble( const double& joinDateAsDouble );

	/* @brief	Returns whether this is a hybrid (split) interpolator
	*/
	virtual bool                isHybrid() const { return true;  };
    
    // Assignment operator
	// This is a faulty implmentation of assignment operation. Should be fixed.
	LALinearMonotoneSplineInterpolation & operator=( const LALinearMonotoneSplineInterpolation & ) { return *this; }

	/* @brief	Returns the pillar points used to initialize the interpolator.
	*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

protected:

    std::shared_ptr<LALinearInterpolation>  linearInterpolation_;
    std::shared_ptr<LAMonotoneSplineInterpolation>  monotoneSplineInterpolation_;
    ExtrapolationType       extrapolationType_;
    
    double                  joinDateAsDouble_;
    bool                    isJoinDateSet_;

    DoubleArray             indices_;
    DoubleArray             values_;

	bool	linearInitialised_;
	bool	splineInitialised_;

};


