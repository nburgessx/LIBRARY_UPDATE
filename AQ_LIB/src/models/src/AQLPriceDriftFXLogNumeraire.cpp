/*! @file
    @brief Source code of drift function of fx sde
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftFXLogNumeraire.h"

#include "AQLMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLMathAttrSDE.h"
#include "AQLRatesSDEBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLBasic.h"
#include "AQLPriceDriftHWQuantAdjustment3F.h"
#include "AQLPriceQuantAdjustmentHWFXDD.h"

using namespace std;

//================ AQLPriceDriftFXLogNumeraire ===================================
/*!
	@brief default constructor
	@param[in] s displaced diffusion parameter
*/
AQLPriceDriftFXLogNumeraire::AQLPriceDriftFXLogNumeraire(const double s, SDE_TYPE type)
: AQLPriceDriftFX(s, type)
{

}
/*!
	@brief constructor
	@param[in] sdeAttrNameD data name of domestic ir model
	@param[in] sdeAttrNameD data name of foreign ir model
	@param[in] s displaced diffusion parameter
*/
AQLPriceDriftFXLogNumeraire::AQLPriceDriftFXLogNumeraire(const AQLString& sdeAttrNameD, const AQLString& sdeAttrNameF, const double s, SDE_TYPE type)
: AQLPriceDriftFX(sdeAttrNameD,  sdeAttrNameF, s, type)
{

}


/*!
	@brief copy constructor
*/
/*AQLPriceDriftFXLogNumeraire::AQLPriceDriftFXLogNumeraire(const AQLPriceDriftFXLogNumeraire& v) 
: AQLCoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLPriceDriftFXLogNumeraire::~AQLPriceDriftFXLogNumeraire() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftFXLogNumeraire::clone() const	
{
    try 
	{
		return new AQLPriceDriftFXLogNumeraire(*this);
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
AQLPriceDriftFXLogNumeraire::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTFXLOGNUMERARIE ? true : AQLPriceDriftFX::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftFXLogNumeraire::getType() const
{
	return FN_DRIFTFXLOGNUMERARIE;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] must be time
    @return drift value

*/
double
AQLPriceDriftFXLogNumeraire::operator()(const DoubleArray& x) const
{

	const AQLRatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();
	const AQLRatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

	double te = x[2]; double ts = x[0];
    double numeRatioD = mpNumeraireD->operator ()(te) / mpNumeraireD->operator ()(ts);
    double numeRatioF = mpNumeraireF->operator ()(te) / mpNumeraireF->operator ()(ts);

	double ret = AQLMath::log(numeRatioD / numeRatioF) / (te - ts);

	return ret;
}




/*!
	@brief set up this class
	@param[in] path path object 
*/
void
AQLPriceDriftFXLogNumeraire::setUp(AQLMathPathEntity& path)
{
	AQLDataHolder* dh = &path.getData(mSDEAttrNameD, ISNOTNULL);
	AQLMathAttrSDE* pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpNumeraireD = pattrsde->getSDE().AQLRatesSDEBase::getNumeraire();
	const AQLRatesPathElementCurve* pInitialCurveD = dynamic_cast<const AQLRatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());

	dh = &path.getData(mSDEAttrNameF, ISNOTNULL);
	pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpNumeraireF = pattrsde->getSDE().AQLRatesSDEBase::getNumeraire();
	const AQLRatesPathElementCurve* pInitialCurveF = dynamic_cast<const AQLRatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());
	vector<AQLFunctionBase *> driftVec = pattrsde->getSDE().getDrift();
	if (driftVec.empty() || !driftVec[0]->isTypeOf(FN_DRIFTHWQUANTADJ3F))
	{
		throw AQLCoreInvalidData("FX drift class is wrong.", __FILE__, __LINE__);
	}
	mpDriftForeign = dynamic_cast<const AQLPriceDriftHWQuantAdjustment3F *>(driftVec[0]);
	mpQuantAdjuster = &(dynamic_cast<const AQLPriceQuantAdjustmentHWFXDD &>(mpDriftForeign->getQuantAdjuster()));
}

/*!
	@brief get quanto adjuster
*/
const AQLPriceQuantAdjustmentHWFXDD &
AQLPriceDriftFXLogNumeraire::getQuantAdjuster() const
{
	if (!mpQuantAdjuster)
	{
		throw AQLCoreInvalidData("mpQuantAdjuster is NULL", __FILE__, __LINE__);
	}
	return *mpQuantAdjuster;
}