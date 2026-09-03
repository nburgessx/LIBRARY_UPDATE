#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLObject.h"
#include "AQLCoreAutoPtr.h"


//// DEFINES ////
// Entitiy id of AQLMathVolatility
#define ENTITY_IR_VOLATILITY 14

#ifndef IR_CALIBRATION_DATA_GRID_LARGE_T					
#define IR_CALIBRATION_DATA_GRID_LARGE_T	"Grid_T"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_GRID_SMALL_T					
#define IR_CALIBRATION_DATA_GRID_SMALL_T	"Grid_t"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_VOLDATA_MAT					
#define IR_CALIBRATION_DATA_VOLDATA_MAT	"VolData_mat"	// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_VOLDATA_VECTOR					
#define IR_CALIBRATION_DATA_VOLDATA_VECTOR	"VolData_vector"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_VOLDATA_SCALAR					
#define IR_CALIBRATION_DATA_VOLDATA_SCALAR	"VolData_scalar"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS					
#define IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS	"VolData_functions"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_VOLDATA_FUNCTION					
#define IR_CALIBRATION_DATA_VOLDATA_FUNCTION	"VolData_function"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_FACTORNUM					
#define IR_CALIBRATION_DATA_FACTORNUM			"FactorNum"				// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_VOLDATA_INITIALVALUES					
#define IR_CALIBRATION_DATA_VOLDATA_INITIALVALUES	"VolData_initialvalues"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_VOLDATA_INITIALVALUE					
#define IR_CALIBRATION_DATA_VOLDATA_INITIALVALUE	"VolData_initialvalue"		// Data Name of
#endif						
#ifndef IR_CALIBRATION_DATA_INPUTTYPE					
#define IR_CALIBRATION_DATA_INPUTTYPE			"VolInputType"				// Data Name of
#endif						



class AQLDataInstance;
class AQLDate;
class AQLDataDoubles;
class AQLString;
class AQLDataString;
class AQLFunctionBase;
class AQLPriceDataInterpolation;

/*! 
    @brief Class to represent volatility.
*/
class AQLMathVolatility : public AQLObject
{
public:
// LIFECYCLE
    // default constructor
	AQLMathVolatility(AQLDataInstance* dataInstance);
    // copy constructor
	AQLMathVolatility(const AQLMathVolatility& vol);
    // destructor
	virtual ~AQLMathVolatility();

//  QUERY
    // Return this class type
	virtual object_t	getType(void) const;
    // Check function for this class type
	virtual bool		isTypeOf(object_t id) const;
	// get this FX name
	const AQLDataString&	getName() const;
	// get this FX name. The setting of name is also possible. 
	AQLDataString&		getName();
	//	get interpolation method
	const AQLPriceDataInterpolation&
						getInterpolation() const;
	//	get interpolation method. The setting of interpolation method is also possible. 
	AQLPriceDataInterpolation&
						getInterpolation();	    


	//	make copy(clone) of this FX object object.
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%

	//	get volatility function.
	AQLFunctionBase*
						getVolatilityFunc(unsigned i, unsigned int j) const;

	// get volatiliy not clone
	const AQLFunctionBase*
						getVolatilityFunc() const;

	// get volatiliy initialvalue
	const double
						getInitialValue() const;

//  OPERATION 
	// set Volatility from grid_t and Volatility of DoubleMatrix vector
	void				setVolatility(const DoubleArray& grid_t, 
									 const std::vector<DoubleMatrix>& vol); //for multifactor HJM
	// set Volatility as function matrix
	void				setVolatility(const std::vector<std::vector<AQLFunctionBase*> >& vol); //for multifactor HJM
	// set Volatility from grid_t and Volatility of Function vector
	void				setVolatility(const DoubleArray& grid_T, const std::vector<AQLFunctionBase*>& vol); //for multifactor HJM
	// set Volatility from grid_t and Volatility of DoubleMatrix
	void				setVolatility(const DoubleArray& grid_t, const DoubleMatrix& vol); 
	// set Volatility as function vector
	void				setVolatility(const std::vector<AQLFunctionBase*>& vol);
	// set Volatility from grid_t and Volatility of function
	void				setVolatility(const DoubleArray& grid_T, AQLFunctionBase* vol);
	// set Volatility from grid_t and Volatility of vector
	void				setVolatility(const DoubleArray& grid_t, const DoubleArray& vol);//for fx
	// set Volatility as function for fx
	void				setVolatility(AQLFunctionBase* vol);//for fx
	// set Volatility as double
	void				setVolatility(double vol);

	// set InitialValue as vector
	void				setInitialValue(const DoubleArray& vol0vec);
	// set InitialValue as double
	void				setInitialValue(const double vol0);

	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const AQLString& dataName);
	// Initialize this Object.
	virtual void		reset(void);

protected:
	// copy FX object	 
	virtual AQLObject&	copy(const AQLObject& e);

private:
	// set Data specified by the name.
	AQLDataHolder&				add(const AQLString& name);

	AQLDataHolder*				mpName;			// name (DATA_STRING)
	AQLDataHolder*				mpGrid_T;		// T grid (DATA_DOUBLES)
	AQLDataHolder*				mpGrid_t;		// t grid (DATA_DOUBLES)
	AQLDataHolder*				mpInter;		// interpolation (DATA_INTERPOLATION)	
	AQLDataHolder*				mpVol_vec_f;	// volatility (DATA_FUNCTIONS)
	AQLDataHolder*				mpVol_f;		// volatility (DATA_FUNCTION)
	AQLDataHolder*				mpVol_mat_d;	// volatility (DATA_DOUBLE_MATRIX)
	AQLDataHolder*				mpVol_vec_d;	// volatility (DATA_DOUBLES)
	AQLDataHolder*				mpVol_d;		// volatility (DATA_DOUBLE)
	AQLDataHolder*				mpFactorNum;	// factor num (DATA_INT)
	AQLDataHolder*				mpInitialValues;// intial value of stochastic factor (DATA_DOUBLES)
	AQLDataHolder*				mpInitialValue;	// intial value of stochastic factor (DATA_DOUBLE)
	AQLDataHolder*				mpVolType;		// type (DATA_INT)
	

};
