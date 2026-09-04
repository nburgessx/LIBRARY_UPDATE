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
class AQLZeroForError :public AQLBlackScholesBase
{
	public :
	AQLZeroForError(){}
	virtual ~AQLZeroForError(){}
	double calc(AnalyticParam& param){(void)param; return 0.0;}
};



//AQLBlackPayOff
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


//AQLBlack
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
class AQLCFpremCall : public AQLBlackScholesBase
{
public :
	AQLCFpremCall(){}
	virtual ~AQLCFpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AQLCFpremCall(param);}
};
class AQLCFvegaCall : public AQLBlackScholesBase
{
public :
	AQLCFvegaCall(){}
	virtual ~AQLCFvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AQLCFvegaCall(param);}
};
class AQLCFpremPut : public AQLBlackScholesBase
{
public :
	AQLCFpremPut(){}
	virtual ~AQLCFpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AQLCFpremPut(param);}
};class AQLCFvegaPut : public AQLBlackScholesBase
{
public :
	AQLCFvegaPut(){}
	virtual ~AQLCFvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AQLCFvegaPut(param);}
};
//AQLGreek
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
class AQLDGpremCall : public AQLBlackScholesBase
{
public :
	AQLDGpremCall(){}
	virtual ~AQLDGpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGpremCall(param);}
};
class AQLDGdeltaCall : public AQLBlackScholesBase
{
public :
	AQLDGdeltaCall(){}
	virtual ~AQLDGdeltaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGdeltaCall(param);}
};
class AQLDGgammaCall : public AQLBlackScholesBase
{
public :
	AQLDGgammaCall(){}
	virtual ~AQLDGgammaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGgammaCall(param);}
};
class AQLDGvegaCall : public AQLBlackScholesBase
{
public :
	AQLDGvegaCall(){}
	virtual ~AQLDGvegaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGvegaCall(param);}
};
class AQLDGthetaCall : public AQLBlackScholesBase
{
public :
	AQLDGthetaCall(){}
	virtual ~AQLDGthetaCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGthetaCall(param);}
};
class AQLDGrhoCall : public AQLBlackScholesBase
{
public :
	AQLDGrhoCall(){}
	virtual ~AQLDGrhoCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGrhoCall(param);}
};
class AQLDGphiCall : public AQLBlackScholesBase
{
public :
	AQLDGphiCall(){}
	virtual ~AQLDGphiCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGphiCall(param);}
};

class AQLDGpremPut : public AQLBlackScholesBase
{
public :
	AQLDGpremPut(){}
	virtual ~AQLDGpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGpremPut(param);}
};
class AQLDGdeltaPut : public AQLBlackScholesBase
{
public :
	AQLDGdeltaPut(){}
	virtual ~AQLDGdeltaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGdeltaPut(param);}
};
class AQLDGgammaPut : public AQLBlackScholesBase
{
public :
	AQLDGgammaPut(){}
	virtual ~AQLDGgammaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGgammaPut(param);}
};
class AQLDGvegaPut : public AQLBlackScholesBase
{
public :
	AQLDGvegaPut(){}
	virtual ~AQLDGvegaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGvegaPut(param);}
};
class AQLDGthetaPut : public AQLBlackScholesBase
{
public :
	AQLDGthetaPut(){}
	virtual ~AQLDGthetaPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGthetaPut(param);}
};
class AQLDGrhoPut : public AQLBlackScholesBase
{
public :
	AQLDGrhoPut(){}
	virtual ~AQLDGrhoPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGrhoPut(param);}
};
class AQLDGphiPut : public AQLBlackScholesBase
{
public :
	AQLDGphiPut(){}
	virtual ~AQLDGphiPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DGphiPut(param);}
};

//hishida vannavolga

class AQLSBprobUNT : public AQLBlackScholesBase
{
public :
	AQLSBprobUNT(){}
	virtual ~AQLSBprobUNT(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AQLSBprobUNT(param);}
};
class AQLSBprobDNT : public AQLBlackScholesBase
{
public :
	AQLSBprobDNT(){}
	virtual ~AQLSBprobDNT(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AQLSBprobDNT(param);}
};
//hishida vannavolga


//MMSB
class AQLSBpremDIC : public AQLBlackScholesBase
{
public :
	AQLSBpremDIC(){}
	virtual ~AQLSBpremDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDIC(param);}
};
class AQLSBpremDIP : public AQLBlackScholesBase
{
public :
	AQLSBpremDIP(){}
	virtual ~AQLSBpremDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDIP(param);}
};
class AQLSBpremDOC : public AQLBlackScholesBase
{
public :
	AQLSBpremDOC(){}
	virtual ~AQLSBpremDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDOC(param);}
};
class AQLSBpremDOP : public AQLBlackScholesBase
{
public :
	AQLSBpremDOP(){}
	virtual ~AQLSBpremDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremDOP(param);}
};
class AQLSBpremUIC : public AQLBlackScholesBase
{
public :
	AQLSBpremUIC(){}
	virtual ~AQLSBpremUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUIC(param);}
};
class AQLSBpremUIP : public AQLBlackScholesBase
{
public :
	AQLSBpremUIP(){}
	virtual ~AQLSBpremUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUIP(param);}
};
class AQLSBpremUOC : public AQLBlackScholesBase
{
public :
	AQLSBpremUOC(){}
	virtual ~AQLSBpremUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUOC(param);}
};
class AQLSBpremUOP : public AQLBlackScholesBase
{
public :
	AQLSBpremUOP(){}
	virtual ~AQLSBpremUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBpremUOP(param);}
};

class AQLSBdeltaDIC : public AQLBlackScholesBase
{
public :
	AQLSBdeltaDIC(){}
	virtual ~AQLSBdeltaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDIC(param);}
};
class AQLSBdeltaDIP : public AQLBlackScholesBase
{
public :
	AQLSBdeltaDIP(){}
	virtual ~AQLSBdeltaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDIP(param);}
};
class AQLSBdeltaDOC : public AQLBlackScholesBase
{
public :
	AQLSBdeltaDOC(){}
	virtual ~AQLSBdeltaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDOC(param);}
};
class AQLSBdeltaDOP : public AQLBlackScholesBase
{
public :
	AQLSBdeltaDOP(){}
	virtual ~AQLSBdeltaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaDOP(param);}
};
class AQLSBdeltaUIC : public AQLBlackScholesBase
{
public :
	AQLSBdeltaUIC(){}
	virtual ~AQLSBdeltaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUIC(param);}
};
class AQLSBdeltaUIP : public AQLBlackScholesBase
{
public :
	AQLSBdeltaUIP(){}
	virtual ~AQLSBdeltaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUIP(param);}
};
class AQLSBdeltaUOC : public AQLBlackScholesBase
{
public :
	AQLSBdeltaUOC(){}
	virtual ~AQLSBdeltaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUOC(param);}
};
class AQLSBdeltaUOP : public AQLBlackScholesBase
{
public :
	AQLSBdeltaUOP(){}
	virtual ~AQLSBdeltaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBdeltaUOP(param);}
};
class AQLSBgammaDIC : public AQLBlackScholesBase
{
public :
	AQLSBgammaDIC(){}
	virtual ~AQLSBgammaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDIC(param);}
};
class AQLSBgammaDIP : public AQLBlackScholesBase
{
public :
	AQLSBgammaDIP(){}
	virtual ~AQLSBgammaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDIP(param);}
};
class AQLSBgammaDOC : public AQLBlackScholesBase
{
public :
	AQLSBgammaDOC(){}
	virtual ~AQLSBgammaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDOC(param);}
};
class AQLSBgammaDOP : public AQLBlackScholesBase
{
public :
	AQLSBgammaDOP(){}
	virtual ~AQLSBgammaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaDOP(param);}
};
class AQLSBgammaUIC : public AQLBlackScholesBase
{
public :
	AQLSBgammaUIC(){}
	virtual ~AQLSBgammaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUIC(param);}
};
class AQLSBgammaUIP : public AQLBlackScholesBase
{
public :
	AQLSBgammaUIP(){}
	virtual ~AQLSBgammaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUIP(param);}
};
class AQLSBgammaUOC : public AQLBlackScholesBase
{
public :
	AQLSBgammaUOC(){}
	virtual ~AQLSBgammaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUOC(param);}
};
class AQLSBgammaUOP : public AQLBlackScholesBase
{
public :
	AQLSBgammaUOP(){}
	virtual ~AQLSBgammaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBgammaUOP(param);}
};

class AQLSBvegaDIC : public AQLBlackScholesBase
{
public :
	AQLSBvegaDIC(){}
	virtual ~AQLSBvegaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDIC(param);}
};
class AQLSBvegaDIP : public AQLBlackScholesBase
{
public :
	AQLSBvegaDIP(){}
	virtual ~AQLSBvegaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDIP(param);}
};
class AQLSBvegaDOC : public AQLBlackScholesBase
{
public :
	AQLSBvegaDOC(){}
	virtual ~AQLSBvegaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDOC(param);}
};
class AQLSBvegaDOP : public AQLBlackScholesBase
{
public :
	AQLSBvegaDOP(){}
	virtual ~AQLSBvegaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaDOP(param);}
};
class AQLSBvegaUIC : public AQLBlackScholesBase
{
public :
	AQLSBvegaUIC(){}
	virtual ~AQLSBvegaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUIC(param);}
};
class AQLSBvegaUIP : public AQLBlackScholesBase
{
public :
	AQLSBvegaUIP(){}
	virtual ~AQLSBvegaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUIP(param);}
};
class AQLSBvegaUOC : public AQLBlackScholesBase
{
public :
	AQLSBvegaUOC(){}
	virtual ~AQLSBvegaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUOC(param);}
};
class AQLSBvegaUOP : public AQLBlackScholesBase
{
public :
	AQLSBvegaUOP(){}
	virtual ~AQLSBvegaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBvegaUOP(param);}
};

class AQLSBthetaDIC : public AQLBlackScholesBase
{
public :
	AQLSBthetaDIC(){}
	virtual ~AQLSBthetaDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDIC(param);}
};
class AQLSBthetaDIP : public AQLBlackScholesBase
{
public :
	AQLSBthetaDIP(){}
	virtual ~AQLSBthetaDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDIP(param);}
};
class AQLSBthetaDOC : public AQLBlackScholesBase
{
public :
	AQLSBthetaDOC(){}
	virtual ~AQLSBthetaDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDOC(param);}
};
class AQLSBthetaDOP : public AQLBlackScholesBase
{
public :
	AQLSBthetaDOP(){}
	virtual ~AQLSBthetaDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaDOP(param);}
};
class AQLSBthetaUIC : public AQLBlackScholesBase
{
public :
	AQLSBthetaUIC(){}
	virtual ~AQLSBthetaUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUIC(param);}
};
class AQLSBthetaUIP : public AQLBlackScholesBase
{
public :
	AQLSBthetaUIP(){}
	virtual ~AQLSBthetaUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUIP(param);}
};
class AQLSBthetaUOC : public AQLBlackScholesBase
{
public :
	AQLSBthetaUOC(){}
	virtual ~AQLSBthetaUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUOC(param);}
};
class AQLSBthetaUOP : public AQLBlackScholesBase
{
public :
	AQLSBthetaUOP(){}
	virtual ~AQLSBthetaUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBthetaUOP(param);}
};

class AQLSBrhoDIC : public AQLBlackScholesBase
{
public :
	AQLSBrhoDIC(){}
	virtual ~AQLSBrhoDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDIC(param);}
};
class AQLSBrhoDIP : public AQLBlackScholesBase
{
public :
	AQLSBrhoDIP(){}
	virtual ~AQLSBrhoDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDIP(param);}
};
class AQLSBrhoDOC : public AQLBlackScholesBase
{
public :
	AQLSBrhoDOC(){}
	virtual ~AQLSBrhoDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDOC(param);}
};
class AQLSBrhoDOP : public AQLBlackScholesBase
{
public :
	AQLSBrhoDOP(){}
	virtual ~AQLSBrhoDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoDOP(param);}
};
class AQLSBrhoUIC : public AQLBlackScholesBase
{
public :
	AQLSBrhoUIC(){}
	virtual ~AQLSBrhoUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUIC(param);}
};
class AQLSBrhoUIP : public AQLBlackScholesBase
{
public :
	AQLSBrhoUIP(){}
	virtual ~AQLSBrhoUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUIP(param);}
};
class AQLSBrhoUOC : public AQLBlackScholesBase
{
public :
	AQLSBrhoUOC(){}
	virtual ~AQLSBrhoUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUOC(param);}
};
class AQLSBrhoUOP : public AQLBlackScholesBase
{
public :
	AQLSBrhoUOP(){}
	virtual ~AQLSBrhoUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBrhoUOP(param);}
};

class AQLSBphiDIC : public AQLBlackScholesBase
{
public :
	AQLSBphiDIC(){}
	virtual ~AQLSBphiDIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDIC(param);}
};
class AQLSBphiDIP : public AQLBlackScholesBase
{
public :
	AQLSBphiDIP(){}
	virtual ~AQLSBphiDIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDIP(param);}
};
class AQLSBphiDOC : public AQLBlackScholesBase
{
public :
	AQLSBphiDOC(){}
	virtual ~AQLSBphiDOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDOC(param);}
};
class AQLSBphiDOP : public AQLBlackScholesBase
{
public :
	AQLSBphiDOP(){}
	virtual ~AQLSBphiDOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiDOP(param);}
};
class AQLSBphiUIC : public AQLBlackScholesBase
{
public :
	AQLSBphiUIC(){}
	virtual ~AQLSBphiUIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUIC(param);}
};
class AQLSBphiUIP : public AQLBlackScholesBase
{
public :
	AQLSBphiUIP(){}
	virtual ~AQLSBphiUIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUIP(param);}
};
class AQLSBphiUOC : public AQLBlackScholesBase
{
public :
	AQLSBphiUOC(){}
	virtual ~AQLSBphiUOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUOC(param);}
};
class AQLSBphiUOP : public AQLBlackScholesBase
{
public :
	AQLSBphiUOP(){}
	virtual ~AQLSBphiUOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::SBphiUOP(param);}
};

//MMDB
class AQLDBpremKOC : public AQLBlackScholesBase
{
public :
	AQLDBpremKOC(){}
	virtual ~AQLDBpremKOC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKOC(param);}
};
class AQLDBpremKIC : public AQLBlackScholesBase
{
public :
	AQLDBpremKIC(){}
	virtual ~AQLDBpremKIC(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKIC(param);}
};
class AQLDBpremKOP : public AQLBlackScholesBase
{
public :
	AQLDBpremKOP(){}
	virtual ~AQLDBpremKOP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKOP(param);}
};
class AQLDBpremKIP : public AQLBlackScholesBase
{
public :
	AQLDBpremKIP(){}
	virtual ~AQLDBpremKIP(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremKIP(param);}
};
class AQLDBpremRL : public AQLBlackScholesBase
{
public :
	AQLDBpremRL(){}
	virtual ~AQLDBpremRL(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRL(param);}
};
class AQLDBpremRH : public AQLBlackScholesBase
{
public :
	AQLDBpremRH(){} 
	virtual ~AQLDBpremRH(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRH(param);}
};
class AQLDBpremRI : public AQLBlackScholesBase
{
public :
	AQLDBpremRI(){}
	virtual ~AQLDBpremRI(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::DBpremRI(param);}
};

class AQLFDdeltaCallSpot : public AQLBlackScholesBase
{
public :
	AQLFDdeltaCallSpot(){}
	virtual ~AQLFDdeltaCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaCallSpot(param);}
};
class AQLFDdeltaPutSpot : public AQLBlackScholesBase
{
public :
	AQLFDdeltaPutSpot(){}
	virtual ~AQLFDdeltaPutSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaPutSpot(param);}
};
class AQLFD1stDiffCallSpot : public AQLBlackScholesBase
{
public :
	AQLFD1stDiffCallSpot(){}
	virtual ~AQLFD1stDiffCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffCallSpot(param);}
};
class AQLFD1stDiffPutSpot : public AQLBlackScholesBase
{
public :
	AQLFD1stDiffPutSpot(){}
	virtual ~AQLFD1stDiffPutSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffPutSpot(param);}
};
class AQLFDdeltaCallFwd : public AQLBlackScholesBase
{
public :
	AQLFDdeltaCallFwd(){}
	virtual ~AQLFDdeltaCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaCallFwd(param);}
};
class AQLFDdeltaPutFwd : public AQLBlackScholesBase
{
public :
	AQLFDdeltaPutFwd(){}
	virtual ~AQLFDdeltaPutFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FDdeltaPutFwd(param);}
};
class AQLFD1stDiffCallFwd : public AQLBlackScholesBase
{
public :
	AQLFD1stDiffCallFwd(){}
	virtual ~AQLFD1stDiffCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffCallFwd(param);}
};
class AQLFD1stDiffPutFwd : public AQLBlackScholesBase
{
public :
	AQLFD1stDiffPutFwd(){}
	virtual ~AQLFD1stDiffPutFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD1stDiffPutFwd(param);}
};
class AQLFD2ndDiffCallFwd : public AQLBlackScholesBase
{
public :
	AQLFD2ndDiffCallFwd(){}
	virtual ~AQLFD2ndDiffCallFwd(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD2ndDiffCallFwd(param);}
};
class AQLFD2ndDiffCallSpot : public AQLBlackScholesBase
{
public :
	AQLFD2ndDiffCallSpot(){}
	virtual ~AQLFD2ndDiffCallSpot(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::FD2ndDiffCallSpot(param);}
};
//MMAFF
class AQLAFFpremCall : public AQLBlackScholesBase
{
public :
	AQLAFFpremCall(){}
	virtual ~AQLAFFpremCall(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AFFpremCall(param);}
};
class AQLAFFpremPut : public AQLBlackScholesBase
{
public :
	AQLAFFpremPut(){}
	virtual ~AQLAFFpremPut(){}
	double calc(AnalyticParam& param){ return AnalyticFormulae::AFFpremPut(param);}
};
