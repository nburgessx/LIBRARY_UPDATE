/*! @file
    @brief Source code of long-jump version term structure sde class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesLJTermStructureSDE.h"
#include "AQLRatesSDEIntegralBase.h"

#include "AQLAlgorithm.h"
#include "AQLBasic.h"

using namespace std;
//================ AQLRatesLJTermStructureSDE ===================================
/*!
	@brief default constructor
	@param[in] type sde type
*/
AQLRatesLJTermStructureSDE::AQLRatesLJTermStructureSDE(SDE_TYPE type)
: AQLRatesTermStructureSDE(type), mpNumeraire_LJ(0)
{

}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLRatesLJTermStructureSDE::AQLRatesLJTermStructureSDE(const AQLRatesLJTermStructureSDE& v) 
: AQLRatesTermStructureSDE(v), mWt(v.mWt), mVar_LJ(v.mVar_LJ), mIntegralRegion(v.mIntegralRegion), mpNumeraire_LJ(0)
{
	mPath_LJ.resize(v.mPath_LJ.size());
	for (unsigned int i = 0; i < mPath_LJ.size(); i++)
		mPath_LJ[i] = v.mPath_LJ[i]->clone();
	if (v.mpNumeraire_LJ != 0)
		mpNumeraire_LJ = dynamic_cast<AQLRatesNumeraireBase*>(v.mpNumeraire_LJ->clone());

}

/*!
	@brief destructor
*/
AQLRatesLJTermStructureSDE::~AQLRatesLJTermStructureSDE() 
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
AQLRatesLJTermStructureSDE::clone() const
{
    try 
	{
		return new AQLRatesLJTermStructureSDE(*this);
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
AQLRatesLJTermStructureSDE::isTypeOf(function_t id) const
{
	return (id==FN_LJTERMSTRUCTURESDE ? true : AQLRatesTermStructureSDE::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesLJTermStructureSDE::getType() const
{
	return FN_LJTERMSTRUCTURESDE;
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
AQLRatesLJTermStructureSDE::check(void) const
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
	return AQLRatesTermStructureSDE::check();
}

/*!
    @brief get path
    @return path
*/
const vector<AQLRatesPathElementBase*>&
AQLRatesLJTermStructureSDE::getPath(void)
{
	getPathElement(mpBM->getTimeGrid().size() - 1);
	return mPath_LJ;
}


/*!
	
	@brief calculate path
	@param[in] pos position of sde integral grid				
*/	
void
AQLRatesLJTermStructureSDE::calcPath(unsigned int pos)
{
	if (mPos == 0)
	{
		mVar = mPath[0]->get();
		unsigned int wt_size = mWt.size();
		for (unsigned int i = 0; i < wt_size; i++)
			mWt[i] = 0.0;
		if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const AQLRatesPathElementCurve*>(mPath[0]));
			mpNumeraire_LJ->clear();
			mpNumeraire_LJ->setCurve(0, dynamic_cast<const AQLRatesPathElementCurve*>(mPath[0]));
		}
	}
	const DoubleArray& grid = mpBM->getTimeGrid();//sde time grid (= BM time grid)

	
	unsigned int j;
	if (!AQLAlgorithm::find<DoubleArray, double>(mTimeGrid, grid[mPos], 0, mTimeGrid.size() - 1, j))
	{
		//error
		throw AQLCoreInvalidData("grid is something wrong", __FILE__, __LINE__);
	}
	j++;

	unsigned int k = 0;
	const DoubleMatrix& bm = mpBM->getBM();
	for (unsigned int i = mPos; i < pos; i++)
	{
		k = mVar.size() - mpBM->getBM()[i].size(); 
		mpIntegral->integral(grid[i], grid[i + 1],
							mDrift.begin() + k, 
							mVolatility.begin() + k,
							bm[i].begin(),
							mVar.begin() + k,
							mVar.size() - k);
		
		int l, m;
		int wt_size = mWt.size();
		int bm_size = bm[i].size();
		for (l = wt_size - 1, m = bm_size - 1; l >= 0 && m >= 0; l--, m--)
			mWt[l] += bm[i][m];


		if (mTimeGrid[j] == grid[i + 1])
		{
			mPath[j]->set(mVar); 
	
			// set curve to numeraire 
			if (mpNumeraire != 0)
				mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const AQLRatesPathElementCurve*>(mPath[j]));
		
			mIntegralRegion[0].second = mTimeGrid[j];
			for (l = k ; l < wt_size; l++)
			{
#ifdef __SCALAR_FLOAT__
				mIntegralRegion[l + 1].second = static_cast<double>(mVar[l]);
#else
				mIntegralRegion[l + 1].second = mVar[l];
#endif
			}

			double inv_sqrt_t = 1.0 / AQLMath::sqrt(mTimeGrid[j]);
			for (l = k; l < wt_size; l++)
			{
				double vol = mVolatility[l][0]->integral(mIntegralRegion);
#ifdef __SCALAR_FLOAT__
				mVar_LJ[l] =  static_cast<SCALAR>(mDrift[l]->integral(mIntegralRegion));
				if (mType == DIVIDEdXbyX) mVar_LJ[l] -= static_cast<SCALAR>(0.5 * vol * vol);
				mVar_LJ[l] += static_cast<SCALAR>(vol * mWt[l] * inv_sqrt_t);
				if (mpIntegral->getIntegralType() == LOG_INTEGRAL) mVar_LJ[l] = static_cast<SCALAR>(mIntegralRegion[l + 1].first * AQLMath::exp(mVar_LJ[l]));
				else mVar_LJ[l] += static_cast<SCALAR>(mIntegralRegion[l + 1].first);
#else
				mVar_LJ[l] = mDrift[l]->integral(mIntegralRegion);
				if (mType == DIVIDEdXbyX) mVar_LJ[l] -= 0.5 * vol * vol;
				mVar_LJ[l] += vol * mWt[l] * inv_sqrt_t;
				if (mpIntegral->getIntegralType() == LOG_INTEGRAL) mVar_LJ[l] = mIntegralRegion[l + 1].first * AQLMath::exp(mVar_LJ[l]);
				else mVar_LJ[l] += mIntegralRegion[l + 1].first;
#endif

			}
			mPath_LJ[j]->set(mVar_LJ);
				
	
			// set curve to numeraire(case of spot rate ir model)
			if (mpNumeraire_LJ != 0)
				mpNumeraire_LJ->setCurve(mTimeGrid[j], dynamic_cast<const AQLRatesPathElementCurve*>(mPath_LJ[j]));				


			j++;
		}

	}	
}
/*!
	@brief set numeraire
	@param[in] pnumeraire	pointer to numeraire
	@note this class becomes pointer owner of the input numeraire 
*/
void
AQLRatesLJTermStructureSDE::setNumeraire(AQLRatesNumeraireBase* pnumeraire)
{
	AQLRatesSDEBase::setNumeraire(pnumeraire);
	if (mpNumeraire != 0)
	{
		mpNumeraire_LJ = dynamic_cast<AQLRatesNumeraireBase*>(mpNumeraire->clone());
		mpNumeraire_LJ->setLongJump(true);
	}

}
/*!
    @brief set up this class for path calculation
*/
void
AQLRatesLJTermStructureSDE::setUp()
{
	AQLRatesTermStructureSDE::setUp();
	mPath_LJ.resize(mPath.size());
	for (unsigned int i = 0; i < mPath.size(); i++)
		mPath_LJ[i] = mPath[i]->clone();
	mVar_LJ.resize(mPath[0]->get().size());
	mWt.resize(mVar_LJ.size());
	const SCALARARRAY& var = mPath[0]->get();
	mIntegralRegion.resize(mVar_LJ.size() + 1);
	mIntegralRegion[0].first = 0.0;
	for (unsigned int i = 1; i < mIntegralRegion.size(); i++)
	{
#ifdef __SCALAR_FLOAT__		
		mIntegralRegion[i].first = static_cast<double>(var[i - 1]);
#else
		mIntegralRegion[i].first = var[i - 1];
#endif
	}
	if (mpTemplate->isTypeOf(PE_CURVE) && mpNumeraire_LJ != 0)
		mpNumeraire_LJ->setInterpolationMethod(mpInter);


}