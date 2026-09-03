// LAStepInterpolation.cpp
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAInterpolationBase.h"
#include "LACoreAppError.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"
#include <vector>

// Function ID of MMStepInterpolatiaon
#define FN_STEPINTERPOLATION	1004

// Function Name of MMStepInterpolation
#define FN_STEPINTERPOLATION_STR				"fn_stepinterpolation"
#define FN_RIGHTCONTINUOUS_INTERPOLATION_STR	"fn_rightcontinuousinterpolation"
#define FN_LEFTCONTINUOUS_INTERPOLATION_STR		"fn_leftcontinuousinterpolation"

// Alias Names
#define STEP_INTERP								"step"
#define RIGHT_CONTINUOUS_INTERP					"rightcontinuous"
#define LEFT_CONTINUOUS_INTERP					"leftcontinuous"

// Step Interpolation Type
enum StepType
{
	RIGHT_CONTINUOUS,   // [ x[i], x[i+1] ) corresponds to y[i], right continous
	LEFT_CONTINUOUS     // ( x[i-1], x[i] ] corresponds to y[i], left continous
};

/* 
    @brief Class declaration for interpolation with step function
	This class derives from LAInterpolationBase
*/
class LAStepInterpolation : public LAInterpolationBase
{
public:

	// constructor
	explicit LAStepInterpolation(StepType steptype = RIGHT_CONTINUOUS);
	
	// destructor
	~LAStepInterpolation();
	
	// copy constructor
	LAStepInterpolation(const LAStepInterpolation& v);

	// Check function for this class ID
    virtual bool isTypeOf(function_t id) const;

	// Make copy(clone) of this class
    virtual LACoreFunctionBase* clone() const;
		
	// Return this class type
    virtual function_t getType() const;

	// Return interpolated value at x1
	virtual double value(const double x1) const; 

	// Differentiation & Integration
	virtual double differentiate( const double & x1 ) const; 
	virtual double integrate(const double & lowerBound, const double & upperBound ) const;

	// Set points and values
	virtual void set(const DoubleArray& index, const DoubleArray& value);
	
	// Methods for Hybrid Interpolation Only
    //---------------------------------------------
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) { set(index, value); }
	virtual const double        getJoinDateAsDouble() const { return 0.0; }
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) {};
	virtual bool                isHybrid() const { return false; }
	//---------------------------------------------

	// assignment operator
	LAStepInterpolation & operator=( const LAStepInterpolation & ) { return *this; }

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

protected:

	// class to hold data necessary for interpolation
	class LAStepInterpolationDataProvider : public LADataProvider
	{
	public:
		
		/*
		@brief constructor
		@param[in] i1 initial data at x coordinate (s1)
		@param[in] v initial data at y coordinate (s1)
		@param[in] s1 number of initial data
		*/
		LAStepInterpolationDataProvider(const DoubleArray& i1, const DoubleArray& v, unsigned int s1)
		: index1(i1), value1(v),size1(s1)
		{}

		const DoubleArray index1;	// Valarray to keep initial x data
		const DoubleArray  value1;  // Valarray to keep initial y data
		unsigned int size1;			// Int to keep initial data number
		LAStepInterpolationDataProvider & operator=( const LAStepInterpolationDataProvider & ) { return *this; }
	};

	// Base Class Method Implementations
	// ---------------------------------------------------

	// Function to calculate the slope for a given interval
	virtual double intervalSlope( const Interval& interval ) const;

	// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
	virtual double intervalArea( const size_t & endIndex , const double & lowerBound, const double & upperBound ) const;

	// Member Data
	LAStepInterpolationDataProvider *mpDataProvider;	// Pointer to class data
	StepType mStepType;

};

