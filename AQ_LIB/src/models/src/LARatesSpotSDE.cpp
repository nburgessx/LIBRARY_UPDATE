/*! @file
    @brief Source code of spot sde class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesSpotSDE.cpp
//
//  SYNOPSIS    :       LARatesSpotSDE
//  DESCRIPTION :       Source code of spot sde class
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


#include "LARatesSpotSDE.h"
#include "LARatesSDEIntegralBase.h"
#include "LAAlgorithm.h"
#include "LARatesHWIntegral.h"
#include "LAMathVolFuncBase.h"
#include "LAMathVolFuncHW.h"

using namespace std;
//================ LARatesSpotSDE ===================================
/*!
	@brief default constructor
	@param[in] type sde type
	@param[in] pTransformer transformation fcuntion from input/output to sde variable X'=f(t,X)
	@param[in] pInvTransformer inverse of transformation fcuntion X = f^{-1}(t,X')
	@param[in] pAdjuster adjustment function
*/
LARatesSpotSDE::LARatesSpotSDE(SDE_TYPE type, LAFunctionBase* pTransformer, LAFunctionBase* pInvTransformer, LAFunctionBase* pAdjuster)
: LARatesSDEBase(type), mpPathElement(0), mpTransformer(pTransformer), mpInvTransformer(pInvTransformer), mpAdjuster(pAdjuster)
{
	if ((pTransformer == 0 && pInvTransformer != 0)
		|| (pTransformer != 0 && pInvTransformer == 0))
	{
		//error
		throw LACoreInvalidData("One of Transformer and InvTransformer is Null, but the other is not NULL", __FILE__, __LINE__);
	}
}
/*!
	@brief copy constructor
	@param[in] copy source
*/
LARatesSpotSDE::LARatesSpotSDE(const LARatesSpotSDE& v) 
: LARatesSDEBase(v), mpPathElement(0), mpTransformer(0), mpInvTransformer(0), mpAdjuster(0)
{
	if (v.mpPathElement != 0)
		mpPathElement = dynamic_cast<LARatesPathElementBase*>(v.mpPathElement->clone());
	if (v.mpTransformer != 0)
		mpTransformer = dynamic_cast<LAFunctionBase*>(v.mpTransformer->clone());
	if (v.mpInvTransformer != 0)
		mpInvTransformer = dynamic_cast<LAFunctionBase*>(v.mpInvTransformer->clone());
	if (v.mpAdjuster != 0)
		mpAdjuster = dynamic_cast<LAFunctionBase*>(v.mpAdjuster->clone());

	mVar = v.mVar;
}

/*!
	@brief destructor
*/
LARatesSpotSDE::~LARatesSpotSDE() 
{
	mVar.clear();
	delete mpPathElement;
	delete mpTransformer;
	delete mpInvTransformer;
	delete mpAdjuster;

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesSpotSDE::clone() const
{
    try 
	{
		return new LARatesSpotSDE(*this);
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
LARatesSpotSDE::isTypeOf(function_t id) const
{
	return (id==FN_SPOTSDE ? true : LARatesSDEBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesSpotSDE::getType() const
{
	return FN_SPOTSDE;
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
LARatesSpotSDE::check(void) const
{
	return LARatesSDEBase::check();
}


/*!
    @brief get path element
	@param[in] pos position of sde integral grid
    @return path element
*/
const LARatesPathElementBase*
LARatesSpotSDE::getPathElement(unsigned int pos)
{
	const DoubleArray& timegrid = mpBM->getTimeGrid();
	if (pos > timegrid.size() - 1)
	{
		throw LACoreInvalidData("pos is over size", __FILE__, __LINE__);
	}

	if (mpBM->getCurrentID() == mID && pos == mPos)
		return mpPathElement;
	
	if (mpBM->getCurrentID() != mID)
		mPos = 0;
	
	mID = mpBM->getCurrentID();

    //calculate path
	calcPath(pos);
	
	mPos = pos;
		
	return mpPathElement;
}

/*!
	
	@brief calculate path
	@param[in] pos position of sde integral grid				
*/	
void
LARatesSpotSDE::calcPath(unsigned int pos)
{
	unsigned int pos_s;
	const DoubleArray& grid = mpBM->getTimeGrid();//sde time grid (= BM time grid)

	if (mPos == 0/* || mPos == grid.size() - 1*/)
	{
		mVar = mPath[0]->get();
		if (mpTransformer != 0)
		{
			DoubleArray var(2);
#ifdef __SCALAR_FLOAT__
			var[1] = static_cast<double>(mVar[0]);
			mVar[0] =  static_cast<SCALAR>((*mpTransformer)(var));
#else
			var[1] = mVar[0];
			mVar[0] = (*mpTransformer)(var);
#endif
		}		
		pos_s = 0;
		if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
		{
			mpNumeraire->clear();
			mpNumeraire->setCurve(0, dynamic_cast<const LARatesPathElementCurve*>(mPath[0]));
		}
		if(mpIntegral->isTypeOf(FN_HWINTEGRAL))
		{
			dynamic_cast<LARatesHWIntegral*>(mpIntegral)->setUpInitialVal(mVar);
		}
	}
	else if (pos < mPos)
	{
		unsigned int _pos_s;
		LAAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos], mTimeGrid.size(), _pos_s);
		if (grid[pos] != mTimeGrid[_pos_s])
		{
			if (_pos_s == 0)
			{
				//error
				throw LACoreInvalidData("grid is something wrong", __FILE__, __LINE__);
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
			mVar[0] =  static_cast<SCALAR>((*mpTransformer)(var));
#else
			var[1] = mVar[0];
			mVar[0] = (*mpTransformer)(var);
#endif
		}		
		if (!LAAlgorithm::find<DoubleArray, double>(grid, mTimeGrid[_pos_s], 0, grid.size() - 1, pos_s))
		{
			throw LACoreInvalidData("TimeGrid is inconsistent with BM grid", __FILE__, __LINE__);
		}	
	}
	else 
	{
		pos_s = mPos;
		if (mpTransformer != 0)
		{
			DoubleArray var(2);
			var[0] = grid[pos_s];
#ifdef __SCALAR_FLOAT__
			var[1] = static_cast<double>(mVar[0]);
			mVar[0] =  static_cast<SCALAR>((*mpTransformer)(var));
#else
			var[1] = mVar[0];
			mVar[0] = (*mpTransformer)(var);
#endif
		}
	}
		
	unsigned int j;
	LAAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos_s], mTimeGrid.size(), j);
	if (grid[pos_s] == mTimeGrid[j]) j++;
	
	const DoubleMatrix& bm = mpBM->getBM();
	double setVal = mVar[0];
	for (unsigned int i = pos_s; i < pos; i++)
	{
		mpIntegral->integral(grid[i], grid[i + 1],
							mDrift.begin(), 
							mVolatility.begin(),
							bm[i].begin(),
							mVar.begin(),
							1);
		
		if (mTimeGrid[j] == grid[i + 1])
		{
			if (mpTransformer == 0) 
			{
				if (mpAdjuster)
				{
					DoubleArray var(2);
					var[0] = grid[i + 1];
					var[1] = mVar[0];
#ifdef __SCALAR_FLOAT__
					mVar[0] = static_cast<SCALAR>((*mpAdjuster)(var));
#else
					mVar[0] = (*mpAdjuster)(var);
#endif
				}
				mPath[j]->set(mVar[0]);
				setVal = mVar[0];
			}
			else
			{
				DoubleArray var(2);
				var[0] = grid[i];
#ifdef __SCALAR_FLOAT__
				var[1] = static_cast<double>(mVar[0]);
				if (mpAdjuster)
				{
					var[1] = (*mpInvTransformer)(var);
					var[0] = grid[i + 1];
					double adjVal = (*mpAdjuster)(var);
					mPath[j]->set(static_cast<SCALAR>(adjVal));
					setVal = adjVal;	
				}
				else
				{
					double val = (*mpInvTransformer)(var);
					mPath[j]->set(static_cast<SCALAR>(val));
					setVal = val;
				}
				// set mVar
				if (i + 1 < pos)
				{
					var[0] = grid[i + 1];
					var[1] = setVal;
					mVar[0] = static_cast<SCALAR>((*mpTransformer)(var));
				}
				else
				{
					mVar[0] = static_cast<SCALAR>(setVal);
				}
#else
				var[1] = mVar[0];
				if (mpAdjuster)
				{
					var[1] = (*mpInvTransformer)(var);
					var[0] = grid[i + 1];
					double adjVal = (*mpAdjuster)(var);
					mPath[j]->set(adjVal);
					setVal = adjVal;
				}
				else
				{
					double val = (*mpInvTransformer)(var);
					mPath[j]->set(val);
					setVal = val;
				}
				// set mVar
				if (i + 1 < pos)
				{
					var[0] = grid[i + 1];
					var[1] = setVal;
					mVar[0] = (*mpTransformer)(var);
				}
				else
				{
					mVar[0] = setVal;
				}
#endif
			}

			// set curve to numeraire //スポットレートの金利モデルの場合
			if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
				mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const LARatesPathElementCurve*>(mPath[j]));
			j++;
		}	

	}
	if (pos_s == pos)
	{
		if (mpTransformer)
		{
			DoubleArray var(2);
			var[0] = grid[pos_s];
#ifdef __SCALAR_FLOAT__	
			var[1] = static_cast<double>(mVar[0]);
			double val = (*mpInvTransformer)(var);
			mPath[j]->set(static_cast<SCALAR>(val));
			setVal = val;
			mVar[0] = static_cast<SCALAR>(val);
#else
			var[1] = mVar[0];
			double val = (*mpInvTransformer)(var);
			mPath[j]->set(val);
			setVal = val;
			mVar[0] = val;
#endif
		}
	}
	mpPathElement->set(setVal);
}
/*!
    @brief set up this class for path calculation
*/
void
LARatesSpotSDE::setUp()
{
	LARatesSDEBase::setUp();

	delete mpPathElement;
	mpPathElement = mPath[0]->clone(); 
	if (mpTransformer != 0)
	{
		DoubleArray var(2);
#ifdef __SCALAR_FLOAT__
		var[1] = static_cast<double>(mPath[0]->get()[0]);
		SCALAR val =  static_cast<SCALAR>((*mpTransformer)(var));
#else
		var[1] = static_cast<double>(mPath[0]->get()[0]);
		double val = (*mpTransformer)(var);
#endif
		mpPathElement->set(val);
	}

	//if(mpIntegral->isTypeOf(FN_HWINTEGRAL))
	//{
	//	if(mpTemplate->isTypeOf(PE_HWCURVETMDPT))
	//	{
	//		dynamic_cast<LARatesHWIntegral*>(mpIntegral)->setUpInitialVal(*mpTemplate,mVar);
	//	}
	//	else
	//	{
	//		const LAMathVolFuncHW* mpvolHW=0;

	//		if(mVolatility[0][0]->isTypeOf(FN_VOLFUNCHW))
	//			mpvolHW = dynamic_cast<LAMathVolFuncHW*>(mVolatility[0][0]);
	//		else
	//		{
	//			const LAFunctionBase* pbase = dynamic_cast<LAMathVolFuncBase*>(mVolatility[0][0])->getVolatility();
	//			mpvolHW = dynamic_cast<const LAMathVolFuncHW*>(pbase);
	//		}

	//		const LAMathHWFuncToolForVar* pTool =  mpvolHW->getHWFuncTool();
	//		LAMathHWFuncMR* pMR = pTool->getHWMR();
	//		LAMathHWFuncSigma* pSigma = pTool->getHWSigma();

	//		LARatesPathElementHW1FCurveTMDPT curve0(0.);

	//		delete curve0.mpHWtoolMR;
	//		curve0.mpHWtoolMR = new LAMathHWFuncToolForMR(*dynamic_cast<LAMathHWFuncMR*>(pMR->clone()),
	//												*dynamic_cast<LAMathHWFuncSigma*>(pSigma->clone()));

	//		delete curve0.mpHWtoolVar;
	//		curve0.mpHWtoolVar = new LAMathHWFuncToolForVar(*dynamic_cast<LAMathHWFuncMR*>(pMR->clone()),
	//												*dynamic_cast<LAMathHWFuncSigma*>(pSigma->clone()));

	//		dynamic_cast<LARatesHWIntegral*>(mpIntegral)->setUpInitialVal(curve0,mVar);
	//	}
	//}

}
