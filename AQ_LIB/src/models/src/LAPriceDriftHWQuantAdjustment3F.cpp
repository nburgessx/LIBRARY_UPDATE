/*! @file
    @brief Source code of drift function of quant adjustment of HW model



*/
//  2008, Mizuho International London.. 

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftHWQuantAdjustment3F.cpp
//
//  SYNOPSIS    :       LAPriceDriftHWQuantAdjustment3F
//  DESCRIPTION :       Source code of drift function of quant adjustment of HW model
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


#include "LAPriceDriftHWQuantAdjustment3F.h"
#include "LAPriceFXVolatility.h"
#include "LAMathPathEntity.h"
#include "LADataHolder.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LAMathAttrSDE.h"
#include "LAPriceDataFunction.h"
#include "LAMathVolFuncBase.h"
#include "LARatesSpotSDE.h"
#include "LAModelDynamicsCurve.h"
#include "LAModelDynamicsScalar.h"
#include "LAModelDynamicsHW1FCurve.h"
#include "LAAlgorithm.h"
#include "LAConstant.h"
#include "LA1DDataSet.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAGaussLegendre.h"
#include "LACombinationFunc.h"
#include "LABasic.h"
#include "LAPriceDriftHW.h"
#include "LAMathVolFuncFXDD.h"
#include "LAPriceQuantAdjustmentHWFXDD.h"
#include "LARatesSpotSDEQuantAdjustment.h"


using namespace std;

#define GAUSSLEGENDREPOINTNUM 20


//================ LAPriceDriftHWQuantAdjustment3F ===================================
/*!
	@brief default constructor
	@param[in] pDriftIR drift class before quant adjustment
*/
LAPriceDriftHWQuantAdjustment3F::LAPriceDriftHWQuantAdjustment3F(const LARatesNumeraireBase& domestic_nu, const LAMathVolFuncFXDD& fxvol, LARatesSpotSDE& fx_sde, 
									LAPriceDriftHW* pDriftIR, const DoubleArray& cor,
									const double fx_criteria)
: LAPriceDriftHWQuantAdjustment(domestic_nu, fxvol, fx_sde, pDriftIR, cor, fx_criteria)
{

}



/*!
	@brief default constructor
	@param[in] sdeAttrNameIR_D data name of domestic ir model
	@param[in] sdeAttrNameIR_F data name of foreign ir model
	@param[in] sdeAttrNameFX data name of fx model
	@param[in] pDriftIR drift class before quant adjustment
*/
LAPriceDriftHWQuantAdjustment3F::LAPriceDriftHWQuantAdjustment3F(const LAString& sdeAttrNameIR_D, 
													   const LAString& sdeAttrNameIR_F, 
													   const LAString& sdeAttrNameFX, 
													   LAPriceDriftHW* pDriftIR,
													   const double fx_criteria)
: LAPriceDriftHWQuantAdjustment(sdeAttrNameIR_D, sdeAttrNameIR_F, sdeAttrNameFX, pDriftIR, fx_criteria)
{
	if (pDriftIR == 0)
	{
		//error
		throw LACoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}
}


/*!
	@brief copy constructor
	@param[in] v copy source
*/
LAPriceDriftHWQuantAdjustment3F::LAPriceDriftHWQuantAdjustment3F(const LAPriceDriftHWQuantAdjustment3F& v) 
: LAPriceDriftHWQuantAdjustment(v)
{

}

/*!
	@brief destructor
*/
LAPriceDriftHWQuantAdjustment3F::~LAPriceDriftHWQuantAdjustment3F() 
{
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftHWQuantAdjustment3F::clone() const	
{
    try 
	{
		return new LAPriceDriftHWQuantAdjustment3F(*this);
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
LAPriceDriftHWQuantAdjustment3F::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTHWQUANTADJ3F ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftHWQuantAdjustment3F::getType() const
{
	return FN_DRIFTHWQUANTADJ3F;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = ts, x[1] = te, x[2] = rf
    @return drift value

*/
double
LAPriceDriftHWQuantAdjustment3F::operator()(const DoubleArray& x) const
{	
	return (*mpDriftIR)(x);
}



/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftHWQuantAdjustment3F::setUp(LAMathPathEntity& path)
{
	LAPriceDriftHWQuantAdjustment::setUp(path);
	//foreign numeraire
	LADataHolder* dh = &path.getData(mSDEAttrNameIR_F, ISNOTNULL);
	LAMathAttrSDE &attrsde = dynamic_cast<LAMathAttrSDE &>(dh->get());
	if (!attrsde.getSDE().isTypeOf(FN_SPOTSDEQUANTADJUSTMENT))
	{
		throw LACoreInvalidData("Foregin sde must be LARatesSpotSDEQuantAdjustment", __FILE__, __LINE__);
	}
	LAPriceQuantAdjustmentFuncBase &qa = dynamic_cast<LARatesSpotSDEQuantAdjustment &>(attrsde.getSDE()).getQuantAdjuster();

	if (!qa.isTypeOf(FN_QUANTADJUSTMENTFXDD))
	{
		throw LACoreInvalidData("QuantAdjustmentFunc must be LAPriceQuantAdjustmentHWFXDD", __FILE__, __LINE__);
	}
	mpQuantAdjuster = &dynamic_cast<LAPriceQuantAdjustmentHWFXDD &>(qa);
	
}

/*!
	@brief get quanto adjuster
*/
const LAPriceQuantAdjustmentHWFXDD &
LAPriceDriftHWQuantAdjustment3F::getQuantAdjuster() const
{
	if (!mpQuantAdjuster)
	{
		throw LACoreInvalidData("mpQuantAdjuster is NULL", __FILE__, __LINE__);
	}
	return *mpQuantAdjuster;
}


