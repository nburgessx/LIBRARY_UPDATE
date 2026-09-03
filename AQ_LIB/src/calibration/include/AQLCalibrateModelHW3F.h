/*! @file
    @brief Hull-White sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef AQLCalibrateModelHW3F_h
#define AQLCalibrateModelHW3F_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelHW3F.h
//
//  DESCRIPTION :       Hull-Whete SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLString.h"
#include "AQLCalibrateModelIR.h"
#include "AQLCalibrateModelHW.h"



class AQLDataInstance;
class AQLRatesCurveLogLinearInterpolation;
class AQLMathCorrelation;
class AQLMathVolatility;
class AQLPriceDriftHWQuantAdjustment;


//===================== Class Declare AQLCalibrateModelHW3F==================================
/*! 
    @brief Hull-White SDE generator 
	

*/
class AQLCalibrateModelHW3F : public AQLCalibrateModelHW
{

public:
	// constructor
	explicit AQLCalibrateModelHW3F(const AQLString &baseCurrency);
	// destructor
	virtual ~AQLCalibrateModelHW3F(void);
	// copy constructor
	AQLCalibrateModelHW3F(const AQLCalibrateModelHW3F &rhs);
	AQLCalibrateModelHW3F &operator=(const AQLCalibrateModelHW3F &rhs);
protected:
	//==============================================
	// create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &currency, AQLRatesSDEBase &sde)  const;
	//==============================================
	// create forein drift
	virtual AQLFunctionBase *createForeinDrift(const AQLString &fx, const AQLString &sdeBase, const AQLString &sdeName, const AQLString &fx_sdeName) const;

private:

};
#endif
