#ifndef AQLAnalyticFormula_h
#define AQLAnalyticFormula_h

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLDist.h"
#include "AQLCoreAppError.h"
#include "AQLBasic.h"
#include "AQLTime.h"
#include "ConstantDeclarations.h"

#include <typeinfo>

#define retchk  if(AQLTime::isError(ret)) throw AQLCoreNumericalError(ERMATH,__FILE__,__LINE__)
#define FORMULAE_BEGIN	try{ 
#define FORMULAE_END		}catch(std::bad_cast){throw AQLCoreInvalidData("params are not set well", __FILE__,__LINE__);}		


using namespace std;
// ==============================================

/*!
Parameter base class for BlackAnalysis
*/
class AnalyticParam
{
public :
	AnalyticParam(): Vol(0.0),K(0.0),actT(0.0){};
	virtual~ AnalyticParam(){};
	double Vol;
	double K;
	double actT;

	virtual void ErrorCheck(void)
	{
		if (K < 0.0)	throw AQLCoreInvalidData("Strike Negative!", __FILE__,__LINE__);
		if (Vol < 0.0)	throw AQLCoreInvalidData("Volatility Negative!", __FILE__,__LINE__);
		if (actT < 0.0)	throw AQLCoreInvalidData("Actual Term Negative!", __FILE__,__LINE__);
	}

	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticParam(*this);};

};
/*!
Parameter base class for AQLBlackAnalyticFormula
*/
class AnalyticBKParam : public AnalyticParam
{
public :
	AnalyticBKParam(): F(0.0),Td(0.0), Te(0.0), Nu(0.0),rd(0.0){};
	virtual~ AnalyticBKParam(){};
	
	double F;	//Future Price
	double Td;	//Delivery Terms
	double Te;	//Expiry Terms
	double Nu;	//Numeraire Ratio
	double rd;	//Domestic Rate

	virtual void ErrorCheck(void)
	{
		AnalyticParam::ErrorCheck();
		if (F < 0.0)					   throw AQLCoreInvalidData("FuturePrice Negative!", __FILE__,__LINE__);
		if (rd < 0.0)					   throw AQLCoreInvalidData("LocalRate Negative!", __FILE__,__LINE__);
		if (Te < 0.0)					   throw AQLCoreInvalidData("ExpiryTerm Negative!", __FILE__,__LINE__);
		if (Td < 0.0)					   throw AQLCoreInvalidData("DeliveryTerm Negative!", __FILE__,__LINE__);
		if (Nu < 0.0)					   throw AQLCoreInvalidData("NumeraireRatio(DF) Negative!", __FILE__,__LINE__);
	}
	
	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticBKParam(*this);};
//============================================================================
};
/*!
Parameter base class for AQLGreekAnalyticFormula
*/
class AnalyticGKParam : public AnalyticParam
{
public :
	AnalyticGKParam(): S(0.0),Td(0.0),Te(0.0),rf(0.0),rd(0.0),F(0.0),DFf(0.0),DFd(0.0){};
	virtual~ AnalyticGKParam(){};
	
	double S;	//Spot price
	double Td;	//deliveryTerms
	double Te;	//expiryTerms
	double rf;	//foreign rate
	double rd;	//local rate
	double F;	//forward price
	double DFf;	//foreign DF
	double DFd;	//domestic DF

	virtual void ErrorCheck(void)
	{
		AnalyticParam::ErrorCheck();
		if (S < 0.0)					   throw AQLCoreInvalidData("SpotPrice Negative!", __FILE__,__LINE__);
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		//if (rd < 0.0)					   throw AQLCoreInvalidData("LocalRate Negative!", __FILE__,__LINE__);
		//if (rf < 0.0)					   throw AQLCoreInvalidData("ForeignRate Negative!", __FILE__,__LINE__);
#endif
		if (Te < 0.0)					   throw AQLCoreInvalidData("ExpiryTerm Negative!", __FILE__,__LINE__);
		if (Td < 0.0)					   throw AQLCoreInvalidData("DeliveryTerm Negative!", __FILE__,__LINE__);
		if (F < 0.0)					   throw AQLCoreInvalidData("ForwardPrice Negative!", __FILE__,__LINE__);
		if (DFf < 0.0)					   throw AQLCoreInvalidData("Foreign DF Negative!", __FILE__,__LINE__);
		if (DFd < 0.0)					   throw AQLCoreInvalidData("Domestic DF Negative!", __FILE__,__LINE__);
	}

	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticGKParam(*this);};

};

/*!
Parameter base class for MMCFAnalyticFormula
*/
class AnalyticCFParam : public AnalyticParam
{
public :
	AnalyticCFParam():F(vector<double > (0,0.0)),Te(vector<double > (0,0.0)),Nu(vector<double > (0,0.0)),pos(1),iswithoutfirst(1){};
	virtual~ AnalyticCFParam(){};

	vector<double > F;	
	vector<double > Te;	
	vector<double > Nu;
	unsigned int pos;
	bool iswithoutfirst;

	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticCFParam(*this);};

};
/*!
Parameter base class for AQLDigitalAnalyticFormula
*/
class AnalyticDGParam : public AnalyticGKParam
{
public :
	AnalyticDGParam(): Dig(0.0){};
	virtual~ AnalyticDGParam(){};

	double Dig;  //payoff of digital option i.e. call digital payoff = (Dig-S)^+;

	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticDGParam(*this);};
};
/*!
Parameter base class for AQLDigitalAnalyticFormula
*/
class AnalyticSBParam : public AnalyticGKParam
{
public :
	AnalyticSBParam(): L(0.0),R(0.0),cp(1),du(1),ReTime(1){};
	virtual~ AnalyticSBParam(){};
	
	double L;	//Limit value
	double R;	//Rebate value
	int cp;		//if call cp=1, if put cp=-1 
	int du;		//if down du=1, if up du=-1
	int ReTime;	//Retime at Barrier Reach ReTime=1, time at DeliveryDate ReTimi =0 (Defalut ReTime==1)

	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticSBParam(*this);};
};
/*!
Parameter base class AQLDigitalAnalyticFormula BlackAnalysis
*/
class AnalyticDBParam : public AnalyticGKParam
{
public :
	AnalyticDBParam(): Ll(0.0),Lh(0.0),Rl(0.0),Rh(0.0),Ri(0.0),RelTime(1),RehTime(1),Num(0){};
	virtual~ AnalyticDBParam(){};
	
	double Ll;	//Limit low value
	double Lh;	//Limit high value
	double Rl;	//Rebate low value(when knockout)
	double Rh;	//Rebate high value(when knockout)
	double Ri;	//Rebate value(when "Knock in")
	int RelTime;//Reltime at Barrier Reach RelTime=1, time at DeliveryDate RelTime =0 (Defalut RelTime==1)
	int RehTime;//Rehtime at Barrier Reach RehTime=1, time at DeliveryDate RehTimi =0 (Defalut ReTime==1)
	int Num;	//Num is number of sum used for Double Barrier

	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticDBParam(*this);};
};
/*!
Parameter base class for AQLAffineModelAnalyticFormula
*/
class AnalyticAFFParam : public AnalyticParam
{
public :
	AnalyticAFFParam(): Pbondm(0.0),Poptm(0.0){};
	virtual~ AnalyticAFFParam(){};
	
	double Pbondm;	//Zero Bond price (underlying) 
	double Poptm;	//Zero Bond price at Option Maturity
	
	virtual void ErrorCheck(void)
	{
		AnalyticParam::ErrorCheck();
		if (Pbondm < 0.0)					   throw AQLCoreInvalidData("Pbondm Negative!", __FILE__,__LINE__);
		if (Poptm < 0.0)					   throw AQLCoreInvalidData("Poptm Negative!", __FILE__,__LINE__);
	}

	// vanna-volga
	virtual AnalyticParam* clone() const { return new AnalyticAFFParam(*this);};
};



/////////////////////////

class AnalyticFormulae
{
public:
/*!
	Garman-Kohlgagen Formula
	S		spot price
	K		strike
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rf	foreign rate
	rd	local rate
*/
	//differencial of Normdist	
	static double diffNormdist(double z);
	//2nd differencial of Normdist
	static double diff2ndNormdist(double z);
	
	static double GKd1(AnalyticParam& param);
	static double GKd2(AnalyticParam& param);
	static double GKpremCall(AnalyticParam& param);
	static double GKpremPut(AnalyticParam& param);
	static double GKdeltaCall(AnalyticParam& param);
	static double GKdeltaPut(AnalyticParam& param);
	static double GKgammaCall(AnalyticParam& param);
	static double GKgammaPut(AnalyticParam& param);
	static double GKthetaCall(AnalyticParam& param);
	static double GKthetaPut(AnalyticParam& param);
	static double GKvegaCall(AnalyticParam& param);
	static double GKvegaPut(AnalyticParam& param);
	static double GKphiCall(AnalyticParam& param);
	static double GKphiPut(AnalyticParam& param);
	static double GKrhoCall(AnalyticParam& param);
	static double GKrhoPut(AnalyticParam& param);
	static double GKvannaCall(AnalyticParam& param);
	static double GKvannaPut(AnalyticParam& param);
	static double GKvolgaCall(AnalyticParam& param);
	static double GKvolgaPut(AnalyticParam& param);
/*!
	Black option Formula
	F		spot price
	K		strike 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	Nu	Numeraire
*/
	static double BKd1			(AnalyticParam& param);
	static double BKd2			(AnalyticParam& param);
	static double BKpremCall	(AnalyticParam& param);
	static double BKpremPut		(AnalyticParam& param);
	static double BKvegaCall	(AnalyticParam& param);
	static double BKvegaPut		(AnalyticParam& param);
	static double BKdeltaCall	(AnalyticParam& param);
	static double BKdeltaPut	(AnalyticParam& param);
	static double BKgammaCall	(AnalyticParam& param);
	static double BKgammaPut	(AnalyticParam& param);
	static double BKthetaCall	(AnalyticParam& param);
	static double BKthetaPut	(AnalyticParam& param);

/*!
	Black option Formula
	F		spot price
	K		strike 
*/
	static double BKPayOffpremCall	(AnalyticParam& param);
	static double BKPayOffpremPut	(AnalyticParam& param);

/*!
Parameter base class for MMCFAnalyticFormula
*/
	static double AQLCFpremCall(AnalyticParam& param);
	static double AQLCFvegaCall(AnalyticParam& param);
	static double AQLCFpremPut(AnalyticParam& param);
	static double AQLCFvegaPut(AnalyticParam& param);
/*!
	/////Digital option Formula
	S		spot price
	K		strike
	Dig	payoff of digital option 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rf	foreign rate
	rd	local rate
*/
	static double DGd1(AnalyticParam& param);
	static double DGd2(AnalyticParam& param);
	static double DGpremCall	(AnalyticParam& param);
	static double DGpremPut	(AnalyticParam& param);
	static double DGdeltaCall	(AnalyticParam& param);
	static double DGdeltaPut	(AnalyticParam& param);
	static double DGgammaCall	(AnalyticParam& param);
	static double DGgammaPut	(AnalyticParam& param);
	static double DGthetaCall	(AnalyticParam& param);
	static double DGthetaPut	(AnalyticParam& param);
	static double DGvegaCall	(AnalyticParam& param);
	static double DGvegaPut	(AnalyticParam& param);
	static double DGrhoCall	(AnalyticParam& param);
	static double DGrhoPut	(AnalyticParam& param);
	static double DGphiCall	(AnalyticParam& param);
	static double DGphiPut	(AnalyticParam& param);
	/*!
	//////Singlebarrier option Formula
	S		spot price
	K		strike 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rd	local rate
	rf	foreign rate
	L		Limit value
	R		Rebate value
	cp	if call cp =1, if put cp=-1
	du	if down du =1, if up  du=-1
	ReTime time at Barrier Reach ReTime=1, time at DeliveryDate ReTime=0 (Defalut ReTime ==1)
*/

	// vanna-volga
	static double AQLSBprobUNT (AnalyticParam& param);
	static double AQLSBprobDNT (AnalyticParam& param);


	static double SBmu1(AnalyticParam& param);
	static double SBmu2(AnalyticParam& param);
	static double SBmu3(AnalyticParam& param);
	static double SBnu1(AnalyticParam& param);
	static double SBnu2(AnalyticParam& param);
	static double SBx1(AnalyticParam& param);
	static double SBx2(AnalyticParam& param);
	static double SBy1(AnalyticParam& param);
	static double SBy2(AnalyticParam& param);
	static double SBz(AnalyticParam& param);
	static double SBI1(AnalyticParam& param);
	static double SBI2(AnalyticParam& param);
	static double SBI3(AnalyticParam& param);
	static double SBI4(AnalyticParam& param);
	static double SBI5(AnalyticParam& param);
	static double SBI6(AnalyticParam& param);
	static double SBpremDIC	(AnalyticParam& param);
	static double SBpremDIP	(AnalyticParam& param);
	static double SBpremDOC	(AnalyticParam& param);
	static double SBpremDOP	(AnalyticParam& param);
	static double SBpremUIC	(AnalyticParam& param);
	static double SBpremUIP	(AnalyticParam& param);
	static double SBpremUOC	(AnalyticParam& param);
	static double SBpremUOP	(AnalyticParam& param);
	static double SBdx1(AnalyticParam& param);
	static double SBdx2(AnalyticParam& param);
	static double SBdy1(AnalyticParam& param);
	static double SBdy2(AnalyticParam& param);
	static double SBdz(AnalyticParam& param);
	static double SBddx1(AnalyticParam& param);
	static double SBddx2(AnalyticParam& param);
	static double SBddy1(AnalyticParam& param);
	static double SBddy2(AnalyticParam& param);
	static double SBddz(AnalyticParam& param);
	static double SBvmu1(AnalyticParam& param);
	static double SBvmu2(AnalyticParam& param);
	static double SBvmu3(AnalyticParam& param);
	static double SBvnu1(AnalyticParam& param);
	static double SBvnu2(AnalyticParam& param);
	static double SBvx1(AnalyticParam& param);
	static double SBvx2(AnalyticParam& param);
	static double SBvy1(AnalyticParam& param);
	static double SBvy2(AnalyticParam& param);
	static double SBvz(AnalyticParam& param);
	static double SBtmu1(AnalyticParam& param);
	static double SBtmu2(AnalyticParam& param);
	static double SBtmu3(AnalyticParam& param);
	static double SBtnu1(AnalyticParam& param);
	static double SBtnu2(AnalyticParam& param);
	static double SBtx1(AnalyticParam& param);
	static double SBtx2(AnalyticParam& param);
	static double SBty1(AnalyticParam& param);
	static double SBty2(AnalyticParam& param);
	static double SBtz(AnalyticParam& param);
	static double SBrmu1(AnalyticParam& param);
	static double SBrmu2(AnalyticParam& param);
	static double SBrmu3(AnalyticParam& param);
	static double SBrnu1(AnalyticParam& param);
	static double SBrnu2(AnalyticParam& param);
	static double SBrx1(AnalyticParam& param);
	static double SBrx2(AnalyticParam& param);
	static double SBry1(AnalyticParam& param);
	static double SBry2(AnalyticParam& param);
	static double SBrz(AnalyticParam& param);
	static double SBpmu1(AnalyticParam& param);
	static double SBpmu2(AnalyticParam& param);
	static double SBpmu3(AnalyticParam& param);
	static double SBpnu1(AnalyticParam& param);
	static double SBpnu2(AnalyticParam& param);
	static double SBpx1(AnalyticParam& param);
	static double SBpx2(AnalyticParam& param);
	static double SBpy1(AnalyticParam& param);
	static double SBpy2(AnalyticParam& param);
	static double SBpz(AnalyticParam& param);
	static double SBdI1(AnalyticParam& param);
	static double SBdI2(AnalyticParam& param);
	static double SBdI3(AnalyticParam& param);
	static double SBdI4(AnalyticParam& param);
	static double SBdI5(AnalyticParam& param);
	static double SBdI6(AnalyticParam& param);
	static double SBgI1(AnalyticParam& param);
	static double SBgI2(AnalyticParam& param);
	static double SBgI3(AnalyticParam& param);
	static double SBgI4(AnalyticParam& param);
	static double SBgI5(AnalyticParam& param);
	static double SBgI6(AnalyticParam& param);
	static double SBvI1(AnalyticParam& param);
	static double SBvI2(AnalyticParam& param);
	static double SBvI3(AnalyticParam& param);
	static double SBvI4(AnalyticParam& param);
	static double SBvI5(AnalyticParam& param);
	static double SBvI6(AnalyticParam& param);
	static double SBtI1(AnalyticParam& param);
	static double SBtI2(AnalyticParam& param);
	static double SBtI3(AnalyticParam& param);
	static double SBtI4(AnalyticParam& param);
	static double SBtI5(AnalyticParam& param);
	static double SBtI6(AnalyticParam& param);
	static double SBrI1(AnalyticParam& param);
	static double SBrI2(AnalyticParam& param);
	static double SBrI3(AnalyticParam& param);
	static double SBrI4(AnalyticParam& param);
	static double SBrI5(AnalyticParam& param);
	static double SBrI6(AnalyticParam& param);
	static double SBpI1(AnalyticParam& param);
	static double SBpI2(AnalyticParam& param);
	static double SBpI3(AnalyticParam& param);
	static double SBpI4(AnalyticParam& param);
	static double SBpI5(AnalyticParam& param);
	static double SBpI6(AnalyticParam& param);
	static double SBdeltaDIC	(AnalyticParam& param);
	static double SBdeltaDIP	(AnalyticParam& param);
	static double SBdeltaDOC	(AnalyticParam& param);
	static double SBdeltaDOP	(AnalyticParam& param);
	static double SBdeltaUIC	(AnalyticParam& param);
	static double SBdeltaUIP	(AnalyticParam& param);
	static double SBdeltaUOC	(AnalyticParam& param);
	static double SBdeltaUOP	(AnalyticParam& param);
	static double SBgammaDIC	(AnalyticParam& param);
	static double SBgammaDIP	(AnalyticParam& param);
	static double SBgammaDOC	(AnalyticParam& param);
	static double SBgammaDOP	(AnalyticParam& param);
	static double SBgammaUIC	(AnalyticParam& param);
	static double SBgammaUIP	(AnalyticParam& param);
	static double SBgammaUOC	(AnalyticParam& param);
	static double SBgammaUOP	(AnalyticParam& param);
	static double SBvegaDIC	(AnalyticParam& param);
	static double SBvegaDIP	(AnalyticParam& param);
	static double SBvegaDOC	(AnalyticParam& param);
	static double SBvegaDOP	(AnalyticParam& param);
	static double SBvegaUIC	(AnalyticParam& param);
	static double SBvegaUIP	(AnalyticParam& param);
	static double SBvegaUOC	(AnalyticParam& param);
	static double SBvegaUOP	(AnalyticParam& param);
	static double SBthetaDIC	(AnalyticParam& param);
	static double SBthetaDIP	(AnalyticParam& param);
	static double SBthetaDOC	(AnalyticParam& param);
	static double SBthetaDOP	(AnalyticParam& param);
	static double SBthetaUIC	(AnalyticParam& param);
	static double SBthetaUIP	(AnalyticParam& param);
	static double SBthetaUOC	(AnalyticParam& param);
	static double SBthetaUOP	(AnalyticParam& param);
	static double SBrhoDIC	(AnalyticParam& param);
	static double SBrhoDIP	(AnalyticParam& param);
	static double SBrhoDOC	(AnalyticParam& param);
	static double SBrhoDOP	(AnalyticParam& param);
	static double SBrhoUIC	(AnalyticParam& param);
	static double SBrhoUIP	(AnalyticParam& param);
	static double SBrhoUOC	(AnalyticParam& param);
	static double SBrhoUOP	(AnalyticParam& param);
	static double SBphiDIC	(AnalyticParam& param);
	static double SBphiDIP	(AnalyticParam& param);
	static double SBphiDOC	(AnalyticParam& param);
	static double SBphiDOP	(AnalyticParam& param);
	static double SBphiUIC	(AnalyticParam& param);
	static double SBphiUIP	(AnalyticParam& param);
	static double SBphiUOC	(AnalyticParam& param);
	static double SBphiUOP	(AnalyticParam& param);

/*!
	//////DoubleBarrier option Formula
	S		spot price
	K		strike 
	Vol	volatility
	Td	delivery terms
	Te	expiry terms
	rd	local rate
	rf	foreign rate
	Ll	Limit low value
	Lh	Limit high value
	Rl	Rebate low value(when receive Knockout)
	Rh	Rebate high value(when receive Knockout)
	Ri	Rebate value(when receive "KnockIn")
	RelTime   time at Barrier Reach RelTime=1, time at DeliveryDate RelTime=0 (Defalut RelTime ==1)
	RehTime	time at Barrier Reach RehTime=1, time at DeliveryDate RehTime=0 (Defalut RehTime ==1)
	Num is number of sum used for Double Barrier
*/

	static double DBmu1(AnalyticParam& param);
	static double DBmu2(AnalyticParam& param);
	static double DBmu3(AnalyticParam& param);
	static double x0	(AnalyticParam& param);
	static double xl	(AnalyticParam& param);
	static double xh	(AnalyticParam& param);
	static double Phi(double z1, double z2, double b, double c, double d);
	static double u(int k, double xh, double xl);
	static double F(double t, double mu, double vol, double xl, double xh, double nu, double z1, double z2, int sumrange);
	static double Gh(double t, double mu, double vol, double xl, double xh, int sumrange);
	static double Gl(double t, double mu, double vol, double xl, double xh, int sumrange);
	static double DBpremKOC(AnalyticParam& param);
	static double DBpremKIC(AnalyticParam& param);
	static double DBpremKOP(AnalyticParam& param);
	static double DBpremKIP(AnalyticParam& param);
	static double DBpremRL(AnalyticParam& param);
	static double DBpremRH(AnalyticParam& param);
	static double DBpremRI(AnalyticParam& param);
	static double FDd1(AnalyticParam& param);
	static double FDd2(AnalyticParam& param);
	static double FDdeltaCallSpot(AnalyticParam& param);
	static double FDdeltaPutSpot(AnalyticParam& param);
	static double FD1stDiffCallSpot(AnalyticParam& param);
	static double FD1stDiffPutSpot(AnalyticParam& param);
	static double FDdeltaCallFwd(AnalyticParam& param);
	static double FDdeltaPutFwd(AnalyticParam& param);
	static double FD1stDiffCallFwd(AnalyticParam& param);
	static double FD1stDiffPutFwd(AnalyticParam& param);
	static double FD2ndDiffCallFwd(AnalyticParam& param);
    static double FD2ndDiffCallSpot(AnalyticParam& param);

/*!
	AFFineModel Formula
	K		strike
	Vol		volatility
	Pbondm	Zero Bond(underlying)
	Poptm	Zero Bond Price whose maturity is option maturity
*/
	static double AFFd1(AnalyticParam& param);
	static double AFFd2(AnalyticParam& param);
	static double AFFpremCall(AnalyticParam& param);
	static double AFFpremPut(AnalyticParam& param);
};

 
#endif 
