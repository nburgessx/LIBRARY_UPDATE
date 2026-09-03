/*! @file
    @brief Source code of covariance class


*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesCovariance.h"

#include "AQLFunctionBase.h"
#include "AQLConstant.h"
#include "AQL1DDataSet.h"
#include "AQLCombinationFunc.h"
#include "AQLLinearInterpolation.h"
#include "AQLStepInterpolation.h"


using namespace std;

const double INFINITESIMAL = 1E-7; 
#define GAUSSLEGENDREPOINTNUM 20

//================ AQLRatesCovariance ===================================
/*!
	@brief constructor
*/
AQLRatesCovariance::AQLRatesCovariance()
: mGL(GAUSSLEGENDREPOINTNUM)
{
	mXX.resize(GAUSSLEGENDREPOINTNUM);
	mWW.resize(GAUSSLEGENDREPOINTNUM);
}


/*!
	@brief copy constructor
	@param[in] v copy source
*/
/*AQLRatesCovariance::AQLRatesCovariance(const AQLRatesCovariance& v)
{
}*/

/*!
	@brief destructor
*/
AQLRatesCovariance::~AQLRatesCovariance() 
{

}


/*!
	@brief get covariance
	@param[in] i suffics
	@param[in] j suffics
	@param[in] t time
*/
double
AQLRatesCovariance::getCov (unsigned int i, unsigned int j, double t) const
{
	double vol = (*mVolatility[i])(t);
	if (i == j)	return vol * vol;
	return vol * (*mVolatility[j])(t) * (*mCorrelation[i][j])(t); 
}


/*!
	@brief get integral of covariance
	@param[in] i suffics
	@param[in] j suffics
	@param[in] pos1 left edge of integral region
	@param[in] pos2 right edge of integral region

	@note pos1 and pos2 are position of timegrid of calcIntegratedCov method's input 
*/
double
AQLRatesCovariance::getIntegratedCov (unsigned int i, unsigned int j, unsigned int pos1, unsigned int pos2) const
{
//	if (j > i) return mIntegratedCovData.at(pos2).at(j).at(i) - mIntegratedCovData.at(pos1).at(j).at(i);
//	else return mIntegratedCovData.at(pos2).at(i).at(j) - mIntegratedCovData.at(pos1).at(i).at(j);
	if (pos1 == 0)
	{
		if (j > i) return mIntegratedCovData[pos2][j][i] ;
		else return mIntegratedCovData[pos2][i][j];
	}
	if (j > i) return mIntegratedCovData[pos2][j][i] - mIntegratedCovData[pos1][j][i];
	else return mIntegratedCovData[pos2][i][j] - mIntegratedCovData[pos1][i][j];


}


/*!
	@brief get integral of covariance
	@param[in] i suffics
	@param[in] j suffics
	@param[in] t1 left edge of integral region
	@param[in] t2 right edge of integral region

*/
double
AQLRatesCovariance::getIntegratedCov (unsigned int i, unsigned int j, double t1, double t2) const
{
	if (t1 == t2) return 0.0;

	if (mVolatility[i]->isTypeOf(FN_CONSTANT) && mVolatility[j]->isTypeOf(FN_CONSTANT))
	{
		double vol = (*mVolatility[i])(t1);
		if (i == j)
			return vol * vol * (t2 - t1);
		double vol2 = (*mVolatility[j])(t1);
		if (mCorrelation[i][j]->isTypeOf(FN_CONSTANT)) 
			return vol * vol2 * (*mCorrelation[i][j])(t1) * (t2 - t1);
		if (mCorrelation[i][j]->isTypeOf(FN_1DDATASET))
		{ 
			const AQL1DDataSet* pCor = dynamic_cast<const AQL1DDataSet*>(mCorrelation[i][j]);
            if(pCor->getInterpolationType() == FN_STEPINTERPOLATION || pCor->getInterpolationType() == FN_LINEARINTERPOLATION)
				return vol * vol2 * pCor->integral(t1, t2);
			else 
				return vol * vol2 * mCorrelation[i][j]->integral(t1, t2, &mGL);
		}
		return vol * vol2 * mCorrelation[i][j]->integral(t1, t2, &mGL);
	}
	
	if (mVolatility[i]->isTypeOf(FN_1DDATASET) && mVolatility[j]->isTypeOf(FN_1DDATASET))
	{
		const AQL1DDataSet* pVol = dynamic_cast<const AQL1DDataSet*>(mVolatility[i]);
		const AQL1DDataSet* pVol2 = dynamic_cast<const AQL1DDataSet*>(mVolatility[j]);
		vector<const AQL1DDataSet*> funcs(2);
		funcs[0] = pVol;
		funcs[1] = pVol2;
		if (i == j) return integral(t1, t2, funcs);
		else if (mCorrelation[i][j]->isTypeOf(FN_CONSTANT))
			return integral(t1, t2, funcs) * (*mCorrelation[i][j])(t1);
		else if (mCorrelation[i][j]->isTypeOf(FN_1DDATASET))
		{
			funcs.push_back(dynamic_cast<const AQL1DDataSet*>(mCorrelation[i][j]));
			return integral(t1, t2, funcs);
		}
//		else return mGL.integrate((*mVolatility[i]) * (*mVolatility[j]) * (*mCorrelation[i][j]), t1, t2);
	}
/*	if (i == j)
		return mGL.integrate((*mVolatility[i]) * (*mVolatility[j]), t1, t2); 
	
	return mGL.integrate((*mVolatility[i]) * (*mVolatility[j]) * (*mCorrelation[i][j]), t1, t2);*/

	int max = mXX.size();
	mGL.get(mXX, mWW, t1, t2);
	double ret = 0.0;

	if (i != j)
	{
		for (int k = 0; k < max; ++k)
		{
			double t = mXX[k];
			ret += mVolatility[i]->operator ()(t) *
					mVolatility[j]->operator ()(t) *
					mCorrelation[i][j]->operator()(t) * mWW[k];
		}
		return ret;
	}
	else
	{
		for (int k = 0; k < max; ++k)
		{
			double t = mXX[k];
			ret += mVolatility[i]->operator ()(t) *	mVolatility[i]->operator ()(t) * mWW[k];
		}
		return ret;
	}
}


/*!
	@brief calculate and store integral of covariance
	@param[in] timegrid timegrid
	@param[in] pReset_timegrid reset times
*/
void
AQLRatesCovariance::calcIntegratedCov(const DoubleArray& timegrid, const DoubleArray* pReset_timegrid)
{
	unsigned int size = timegrid.size();
	unsigned int matsize = mVolatility.size();

	if (pReset_timegrid != 0 && pReset_timegrid->size() != matsize)
	{
		//error
		throw AQLCoreInvalidData("reset time size and volatility num are not same", __FILE__, __LINE__);
	}
	

	mIntegratedCovData.resize(timegrid.size());
	for (unsigned int i = 0; i < size; i++)
	{
		mIntegratedCovData[i].resize(matsize);
		for (unsigned int j = 0; j < matsize; j++)
			mIntegratedCovData[i][j].resize(j + 1);
	}
	for (unsigned int i = 0; i < size; i++)
	{
		for (unsigned int j = 0; j < matsize; j++)
			for (unsigned int k = 0; k <= j; k++)
			{
				if (i == 0) mIntegratedCovData[i][j][k] = 0.0;
				else if (pReset_timegrid == 0 || (*pReset_timegrid)[k] + INFINITESIMAL >= timegrid[i])
					mIntegratedCovData[i][j][k] = mIntegratedCovData[i - 1][j][k] + getIntegratedCov(j, k, timegrid[i - 1], timegrid[i]);
				else if ((*pReset_timegrid)[k] + INFINITESIMAL < timegrid[i] && (*pReset_timegrid)[k] > timegrid[i - 1] + INFINITESIMAL)
					mIntegratedCovData[i][j][k] = mIntegratedCovData[i - 1][j][k] + getIntegratedCov(j, k, timegrid[i - 1], (*pReset_timegrid)[k]);
				else
					mIntegratedCovData[i][j][k] = mIntegratedCovData[i - 1][j][k];
			}
	}
}

/*!
	@brief calculte integral of product of AQL1DDataSet functions
	@param[in] t1 left edge of integral region
	@param[in] t2 right edge of integral region
	@param[in] funcs vector of AQL1DDataSet functions
*/
double
AQLRatesCovariance::integral(double t1, double t2, vector<const AQL1DDataSet*>& funcs) const
{
	if (t1 > t2) return integral (t2, t1, funcs);
	else if (t1 == t2) return 0.0;

	bool flag_step = true;
	set<double> grid_set;
	unsigned int func_num = funcs.size();
	for (unsigned int i = 0; i < func_num; i++)
	{
		const DoubleArray& grid = funcs[i]->getGrids();
		unsigned int grid_size = grid.size();
		for (unsigned int j = 0; j < grid_size; j++)
		{
			if (grid[j] > t1 && grid[j] < t2) grid_set.insert(grid[j]);
			else if (grid[j] >= t2) break;
		}
		if (funcs[i]->getInterpolationType() != FN_STEPINTERPOLATION)
			flag_step = false;
	}
	
	set<double>::const_iterator it;
	double start = t1;
	double ret = 0.0;
	if (flag_step)
	{
		double ret2;
		for (it = grid_set.begin(); it != grid_set.end(); it++)
		{
			ret2 = *it - start;
			for (unsigned int i = 0; i < func_num; i++)
				ret2 *= (*funcs[i])(0.5 * (start + *it));
			start = *it;
			ret += ret2;
		}
		ret2 = (t2 - start);
		for (unsigned int i = 0; i < func_num; i++)
			ret2 *= (*funcs[i])(0.5 * (start + t2));
		return ret + ret2;
	}
	else
	{
		if (funcs.size() == 1)
			return funcs[0]->integral(t1, t2, &mGL); 

		AQLCombinationMethod combi = (*funcs[0]) * (*funcs[1]);
		for (unsigned int i = 2; i < func_num; i++)
			combi = combi * (*funcs[i]);
		
		for (it = grid_set.begin(); it != grid_set.end(); it++)
		{
			ret += mGL.integrate(combi, start, *it); 
			start = *it;
		}
		ret += mGL.integrate(combi, start, t2); 
		return ret;
	}

}
