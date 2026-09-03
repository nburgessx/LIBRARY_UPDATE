/*! @file
    @brief Source code of drift function of HJM sde 



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftHJM.cpp
//
//  SYNOPSIS    :       LAPriceDriftHJM
//  DESCRIPTION :       Source code of drift function of HJM sde 
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


#include "LAPriceDriftHJM.h"
#include "LAFunctionBase.h"
#include "LAConstant.h"
#include "LA1DDataSet.h"
#include "LACombinationFunc.h"
#include "LALinearInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAGaussLegendre.h"

#include "LAMathPathEntity.h"
#include "LADataHolder.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LAMathAttrSDE.h"
#include "LARatesSDEBase.h"
#include "LAModelDynamicsCurve.h"
#include "LAMathCorrelation.h"
#include "LAAlgorithm.h"

using namespace std;

#define COR "COR"
#define GAUSSLEGENDREPOINTNUM 20

//================ LAPriceDriftHJM ===================================
/*!
	@brief default constructor
*/
LAPriceDriftHJM::LAPriceDriftHJM()
: LAMathDriftFuncBase(), mpTimes(0), mPos_old(0)//, mpCache(0)
{

}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] deltatenor delta of tenor

*/
LAPriceDriftHJM::LAPriceDriftHJM(const LAString& sdeAttrName, double T, const DoubleArray& grid_T)	
: LAMathDriftFuncBase(), mT(T), mSDEAttrName(sdeAttrName), mgrid_T(grid_T),
mpTimes(0), mPos_old(0) //, mpCache(0)
{
	
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LAPriceDriftHJM::LAPriceDriftHJM(const LAPriceDriftHJM& v) 
: LAMathDriftFuncBase(v), mT(v.mT), mVolatility(v.mVolatility), mSDEAttrName(v.mSDEAttrName), 
mgrid_T(v.mgrid_T), mpTimes(v.mpTimes), /*mCorData(v.mCorData),*/ mPos_old(v.mPos_old)//, mpCache(0)
{
	mCorrelation.resize(v.mCorrelation.size());
	for (unsigned int i = 0; i < mCorrelation.size(); i++)
	{
		mCorrelation[i].resize(v.mCorrelation[i].size());
		for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
		{
			if (mT == 0) mCorrelation[i][j] = dynamic_cast<LAFunctionBase*>(v.mCorrelation[i][j]->clone());
			else mCorrelation[i][j] = dynamic_cast<LAFunctionBase*>(v.mCorrelation[i][j]);
		}
	}
	//if (mT == 0.0 && v.mpCache != 0)
	//	mpCache = new DoubleArray(*v.mpCache);
}

/*!
	@brief destructor
*/
LAPriceDriftHJM::~LAPriceDriftHJM() 
{
	if (mT == 0.0)
	{
		for (unsigned int i = 0; i < mCorrelation.size(); i++)
			for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
				delete mCorrelation[i][j];
		//if (mpCache != 0) delete mpCache;
	}
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftHJM::clone() const	
{
    try 
	{
		return new LAPriceDriftHJM(*this);
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
LAPriceDriftHJM::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTHJM ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftHJM::getType() const
{
	return FN_DRIFTHJM;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t
    @return drift value

*/
double
LAPriceDriftHJM::operator()(const DoubleArray& x) const
{
	return operator()(x[0]);
}
/*!
    @brief Return drift value
	@param[in] x input value, x = t
    @return drift value

*/
double
LAPriceDriftHJM::operator()(double x) const
{
	if (x > mT || x < 0.0) 
		throw LACoreInvalidData("t is something wrong", __FILE__, __LINE__);
	
	double ret = 0.0;
	unsigned int pos;

	bool flag_data = true;
	if (mpTimes == 0) flag_data = false;
	else
	{
		if (x == 0.0) pos = 0;
		else if (x == (*mpTimes)[mPos_old]) pos = mPos_old;
		else if (x == (*mpTimes)[mPos_old + 1]) pos = mPos_old + 1;
		else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x, 0, mpTimes->size() - 1, pos))
		{
			pos = 0;
			mPos_old = 0;
			flag_data = false;	
		}
		mPos_old = pos; 
	}		
	
    if (flag_data)
		ret = mDrift[mPos_old];
	else
		ret = calcDrift(x);
		/*
		LAAlgorithm::locate<DoubleArray, double>(mgrid_T, mT, mgrid_T.size(), pos);
		for (unsigned int i = 0; i < mFactorNum; i++)
			for (unsigned int j = 0; i < mFactorNum; j++)
				ret += mVolatility[pos][i]->operator ()(x) * getIntegratedpart(i, j, x, mT, pos);*/

	return ret;	
}

/*!
	@brief calc Drift
	@param[in] x t
	

*/
double
LAPriceDriftHJM::calcDrift (double x) const
{
	unsigned int pos;
	LAAlgorithm::locate<DoubleArray, double>(mgrid_T, mT, mgrid_T.size(), pos);
	double ret = 0.0;
	for (unsigned int i = 0; i < mFactorNum; i++)
		for (unsigned int j = 0; i < mFactorNum; j++)
			ret += mVolatility[pos][i]->operator ()(x) * getIntegratedpart(i, j, x, mT, pos);
	return ret;
}

/*!
	@brief get integral
	@param[in] i	suffics
	@param[in] j	suffics
	@param[in] t1	left edge of integral region
	@param[in] t2	right edge of integral region
	@param[in] pos	T position of grid_T  

*/
double
LAPriceDriftHJM::getIntegratedpart (unsigned int i, unsigned int j, 
								   double t1, double t2, unsigned int pos) const
{
	LAGaussLegendre GL(GAUSSLEGENDREPOINTNUM);
	if (t1 == t2) return 0;
	if (mVolatility[pos][j]->isTypeOf(FN_CONSTANT))// && mVolatility[j]->isTypeOf(FN_CONSTANT))
	{
		double vol = (*mVolatility[pos][j])(t1);
		if (i == j)
			return vol * (t2 - t1);
		//double vol2 = (*mVolatility[j])(t1);
		if (mCorrelation[i][j]->isTypeOf(FN_CONSTANT)) 
			return vol * (*mCorrelation[i][j])(t1) * (t2 - t1);
		if (mCorrelation[i][j]->isTypeOf(FN_1DDATASET))
		{ 
			const LA1DDataSet* pCor = dynamic_cast<const LA1DDataSet*>(mCorrelation[i][j]);
			if(pCor->getInterpolationType() == FN_STEPINTERPOLATION || 
			   pCor->getInterpolationType() == FN_LINEARINTERPOLATION)
				return vol * pCor->integral(t1, t2);
			//else 
			//	return vol * mCorrelation[i][j]->integral(t1, t2, &mGL);
		}
		return vol * mCorrelation[i][j]->integral(t1, t2, &GL);
	}
	
	if (mVolatility[pos][j]->isTypeOf(FN_1DDATASET))// && mVolatility[j]->isTypeOf(FN_1DDATASET))
	{
		const LA1DDataSet* pVol = dynamic_cast<const LA1DDataSet*>(mVolatility[pos][j]);
		//const LA1DDataSet* pVol2 = dynamic_cast<const LA1DDataSet*>(mVolatility[j]);
		vector<const LA1DDataSet*> funcs(1);
		funcs[0] = pVol;
		//funcs[1] = pVol2;
		if (i == j) return integral(t1, t2, funcs);
		else if (mCorrelation[i][j]->isTypeOf(FN_CONSTANT))
			return integral(t1, t2, funcs) * (*mCorrelation[i][j])(t1);
		else if (mCorrelation[i][j]->isTypeOf(FN_1DDATASET))
		{
			funcs.push_back(dynamic_cast<const LA1DDataSet*>(mCorrelation[i][j]));
			return integral(t1, t2, funcs);
		}
		//else return mGL.integrate((*mVolatility[i]) * (*mVolatility[j]) * (*mCorrelation[i][j]), t1, t2);
	}
	
	if (i == j)
		return GL.integrate((*mVolatility[pos][j]), t1, t2); 
	return GL.integrate((*mVolatility[pos][j]) * (*mCorrelation[i][j]), t1, t2); 

}

/*!
	@brief calculte integral of product of LA1DDataSet functions
	@param[in] t1 left edge of integral region
	@param[in] t2 right edge of integral region
	@param[in] funcs vector of LA1DDataSet functions
*/
double
LAPriceDriftHJM::integral(double t1, double t2, vector<const LA1DDataSet*>& funcs) const
{
	LAGaussLegendre GL(GAUSSLEGENDREPOINTNUM);
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
			return funcs[0]->integral(t1, t2, &GL); 

		LACombinationMethod combi = (*funcs[0]) * (*funcs[1]);
		for (unsigned int i = 2; i < func_num; i++)
			combi = combi * (*funcs[i]);
		
		for (it = grid_set.begin(); it != grid_set.end(); it++)
		{
			ret += GL.integrate(combi, start, *it); 
			start = *it;
		}
		ret += GL.integrate(combi, start, t2); 
		return ret;
	}

}

/*!
    @brief return string representaion
    @return string representaion  (sde attr name : maturity)
*/
LAString
LAPriceDriftHJM::convertToString(void) const
{
	LAString ret;
	ret += mSDEAttrName;
	ret += ":";
	ret += LADataDouble(mT).convertToString();
	
	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion  (sde attr name : maturity)
*/
void
LAPriceDriftHJM::convertFromString(const LAString& str)
{
	LADataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 3)
	{
		//error
		throw LACoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}

	mSDEAttrName = tmp.get()[0];
	mT = tmp.get()[1].getDoubleValue();
	
}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftHJM::setUp(LAMathPathEntity& path)
{
	LADataHolder* dh = &path.getData(mSDEAttrName, ISNOTNULL);
	LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mVolatility = pattrsde->getSDE().getVolatility();

	mpTimes = 0;
	mDrift.clear();
	mpTimes = &pattrsde->getSDE().getBM()->getTimeGrid();
	mDrift.resize(mpTimes->size());
	mPos_old = 0;
	//unsigned int pos;
	//LAAlgorithm::locate<DoubleArray, double>(mgrid_T, mT, mgrid_T.size(), pos);
	for (unsigned int i = 0; i < mpTimes->size(); i++)
		mDrift[i] = calcDrift((*mpTimes)[i]);	

}
