#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <tuple>

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "ExceptionMacros.h"
#include <sstream>

// AQLInterpolationBase Data Enum ID
#define FN_INTERPOLATION	1001 

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of base class to represent function used for intterpolation.

	In this class, based on the data set of n pieces of data to be interpolated n \n
	returns the value of one-dimensional. Data every time you set the data, \
	the data you set just before is destroyed.

*/

struct InternalDataInformation
{
	virtual std::string getTypeAsString() const =0;
};


// Struct to Hold Integration Interval Info
struct Interval
{
	size_t  startIndex_;
	size_t	endIndex_;
};


// Struct to Store Interpolation Input Data, namely the xValues, yValues and the data size
struct InterpolationData
{
	InterpolationData( const std::vector<double> & xValues, const std::vector<double> & yValues )
		: xValues_( xValues ), yValues_( yValues ), size_( xValues.size() )
	{
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Interpolation Data: Must have the same number of xValues and yValues" )
	}

	std::vector<double> xValues_;
	std::vector<double>	yValues_;
	size_t size_;
};


class AQLInterpolationBase : public AQLCoreFunctionBase
{
public:

	// Constructors
	AQLInterpolationBase();
	virtual ~AQLInterpolationBase();
	AQLInterpolationBase(const AQLInterpolationBase& v);

	// Accessors
	//======================================
	// check whether this class derives from base class with type id
	virtual bool                isTypeOf(function_t id) const;
	
	// Create a deep copy of this object
	virtual AQLCoreFunctionBase*	clone() const = 0;
	
	// Get function type
	virtual function_t			getType() const;
	
	// Function to return the interpolated value of one-dimensional curve
	virtual double				value( const double xPoint ) const = 0; 

	// Method to differentiate the interpolation function - Must implement this separately in every derived interpolation class
	virtual double				differentiate( const double xPoint ) const;
	
	// Method to discretely differentiate the interpolation function between two points
	virtual double				differentiateOverRange( const double fromXPoint, const double toXPoint ) const;

	// Method to integrate the interpolation function - Must implement this separately in every derived interpolation class
	virtual double				integrate( const double lowerBound, const double upperBound ) const;
	
	// This method sets a interpolation value of one-dimensional curve
	virtual void				set(const DoubleArray& index, const DoubleArray& value) = 0;

    // This function allows for direct access to the underlying numbers being interpolated
	virtual std::tuple<std::vector<double>,std::vector<double> > getXY() const = 0;
	
	// Interpolation data accessors
	virtual std::vector<double> getXValues() const	{ return interpolationData_->xValues_;	}
	virtual std::vector<double> getYValues() const	{ return interpolationData_->yValues_;	}
	virtual size_t size() const						{ return interpolationData_->size_;		}

    // Methods for Hybrid Interpolation Only
    //---------------------------------------------
    // These methods are required for mixed hybrid interpolation only
	// Non-Hybrid interpolators initialize these with a joinDateAsDouble = 0.0
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) = 0;
	virtual const double        getJoinDateAsDouble() const = 0;
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) = 0;
	virtual bool                isHybrid() const = 0;

	// Copy below to Non-Hybrid Interpolator header files to make them compatible

	// // Methods for Hybrid Interpolation Only
    // //---------------------------------------------
	// virtual void				   set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) { set(index, value); }
	// virtual const double        getJoinDateAsDouble() const { return 0.0; }
	// virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) {};
	// virtual bool                isHybrid() const { return false; }
	
	//---------------------------------------------

	// Shared Pointer to Interpolation Data
	virtual std::shared_ptr<InterpolationData> interpolationData() { return interpolationData_; }

	// Interpolation Data Container - Contains the Interpolation xValues, yValues and the size of the data
	// *** IMPORTANT: This data is initialised within the legacy interpolation set methods or via the legacy copy or clone methods ***
	std::shared_ptr<InterpolationData> interpolationData_;

	// Function to Calculate the Position of the Supremum i.e. the position of the first node point greater than or equal to a given value
	virtual size_t supremum( const double & value ) const;

	// Function to get the end index and shift the index to account for extrapolation
	virtual Interval getInterval( const size_t & endIndex ) const;

	// Function to calculate the start height at the start of a given interval
	virtual double intervalStartHeight( const Interval& interval ) const;

	// Function to calculate the end height at the start of a given interval
	virtual double intervalEndHeight( const Interval& interval ) const;

	// Function to calculate the integral width for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
	virtual double intervalWidth( const Interval& interval, const double & lowerBound, const double & upperBound ) const;
	
	// Function to calculate the slope for a given interval
	virtual double intervalSlope( const Interval& interval ) const;

	// Function to calculate the area for a given interval given the integral lower- and upper bounds to evaluate partial interval and allow for extrapolation
	virtual double intervalArea( const size_t & endIndex , const double & lowerBound, const double & upperBound ) const;
};


