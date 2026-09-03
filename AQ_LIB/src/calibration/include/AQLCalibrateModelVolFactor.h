// $Id: AQLCalibrateModelVolFactor.h,v 1.0 2016/10/11 12:03:18 fukuitak Exp $
/*! @file
    @brief Volatility factor SDE generator class
*/
//  2016, AlgoQuantHub.
#ifndef AQLCalibrateModelVolFactor_h
#define AQLCalibrateModelVolFactor_h
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

#include "AQLCalibrateModel.h"


class AQLDataInstance;
class AQLString;
class AQLStaticData;


//===================== Class Declare AQLCalibrateModelVolFactor==================================
/*! 
    @brief Volatility factor SDE generator 
	
	this class is abstract

*/
class AQLCalibrateModelVolFactor  : public AQLCalibrateModel
{
public:
	// constructor
	AQLCalibrateModelVolFactor(void);
	// destructor
	virtual ~AQLCalibrateModelVolFactor(void);
	// copy constructor
	AQLCalibrateModelVolFactor(const AQLCalibrateModelVolFactor &rhs);
	AQLCalibrateModelVolFactor &operator=(const AQLCalibrateModelVolFactor &rhs);
	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const AQLString &vola, AQLDataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const AQLString & curveID = "", const AQLString & marketName = "") const;
	//==============================================
	// generate sde initial value for fwdvola constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const {}

protected:
	// set path 
	virtual  void setOutputTemplate(const AQLString &vola, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &vola, AQLRatesSDEBase &sde) const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &vola, AQLRatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &vola, AQLDataInstance &dataInstance) const;
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
