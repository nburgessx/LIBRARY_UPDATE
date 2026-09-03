/*! @file
    @brief Source code of spot sde class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesSpotSDEQuantAdjustment.cpp
//
//  SYNOPSIS    :       LARatesSpotSDEQuantAdjustment
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


#include "LARatesSpotSDEQuantAdjustment.h"
#include "LARatesSDEIntegralBase.h"
#include "AQLAlgorithm.h"
#include "LARatesHWIntegral.h"
#include "LAPriceQuantAdjustmentFuncBase.h"

using namespace std;
//================ LARatesSpotSDEQuantAdjustment ===================================
/*!
	@brief default constructor
	@param[in] type sde type
	@param[in] pTransformer transformation fcuntion from input/output to sde variable X'=f(t,X)
	@param[in] pInvTransformer inverse of transformation fcuntion X = f^{-1}(t,X')
	@param[in] pAdjuster adjustment function
*/
LARatesSpotSDEQuantAdjustment::LARatesSpotSDEQuantAdjustment(SDE_TYPE type, LAPriceQuantAdjustmentFuncBase* pQuantAduster, AQLFunctionBase* pTransformer, AQLFunctionBase* pInvTransformer, AQLFunctionBase* pAdjuster)
: LARatesSpotSDE(type, pTransformer, pInvTransformer, pAdjuster), mpQuantAduster(pQuantAduster)
{

}
/*!
	@brief copy constructor
	@param[in] copy source
*/
LARatesSpotSDEQuantAdjustment::LARatesSpotSDEQuantAdjustment(const LARatesSpotSDEQuantAdjustment& v) 
: LARatesSpotSDE(v), mpQuantAduster(v.mpQuantAduster)
{

}

/*!
	@brief destructor
*/
LARatesSpotSDEQuantAdjustment::~LARatesSpotSDEQuantAdjustment() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LARatesSpotSDEQuantAdjustment::clone() const
{
    try 
	{
		return new LARatesSpotSDEQuantAdjustment(*this);
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
LARatesSpotSDEQuantAdjustment::isTypeOf(function_t id) const
{
	return (id==FN_SPOTSDEQUANTADJUSTMENT ? true : LARatesSpotSDE::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesSpotSDEQuantAdjustment::getType() const
{
	return FN_SPOTSDEQUANTADJUSTMENT;
}



/*!
	
	@brief calculate path
	@param[in] pos position of sde integral grid				
*/	
void
LARatesSpotSDEQuantAdjustment::calcPath(unsigned int pos)
{
	LARatesSpotSDE::calcPath(pos);

	if (!mpQuantAduster)
	{
		throw AQLCoreInvalidData("mpQuantAduster is NULL", __FILE__, __LINE__);
	}
	const DoubleArray& grid = mpBM->getTimeGrid();//sde time grid (= BM time grid)
	unsigned int pos_s = mPos;		
	unsigned int j;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, grid[pos_s], mTimeGrid.size(), j);
	if (grid[pos_s] == mTimeGrid[j]) j++;
	
	const DoubleMatrix& bm = mpBM->getBM();
	double setVal = mPath[pos_s]->get()[0];
	for (unsigned int i = pos_s; i < pos; i++)
	{
		if (mTimeGrid[j] == grid[i + 1])
		{
			const double r_noqa = dynamic_cast<const LARatesPathElementSRCurve *>(mPath[j])->get_r();
			const double quantadjust = mpQuantAduster->getQuantAdjust(grid[i]);
			setVal = r_noqa - quantadjust;
			mPath[j]->set(setVal);
			mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const LARatesPathElementCurve*>(mPath[j]));
			j++;
		}	
	}
	mpPathElement->set(setVal);
}

/*!
    @brief check whether preparation of calcuation is finished or not
    @return check status
*/
bool
LARatesSpotSDEQuantAdjustment::check(void) const
{
	bool ret =  LARatesSpotSDE::check();
	if (!mpQuantAduster)
	{
		ret = false;
	}
	return ret;
}

/*!
    @brief get quanto adjuster
*/
LAPriceQuantAdjustmentFuncBase	&
LARatesSpotSDEQuantAdjustment::getQuantAdjuster(void)
{
	if (!mpQuantAduster)
	{
		throw AQLCoreInvalidData("mpQuantAduster is NULL", __FILE__, __LINE__);
	}
	return *mpQuantAduster;
}