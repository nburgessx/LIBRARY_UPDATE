#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAObject.h"
#include "LACoreAutoPtr.h"


//// DEFINES ////
// Entitiy id of LAMathVolatility
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



class LADataInstance;
class LADate;
class LADataDoubles;
class LAString;
class LADataString;
class LAFunctionBase;
class LAPriceDataInterpolation;

/*! 
    @brief Class to represent volatility.
*/
class LAMathVolatility : public LAObject
{
public:
// LIFECYCLE
    // default constructor
	LAMathVolatility(LADataInstance* dataInstance);
    // copy constructor
	LAMathVolatility(const LAMathVolatility& vol);
    // destructor
	virtual ~LAMathVolatility();

//  QUERY
    // Return this class type
	virtual object_t	getType(void) const;
    // Check function for this class type
	virtual bool		isTypeOf(object_t id) const;
	// get this FX name
	const LADataString&	getName() const;
	// get this FX name. The setting of name is also possible. 
	LADataString&		getName();
	//	get interpolation method
	const LAPriceDataInterpolation&
						getInterpolation() const;
	//	get interpolation method. The setting of interpolation method is also possible. 
	LAPriceDataInterpolation&
						getInterpolation();	    


	//	make copy(clone) of this FX object object.
	LAObject*			clone() const;// %%% COVARIANT RETURN %%%

	//	get volatility function.
	LAFunctionBase*
						getVolatilityFunc(unsigned i, unsigned int j) const;

	// get volatiliy not clone
	const LAFunctionBase*
						getVolatilityFunc() const;

	// get volatiliy initialvalue
	const double
						getInitialValue() const;

//  OPERATION 
	// set Volatility from grid_t and Volatility of DoubleMatrix vector
	void				setVolatility(const DoubleArray& grid_t, 
									 const std::vector<DoubleMatrix>& vol); //for multifactor HJM
	// set Volatility as function matrix
	void				setVolatility(const std::vector<std::vector<LAFunctionBase*> >& vol); //for multifactor HJM
	// set Volatility from grid_t and Volatility of Function vector
	void				setVolatility(const DoubleArray& grid_T, const std::vector<LAFunctionBase*>& vol); //for multifactor HJM
	// set Volatility from grid_t and Volatility of DoubleMatrix
	void				setVolatility(const DoubleArray& grid_t, const DoubleMatrix& vol); 
	// set Volatility as function vector
	void				setVolatility(const std::vector<LAFunctionBase*>& vol);
	// set Volatility from grid_t and Volatility of function
	void				setVolatility(const DoubleArray& grid_T, LAFunctionBase* vol);
	// set Volatility from grid_t and Volatility of vector
	void				setVolatility(const DoubleArray& grid_t, const DoubleArray& vol);//for fx
	// set Volatility as function for fx
	void				setVolatility(LAFunctionBase* vol);//for fx
	// set Volatility as double
	void				setVolatility(double vol);

	// set InitialValue as vector
	void				setInitialValue(const DoubleArray& vol0vec);
	// set InitialValue as double
	void				setInitialValue(const double vol0);

	// remove specified Data.If there is not Data to remove, do nothing.If member variable is specified to remove, do not remove it.
	virtual void        remove(const LAString& dataName);
	// Initialize this Object.
	virtual void		reset(void);

protected:
	// copy FX object	 
	virtual LAObject&	copy(const LAObject& e);

private:
	// set Data specified by the name.
	LADataHolder&				add(const LAString& name);

	LADataHolder*				mpName;			// name (DATA_STRING)
	LADataHolder*				mpGrid_T;		// T grid (DATA_DOUBLES)
	LADataHolder*				mpGrid_t;		// t grid (DATA_DOUBLES)
	LADataHolder*				mpInter;		// interpolation (DATA_INTERPOLATION)	
	LADataHolder*				mpVol_vec_f;	// volatility (DATA_FUNCTIONS)
	LADataHolder*				mpVol_f;		// volatility (DATA_FUNCTION)
	LADataHolder*				mpVol_mat_d;	// volatility (DATA_DOUBLE_MATRIX)
	LADataHolder*				mpVol_vec_d;	// volatility (DATA_DOUBLES)
	LADataHolder*				mpVol_d;		// volatility (DATA_DOUBLE)
	LADataHolder*				mpFactorNum;	// factor num (DATA_INT)
	LADataHolder*				mpInitialValues;// intial value of stochastic factor (DATA_DOUBLES)
	LADataHolder*				mpInitialValue;	// intial value of stochastic factor (DATA_DOUBLE)
	LADataHolder*				mpVolType;		// type (DATA_INT)
	

};
