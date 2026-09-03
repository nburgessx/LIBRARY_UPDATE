#pragma once

#include "AQLMathVolatilityLMM.h"

class AQLMathVolatilityLMMContModel : public AQLMathVolatilityLMMCont
{

public:

    AQLMathVolatilityLMMContModel(const DoubleVector& paramV_,
							   const DoubleVector& paramF_,
							   const DoubleVector& G_,
							   const DoubleVector& T_fix_special_,
							   const DoubleVector& T_fix,
							   size_t              n = 20
			);

    virtual AQLMathVolatilityLMMContModel* clone() const { return new AQLMathVolatilityLMMContModel(*this); }

    virtual double get(double t, size_t i);

private:
    
    DoubleVector mParamV;
    DoubleVector mParamF;
    DoubleVector mG;
    DoubleVector mT_fix_special;
};

