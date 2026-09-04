/*! @file
    @brief Ptberg SDE generator class. The SDE includes skew
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateModelPtbergSkew.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrateModelPtbergSkew::AQLCalibrateModelPtbergSkew()
: AQLCalibrateModelPtberg()
{
}

// destructor
/*!

*/
AQLCalibrateModelPtbergSkew::~AQLCalibrateModelPtbergSkew(void)
{
}


/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
AQLCalibrateModelPtbergSkew::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	// TO DO
	AQLCalibrateModelPtberg::setDrift(fx, sde);
}



/*!
	@brief set integral function

	@param[in] fx
	@param[out] sde

*/
void
AQLCalibrateModelPtbergSkew::setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	// TO DO
	AQLCalibrateModelPtberg::setIntegralFunction(fx, sde);
}


// 
/*!
    @brief set volatility function

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
AQLCalibrateModelPtbergSkew::setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	// TO DO
	AQLCalibrateModelPtberg::setUpVolFunc(fx, vol, dataInstance);
}