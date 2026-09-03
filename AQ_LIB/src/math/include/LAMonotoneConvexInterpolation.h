#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAInterpolationBase.h"
#include "LACoreAppError.h"
#include "LADataValuation.h"
#include "LACoreTemplateType.h"
#include <vector>

// LAMonotoneConvexInterpolation Function ID
#define FN_MONOTONECONVEXINTERPOLATION	1006

// LAMonotoneConvexInterpolation Function Name
#define FN_MONOTONECONVEXINTERPOLATION_STR	"fn_monotoneconvexinterpolation"

// Alias Name
#define MONOTONE_CONVEX_INTERP				"monotoneconvex"

#define INPUTFORWARDRATE 0
#define INPUTZERORATE 1
#define INPUTGENERAL 2
#define OUTPUTFORWARDRATE 0
#define OUTPUTZERORATE 1
#define OUTPUTGENERAL 2
#define OUTPUTANALYZE 9

/*! 
    @brief Class of Monotone Convex interpolation for the yield curve.

    In this class, based on the data set of n pieces of data to be interpolated n \n
	returns the value of one-dimensional. Data every time you set the data, \
	the data you set just before is destroyed. \n
	See discription "WILMOTT magazine, p70-p81, May 2008" and "Applied Mathematical Finance, Vol.13, No.2, 89-129, June 2006"
*/
class LAMonotoneConvexInterpolation : public LAInterpolationBase
{
public:

	// default constructor
	LAMonotoneConvexInterpolation();
	LAMonotoneConvexInterpolation(const double dLambda, const bool dIsAllowedNegative);
	LAMonotoneConvexInterpolation(const double dLambda, const bool dIsAllowedNegative, const int dInputMode, const int dOutputMode);

	// destructor
	~LAMonotoneConvexInterpolation();

	//	copy constructor
	LAMonotoneConvexInterpolation(const LAMonotoneConvexInterpolation& v);

	// check whether this class derives from base class with type id
	bool		                isTypeOf(function_t id) const;
	// deep copy of this object
	virtual LACoreFunctionBase*		clone() const;
	// get function type
	virtual function_t			getType() const;
	// return a value of one-dimensional complement of the curve
	virtual double				value(const double x1) const; 

	// set the information of one-dimensional curve
	virtual void				set(const DoubleArray& index, const DoubleArray& value);
	
	// Methods for Hybrid Interpolation Only
    //---------------------------------------------
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) { set(index, value); }
	virtual const double        getJoinDateAsDouble() const { return 0.0; }
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) {};
	virtual bool                isHybrid() const { return false; }
	//---------------------------------------------

	// get input mode
	int				getInputMode(void) const;
	// get output mode
	int				getOutputMode(void) const;
	// set input mode
	void				setInputMode(const int dInputMode);
	// set output mode
	void				setOutputMode(const int dOutputMode);

	// assignment operator
	LAMonotoneConvexInterpolation & operator=( const LAMonotoneConvexInterpolation & ) { return *this; }

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;

protected:

	/*!
		@brief Class that holds the data needed to interpolate.
	*/
	class LAMonotoneConvexInterpolationDataProvider : public LADataProvider
	{
	public:
		/*!
		@brief constructor
		@param[in] Data_Index x-axis of the data(s1)
		@param[in] Data_Value y-axis of the data(s1)
		@param[in] s1 initial data numbers
		*/

		LAMonotoneConvexInterpolationDataProvider(const DoubleArray dataIndex,
									const DoubleArray dataValue,
									const DoubleArray dataFdiscrete,
									const DoubleArray dataF,
									unsigned int s1)
									: index(dataIndex), value(dataValue), fdiscrete(dataFdiscrete), f(dataF), size(s1)
		{
		}

		const DoubleArray index;	// Valarray to save the data of the x-axis of the data to be interpolated
		const DoubleArray value;	// Valarray to save the data of the y-axis of the data to be interpolated
		const DoubleArray fdiscrete;// Valarray to save the fdiscrete at index
		const DoubleArray f;		// Valarray to save the f at index
		const unsigned int size;	// initial data numbers
		// assignment operator
		LAMonotoneConvexInterpolationDataProvider & operator=( const LAMonotoneConvexInterpolationDataProvider & ) { return *this; }
	};
	
	// pointer to hold data
	LAMonotoneConvexInterpolationDataProvider *mpDataProvider;

	double lambda;			// coefficient to ameliorate
	bool isAllowedNegative;	// allow negative or not
	int inputMode;			// 0:forward rate, 1:zero rate, 2:general
	int outputMode;			// 0:forward rate, 1:zero rate, 2:general

	double forward(const double x1) const; 
	double interpolant(const double x1) const; 
	double interpolantGeneral(const double x1) const; 
	double analyzeInterpolant(const double x1) const; 

};

