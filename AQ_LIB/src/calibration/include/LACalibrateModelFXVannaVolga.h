/*! @file
    @brief FXVanilla sde generator class
*/
//  2011, AlgoQuantHub.
#ifndef LACalibrateModelFXVannaVolga_h
#define LACalibrateModelFXVannaVolga_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelFXVannaVolga.h
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

//===================== Class Declare LACalibrateModelFXVannaVolga==================================
/*! 
    @brief FXVanilla SDE generator 
	

*/
class LACalibrateModelFXVannaVolga : public LACalibrateModelFX
{

public:
	// constructor
	explicit LACalibrateModelFXVannaVolga();
	// destructor
	virtual ~LACalibrateModelFXVannaVolga(void);
	// copy constructor
	LACalibrateModelFXVannaVolga(const LACalibrateModelFXVannaVolga &rhs);
	LACalibrateModelFXVannaVolga &operator=(const LACalibrateModelFXVannaVolga &rhs);
protected:
	friend class MAPtbergUtils;
	//==============================================
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
