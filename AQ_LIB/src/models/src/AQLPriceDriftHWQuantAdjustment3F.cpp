/*! @file
    @brief Source code of drift function of quant adjustment of HW model
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftHWQuantAdjustment3F.h"
#include "AQLPriceFXVolatility.h"
#include "AQLMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathAttrSDE.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathVolFuncBase.h"
#include "AQLRatesSpotSDE.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLModelDynamicsScalar.h"
#include "AQLModelDynamicsHW1FCurve.h"
#include "AQLAlgorithm.h"
#include "AQLConstant.h"
#include "AQL1DDataSet.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLGaussLegendre.h"
#include "AQLCombinationFunc.h"
#include "AQLBasic.h"
#include "AQLPriceDriftHW.h"
#include "AQLMathVolFuncFXDD.h"
#include "AQLPriceQuantAdjustmentHWFXDD.h"
#include "AQLRatesSpotSDEQuantAdjustment.h"


using namespace std;

#define GAUSSLEGENDREPOINTNUM 20


//================ AQLPriceDriftHWQuantAdjustment3F ===================================
/*!
	@brief default constructor
	@param[in] pDriftIR drift class before quant adjustment
*/
AQLPriceDriftHWQuantAdjustment3F::AQLPriceDriftHWQuantAdjustment3F(const AQLRatesNumeraireBase& domestic_nu, const AQLMathVolFuncFXDD& fxvol, AQLRatesSpotSDE& fx_sde, 
									AQLPriceDriftHW* pDriftIR, const DoubleArray& cor,
									const double fx_criteria)
: AQLPriceDriftHWQuantAdjustment(domestic_nu, fxvol, fx_sde, pDriftIR, cor, fx_criteria)
{

}



/*!
	@brief default constructor
	@param[in] sdeAttrNameIR_D data name of domestic ir model
	@param[in] sdeAttrNameIR_F data name of foreign ir model
	@param[in] sdeAttrNameFX data name of fx model
	@param[in] pDriftIR drift class before quant adjustment
*/
AQLPriceDriftHWQuantAdjustment3F::AQLPriceDriftHWQuantAdjustment3F(const AQLString& sdeAttrNameIR_D, 
													   const AQLString& sdeAttrNameIR_F, 
													   const AQLString& sdeAttrNameFX, 
													   AQLPriceDriftHW* pDriftIR,
													   const double fx_criteria)
: AQLPriceDriftHWQuantAdjustment(sdeAttrNameIR_D, sdeAttrNameIR_F, sdeAttrNameFX, pDriftIR, fx_criteria)
{
	if (pDriftIR == 0)
	{
		//error
		throw AQLCoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}
}


/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLPriceDriftHWQuantAdjustment3F::AQLPriceDriftHWQuantAdjustment3F(const AQLPriceDriftHWQuantAdjustment3F& v) 
: AQLPriceDriftHWQuantAdjustment(v)
{

}

/*!
	@brief destructor
*/
AQLPriceDriftHWQuantAdjustment3F::~AQLPriceDriftHWQuantAdjustment3F() 
{
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftHWQuantAdjustment3F::clone() const	
{
    try 
	{
		return new AQLPriceDriftHWQuantAdjustment3F(*this);
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
AQLPriceDriftHWQuantAdjustment3F::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTHWQUANTADJ3F ? true : AQLMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftHWQuantAdjustment3F::getType() const
{
	return FN_DRIFTHWQUANTADJ3F;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = ts, x[1] = te, x[2] = rf
    @return drift value

*/
double
AQLPriceDriftHWQuantAdjustment3F::operator()(const DoubleArray& x) const
{	
	return (*mpDriftIR)(x);
}



/*!
	@brief set up this class
	@param[in] path path object 
*/
void
AQLPriceDriftHWQuantAdjustment3F::setUp(AQLMathPathEntity& path)
{
	AQLPriceDriftHWQuantAdjustment::setUp(path);
	//foreign numeraire
	AQLDataHolder* dh = &path.getData(mSDEAttrNameIR_F, ISNOTNULL);
	AQLMathAttrSDE &attrsde = dynamic_cast<AQLMathAttrSDE &>(dh->get());
	if (!attrsde.getSDE().isTypeOf(FN_SPOTSDEQUANTADJUSTMENT))
	{
		throw AQLCoreInvalidData("Foregin sde must be AQLRatesSpotSDEQuantAdjustment", __FILE__, __LINE__);
	}
	AQLPriceQuantAdjustmentFuncBase &qa = dynamic_cast<AQLRatesSpotSDEQuantAdjustment &>(attrsde.getSDE()).getQuantAdjuster();

	if (!qa.isTypeOf(FN_QUANTADJUSTMENTFXDD))
	{
		throw AQLCoreInvalidData("QuantAdjustmentFunc must be AQLPriceQuantAdjustmentHWFXDD", __FILE__, __LINE__);
	}
	mpQuantAdjuster = &dynamic_cast<AQLPriceQuantAdjustmentHWFXDD &>(qa);
	
}

/*!
	@brief get quanto adjuster
*/
const AQLPriceQuantAdjustmentHWFXDD &
AQLPriceDriftHWQuantAdjustment3F::getQuantAdjuster() const
{
	if (!mpQuantAdjuster)
	{
		throw AQLCoreInvalidData("mpQuantAdjuster is NULL", __FILE__, __LINE__);
	}
	return *mpQuantAdjuster;
}


