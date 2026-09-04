/*! @file
    @brief Source code of spot sde class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesSpotSDE.h"
#include "AQLRatesSDEIntegralBase.h"
#include "AQLAlgorithm.h"
#include "AQLRatesHWIntegral.h"
#include "AQLMathVolFuncBase.h"
#include "AQLMathVolFuncHW.h"

using namespace std;
//================ AQLRatesSpotSDE ===================================
/*!
	@brief default constructor
	@param[in] type sde type
	@param[in] pTransformer transformation fcuntion from input/output to sde variable X'=f(t,X)
	@param[in] pInvTransformer inverse of transformation fcuntion X = f^{-1}(t,X')
	@param[in] pAdjuster adjustment function
*/
AQLRatesSpotSDE::AQLRatesSpotSDE(SDE_TYPE type, AQLFunctionBase* pTransformer, AQLFunctionBase* pInvTransformer, AQLFunctionBase* pAdjuster)
: AQLRatesSDEBase(type), mpPathElement(0), mpTransformer(pTransformer), mpInvTransformer(pInvTransformer), mpAdjuster(pAdjuster)
{
	if ((pTransformer == 0 && pInvTransformer != 0)
		|| (pTransformer != 0 && pInvTransformer == 0))
	{
		//error
		throw AQLCoreInvalidData("One of Transformer and InvTransformer is Null, but the other is not NULL", __FILE__, __LINE__);
	}
}
/*!
	@brief copy constructor
	@param[in] copy source
*/
AQLRatesSpotSDE::AQLRatesSpotSDE(const AQLRatesSpotSDE& v) 
: AQLRatesSDEBase(v), mpPathElement(0), mpTransformer(0), mpInvTransformer(0), mpAdjuster(0)
{
	if (v.mpPathElement != 0)
		mpPathElement = dynamic_cast<AQLRatesPathElementBase*>(v.mpPathElement->clone());
	if (v.mpTransformer != 0)
		mpTransformer = dynamic_cast<AQLFunctionBase*>(v.mpTransformer->clone());
	if (v.mpInvTransformer != 0)
		mpInvTransformer = dynamic_cast<AQLFunctionBase*>(v.mpInvTransformer->clone());
	if (v.mpAdjuster != 0)
		mpAdjuster = dynamic_cast<AQLFunctionBase*>(v.mpAdjuster->clone());

	mVar = v.mVar;
}

/*!
	@brief destructor
*/
AQLRatesSpotSDE::~AQLRatesSpotSDE() 
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
AQLCoreFunctionBase*	
AQLRatesSpotSDE::clone() const
{
    try 
	{
		return new AQLRatesSpotSDE(*this);
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
AQLRatesSpotSDE::isTypeOf(function_t id) const
{
	return (id==FN_SPOTSDE ? true : AQLRatesSDEBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesSpotSDE::getType() const
{
	return FN_SPOTSDE;
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
AQLRatesSpotSDE::check(void) const
{
	return AQLRatesSDEBase::check();
}


/*!
    @brief get path element
	@param[in] pos position of sde integral grid
    @return path element
*/
const AQLRatesPathElementBase*
AQLRatesSpotSDE::getPathElement(unsigned int pos)
{
	const DoubleArray& timegrid = mpBM->getTimeGrid();
	if (pos > timegrid.size() - 1)
	{
		throw AQLCoreInvalidData("pos is over size", __FILE__, __LINE__);
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
AQLRatesSpotSDE::calcPath(unsigned int pos)
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
			mpNumeraire->setCurve(0, dynamic_cast<const AQLRatesPathElementCurve*>(mPath[0]));
		}
		if(mpIntegral->isTypeOf(FN_HWINTEGRAL))
		{
			dynamic_cast<AQLRatesHWIntegral*>(mpIntegral)->setUpInitialVal(mVar);
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
			mVar[0] =  static_cast<SCALAR>((*mpTransformer)(var));
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
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos_s], mTimeGrid.size(), j);
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

			// set curve to numeraire //
			if (mpNumeraire != 0 && mpTemplate->isTypeOf(PE_CURVE))
				mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const AQLRatesPathElementCurve*>(mPath[j]));
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
AQLRatesSpotSDE::setUp()
{
	AQLRatesSDEBase::setUp();

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
	//		dynamic_cast<AQLRatesHWIntegral*>(mpIntegral)->setUpInitialVal(*mpTemplate,mVar);
	//	}
	//	else
	//	{
	//		const AQLMathVolFuncHW* mpvolHW=0;

	//		if(mVolatility[0][0]->isTypeOf(FN_VOLFUNCHW))
	//			mpvolHW = dynamic_cast<AQLMathVolFuncHW*>(mVolatility[0][0]);
	//		else
	//		{
	//			const AQLFunctionBase* pbase = dynamic_cast<AQLMathVolFuncBase*>(mVolatility[0][0])->getVolatility();
	//			mpvolHW = dynamic_cast<const AQLMathVolFuncHW*>(pbase);
	//		}

	//		const AQLMathHWFuncToolForVar* pTool =  mpvolHW->getHWFuncTool();
	//		AQLMathHWFuncMR* pMR = pTool->getHWMR();
	//		AQLMathHWFuncSigma* pSigma = pTool->getHWSigma();

	//		AQLRatesPathElementHW1FCurveTMDPT curve0(0.);

	//		delete curve0.mpHWtoolMR;
	//		curve0.mpHWtoolMR = new AQLMathHWFuncToolForMR(*dynamic_cast<AQLMathHWFuncMR*>(pMR->clone()),
	//												*dynamic_cast<AQLMathHWFuncSigma*>(pSigma->clone()));

	//		delete curve0.mpHWtoolVar;
	//		curve0.mpHWtoolVar = new AQLMathHWFuncToolForVar(*dynamic_cast<AQLMathHWFuncMR*>(pMR->clone()),
	//												*dynamic_cast<AQLMathHWFuncSigma*>(pSigma->clone()));

	//		dynamic_cast<AQLRatesHWIntegral*>(mpIntegral)->setUpInitialVal(curve0,mVar);
	//	}
	//}

}
