/*! @file
    @brief Source code of long-jump version of spot sde class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesLJSpotSDE.cpp
//
//  SYNOPSIS    :       LARatesLJSpotSDE
//  DESCRIPTION :       Source code of long-jump version of spot sde class
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


#include "LARatesLJSpotSDE.h"
#include "LARatesSDEIntegralBase.h"
#include "AQLAlgorithm.h"
#include "AQLBasic.h"

using namespace std;
//================ LARatesLJSpotSDE ===================================
/*!
	@brief default constructor
	@param[in] type sde type
	@param[in] pTransformer transformation function from input/output to sde variablue X'=f(t,X)
	@param[in] pInvTransformer inverse of transformation function X=f^{-1}(t,X')
	@param[in] pAdjuster adjustment function

*/
LARatesLJSpotSDE::LARatesLJSpotSDE(SDE_TYPE type, AQLFunctionBase* pTransformer, AQLFunctionBase* pInvTransformer, AQLFunctionBase* pAdjuster)
: LARatesSpotSDE(type, pTransformer, pInvTransformer, pAdjuster), mpNumeraire_LJ(0)
{

}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesLJSpotSDE::LARatesLJSpotSDE(const LARatesLJSpotSDE& v) 
: LARatesSpotSDE(v), mpNumeraire_LJ(0), mWt(v.mWt), mIntegralRegion(v.mIntegralRegion)
{
	mPath_LJ.resize(v.mPath_LJ.size());
	for (unsigned int i = 0; i < mPath_LJ.size(); i++)
		mPath_LJ[i] = v.mPath_LJ[i]->clone();
	if (v.mpNumeraire_LJ != 0)
		mpNumeraire_LJ = dynamic_cast<LARatesNumeraireBase*>(v.mpNumeraire->clone());

}


/*!
	@brief destructor
*/
LARatesLJSpotSDE::~LARatesLJSpotSDE() 
{
	for (unsigned int i = 0; i < mPath_LJ.size(); i++)
		delete mPath_LJ[i];
	mPath_LJ.clear();
	delete mpNumeraire_LJ;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LARatesLJSpotSDE::clone() const
{
    try 
	{
		return new LARatesLJSpotSDE(*this);
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
LARatesLJSpotSDE::isTypeOf(function_t id) const
{
	return (id==FN_LJSPOTSDE ? true : LARatesSpotSDE::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesLJSpotSDE::getType() const
{
	return FN_LJSPOTSDE;
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
LARatesLJSpotSDE::check(void) const
{
	if ((mpIntegral->getIntegralType() == LOG_INTEGRAL || mpIntegral->getIntegralType() == LOG_INTEGRAL_LOG_OUTPUT)
		&& mType != DIVIDEdXbyX)
    {
		//error
		throw AQLCoreInvalidData("If SDEType is DIVIDEdXbyX, SDEIntegralType must be LOG", __FILE__, __LINE__);
	}
	else if (mpIntegral->getIntegralType() == NORMAL_INTEGRAL && mType != dX)
	{
		//error
		throw AQLCoreInvalidData("If SDEType is dX, SDEIntegralType must be NORMAL_INTEGRAL", __FILE__, __LINE__);

	}
	return LARatesSpotSDE::check();
}


/*!
    @brief get path
    @return path
*/
const vector<LARatesPathElementBase*>&
LARatesLJSpotSDE::getPath(void)
{
	getPathElement(mpBM->getTimeGrid().size() - 1);
	return mPath_LJ;
}


/*!
	
	@brief calculate path
	@param[in] pos position of sde integral grid				
*/	
void
LARatesLJSpotSDE::calcPath(unsigned int pos)
{
	unsigned int pos_s;
	const DoubleArray& grid = mpBM->getTimeGrid();//sde time grid (= BM time grid)
	if (mPos == 0)
	{
		mVar = mPath[0]->get();
		if (mpTransformer != 0)
		{
			DoubleArray var(2);
#ifdef __SCALAR_FLOAT__
			var[1] = static_cast<double>(mVar[0]);
			mVar[0] = static_cast<SCALAR>((*mpTransformer)(var));
#else
			var[1] = mVar[0];
			mVar[0] = (*mpTransformer)(var);
#endif
		}				
		pos_s = 0;
		mWt = 0.0;
		if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const LARatesPathElementCurve*>(mPath[0]));
			mpNumeraire_LJ->clear();
			mpNumeraire_LJ->setCurve(0, dynamic_cast<const LARatesPathElementCurve*>(mPath[0]));
		}
	}
	else if (pos < mPos)
	{
		unsigned int _pos_s;
		AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos], mTimeGrid.size(), _pos_s);
		if (grid[pos] != mTimeGrid[_pos_s])
		{
			if (_pos_s == 0)
			{
				//error
				throw AQLCoreInvalidData("grid is something wrong", __FILE__, __LINE__);
			}
			_pos_s--;
		}
		mVar[0] = mPath[_pos_s]->get()[0];
		if (mpTransformer != 0)
		{
			DoubleArray var(2);
			var[0] = mTimeGrid[_pos_s];
#ifdef __SCALAR_FLOAT__
			var[1] = static_cast<double>(mVar[0]);
			mVar[0] = static_cast<SCALAR>((*mpTransformer)(var));
#else
			var[1] = mVar[0];
			mVar[0] = (*mpTransformer)(var);
#endif
		}			
		if (!AQLAlgorithm::find<DoubleArray, double>(grid, mTimeGrid[_pos_s], 0, grid.size() - 1, pos_s))
		{
			throw AQLCoreInvalidData("TimeGrid is inconsistent with BM grid", __FILE__, __LINE__);
		}	
	}
	else pos_s = mPos;
		
	unsigned int j;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos_s], mTimeGrid.size(), j);
	if (grid[pos_s] == mTimeGrid[j]) j++;
	
	const DoubleMatrix& bm = mpBM->getBM();
	for (unsigned int i = pos_s; i < pos; i++)
	{
		mpIntegral->integral(grid[i], grid[i + 1],
							mDrift.begin(), 
							mVolatility.begin(),
							bm[i].begin(),
							mVar.begin(),
							1);
		
		mWt += bm[i][0];		


		if (mTimeGrid[j] == grid[i + 1])
		{
			if (mpTransformer == 0) mPath[j]->set(mVar[0]);
			else
			{
				DoubleArray var(2);
				var[0] = mTimeGrid[j];
				var[1] = static_cast<double>(mVar[0]);
				mPath[j]->set(static_cast<SCALAR>((*mpInvTransformer)(var)));		
			}

			// set curve to numeraire(case of spot rate ir model)
			if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
				mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const LARatesPathElementCurve*>(mPath[j]));				


			mIntegralRegion[0].second = mTimeGrid[j];
#ifdef __SCALAR_FLOAT__			
			mIntegralRegion[1].second = static_cast<double>(mVar[0]);
#else
			mIntegralRegion[1].second = mVar[0];
#endif
			double ret = mDrift[0]->integral(mIntegralRegion);
			double vol = mVolatility[0][0]->integral(mIntegralRegion);
			if (mType == DIVIDEdXbyX) ret -= 0.5 * vol * vol;
			ret += vol * mWt / AQLMath::sqrt(mTimeGrid[j]);
			if (mpIntegral->getIntegralType() == LOG_INTEGRAL) ret = mIntegralRegion[1].first * AQLMath::exp(ret);
			else ret += mIntegralRegion[1].first;
			if (mpTransformer == 0)
			{
#ifdef __SCALAR_FLOAT__			
				mPath_LJ[j]->set(static_cast<SCALAR>(ret));
#else
				mPath_LJ[j]->set(ret);
#endif
			}
			else
			{
				DoubleArray var(2);
				var[0] = mTimeGrid[j];
				var[1] = static_cast<double>(ret);
				mPath_LJ[j]->set(static_cast<SCALAR>((*mpInvTransformer)(var)));		
			}


			// set curve to numeraire(case of spot rate ir model)
			if (mpNumeraire_LJ != 0 && mpTemplate->isTypeOf(PE_CURVE))
				mpNumeraire_LJ->setCurve(mTimeGrid[j], dynamic_cast<const LARatesPathElementCurve*>(mPath_LJ[j]));				
				
			j++;
		}	

	}
	mpPathElement->set(mVar[0]);//return Euler Maruyama output(not LJ)

}
/*!
	@brief set numeraire
	@param[in] pnumeraire	pointer to numeraire
	@note this class becomes pointer owner of the input numeraire 
*/
void
LARatesLJSpotSDE::setNumeraire(LARatesNumeraireBase* pnumeraire)
{
	LARatesSDEBase::setNumeraire(pnumeraire);
	if (mpNumeraire != 0) 
	{
		mpNumeraire_LJ = dynamic_cast<LARatesNumeraireBase*>(mpNumeraire->clone());
		mpNumeraire_LJ->setLongJump(true);
	}

}
/*!
    @brief set up this class for path calculation
*/
void
LARatesLJSpotSDE::setUp()
{
	LARatesSpotSDE::setUp();
	mPath_LJ.resize(mPath.size());
	for (unsigned int i = 0; i < mPath.size(); i++)
		mPath_LJ[i] = mPath[i]->clone();
	mIntegralRegion.resize(2);
	mIntegralRegion[0].first = 0.0;
	if (mpTransformer == 0) mIntegralRegion[1].first = mPath[0]->get()[0];
	else
	{
		DoubleArray var(2);
		var[1] = static_cast<double>(mPath[0]->get()[0]);
		mIntegralRegion[1].first = (*mpTransformer)(var);
	}
	if (mpTemplate->isTypeOf(PE_CURVE) && mpNumeraire_LJ != 0)
		mpNumeraire_LJ->setInterpolationMethod(mpInter);

}