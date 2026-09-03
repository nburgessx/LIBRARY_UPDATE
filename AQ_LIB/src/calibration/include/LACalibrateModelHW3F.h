/*! @file
    @brief Hull-White sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelHW3F_h
#define LACalibrateModelHW3F_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelHW3F.h
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
#include "LACalibrateModelIR.h"
#include "LACalibrateModelHW.h"



class AQLDataInstance;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class LAMathVolatility;
class LAPriceDriftHWQuantAdjustment;


//===================== Class Declare LACalibrateModelHW3F==================================
/*! 
    @brief Hull-White SDE generator 
	

*/
class LACalibrateModelHW3F : public LACalibrateModelHW
{

public:
	// constructor
	explicit LACalibrateModelHW3F(const AQLString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelHW3F(void);
	// copy constructor
	LACalibrateModelHW3F(const LACalibrateModelHW3F &rhs);
	LACalibrateModelHW3F &operator=(const LACalibrateModelHW3F &rhs);
protected:
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &currency, LARatesSDEBase &sde)  const;
	//==============================================
	// create forein drift
	virtual AQLFunctionBase *createForeinDrift(const AQLString &fx, const AQLString &sdeBase, const AQLString &sdeName, const AQLString &fx_sdeName) const;

private:

};
#endif
