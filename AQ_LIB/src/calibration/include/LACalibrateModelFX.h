#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACalibrateModel.h"


class AQLDataInstance;
class AQLString;
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
	virtual void loadModelDataAndCalibrate(const AQLString &fx, AQLDataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const AQLString & curveID = "", const AQLString & marketName = "") const;
	//==============================================
	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const {}

protected:
	// set path 
	virtual  void setOutputTemplate(const AQLString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &fx, LARatesSDEBase &sde) const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &fx, LARatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// get SDE data name
	virtual AQLString getSDEAttrName(const AQLString &fx) const;

};
