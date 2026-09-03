/*! @file
    @brief Hull-White sde generator class
*/
//  2007, Mizuho International London.
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

#include "LACoreTemplateType.h"
#include "LAString.h"
#include "LACalibrateModelIR.h"
#include "LACalibrateModelHW.h"



class LADataInstance;
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
	explicit LACalibrateModelHW3F(const LAString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelHW3F(void);
	// copy constructor
	LACalibrateModelHW3F(const LACalibrateModelHW3F &rhs);
	LACalibrateModelHW3F &operator=(const LACalibrateModelHW3F &rhs);
protected:
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const LAString &currency, LARatesSDEBase &sde)  const;
	//==============================================
	// create forein drift
	virtual LAFunctionBase *createForeinDrift(const LAString &fx, const LAString &sdeBase, const LAString &sdeName, const LAString &fx_sdeName) const;

private:

};
#endif
