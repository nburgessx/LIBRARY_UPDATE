/*! @file
    @brief Source code of spot sde class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesSpotSDEQuantAdjustment.h"
#include "AQLRatesSDEIntegralBase.h"
#include "AQLAlgorithm.h"
#include "AQLRatesHWIntegral.h"
#include "AQLPriceQuantAdjustmentFuncBase.h"

using namespace std;
//================ AQLRatesSpotSDEQuantAdjustment ===================================
/*!
	@brief default constructor
	@param[in] type sde type
	@param[in] pTransformer transformation fcuntion from input/output to sde variable X'=f(t,X)
	@param[in] pInvTransformer inverse of transformation fcuntion X = f^{-1}(t,X')
	@param[in] pAdjuster adjustment function
*/
AQLRatesSpotSDEQuantAdjustment::AQLRatesSpotSDEQuantAdjustment(SDE_TYPE type, AQLPriceQuantAdjustmentFuncBase* pQuantAduster, AQLFunctionBase* pTransformer, AQLFunctionBase* pInvTransformer, AQLFunctionBase* pAdjuster)
: AQLRatesSpotSDE(type, pTransformer, pInvTransformer, pAdjuster), mpQuantAduster(pQuantAduster)
{

}
/*!
	@brief copy constructor
	@param[in] copy source
*/
AQLRatesSpotSDEQuantAdjustment::AQLRatesSpotSDEQuantAdjustment(const AQLRatesSpotSDEQuantAdjustment& v) 
: AQLRatesSpotSDE(v), mpQuantAduster(v.mpQuantAduster)
{

}

/*!
	@brief destructor
*/
AQLRatesSpotSDEQuantAdjustment::~AQLRatesSpotSDEQuantAdjustment() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesSpotSDEQuantAdjustment::clone() const
{
    try 
	{
		return new AQLRatesSpotSDEQuantAdjustment(*this);
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
AQLRatesSpotSDEQuantAdjustment::isTypeOf(function_t id) const
{
	return (id==FN_SPOTSDEQUANTADJUSTMENT ? true : AQLRatesSpotSDE::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesSpotSDEQuantAdjustment::getType() const
{
	return FN_SPOTSDEQUANTADJUSTMENT;
}



/*!
	
	@brief calculate path
	@param[in] pos position of sde integral grid				
*/	
void
AQLRatesSpotSDEQuantAdjustment::calcPath(unsigned int pos)
{
	AQLRatesSpotSDE::calcPath(pos);

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
			const double r_noqa = dynamic_cast<const AQLRatesPathElementSRCurve *>(mPath[j])->get_r();
			const double quantadjust = mpQuantAduster->getQuantAdjust(grid[i]);
			setVal = r_noqa - quantadjust;
			mPath[j]->set(setVal);
			mpNumeraire->setCurve(mTimeGrid[j], dynamic_cast<const AQLRatesPathElementCurve*>(mPath[j]));
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
AQLRatesSpotSDEQuantAdjustment::check(void) const
{
	bool ret =  AQLRatesSpotSDE::check();
	if (!mpQuantAduster)
	{
		ret = false;
	}
	return ret;
}

/*!
    @brief get quanto adjuster
*/
AQLPriceQuantAdjustmentFuncBase	&
AQLRatesSpotSDEQuantAdjustment::getQuantAdjuster(void)
{
	if (!mpQuantAduster)
	{
		throw AQLCoreInvalidData("mpQuantAduster is NULL", __FILE__, __LINE__);
	}
	return *mpQuantAduster;
}