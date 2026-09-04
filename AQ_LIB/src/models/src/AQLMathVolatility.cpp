/*! @file
    @brief Source code for class to represent volatility.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(AQLDataString)<BR>
			2.IR_CALIBRATION_DATA_GRID_LARGE_T(AQLDataDoubles)<BR>
			3.IR_CALIBRATION_DATA_GRID_SMALL_T(AQLDataDoubles)<BR>
			4.CALIBRATION_DATA_INTERPOLATION(AQLPriceDataInterpolation)<BR>
			5.IR_CALIBRATION_DATA_VOLDATA_MAT(AQLDataDoubleMatrix)<BR>
			6.IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS(AQLDataFunctions)<BR>
			7.IR_CALIBRATION_DATA_VOLDATA_FUNCTION(AQLDataFunction)<BR>
			8.IR_CALIBRATION_DATA_VOLDATA_VECTOR(AQLDataDoubles)<BR>
			9.IR_CALIBRATION_DATA_VOLDATA_SCALAR(AQLDataDouble)<BR>
			10.IR_CALIBRATION_DATA_FACTORNUM(AQLPriceInt)<BR>
			11.IR_CALIBRATION_DATA_INPUTTYPE(AQLPriceInt)<BR>
	
*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathVolatility.h"
#include "AQLMathDefine.h"
#include "AQLBasic.h"
#include "AQLConstant.h"
#include "AQL1DDataSet.h"

#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataManager.h"
#include "AQLDataProcedure.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataFunction.h"
#include "AQLPriceDataFunctions.h"
#include "AQLPriceDataInterpolation.h"

#include <cmath>

using namespace std;

#define	T_I_DATA_MATRIX	1
#define	T_I_FUNC_MATRIX	2
#define	T_I_FUNC_VECTOR	3
#define	T_DATA_MATRIX	4
#define	T_FUNC_VECTOR	5
#define	T_FUNC_SCALAR	6
#define	DATA_VECTOR	7
#define	FUNC_SCALAR	8
#define	T_DATA_SCALAR	9
#define	DATA_SCALAR	10


/*!
    @brief default constructor

	@param[in] dataInstance pointer of AQLDataInstance object

*/
AQLMathVolatility::AQLMathVolatility(AQLDataInstance* dataInstance) : 
				AQLObject()
{
	setDataInstance(dataInstance);
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	dm.setData(CALIBRATION_DATA_NAME						, DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_GRID_LARGE_T			, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_GRID_SMALL_T			, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_INTERPOLATION			, DATA_INTERPOLATION);
	dm.setData(IR_CALIBRATION_DATA_VOLDATA_MAT			, DATA_DOUBLE_MATRIX);
	dm.setData(IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS		, DATA_FUNCTIONS);
	dm.setData(IR_CALIBRATION_DATA_VOLDATA_FUNCTION		, DATA_FUNCTION);
	dm.setData(IR_CALIBRATION_DATA_VOLDATA_VECTOR		, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_VOLDATA_SCALAR		, DATA_DOUBLE);
	dm.setData(IR_CALIBRATION_DATA_VOLDATA_INITIALVALUES	, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_VOLDATA_INITIALVALUE	, DATA_DOUBLE);
	dm.setData(IR_CALIBRATION_DATA_FACTORNUM				, DATA_INT);
	dm.setData(IR_CALIBRATION_DATA_INPUTTYPE				, DATA_INT);

	mpName			= &add(CALIBRATION_DATA_NAME);
	mpGrid_T  		= &add(IR_CALIBRATION_DATA_GRID_LARGE_T	);
	mpGrid_t   		= &add(IR_CALIBRATION_DATA_GRID_SMALL_T	);
	mpInter			= &add(CALIBRATION_DATA_INTERPOLATION	);
	mpVol_mat_d		= &add(IR_CALIBRATION_DATA_VOLDATA_MAT	);
	mpVol_vec_f		= &add(IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS	);
	mpVol_f			= &add(IR_CALIBRATION_DATA_VOLDATA_FUNCTION	);
	mpVol_vec_d		= &add(IR_CALIBRATION_DATA_VOLDATA_VECTOR	);
	mpVol_d			= &add(IR_CALIBRATION_DATA_VOLDATA_SCALAR	);
	mpInitialValues = &add(IR_CALIBRATION_DATA_VOLDATA_INITIALVALUES);
	mpInitialValue  = &add(IR_CALIBRATION_DATA_VOLDATA_INITIALVALUE );
	mpFactorNum		= &add(IR_CALIBRATION_DATA_FACTORNUM);
	mpVolType		= &add(IR_CALIBRATION_DATA_INPUTTYPE);
}
/*!
    @brief copy constructor

	@param[in] fx AQLMathVolatility object
*/
AQLMathVolatility::AQLMathVolatility(
	const AQLMathVolatility& vol) : 
	AQLObject(vol)
{
	mpName			= &getData(CALIBRATION_DATA_NAME	);
	mpGrid_T		= &getData(IR_CALIBRATION_DATA_GRID_LARGE_T	);
	mpGrid_t		= &getData(IR_CALIBRATION_DATA_GRID_SMALL_T	);
	mpInter			= &getData(CALIBRATION_DATA_INTERPOLATION	);
	mpVol_mat_d		= &getData(IR_CALIBRATION_DATA_VOLDATA_MAT	);
	mpVol_vec_f		= &getData(IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS	);
	mpVol_f			= &getData(IR_CALIBRATION_DATA_VOLDATA_FUNCTION	);
	mpVol_vec_d		= &getData(IR_CALIBRATION_DATA_VOLDATA_VECTOR	);
	mpVol_d			= &getData(IR_CALIBRATION_DATA_VOLDATA_SCALAR	);
	mpInitialValues = &getData(IR_CALIBRATION_DATA_VOLDATA_INITIALVALUES);
	mpInitialValue  = &getData(IR_CALIBRATION_DATA_VOLDATA_INITIALVALUE );
	mpFactorNum		= &getData(IR_CALIBRATION_DATA_FACTORNUM	);
	mpVolType		= &getData(IR_CALIBRATION_DATA_INPUTTYPE	);
}

/*!
    @brief destructor
*/
AQLMathVolatility::~AQLMathVolatility()
{

}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
AQLMathVolatility::getType(void) const
{
	return ENTITY_IR_VOLATILITY;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
AQLMathVolatility::isTypeOf(object_t id) const
{
	return (id == ENTITY_IR_VOLATILITY ? true : AQLObject::isTypeOf(id));
}


/*!
    @brief get this volatility Object-name.
	@return name
*/
const AQLDataString&	
AQLMathVolatility::getName() const	
{
	return dynamic_cast<const AQLDataString&>(mpName->get());
}
/*!
    @brief get this volatility  Object-name.The setting of name is also possible.
	@return name
*/
AQLDataString&	
AQLMathVolatility::getName()
{
	return dynamic_cast<AQLDataString&>(mpName->get());
}


/*!
    @brief Make copy(clone) of this Volatility Object object.
    @return pointer of this Volatility Object object.
*/
AQLObject* 
AQLMathVolatility::clone() const
{
    try {
    	AQLMathVolatility*	pVol = new AQLMathVolatility(*this);
    	return pVol;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
    @brief get Interpolation
			
	@return Interpolation
*/
const AQLPriceDataInterpolation&
AQLMathVolatility::getInterpolation() const	
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get Interpolation
			
	@return Interpolation. The setting of interpolation method is also possible.
*/
AQLPriceDataInterpolation&
AQLMathVolatility::getInterpolation() 
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get volatility function
			
	@return volatility function   get volatility function.
*/

AQLFunctionBase*
AQLMathVolatility::getVolatilityFunc(unsigned i, unsigned int j) const
{
	int type = dynamic_cast<AQLDataInt&>(mpVolType->get()).get();
	
	if (type == DATA_SCALAR)
	{
		double ret = dynamic_cast<const AQLDataDouble&>(mpVol_d->get()).get();
		return  dynamic_cast<AQLFunctionBase*>(AQLConstant(ret).clone());
	}
	else if (type == FUNC_SCALAR)
	{
		const AQLFunctionBase& method = dynamic_cast<const AQLPriceDataFunction&>(mpVol_f->get()).getFunction();
		return  dynamic_cast<AQLFunctionBase*>(method.clone());
	}	

	
	else if (type == T_I_DATA_MATRIX)
	{
		unsigned int factornum = dynamic_cast<const AQLDataInt&>(mpFactorNum->get()).get();
		const DoubleMatrix& mat = dynamic_cast<AQLDataDoubleMatrix&>(mpVol_mat_d->get()).get();
		if (i * factornum + j >= mat.size())
		{
			//error
			throw AQLCoreInvalidData("Suffix is over Size", __FILE__, __LINE__);
		}
		const DoubleArray& data = mat[i * factornum + j];
		const DoubleArray& time = dynamic_cast<AQLDataDoubles&>(mpGrid_t->get()).get();		
		AQL1DDataSet method;
		if (data.size() < time.size())
		{
			DoubleArray _time = time;
			_time.resize(data.size());
			method.set(_time, data);
		}
		else if (data.size() > time.size())
		{
			//error
			throw AQLCoreInvalidData("gird_t size is smaller than data size", __FILE__, __LINE__);
		}
		else
			method.set(time, data);
		const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
		method.setInterpolation(inter);
		return  dynamic_cast<AQLFunctionBase*>(method.clone());
	}
	
	else if (type == T_I_FUNC_MATRIX)
	{
		unsigned int factornum = dynamic_cast<const AQLDataInt&>(mpFactorNum->get()).get();
		const AQLPriceDataFunctions& attr = dynamic_cast<const AQLPriceDataFunctions&>(mpVol_vec_f->get());
		if (i * factornum + j >= attr.getSize())
		{
			//error
			throw AQLCoreInvalidData("Suffix is over Size", __FILE__, __LINE__);
		}		
		const AQLFunctionBase& method = attr[i * factornum + j];
		return  dynamic_cast<AQLFunctionBase*>(method.clone());
	}

	else if (type == T_I_FUNC_VECTOR)
	{
		//unsigned int factornum = dynamic_cast<const AQLDataInt&>(mpFactorNum->get()).get();
		const AQLPriceDataFunctions& attr = dynamic_cast<const AQLPriceDataFunctions&>(mpVol_vec_f->get());
		if (j >= attr.getSize())
		{
			//error
			throw AQLCoreInvalidData("Suffix is over Size", __FILE__, __LINE__);
		}			
		const AQLFunctionBase& method = attr[j];
		const DoubleArray& grid_T = dynamic_cast<AQLDataDoubles&>(mpGrid_T->get()).get();
		DoubleArray x(2);
		x[1] = grid_T.at(i);
		AQLFunctionBase* pfunc = dynamic_cast<AQLFunctionBase*>(method.clone());
		pfunc->operator ()(0, x);
		return pfunc; 	
	}

	if (j != 0)
	{
		//error
		throw AQLCoreInvalidData("Suffix is over Size(j must be 0)", __FILE__, __LINE__);
	}	
	
	if (type == T_DATA_MATRIX)
	{
		const DoubleArray& data = dynamic_cast<AQLDataDoubleMatrix&>(mpVol_mat_d->get()).get()[i];
		const DoubleArray& time = dynamic_cast<AQLDataDoubles&>(mpGrid_t->get()).get();
		AQL1DDataSet method;
		if (data.size() < time.size())
		{
			DoubleArray _time = time;
			_time.resize(data.size());
			method.set(_time, data);
		}
		else if (data.size() > time.size())
		{
			//error
			throw AQLCoreInvalidData("grid_t size is smaller than data size", __FILE__, __LINE__);
		}
		else
			method.set(time, data);
		const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
		method.setInterpolation(inter);
		return  dynamic_cast<AQLFunctionBase*>(method.clone());
	}
	else if (type == T_FUNC_VECTOR)
	{
		const AQLFunctionBase& method = dynamic_cast<const AQLPriceDataFunctions&>(mpVol_vec_f->get())[i];
		return  dynamic_cast<AQLFunctionBase*>(method.clone());
	}
	else if (type == T_FUNC_SCALAR)
	{
		const AQLFunctionBase& method = dynamic_cast<const AQLPriceDataFunction&>(mpVol_f->get()).getFunction();
		const DoubleArray& grid_T = dynamic_cast<AQLDataDoubles&>(mpGrid_T->get()).get();
		DoubleArray x(2);
		x[1] = grid_T.at(i);
		AQLFunctionBase* pfunc = dynamic_cast<AQLFunctionBase*>(method.clone());
		pfunc->operator ()(0, x);
		return pfunc; 
	}
	
	if (i != 0)
	{
		throw AQLCoreInvalidData("Suffix is over Size(i must be 0)", __FILE__, __LINE__);
	}
	
	else if (type == DATA_VECTOR)
	{
		const DoubleArray& data = dynamic_cast<AQLDataDoubles&>(mpVol_vec_d->get()).get();
		const DoubleArray& time = dynamic_cast<AQLDataDoubles&>(mpGrid_t->get()).get();
		AQL1DDataSet method;
		method.set(time, data);
		const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
		method.setInterpolation(inter);
		return  dynamic_cast<AQLFunctionBase*>(method.clone());			
	}
	else
	{
		//error
		throw AQLCoreInvalidData("Not support InputType", __FILE__, __LINE__);
	}
	return NULL;

}

const AQLFunctionBase*
AQLMathVolatility::getVolatilityFunc() const
{
	int type = dynamic_cast<AQLDataInt&>(mpVolType->get()).get();
	
	if (type == FUNC_SCALAR)
	{
		const AQLFunctionBase* method = &(dynamic_cast<const AQLPriceDataFunction&>(mpVol_f->get()).getFunction());
		return method;
	}
	else if (type == T_FUNC_VECTOR)
	{
		const AQLFunctionBase* method = &(dynamic_cast<const AQLPriceDataFunctions&>(mpVol_vec_f->get())[0]);
		return method;
		
	}
	else
	{
		throw AQLCoreInvalidData("Error in AQLMathVolatility",__FILE__,__LINE__);
	}


}


/*!
    @brief set Volatility from grid_t and Volatility of DoubleMatrix vector (T*i*t)
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of DoubleMatrix vector

*/

void
AQLMathVolatility::setVolatility(const DoubleArray& grid_t, const vector<DoubleMatrix>& vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(T_I_DATA_MATRIX);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(vol[0].size());
	

	DoubleMatrix mat;
	mat.resize(vol.size() * vol[0].size());
	unsigned int count = 0;
	for (unsigned int i = 0; i < vol.size(); i++)
	{
		for (unsigned int j = 0; j < vol[i].size(); j++)
		{
			mat[count].resize(vol[i][j].size());
			for (unsigned int k = 0; k < vol[i][j].size(); k++)
				mat[count][k] = vol[i][j][k];			
			count++;
		}
	}
	dynamic_cast<AQLDataDoubleMatrix&>(mpVol_mat_d->get()).set(mat);
}


/*!
    @brief set Volatility as function matrix (T*i*f(t))
	
	@param[in] vol		Volatility as function matrix

*/

void
AQLMathVolatility::setVolatility(const vector<vector<AQLFunctionBase*> >& vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(T_I_FUNC_MATRIX);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(vol[0].size());
	
	AQLPriceDataFunctions& attr = dynamic_cast<AQLPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();
	for (unsigned int i = 0; i < vol.size(); i++)
		for (unsigned int j = 0; j < vol[i].size(); j++)
		{
			AQLString name = getName().get();
			name += "_";
			name += AQLDataInt(i).convertToString();
			name += "_";
			name += AQLDataInt(j).convertToString();
			attr.push_back(vol[i][j], name);
		}


}


/*!
    @brief set Volatility from grid_t and Volatility of Function vector (i*f(t,T))
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of Function vector

*/

void
AQLMathVolatility::setVolatility(const DoubleArray& grid_T, const vector<AQLFunctionBase*>& vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(T_I_FUNC_VECTOR);
	dynamic_cast<AQLDataDoubles&>(mpGrid_T->get()).set(grid_T);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(vol.size());
	
	DoubleArray x(2);
	AQLPriceDataFunctions& attr = dynamic_cast<AQLPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();

	for (unsigned int j = 0; j < vol.size(); j++)
	{
		AQLString name = getName().get();
		name += "_0_";
		name += AQLDataInt(j).convertToString();
		attr.push_back(vol[j], name);
	}


/*	for (unsigned int i = 0; i < grid_T.size(); i++)
	{
		x[1] = grid_T[i];
		for (unsigned int j = 0; j < vol.size(); j++)
		{
			AQLString name = getName().get();
			name += "_";
			name += AQLDataInt(i).convertToString();
			name += "_";
			name += AQLDataInt(j).convertToString();
			AQLFunctionBase* method;
			
			if (i == 0) method = vol[j];
			else method = dynamic_cast<AQLFunctionBase*>(vol[j]->clone());
			
			method->operator ()(1, x);
			attr.push_back(method, name);
		}
	}*/
}

/*!
    @brief set Volatility from grid_t and Volatility of DoubleMatrix (T*t)
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of DoubleMatrix

*/

void
AQLMathVolatility::setVolatility(const DoubleArray& grid_t, const DoubleMatrix& vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(T_DATA_MATRIX);
	dynamic_cast<AQLDataDoubles&>(mpGrid_t->get()).set(grid_t);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(1);
	dynamic_cast<AQLDataDoubleMatrix&>(mpVol_mat_d->get()).set(vol);
}
	
/*!
    @brief set Volatility as function vector (T*f(t))
	
	@param[in] vol		Volatility as function vector

*/

void
AQLMathVolatility::setVolatility(const vector<AQLFunctionBase*>& vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(T_FUNC_VECTOR);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(1);
	
	AQLPriceDataFunctions& attr = dynamic_cast<AQLPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();
	for (unsigned int i = 0; i < vol.size(); i++)
	{
		AQLString name = getName().get();
		name += "_";
		name += AQLDataInt(i).convertToString();
		name += "_";
		name += AQLDataInt(0).convertToString();
			
		attr.push_back(vol[i], name);
	}
}

/*!
    @brief set Volatility from grid_t and Volatility of function (f(t,T))
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of function

*/
void
AQLMathVolatility::setVolatility(const DoubleArray& grid_T, AQLFunctionBase* vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(T_FUNC_SCALAR);
	dynamic_cast<AQLDataDoubles&>(mpGrid_T->get()).set(grid_T);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(1);
	AQLPriceDataFunction& attr = dynamic_cast<AQLPriceDataFunction&>(mpVol_f->get());
	AQLString name = getName().get();
	name += "_0_0";	
	attr.setFunction(vol, name);

/*	AQLPriceDataFunctions& attr = dynamic_cast<AQLPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();
	DoubleArray x(2);
	for (unsigned int i = 0; i < grid_T.size(); i++)
	{
		x[1] = grid_T[i];
		AQLString name = getName().get();
		name += "_";
		name += AQLDataInt(i).convertToString();
		name += "_";
		name += AQLDataInt(0).convertToString();
		AQLFunctionBase* method;
			
		if (i == 0) method = vol;
		else method = dynamic_cast<AQLFunctionBase*>(vol->clone());
			
		method->operator ()(1, x);
		attr.push_back(method, name);
	}*/
}

/*!
    @brief set Volatility from grid_t and Volatility of vector (t)
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of vector

*/

void
AQLMathVolatility::setVolatility(const DoubleArray& grid_t, const DoubleArray& vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(DATA_VECTOR);
	dynamic_cast<AQLDataDoubles&>(mpGrid_t->get()).set(grid_t);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(1);
	dynamic_cast<AQLDataDoubles&>(mpVol_vec_d->get()).set(vol);
}


/*!
    @brief set Volatility as function for fx (f(t))
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of vector

*/


void
AQLMathVolatility::setVolatility(AQLFunctionBase* vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(FUNC_SCALAR);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(1);
	AQLPriceDataFunction& attr = dynamic_cast<AQLPriceDataFunction&>(mpVol_f->get());
	AQLString name = getName().get();
	name += "_0_0";	
	attr.setFunction(vol, name);
}

/*!
    @brief set Volatility as double
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility as double

*/
void
AQLMathVolatility::setVolatility(double vol)
{
	dynamic_cast<AQLDataInt&>(mpVolType->get()).set(DATA_SCALAR);
	dynamic_cast<AQLDataInt&>(mpFactorNum->get()).set(1);
	dynamic_cast<AQLDataDouble&>(mpVol_d->get()).set(vol);
}



/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
AQLMathVolatility::remove(
	const AQLString& dataName)
{
	if(dataName == CALIBRATION_DATA_NAME
		|| dataName == CALIBRATION_DATA_INTERPOLATION
		|| dataName == IR_CALIBRATION_DATA_GRID_LARGE_T
		|| dataName == IR_CALIBRATION_DATA_GRID_SMALL_T
		|| dataName == IR_CALIBRATION_DATA_VOLDATA_MAT
		|| dataName == IR_CALIBRATION_DATA_VOLDATA_VECTOR
		|| dataName == IR_CALIBRATION_DATA_VOLDATA_SCALAR
		|| dataName == IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS
		|| dataName == IR_CALIBRATION_DATA_VOLDATA_FUNCTION
		|| dataName == IR_CALIBRATION_DATA_FACTORNUM
		|| dataName == IR_CALIBRATION_DATA_INPUTTYPE)
	{
		return; 
	}
	AQLObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
AQLMathVolatility::reset(void)
{
	clear();
	mpName		 = &add(CALIBRATION_DATA_NAME);
	mpGrid_T  	 = &add(IR_CALIBRATION_DATA_GRID_LARGE_T	);
	mpGrid_t   	 = &add(IR_CALIBRATION_DATA_GRID_SMALL_T	);
	mpInter		 = &add(CALIBRATION_DATA_INTERPOLATION	);
	mpVol_mat_d	 = &add(IR_CALIBRATION_DATA_VOLDATA_MAT	);
	mpVol_vec_f	 = &add(IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS	);
	mpVol_f		 = &add(IR_CALIBRATION_DATA_VOLDATA_FUNCTION	);
	mpVol_vec_d	 = &add(IR_CALIBRATION_DATA_VOLDATA_VECTOR	);
	mpVol_d		 = &add(IR_CALIBRATION_DATA_VOLDATA_SCALAR	);
	mpFactorNum	 = &add(IR_CALIBRATION_DATA_FACTORNUM);
	mpVolType	 = &add(IR_CALIBRATION_DATA_INPUTTYPE);
}

/*!
    @brief set InitialValue as vector
	
	@param[in] vol0vec		Initial value of stochastic factor as vector
*/
void
AQLMathVolatility::setInitialValue(const DoubleArray& vol0vec)
{
	dynamic_cast<AQLDataDoubles&>(mpInitialValues->get()).set(vol0vec);
}

/*!
    @brief set InitialValue as double
	
	@param[in] vol0		Initial value of stochastic factor as double
*/
void
AQLMathVolatility::setInitialValue(const double vol0)
{
	dynamic_cast<AQLDataDouble&>(mpInitialValue->get()).set(vol0);
}

/*!
	@brief get initial value of stochastic factor
	@return reference to holder class 
*/
const double
AQLMathVolatility::getInitialValue() const
{
	int type = dynamic_cast<AQLDataInt&>(mpVolType->get()).get();
	
	if (type == FUNC_SCALAR)
	{
		const double ret = dynamic_cast<const AQLDataDouble&>(mpInitialValue->get()).get();
		return ret;
	}
	else if (type == T_FUNC_VECTOR)
	{
		const double ret = dynamic_cast<const AQLDataDoubles&>(mpInitialValues->get()).get()[0];
		return ret;		
	}
	else
	{
		throw AQLCoreInvalidData("Error in AQLMathVolatility",__FILE__,__LINE__);
	}


}


/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy AQLSingleCredit
	@param[in] e copy source
	@return reference to this object
*/
AQLObject&
AQLMathVolatility::copy(
	const AQLObject& e)
{
	if (this == &e) return *this;

	AQLObject::copy(e);
	if (!e.isTypeOf(ENTITY_IR_VOLATILITY))
	{
		AQLString err = "Assignement error for AQLMathVolatility : from ";
		err += AQLString(e.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}
	mpName		 = &getData(CALIBRATION_DATA_NAME	);
	mpGrid_T	 = &getData(IR_CALIBRATION_DATA_GRID_LARGE_T	);
	mpGrid_t	 = &getData(IR_CALIBRATION_DATA_GRID_SMALL_T	);
	mpInter		 = &getData(CALIBRATION_DATA_INTERPOLATION	);
	mpVol_mat_d	 = &getData(IR_CALIBRATION_DATA_VOLDATA_MAT	);
	mpVol_vec_f	 = &getData(IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS	);
	mpVol_f		 = &getData(IR_CALIBRATION_DATA_VOLDATA_FUNCTION	);
	mpVol_vec_d	 = &getData(IR_CALIBRATION_DATA_VOLDATA_VECTOR	);
	mpVol_d		 = &getData(IR_CALIBRATION_DATA_VOLDATA_SCALAR	);
	mpFactorNum	 = &getData(IR_CALIBRATION_DATA_FACTORNUM	);
	mpVolType	 = &getData(IR_CALIBRATION_DATA_INPUTTYPE	);

	return *this;
}
/*!
	@brief add certain data
	@param[in] name name of certain data
	@return reference to holder class 
*/
AQLDataHolder&
AQLMathVolatility::add(const AQLString& name)
{
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::add(name, dh);
}
