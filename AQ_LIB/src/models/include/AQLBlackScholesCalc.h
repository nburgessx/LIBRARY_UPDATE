#pragma once

#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLCoreAppError.h"
#include <cmath>
#include "AQLAnalyticFormula.h"

class AQLBlackScholesBase
{
public :
	AQLBlackScholesBase(){}
	virtual ~AQLBlackScholesBase(){}
	virtual double calc(AnalyticParam& param)=0;
};
//0Value
class MMZeroForError :public AQLBlackScholesBase
{
	public :
	MMZeroForError(){}
	virtual ~MMZeroForError(){}
	double calc(AnalyticParam& param){(void)param; return 0.0;}
};



//LABlackPayOff
class AQLBlackPayOffpremCall : public AQLBlackScholesBase
{
public :
	AQLBlackPayOffpremCall(){}
	virtual ~AQLBlackPayOffpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKPayOffpremCall(param);}
};
class AQLBlackPayOffpremPut : public AQLBlackScholesBase
{
public :
	AQLBlackPayOffpremPut(){}
	virtual ~AQLBlackPayOffpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKPayOffpremPut(param);}
};


//LABlack
class AQLBlackpremCall : public AQLBlackScholesBase
{
public :
	AQLBlackpremCall(){}
	virtual ~AQLBlackpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKpremCall(param);}
};
class AQLBlackpremPut : public AQLBlackScholesBase
{
public :
	AQLBlackpremPut(){}
	virtual ~AQLBlackpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKpremPut(param);}
};
class AQLBlackvegaCall : public AQLBlackScholesBase
{
public :
	AQLBlackvegaCall(){}
	virtual ~AQLBlackvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKvegaCall(param);}
};
class AQLBlackvegaPut : public AQLBlackScholesBase
{
public :
	AQLBlackvegaPut(){}
	virtual ~AQLBlackvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKvegaPut(param);}
};
class AQLBlackdeltaCall : public AQLBlackScholesBase
{
public :
	AQLBlackdeltaCall(){}
	virtual ~AQLBlackdeltaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKdeltaCall(param);}
};
class AQLBlackdeltaPut : public AQLBlackScholesBase
{
public :
	AQLBlackdeltaPut(){}
	virtual ~AQLBlackdeltaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKdeltaPut(param);}
};

class AQLBlackgammaCall : public AQLBlackScholesBase
{
public :
	AQLBlackgammaCall(){}
	virtual ~AQLBlackgammaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKgammaCall(param);}
};
class AQLBlackgammaPut : public AQLBlackScholesBase
{
public :
	AQLBlackgammaPut(){}
	virtual ~AQLBlackgammaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKgammaPut(param);}
};
class AQLBlackthetaCall : public AQLBlackScholesBase
{
public :
	AQLBlackthetaCall(){}
	virtual ~AQLBlackthetaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKthetaCall(param);}
};
class AQLBlackthetaPut : public AQLBlackScholesBase
{
public :
	AQLBlackthetaPut(){}
	virtual ~AQLBlackthetaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::BKthetaPut(param);}
};

//MMCF
class MMCFpremCall : public AQLBlackScholesBase
{
public :
	MMCFpremCall(){}
	virtual ~MMCFpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFpremCall(param);}
};
class MMCFvegaCall : public AQLBlackScholesBase
{
public :
	MMCFvegaCall(){}
	virtual ~MMCFvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFvegaCall(param);}
};
class MMCFpremPut : public AQLBlackScholesBase
{
public :
	MMCFpremPut(){}
	virtual ~MMCFpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFpremPut(param);}
};class MMCFvegaPut : public AQLBlackScholesBase
{
public :
	MMCFvegaPut(){}
	virtual ~MMCFvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMCFvegaPut(param);}
};
//LAGreek
class AQLGreekpremCall : public AQLBlackScholesBase
{
public :
	AQLGreekpremCall(){}
	virtual ~AQLGreekpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKpremCall(param);}
};
class AQLGreekdeltaCall : public AQLBlackScholesBase
{
public :
	AQLGreekdeltaCall(){}
	virtual ~AQLGreekdeltaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKdeltaCall(param);}
};
class AQLGreekgammaCall : public AQLBlackScholesBase
{
public :
	AQLGreekgammaCall(){}
	virtual ~AQLGreekgammaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKgammaCall(param);}
};
class AQLGreekvegaCall : public AQLBlackScholesBase
{
public :
	AQLGreekvegaCall(){}
	virtual ~AQLGreekvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvegaCall(param);}
};
class AQLGreekthetaCall : public AQLBlackScholesBase
{
public :
	AQLGreekthetaCall(){}
	virtual ~AQLGreekthetaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKthetaCall(param);}
};
class AQLGreekrhoCall : public AQLBlackScholesBase
{
public :
	AQLGreekrhoCall(){}
	virtual ~AQLGreekrhoCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKrhoCall(param);}
};
class AQLGreekphiCall : public AQLBlackScholesBase
{
public :
	AQLGreekphiCall(){}
	virtual ~AQLGreekphiCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKphiCall(param);}
};
class AQLGreekvannaCall : public AQLBlackScholesBase
{
public :
	AQLGreekvannaCall(){}
	virtual ~AQLGreekvannaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvannaCall(param);}
};
class AQLGreekvolgaCall : public AQLBlackScholesBase
{
public :
	AQLGreekvolgaCall(){}
	virtual ~AQLGreekvolgaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvolgaCall(param);}
};
class AQLGreekpremPut : public AQLBlackScholesBase
{
public :
	AQLGreekpremPut(){}
	virtual ~AQLGreekpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKpremPut(param);}
};
class AQLGreekdeltaPut : public AQLBlackScholesBase
{
public :
	AQLGreekdeltaPut(){}
	virtual ~AQLGreekdeltaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKdeltaPut(param);}
};
class AQLGreekgammaPut : public AQLBlackScholesBase
{
public :
	AQLGreekgammaPut(){}
	virtual ~AQLGreekgammaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKgammaPut(param);}
};
class AQLGreekvegaPut : public AQLBlackScholesBase
{
public :
	AQLGreekvegaPut(){}
	virtual ~AQLGreekvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvegaPut(param);}
};
class AQLGreekthetaPut : public AQLBlackScholesBase
{
public :
	AQLGreekthetaPut(){}
	virtual ~AQLGreekthetaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKthetaPut(param);}
};
class AQLGreekrhoPut : public AQLBlackScholesBase
{
public :
	AQLGreekrhoPut(){}
	virtual ~AQLGreekrhoPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKrhoPut(param);}
};
class AQLGreekphiPut : public AQLBlackScholesBase
{
public :
	AQLGreekphiPut(){}
	virtual ~AQLGreekphiPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKphiPut(param);}
};
class AQLGreekvannaPut : public AQLBlackScholesBase
{
public :
	AQLGreekvannaPut(){}
	virtual ~AQLGreekvannaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvannaPut(param);}
};
class AQLGreekvolgaPut : public AQLBlackScholesBase
{
public :
	AQLGreekvolgaPut(){}
	virtual ~AQLGreekvolgaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::GKvolgaPut(param);}
};
//MMDG
class MMDGpremCall : public AQLBlackScholesBase
{
public :
	MMDGpremCall(){}
	virtual ~MMDGpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGpremCall(param);}
};
class MMDGdeltaCall : public AQLBlackScholesBase
{
public :
	MMDGdeltaCall(){}
	virtual ~MMDGdeltaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGdeltaCall(param);}
};
class MMDGgammaCall : public AQLBlackScholesBase
{
public :
	MMDGgammaCall(){}
	virtual ~MMDGgammaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGgammaCall(param);}
};
class MMDGvegaCall : public AQLBlackScholesBase
{
public :
	MMDGvegaCall(){}
	virtual ~MMDGvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGvegaCall(param);}
};
class MMDGthetaCall : public AQLBlackScholesBase
{
public :
	MMDGthetaCall(){}
	virtual ~MMDGthetaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGthetaCall(param);}
};
class MMDGrhoCall : public AQLBlackScholesBase
{
public :
	MMDGrhoCall(){}
	virtual ~MMDGrhoCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGrhoCall(param);}
};
class MMDGphiCall : public AQLBlackScholesBase
{
public :
	MMDGphiCall(){}
	virtual ~MMDGphiCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGphiCall(param);}
};

class MMDGpremPut : public AQLBlackScholesBase
{
public :
	MMDGpremPut(){}
	virtual ~MMDGpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGpremPut(param);}
};
class MMDGdeltaPut : public AQLBlackScholesBase
{
public :
	MMDGdeltaPut(){}
	virtual ~MMDGdeltaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGdeltaPut(param);}
};
class MMDGgammaPut : public AQLBlackScholesBase
{
public :
	MMDGgammaPut(){}
	virtual ~MMDGgammaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGgammaPut(param);}
};
class MMDGvegaPut : public AQLBlackScholesBase
{
public :
	MMDGvegaPut(){}
	virtual ~MMDGvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGvegaPut(param);}
};
class MMDGthetaPut : public AQLBlackScholesBase
{
public :
	MMDGthetaPut(){}
	virtual ~MMDGthetaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGthetaPut(param);}
};
class MMDGrhoPut : public AQLBlackScholesBase
{
public :
	MMDGrhoPut(){}
	virtual ~MMDGrhoPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGrhoPut(param);}
};
class MMDGphiPut : public AQLBlackScholesBase
{
public :
	MMDGphiPut(){}
	virtual ~MMDGphiPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGphiPut(param);}
};

//hishida vannavolga

class MMSBprobUNT : public AQLBlackScholesBase
{
public :
	MMSBprobUNT(){}
	virtual ~MMSBprobUNT(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMSBprobUNT(param);}
};
class MMSBprobDNT : public AQLBlackScholesBase
{
public :
	MMSBprobDNT(){}
	virtual ~MMSBprobDNT(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::MMSBprobDNT(param);}
};
//hishida vannavolga


//MMSB
class MMSBpremDIC : public AQLBlackScholesBase
{
public :
	MMSBpremDIC(){}
	virtual ~MMSBpremDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDIC(param);}
};
class MMSBpremDIP : public AQLBlackScholesBase
{
public :
	MMSBpremDIP(){}
	virtual ~MMSBpremDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDIP(param);}
};
class MMSBpremDOC : public AQLBlackScholesBase
{
public :
	MMSBpremDOC(){}
	virtual ~MMSBpremDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDOC(param);}
};
class MMSBpremDOP : public AQLBlackScholesBase
{
public :
	MMSBpremDOP(){}
	virtual ~MMSBpremDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDOP(param);}
};
class MMSBpremUIC : public AQLBlackScholesBase
{
public :
	MMSBpremUIC(){}
	virtual ~MMSBpremUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUIC(param);}
};
class MMSBpremUIP : public AQLBlackScholesBase
{
public :
	MMSBpremUIP(){}
	virtual ~MMSBpremUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUIP(param);}
};
class MMSBpremUOC : public AQLBlackScholesBase
{
public :
	MMSBpremUOC(){}
	virtual ~MMSBpremUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUOC(param);}
};
class MMSBpremUOP : public AQLBlackScholesBase
{
public :
	MMSBpremUOP(){}
	virtual ~MMSBpremUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUOP(param);}
};

class MMSBdeltaDIC : public AQLBlackScholesBase
{
public :
	MMSBdeltaDIC(){}
	virtual ~MMSBdeltaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDIC(param);}
};
class MMSBdeltaDIP : public AQLBlackScholesBase
{
public :
	MMSBdeltaDIP(){}
	virtual ~MMSBdeltaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDIP(param);}
};
class MMSBdeltaDOC : public AQLBlackScholesBase
{
public :
	MMSBdeltaDOC(){}
	virtual ~MMSBdeltaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDOC(param);}
};
class MMSBdeltaDOP : public AQLBlackScholesBase
{
public :
	MMSBdeltaDOP(){}
	virtual ~MMSBdeltaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDOP(param);}
};
class MMSBdeltaUIC : public AQLBlackScholesBase
{
public :
	MMSBdeltaUIC(){}
	virtual ~MMSBdeltaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUIC(param);}
};
class MMSBdeltaUIP : public AQLBlackScholesBase
{
public :
	MMSBdeltaUIP(){}
	virtual ~MMSBdeltaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUIP(param);}
};
class MMSBdeltaUOC : public AQLBlackScholesBase
{
public :
	MMSBdeltaUOC(){}
	virtual ~MMSBdeltaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUOC(param);}
};
class MMSBdeltaUOP : public AQLBlackScholesBase
{
public :
	MMSBdeltaUOP(){}
	virtual ~MMSBdeltaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUOP(param);}
};
class MMSBgammaDIC : public AQLBlackScholesBase
{
public :
	MMSBgammaDIC(){}
	virtual ~MMSBgammaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDIC(param);}
};
class MMSBgammaDIP : public AQLBlackScholesBase
{
public :
	MMSBgammaDIP(){}
	virtual ~MMSBgammaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDIP(param);}
};
class MMSBgammaDOC : public AQLBlackScholesBase
{
public :
	MMSBgammaDOC(){}
	virtual ~MMSBgammaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDOC(param);}
};
class MMSBgammaDOP : public AQLBlackScholesBase
{
public :
	MMSBgammaDOP(){}
	virtual ~MMSBgammaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDOP(param);}
};
class MMSBgammaUIC : public AQLBlackScholesBase
{
public :
	MMSBgammaUIC(){}
	virtual ~MMSBgammaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUIC(param);}
};
class MMSBgammaUIP : public AQLBlackScholesBase
{
public :
	MMSBgammaUIP(){}
	virtual ~MMSBgammaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUIP(param);}
};
class MMSBgammaUOC : public AQLBlackScholesBase
{
public :
	MMSBgammaUOC(){}
	virtual ~MMSBgammaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUOC(param);}
};
class MMSBgammaUOP : public AQLBlackScholesBase
{
public :
	MMSBgammaUOP(){}
	virtual ~MMSBgammaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUOP(param);}
};

class MMSBvegaDIC : public AQLBlackScholesBase
{
public :
	MMSBvegaDIC(){}
	virtual ~MMSBvegaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDIC(param);}
};
class MMSBvegaDIP : public AQLBlackScholesBase
{
public :
	MMSBvegaDIP(){}
	virtual ~MMSBvegaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDIP(param);}
};
class MMSBvegaDOC : public AQLBlackScholesBase
{
public :
	MMSBvegaDOC(){}
	virtual ~MMSBvegaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDOC(param);}
};
class MMSBvegaDOP : public AQLBlackScholesBase
{
public :
	MMSBvegaDOP(){}
	virtual ~MMSBvegaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDOP(param);}
};
class MMSBvegaUIC : public AQLBlackScholesBase
{
public :
	MMSBvegaUIC(){}
	virtual ~MMSBvegaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUIC(param);}
};
class MMSBvegaUIP : public AQLBlackScholesBase
{
public :
	MMSBvegaUIP(){}
	virtual ~MMSBvegaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUIP(param);}
};
class MMSBvegaUOC : public AQLBlackScholesBase
{
public :
	MMSBvegaUOC(){}
	virtual ~MMSBvegaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUOC(param);}
};
class MMSBvegaUOP : public AQLBlackScholesBase
{
public :
	MMSBvegaUOP(){}
	virtual ~MMSBvegaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUOP(param);}
};

class MMSBthetaDIC : public AQLBlackScholesBase
{
public :
	MMSBthetaDIC(){}
	virtual ~MMSBthetaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDIC(param);}
};
class MMSBthetaDIP : public AQLBlackScholesBase
{
public :
	MMSBthetaDIP(){}
	virtual ~MMSBthetaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDIP(param);}
};
class MMSBthetaDOC : public AQLBlackScholesBase
{
public :
	MMSBthetaDOC(){}
	virtual ~MMSBthetaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDOC(param);}
};
class MMSBthetaDOP : public AQLBlackScholesBase
{
public :
	MMSBthetaDOP(){}
	virtual ~MMSBthetaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDOP(param);}
};
class MMSBthetaUIC : public AQLBlackScholesBase
{
public :
	MMSBthetaUIC(){}
	virtual ~MMSBthetaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUIC(param);}
};
class MMSBthetaUIP : public AQLBlackScholesBase
{
public :
	MMSBthetaUIP(){}
	virtual ~MMSBthetaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUIP(param);}
};
class MMSBthetaUOC : public AQLBlackScholesBase
{
public :
	MMSBthetaUOC(){}
	virtual ~MMSBthetaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUOC(param);}
};
class MMSBthetaUOP : public AQLBlackScholesBase
{
public :
	MMSBthetaUOP(){}
	virtual ~MMSBthetaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUOP(param);}
};

class MMSBrhoDIC : public AQLBlackScholesBase
{
public :
	MMSBrhoDIC(){}
	virtual ~MMSBrhoDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDIC(param);}
};
class MMSBrhoDIP : public AQLBlackScholesBase
{
public :
	MMSBrhoDIP(){}
	virtual ~MMSBrhoDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDIP(param);}
};
class MMSBrhoDOC : public AQLBlackScholesBase
{
public :
	MMSBrhoDOC(){}
	virtual ~MMSBrhoDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDOC(param);}
};
class MMSBrhoDOP : public AQLBlackScholesBase
{
public :
	MMSBrhoDOP(){}
	virtual ~MMSBrhoDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDOP(param);}
};
class MMSBrhoUIC : public AQLBlackScholesBase
{
public :
	MMSBrhoUIC(){}
	virtual ~MMSBrhoUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUIC(param);}
};
class MMSBrhoUIP : public AQLBlackScholesBase
{
public :
	MMSBrhoUIP(){}
	virtual ~MMSBrhoUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUIP(param);}
};
class MMSBrhoUOC : public AQLBlackScholesBase
{
public :
	MMSBrhoUOC(){}
	virtual ~MMSBrhoUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUOC(param);}
};
class MMSBrhoUOP : public AQLBlackScholesBase
{
public :
	MMSBrhoUOP(){}
	virtual ~MMSBrhoUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUOP(param);}
};

class MMSBphiDIC : public AQLBlackScholesBase
{
public :
	MMSBphiDIC(){}
	virtual ~MMSBphiDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDIC(param);}
};
class MMSBphiDIP : public AQLBlackScholesBase
{
public :
	MMSBphiDIP(){}
	virtual ~MMSBphiDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDIP(param);}
};
class MMSBphiDOC : public AQLBlackScholesBase
{
public :
	MMSBphiDOC(){}
	virtual ~MMSBphiDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDOC(param);}
};
class MMSBphiDOP : public AQLBlackScholesBase
{
public :
	MMSBphiDOP(){}
	virtual ~MMSBphiDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDOP(param);}
};
class MMSBphiUIC : public AQLBlackScholesBase
{
public :
	MMSBphiUIC(){}
	virtual ~MMSBphiUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUIC(param);}
};
class MMSBphiUIP : public AQLBlackScholesBase
{
public :
	MMSBphiUIP(){}
	virtual ~MMSBphiUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUIP(param);}
};
class MMSBphiUOC : public AQLBlackScholesBase
{
public :
	MMSBphiUOC(){}
	virtual ~MMSBphiUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUOC(param);}
};
class MMSBphiUOP : public AQLBlackScholesBase
{
public :
	MMSBphiUOP(){}
	virtual ~MMSBphiUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUOP(param);}
};

//MMDB
class MMDBpremKOC : public AQLBlackScholesBase
{
public :
	MMDBpremKOC(){}
	virtual ~MMDBpremKOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKOC(param);}
};
class MMDBpremKIC : public AQLBlackScholesBase
{
public :
	MMDBpremKIC(){}
	virtual ~MMDBpremKIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKIC(param);}
};
class MMDBpremKOP : public AQLBlackScholesBase
{
public :
	MMDBpremKOP(){}
	virtual ~MMDBpremKOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKOP(param);}
};
class MMDBpremKIP : public AQLBlackScholesBase
{
public :
	MMDBpremKIP(){}
	virtual ~MMDBpremKIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKIP(param);}
};
class MMDBpremRL : public AQLBlackScholesBase
{
public :
	MMDBpremRL(){}
	virtual ~MMDBpremRL(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRL(param);}
};
class MMDBpremRH : public AQLBlackScholesBase
{
public :
	MMDBpremRH(){} 
	virtual ~MMDBpremRH(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRH(param);}
};
class MMDBpremRI : public AQLBlackScholesBase
{
public :
	MMDBpremRI(){}
	virtual ~MMDBpremRI(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRI(param);}
};

class MMFDdeltaCallSpot : public AQLBlackScholesBase
{
public :
	MMFDdeltaCallSpot(){}
	virtual ~MMFDdeltaCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaCallSpot(param);}
};
class MMFDdeltaPutSpot : public AQLBlackScholesBase
{
public :
	MMFDdeltaPutSpot(){}
	virtual ~MMFDdeltaPutSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaPutSpot(param);}
};
class MMFD1stDiffCallSpot : public AQLBlackScholesBase
{
public :
	MMFD1stDiffCallSpot(){}
	virtual ~MMFD1stDiffCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffCallSpot(param);}
};
class MMFD1stDiffPutSpot : public AQLBlackScholesBase
{
public :
	MMFD1stDiffPutSpot(){}
	virtual ~MMFD1stDiffPutSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffPutSpot(param);}
};
class MMFDdeltaCallFwd : public AQLBlackScholesBase
{
public :
	MMFDdeltaCallFwd(){}
	virtual ~MMFDdeltaCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaCallFwd(param);}
};
class MMFDdeltaPutFwd : public AQLBlackScholesBase
{
public :
	MMFDdeltaPutFwd(){}
	virtual ~MMFDdeltaPutFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaPutFwd(param);}
};
class MMFD1stDiffCallFwd : public AQLBlackScholesBase
{
public :
	MMFD1stDiffCallFwd(){}
	virtual ~MMFD1stDiffCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffCallFwd(param);}
};
class MMFD1stDiffPutFwd : public AQLBlackScholesBase
{
public :
	MMFD1stDiffPutFwd(){}
	virtual ~MMFD1stDiffPutFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffPutFwd(param);}
};
class MMFD2ndDiffCallFwd : public AQLBlackScholesBase
{
public :
	MMFD2ndDiffCallFwd(){}
	virtual ~MMFD2ndDiffCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD2ndDiffCallFwd(param);}
};
class MMFD2ndDiffCallSpot : public AQLBlackScholesBase
{
public :
	MMFD2ndDiffCallSpot(){}
	virtual ~MMFD2ndDiffCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD2ndDiffCallSpot(param);}
};
//MMAFF
class MMAFFpremCall : public AQLBlackScholesBase
{
public :
	MMAFFpremCall(){}
	virtual ~MMAFFpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AFFpremCall(param);}
};
class MMAFFpremPut : public AQLBlackScholesBase
{
public :
	MMAFFpremPut(){}
	virtual ~MMAFFpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AFFpremPut(param);}
};
