#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LAUpdateObjectPoolForCurves.h"


class LADataInstance;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class LAStaticData;

//===================== Class Declare UpdateObjectPoolForSDEsAndCurves==================================
/*! 
    @brief IRVanilla SDE generator 
	
	this class is abstract

*/
class UpdateObjectPoolForSDEsAndCurves : public LAUpdateObjectPoolForCurves
{
public:
	//! constructor
	explicit UpdateObjectPoolForSDEsAndCurves(const LAString &baseCurrency);
	//! destructor
	virtual ~UpdateObjectPoolForSDEsAndCurves(void);
	//! copy constructor
	UpdateObjectPoolForSDEsAndCurves(const UpdateObjectPoolForSDEsAndCurves &rhs);
	UpdateObjectPoolForSDEsAndCurves &operator=(const UpdateObjectPoolForSDEsAndCurves &rhs);
	//! generate sde correlation
	virtual void generateCorrelation(const LAString &currency, LADataInstance &dataInstance) const {return;};
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	//! generate sde volatility
	virtual void generateVolatility(const LAString &key, LADataInstance &dataInstance) const {return;};
#endif
	
protected:
	//==============================================
	//! get sde Type
	
    // TODO: Fix me, originally this function returned an undefined variable. Have set this temporarily to return a valid type to resolve compiler issues
    //virtual  SDE_TYPE getSDEType(const LAString &key) const {SDE_TYPE a; return a;};
    virtual SDE_TYPE getSDEType(const LAString &key) const { SDE_TYPE a = dX; return a; }

	//==============================================
	//! check LJ
	virtual bool isLJ(const LAString &key) const {return false;};
	//==============================================
	//! set volatility 
	virtual  void setVolatility(const LAString &key, LARatesSDEBase &sde)  const {return;};	
	//==============================================
	//! set drift 
	virtual  void setDrift(const LAString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	//! set numeraire 
	virtual  void setNumeraire(const LAString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	//! set path 
	virtual  void setOutputTemplate(const LAString &key, LARatesSDEBase &sde)  const  {return;};
	//==============================================
	//! set integralfunction 
	virtual  void setIntegralFunction(const LAString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	//! set interpolation method 
	virtual  void setInterpolationMethod(const LAString &key, LARatesSDEBase &sde) const {return;};
	//==============================================
	//! get function master regist name 
	virtual LAString getFunctionMasterResistName(const LAString &key) const {LAString ret; return ret;};
	//==============================================
	//! get create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &key, LADataInstance &dataInstance) const {return 0;};
	//==============================================
	//! setup vol type
	virtual LAString getVolType(const LAString &key) const {LAString ret; return ret;};
	//==============================================
	//! setup vol data
	virtual void setUpVolData(const LAString &key, LAMathVolatility &vol, LADataInstance &dataInstance) const {return;};
	//==============================================
	//! setup vol method
	virtual void setUpVolFunc(const LAString &key,LAMathVolatility &vol, LADataInstance &dataInstance) const {return;};
	//==============================================
	//! setup correlation input type
	virtual LAString getCorTye(const LAString &currency) const {LAString ret; return ret;};
	//==============================================
	//! setup correlation factor
	virtual void setUpCorFactor(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const {return;};
	//==============================================
	//! setup correlation data
	virtual void setUpCorData(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const {return;};
	//==============================================
	//! setup correlation method
	virtual void setUpCorFunc(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const {return;};
	//==============================================
		
};
