#pragma once

#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLCoreAppError.h"
#include <cmath>
#include "LAAnalyticFormula.h"

class LABlackScholesBase
{
public :
	LABlackScholesBase(){}
	virtual ~LABlackScholesBase(){}
	virtual double calc(AnalyticParam& param)=0;
};
//0Value
class MMZeroForError :public LABlackScholesBase
{
	public :
	MMZeroForError(){}
	virtual ~MMZeroForError(){}
	double calc(AnalyticParam& param){(void)param; return 0.0;}
};



//LABlackPayOff
class LABlackPayOffpremCall : public LABlackScholesBase
{
public :
	LABlackPayOffpremCall(){}
	virtual ~LABlackPayOffpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKPayOffpremCall(param);}
};
class LABlackPayOffpremPut : public LABlackScholesBase
{
public :
	LABlackPayOffpremPut(){}
	virtual ~LABlackPayOffpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKPayOffpremPut(param);}
};


//LABlack
class LABlackpremCall : public LABlackScholesBase
{
public :
	LABlackpremCall(){}
	virtual ~LABlackpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKpremCall(param);}
};
class LABlackpremPut : public LABlackScholesBase
{
public :
	LABlackpremPut(){}
	virtual ~LABlackpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKpremPut(param);}
};
class LABlackvegaCall : public LABlackScholesBase
{
public :
	LABlackvegaCall(){}
	virtual ~LABlackvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKvegaCall(param);}
};
class LABlackvegaPut : public LABlackScholesBase
{
public :
	LABlackvegaPut(){}
	virtual ~LABlackvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKvegaPut(param);}
};
class LABlackdeltaCall : public LABlackScholesBase
{
public :
	LABlackdeltaCall(){}
	virtual ~LABlackdeltaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKdeltaCall(param);}
};
class LABlackdeltaPut : public LABlackScholesBase
{
public :
	LABlackdeltaPut(){}
	virtual ~LABlackdeltaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKdeltaPut(param);}
};

class LABlackgammaCall : public LABlackScholesBase
{
public :
	LABlackgammaCall(){}
	virtual ~LABlackgammaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKgammaCall(param);}
};
class LABlackgammaPut : public LABlackScholesBase
{
public :
	LABlackgammaPut(){}
	virtual ~LABlackgammaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKgammaPut(param);}
};
class LABlackthetaCall : public LABlackScholesBase
{
public :
	LABlackthetaCall(){}
	virtual ~LABlackthetaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKthetaCall(param);}
};
class LABlackthetaPut : public LABlackScholesBase
{
public :
	LABlackthetaPut(){}
	virtual ~LABlackthetaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKthetaPut(param);}
};

//MMCF
class MMCFpremCall : public LABlackScholesBase
{
public :
	MMCFpremCall(){}
	virtual ~MMCFpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFpremCall(param);}
};
class MMCFvegaCall : public LABlackScholesBase
{
public :
	MMCFvegaCall(){}
	virtual ~MMCFvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFvegaCall(param);}
};
class MMCFpremPut : public LABlackScholesBase
{
public :
	MMCFpremPut(){}
	virtual ~MMCFpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFpremPut(param);}
};class MMCFvegaPut : public LABlackScholesBase
{
public :
	MMCFvegaPut(){}
	virtual ~MMCFvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFvegaPut(param);}
};
//LAGreek
class LAGreekpremCall : public LABlackScholesBase
{
public :
	LAGreekpremCall(){}
	virtual ~LAGreekpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKpremCall(param);}
};
class LAGreekdeltaCall : public LABlackScholesBase
{
public :
	LAGreekdeltaCall(){}
	virtual ~LAGreekdeltaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKdeltaCall(param);}
};
class LAGreekgammaCall : public LABlackScholesBase
{
public :
	LAGreekgammaCall(){}
	virtual ~LAGreekgammaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKgammaCall(param);}
};
class LAGreekvegaCall : public LABlackScholesBase
{
public :
	LAGreekvegaCall(){}
	virtual ~LAGreekvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvegaCall(param);}
};
class LAGreekthetaCall : public LABlackScholesBase
{
public :
	LAGreekthetaCall(){}
	virtual ~LAGreekthetaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKthetaCall(param);}
};
class LAGreekrhoCall : public LABlackScholesBase
{
public :
	LAGreekrhoCall(){}
	virtual ~LAGreekrhoCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKrhoCall(param);}
};
class LAGreekphiCall : public LABlackScholesBase
{
public :
	LAGreekphiCall(){}
	virtual ~LAGreekphiCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKphiCall(param);}
};
class LAGreekvannaCall : public LABlackScholesBase
{
public :
	LAGreekvannaCall(){}
	virtual ~LAGreekvannaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvannaCall(param);}
};
class LAGreekvolgaCall : public LABlackScholesBase
{
public :
	LAGreekvolgaCall(){}
	virtual ~LAGreekvolgaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvolgaCall(param);}
};
class LAGreekpremPut : public LABlackScholesBase
{
public :
	LAGreekpremPut(){}
	virtual ~LAGreekpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKpremPut(param);}
};
class LAGreekdeltaPut : public LABlackScholesBase
{
public :
	LAGreekdeltaPut(){}
	virtual ~LAGreekdeltaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKdeltaPut(param);}
};
class LAGreekgammaPut : public LABlackScholesBase
{
public :
	LAGreekgammaPut(){}
	virtual ~LAGreekgammaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKgammaPut(param);}
};
class LAGreekvegaPut : public LABlackScholesBase
{
public :
	LAGreekvegaPut(){}
	virtual ~LAGreekvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvegaPut(param);}
};
class LAGreekthetaPut : public LABlackScholesBase
{
public :
	LAGreekthetaPut(){}
	virtual ~LAGreekthetaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKthetaPut(param);}
};
class LAGreekrhoPut : public LABlackScholesBase
{
public :
	LAGreekrhoPut(){}
	virtual ~LAGreekrhoPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKrhoPut(param);}
};
class LAGreekphiPut : public LABlackScholesBase
{
public :
	LAGreekphiPut(){}
	virtual ~LAGreekphiPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKphiPut(param);}
};
class LAGreekvannaPut : public LABlackScholesBase
{
public :
	LAGreekvannaPut(){}
	virtual ~LAGreekvannaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvannaPut(param);}
};
class LAGreekvolgaPut : public LABlackScholesBase
{
public :
	LAGreekvolgaPut(){}
	virtual ~LAGreekvolgaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvolgaPut(param);}
};
//MMDG
class MMDGpremCall : public LABlackScholesBase
{
public :
	MMDGpremCall(){}
	virtual ~MMDGpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGpremCall(param);}
};
class MMDGdeltaCall : public LABlackScholesBase
{
public :
	MMDGdeltaCall(){}
	virtual ~MMDGdeltaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGdeltaCall(param);}
};
class MMDGgammaCall : public LABlackScholesBase
{
public :
	MMDGgammaCall(){}
	virtual ~MMDGgammaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGgammaCall(param);}
};
class MMDGvegaCall : public LABlackScholesBase
{
public :
	MMDGvegaCall(){}
	virtual ~MMDGvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGvegaCall(param);}
};
class MMDGthetaCall : public LABlackScholesBase
{
public :
	MMDGthetaCall(){}
	virtual ~MMDGthetaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGthetaCall(param);}
};
class MMDGrhoCall : public LABlackScholesBase
{
public :
	MMDGrhoCall(){}
	virtual ~MMDGrhoCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGrhoCall(param);}
};
class MMDGphiCall : public LABlackScholesBase
{
public :
	MMDGphiCall(){}
	virtual ~MMDGphiCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGphiCall(param);}
};

class MMDGpremPut : public LABlackScholesBase
{
public :
	MMDGpremPut(){}
	virtual ~MMDGpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGpremPut(param);}
};
class MMDGdeltaPut : public LABlackScholesBase
{
public :
	MMDGdeltaPut(){}
	virtual ~MMDGdeltaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGdeltaPut(param);}
};
class MMDGgammaPut : public LABlackScholesBase
{
public :
	MMDGgammaPut(){}
	virtual ~MMDGgammaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGgammaPut(param);}
};
class MMDGvegaPut : public LABlackScholesBase
{
public :
	MMDGvegaPut(){}
	virtual ~MMDGvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGvegaPut(param);}
};
class MMDGthetaPut : public LABlackScholesBase
{
public :
	MMDGthetaPut(){}
	virtual ~MMDGthetaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGthetaPut(param);}
};
class MMDGrhoPut : public LABlackScholesBase
{
public :
	MMDGrhoPut(){}
	virtual ~MMDGrhoPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGrhoPut(param);}
};
class MMDGphiPut : public LABlackScholesBase
{
public :
	MMDGphiPut(){}
	virtual ~MMDGphiPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGphiPut(param);}
};

//hishida vannavolga

class MMSBprobUNT : public LABlackScholesBase
{
public :
	MMSBprobUNT(){}
	virtual ~MMSBprobUNT(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMSBprobUNT(param);}
};
class MMSBprobDNT : public LABlackScholesBase
{
public :
	MMSBprobDNT(){}
	virtual ~MMSBprobDNT(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMSBprobDNT(param);}
};
//hishida vannavolga


//MMSB
class MMSBpremDIC : public LABlackScholesBase
{
public :
	MMSBpremDIC(){}
	virtual ~MMSBpremDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDIC(param);}
};
class MMSBpremDIP : public LABlackScholesBase
{
public :
	MMSBpremDIP(){}
	virtual ~MMSBpremDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDIP(param);}
};
class MMSBpremDOC : public LABlackScholesBase
{
public :
	MMSBpremDOC(){}
	virtual ~MMSBpremDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDOC(param);}
};
class MMSBpremDOP : public LABlackScholesBase
{
public :
	MMSBpremDOP(){}
	virtual ~MMSBpremDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDOP(param);}
};
class MMSBpremUIC : public LABlackScholesBase
{
public :
	MMSBpremUIC(){}
	virtual ~MMSBpremUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUIC(param);}
};
class MMSBpremUIP : public LABlackScholesBase
{
public :
	MMSBpremUIP(){}
	virtual ~MMSBpremUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUIP(param);}
};
class MMSBpremUOC : public LABlackScholesBase
{
public :
	MMSBpremUOC(){}
	virtual ~MMSBpremUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUOC(param);}
};
class MMSBpremUOP : public LABlackScholesBase
{
public :
	MMSBpremUOP(){}
	virtual ~MMSBpremUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUOP(param);}
};

class MMSBdeltaDIC : public LABlackScholesBase
{
public :
	MMSBdeltaDIC(){}
	virtual ~MMSBdeltaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDIC(param);}
};
class MMSBdeltaDIP : public LABlackScholesBase
{
public :
	MMSBdeltaDIP(){}
	virtual ~MMSBdeltaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDIP(param);}
};
class MMSBdeltaDOC : public LABlackScholesBase
{
public :
	MMSBdeltaDOC(){}
	virtual ~MMSBdeltaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDOC(param);}
};
class MMSBdeltaDOP : public LABlackScholesBase
{
public :
	MMSBdeltaDOP(){}
	virtual ~MMSBdeltaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDOP(param);}
};
class MMSBdeltaUIC : public LABlackScholesBase
{
public :
	MMSBdeltaUIC(){}
	virtual ~MMSBdeltaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUIC(param);}
};
class MMSBdeltaUIP : public LABlackScholesBase
{
public :
	MMSBdeltaUIP(){}
	virtual ~MMSBdeltaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUIP(param);}
};
class MMSBdeltaUOC : public LABlackScholesBase
{
public :
	MMSBdeltaUOC(){}
	virtual ~MMSBdeltaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUOC(param);}
};
class MMSBdeltaUOP : public LABlackScholesBase
{
public :
	MMSBdeltaUOP(){}
	virtual ~MMSBdeltaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUOP(param);}
};
class MMSBgammaDIC : public LABlackScholesBase
{
public :
	MMSBgammaDIC(){}
	virtual ~MMSBgammaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDIC(param);}
};
class MMSBgammaDIP : public LABlackScholesBase
{
public :
	MMSBgammaDIP(){}
	virtual ~MMSBgammaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDIP(param);}
};
class MMSBgammaDOC : public LABlackScholesBase
{
public :
	MMSBgammaDOC(){}
	virtual ~MMSBgammaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDOC(param);}
};
class MMSBgammaDOP : public LABlackScholesBase
{
public :
	MMSBgammaDOP(){}
	virtual ~MMSBgammaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDOP(param);}
};
class MMSBgammaUIC : public LABlackScholesBase
{
public :
	MMSBgammaUIC(){}
	virtual ~MMSBgammaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUIC(param);}
};
class MMSBgammaUIP : public LABlackScholesBase
{
public :
	MMSBgammaUIP(){}
	virtual ~MMSBgammaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUIP(param);}
};
class MMSBgammaUOC : public LABlackScholesBase
{
public :
	MMSBgammaUOC(){}
	virtual ~MMSBgammaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUOC(param);}
};
class MMSBgammaUOP : public LABlackScholesBase
{
public :
	MMSBgammaUOP(){}
	virtual ~MMSBgammaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUOP(param);}
};

class MMSBvegaDIC : public LABlackScholesBase
{
public :
	MMSBvegaDIC(){}
	virtual ~MMSBvegaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDIC(param);}
};
class MMSBvegaDIP : public LABlackScholesBase
{
public :
	MMSBvegaDIP(){}
	virtual ~MMSBvegaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDIP(param);}
};
class MMSBvegaDOC : public LABlackScholesBase
{
public :
	MMSBvegaDOC(){}
	virtual ~MMSBvegaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDOC(param);}
};
class MMSBvegaDOP : public LABlackScholesBase
{
public :
	MMSBvegaDOP(){}
	virtual ~MMSBvegaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDOP(param);}
};
class MMSBvegaUIC : public LABlackScholesBase
{
public :
	MMSBvegaUIC(){}
	virtual ~MMSBvegaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUIC(param);}
};
class MMSBvegaUIP : public LABlackScholesBase
{
public :
	MMSBvegaUIP(){}
	virtual ~MMSBvegaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUIP(param);}
};
class MMSBvegaUOC : public LABlackScholesBase
{
public :
	MMSBvegaUOC(){}
	virtual ~MMSBvegaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUOC(param);}
};
class MMSBvegaUOP : public LABlackScholesBase
{
public :
	MMSBvegaUOP(){}
	virtual ~MMSBvegaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUOP(param);}
};

class MMSBthetaDIC : public LABlackScholesBase
{
public :
	MMSBthetaDIC(){}
	virtual ~MMSBthetaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDIC(param);}
};
class MMSBthetaDIP : public LABlackScholesBase
{
public :
	MMSBthetaDIP(){}
	virtual ~MMSBthetaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDIP(param);}
};
class MMSBthetaDOC : public LABlackScholesBase
{
public :
	MMSBthetaDOC(){}
	virtual ~MMSBthetaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDOC(param);}
};
class MMSBthetaDOP : public LABlackScholesBase
{
public :
	MMSBthetaDOP(){}
	virtual ~MMSBthetaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDOP(param);}
};
class MMSBthetaUIC : public LABlackScholesBase
{
public :
	MMSBthetaUIC(){}
	virtual ~MMSBthetaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUIC(param);}
};
class MMSBthetaUIP : public LABlackScholesBase
{
public :
	MMSBthetaUIP(){}
	virtual ~MMSBthetaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUIP(param);}
};
class MMSBthetaUOC : public LABlackScholesBase
{
public :
	MMSBthetaUOC(){}
	virtual ~MMSBthetaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUOC(param);}
};
class MMSBthetaUOP : public LABlackScholesBase
{
public :
	MMSBthetaUOP(){}
	virtual ~MMSBthetaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUOP(param);}
};

class MMSBrhoDIC : public LABlackScholesBase
{
public :
	MMSBrhoDIC(){}
	virtual ~MMSBrhoDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDIC(param);}
};
class MMSBrhoDIP : public LABlackScholesBase
{
public :
	MMSBrhoDIP(){}
	virtual ~MMSBrhoDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDIP(param);}
};
class MMSBrhoDOC : public LABlackScholesBase
{
public :
	MMSBrhoDOC(){}
	virtual ~MMSBrhoDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDOC(param);}
};
class MMSBrhoDOP : public LABlackScholesBase
{
public :
	MMSBrhoDOP(){}
	virtual ~MMSBrhoDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDOP(param);}
};
class MMSBrhoUIC : public LABlackScholesBase
{
public :
	MMSBrhoUIC(){}
	virtual ~MMSBrhoUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUIC(param);}
};
class MMSBrhoUIP : public LABlackScholesBase
{
public :
	MMSBrhoUIP(){}
	virtual ~MMSBrhoUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUIP(param);}
};
class MMSBrhoUOC : public LABlackScholesBase
{
public :
	MMSBrhoUOC(){}
	virtual ~MMSBrhoUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUOC(param);}
};
class MMSBrhoUOP : public LABlackScholesBase
{
public :
	MMSBrhoUOP(){}
	virtual ~MMSBrhoUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUOP(param);}
};

class MMSBphiDIC : public LABlackScholesBase
{
public :
	MMSBphiDIC(){}
	virtual ~MMSBphiDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDIC(param);}
};
class MMSBphiDIP : public LABlackScholesBase
{
public :
	MMSBphiDIP(){}
	virtual ~MMSBphiDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDIP(param);}
};
class MMSBphiDOC : public LABlackScholesBase
{
public :
	MMSBphiDOC(){}
	virtual ~MMSBphiDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDOC(param);}
};
class MMSBphiDOP : public LABlackScholesBase
{
public :
	MMSBphiDOP(){}
	virtual ~MMSBphiDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDOP(param);}
};
class MMSBphiUIC : public LABlackScholesBase
{
public :
	MMSBphiUIC(){}
	virtual ~MMSBphiUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUIC(param);}
};
class MMSBphiUIP : public LABlackScholesBase
{
public :
	MMSBphiUIP(){}
	virtual ~MMSBphiUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUIP(param);}
};
class MMSBphiUOC : public LABlackScholesBase
{
public :
	MMSBphiUOC(){}
	virtual ~MMSBphiUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUOC(param);}
};
class MMSBphiUOP : public LABlackScholesBase
{
public :
	MMSBphiUOP(){}
	virtual ~MMSBphiUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUOP(param);}
};

//MMDB
class MMDBpremKOC : public LABlackScholesBase
{
public :
	MMDBpremKOC(){}
	virtual ~MMDBpremKOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKOC(param);}
};
class MMDBpremKIC : public LABlackScholesBase
{
public :
	MMDBpremKIC(){}
	virtual ~MMDBpremKIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKIC(param);}
};
class MMDBpremKOP : public LABlackScholesBase
{
public :
	MMDBpremKOP(){}
	virtual ~MMDBpremKOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKOP(param);}
};
class MMDBpremKIP : public LABlackScholesBase
{
public :
	MMDBpremKIP(){}
	virtual ~MMDBpremKIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKIP(param);}
};
class MMDBpremRL : public LABlackScholesBase
{
public :
	MMDBpremRL(){}
	virtual ~MMDBpremRL(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRL(param);}
};
class MMDBpremRH : public LABlackScholesBase
{
public :
	MMDBpremRH(){} 
	virtual ~MMDBpremRH(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRH(param);}
};
class MMDBpremRI : public LABlackScholesBase
{
public :
	MMDBpremRI(){}
	virtual ~MMDBpremRI(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRI(param);}
};

class MMFDdeltaCallSpot : public LABlackScholesBase
{
public :
	MMFDdeltaCallSpot(){}
	virtual ~MMFDdeltaCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaCallSpot(param);}
};
class MMFDdeltaPutSpot : public LABlackScholesBase
{
public :
	MMFDdeltaPutSpot(){}
	virtual ~MMFDdeltaPutSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaPutSpot(param);}
};
class MMFD1stDiffCallSpot : public LABlackScholesBase
{
public :
	MMFD1stDiffCallSpot(){}
	virtual ~MMFD1stDiffCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffCallSpot(param);}
};
class MMFD1stDiffPutSpot : public LABlackScholesBase
{
public :
	MMFD1stDiffPutSpot(){}
	virtual ~MMFD1stDiffPutSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffPutSpot(param);}
};
class MMFDdeltaCallFwd : public LABlackScholesBase
{
public :
	MMFDdeltaCallFwd(){}
	virtual ~MMFDdeltaCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaCallFwd(param);}
};
class MMFDdeltaPutFwd : public LABlackScholesBase
{
public :
	MMFDdeltaPutFwd(){}
	virtual ~MMFDdeltaPutFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaPutFwd(param);}
};
class MMFD1stDiffCallFwd : public LABlackScholesBase
{
public :
	MMFD1stDiffCallFwd(){}
	virtual ~MMFD1stDiffCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffCallFwd(param);}
};
class MMFD1stDiffPutFwd : public LABlackScholesBase
{
public :
	MMFD1stDiffPutFwd(){}
	virtual ~MMFD1stDiffPutFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffPutFwd(param);}
};
class MMFD2ndDiffCallFwd : public LABlackScholesBase
{
public :
	MMFD2ndDiffCallFwd(){}
	virtual ~MMFD2ndDiffCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD2ndDiffCallFwd(param);}
};
class MMFD2ndDiffCallSpot : public LABlackScholesBase
{
public :
	MMFD2ndDiffCallSpot(){}
	virtual ~MMFD2ndDiffCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD2ndDiffCallSpot(param);}
};
//MMAFF
class MMAFFpremCall : public LABlackScholesBase
{
public :
	MMAFFpremCall(){}
	virtual ~MMAFFpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AFFpremCall(param);}
};
class MMAFFpremPut : public LABlackScholesBase
{
public :
	MMAFFpremPut(){}
	virtual ~MMAFFpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AFFpremPut(param);}
};
