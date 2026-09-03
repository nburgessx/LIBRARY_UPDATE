// $Id: LACalibrateModelVolFactor.h,v 1.0 2016/10/11 12:03:18 fukuitak Exp $
/*! @file
    @brief Volatility factor SDE generator class
*/
//  2016, AlgoQuantHub.
#ifndef LACalibrateModelVolFactor_h
#define LACalibrateModelVolFactor_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelvolaaFactor.h
//
//  DESCRIPTION :       Volatility factor SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACalibrateModel.h"


class LADataInstance;
class LAString;
class LAStaticData;


//===================== Class Declare LACalibrateModelVolFactor==================================
/*! 
    @brief Volatility factor SDE generator 
	
	this class is abstract

*/
class LACalibrateModelVolFactor  : public LACalibrateModel
{
public:
	// constructor
	LACalibrateModelVolFactor(void);
	// destructor
	virtual ~LACalibrateModelVolFactor(void);
	// copy constructor
	LACalibrateModelVolFactor(const LACalibrateModelVolFactor &rhs);
	LACalibrateModelVolFactor &operator=(const LACalibrateModelVolFactor &rhs);
	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const LAString &vola, LADataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const LAString & curveID = "", const LAString & marketName = "") const;
	//==============================================
	// generate sde initial value for fwdvola constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const {}

protected:
	// set path 
	virtual  void setOutputTemplate(const LAString &vola, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &vola, LARatesSDEBase &sde) const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const LAString &vola, LARatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &vola, LADataInstance &dataInstance) const;
	//==============================================
	// get SDE data name
	virtual LAString getSDEAttrName(const LAString &vola) const;
	// check cancel target for funding change 
	////==============================================
	//// check cancel target for funding change 
	//virtual  bool isCancelForFunding(const LAString &ccy) const;
	////==============================================
	// get Spot SDE data name
	LAString getSpotIndex(const LAString &vola) const;


};
#endif
