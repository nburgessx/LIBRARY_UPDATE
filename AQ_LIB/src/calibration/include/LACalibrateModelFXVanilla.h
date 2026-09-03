/*! @file
    @brief FXVanilla sde generator class
*/
//  2007, Mizuho International London.
#ifndef LACalibrateModelFXVanilla_h
#define LACalibrateModelFXVanilla_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelFXVanilla.h
//
//  DESCRIPTION :       FXVanilla SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LACalibrateModelFX.h"


class LADataInstance;
class LAMathVolatility;

//===================== Class Declare LACalibrateModelFXStrangleSolver==================================
/*! 
    @brief FXVanilla SDE generator 
	

*/
class LACalibrateModelFXVanilla : public LACalibrateModelFX
{

public:
	// constructor
	explicit LACalibrateModelFXVanilla();
	// destructor
	virtual ~LACalibrateModelFXVanilla(void);
	// copy constructor
	LACalibrateModelFXVanilla(const LACalibrateModelFXVanilla &rhs);

	LACalibrateModelFXVanilla &operator=(const LACalibrateModelFXVanilla &rhs);
protected:
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const LAString &fx) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const LAString &fx) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const LAString &fx, LARatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	virtual  void setOutputTemplate(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &fx, LARatesSDEBase &sde) const;
	//==============================================
	// get function master regist name 
	virtual LAString getFunctionMasterResistName(const LAString &fx) const;

	//==============================================
	// get vol type
	virtual LAString getVolType(const LAString &fx) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const;

};
#endif
