#pragma once

#include "LAInterpolationBase.h"
#include "LACoreAppError.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"
#include <vector>

#include <ql/math/interpolations/cubicinterpolation.hpp> // QuantLib interpolators

// MMLinearInterpolatiaon Function ID
#define FN_MONOTONESPLINEINTERPOLATION	    1011
#define FN_MONOTONEPARABOLICINTERPOLATION	1013

// MMLinearInterpolatiaon Function Name
#define FN_MONOTONESPLINEINTERPOLATION_STR	        "fn_monotonesplineinterpolation"
#define FN_MONOTONEPARABOLICINTERPOLATION_STR	    "fn_monotoneparabolicinterpolation"

// Alias Names
#define MONOTONE_SPLINE_INTERP						"monotonespline"
#define MONOTONE_PARABOLIC_INTERP					"monotoneparabolic"

class LAMonotoneSplineInterpolation : public LAInterpolationBase
{
public:

	enum MonotoneInterpolationEnum
	{
		FRITSCH_BUTLAND,
		MONOTONE_PARABOLIC,
		MONOTONE_CONVEX
	};


	// @brief Default constructor
	LAMonotoneSplineInterpolation();
	
	// @brief Constructor allowing the setting of the monotone interpolation sub-type
	LAMonotoneSplineInterpolation( const MonotoneInterpolationEnum monotoneInterpolationType );

	// @brief Destructor
	virtual ~LAMonotoneSplineInterpolation();
	
	// @brief Copy constructor
	LAMonotoneSplineInterpolation(const LAMonotoneSplineInterpolation& v);


	// @brief  Returns whether this class derives from base class with specified type id
	bool		                isTypeOf(function_t id) const;
	
	// @brief  Deep copy of this object
	virtual LACoreFunctionBase*		clone() const;
	
	// @brief Returns the interpolator type
	virtual function_t			getType() const;
	
	// return a value of one-dimensional complement of the curve
	virtual double				value(const double x1) const; 

    /* @brief	Initializes the interpolator with the specified vectors of X and Y values
	*  @param[in]	index	Vector of X values
	*  @param[in]	value	Vector of Y values
	*/
	virtual void				set(const DoubleArray& index, const DoubleArray& value );

	// Methods for Hybrid Interpolation Only
    //---------------------------------------------
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) { set(index, value); }
	virtual const double        getJoinDateAsDouble() const { return 0.0; }
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) {};
	virtual bool                isHybrid() const { return false; }
	//---------------------------------------------

	// Assignment operator
	// This is a faulty implmentation of assignment operation. Should be fixed.
	LAMonotoneSplineInterpolation & operator=( const LAMonotoneSplineInterpolation & ) { return *this; }

	/* @brief	Returns the pillar points used to initialize the interpolator.
	*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

	// Accesssors
	DoubleVector xValues() const { return xValues_; }
	DoubleVector yValues() const { return yValues_; }

protected:
	
	MonotoneInterpolationEnum monotoneInterpolationType_; // The sub-type of monotone interpolator to use

	std::shared_ptr<QuantLib::Interpolation> qlInterpolator_; // Holds the QuantLib interpolator instance
	DoubleVector xValues_;
	DoubleVector yValues_;

};



