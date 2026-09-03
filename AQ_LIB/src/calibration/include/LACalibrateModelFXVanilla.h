/*! @file
    @brief FXVanilla sde generator class
*/
//  2007, AlgoQuantHub.
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

#include "AQLString.h"
#include "LACalibrateModelFX.h"


class AQLDataInstance;
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
