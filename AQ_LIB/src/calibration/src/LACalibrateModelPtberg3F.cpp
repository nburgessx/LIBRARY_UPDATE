/*! @file
    @brief Ptberg sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelPtberg3F.cpp
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


#include "LACalibrateModelPtberg3F.h"
#include "LACalibrationParametersPtberg3F.h"
#include "LAPriceDriftFXLogNumeraire.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LAPtbergUtils.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelPtberg3F::LACalibrateModelPtberg3F()
: LACalibrateModelPtberg()
{
}

// destructor
/*!

*/
LACalibrateModelPtberg3F::~LACalibrateModelPtberg3F(void)
{
}


/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelPtberg3F::setDrift(const LAString &fx, LARatesSDEBase &sde) const
{
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	LAString sdeName_d = mpStaticData->getStaticData(ccys[0].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	LAString sdeName_f = mpStaticData->getStaticData(ccys[1].toLower() + STATIC_DATA_FX_KEY_SDE_NAME);
	
	vector<LAFunctionBase*> drift(1,  new LAPriceDriftFXLogNumeraire(sdeName_d, sdeName_f));
	sde.setDrift(drift);
}

// 
/*!
    @brief  create calibinfocreator

*/
LACalibrationParametersPtberg*
LACalibrateModelPtberg3F::createCalibInfoCreator(void) const
{
	const bool is3F = MAPtbergUtils::is3FCalib();
	if (is3F)
	{
		return new LACalibrationParametersPtberg3F;
	}
	else
	{
		return new LACalibrationParametersPtberg;
	}
}

/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelPtberg3F::getSDEType(const LAString &fx) const
{
	return dX;
}