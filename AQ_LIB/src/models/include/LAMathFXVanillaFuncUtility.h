#ifndef LAMathFXVanillaUtility_h
#define LAMathFXVanillaUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"
#include "LAMathYieldCurve.h"
#include "ConstantDeclarations.h"
#include "LATime.h"
//////////////////////////





//date method
class LAMathFXVanillaFuncUtility
{
public:
	static double gkOption(	LAString& optiontype, LAString& buysell, LAString& callput,
							double spot, double strike, double vol, 
							double localrate, double foreignrate, const LADate& basedate, 
							const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate);
	static double gkOptionIV(LAString& buysell, LAString& callput,
							double spot, double strike, double prem, 
							double localrate, double foreignrate, const LADate& basedate, 
							const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate,
							double low, double high);
	//digitaloption
	static double digitalOption(LAString& optiontype, LAString& buysell, LAString& callput,
								double spot, double strike, double vol, 
								double localrate, double foreignrate, const LADate& basedate, 
								const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate);
	//single barrier option
	static double singleBarrierOption(LAString& optiontype, LAString& buysell, LAString& callput,
									LAString& downup, LAString& inout, double rebate, double limit,
									double spot, double strike, double vol, 
									double localrate, double foreignrate, const LADate& basedate, 
									const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate);
	//single barrier option easily type
	static double singleBarrierOptionEasily(LAString& optiontype, LAString& buysell, LAString& callput,
											LAString& downup, LAString& inout, double rebate, double limit,
											double spot, double strike, double vol, 
											double localrate, double foreignrate,
											double expiryterms, double deliveryterms, int rebatetime);
	//double barrier option easily type
	static double doubleBarrierOption(LAString& optiontype, LAString& buysell, LAString& callput, LAString& inout,
									double limitlow, double limithigh, double rebate, LAString& rebatetype,  
									double spot, double strike, double vol, 
									double localrate, double foreignrate, const LADate& basedate, 
									const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate,
									int num = 100);
	//forward delta 
	static double calcstrikefromdelta(double target, LAString& spotfwd,  LAString& callput, double spotfx, double fwdfx,
							double atmvol, double reversal, double strangle, double foreignrate,
								  LADate& basedate, LADate& spotdate, LADate& expirydate, LADate& deliverydate);

	static double calcstrikefromdelta(double target, LAString& spotfwd,  LAString& callput, double spotfx, double fwdfx,
							double atmvol, double reversal, double strangle, double foreignrate,
								  LADate& basedate, LADate& spotdate, LADate& expirydate, LADate& deliverydate,
								  double lower, double upper);
	static double calcstrikefromdelta(double target, LAString& spotfwd, LAString& callput, double spotfx, 
											  double fwdfx, double atmvol, double reversal, double strangle,
											  double foreigndf, double expiryterm);

	static double calcstrikefromdelta(double target, LAString& spotfwd, LAString& callput, double spotfx, 
											  double fwdfx, double atmvol, double reversal, double strangle,
											  double foreigndf, double expiryterm, double lower, double upper);
    
    static double calcmaxstrike(LAString& spotfwd, const double spotfx, 
											  const double fwdfx, const double atmvol, const double reversal, const double strangle,
											  const double foreigndf, const double expiryterm, double lower = 0.01, double upper = -10.0);

    static double calcmaxfxdelta(LAString& spotfwd, const double spotfx, 
											  const double fwdfx, const double atmvol, const double reversal, const double strangle,
											  const double foreigndf, const double expiryterm, double lower = 0.01 , double upper = -10.0);

	static void setupfxvolobject(LADataInstance* dataInstance, LAString& interpid, LAStringMatrix& baseinfomat, const DoubleArray& termvec, 
										   const DoubleMatrix& strikemat, const DoubleMatrix& volmat , double spotfx = 100.);
	
	static double getvaluefromfxvolobject(LADataInstance* dataInstance, LAStringMatrix& baseinfomat, const double term,
												  const double strike);

	static double getspotfxfromfxvolobject(LADataInstance* dataInstance, LAStringMatrix& baseinfomat );

private:
	LAMathFXVanillaFuncUtility(void);
	~LAMathFXVanillaFuncUtility(void);
	LAMathFXVanillaFuncUtility(const LAMathFXVanillaFuncUtility &rhs);
	LAMathFXVanillaFuncUtility &operator=(const LAMathFXVanillaFuncUtility &rhs);
};
#endif 