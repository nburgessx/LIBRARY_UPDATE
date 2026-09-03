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
#define FN_PARABOLICINTERPOLATION           1009

// MMLinearInterpolatiaon Function Name
#define FN_PARABOLICINTERPOLATION_STR	    "fn_parabolicinterpolation"

// Alias Names
#define PARABOLIC_INTERP					"parabolic"

/*! 
    @brief Class of spline interpolation for the yield curve.

    In this class, based on the data set of n pieces of data to be interpolated n \n
	returns the value of one-dimensional. Data every time you set the data, \
	the data you set just before is destroyed. \n
	See discription "Numerical Recipes in C"
*/
class LAParabolicInterpolation : public LAInterpolationBase
{
public:

    // default constructor
	LAParabolicInterpolation();
    
    // destructor
	virtual ~LAParabolicInterpolation();
	
    //	copy constructor
	LAParabolicInterpolation(const LAParabolicInterpolation& v);

    // check whether this class derives from base class with type id
	bool		                isTypeOf(function_t id) const;
	
    // deep copy of this object
	virtual LACoreFunctionBase*		clone() const;
	
    // get function type
	virtual function_t			getType() const;
	
    // return a value of one-dimensional complement of the curve
	virtual double				value(const double x1) const; 

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
	LAParabolicInterpolation & operator=( const LAParabolicInterpolation & ) { return *this; }

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

protected:

    /*!
        @brief Class that holds the data needed to interpolate.
    */
    class LAParabolicInterpolationDataProvider : public LADataProvider
    {
    public:

	    /*!
        @brief constructor
        @param[in] Data_Index x-axis of the data(s1)
	    @param[in] Data_Value y-axis of the data(s1)
	    @param[in] Coeff coefficient of the second derivatives of the spline function calculated by the method from the Data Set
	    @param[in] s1 initial data numbers
	    */
        LAParabolicInterpolationDataProvider(const DoubleArray dataIndex,const DoubleArray dataValue, const DoubleArray coeff, unsigned int s1)
            : index(dataIndex), value(dataValue), coeff(coeff), size(s1) {}

	    const DoubleArray index;	// Valarray to save the data of the x-axis of the data to be interpolated
	    const DoubleArray value;	// Valarray to save the data of the y-axis of the data to be interpolated
	    const DoubleArray coeff;	// Valarray to save the second derivatives of the spline function calculated by the method from the Data Set
	    const unsigned int size;	// initial data numbers
	    
        // assignment operator
	    LAParabolicInterpolationDataProvider & operator=( const LAParabolicInterpolationDataProvider & ) { return *this; }
    };
	
    // pointer to hold data
	LAParabolicInterpolationDataProvider *mpDataProvider;


};
