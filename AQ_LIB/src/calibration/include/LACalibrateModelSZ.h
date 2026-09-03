#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "LACalibrateModelFX.h"
#include "LACalibrateModelVolFactor.h"

class AQLDataInstance;
class AQLMathVolatility;
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
		virtual  SDE_TYPE getSDEType(const AQLString &currency) const;
		//==============================================
		// check LJ
		virtual bool isLJ(const AQLString &currency) const;
		//==============================================
		// set volatility 
		virtual  void setVolatility(const AQLString &currency, AQLRatesSDEBase &sde)  const;	
		//==============================================
		// set drift 
		virtual  void setDrift(const AQLString &currency, AQLRatesSDEBase &sde)  const;
		//==============================================
		// set integral function 
		virtual void setIntegralFunction(const AQLString &currency, AQLRatesSDEBase &sde) const;
		//==============================================
		// get function master regist name 
		virtual AQLString getFunctionMasterResistName(const AQLString &currency) const;
		//==============================================
		// get vol type
		virtual AQLString getVolType(const AQLString &currency) const;
		//==============================================
		// setup vol method
		virtual void setUpVolFunc(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;
		//==============================================
		// setup vol data (not supported)
		virtual void setUpVolData(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;

	};

protected:
	//==============================================
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
	//==============================================
	// create calibration info
	virtual LACalibrationParametersSZ *createCalibInfoCreator(void) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const;

};
