/*! @file
    @brief FXVanilla sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef AQLCalibrateModelFXVanilla_h
#define AQLCalibrateModelFXVanilla_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelFXVanilla.h
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
#include "AQLCalibrateModelFX.h"


class AQLDataInstance;
class AQLMathVolatility;

//===================== Class Declare AQLCalibrateModelFXStrangleSolver==================================
/*! 
    @brief FXVanilla SDE generator 
	

*/
class AQLCalibrateModelFXVanilla : public AQLCalibrateModelFX
{

public:
	// constructor
	explicit AQLCalibrateModelFXVanilla();
	// destructor
	virtual ~AQLCalibrateModelFXVanilla(void);
	// copy constructor
	AQLCalibrateModelFXVanilla(const AQLCalibrateModelFXVanilla &rhs);

	AQLCalibrateModelFXVanilla &operator=(const AQLCalibrateModelFXVanilla &rhs);
protected:
	// create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &fx) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &fx) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &fx, AQLRatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	virtual  void setOutputTemplate(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &fx, AQLRatesSDEBase &sde) const;
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &fx) const;

	//==============================================
	// get vol type
	virtual AQLString getVolType(const AQLString &fx) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const;

};
#endif
