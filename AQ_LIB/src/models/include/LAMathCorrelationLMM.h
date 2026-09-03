#pragma once


#include <vector>
#include "AQLObject.h"
#include "LAMathCorrelationFuncLMM.h"
#include "AQLCoreTemplateType.h"

using namespace std;


class LAMathCorrelationLMM : public AQLObject
{

public:

    LAMathCorrelationLMM();

    LAMathCorrelationLMM( const LAMathCorrelationLMM& rhs );

    virtual ~LAMathCorrelationLMM();

    LAMathCorrelationLMM& operator =(const LAMathCorrelationLMM& rhs);

    virtual LAMathCorrelationLMM* clone() const = 0;

    virtual double get(double t, size_t p, size_t q) = 0;

    virtual DoubleVector getFactors(double t, size_t p) = 0;

    DoubleVector getTgrid() const { return mT_fix; }

    virtual bool isDiscrete() const = 0;

    size_t getNoFactors() const { return mNoFactors; }

    DoubleMatrix getFullRankCorr(double t) const;

protected:

	LAMathCorrelationLMM(	const DoubleVector&	T_fix_,
						size_t				no_factors_ = 0
			);

	LAMathCorrelationLMM( LAMathCorrelationFuncLMM*	corr_,
						const DoubleVector&		T_fix_,
						size_t					no_factors_ = 0
			);

    LAMathCorrelationFuncLMM* mCorr;
    DoubleVector mT_fix;
    size_t mNoFactors;
    DoubleVector mUnity;

private:

    DoubleMatrix mFullRankCorrMat;
    bool mCloned;
};
