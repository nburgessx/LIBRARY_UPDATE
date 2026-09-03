// AQLLinearInterpolation.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLInterpolationBase.h"
#include "AQLCoreAppError.h"
#include "AQLDataValuation.h"
#include "AQLCoreTemplateType.h"
#include <vector>

// MMLinearInterpolatiaon Function ID
#define FN_LINEARINTERPOLATION	1002

// MMLinearInterpolatiaon Function Name
#define FN_LINEARINTERPOLATION_STR	"fn_linearinterpolation"

// Alias Name
#define LINEAR_INTERP				"linear"

// Extrapolation Type
enum ExtrapolationType
{
	FLAT_EXTRAPOLATION_TYPE,
	LINEAR_EXTRAPOLATION_TYPE
};

/*	Linear Interpolation Class
	--------------------------------
	Must 'Set' the interpolation data after calling the constructor, which overwrites existing data.
    Extrapolation can be set to flat or linear
*/
class AQLLinearInterpolation : public AQLInterpolationBase
{
public:
	// Constructors
	// ------------------------
	explicit AQLLinearInterpolation(ExtrapolationType type = LINEAR_EXTRAPOLATION_TYPE);
	virtual ~AQLLinearInterpolation();
	AQLLinearInterpolation(const AQLLinearInterpolation& v);

	// Accessors
	// --------------------------

	// check whether this class derives from base class with type id
	bool isTypeOf(function_t id) const;
	
	// deep copy of this object
	virtual AQLCoreFunctionBase*	clone() const;
	
	// get function type
	virtual function_t getType() const;

	// Interpolation Value
	virtual double value( const double x1 ) const; 

	// Differentiation & Integration
	virtual double differentiate( const double & x1 ) const; 
	virtual double integrate(const double & lowerBound, const double & upperBound ) const;

	// Initialise
	// --------------------------

	// set the information of one-dimensional curve
	virtual void set(const DoubleArray& index, const DoubleArray& value);
	
	// Methods for Hybrid Interpolation Only
    //---------------------------------------------
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) { set(index, value); }
	virtual const double        getJoinDateAsDouble() const { return 0.0; }
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) {};
	virtual bool                isHybrid() const { return false; }
	//---------------------------------------------

	// assignment operator
	AQLLinearInterpolation & operator=( const AQLLinearInterpolation & ) { return *this; }

	/* This function allows for direct access to the underlying numbers being interpolated */
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

protected:

	// Class that holds the data needed to interpolate.
	class AQLLinearInterpolationDataProvider : public AQLDataProvider
	{
	public:
		/*!
		@brief constructor
		@param[in] i1 initial data at x coordinate (s1)
		@param[in] v initial data at y coordinate (s1)
		@param[in] s1 number of initial data
		*/
		AQLLinearInterpolationDataProvider(const DoubleArray& i1, const DoubleArray& v, unsigned int s1)
			: index1(i1), value1(v),size1(s1) 
		{}
		
		const DoubleArray index1;	// Valarray to keep initial x data
		const DoubleArray  value1;  // Valarray to keep initial y data
		unsigned int size1;			// Int to keep initial data number

		// assignment operator
		AQLLinearInterpolationDataProvider & operator=( const AQLLinearInterpolationDataProvider & ) { return *this; }
	};

	// Base Class Method Implementations
	// ---------------------------------------------------

	// Function to calculate the slope for a given interval
	virtual double intervalSlope( const Interval& interval ) const;
	
	// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
	virtual double intervalArea( const size_t & endIndex , const double & lowerBound, const double & upperBound ) const;

	// Member Data
	AQLLinearInterpolationDataProvider *mpDataProvider;
	ExtrapolationType mExtrapolationType;

};
