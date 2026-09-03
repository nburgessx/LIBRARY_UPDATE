#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCalibrateModel.h"


class AQLDataInstance;
class AQLString;
class AQLStaticData;


//===================== Class Declare AQLCalibrateModelFX==================================
/*! 
    @brief FX SDE generator 
	
	this class is abstract

*/
class AQLCalibrateModelFX  : public AQLCalibrateModel
{
public:
	// constructor
	AQLCalibrateModelFX(void);
	// destructor
	virtual ~AQLCalibrateModelFX(void);
	// copy constructor
	AQLCalibrateModelFX(const AQLCalibrateModelFX &rhs);
	AQLCalibrateModelFX &operator=(const AQLCalibrateModelFX &rhs);
	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const AQLString &fx, AQLDataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const AQLString & curveID = "", const AQLString & marketName = "") const;
	//==============================================
	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const {}

protected:
	// set path 
	virtual  void setOutputTemplate(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &fx, AQLRatesSDEBase &sde) const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &fx, AQLRatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// get SDE data name
	virtual AQLString getSDEAttrName(const AQLString &fx) const;

};
