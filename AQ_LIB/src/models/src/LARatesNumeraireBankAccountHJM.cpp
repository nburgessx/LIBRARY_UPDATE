/*! @file
    @brief Source code of bank account numeraire class for HJM



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesNumeraireBankAccountHJM.cpp
//
//  SYNOPSIS    :       LARatesNumeraireBankAccountHJM
//  DESCRIPTION :       Source code of bank account numeraire for HJM class
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
#define GAUSSLEGENDREPOINTNUM 20


#include "LARatesNumeraireBankAccountHJM.h"
#include "LAModelDynamicsCurve.h"
#include "LAAlgorithm.h"
#include "LA1DDataSet.h"
#include "LAGaussLegendre.h"


//#ifdef _MSC_VER 	//20070409--Nagase--g++(g++stdext)
//using namespace stdext;
//#else
using namespace std;
//#endif
const double INFINITESIMAL = 1E-7; 

//================ LARatesNumeraireBankAccountHJM ===================================
/*!
	@brief default constructor
*/
LARatesNumeraireBankAccountHJM::LARatesNumeraireBankAccountHJM(double T, bool isStochasticIR) 
: mTerminal(T), LARatesNumeraireBase(isStochasticIR)
{

}
/*!
	@brief copy constructor
*/
/*LARatesNumeraireBankAccountHJM::LARatesNumeraireBankAccountHJM(const LARatesNumeraireBankAccountHJM& v) 
: LARatesNumeraireBase(v)
{

}*/

/*!
	@brief destructor
*/
LARatesNumeraireBankAccountHJM::~LARatesNumeraireBankAccountHJM() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesNumeraireBankAccountHJM::clone() const
{
    try 
	{
		return new LARatesNumeraireBankAccountHJM(*this);
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
LARatesNumeraireBankAccountHJM::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREBANKACCOUNTHJM ? true : LARatesNumeraireBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesNumeraireBankAccountHJM::getType() const
{
	return FN_NUMERAIREBANKACCOUNTHJM;
}


/*!
	@brief get numeraire value at t
	@param[in] t time
	@return numeraire value
*/
double
LARatesNumeraireBankAccountHJM::operator()(double t) const
{
	if (!mIsStochastic)
		return 1.0 / getCurve(0).getP(t);
	
	if (t > mTerminal + INFINITESIMAL || t < 0.0)
	{
		//error
        throw LACoreInvalidData("input t is before 0 or after Terminal", __FILE__, __LINE__);
	}
	
	//LA1DDataSet method;	
	LARatesContiBankAccountFunction method;
	LAGaussLegendre GL(GAUSSLEGENDREPOINTNUM);// integral method	
	if (mUpdateFlag) 
	{
		//method.setInterpolation(*mpInter_hjm);

		mNumeraireArray.clear();
		mTimeGrid.clear();
		mNumeraireArray.resize(mCurves.size());
		mTimeGrid.resize(mCurves.size());
	
		map<double, const LARatesPathElementCurve*>::const_iterator it = mCurves.begin();
		map<double, const LARatesPathElementCurve*>::const_iterator it2;
		
		int i;
		for (i = 0; i <  (int)mCurves.size() - 1; i++, it++)
		{
			it2 = it;
			it2++;
			method.setCurve(it->second);

			if (i == 0)
				mNumeraireArray[i] = method.integral(0.0,       it2->first, &GL);
			else
				mNumeraireArray[i] = method.integral(it->first, it2->first, &GL) * mNumeraireArray[i - 1];

//			mNumeraireArray[i] = it->second->getZero(it2->first);

			mTimeGrid[i] = it2->first;
		}

		//mNumeraireArray[i] = it->second->getZero(mTerminal);
		mNumeraireArray[i] = method.integral(it->first, mTerminal, &GL) * mNumeraireArray[i - 1];
		mTimeGrid[i] = mTerminal;
		
		//method.set(mTimeGrid, mNumeraireArray);
		mUpdateFlag = false;
	}

	unsigned int pos;
	LAAlgorithm::locate<DoubleArray, double>(mTimeGrid, t, mTimeGrid.size(), pos);
	
	if (pos == mTimeGrid.size())
		return mNumeraireArray.back();
	else if (mTimeGrid[pos] == t)
		return mNumeraireArray[pos];
	else
	{
		map<double, const LARatesPathElementCurve*>::const_iterator it = mCurves.begin();
		for (int i = 0; i <  pos; i++, it++) ;
		method.setCurve(it->second);
		double aa=method.integral(t, mTimeGrid[pos], &GL);
		return mNumeraireArray[pos] / method.integral(t, mTimeGrid[pos], &GL);//getCurve(t).getP(mTimeGrid[pos]);
	}
}

/*!
	@brief set curve

	@param[in] t time
	@return pcurve curve object
									
	@note this class is not pointer owner of the of curve 
*/
void
LARatesNumeraireBankAccountHJM::setCurve(double t, const LARatesPathElementCurve* pcurve)
{
	LARatesNumeraireBase::setCurve(t, pcurve);
	mUpdateFlag = true;
}

/*!
	@brief set terminal
	@param[in] T terminal
*/	
void
LARatesNumeraireBankAccountHJM::setTerminal(double T) 
{
	mTerminal = T; 
/*	if (mIsStochastic)
	{
		mLastCurve.set_t(T); 
		mCurves[mTerminal] = &mLastCurve;
	}*/
	mUpdateFlag = true;
}

void
LARatesNumeraireBankAccountHJM::setInterpolationMethod_hjm(LAInterpolationBase* pinter) 
{
	mpInter_hjm = pinter; 
	mUpdateFlag = true;
}

//======================================
// calc nuemraire	
/*void
LARatesNumeraireBankAccountHJM::calcNumeraire(void) const
{
	mNumeraireArray.clear();
	mTimeGrid.clear();
	mNumeraireArray.resize(mCurves.size());
	mTimeGrid.resize(mCurves.size());

	map<double, const LARatesPathElementCurve*>::const_iterator it = mCurves.begin();
	map<double, const LARatesPathElementCurve*>::const_iterator it2;

	int i;
	for (i = 0; i <  (int)mCurves.size() - 1; i++, it++)
	{
		it2 = it;
		it2++;
		if (i == 0)
			mNumeraireArray[i] = 1.0 / it->second->getP(it2->first);
		else
			mNumeraireArray[i] = mNumeraireArray[i - 1] / it->second->getP(it2->first);
		
		mTimeGrid[i] = it2->first;
	}
	mNumeraireArray[i] = mNumeraireArray[i - 1] / it->second->getP(mTerminal);
	mTimeGrid[i] = mTerminal;
	
	mUpdateFlag = false;
}
*/

LARatesContiBankAccountFunction::LARatesContiBankAccountFunction()
{
}

LARatesContiBankAccountFunction::~LARatesContiBankAccountFunction()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LARatesContiBankAccountFunction::getType() const
{
	return FN_CONTIBANKACCOUNTFUNC;
}

LACoreFunctionBase*
LARatesContiBankAccountFunction::clone() const
{
    try 
	{
		return new LARatesContiBankAccountFunction(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

void
LARatesContiBankAccountFunction::setCurve(const LARatesPathElementCurve* curve)
{
	pCurve = curve;
}

double
LARatesContiBankAccountFunction::operator()(const DoubleArray& x) const
{
	if(x.size() == 1)
		return operator()(x[0]);
	throw LACoreInvalidData("parameter size must be one", __FILE__, __LINE__);
}

double
LARatesContiBankAccountFunction::operator()(const double& x) const
{
	double ret = 1 / pCurve->getP(x);
	return ret;
}

