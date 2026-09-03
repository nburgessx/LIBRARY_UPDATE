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
#define FN_CONSTRAINEDSPLINEINTERPOLATION		1005

// MMLinearInterpolatiaon Function Name
#define FN_CONSTRAINEDSPLINEINTERPOLATION_STR	"fn_constrainedsplineinterpolation"

// Alias Name
#define CONSTRAINED_SPLINE_INTERP				"constrainedspline"


/*! 
    @brief class to interpolate yeild curve by  spline

    In the class, n data are set and one-dimensional vale is returned.
    Every time data are set, old data are destructed.

	See discription "Numerical Recipes in C"
*/
class AQLConstrainedSplineInterpolation : public AQLInterpolationBase
{
public:

	// constructor
	AQLConstrainedSplineInterpolation();
	
	// destructor
	~AQLConstrainedSplineInterpolation();
	
	//	copy constructor
	AQLConstrainedSplineInterpolation(const AQLConstrainedSplineInterpolation& v);

	// check whether the class is a class of ID 
	bool		                isTypeOf(function_t id) const;

	// clone
	virtual AQLCoreFunctionBase*		clone() const;

	// return the type of class
	virtual function_t			getType() const;
	
	// return a one-dimensional interpolated value
	virtual double				value(const double x1) const; 

	//	set one-dimensional information
	virtual void				set(const DoubleArray& index, const DoubleArray& value);

	// Methods for Hybrid Interpolation Only
    //---------------------------------------------
	virtual void				set( const DoubleArray& index, const DoubleArray& value, const double & joinDateAsDouble ) { set(index, value); }
	virtual const double        getJoinDateAsDouble() const { return 0.0; }
	virtual void                setJoinDateAsDouble( const double& joinDateAsDouble ) {};
	virtual bool                isHybrid() const { return false; }
	//---------------------------------------------

    static double calc_dxx(const double x1, const double x0);

	AQLConstrainedSplineInterpolation & operator=( const AQLConstrainedSplineInterpolation & ) { return *this; }

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	virtual std::tuple<std::vector<double>,std::vector<double>> getXY() const;


protected:

	/*!
		@brief class to hold data to interpolate
	*/
	class AQLConstrainedSplineInterpolationDataProvider : public AQLDataProvider
	{
	public:
		/*!
		@brief constructor
		@param[in] Data_Index  x-axis of data (s1)
		@param[in] Data_Value  y-axis of data (s1)
		@param[in] Coeff  two dimensional differential coefficient which were calculated from data by the set method
		@param[in] s1  the number of initial data
		*/

		AQLConstrainedSplineInterpolationDataProvider(const DoubleArray dataIndex,
												const DoubleArray dataValue, 
												const DoubleArray a_,
												const DoubleArray b_, 
												const DoubleArray c_, 
												const DoubleArray d_, 
												unsigned int s1,
												double slope0_,
												double slope1_)
												: index(dataIndex), value(dataValue), a(a_), b(b_), c(c_), d(d_), size(s1), 
													slope0(slope0_), slope1(slope1_)
		{
		}

		const DoubleArray index;	// Valarray to conserve x-axis of data
		const DoubleArray value;	// Valarray to conserve y-axis of data
		const DoubleArray a,b,c,d;	// Valarray to conserve coefficients of spline function which were calculated from data 
									// by the set method. each coefficient has size values
		const unsigned int size;	// the number of initial data
		const double slope0, slope1;// slope to extrapolate
	
		AQLConstrainedSplineInterpolationDataProvider & operator=( const AQLConstrainedSplineInterpolationDataProvider & ) { return *this; }
	};

//	pointer of the class
AQLConstrainedSplineInterpolationDataProvider *mpDataProvider;

};


