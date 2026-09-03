#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAInterpolationBase.h"
#include "LACoreAppError.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"
#include <vector>

// MMLinearInterpolatiaon Function ID
#define FN_SPLINEINTERPOLATION	            1003
#define FN_NATURALSPLINEINTERPOLATION	    1007
#define FN_CLAMPEDSPLINEINTERPOLATION	    1008

// MMLinearInterpolatiaon Function Name
#define FN_SPLINEINTERPOLATION_STR	        "fn_splineinterpolation"
#define FN_NATURALSPLINEINTERPOLATION_STR	"fn_naturalsplineinterpolation"
#define FN_CLAMPEDSPLINEINTERPOLATION_STR	"fn_clampedsplineinterpolation"

// Alias Names
#define SPLINE_INTERP						"spline"
#define NATURAL_SPLINE_INTERP				"naturalspline"
#define CLAMPED_SPLINE_INTERP				"clampedspline"


/*! 
    @brief Class of spline interpolation for the yield curve.

    In this class, based on the data set of n pieces of data to be interpolated n \n
	returns the value of one-dimensional. Data every time you set the data, \
	the data you set just before is destroyed. \n
	See discription "Numerical Recipes in C"
*/
class LASplineInterpolation : public LAInterpolationBase
{
public:

	// default constructor
	LASplineInterpolation();

    // constructor allowing the setting of the 'useNaturalSpline' parameter
	LASplineInterpolation( const bool useNaturalSpline );

	// destructor
	virtual ~LASplineInterpolation();

	//	copy constructor
	LASplineInterpolation(const LASplineInterpolation& v);

	// check whether this class derives from base class with type id
	bool		                isTypeOf(function_t id) const;

	// deep copy of this object
	virtual LACoreFunctionBase*	clone() const;

	// get function type
	virtual function_t			getType() const;
	
	// Interpolation Method
	virtual double				value(const double x1) const; 

	// Differentiation & Integration
	virtual double				differentiate( const double & x1 ) const; 
	virtual double				integrate(const double & lowerBound, const double & upperBound ) const;
	
	// set the information of one-dimensional curve
	virtual void				set(const DoubleArray& index, const DoubleArray& value );

	// Methods for Hybrid Interpolation Only
    //---------------------------------------------
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) { set(index, value); }
	virtual const double        getJoinDateAsDouble() const { return 0.0; }
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) {};
	virtual bool                isHybrid() const { return false; }
	//---------------------------------------------

	// assignment operator
	LASplineInterpolation & operator=( const LASplineInterpolation & ) { return *this; }

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

protected:

    bool useNaturalSpline_;

	/*!
		@brief Class that holds the data needed to interpolate.
	*/
	class LASplineInterpolationDataProvider : public LADataProvider
	{
	public:
		/*!
		@brief constructor
		@param[in] Data_Index x-axis of the data(s1)
		@param[in] Data_Value y-axis of the data(s1)
		@param[in] Coeff coefficient of the second derivatives of the spline function calculated by the method from the Data Set
		@param[in] s1 initial data numbers
		*/

		LASplineInterpolationDataProvider(const DoubleArray& dataIndex,
									      const DoubleArray& dataValue, 
									      const DoubleArray& coeff, 
									      unsigned int s1)
									      : index(dataIndex), value(dataValue), coeff(coeff), size(s1)
		{
		}

		const DoubleArray index;	// Valarray to save the data of the x-axis of the data to be interpolated
		const DoubleArray value;	// Valarray to save the data of the y-axis of the data to be interpolated
		const DoubleArray coeff;	// Valarray to save the second derivatives of the spline function calculated by the method from the Data Set
		const unsigned int size;	// initial data numbers
	
		// assignment operator
		LASplineInterpolationDataProvider & operator=( const LASplineInterpolationDataProvider & ) { return *this; }
	};

	// Base Class Method Implementations
	// ---------------------------------------------------

	// Function to calculate the slope for a given interval
	virtual double intervalSlope( const Interval& interval, const double& x1 ) const;
	
	// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
	virtual double intervalArea( const size_t & endIndex , const double & lowerBound, const double & upperBound ) const;

	// pointer to hold data
	LASplineInterpolationDataProvider *mpDataProvider;

};



