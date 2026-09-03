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


class AQLDataInstance;
class AQLString;
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
	virtual void loadModelDataAndCalibrate(const AQLString &vola, AQLDataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const AQLString & curveID = "", const AQLString & marketName = "") const;
	//==============================================
	// generate sde initial value for fwdvola constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const {}

protected:
	// set path 
	virtual  void setOutputTemplate(const AQLString &vola, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &vola, LARatesSDEBase &sde) const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &vola, LARatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const AQLString &vola, AQLDataInstance &dataInstance) const;
	//==============================================
	// get SDE data name
	virtual AQLString getSDEAttrName(const AQLString &vola) const;
	// check cancel target for funding change 
	////==============================================
	//// check cancel target for funding change 
	//virtual  bool isCancelForFunding(const AQLString &ccy) const;
	////==============================================
	// get Spot SDE data name
	AQLString getSpotIndex(const AQLString &vola) const;


};
#endif
