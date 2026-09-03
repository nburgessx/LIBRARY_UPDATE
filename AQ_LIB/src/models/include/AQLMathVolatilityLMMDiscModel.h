#pragma once

#include "AQLMathVolatilityLMM.h"
#include "AQLPriceDataInterpolation.h"
#include <memory>

//
class AQLMathVolatilityLMMDiscModel : public AQLMathVolatilityLMMDisc
{

public:

    AQLMathVolatilityLMMDiscModel(const DoubleVector& paramV,
							   const DoubleVector& paramF,
							   const DoubleVector& tenorG,
							   const DoubleVector& G,
							   std::shared_ptr<AQLInterpolationBase> interG,
							   const DoubleVector& T_fix_special,
							   const DoubleVector& T_fix,
							   size_t num_small_step_ = 1
							  );

    AQLMathVolatilityLMMDiscModel( const AQLMathVolatilityLMMDiscModel& rhs );

	AQLMathVolatilityLMMDiscModel* clone() const { return new AQLMathVolatilityLMMDiscModel(*this); }

    void setParam(const DoubleVector& paramV,
				  const DoubleVector& paramF,
				  const DoubleVector& G
				 );

    void setParamV(const DoubleVector& paramV);

    void setParamF(const DoubleVector& paramF);

    void setParamG(const DoubleVector& G);

    DoubleVector getParamV() { return mParamV; }
    
    DoubleVector getParamF() { return mParamF; }
    
    DoubleVector getG()      { return mG;      }

	DoubleVector getDividedT_fix()  { return mDividedT_fix_special; }

    virtual double get(double t, size_t i);

private:
    
    DoubleMatrix mCacheVolMatrix;
    DoubleVector mVol_special;

    DoubleVector mT_fix_special;
    DoubleVector mDividedT_fix_special;

    //
    DoubleVector mParamV;
    DoubleVector mParamF;
    DoubleVector mG;
    DoubleVector mTenorG;
	std::shared_ptr<AQLInterpolationBase> mpInterG;
	

    void createCacheVolMatrix( size_t i );
};

