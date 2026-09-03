/*! @file
    @brief Source code of drift function of fx sde



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftFXLogNumeraire.cpp
//
//  SYNOPSIS    :       LAPriceDriftFXLogNumeraire
//  DESCRIPTION :       Source code of drift function of fx sde
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


#include "LAPriceDriftFXLogNumeraire.h"

#include "LAMathPathEntity.h"
#include "LADataHolder.h"
#include "LADataVector.h"
#include "LAMathAttrSDE.h"
#include "LARatesSDEBase.h"
#include "LAModelDynamicsCurve.h"
#include "LABasic.h"
#include "LAPriceDriftHWQuantAdjustment3F.h"
#include "LAPriceQuantAdjustmentHWFXDD.h"

using namespace std;

//================ LAPriceDriftFXLogNumeraire ===================================
/*!
	@brief default constructor
	@param[in] s displaced diffusion parameter
*/
LAPriceDriftFXLogNumeraire::LAPriceDriftFXLogNumeraire(const double s, SDE_TYPE type)
: LAPriceDriftFX(s, type)
{

}
/*!
	@brief constructor
	@param[in] sdeAttrNameD data name of domestic ir model
	@param[in] sdeAttrNameD data name of foreign ir model
	@param[in] s displaced diffusion parameter
*/
LAPriceDriftFXLogNumeraire::LAPriceDriftFXLogNumeraire(const LAString& sdeAttrNameD, const LAString& sdeAttrNameF, const double s, SDE_TYPE type)
: LAPriceDriftFX(sdeAttrNameD,  sdeAttrNameF, s, type)
{

}


/*!
	@brief copy constructor
*/
/*LAPriceDriftFXLogNumeraire::LAPriceDriftFXLogNumeraire(const LAPriceDriftFXLogNumeraire& v) 
: LACoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
LAPriceDriftFXLogNumeraire::~LAPriceDriftFXLogNumeraire() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftFXLogNumeraire::clone() const	
{
    try 
	{
		return new LAPriceDriftFXLogNumeraire(*this);
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
LAPriceDriftFXLogNumeraire::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTFXLOGNUMERARIE ? true : LAPriceDriftFX::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftFXLogNumeraire::getType() const
{
	return FN_DRIFTFXLOGNUMERARIE;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] must be time
    @return drift value

*/
double
LAPriceDriftFXLogNumeraire::operator()(const DoubleArray& x) const
{

	const LARatesPathElementCurve* pInitialCurveD = &mpNumeraireD->getInitialCurve();
	const LARatesPathElementCurve* pInitialCurveF = &mpNumeraireF->getInitialCurve();

	double te = x[2]; double ts = x[0];
    double numeRatioD = mpNumeraireD->operator ()(te) / mpNumeraireD->operator ()(ts);
    double numeRatioF = mpNumeraireF->operator ()(te) / mpNumeraireF->operator ()(ts);

	double ret = LAMath::log(numeRatioD / numeRatioF) / (te - ts);

	return ret;
}




/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftFXLogNumeraire::setUp(LAMathPathEntity& path)
{
	LADataHolder* dh = &path.getData(mSDEAttrNameD, ISNOTNULL);
	LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mpNumeraireD = pattrsde->getSDE().LARatesSDEBase::getNumeraire();
	const LARatesPathElementCurve* pInitialCurveD = dynamic_cast<const LARatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());

	dh = &path.getData(mSDEAttrNameF, ISNOTNULL);
	pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mpNumeraireF = pattrsde->getSDE().LARatesSDEBase::getNumeraire();
	const LARatesPathElementCurve* pInitialCurveF = dynamic_cast<const LARatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());
	vector<LAFunctionBase *> driftVec = pattrsde->getSDE().getDrift();
	if (driftVec.empty() || !driftVec[0]->isTypeOf(FN_DRIFTHWQUANTADJ3F))
	{
		throw LACoreInvalidData("FX drift class is wrong.", __FILE__, __LINE__);
	}
	mpDriftForeign = dynamic_cast<const LAPriceDriftHWQuantAdjustment3F *>(driftVec[0]);
	mpQuantAdjuster = &(dynamic_cast<const LAPriceQuantAdjustmentHWFXDD &>(mpDriftForeign->getQuantAdjuster()));
}

/*!
	@brief get quanto adjuster
*/
const LAPriceQuantAdjustmentHWFXDD &
LAPriceDriftFXLogNumeraire::getQuantAdjuster() const
{
	if (!mpQuantAdjuster)
	{
		throw LACoreInvalidData("mpQuantAdjuster is NULL", __FILE__, __LINE__);
	}
	return *mpQuantAdjuster;
}