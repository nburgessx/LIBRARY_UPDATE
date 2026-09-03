/*! @file
    @brief Source code for class to represent correlation.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(AQLDataString)<BR>
			2.CALIBRATION_DATA_ASOFDATE(AQLDataDate)<BR>
        	3.IR_CALIBRATION_DATA_ISCORINPUT(AQLDataBool)<BR>
			4.CALIBRATION_DATA_INTERPOLATION(AQLPriceDataInterpolation)<BR>
			5.IR_CALIBRATION_DATA_FACTORNUM_AFTER(LAPriceInt)<BR>
			6.IR_CALIBRATION_DATA_FACTORNUM_BEFORE(LAPriceInt)<BR>
			7.IR_CALIBRATION_DATA_ISOPTIM(AQLDataBool)<BR>
			8.IR_CALIBRATION_DATA_ISMULTIVOL(AQLDataBool)<BR>
			9.IR_CALIBRATION_DATA_GRID_LARGE_T(AQLDataDoubles)<BR>
			10.IR_CALIBRATION_DATA_GRID_SMALL_T(AQLDataDoubles)<BR>
			11.IR_CALIBRATION_DATA_ISDATAINPUT(AQLDataBool)<BR>
			12.IR_CALIBRATION_DATA_CORRELATIONDATA(AQLDataDoubleMatrix)<BR>
			13.IR_CALIBRATION_DATA_FACTORLOADINGDATA(AQLDataDoubleMatrix)<BR>
			14.IR_CALIBRATION_DATA_FUNCTIONS(LADataFunctions)<BR>
			15.IR_CALIBRATION_DATA_OPTWEIGHT(AQLDataDoubleMatrix)<BR>
			16.IR_CALIBRATION_DATA_CORFACTORDATA(AQLDataDoubleMatrix)<BR>

*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathCorrelation.h"
#include "AQLMathDefine.h"

#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataManager.h"
#include "AQLDataProcedure.h"
#include "AQLDataInstance.h"

#include "AQLPriceDataFunctions.h"
#include "AQLPriceDataInterpolation.h"

#include "AQLBasic.h"
#include "AQLConstant.h"
#include "AQL1DDataSet.h"
#include "AQLCombinationFunc.h"
#include "AQLMatrix.h"
#include "AQLOptimumBFGS.h"
#include "AQLCholeskyDecompSC.h"

#include <functional>
#include <cmath>
#include <algorithm>
#include <map>

using namespace std;

const double INFINITESIMAL = 1E-7; 

/*!
    @brief default constructor

	@param[in] dataInstance pointer of AQLDataInstance object

*/
LAMathCorrelation::LAMathCorrelation(AQLDataInstance* dataInstance) : 
				AQLObject()
{
	setDataInstance(dataInstance);
	AQLPriceDataManager& dm = dataInstance->getDataMaster();

	dm.setData(CALIBRATION_DATA_NAME, DATA_STRING);
	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
	dm.setData(IR_CALIBRATION_DATA_ISCORINPUT, DATA_BOOL);
	dm.setData(CALIBRATION_DATA_INTERPOLATION, DATA_INTERPOLATION);
	dm.setData(IR_CALIBRATION_DATA_FACTORNUM_AFTER, DATA_INT);
	dm.setData(IR_CALIBRATION_DATA_FACTORNUM_BEFORE, DATA_INT);
	dm.setData(IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T, DATA_BOOLS);
	dm.setData(IR_CALIBRATION_DATA_ISOPTIM, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_ISMULTIVOL, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_GRID_LARGE_T, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_GRID_SMALL_T, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_ISDATAINPUT, DATA_BOOL);
	dm.setData(IR_CALIBRATION_DATA_CORRELATIONDATA, DATA_DOUBLE_MATRIX);
	dm.setData(IR_CALIBRATION_DATA_FACTORLOADINGDATA, DATA_DOUBLE_MATRIX);
	dm.setData(IR_CALIBRATION_DATA_FUNCTIONS, DATA_FUNCTIONS);
	dm.setData(IR_CALIBRATION_DATA_OPTWEIGHT, DATA_DOUBLE_MATRIX);
	dm.setData(IR_CALIBRATION_DATA_CORFACTORDATA, DATA_DOUBLE_MATRIX);
	dm.setData(IR_CALIBRATION_DATA_EIGENVALUES, DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_EIGENVECTORS, DATA_DOUBLE_MATRIX);


	mpName			  = &add(CALIBRATION_DATA_NAME);
	mpIsCorInput	  = &add(IR_CALIBRATION_DATA_ISCORINPUT);
	mpInter			  = &add(CALIBRATION_DATA_INTERPOLATION);
	mpFactorNum_after = &add(IR_CALIBRATION_DATA_FACTORNUM_AFTER);
	mpFactorNum_before= &add(IR_CALIBRATION_DATA_FACTORNUM_BEFORE);
	mpIsExtraGrid_T	  = &add(IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T);
	mpIsOptim		  = &add(IR_CALIBRATION_DATA_ISOPTIM);
	mpIsMultiVol	  = &add(IR_CALIBRATION_DATA_ISMULTIVOL);
	mpGrid_T		  = &add(IR_CALIBRATION_DATA_GRID_LARGE_T);
	mpGrid_t		  = &add(IR_CALIBRATION_DATA_GRID_SMALL_T);
	mpIsDataInput	  = &add(IR_CALIBRATION_DATA_ISDATAINPUT);
	mpCorrelation	  = &add(IR_CALIBRATION_DATA_CORRELATIONDATA);
	mpFactorLoading	  = &add(IR_CALIBRATION_DATA_FACTORLOADINGDATA);
	mpFunctions		  = &add(IR_CALIBRATION_DATA_FUNCTIONS);
	mpOptWeight		  = &add(IR_CALIBRATION_DATA_OPTWEIGHT);
	mpCorFactors	  = &add(IR_CALIBRATION_DATA_CORFACTORDATA);
	mpEigenValues	  = &add(IR_CALIBRATION_DATA_EIGENVALUES);
	mpEigenVectors	  = &add(IR_CALIBRATION_DATA_EIGENVECTORS);

}
/*!
    @brief copy constructor

	@param[in] fx LAMathCorrelation object
*/
LAMathCorrelation::LAMathCorrelation(
	const LAMathCorrelation& cor) : 
	AQLObject(cor)
{

	mpName			  = &getData(CALIBRATION_DATA_NAME);
	mpIsCorInput	  = &getData(IR_CALIBRATION_DATA_ISCORINPUT);
	mpInter			  = &getData(CALIBRATION_DATA_INTERPOLATION);
	mpFactorNum_after = &getData(IR_CALIBRATION_DATA_FACTORNUM_AFTER);
	mpFactorNum_before= &getData(IR_CALIBRATION_DATA_FACTORNUM_BEFORE);
	mpIsExtraGrid_T   = &getData(IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T);
	mpIsOptim		  = &getData(IR_CALIBRATION_DATA_ISOPTIM);
	mpIsMultiVol	  = &getData(IR_CALIBRATION_DATA_ISMULTIVOL);
	mpGrid_T		  = &getData(IR_CALIBRATION_DATA_GRID_LARGE_T);
	mpGrid_t		  = &getData(IR_CALIBRATION_DATA_GRID_SMALL_T);
	mpIsDataInput	  = &getData(IR_CALIBRATION_DATA_ISDATAINPUT);
	mpCorrelation	  = &getData(IR_CALIBRATION_DATA_CORRELATIONDATA);
	mpFactorLoading	  = &getData(IR_CALIBRATION_DATA_FACTORLOADINGDATA);
	mpFunctions		  = &getData(IR_CALIBRATION_DATA_FUNCTIONS);
	mpOptWeight		  = &getData(IR_CALIBRATION_DATA_OPTWEIGHT);
	mpCorFactors	  = &getData(IR_CALIBRATION_DATA_CORFACTORDATA);
	mpEigenValues	  = &getData(IR_CALIBRATION_DATA_EIGENVALUES);
	mpEigenVectors	  = &getData(IR_CALIBRATION_DATA_EIGENVECTORS);

}

/*!
    @brief destructor
*/
LAMathCorrelation::~LAMathCorrelation()
{

}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
LAMathCorrelation::getType(void) const
{
	return ENTITY_IR_CORRELATION;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
LAMathCorrelation::isTypeOf(object_t id) const
{
	return (id == ENTITY_IR_CORRELATION ? true : AQLObject::isTypeOf(id));
}


/*!
    @brief get this Correlation Object-name.
	@return name
*/
const AQLDataString&	
LAMathCorrelation::getName() const	
{
	return dynamic_cast<const AQLDataString&>(mpName->get());
}
/*!
    @brief get this Correlation Object-name.The setting of name is also possible.
	@return name
*/
AQLDataString&	
LAMathCorrelation::getName()
{
	return dynamic_cast<AQLDataString&>(mpName->get());
}

/*!
    @brief get interpolation method
	@return interpolation method
*/
const AQLPriceDataInterpolation&
LAMathCorrelation::getInterpolation() const
{
	return dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get interpolation method. The setting of interpolation method is also possible. 
	@return interpolation method
*/
AQLPriceDataInterpolation&
LAMathCorrelation::getInterpolation()
{
	return dynamic_cast<AQLPriceDataInterpolation&>(mpInter->get());
}

/*!
    @brief get multi volatility flag
	@return multi volatility flag
*/
const AQLDataBool&
LAMathCorrelation::getIsMultiVol() const
{
	return dynamic_cast<const AQLDataBool&>(mpIsMultiVol->get());
}

/*!
    @brief get multi volatility or not. The setting of multi volatility flag is also possible. 
	@return multi volatility flag
*/
AQLDataBool&
LAMathCorrelation::getIsMultiVol()
{
	return dynamic_cast<AQLDataBool&>(mpIsMultiVol->get());
}

/*!
    @brief get T (rate reset time) grid
	@return T (rate reset time) grid
*/
const AQLDataDoubles&
LAMathCorrelation::getTGrid() const
{
	return dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get());
}
/*!
    @brief get T (rate reset time) grid. The setting of T grid is also possible. 
	@return T (rate reset time) grid
*/
AQLDataDoubles&
LAMathCorrelation::getTGrid()
{
	return dynamic_cast<AQLDataDoubles&>(mpGrid_T->get());
}

/*!
    @brief get Optimize Weight.

	@return Optimize Weight
*/
const AQLDataDoubleMatrix&	
LAMathCorrelation::getOptWeight() const	
{
	return dynamic_cast<const AQLDataDoubleMatrix&>(mpOptWeight->get());
}
/*!
    @brief get Optimize Weight.

	@return Optimize Weight
*/
AQLDataDoubleMatrix&	
LAMathCorrelation::getOptWeight()
{
	return dynamic_cast<AQLDataDoubleMatrix&>(mpOptWeight->get());
}

/*!
    @brief get cor_factors as double matrix

    @return cor_factors as double matrix
*/
const AQLDataDoubleMatrix&
LAMathCorrelation::getCorFactors() const
{
    return dynamic_cast<AQLDataDoubleMatrix&>(mpCorFactors->get());
}

/*!
    @brief get cor_factors as double matrix

    @return cor_factors as double matrix
*/
AQLDataDoubleMatrix&
LAMathCorrelation::getCorFactors()
{
    return dynamic_cast<AQLDataDoubleMatrix&>(mpCorFactors->get());
}

/*!
@brief get extra grid_T flag as bool vector

@return extra grid_T flag as bool vector
*/
const AQLDataBools&
LAMathCorrelation::getIsExtraTGrid() const
{
	return dynamic_cast<AQLDataBools&>(mpIsExtraGrid_T->get());
}

/*!
@brief get extra grid_T flag as bool vector

@return extra grid_T flag as bool vector
*/
AQLDataBools&
LAMathCorrelation::getIsExtraTGrid()
{
	return dynamic_cast<AQLDataBools&>(mpIsExtraGrid_T->get());
}

/*!
    @brief get correlation as function matrix
    
    @return correlation as function matrix
*/

vector<vector<AQLFunctionBase*> >
LAMathCorrelation::getCorrelationFunc() const
{
	bool isCorInput = dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).get();	
	if (!isCorInput)//Factor Loading input
		return calcCorrelationFromFactorLoading();
	else
	{
		unsigned int bi = dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).get();
		if (mpFactorNum_after->isDefined() && !mpFactorNum_after->isNull())
		{
			unsigned int ai = dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).get();
			if (ai != bi)//factor reduction
				return calcCorrelationFromFactorLoading();
		}
	} 
		
	return calcCorrelationFromCorrelation();

}

/*!
    @brief get FactorLoading
    
    @return FactorLoading
*/

vector<DoubleMatrix>
LAMathCorrelation::getFactorLoading(const DoubleArray& grid_t) const
{
	bool isCorInput = dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).get();	
	bool isMultiVol = dynamic_cast<const AQLDataBool&>(mpIsMultiVol->get()).get();	

	unsigned int ai = dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).get();
	unsigned int bi = dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).get();

	

	UintArray size_array(bi, grid_t.size());
	if (!isMultiVol)
	{
		DoubleArray grid_T = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();	
		if (grid_T.at(0) == 0.0) grid_T.erase(grid_T.begin());
		
		if (grid_T.size() != bi)
		{
			//error
			throw AQLCoreInvalidData("Grid_T size must be same as FactorNumBeforeReduction", __FILE__, __LINE__);
		}
		
		for (unsigned int i = 0; i < bi; i++)
		{
			unsigned j = 0;
			for (; j < grid_t.size(); j++)
			{
				if (grid_t[j] + INFINITESIMAL >= grid_T[i]) break;
			}
			size_array[i] = j;
		}
	}

	//return variable
	vector<DoubleMatrix> ret(bi);
	for (unsigned int i = 0; i < bi; i++)
	{
		ret[i].resize(ai);
		for (unsigned int j = 0; j < ai; j++)
			ret[i][j].resize(size_array[i]);
	}

	if (isCorInput)
	{
		if (mpFactorLoading->isNull())
		{
			//error
			throw AQLCoreInvalidData("Factor Loading is not calculated yet", __FILE__, __LINE__);
		}
		
		const DoubleMatrix& mat = dynamic_cast<const AQLDataDoubleMatrix&>(mpFactorLoading->get()).get();
		AQL1DDataSet method;
		const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
		method.setInterpolation(inter);

		DoubleArray timegrid;
		if (!mpGrid_t->isNull())
			timegrid = dynamic_cast<const AQLDataDoubles&>(mpGrid_t->get()).get();	
		else
		{
			timegrid = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();
			if (timegrid[0] != 0.0) timegrid.insert(timegrid.begin(), 0.0);
		}
		for (unsigned int i = 0; i < bi; i++)
		{
			DoubleArray _timegrid = timegrid;			
			if (!isMultiVol) _timegrid.resize(mat[i * ai].size());
			for (unsigned int j = 0; j < ai; j++)
			{
				method.set(_timegrid, mat[i * ai + j]);				
				for (unsigned int k = 0; k < size_array[i]; k++)
					ret[i][j][k] = method(grid_t[k]);
			}	
		}
		return ret;
	}
	
	//factor loading input case
	if (!mpFactorLoading->isNull())//data case
	{
		const DoubleMatrix& mat = dynamic_cast<const AQLDataDoubleMatrix&>(mpFactorLoading->get()).get();
		if (mat.back().size() == 1)
		{
			for (unsigned int i = 0; i < bi; i++)
				for (unsigned int j = 0; j < ai; j++)
					for (unsigned int k = 0; k < size_array[i]; k++)
						ret[i][j][k] = mat[i * ai + j][0];				
		}
		else
		{
			AQL1DDataSet method;
			const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
			method.setInterpolation(inter);
			DoubleArray data_grid_t;	
			if (!mpGrid_t->isNull())
				data_grid_t = dynamic_cast<const AQLDataDoubles&>(mpGrid_t->get()).get();	
			else
			{
				data_grid_t = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();
				if (data_grid_t[0] != 0.0) data_grid_t.insert(data_grid_t.begin(), 0.0);
			}			
			for (unsigned int i = 0; i < bi; i++)
				for (unsigned int j = 0; j < ai; j++)
				{
					DoubleArray _data_grid_t = data_grid_t;
					_data_grid_t.resize(mat[i * ai + j].size());
					method.set(_data_grid_t, mat[i * ai + j]);
					for (unsigned int k = 0; k < size_array[i]; k++)
						ret[i][j][k] = method(grid_t[k]);
				}
		}
	}
	else
	{
		const AQLPriceDataFunctions& attr = dynamic_cast<const AQLPriceDataFunctions&>(mpFunctions->get());
		for (unsigned int i = 0; i < bi; i++)
			for (unsigned int j = 0; j < ai; j++)
			{
				const AQLFunctionBase& method = attr[i * ai + j];
				
				for (unsigned int k = 0; k < size_array[i]; k++)
					ret[i][j][k] = method(grid_t[k]);
			}	
	}
			
	return ret;	

}

//  OPERATION 
/*!
    @brief set FactorLoading from grid_t and loading
  
	@param[in] grid_t	grid of t
	@param[in] loading  loading

*/
void
LAMathCorrelation::setFactorLoading(const DoubleArray& grid_t, const vector<DoubleMatrix>& loading)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(false);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(true);
	dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).set(loading.size());
	dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).set(loading[0].size());
	dynamic_cast<AQLDataDoubles&>(mpGrid_t->get()).set(grid_t);
	
	DoubleMatrix mat(loading.size() * loading[0].size());
	for (unsigned int i = 0; i < loading.size(); i++)
		for (unsigned int j = 0; j < loading[i].size(); j++)
			mat[i * loading[0].size() + j] = loading[i][j];		
	dynamic_cast<AQLDataDoubleMatrix&>(mpFactorLoading->get()).set(mat);
	
	mpFunctions->convertFromString(NULL_STR);
}
/*!
    @brief set FactorLoading from loading
  
	@param[in] loading  loading

*/
void
LAMathCorrelation::setFactorLoading(const DoubleMatrix& loading)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(false);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(true);
	dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).set(loading.size());
	dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).set(loading[0].size());
	dynamic_cast<AQLDataDoubleMatrix&>(mpCorFactors->get()).set(loading);
	
	DoubleMatrix mat(loading.size() * loading[0].size());
	bool isMultiVol = dynamic_cast<const AQLDataBool&>(mpIsMultiVol->get()).get();	
	if (isMultiVol)
	{
		for (unsigned int i = 0; i < loading.size(); i++)
			for (unsigned int j = 0; j < loading[i].size(); j++)
				mat[i * loading[0].size() + j].push_back(loading[i][j]);
	}
	else
	{
		for (unsigned int i = 0; i < loading.size(); i++)
		{
			for (unsigned int j = 0; j < loading[i].size(); j++)
			{
				mat[i * loading[0].size() + j].resize(i + 1);
				for (unsigned int k = 0; k <= i; k++)
					mat[i * loading[0].size() + j][k] = loading[i - k][j];
			}
		}
		mpGrid_t->convertFromString(NULL_STR);
	}
	dynamic_cast<AQLDataDoubleMatrix&>(mpFactorLoading->get()).set(mat);
	
	mpFunctions->convertFromString(NULL_STR);
}
/*!
    @brief set FactorLoading from loading as function matrix
  
	@param[in] loading  loading as function matrix

*/
void
LAMathCorrelation::setFactorLoading(const vector<vector<AQLFunctionBase*> >& loading)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(false);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(false);
	dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).set(loading.size());
	dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).set(loading[0].size());
	
	AQLPriceDataFunctions& attr = dynamic_cast<AQLPriceDataFunctions&>(mpFunctions->get());
	attr.clear();
	for (unsigned int i = 0; i < loading.size(); i++)
		for (unsigned int j = 0; j < loading[i].size(); j++)
		{
			AQLString name = getName().get();
			name += "_";
			name += AQLDataInt(i).convertToString();
			name += "_";
			name += AQLDataInt(j).convertToString();
			attr.push_back(loading[i][j], name);
		}

	mpFactorLoading->convertFromString(NULL_STR);

}
/*!
    @brief set Correlation from grid_t and correlation of DoubleMatrix vector
	
	@param[in] grid_t	grid of t
	@param[in] cor		correlation of DoubleMatrix vector

*/
void
LAMathCorrelation::setCorrelation(const DoubleArray& grid_t, const vector<DoubleMatrix>& cor)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(true);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(true);
	dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).set(cor.size());
	dynamic_cast<AQLDataDoubles&>(mpGrid_t->get()).set(grid_t);

	DoubleMatrix mat(cor.size() * cor.size());
	for (unsigned int i = 0; i < cor.size(); i++)
		for (unsigned int j = 0; j < cor[i].size(); j++)
			mat[i * cor.size() + j] = cor[i][j];		
	dynamic_cast<AQLDataDoubleMatrix&>(mpCorrelation->get()).set(mat);

	mpFunctions->convertFromString(NULL_STR);
	mpFactorLoading->convertFromString(NULL_STR);

}
/*!
    @brief set Correlation as DoubleMatrix
	
	@param[in] cor	Correlation as DoubleMatrix

*/
void
LAMathCorrelation::setCorrelation(const DoubleMatrix& cor)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(true);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(true);
	dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).set(cor.size());

	DoubleMatrix mat(cor.size() * cor.size());
	for (unsigned int i = 0; i < cor.size(); i++)
		for (unsigned int j = 0; j < cor[i].size(); j++)
			mat[i * cor.size() + j].push_back(cor[i][j]);
	dynamic_cast<AQLDataDoubleMatrix&>(mpCorrelation->get()).set(mat);

	mpGrid_t->convertFromString(NULL_STR);
	mpFunctions->convertFromString(NULL_STR);
	mpFactorLoading->convertFromString(NULL_STR);


}
/*!
    @brief set Correlation as function matrix
	
	@param[in] cor	Correlation as function matrix

*/
void
LAMathCorrelation::setCorrelation(const vector<vector<AQLFunctionBase*> >& cor)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(true);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(false);
	dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).set(cor.size());

	AQLPriceDataFunctions& attr = dynamic_cast<AQLPriceDataFunctions&>(mpFunctions->get());
	attr.clear();
	for (unsigned int i = 0; i < cor.size(); i++)
		for (unsigned int j = 0; j < cor[i].size(); j++)
		{
			AQLString name = getName().get();
			name += "_";
			name += AQLDataInt(i).convertToString();
			name += "_";
			name += AQLDataInt(j).convertToString();
			attr.push_back(cor[i][j], name);
		}

	mpCorrelation->convertFromString(NULL_STR);
	mpFactorLoading->convertFromString(NULL_STR);

}
/*!
    @brief set Correlation as function 
	
	@param[in] cor	Correlation as function 

*/
void
LAMathCorrelation::setCorrelation(AQLFunctionBase* cor)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(true);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(false);

	AQLPriceDataFunctions& attr = dynamic_cast<AQLPriceDataFunctions&>(mpFunctions->get());
	attr.clear();
	AQLString name = getName().get();
	name += "_0_0";		
	attr.push_back(cor, name);

	mpCorrelation->convertFromString(NULL_STR);
	mpFactorLoading->convertFromString(NULL_STR);


}
/*!
    @brief set Correlation as zero  
	
	@param[in] factornum factor number

*/
void
LAMathCorrelation::setCorrelation(unsigned int factornum)
{
	dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).set(true);
	dynamic_cast<AQLDataBool&>(mpIsDataInput->get()).set(true);
	dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).set(factornum);
	

	DoubleMatrix mat(factornum * factornum);
	for (unsigned int i = 0; i < mat.size(); i++)
		mat[i].push_back(1.0);
	dynamic_cast<AQLDataDoubleMatrix&>(mpCorrelation->get()).set(mat);

	mpFunctions->convertFromString(NULL_STR);
	mpFactorLoading->convertFromString(NULL_STR);


}
/*!
    @brief calc FactorLoading
	
*/
void
LAMathCorrelation::calcFactorLoading() const
{
	bool isCorInput = dynamic_cast<AQLDataBool&>(mpIsCorInput->get()).get();	
	if (!isCorInput) return;

	bool isMultiVol = dynamic_cast<const AQLDataBool&>(mpIsMultiVol->get()).get();	
	bool isOptim = dynamic_cast<const AQLDataBool&>(mpIsOptim->get()).get();	
	
	unsigned int ai = dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).get();
	unsigned int bi = dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).get();

	DoubleArray timegrid;
	if (!mpGrid_t->isNull())
		timegrid = dynamic_cast<const AQLDataDoubles&>(mpGrid_t->get()).get();	
	else
	{
		timegrid = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();
		if (timegrid[0] != 0.0) timegrid.insert(timegrid.begin(), 0.0);
	}

	DoubleArray grid_T = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();	
	if (grid_T[0] == 0.0) grid_T.erase(grid_T.begin());		
	UintArray size_array(bi, timegrid.size());
	if (!isMultiVol)
	{
		if (grid_T.size() != bi)
		{
			//error
			throw AQLCoreInvalidData("Grid_T size must be same as FactorNumBeforeReduction", __FILE__, __LINE__);
		}

		unsigned j = 0;
		for (unsigned int i = 0; i < bi; i++)
		{
			for (; j < timegrid.size(); j++)
			{
				if (timegrid[j] + INFINITESIMAL >= grid_T[i]) break;
			}
			size_array[i] = j;
		}
	}

	DoubleMatrix ret(bi * ai);
	
	if (!mpCorrelation->isNull())//data case
	{
		const DoubleMatrix& mat = dynamic_cast<const AQLDataDoubleMatrix&>(mpCorrelation->get()).get();
		if (mat.back().size() == 1) // 2dim input
		{
			DoubleMatrix cor(bi);
			for (unsigned int i = 0; i < bi; i++)
			{
				cor[i].resize(bi);
				cor[i][i] = mat[i * bi + i][0];
				for (unsigned int j = 0; j < i; j++)
				{
					cor[i][j] = mat[i * bi + j][0];
					cor[j][i] = cor[i][j];
				}
			}
			DoubleMatrix loading;			
			calcFactorLoading(cor, loading, ai , isOptim);
			
			if (isMultiVol)
			{
				for (unsigned int i = 0; i < bi; i++)
					for (unsigned int j = 0; j < ai; j++)
						ret[i * ai + j].resize(size_array[i], loading[i][j]);
			}
			else
			{
				for (unsigned int i = 0; i < loading.size(); i++)
				{
					for (unsigned int j = 0; j < loading[i].size(); j++)
					{
						ret[i * loading[0].size() + j].resize(i + 1);
						for (unsigned int k = 0; k <= i; k++)
							ret[i * loading[0].size() + j][k] = loading[i - k][j];
					}
				}
			}
			dynamic_cast<AQLDataDoubleMatrix&>(mpFactorLoading->get()).set(ret);
			return;
		}
	}

	
	const vector<vector<AQLFunctionBase*> >& corfunc = calcCorrelationFromCorrelation();
	// for delete memory of corfunc out of scope of this method
	vector<vector<AQLCoreFunctionHolder> >  holder(corfunc.size());
	for (unsigned int i = 0; i < corfunc.size(); i++)
	{
		holder[i].resize(corfunc[i].size());
		for (unsigned int j = 0; j < corfunc[i].size(); j++)
			holder[i][j].set(corfunc[i][j], true);
	}
	
	for (unsigned int i = 0; i < bi; i++)
		for (unsigned int j = 0; j < ai; j++)
			ret[i * ai + j].resize(size_array[i], 0.0);

	for (unsigned int i = 0; i < timegrid.size(); i++)
	{
		unsigned int size = bi;
		if (!isMultiVol)
		{
			for (unsigned int j = 0; j < bi; j++, size--)
				if (timegrid[i] + INFINITESIMAL < grid_T[j]) break;
		}
		if (size == 0) break;

		//make correlation matrix
		DoubleMatrix cor(size);
		for (unsigned int j = 0; j < size; j++)
		{
			cor[j].resize(size);
			cor[j][j] = 1.0;
			for (unsigned int k = 0; k < j; k++)
			{
				cor[j][k] = corfunc[j + bi - size][k + bi - size]->operator ()(timegrid[i]);
				cor[k][j] = cor[j][k];
			}
		}
		DoubleMatrix loading;
		calcFactorLoading(cor, loading, ai, isOptim );
		for (unsigned int j = bi - size; j < bi; j++)
			for (unsigned int k = 0; k < ai; k++)
				ret[j * ai + k][i] = loading[j - bi + size][k];
	}
	dynamic_cast<AQLDataDoubleMatrix&>(mpFactorLoading->get()).set(ret);

}

/*!
    @brief calc FactorLoading
	
*/
DoubleMatrix
LAMathCorrelation::calcFactorLoading(const DoubleMatrix& corrMat, const DoubleMatrix& initialValue) const
{	
	unsigned int factorNum = dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).get();

	LAMathFactorLoadingFunction method;
	DoubleMatrix weightMat(corrMat.size(), DoubleArray(corrMat.size(), 1.));

	method.setOptWeight(weightMat);
	method.setCorrelation(corrMat);
	method.setFactornum(factorNum);

	DoubleArray x(corrMat.size() * (factorNum - 1));
	for (unsigned int i = 0; i < corrMat.size(); i++)
	{
		for (unsigned int j = 0; j < factorNum - 1; j++)
		{
			if (j==0)
			{
				x[j + i * (factorNum - 1)] = acos(initialValue[i][j]);
			}
			else
			{
				x[j + i * (factorNum - 1)] = acos(initialValue[i][j] / sin(x[j + i * (factorNum - 1) - 1]));
			}
		}
	}

	AQLOptimumBFGS bfgs;
	bfgs.findMinimum(method, x);

	return LAMathFactorLoadingFunction::fromthitaTob(x, corrMat.size(), factorNum);
}

/*!
    @brief calc Correlation From FactorLoading

    @return Correlation of function matrix
*/

vector<vector<AQLFunctionBase*> >
LAMathCorrelation::calcCorrelationFromFactorLoading() const
{
	unsigned int ai = dynamic_cast<AQLDataInt&>(mpFactorNum_after->get()).get();
	unsigned int bi = dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).get();
	vector<vector<AQLFunctionBase*> > ret(bi);
	for (unsigned int i = 0; i < bi; i++)
		ret[i].resize(bi);

	AQLConstant con1(1);
	if (!mpFactorLoading->isNull())
	{
		
		const DoubleMatrix& mat = dynamic_cast<const AQLDataDoubleMatrix&>(mpFactorLoading->get()).get();
		if (mat.back().size() == 1)
		{
			for (unsigned int i = 0; i < bi; i++)
			{
				ret[i][i] = dynamic_cast<AQLFunctionBase*>(con1.clone());
				for (unsigned int j = 0; j < i; j++)
				{
					double cor = 0.0;
					for (unsigned int k = 0; k < ai; k++)
						cor+= mat[i * ai + k][0] * mat[j * ai + k][0];												
					AQLConstant method(cor);
					ret[i][j] = dynamic_cast<AQLFunctionBase*>(method.clone());
					ret[j][i] = dynamic_cast<AQLFunctionBase*>(method.clone());
				}
			}
		}
		else
		{		
			AQL1DDataSet method;
			//interpolation
			const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
			method.setInterpolation(inter);		
			
			DoubleArray timegrid;
			if (!mpGrid_t->isNull())
				timegrid = dynamic_cast<const AQLDataDoubles&>(mpGrid_t->get()).get();	
			else
			{
				timegrid = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();
				if (timegrid[0] != 0.0) timegrid.insert(timegrid.begin(), 0.0);
			}

			for (unsigned int i = 0; i < bi; i++)
			{
				ret[i][i] = dynamic_cast<AQLFunctionBase*>(con1.clone());
				for (unsigned int j = 0; j < i; j++)
				{		
					DoubleArray cor(mat[j * ai].size(), 0);
					for (unsigned int k = 0; k < mat[j * ai].size(); k++)
					{
						for (unsigned int l = 0; l < ai; l++)
							cor[k] += mat[i * ai + l][k] * mat[j * ai + l][k];

					}
					
					DoubleArray _timegrid = timegrid;
					_timegrid.resize(cor.size());
					method.set(_timegrid, cor);
					ret[i][j] = dynamic_cast<AQLFunctionBase*>(method.clone());
					ret[j][i] = dynamic_cast<AQLFunctionBase*>(method.clone());
				}
			}
		}
	}
	else
	{
		const AQLPriceDataFunctions& attr = dynamic_cast<const AQLPriceDataFunctions&>(mpFunctions->get());
		for (unsigned int i = 0; i < bi; i++)
		{
			ret[i][i] = dynamic_cast<AQLFunctionBase*>(con1.clone());
			for (unsigned int j = 0; j < i; j++)
			{		
				AQLCombinationMethod method;
				for (unsigned int k = 0; k < ai; k++)
					method = method + attr[i * ai + k] * attr[j * ai + k];
				ret[i][j] = dynamic_cast<AQLFunctionBase*>(method.clone());
				ret[j][i] = dynamic_cast<AQLFunctionBase*>(method.clone());
			}
		}						

	}
		
	return ret;
}

/*!
    @brief calc Correlation From Correlation

    @return Correlation of function matrix
	
*/
vector<vector<AQLFunctionBase*> >
LAMathCorrelation::calcCorrelationFromCorrelation() const
{
	unsigned int bi = dynamic_cast<AQLDataInt&>(mpFactorNum_before->get()).get();
	vector<vector<AQLFunctionBase*> > ret(bi);
	for (unsigned int i = 0; i < ret.size(); i++)
		ret[i].resize(bi);
	
	AQLConstant con1(1);
	if (!mpCorrelation->isNull())//data case
	{
		const DoubleMatrix& mat = dynamic_cast<const AQLDataDoubleMatrix&>(mpCorrelation->get()).get();
		if (mat.back().size() == 1) // correlation does not depend t
		{
			bool isMultiVol = dynamic_cast<const AQLDataBool&>(mpIsMultiVol->get()).get();	
			if (isMultiVol)
			{
				AQLConstant method;
				for (unsigned int i = 0; i < ret.size(); i++)
				{
					ret[i][i] = dynamic_cast<AQLFunctionBase*>(con1.clone());
					for (unsigned int j = 0; j < i; j++)
					{
						method.set(mat[i * bi + j][0]);
						ret[i][j] = dynamic_cast<AQLFunctionBase*>(method.clone()); 
						ret[j][i] = dynamic_cast<AQLFunctionBase*>(method.clone()); 
					}
				}
			}
			else
			{
				AQLConstant con_func;
				AQL1DDataSet method;
				//interpolation
				const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
				method.setInterpolation(inter);
				
				//time grid
				DoubleArray time;
				time = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();
				if (time[0] != 0.0) time.insert(time.begin(), 0.0);
							
				for (unsigned int i = 0; i < ret.size(); i++)
				{
					ret[i][i] = dynamic_cast<AQLFunctionBase*>(con1.clone());
					for (unsigned int j = 0; j < i; j++)
					{
						if (j == 0)
						{
							con_func.set(mat[i * bi + j][0]);
							ret[i][j] = dynamic_cast<AQLFunctionBase*>(con_func.clone()); 
							ret[j][i] = dynamic_cast<AQLFunctionBase*>(con_func.clone()); 
							continue;
						}
						
						DoubleArray _time(j + 1), data(j + 1);
						for (unsigned int k = 0; k < j + 1; k++)
						{
							_time[k] = time[k];
							data[k] = mat[(i - k)* bi + j - k][0];
						}
						method.set(_time, data);
											
						
						ret[i][j] = dynamic_cast<AQLFunctionBase*>(method.clone()); 
						ret[j][i] = dynamic_cast<AQLFunctionBase*>(method.clone()); 
					}
				}					
			
			}
		}
		else
		{
			AQL1DDataSet method;
            //interpolation
			const AQLInterpolationBase& inter = dynamic_cast<const AQLPriceDataInterpolation&>(mpInter->get()).getMethod();
			method.setInterpolation(inter);
			
			//time grid
			DoubleArray time;
			if (!mpGrid_t->isNull())
				time = dynamic_cast<const AQLDataDoubles&>(mpGrid_t->get()).get();	
			else
			{
				time = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();
				if (time[0] != 0.0) time.insert(time.begin(), 0.0);
			}			
			
			for (unsigned int i = 0; i < ret.size(); i++)
			{
				ret[i][i] = dynamic_cast<AQLFunctionBase*>(con1.clone());
				for (unsigned int j = 0; j < i; j++)
				{
					if (mat[i * bi + j].size() < time.size())
					{
						DoubleArray _time = time;
						_time.resize(mat[i * bi + j].size());
						method.set(_time, mat[i * bi + j]);
					}
					else
						method.set(time, mat[i * bi + j]);
					
					ret[i][j] = dynamic_cast<AQLFunctionBase*>(method.clone()); 
					ret[j][i] = dynamic_cast<AQLFunctionBase*>(method.clone()); 
				}
			}

		}

	}
	else//function case
	{
		const AQLPriceDataFunctions& attr = dynamic_cast<const AQLPriceDataFunctions&>(mpFunctions->get());
		if (attr.getSize() == 1)//f(t,T1,T2) case
		{
			DoubleArray grid_T = dynamic_cast<const AQLDataDoubles&>(mpGrid_T->get()).get();	
			if (grid_T[0] == 0.0) grid_T.erase(grid_T.begin());
			if (grid_T.size() != bi)
			{
				//error
				throw AQLCoreInvalidData("Grid_T is something wrong", __FILE__, __LINE__);
			}
			DoubleArray tmp(3);
			for (unsigned int i = 0; i < bi; i++)
			{
				ret[i][i] = dynamic_cast<AQLFunctionBase*>(con1.clone());
				for (unsigned int j = 0; j < i; j++)
				{
					tmp[1] = grid_T[i];
					tmp[2] = grid_T[j];
					AQLFunctionBase& method = *dynamic_cast<AQLFunctionBase*>(attr[0].clone());
					method(0, tmp);
					ret[i][j] = &method;
					ret[j][i] = dynamic_cast<AQLFunctionBase*>(method.clone());
				}			
			}
		
		}
		else
		{
			if (attr.getSize() != bi * bi)
			{
				//error
				throw AQLCoreInvalidData("functions element size is not right", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < bi; i++)
				for (unsigned int j = 0; j < bi; j++)
					ret[i][j] = dynamic_cast<AQLFunctionBase*>(attr[i * bi + j].clone());
		}
	}

	
	return ret;

}
/*!
    @brief culc Principal Component

	@param[in] cor	Correlation
	@param[in] vec	Eigen Vector
	@param[in] val	Eigen Value
	@param[in] factornum factor number

*/
void
LAMathCorrelation::calcPCA(const DoubleMatrix& cor, DoubleMatrix& vec, DoubleArray& val, unsigned int factornum) const
{
	AQLMatrix mat(cor), mvec, mval;
    DoubleMatrix tempvec; 
	DoubleArray tempval;

	mat.eigenMatrix(mvec, mval);
	
	unsigned int i, j;

	tempvec.resize(cor.size());
	tempval.resize(cor.size());
	for (i = 0; i < cor.size(); i++)
	{
		tempvec[i].resize(cor.size());
		tempval[i] = mval.getValue(0, i);
		for (j = 0; j < cor.size(); j++)
			tempvec[i][j] = mvec.getValue(j, i);
	}
	
	if (factornum > cor.size())
		throw AQLCoreInvalidData("factornumber is bigger than correlation size", __FILE__, __LINE__);

#ifndef VISUAL_STUDIO_2010_ANALYTICS
	UintArray num(factornum);
	double max = tempval[0];
	for (i = 0; i < factornum; i++)
	{
		for (j = 0; j < tempval.size() - 1; j++)
		{
			if (tempval[j] < tempval[j + 1])
			{
				max = tempval[j + 1];
				num[i] = j + 1;
			}
		}
		vec.push_back(tempvec[num[i]]);
		val.push_back(tempval[num[i]]);
		tempval[num[i]] = 0.0;
	}
#else
	std::vector<std::pair<double, DoubleArray > > tmp_pair;
	for (size_t i = 0; i < cor.size(); i++)
	{
		tmp_pair.push_back(std::make_pair(tempval[i], tempvec[i]));
	}
	std::sort(tmp_pair.begin(), tmp_pair.end(), std::greater<std::pair<double, DoubleArray > >());

	for (i = 0; i < factornum; i++)
	{
		val.push_back(tmp_pair[i].first);
		vec.push_back(tmp_pair[i].second);
	}
#endif

}

/*!
    @brief calc FactorLoading

	@param[in] cor	Correlation
	@param[in] loading	loading
	@param[in] factornum factor number
	
*/
void
LAMathCorrelation::calcFactorLoading(const DoubleMatrix& cor, DoubleMatrix& loading, unsigned int factornum, bool isOptim) const
{
	LAMathFactorLoadingFunction method;
	DoubleArray		y(cor.size());
	DoubleMatrix	wmat(cor.size());
	unsigned int i, j;
	
	loading.clear();
	if(factornum > cor.size())
	{
		AQLMatrix mcor(cor), mat;
		mat = AQLCholeskyDecompSC::choleskyDecompositionSC(mcor);
		loading.resize(cor.size());
		for (i = 0; i < cor.size(); i++)
		{
			loading[i].resize(factornum);
			for (j = 0; j < factornum; j++)
				loading[i][j] = (j < cor.size()) ? mat.getValue(i, j): 0.0;
		}
		return;
	}
	if (!isOptim)
	{
		DoubleMatrix vec; 
		DoubleArray val;
		calcPCA(cor, vec, val, factornum);

		AQLDataDoubles& attrEVal = dynamic_cast<AQLDataDoubles&>(mpEigenValues->get()); 
		attrEVal.set(val);
		AQLDataDoubleMatrix& attrEVec = dynamic_cast<AQLDataDoubleMatrix&>(mpEigenVectors->get()); 
		attrEVec.set(vec);

		i = val.size();
		j = vec.size();
		for (i = 0; i < vec.size(); i++)
			j = vec[i].size();
		
		loading.resize(cor.size());
		for (i = 0; i < cor.size(); i++)
		{
			loading[i].resize(factornum);
			for (j = 0; j < factornum; j++)
				loading[i][j] = vec[j][i] * sqrt(val[j]);
		}
		return;
	}
	else
	{
		if (getOptWeight().isNull())
		{
			for (i = 0; i < cor.size(); i++)
				for (j = 0; j < cor.size(); j++)
				{
					y[j] = 1.0;
					wmat[i] = y;
				}
			AQLDataDoubleMatrix& attrDM = dynamic_cast<AQLDataDoubleMatrix&>(mpOptWeight->get()); 
			attrDM.set(wmat);
		}

		method.setOptWeight(getOptWeight().get());
		method.setCorrelation(cor);
		method.setFactornum(factornum);

		DoubleArray x(cor.size() * (factornum - 1));
		
		x[0] = 1.0;
		for (i = 1; i < x.size(); i++) x[i] = x[i - 1] + 1.0; 

		AQLOptimumBFGS bfgs;
		bfgs.findMinimum(method, x);

		DoubleMatrix b = LAMathFactorLoadingFunction::fromthitaTob(x, cor.size(), factornum);

		y.clear();
		y.resize(factornum);
		for (i = 0; i < cor.size(); i++)
		{
			for (j = 0; j < factornum; j++)	
				y[j] = b[i][j];
			loading.push_back(y);
		}
		return;
	}


}

/*!
    @brief Make copy(clone) of this FX Object object.
    @return pointer of this FX Object object.
*/
AQLObject* 
LAMathCorrelation::clone() const
{
    try 
	{
    	LAMathCorrelation*	pCor = new LAMathCorrelation(*this);
    	return pCor;
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
LAMathCorrelation::remove(
	const AQLString& dataName)
{
	if (dataName == CALIBRATION_DATA_NAME
		|| dataName == CALIBRATION_DATA_ASOFDATE
		|| dataName == IR_CALIBRATION_DATA_ISCORINPUT
		|| dataName == CALIBRATION_DATA_INTERPOLATION
		|| dataName == IR_CALIBRATION_DATA_FACTORNUM_AFTER
		|| dataName == IR_CALIBRATION_DATA_FACTORNUM_BEFORE
		|| dataName == IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T
		|| dataName == IR_CALIBRATION_DATA_ISOPTIM
		|| dataName == IR_CALIBRATION_DATA_ISMULTIVOL
		|| dataName == IR_CALIBRATION_DATA_GRID_LARGE_T
		|| dataName == IR_CALIBRATION_DATA_GRID_SMALL_T
		|| dataName == IR_CALIBRATION_DATA_ISDATAINPUT
		|| dataName == IR_CALIBRATION_DATA_CORRELATIONDATA
		|| dataName == IR_CALIBRATION_DATA_FACTORLOADINGDATA
		|| dataName == IR_CALIBRATION_DATA_FUNCTIONS
		|| dataName == IR_CALIBRATION_DATA_OPTWEIGHT
		|| dataName == IR_CALIBRATION_DATA_CORFACTORDATA)
	{
		return; 
	}
	AQLObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
LAMathCorrelation::reset(void)
{
	clear();
	mpName			  = &add(CALIBRATION_DATA_NAME);
	mpIsCorInput	  = &add(IR_CALIBRATION_DATA_ISCORINPUT);
	mpInter			  = &add(CALIBRATION_DATA_INTERPOLATION);
	mpFactorNum_after = &add(IR_CALIBRATION_DATA_FACTORNUM_AFTER);
	mpFactorNum_before= &add(IR_CALIBRATION_DATA_FACTORNUM_BEFORE);
	mpIsExtraGrid_T	  = &add(IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T);
	mpIsOptim		  = &add(IR_CALIBRATION_DATA_ISOPTIM);
	mpIsMultiVol	  = &add(IR_CALIBRATION_DATA_ISMULTIVOL);
	mpGrid_T		  = &add(IR_CALIBRATION_DATA_GRID_LARGE_T);
	mpGrid_t		  = &add(IR_CALIBRATION_DATA_GRID_SMALL_T);
	mpIsDataInput	  = &add(IR_CALIBRATION_DATA_ISDATAINPUT);
	mpCorrelation	  = &add(IR_CALIBRATION_DATA_CORRELATIONDATA);
	mpFactorLoading	  = &add(IR_CALIBRATION_DATA_FACTORLOADINGDATA);
	mpFunctions		  = &add(IR_CALIBRATION_DATA_FUNCTIONS);
	mpOptWeight		  = &add(IR_CALIBRATION_DATA_OPTWEIGHT);
	mpCorFactors	  = &add(IR_CALIBRATION_DATA_CORFACTORDATA);
	mpEigenValues	  = &add(IR_CALIBRATION_DATA_EIGENVALUES);
	mpEigenVectors	  = &add(IR_CALIBRATION_DATA_EIGENVECTORS);

}

/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy MFSingleCredit
	@param[in] e copy source
	@return reference to this object
*/
AQLObject&
LAMathCorrelation::copy(
	const AQLObject& e)
{
	if (this == &e) return *this;

	AQLObject::copy(e);
	if (!e.isTypeOf(ENTITY_IR_CORRELATION))
	{
		AQLString err = "Assignement error for LAMathCorrelation : from ";
		err += AQLString(e.getType());
		throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	mpName			  = &getData(CALIBRATION_DATA_NAME);
	mpIsCorInput	  = &getData(IR_CALIBRATION_DATA_ISCORINPUT);
	mpInter			  = &getData(CALIBRATION_DATA_INTERPOLATION);
	mpFactorNum_after = &getData(IR_CALIBRATION_DATA_FACTORNUM_AFTER);
	mpFactorNum_before= &getData(IR_CALIBRATION_DATA_FACTORNUM_BEFORE);
	mpIsExtraGrid_T   = &getData(IR_CALIBRATION_DATA_ISEXTRA_GRID_LARGE_T);
	mpIsOptim		  = &getData(IR_CALIBRATION_DATA_ISOPTIM);
	mpIsMultiVol	  = &getData(IR_CALIBRATION_DATA_ISMULTIVOL);
	mpGrid_T		  = &getData(IR_CALIBRATION_DATA_GRID_LARGE_T);
	mpGrid_t		  = &getData(IR_CALIBRATION_DATA_GRID_SMALL_T);
	mpIsDataInput	  = &getData(IR_CALIBRATION_DATA_ISDATAINPUT);
	mpCorrelation	  = &getData(IR_CALIBRATION_DATA_CORRELATIONDATA);
	mpFactorLoading	  = &getData(IR_CALIBRATION_DATA_FACTORLOADINGDATA);
	mpFunctions		  = &getData(IR_CALIBRATION_DATA_FUNCTIONS);
	mpOptWeight		  = &getData(IR_CALIBRATION_DATA_OPTWEIGHT);
	mpCorFactors	  = &getData(IR_CALIBRATION_DATA_CORFACTORDATA);
	mpEigenValues	  = &getData(IR_CALIBRATION_DATA_EIGENVALUES);
	mpEigenVectors	  = &getData(IR_CALIBRATION_DATA_EIGENVECTORS);

	return *this;
}
/*!
	@brief add certain data
	@param[in] name name of certain data
	@return reference to holder class 
*/
AQLDataHolder&
LAMathCorrelation::add(const AQLString& name)
{
	AQLDataInstance* dataInstance = getDataInstance();
	AQLPriceDataManager& dm = dataInstance->getDataMaster();
	const AQLDataHolder& dh = dm.getData(name);
	return AQLObject::add(name, dh);
}


///////////////LAMathFactorLoadingFunction class///////////////////////////
/*!
    @brief Default constructor
*/

LAMathCorrelation::LAMathFactorLoadingFunction::LAMathFactorLoadingFunction()
{
}
/*!
    @brief Destructor
*/

LAMathCorrelation::LAMathFactorLoadingFunction::~LAMathFactorLoadingFunction()
{
}
/*!
    @brief Make copy(clone) of this class
*/

AQLCoreFunctionBase*
LAMathCorrelation::LAMathFactorLoadingFunction::clone() const
{
    try 
	{
		return new LAMathFactorLoadingFunction(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief purpose value for optimize
*/

double
LAMathCorrelation::LAMathFactorLoadingFunction::operator()(const DoubleArray& x) const
{
	if (x.size() != mCorr.size() * (mFactornum - 1))
		throw AQLCoreInvalidData("parameter size must be Correlation Size * Factor number", __FILE__, __LINE__);
	
	DoubleArray y(mFactornum);
	unsigned int i, j;
	DoubleMatrix dmat = fromthitaTob(x, mCorr.size(), mFactornum);
	AQLMatrix mat(dmat);
	mat = mat * mat.transpose();
	
	double ret = 0.0;
    for (i = 0; i < mCorr.size(); i++)
		for (j = 0; j < mCorr.size(); j++) ret += mWeight[i][j] * AQLMath::sqr((mCorr[i][j] - mat[i][j]));;
	
	return ret;
}

double
LAMathCorrelation::LAMathFactorLoadingFunction::operator()(const double& x) const
{
	return 0.0;
}

/*!
    @brief Set Correlation
	@param[in] corr Correlation
*/
void
LAMathCorrelation::LAMathFactorLoadingFunction::setCorrelation(const DoubleMatrix& corr)
{
	mCorr.resize(corr.size());
	mCorr = corr;
};

/*!
    @brief set Optimize Weight
	@param[in] weight weight
*/
void
LAMathCorrelation::LAMathFactorLoadingFunction::setOptWeight(const DoubleMatrix& weight)
{
	mWeight.resize(weight.size());
	mWeight = weight;
};

/*!
    @brief set Factor number
	@param[in] factornum Factor number
*/
void
LAMathCorrelation::LAMathFactorLoadingFunction::setFactornum(unsigned int& factornum)
{
	mFactornum = factornum;
};

/*!
    @brief culc b from paramater of thita
*/
DoubleMatrix
LAMathCorrelation::LAMathFactorLoadingFunction::fromthitaTob(const DoubleArray& thita, unsigned int CorrSize, unsigned int factornum)
{
	DoubleArray y(factornum);
	DoubleMatrix dmat(CorrSize);
	unsigned int i, j, k;
	
	for (i = 0; i < CorrSize; i++)
	{
		for (j = 0; j < factornum; j++)
		{
			if (j == factornum - 1)
			{
				y[j] = 1.0;				
				for (k = 0; k < factornum - 1; k++)
					y[j] *= sin(thita[k + i * (factornum - 1)]);
			}
			else
			{
				y[j] = cos(thita[j + i * (factornum - 1)]);				
				for (k = 0; k < j; k++)
					y[j] *= sin(thita[k + i * (factornum - 1)]);
			}
		}
		dmat[i] = y;
	}
	return dmat;
}
