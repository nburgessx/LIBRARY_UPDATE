/*! @file
    @brief Source code of fx volatility function class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceFXVolatility.cpp
//
//  SYNOPSIS    :       LAPriceFXVolatility
//  DESCRIPTION :       Source code of fx volatility function class
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAPriceFXVolatility.h"


#include "LADataBasics.h"
#include "LADataVector.h"



#include "LAAlgorithm.h"
#include "LAConstant.h"
#include "LA1DDataSet.h"
#include "LACombinationFunc.h"
#include "LALinearInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAGaussLegendre.h"
#include "LACombinationFunc.h"
#include "LABasic.h"

using namespace std;
#define GAUSSLEGENDREPOINTNUM 20

//================ LAPriceFXVolatility ===================================
/*!
	@brief default constructor
	@param[in] pVol_fx_and_t volatility of fx function part
	@param[in] pVol_t volatility of t function part

	@note volatility function = fx function part * t function part
	@note input pointers are deleted of this class destructor 
*/
LAPriceFXVolatility::LAPriceFXVolatility(LAFunctionBase* pVol_fx_and_t, LAFunctionBase* pVol_t, const DoubleArray& timegrid)
: mpVol_t(pVol_t), mpVol_fx_and_t(pVol_fx_and_t), mTimeGrid(timegrid), mPos_old(0)
{
	if (pVol_t == 0 || pVol_fx_and_t == 0)
	{
		//error
		throw LACoreInvalidData("Input Volatility is NULL", __FILE__, __LINE__);
	}
	setUp();

}


/*!
	@brief copy constructor
*/
LAPriceFXVolatility::LAPriceFXVolatility(const LAPriceFXVolatility& v) 
: LAFunctionBase(v), mTimeGrid(v.mTimeGrid), mVolData(v.mVolData), mIntegratedVolData(v.mIntegratedVolData), mPos_old(0) 
{
	mpVol_t = dynamic_cast<LAFunctionBase*>(v.mpVol_t->clone());//%%% COVARIANT RETURN %%%
	mpVol_fx_and_t = dynamic_cast<LAFunctionBase*>(v.mpVol_fx_and_t->clone());//%%% COVARIANT RETURN %%%
}

/*!
	@brief destructor
*/
LAPriceFXVolatility::~LAPriceFXVolatility() 
{
	delete mpVol_t;
	delete mpVol_fx_and_t;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceFXVolatility::clone() const	
{
    try 
	{
		return new LAPriceFXVolatility(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceFXVolatility::isTypeOf(function_t id) const
{
	return (id==FN_FXVOLATILITY ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceFXVolatility::getType() const
{
	return FN_FXVOLATILITY;
}

/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time > t and L_{s-1} reset time <= t     
    @return drift value
*/
double
LAPriceFXVolatility::operator()(const DoubleArray& x) const
{
	
	unsigned int size = mTimeGrid.size();
	if (size == 0) return (*mpVol_t)(x[0]) * (*mpVol_fx_and_t)(x);	

	unsigned int pos;
	if (x[0] == 0.0) pos = 0;
	else if (x[0] == mTimeGrid[mPos_old]) pos = mPos_old;
	else if (mPos_old + 1 < size && x[0] == mTimeGrid[mPos_old + 1]) pos = mPos_old + 1;
	else if (!LAAlgorithm::find<DoubleArray, double>(mTimeGrid, x[0], 0, size - 1, pos))
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
LAPriceFXVolatility::integral(const std::vector<std::pair<double,double> >& x) const
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
		LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return fx_and_t_part * LAMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}

	unsigned int pos_e;
	if (x[0].second == mTimeGrid[mPos_old]) pos_e = mPos_old;
	else if (x[0].second == mTimeGrid[mPos_old + 1]) pos_e = mPos_old + 1;
	else if (!LAAlgorithm::find<DoubleArray, double>(mTimeGrid, x[0].second, 0, size - 1, pos_e))
	{
		pos_e = 0;
		mPos_old = 0;
		LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return fx_and_t_part * LAMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}
	mPos_old = pos_e; 
	if (x[0].first == 0.0)
		return fx_and_t_part * LAMath::sqrt(mIntegratedVolData[pos_e]);	
	
    
	unsigned int pos_s;
	if (x[0].first == mTimeGrid[mPos_old - 1]) pos_s = mPos_old - 1;
	else if (!LAAlgorithm::find<DoubleArray, double>(mTimeGrid, x[0].first, 0, size - 1, pos_s))
	{
		LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
		return fx_and_t_part * LAMath::sqrt(integral(x[0].first, x[0].second, &gl));	
	}
	
	return fx_and_t_part * LAMath::sqrt((mIntegratedVolData[pos_e] - mIntegratedVolData[pos_s]));

}

void
LAPriceFXVolatility::setUp()
{
	unsigned int size = mTimeGrid.size();
	mVolData.resize(size);
	for (unsigned int i = 0; i < size; i++)
		mVolData[i] = (*mpVol_t)(mTimeGrid[i]);

	mIntegratedVolData.resize(size);
	LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
	for (unsigned int i = 1; i < size; i++)
		mIntegratedVolData[i] = mIntegratedVolData[i - 1] + integral(mTimeGrid[i - 1], mTimeGrid[i], &gl);
}

double
LAPriceFXVolatility::integral(double t1, double t2, LA1DIntegral* pIntegral) const
{
	if (mpVol_t->isTypeOf(FN_CONSTANT))
	{
		double vol = mpVol_t->getParam()[0];
		return vol * vol * (t2 - t1);
	}
	else if (mpVol_t->isTypeOf(FN_1DDATASET) 
		&& dynamic_cast<const LA1DDataSet*>(mpVol_t)->getInterpolationType() == FN_STEPINTERPOLATION)
	{
		const DoubleArray& grid = dynamic_cast<const LA1DDataSet*>(mpVol_t)->getGrids();
		unsigned int pos1, pos2;
		LAAlgorithm::locate<DoubleArray, double>(grid, t1, grid.size(), pos1);
		LAAlgorithm::locate<DoubleArray, double>(grid, t2, grid.size(), pos2);

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