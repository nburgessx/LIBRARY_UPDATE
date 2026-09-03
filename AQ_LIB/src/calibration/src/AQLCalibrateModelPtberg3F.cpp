/*! @file
    @brief Ptberg sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelPtberg3F.cpp
//
//  DESCRIPTION :        Ptberg SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateModelPtberg3F.h"
#include "AQLCalibrationParametersPtberg3F.h"
#include "AQLPriceDriftFXLogNumeraire.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLPtbergUtils.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrateModelPtberg3F::AQLCalibrateModelPtberg3F()
: AQLCalibrateModelPtberg()
{
}

// destructor
/*!

*/
AQLCalibrateModelPtberg3F::~AQLCalibrateModelPtberg3F(void)
{
}


/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
AQLCalibrateModelPtberg3F::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(fx, ccys);

	AQLString sdeName_d = mpStaticData->getStaticData(ccys[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	AQLString sdeName_f = mpStaticData->getStaticData(ccys[1].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	
	vector<AQLFunctionBase*> drift(1,  new AQLPriceDriftFXLogNumeraire(sdeName_d, sdeName_f));
	sde.setDrift(drift);
}

// 
/*!
    @brief  create calibinfocreator

*/
AQLCalibrationParametersPtberg*
AQLCalibrateModelPtberg3F::createCalibInfoCreator(void) const
{
	const bool is3F = AQLPtbergUtils::is3FCalib();
	if (is3F)
	{
		return new AQLCalibrationParametersPtberg3F;
	}
	else
	{
		return new AQLCalibrationParametersPtberg;
	}
}

/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
AQLCalibrateModelPtberg3F::getSDEType(const AQLString &fx) const
{
	return dX;
}