/*! @file
    @brief Source code of fx volatility function class



*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceFXVolatility.h"


#include "AQLDataBasics.h"
#include "AQLDataVector.h"



#include "AQLAlgorithm.h"
#include "AQLConstant.h"
#include "AQL1DDataSet.h"
#include "AQLCombinationFunc.h"
#include "AQLLinearInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLGaussLegendre.h"
#include "AQLCombinationFunc.h"
#include "AQLBasic.h"

using namespace std;
#define GAUSSLEGENDREPOINTNUM 20

//================ AQLPriceFXVolatility ===================================
/*!
	@brief default constructor
	@param[in] pVol_fx_and_t volatility of fx function part
	@param[in] pVol_t volatility of t function part

	@note volatility function = fx function part * t function part
	@note input pointers are deleted of this class destructor 
*/
AQLPriceFXVolatility::AQLPriceFXVolatility(AQLFunctionBase* pVol_fx_and_t, AQLFunctionBase* pVol_t, const DoubleArray& timegrid)
: mpVol_t(pVol_t), mpVol_fx_and_t(pVol_fx_and_t), mTimeGrid(timegrid), mPos_old(0)
{
	if (pVol_t == 0 || pVol_fx_and_t == 0)
	{
		//error
		throw AQLCoreInvalidData("Input Volatility is NULL", __FILE__, __LINE__);
	}
	setUp();

}


/*!
	@brief copy constructor
*/
AQLPriceFXVolatility::AQLPriceFXVolatility(const AQLPriceFXVolatility& v) 
: AQLFunctionBase(v), mTimeGrid(v.mTimeGrid), mVolData(v.mVolData), mIntegratedVolData(v.mIntegratedVolData), mPos_old(0) 
{
	mpVol_t = dynamic_cast<AQLFunctionBase*>(v.mpVol_t->clone());//%%% COVARIANT RETURN %%%
	mpVol_fx_and_t = dynamic_cast<AQLFunctionBase*>(v.mpVol_fx_and_t->clone());//%%% COVARIANT RETURN %%%
}

/*!
	@brief destructor
*/
AQLPriceFXVolatility::~AQLPriceFXVolatility() 
{
	delete mpVol_t;
	delete mpVol_fx_and_t;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceFXVolatility::clone() const	
{
    try 
	{
		return new AQLPriceFXVolatility(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceFXVolatility::isTypeOf(function_t id) const
{
	return (id==FN_FXVOLATILITY ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceFXVolatility::getType() const
{
	return FN_FXVOLATILITY;
}

/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time > t and L_{s-1} reset time <= t     
    @return drift value
*/
double
AQLPriceFXVolatility::operator()(const DoubleArray& x) const
{
	
	unsigned int size = mTimeGrid.size();
	if (size == 0) return (*mpVol_t)(x[0]) * (*mpVol_fx_and_t)(x);	

	unsigned int pos;
	if (x[0] == 0.0) pos = 0;
	else if (x[0] == mTimeGrid[mPos_old]) pos = mPos_old;
	else if (mPos_old + 1 < size && x[0] == mTimeGrid[mPos_old + 1]) pos = mPos_old + 1;
	else if (!AQLAlgorithm::find<DoubleArray, double>(mTimeGrid, x[0], 0, size - 1, pos))
	{
		pos = 0;
		mPos_old = 0;
		return (*mpVol_t)(x[0]) * (*mpVol_fx_and_t)(x);	
	}
	mPos_old = pos; 
	
	return mVolData[pos] * (*mpVol_fx_and_t)(x);
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
AQLPriceFXVolatility::integral(const std::vector<std::pair<double,double> >& x) const
{
	DoubleArray x1(2), x2(2);
	x1[0] = x[0].first;
	x1[1] = x[1].first;
	x2[0] = x[0].second;
	x2[1] = x[1].second;
	
	double fx_and_t_part = 0.5 * ((*mpVol_fx_and_t)(x1) + (*mpVol_fx_and_t)(x2));
	unsigned int size = mTimeGrid.size();
	if (size == 0)
	{
		AQLGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return fx_and_t_part * AQLMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}

	unsigned int pos_e;
	if (x[0].second == mTimeGrid[mPos_old]) pos_e = mPos_old;
	else if (x[0].second == mTimeGrid[mPos_old + 1]) pos_e = mPos_old + 1;
	else if (!AQLAlgorithm::find<DoubleArray, double>(mTimeGrid, x[0].second, 0, size - 1, pos_e))
	{
		pos_e = 0;
		mPos_old = 0;
		AQLGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return fx_and_t_part * AQLMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}
	mPos_old = pos_e; 
	if (x[0].first == 0.0)
		return fx_and_t_part * AQLMath::sqrt(mIntegratedVolData[pos_e]);	
	
    
	unsigned int pos_s;
	if (x[0].first == mTimeGrid[mPos_old - 1]) pos_s = mPos_old - 1;
	else if (!AQLAlgorithm::find<DoubleArray, double>(mTimeGrid, x[0].first, 0, size - 1, pos_s))
	{
		AQLGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return fx_and_t_part * AQLMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}
	
	return fx_and_t_part * AQLMath::sqrt((mIntegratedVolData[pos_e] - mIntegratedVolData[pos_s]));

}

void
AQLPriceFXVolatility::setUp()
{
	unsigned int size = mTimeGrid.size();
	mVolData.resize(size);
	for (unsigned int i = 0; i < size; i++)
		mVolData[i] = (*mpVol_t)(mTimeGrid[i]);

	mIntegratedVolData.resize(size);
	AQLGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
	for (unsigned int i = 1; i < size; i++)
		mIntegratedVolData[i] = mIntegratedVolData[i - 1] + integral(mTimeGrid[i - 1], mTimeGrid[i], &gl);
}

double
AQLPriceFXVolatility::integral(double t1, double t2, AQL1DIntegral* pIntegral) const
{
	if (mpVol_t->isTypeOf(FN_CONSTANT))
	{
		double vol = mpVol_t->getParam()[0];
		return vol * vol * (t2 - t1);
	}
	else if (mpVol_t->isTypeOf(FN_1DDATASET) 
		&& dynamic_cast<const AQL1DDataSet*>(mpVol_t)->getInterpolationType() == FN_STEPINTERPOLATION)
	{
		const DoubleArray& grid = dynamic_cast<const AQL1DDataSet*>(mpVol_t)->getGrids();
		unsigned int pos1, pos2;
		AQLAlgorithm::locate<DoubleArray, double>(grid, t1, grid.size(), pos1);
		AQLAlgorithm::locate<DoubleArray, double>(grid, t2, grid.size(), pos2);

		double sum = 0.0;
		double xx1, xx2;
		double vol;
		if (pos1 == pos2)
		{
    		vol = (*mpVol_t)(0.5 * (t2 + t1));
			return	(t2 - t1) * vol * vol;
		}
		sum = 0.0;
        xx1 = t1;
		xx2 = grid[pos1];
		vol = (*mpVol_t)(0.5 * (xx2 + xx1));
		sum += (xx2 - xx1) * vol * vol;
		for (unsigned int i = pos1; i < pos2 - 1; i++)
		{
			xx1 = grid[i];
			xx2 = grid[i + 1];
			vol = (*mpVol_t)(0.5 * (xx2 + xx1));
			sum += (xx2 - xx1) * vol * vol;
		}
		xx1 = grid[pos2-1];
		xx2 = t2;
		vol = (*mpVol_t)(0.5 * (xx2 + xx1));
		sum += (xx2 - xx1) * vol * vol;
		return sum;
	}
	else
		return ((*mpVol_t) * (*mpVol_t)).integral(t1, t2, pIntegral);

}