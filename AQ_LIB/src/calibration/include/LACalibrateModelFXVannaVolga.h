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

#include "AQLString.h"
#include "LACalibrateModelFX.h"


class AQLDataInstance;
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
	virtual LARatesSDEBase *createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &fx) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &fx) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &fx, LARatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	virtual  void setOutputTemplate(const AQLString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &fx, LARatesSDEBase &sde) const;
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &fx) const;

	//==============================================
	// get vol type
	virtual AQLString getVolType(const AQLString &fx) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const AQLString &fx, LAMathVolatility &vol) const;

};
#endif
