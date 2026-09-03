#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LACalibrateModelFX.h"
#include "LACalibrateModelVolFactor.h"

class LADataInstance;
class LAMathVolatility;
class LACalibrationParametersSZ;

// Schobel-Zhu SDE generator 
class LACalibrateModelSZ : public LACalibrateModelFX
{

public:
	// constructor
	explicit LACalibrateModelSZ();
	// destructor
	virtual ~LACalibrateModelSZ(void);
	// copy constructor
	LACalibrateModelSZ(const LACalibrateModelSZ &rhs);
	LACalibrateModelSZ &operator=(const LACalibrateModelSZ &rhs);

	//==============================================
	// sde generator class for sde of volatility
	class VF : public LACalibrateModelVolFactor
	{

	public:
		// constructor
		explicit VF(void);
		// destructor
		virtual ~VF(void);
		// copy constructor
		VF(const VF &rhs);
		VF &operator=(const VF &rhs);
	protected:
		//==============================================
		// get sde Type
		virtual  SDE_TYPE getSDEType(const LAString &currency) const;
		//==============================================
		// check LJ
		virtual bool isLJ(const LAString &currency) const;
		//==============================================
		// set volatility 
		virtual  void setVolatility(const LAString &currency, LARatesSDEBase &sde)  const;	
		//==============================================
		// set drift 
		virtual  void setDrift(const LAString &currency, LARatesSDEBase &sde)  const;
		//==============================================
		// set integral function 
		virtual void setIntegralFunction(const LAString &currency, LARatesSDEBase &sde) const;
		//==============================================
		// get function master regist name 
		virtual LAString getFunctionMasterResistName(const LAString &currency) const;
		//==============================================
		// get vol type
		virtual LAString getVolType(const LAString &currency) const;
		//==============================================
		// setup vol method
		virtual void setUpVolFunc(const LAString &currency, LAMathVolatility &vol, LADataInstance &dataInstance) const;
		//==============================================
		// setup vol data (not supported)
		virtual void setUpVolData(const LAString &currency, LAMathVolatility &vol, LADataInstance &dataInstance) const;

	};

protected:
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const LAString &fx) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const LAString &fx) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const LAString &fx, LARatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// get function master regist name 
	virtual LAString getFunctionMasterResistName(const LAString &fx) const;

	//==============================================
	// get vol type
	virtual LAString getVolType(const LAString &fx) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const;
	//==============================================
	// create calibration info
	virtual LACalibrationParametersSZ *createCalibInfoCreator(void) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const;

};
