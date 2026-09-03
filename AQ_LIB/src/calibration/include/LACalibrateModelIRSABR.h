/*! @file
    @brief IRSABR sde generator class
*/
//  2007, Mizuho International London.
#ifndef LACalibrateModelIRSABR_h
#define LACalibrateModelIRSABR_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelIRSABR.h
//
//  DESCRIPTION :       IRSABR SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LACalibrateModelIR.h"


class LADataInstance;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class LAStaticData;

//===================== Class Declare LACalibrateModelIRSABR==================================
/*! 
    @brief IRSABR SDE generator 
	
	this class is abstract

*/
class LACalibrateModelIRSABR : public LACalibrateModelIR
{
public:
	// constructor
	explicit LACalibrateModelIRSABR(const LAString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelIRSABR(void);
	// copy constructor
	LACalibrateModelIRSABR(const LACalibrateModelIRSABR &rhs);
	LACalibrateModelIRSABR &operator=(const LACalibrateModelIRSABR &rhs);
	// generate sde correlation
	virtual void loadCorrelationDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const {return;};
	
	
protected:
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const LAString &key) const {SDE_TYPE a; return a;};
	//==============================================
	// check LJ
	virtual bool isLJ(const LAString &key) const {return false;};
	//==============================================
	// set volatility 
	virtual  void setVolatility(const LAString &key, LARatesSDEBase &sde) const ;	
	//==============================================
	// set drift 
	virtual  void setDrift(const LAString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const LAString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	// set path 
	virtual  void setOutputTemplate(const LAString &key, LARatesSDEBase &sde)  const  {return;};
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const LAString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &key, LARatesSDEBase &sde) const {return;};
	//==============================================
	// get function master regist name 
	virtual LAString getFunctionMasterResistName(const LAString &key) const {LAString ret; return ret;};
	//==============================================
	// get create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &key, LADataInstance &dataInstance) const {return 0;};
	//==============================================
	// setup vol type
	virtual LAString getVolType(const LAString &key) const ;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const LAString &key, LAMathVolatility &vol, LADataInstance &dataInstance) const {return;};
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const LAString &key,LAMathVolatility &vol, LADataInstance &dataInstance) const ;
	//==============================================
	// setup correlation input type
	virtual LAString getCorTye(const LAString &currency) const {LAString ret; return ret;};
	//==============================================
	// setup correlation factor
	virtual void setUpCorFactor(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const {return;};
	//==============================================
	// setup correlation data
	virtual void setUpCorData(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const {return;};
	//==============================================
	// setup correlation method
	virtual void setUpCorFunc(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const {return;};
	//==============================================
		
};
#endif
