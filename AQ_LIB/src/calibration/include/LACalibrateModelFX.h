#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACalibrateModel.h"


class LADataInstance;
class LAString;
class LAStaticData;


//===================== Class Declare LACalibrateModelFX==================================
/*! 
    @brief FX SDE generator 
	
	this class is abstract

*/
class LACalibrateModelFX  : public LACalibrateModel
{
public:
	// constructor
	LACalibrateModelFX(void);
	// destructor
	virtual ~LACalibrateModelFX(void);
	// copy constructor
	LACalibrateModelFX(const LACalibrateModelFX &rhs);
	LACalibrateModelFX &operator=(const LACalibrateModelFX &rhs);
	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const LAString &fx, LADataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const LAString & curveID = "", const LAString & marketName = "") const;
	//==============================================
	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const {}

protected:
	// set path 
	virtual  void setOutputTemplate(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &fx, LARatesSDEBase &sde) const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const LAString &fx, LARatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const;
	//==============================================
	// get SDE data name
	virtual LAString getSDEAttrName(const LAString &fx) const;

};
