#pragma once


#include <vector>
#include "AQLObject.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLCoreTemplateType.h"

using namespace std;


class AQLMathCorrelationLMM : public AQLObject
{

public:

    AQLMathCorrelationLMM();

    AQLMathCorrelationLMM( const AQLMathCorrelationLMM& rhs );

    virtual ~AQLMathCorrelationLMM();

    AQLMathCorrelationLMM& operator =(const AQLMathCorrelationLMM& rhs);

    virtual AQLMathCorrelationLMM* clone() const = 0;

    virtual double get(double t, size_t p, size_t q) = 0;

    virtual DoubleVector getFactors(double t, size_t p) = 0;

    DoubleVector getTgrid() const { return mT_fix; }

    virtual bool isDiscrete() const = 0;

    size_t getNoFactors() const { return mNoFactors; }

    DoubleMatrix getFullRankCorr(double t) const;

protected:

	AQLMathCorrelationLMM(	const DoubleVector&	T_fix_,
						size_t				no_factors_ = 0
			);

	AQLMathCorrelationLMM( AQLMathCorrelationFuncLMM*	corr_,
						const DoubleVector&		T_fix_,
						size_t					no_factors_ = 0
			);

    AQLMathCorrelationFuncLMM* mCorr;
    DoubleVector mT_fix;
    size_t mNoFactors;
    DoubleVector mUnity;

private:

    DoubleMatrix mFullRankCorrMat;
    bool mCloned;
};
