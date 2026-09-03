#ifndef LAMathFXVanillaUtility_h
#define LAMathFXVanillaUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "LAMathYieldCurve.h"
#include "ConstantDeclarations.h"
#include "LATime.h"
//////////////////////////





//date method
class LAMathFXVanillaFuncUtility
{
public:
	static double gkOption(	AQLString& optiontype, AQLString& buysell, AQLString& callput,
							double spot, double strike, double vol, 
							double localrate, double foreignrate, const AQLDate& basedate, 
							const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate);
	static double gkOptionIV(AQLString& buysell, AQLString& callput,
							double spot, double strike, double prem, 
							double localrate, double foreignrate, const AQLDate& basedate, 
							const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate,
							double low, double high);
	//digitaloption
	static double digitalOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
								double spot, double strike, double vol, 
								double localrate, double foreignrate, const AQLDate& basedate, 
								const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate);
	//single barrier option
	static double singleBarrierOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
									AQLString& downup, AQLString& inout, double rebate, double limit,
									double spot, double strike, double vol, 
									double localrate, double foreignrate, const AQLDate& basedate, 
									const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate);
	//single barrier option easily type
	static double singleBarrierOptionEasily(AQLString& optiontype, AQLString& buysell, AQLString& callput,
											AQLString& downup, AQLString& inout, double rebate, double limit,
											double spot, double strike, double vol, 
											double localrate, double foreignrate,
											double expiryterms, double deliveryterms, int rebatetime);
	//double barrier option easily type
	static double doubleBarrierOption(AQLString& optiontype, AQLString& buysell, AQLString& callput, AQLString& inout,
									double limitlow, double limithigh, double rebate, AQLString& rebatetype,  
									double spot, double strike, double vol, 
									double localrate, double foreignrate, const AQLDate& basedate, 
									const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate,
									int num = 100);
	//forward delta 
	static double calcstrikefromdelta(double target, AQLString& spotfwd,  AQLString& callput, double spotfx, double fwdfx,
							double atmvol, double reversal, double strangle, double foreignrate,
								  AQLDate& basedate, AQLDate& spotdate, AQLDate& expirydate, AQLDate& deliverydate);

	static double calcstrikefromdelta(double target, AQLString& spotfwd,  AQLString& callput, double spotfx, double fwdfx,
							double atmvol, double reversal, double strangle, double foreignrate,
								  AQLDate& basedate, AQLDate& spotdate, AQLDate& expirydate, AQLDate& deliverydate,
								  double lower, double upper);
	static double calcstrikefromdelta(double target, AQLString& spotfwd, AQLString& callput, double spotfx, 
											  double fwdfx, double atmvol, double reversal, double strangle,
											  double foreigndf, double expiryterm);

	static double calcstrikefromdelta(double target, AQLString& spotfwd, AQLString& callput, double spotfx, 
											  double fwdfx, double atmvol, double reversal, double strangle,
											  double foreigndf, double expiryterm, double lower, double upper);
    
    static double calcmaxstrike(AQLString& spotfwd, const double spotfx, 
											  const double fwdfx, const double atmvol, const double reversal, const double strangle,
											  const double foreigndf, const double expiryterm, double lower = 0.01, double upper = -10.0);

    static double calcmaxfxdelta(AQLString& spotfwd, const double spotfx, 
											  const double fwdfx, const double atmvol, const double reversal, const double strangle,
											  const double foreigndf, const double expiryterm, double lower = 0.01 , double upper = -10.0);

	static void setupfxvolobject(AQLDataInstance* dataInstance, AQLString& interpid, AQLStringMatrix& baseinfomat, const DoubleArray& termvec, 
										   const DoubleMatrix& strikemat, const DoubleMatrix& volmat , double spotfx = 100.);
	
	static double getvaluefromfxvolobject(AQLDataInstance* dataInstance, AQLStringMatrix& baseinfomat, const double term,
												  const double strike);

	static double getspotfxfromfxvolobject(AQLDataInstance* dataInstance, AQLStringMatrix& baseinfomat );

private:
	LAMathFXVanillaFuncUtility(void);
	~LAMathFXVanillaFuncUtility(void);
	LAMathFXVanillaFuncUtility(const LAMathFXVanillaFuncUtility &rhs);
	LAMathFXVanillaFuncUtility &operator=(const LAMathFXVanillaFuncUtility &rhs);
};
#endif 