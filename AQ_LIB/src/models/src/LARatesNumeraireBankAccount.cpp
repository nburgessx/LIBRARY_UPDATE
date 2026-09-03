/*! @file
    @brief Source code of bank account numeraire class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesNumeraireBankAccount.cpp
//
//  SYNOPSIS    :       LARatesNumeraireBankAccount
//  DESCRIPTION :       Source code of bank account numeraire class
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


#include "LARatesNumeraireBankAccount.h"
#include "LAModelDynamicsCurve.h"
#include "AQLAlgorithm.h"

//#ifdef _MSC_VER 	//20070409--Nagase--g++(g++stdext)
//using namespace stdext;
//#else
using namespace std;
//#endif
const double INFINITESIMAL = 1E-7; 

//================ LARatesNumeraireBankAccount ===================================
/*!
	@brief default constructor
	@param[in] T payment time of last forward libor 
	@param[in] isStochasticIR interest rate is stochastic or not
*/
LARatesNumeraireBankAccount::LARatesNumeraireBankAccount(double T, bool isStochasticIR) 
: mTerminal(T), LARatesNumeraireBase(isStochasticIR)
{

}
/*!
	@brief copy constructor
*/
/*LARatesNumeraireBankAccount::LARatesNumeraireBankAccount(const LARatesNumeraireBankAccount& v) 
: LARatesNumeraireBase(v)
{

}*/

/*!
	@brief destructor
*/
LARatesNumeraireBankAccount::~LARatesNumeraireBankAccount() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LARatesNumeraireBankAccount::clone() const
{
    try 
	{
		return new LARatesNumeraireBankAccount(*this);
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
LARatesNumeraireBankAccount::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREBANKACCOUNT ? true : LARatesNumeraireBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesNumeraireBankAccount::getType() const
{
	return FN_NUMERAIREBANKACCOUNT;
}


/*!
	@brief get numeraire value at t
	@param[in] t time
	@return numeraire value
*/
double
LARatesNumeraireBankAccount::operator()(double t) const
{
	if (!mIsStochastic)
	{
		return 1.0 / getCurve(0).getP(t);
	}
	
	if (t > mTerminal + INFINITESIMAL || t < 0.0)
	{
		//error
        throw AQLCoreInvalidData("input t is before 0 or after Terminal", __FILE__, __LINE__);
	}
	
	if (mUpdateFlag) {
		if (mTimeGrid.empty())
		{
			map<double, const LARatesPathElementCurve*>::const_iterator it = mCurves.begin();
			++it;
			while (it != mCurves.end())
			{
				mTimeGrid.push_back(it->first);
				++it;
			}
			mTimeGrid.push_back(mTerminal);
		}
		setDFRatioForNumeraire();
		calcNumeraire();
	}

	unsigned int pos;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, t, mTimeGrid.size(), pos);
	
	if (pos == mTimeGrid.size())
		return mNumeraireArray.back();
	else if (mTimeGrid[pos] == t)
		return mNumeraireArray[pos];
	else if (mIsLongJump)
	{
		if (pos > 0)
		{
			double val =  getCurve(mTimeGrid[pos - 1]).getP(t);
			return 1.0 / (getCurve(0).getP(mTimeGrid[pos - 1]) * val);
		}
		else
		{
			double val = getCurve(0).getP(mTimeGrid[pos]);
			return 1.0 / val * getCurve(t).getP(mTimeGrid[pos]);
		}
		//return 1.0 / getCurve(0).getP(mTimeGrid[pos]) * getCurve(t).getP(mTimeGrid[pos]);
	}
	else
		return mNumeraireArray[pos] * getCurve(t).getP(mTimeGrid[pos]);
}

/*!
	@brief set curve

	@param[in] t time
	@return pcurve curve object
									
	@note this class is not pointer owner of the of curve 
*/
void
LARatesNumeraireBankAccount::setCurve(double t, const LARatesPathElementCurve* pcurve)
{
	LARatesNumeraireBase::setCurve(t, pcurve);
	//mUpdateFlag = true;
}

/*!
	@brief set terminal
	@param[in] T terminal
*/	
void
LARatesNumeraireBankAccount::setTerminal(double T) 
{
	mTerminal = T; 
/*	if (mIsStochastic)
	{
		mLastCurve.set_t(T); 
		mCurves[mTerminal] = &mLastCurve;
	}*/
	mUpdateFlag = true;
}

//======================================
// calc nuemraire	
void
LARatesNumeraireBankAccount::calcNumeraire(void) const
{
	if (mTimeGrid.size() != mCurves.size())
	{
		throw AQLCoreInvalidData("mTimeGrid and mCurve's t is not consistent", __FILE__, __LINE__);
	}
	mNumeraireArray.clear();
	//mTimeGrid.clear();
	mNumeraireArray.resize(mCurves.size());
	//mTimeGrid.resize(mCurves.size());
//#ifdef _MSC_VER 	//20070409--Nagase--g++(g++stdext)
//	hash_map<double, const LARatesPathElementCurve*>::const_iterator it = mCurves.begin();
//	hash_map<double, const LARatesPathElementCurve*>::const_iterator it2;
//#else
	map<double, const LARatesPathElementCurve*>::const_iterator it = mCurves.begin();
	map<double, const LARatesPathElementCurve*>::const_iterator it2;
//#endif
       
	int i;
	for (i = 0; i <  (int)mCurves.size() - 1; i++, it++)
	{
		it2 = it;
		it2++;
		if (mTimeGrid[i] != it2->first)
		{
			throw AQLCoreInvalidData("mTimeGrid and mCurve's t is not consistent", __FILE__, __LINE__);
		} 
		if (i == 0)
			//mNumeraireArray[i] = 1.0 / it->second->getP(it2->first);
			mNumeraireArray[i] = 1.0 / (it->second->getP(it2->first) * mDFRatioNumeArray[i]);
		else
		{
			/*if (mIsLongJump)
				mNumeraireArray[i] = 1.0 / getCurve(0).getP(it->first) / it->second->getP(it2->first);
			else
				mNumeraireArray[i] = mNumeraireArray[i - 1] / it->second->getP(it2->first);
                        */
			if (mIsLongJump)
				mNumeraireArray[i] = 1.0 / getCurve(0).getP(it->first) / (it->second->getP(it2->first) * mDFRatioNumeArray[i]);
			else
				mNumeraireArray[i] = mNumeraireArray[i - 1] / (it->second->getP(it2->first) * mDFRatioNumeArray[i]);
		}
		
		//mTimeGrid[i] = it2->first;
	}
	if (mTimeGrid.back() != mTerminal)
	{
		throw AQLCoreInvalidData("mTimeGrid's last element is not mTerminal ", __FILE__, __LINE__);
	}
	/*if (mIsLongJump)
		mNumeraireArray[i] = 1.0 / getCurve(0).getP(it->first) / it->second->getP(mTerminal);
	else
		mNumeraireArray[i] = mNumeraireArray[i - 1] / it->second->getP(mTerminal);
         */
	if (mIsLongJump)
		mNumeraireArray[i] = 1.0 / getCurve(0).getP(it->first) / (it->second->getP(mTerminal) * mDFRatioNumeArray[i]);
	else
		mNumeraireArray[i] = mNumeraireArray[i - 1] / (it->second->getP(mTerminal) * mDFRatioNumeArray[i]);

	//mTimeGrid[i] = mTerminal;
	
	mUpdateFlag = false;
}
//======================================
// set time grid
void
LARatesNumeraireBankAccount::setTimeGrid(const DoubleArray &timeGrid) const
{
	if (timeGrid.empty())
	{
		throw AQLCoreInvalidData("timeGrid is empty.", __FILE__, __LINE__);
	}
	if (timeGrid[0] == 0.0)
	{
		throw AQLCoreInvalidData("timeGrid first element is zero.", __FILE__, __LINE__);
	}
	mTimeGrid.clear();
	mTimeGrid = timeGrid;
	if (mTimeGrid.back() < mTerminal)
	{
		mTimeGrid.push_back(mTerminal);
	}
}

//======================================
// set df ratio for numeraire
void
LARatesNumeraireBankAccount::setDFRatioForNumeraire() const
{
	if (mTimeGrid.empty())
	{
		throw AQLCoreInvalidData("mTimeGrid is empty.", __FILE__, __LINE__);
	}
	mDFRatioNumeArray.resize(mTimeGrid.size(), 1.0);
	if (mpBasisCurve)
	{
		mDFRatioNumeArray[0] = AQLMath::exp(-(*mpBasisCurve)(mTimeGrid[0]) * mTimeGrid[0]);
		for (unsigned int i = 1; i < mDFRatioNumeArray.size(); ++i)
		{
			mDFRatioNumeArray[i] = AQLMath::exp((*mpBasisCurve)(mTimeGrid[i - 1]) * mTimeGrid[i - 1] - (*mpBasisCurve)(mTimeGrid[i]) * mTimeGrid[i]);
		}
	}
}

//======================================
// set df ratio 
void
LARatesNumeraireBankAccount::setDFRatio(double t) const
{
	if (mpBasisCurve)
	{      
		if (mTimeGrid.empty())
		{
			throw AQLCoreInvalidData("mTimeGrid is empty.", __FILE__, __LINE__);
		}
		map<double, double>::const_iterator it = mDFRatio.find(t);
		if (it == mDFRatio.end())
		{
			unsigned int pos;
			AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, t, mTimeGrid.size(), pos);
			mDFRatio[t] = AQLMath::exp((*mpBasisCurve)(t) * t - (*mpBasisCurve)(mTimeGrid[pos]) * mTimeGrid[pos]);
		}
	}
}

