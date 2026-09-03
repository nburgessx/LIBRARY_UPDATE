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
class AQLParabolicInterpolation : public AQLInterpolationBase
{
public:

    // default constructor
	AQLParabolicInterpolation();
    
    // destructor
	virtual ~AQLParabolicInterpolation();
	
    //	copy constructor
	AQLParabolicInterpolation(const AQLParabolicInterpolation& v);

    // check whether this class derives from base class with type id
	bool		                isTypeOf(function_t id) const;
	
    // deep copy of this object
	virtual AQLCoreFunctionBase*		clone() const;
	
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
	AQLParabolicInterpolation & operator=( const AQLParabolicInterpolation & ) { return *this; }

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

protected:

    /*!
        @brief Class that holds the data needed to interpolate.
    */
    class AQLParabolicInterpolationDataProvider : public AQLDataProvider
    {
    public:

	    /*!
        @brief constructor
        @param[in] Data_Index x-axis of the data(s1)
	    @param[in] Data_Value y-axis of the data(s1)
	    @param[in] Coeff coefficient of the second derivatives of the spline function calculated by the method from the Data Set
	    @param[in] s1 initial data numbers
	    */
        AQLParabolicInterpolationDataProvider(const DoubleArray dataIndex,const DoubleArray dataValue, const DoubleArray coeff, unsigned int s1)
            : index(dataIndex), value(dataValue), coeff(coeff), size(s1) {}

	    const DoubleArray index;	// Valarray to save the data of the x-axis of the data to be interpolated
	    const DoubleArray value;	// Valarray to save the data of the y-axis of the data to be interpolated
	    const DoubleArray coeff;	// Valarray to save the second derivatives of the spline function calculated by the method from the Data Set
	    const unsigned int size;	// initial data numbers
	    
        // assignment operator
	    AQLParabolicInterpolationDataProvider & operator=( const AQLParabolicInterpolationDataProvider & ) { return *this; }
    };
	
    // pointer to hold data
	AQLParabolicInterpolationDataProvider *mpDataProvider;


};
