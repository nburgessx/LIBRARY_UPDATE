/*! @file
    @brief Source code of term structure sde class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRatesTermStructureSDE.cpp
//
//  SYNOPSIS    :       AQLRatesTermStructureSDE
//  DESCRIPTION :       Source code of term structure sde class
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


#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesSDEIntegralBase.h"

#include "AQLAlgorithm.h"

#define ISINCLUEDECAP
using namespace std;
//================ AQLRatesTermStructureSDE ===================================
/*!
	@brief default constructor
	@param[in] type sde type
*/
AQLRatesTermStructureSDE::AQLRatesTermStructureSDE(SDE_TYPE type)
: AQLRatesSDEBase(type), mCapRatio(-100.0)
{

}
/*!
	@brief copy constructor
*/
/*AQLRatesTermStructureSDE::AQLRatesTermStructureSDE(const AQLRatesTermStructureSDE& v) 
: AQLRatesSDEBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLRatesTermStructureSDE::~AQLRatesTermStructureSDE() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesTermStructureSDE::clone() const
{
    try 
	{
		return new AQLRatesTermStructureSDE(*this);
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
AQLRatesTermStructureSDE::isTypeOf(function_t id) const
{
	return (id==FN_TERMSTRUCTURESDE ? true : AQLRatesSDEBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesTermStructureSDE::getType() const
{
	return FN_TERMSTRUCTURESDE;
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
AQLRatesTermStructureSDE::check(void) const
{
	return AQLRatesSDEBase::check();
}

/*!
    @brief get path element
	@param[in] pos position of sde integral grid
    @return path element
*/
const AQLRatesPathElementBase*
AQLRatesTermStructureSDE::getPathElement(unsigned int pos)
{
	const DoubleArray& grid = mpBM->getTimeGrid();
	if (pos > grid.size() - 1)
	{
		throw AQLCoreInvalidData("pos is over size", __FILE__, __LINE__);
	}

	unsigned int pos_e;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos], mTimeGrid.size(), pos_e);

	if (mpBM->getCurrentID() == mID && pos <= mPos)
	{
		if (grid[pos] == mTimeGrid[pos_e])
			return mPath[pos_e];
		else
		{
			return &mpInter->value(grid[pos], mTimeGrid[pos_e - 1], mTimeGrid[pos_e], *mPath[pos_e - 1], *mPath[pos_e]);
		}
		
	}
	
	if (mpBM->getCurrentID() != mID)
		mPos = 0;

	
	mID = mpBM->getCurrentID();

	unsigned int _pos_e;
	if (!AQLAlgorithm::find<DoubleArray, double>(grid, mTimeGrid[pos_e], 0, grid.size() - 1, _pos_e))
	{
		//error
		throw AQLCoreInvalidData("grid is something wrong", __FILE__, __LINE__);
	}
    //calculate path
	calcPath(_pos_e);
	
	if (mPos < _pos_e) mPos = _pos_e;
	
	if (grid[pos] == mTimeGrid[pos_e]) return mPath[pos_e];
	return &mpInter->value(grid[pos], mTimeGrid[pos_e - 1], mTimeGrid[pos_e], *mPath[pos_e - 1], *mPath[pos_e]);
}


/*!
    @brief calculate path
*/
/*
void
AQLRatesTermStructureSDE::calcPath(void)
{
	
	const SCALARARRAY& var = *reinterpret_cast<const SCALARARRAY*>(mPath[0]->get());//initial value 
	for (unsigned int i = 0; i < var.size(); i++)
		mVar[i] = var[i]; 
	
	const DoubleArray& grid = mpBM->getTimeGrid();//sde time grid (= BM time grid)

//	unsigned int j = 1;
//	unsigned int k = 0;
//	for (unsigned int i = 1; i < grid.size(); i++)
//	{
//		if (grid[i + 1] > mResetTimes[k])
//		{	
//			k++;
//		}
//		mpIntegral->integral(grid[i], grid[i + 1],
//							mDrift.begin() + k, 
//							mVolatility.begin() + k,
//							mpBM->getBM()[i - 1].begin() + k,
//							mVar.begin() + k,
//							mVar.size() - k);
//	
//		if (mTimeGrid[j] == grid[i])
//			mPath[j++]->set(&mVar); 
//	}

	unsigned int j = 1;
	unsigned int k = 0;
	//unsigned int l = 0;
	for (unsigned int i = 1; i < grid.size(); i++)
	{
		if (grid[i] > mTimeGrid[k + 1])
		{	
			k++;
		}
		//if (mVolatility.begin()->size() == 1) 
		//	l = k;

		mpIntegral->integral(grid[i - 1], grid[i],
							mDrift.begin() + k, 
							mVolatility.begin() + k,
							mpBM->getBM()[i - 1].begin(),// + l,
							mVar.begin() + k,
							mVar.size() - k);
	
		if (mTimeGrid[j] == grid[i])
		{
			mPath[j]->set(&mVar); 
			// set curve to numeraire 
			if (mpNumeraire != 0)
				mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const AQLRatesPathElementCurve*>(mPath[j]));
			j++;
		}
	}	

}
*/
/*!
	
	@brief calculate path
	@param[in] pos position of sde integral grid				
*/	
void
AQLRatesTermStructureSDE::calcPath(unsigned int pos)
{
	if (mPos == 0)
	{
//		const SCALARARRAY& var = *reinterpret_cast<const SCALARARRAY*>(mPath[0]->get());//initial value 
//		for (unsigned int i = 0; i < var.size(); i++) mVar[i] = var[i];
		mVar = mPath[0]->get();
		if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const AQLRatesPathElementCurve*>(mPath[0]));
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
#ifdef ISINCLUEDECAP
		if(mCapRatio >=0.0)
		{
			for(unsigned int l = k;l<mVar.size();l++)
			{
				if(mVar[l] > mCAP[l] )
				{
					mVar[l] = mCAP[l];
				}
					
			}
		}

#endif
		if (mTimeGrid[j] == grid[i + 1])
		{
			mPath[j]->set(mVar); 
			// set curve to numeraire 
			if (mpNumeraire != 0)
				mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const AQLRatesPathElementCurve*>(mPath[j]));
			j++;
		}

	}	

/*	unsigned int pos_s;
	const DoubleArray& grid = mpBM->getTimeGrid();//sde time grid (= BM time grid)
	if (mPos == 0 || mPos == grid.size() - 1)
	{
		const SCALARARRAY& var = *reinterpret_cast<const SCALARARRAY*>(mPath[0]->get());//initial value 
		for (unsigned int i = 0; i < var.size(); i++) mVar[i] = var[i];
		pos_s = 0;
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
		const SCALARARRAY& var = *reinterpret_cast<const SCALARARRAY*>(mPath[_pos_s]->get()); 
		int i, j;
		for (i = var.size() - 1, j = mVar.size() - 1; i >= 0; i--, j--)
			mVar[j] = var[i];
		
		if (!AQLAlgorithm::find<DoubleArray, double>(grid, mTimeGrid[_pos_s], 0, grid.size() - 1, pos_s))
		{
			throw AQLCoreInvalidData("TimeGrid is inconsistent with BM grid", __FILE__, __LINE__);
		}	
	}
	else pos_s = mPos;
		
	unsigned int j;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos_s], mTimeGrid.size(), j);
	if (grid[pos_s] == mTimeGrid[j]) j++;
	
	unsigned int pos_e;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos], mTimeGrid.size(), pos_e);


	unsigned int k = 0;
	const DoubleMatrix& bm = mpBM->getBM();
	for (unsigned int i = pos_s; i < pos_e; i++)
	{
		k = mVar.size() - mpBM->getBM()[i].size(); 
		mpIntegral->integral(grid[i], grid[i + 1],
							mDrift.begin() + k, 
							mVolatility.begin() + k,
							bm[i].begin(),
							mVar.begin() + k,
							mVar.size() - k);
		if (mTimeGrid[j] == grid[i + 1])
		{
			mPath[j]->set(&mVar); 
			// set curve to numeraire 
			if (mpNumeraire != 0)
				mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const AQLRatesPathElementCurve*>(mPath[j]));
			j++;
		}

	}	

	mpPathElement->set(&mVar);
*/
}

/*void
AQLRatesTermStructureSDE::setResetTimes(const DoubleArray& resettimes) 
{
	mResetTimes = resettimes;
	mVar.clear();
	mVar.resize(mResetTimes.size());
}*/


/*!
    @brief set up this class for path calculation
*/
void
AQLRatesTermStructureSDE::setUp()
{
	AQLRatesSDEBase::setUp();
#ifdef ISINCLUEDECAP
	SCALARARRAY initial_L = mPath[0]->get();
	mCAP.resize(initial_L.size());
	
	for (unsigned int i = 0; i < mCAP.size(); ++i)
		mCAP[i] =  AQLMath::max(0.05, AQLMath::abs(mCapRatio * initial_L[i])) ;
#endif
}
