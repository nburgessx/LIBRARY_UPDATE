#pragma once

#include "LAMathVolatilityLMM.h"

class LAMathVolatilityLMMContModel : public LAMathVolatilityLMMCont
{

public:

    LAMathVolatilityLMMContModel(const DoubleVector& paramV_,
							   const DoubleVector& paramF_,
							   const DoubleVector& G_,
							   const DoubleVector& T_fix_special_,
							   const DoubleVector& T_fix,
							   size_t              n = 20
			);

    virtual LAMathVolatilityLMMContModel* clone() const { return new LAMathVolatilityLMMContModel(*this); }

    virtual double get(double t, size_t i);

private:
    
    DoubleVector mParamV;
    DoubleVector mParamF;
    DoubleVector mG;
    DoubleVector mT_fix_special;
};

