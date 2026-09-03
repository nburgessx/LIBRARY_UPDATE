/*! @file
    @brief IRSABR sde generator class
*/
//  2007, AlgoQuantHub.
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

#include "AQLString.h"
#include "LACalibrateModelIR.h"


class AQLDataInstance;
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
	explicit LACalibrateModelIRSABR(const AQLString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelIRSABR(void);
	// copy constructor
	LACalibrateModelIRSABR(const LACalibrateModelIRSABR &rhs);
	LACalibrateModelIRSABR &operator=(const LACalibrateModelIRSABR &rhs);
	// generate sde correlation
	virtual void loadCorrelationDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const {return;};
	
	
protected:
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &key) const {SDE_TYPE a; return a;};
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &key) const {return false;};
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &key, LARatesSDEBase &sde) const ;	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	// set path 
	virtual  void setOutputTemplate(const AQLString &key, LARatesSDEBase &sde)  const  {return;};
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &key, LARatesSDEBase &sde)  const {return;};
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &key, LARatesSDEBase &sde) const {return;};
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &key) const {AQLString ret; return ret;};
	//==============================================
	// get create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const AQLString &key, AQLDataInstance &dataInstance) const {return 0;};
	//==============================================
	// setup vol type
	virtual AQLString getVolType(const AQLString &key) const ;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &key, LAMathVolatility &vol, AQLDataInstance &dataInstance) const {return;};
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &key,LAMathVolatility &vol, AQLDataInstance &dataInstance) const ;
	//==============================================
	// setup correlation input type
	virtual AQLString getCorTye(const AQLString &currency) const {AQLString ret; return ret;};
	//==============================================
	// setup correlation factor
	virtual void setUpCorFactor(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const {return;};
	//==============================================
	// setup correlation data
	virtual void setUpCorData(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const {return;};
	//==============================================
	// setup correlation method
	virtual void setUpCorFunc(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const {return;};
	//==============================================
		
};
#endif
