/*! @file
    @brief Source code for class to represent volatility.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(LADataString)<BR>
			2.IR_CALIBRATION_DATA_GRID_LARGE_T(LADataDoubles)<BR>
			3.IR_CALIBRATION_DATA_GRID_SMALL_T(LADataDoubles)<BR>
			4.CALIBRATION_DATA_INTERPOLATION(LAPriceDataInterpolation)<BR>
			5.IR_CALIBRATION_DATA_VOLDATA_MAT(LADataDoubleMatrix)<BR>
			6.IR_CALIBRATION_DATA_VOLDATA_FUNCTIONS(LADataFunctions)<BR>
			7.IR_CALIBRATION_DATA_VOLDATA_FUNCTION(LADataFunction)<BR>
			8.IR_CALIBRATION_DATA_VOLDATA_VECTOR(LADataDoubles)<BR>
			9.IR_CALIBRATION_DATA_VOLDATA_SCALAR(LADataDouble)<BR>
			10.IR_CALIBRATION_DATA_FACTORNUM(LAPriceInt)<BR>
			11.IR_CALIBRATION_DATA_INPUTTYPE(LAPriceInt)<BR>
	
*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathVolatility.h"
#include "LAMathDefine.h"
#include "LABasic.h"
#include "LAConstant.h"
#include "LA1DDataSet.h"

#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LAPriceDataManager.h"
#include "LADataProcedure.h"
#include "LADataInstance.h"
#include "LAPriceDataFunction.h"
#include "LAPriceDataFunctions.h"
#include "LAPriceDataInterpolation.h"

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

	@param[in] dataInstance pointer of LADataInstance object

*/
LAMathVolatility::LAMathVolatility(LADataInstance* dataInstance) : 
				LAObject()
{
	setDataInstance(dataInstance);
	LAPriceDataManager& dm = dataInstance->getDataMaster();
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

	@param[in] fx LAMathVolatility object
*/
LAMathVolatility::LAMathVolatility(
	const LAMathVolatility& vol) : 
	LAObject(vol)
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
LAMathVolatility::~LAMathVolatility()
{

}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
LAMathVolatility::getType(void) const
{
	return ENTITY_IR_VOLATILITY;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
LAMathVolatility::isTypeOf(object_t id) const
{
	return (id == ENTITY_IR_VOLATILITY ? true : LAObject::isTypeOf(id));
}


/*!
    @brief get this volatility Object-name.
	@return name
*/
const LADataString&	
LAMathVolatility::getName() const	
{
	return dynamic_cast<const LADataString&>(mpName->get());
}
/*!
    @brief get this volatility  Object-name.The setting of name is also possible.
	@return name
*/
LADataString&	
LAMathVolatility::getName()
{
	return dynamic_cast<LADataString&>(mpName->get());
}


/*!
    @brief Make copy(clone) of this Volatility Object object.
    @return pointer of this Volatility Object object.
*/
LAObject* 
LAMathVolatility::clone() const
{
    try {
    	LAMathVolatility*	pVol = new LAMathVolatility(*this);
    	return pVol;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
    @brief get Interpolation
			
	@return Interpolation
*/
const LAPriceDataInterpolation&
LAMathVolatility::getInterpolation() const	
{
	return dynamic_cast<const LAPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get Interpolation
			
	@return Interpolation. The setting of interpolation method is also possible.
*/
LAPriceDataInterpolation&
LAMathVolatility::getInterpolation() 
{
	return dynamic_cast<LAPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get volatility function
			
	@return volatility function   get volatility function.
*/

LAFunctionBase*
LAMathVolatility::getVolatilityFunc(unsigned i, unsigned int j) const
{
	int type = dynamic_cast<LADataInt&>(mpVolType->get()).get();
	
	if (type == DATA_SCALAR)
	{
		double ret = dynamic_cast<const LADataDouble&>(mpVol_d->get()).get();
		return  dynamic_cast<LAFunctionBase*>(LAConstant(ret).clone());
	}
	else if (type == FUNC_SCALAR)
	{
		const LAFunctionBase& method = dynamic_cast<const LAPriceDataFunction&>(mpVol_f->get()).getFunction();
		return  dynamic_cast<LAFunctionBase*>(method.clone());
	}	

	
	else if (type == T_I_DATA_MATRIX)
	{
		unsigned int factornum = dynamic_cast<const LADataInt&>(mpFactorNum->get()).get();
		const DoubleMatrix& mat = dynamic_cast<LADataDoubleMatrix&>(mpVol_mat_d->get()).get();
		if (i * factornum + j >= mat.size())
		{
			//error
			throw LACoreInvalidData("Suffix is over Size", __FILE__, __LINE__);
		}
		const DoubleArray& data = mat[i * factornum + j];
		const DoubleArray& time = dynamic_cast<LADataDoubles&>(mpGrid_t->get()).get();		
		LA1DDataSet method;
		if (data.size() < time.size())
		{
			DoubleArray _time = time;
			_time.resize(data.size());
			method.set(_time, data);
		}
		else if (data.size() > time.size())
		{
			//error
			throw LACoreInvalidData("gird_t size is smaller than data size", __FILE__, __LINE__);
		}
		else
			method.set(time, data);
		const LAInterpolationBase& inter = dynamic_cast<const LAPriceDataInterpolation&>(mpInter->get()).getMethod();
		method.setInterpolation(inter);
		return  dynamic_cast<LAFunctionBase*>(method.clone());
	}
	
	else if (type == T_I_FUNC_MATRIX)
	{
		unsigned int factornum = dynamic_cast<const LADataInt&>(mpFactorNum->get()).get();
		const LAPriceDataFunctions& attr = dynamic_cast<const LAPriceDataFunctions&>(mpVol_vec_f->get());
		if (i * factornum + j >= attr.getSize())
		{
			//error
			throw LACoreInvalidData("Suffix is over Size", __FILE__, __LINE__);
		}		
		const LAFunctionBase& method = attr[i * factornum + j];
		return  dynamic_cast<LAFunctionBase*>(method.clone());
	}

	else if (type == T_I_FUNC_VECTOR)
	{
		//unsigned int factornum = dynamic_cast<const LADataInt&>(mpFactorNum->get()).get();
		const LAPriceDataFunctions& attr = dynamic_cast<const LAPriceDataFunctions&>(mpVol_vec_f->get());
		if (j >= attr.getSize())
		{
			//error
			throw LACoreInvalidData("Suffix is over Size", __FILE__, __LINE__);
		}			
		const LAFunctionBase& method = attr[j];
		const DoubleArray& grid_T = dynamic_cast<LADataDoubles&>(mpGrid_T->get()).get();
		DoubleArray x(2);
		x[1] = grid_T.at(i);
		LAFunctionBase* pfunc = dynamic_cast<LAFunctionBase*>(method.clone());
		pfunc->operator ()(0, x);
		return pfunc; 	
	}

	if (j != 0)
	{
		//error
		throw LACoreInvalidData("Suffix is over Size(j must be 0)", __FILE__, __LINE__);
	}	
	
	if (type == T_DATA_MATRIX)
	{
		const DoubleArray& data = dynamic_cast<LADataDoubleMatrix&>(mpVol_mat_d->get()).get()[i];
		const DoubleArray& time = dynamic_cast<LADataDoubles&>(mpGrid_t->get()).get();
		LA1DDataSet method;
		if (data.size() < time.size())
		{
			DoubleArray _time = time;
			_time.resize(data.size());
			method.set(_time, data);
		}
		else if (data.size() > time.size())
		{
			//error
			throw LACoreInvalidData("grid_t size is smaller than data size", __FILE__, __LINE__);
		}
		else
			method.set(time, data);
		const LAInterpolationBase& inter = dynamic_cast<const LAPriceDataInterpolation&>(mpInter->get()).getMethod();
		method.setInterpolation(inter);
		return  dynamic_cast<LAFunctionBase*>(method.clone());
	}
	else if (type == T_FUNC_VECTOR)
	{
		const LAFunctionBase& method = dynamic_cast<const LAPriceDataFunctions&>(mpVol_vec_f->get())[i];
		return  dynamic_cast<LAFunctionBase*>(method.clone());
	}
	else if (type == T_FUNC_SCALAR)
	{
		const LAFunctionBase& method = dynamic_cast<const LAPriceDataFunction&>(mpVol_f->get()).getFunction();
		const DoubleArray& grid_T = dynamic_cast<LADataDoubles&>(mpGrid_T->get()).get();
		DoubleArray x(2);
		x[1] = grid_T.at(i);
		LAFunctionBase* pfunc = dynamic_cast<LAFunctionBase*>(method.clone());
		pfunc->operator ()(0, x);
		return pfunc; 
	}
	
	if (i != 0)
	{
		throw LACoreInvalidData("Suffix is over Size(i must be 0)", __FILE__, __LINE__);
	}
	
	else if (type == DATA_VECTOR)
	{
		const DoubleArray& data = dynamic_cast<LADataDoubles&>(mpVol_vec_d->get()).get();
		const DoubleArray& time = dynamic_cast<LADataDoubles&>(mpGrid_t->get()).get();
		LA1DDataSet method;
		method.set(time, data);
		const LAInterpolationBase& inter = dynamic_cast<const LAPriceDataInterpolation&>(mpInter->get()).getMethod();
		method.setInterpolation(inter);
		return  dynamic_cast<LAFunctionBase*>(method.clone());			
	}
	else
	{
		//error
		throw LACoreInvalidData("Not support InputType", __FILE__, __LINE__);
	}
	return NULL;

}

const LAFunctionBase*
LAMathVolatility::getVolatilityFunc() const
{
	int type = dynamic_cast<LADataInt&>(mpVolType->get()).get();
	
	if (type == FUNC_SCALAR)
	{
		const LAFunctionBase* method = &(dynamic_cast<const LAPriceDataFunction&>(mpVol_f->get()).getFunction());
		return method;
	}
	else if (type == T_FUNC_VECTOR)
	{
		const LAFunctionBase* method = &(dynamic_cast<const LAPriceDataFunctions&>(mpVol_vec_f->get())[0]);
		return method;
		
	}
	else
	{
		throw LACoreInvalidData("Error in LAMathVolatility",__FILE__,__LINE__);
	}


}


/*!
    @brief set Volatility from grid_t and Volatility of DoubleMatrix vector (T*i*t)
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of DoubleMatrix vector

*/

void
LAMathVolatility::setVolatility(const DoubleArray& grid_t, const vector<DoubleMatrix>& vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(T_I_DATA_MATRIX);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(vol[0].size());
	

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
	dynamic_cast<LADataDoubleMatrix&>(mpVol_mat_d->get()).set(mat);
}


/*!
    @brief set Volatility as function matrix (T*i*f(t))
	
	@param[in] vol		Volatility as function matrix

*/

void
LAMathVolatility::setVolatility(const vector<vector<LAFunctionBase*> >& vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(T_I_FUNC_MATRIX);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(vol[0].size());
	
	LAPriceDataFunctions& attr = dynamic_cast<LAPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();
	for (unsigned int i = 0; i < vol.size(); i++)
		for (unsigned int j = 0; j < vol[i].size(); j++)
		{
			LAString name = getName().get();
			name += "_";
			name += LADataInt(i).convertToString();
			name += "_";
			name += LADataInt(j).convertToString();
			attr.push_back(vol[i][j], name);
		}


}


/*!
    @brief set Volatility from grid_t and Volatility of Function vector (i*f(t,T))
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of Function vector

*/

void
LAMathVolatility::setVolatility(const DoubleArray& grid_T, const vector<LAFunctionBase*>& vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(T_I_FUNC_VECTOR);
	dynamic_cast<LADataDoubles&>(mpGrid_T->get()).set(grid_T);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(vol.size());
	
	DoubleArray x(2);
	LAPriceDataFunctions& attr = dynamic_cast<LAPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();

	for (unsigned int j = 0; j < vol.size(); j++)
	{
		LAString name = getName().get();
		name += "_0_";
		name += LADataInt(j).convertToString();
		attr.push_back(vol[j], name);
	}


/*	for (unsigned int i = 0; i < grid_T.size(); i++)
	{
		x[1] = grid_T[i];
		for (unsigned int j = 0; j < vol.size(); j++)
		{
			LAString name = getName().get();
			name += "_";
			name += LADataInt(i).convertToString();
			name += "_";
			name += LADataInt(j).convertToString();
			LAFunctionBase* method;
			
			if (i == 0) method = vol[j];
			else method = dynamic_cast<LAFunctionBase*>(vol[j]->clone());
			
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
LAMathVolatility::setVolatility(const DoubleArray& grid_t, const DoubleMatrix& vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(T_DATA_MATRIX);
	dynamic_cast<LADataDoubles&>(mpGrid_t->get()).set(grid_t);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(1);
	dynamic_cast<LADataDoubleMatrix&>(mpVol_mat_d->get()).set(vol);
}
	
/*!
    @brief set Volatility as function vector (T*f(t))
	
	@param[in] vol		Volatility as function vector

*/

void
LAMathVolatility::setVolatility(const vector<LAFunctionBase*>& vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(T_FUNC_VECTOR);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(1);
	
	LAPriceDataFunctions& attr = dynamic_cast<LAPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();
	for (unsigned int i = 0; i < vol.size(); i++)
	{
		LAString name = getName().get();
		name += "_";
		name += LADataInt(i).convertToString();
		name += "_";
		name += LADataInt(0).convertToString();
			
		attr.push_back(vol[i], name);
	}
}

/*!
    @brief set Volatility from grid_t and Volatility of function (f(t,T))
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of function

*/
void
LAMathVolatility::setVolatility(const DoubleArray& grid_T, LAFunctionBase* vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(T_FUNC_SCALAR);
	dynamic_cast<LADataDoubles&>(mpGrid_T->get()).set(grid_T);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(1);
	LAPriceDataFunction& attr = dynamic_cast<LAPriceDataFunction&>(mpVol_f->get());
	LAString name = getName().get();
	name += "_0_0";	
	attr.setFunction(vol, name);

/*	LAPriceDataFunctions& attr = dynamic_cast<LAPriceDataFunctions&>(mpVol_vec_f->get());
	attr.clear();
	DoubleArray x(2);
	for (unsigned int i = 0; i < grid_T.size(); i++)
	{
		x[1] = grid_T[i];
		LAString name = getName().get();
		name += "_";
		name += LADataInt(i).convertToString();
		name += "_";
		name += LADataInt(0).convertToString();
		LAFunctionBase* method;
			
		if (i == 0) method = vol;
		else method = dynamic_cast<LAFunctionBase*>(vol->clone());
			
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
LAMathVolatility::setVolatility(const DoubleArray& grid_t, const DoubleArray& vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(DATA_VECTOR);
	dynamic_cast<LADataDoubles&>(mpGrid_t->get()).set(grid_t);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(1);
	dynamic_cast<LADataDoubles&>(mpVol_vec_d->get()).set(vol);
}


/*!
    @brief set Volatility as function for fx (f(t))
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility of vector

*/


void
LAMathVolatility::setVolatility(LAFunctionBase* vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(FUNC_SCALAR);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(1);
	LAPriceDataFunction& attr = dynamic_cast<LAPriceDataFunction&>(mpVol_f->get());
	LAString name = getName().get();
	name += "_0_0";	
	attr.setFunction(vol, name);
}

/*!
    @brief set Volatility as double
	
	@param[in] grid_t	grid of t
	@param[in] vol		Volatility as double

*/
void
LAMathVolatility::setVolatility(double vol)
{
	dynamic_cast<LADataInt&>(mpVolType->get()).set(DATA_SCALAR);
	dynamic_cast<LADataInt&>(mpFactorNum->get()).set(1);
	dynamic_cast<LADataDouble&>(mpVol_d->get()).set(vol);
}



/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
LAMathVolatility::remove(
	const LAString& dataName)
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
	LAObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
LAMathVolatility::reset(void)
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
LAMathVolatility::setInitialValue(const DoubleArray& vol0vec)
{
	dynamic_cast<LADataDoubles&>(mpInitialValues->get()).set(vol0vec);
}

/*!
    @brief set InitialValue as double
	
	@param[in] vol0		Initial value of stochastic factor as double
*/
void
LAMathVolatility::setInitialValue(const double vol0)
{
	dynamic_cast<LADataDouble&>(mpInitialValue->get()).set(vol0);
}

/*!
	@brief get initial value of stochastic factor
	@return reference to holder class 
*/
const double
LAMathVolatility::getInitialValue() const
{
	int type = dynamic_cast<LADataInt&>(mpVolType->get()).get();
	
	if (type == FUNC_SCALAR)
	{
		const double ret = dynamic_cast<const LADataDouble&>(mpInitialValue->get()).get();
		return ret;
	}
	else if (type == T_FUNC_VECTOR)
	{
		const double ret = dynamic_cast<const LADataDoubles&>(mpInitialValues->get()).get()[0];
		return ret;		
	}
	else
	{
		throw LACoreInvalidData("Error in LAMathVolatility",__FILE__,__LINE__);
	}


}


/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy MFSingleCredit
	@param[in] e copy source
	@return reference to this object
*/
LAObject&
LAMathVolatility::copy(
	const LAObject& e)
{
	if (this == &e) return *this;

	LAObject::copy(e);
	if (!e.isTypeOf(ENTITY_IR_VOLATILITY))
	{
		LAString err = "Assignement error for LAMathVolatility : from ";
		err += LAString(e.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
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
LADataHolder&
LAMathVolatility::add(const LAString& name)
{
	LADataInstance* dataInstance = getDataInstance();
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	const LADataHolder& dh = dm.getData(name);
	return LAObject::add(name, dh);
}
