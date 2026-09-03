#pragma once

#include <vector>
#include "AQLMathCorrelationLMM.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLCoreTemplateType.h"

using namespace std;


class AQLMathCorrelationLMMDisc : public AQLMathCorrelationLMM
{

public:

    AQLMathCorrelationLMMDisc();

	AQLMathCorrelationLMMDisc( AQLMathCorrelationFuncLMM*	corr,
							const DoubleVector&		T_fix,
							size_t					no_factors = 0
			);

	AQLMathCorrelationLMMDisc(	const DoubleMatrix&	corr_mat_,
							const DoubleVector&	T_fix,
							size_t				no_factors,
							bool				full_mat
			);

	AQLMathCorrelationLMMDisc(	const DoubleMatrix&	factor_loading_,
							const DoubleVector&	T_fix,
							bool				full_mat
			);

	AQLMathCorrelationLMMDisc(	const DoubleMatrix& factor_loading_,
							const DoubleVector&	T_fix
			);

    AQLMathCorrelationLMMDisc( const AQLMathCorrelationLMMDisc& rhs );

    virtual ~AQLMathCorrelationLMMDisc() {}

    virtual AQLMathCorrelationLMM* clone() const { return new AQLMathCorrelationLMMDisc(*this); }

	DoubleMatrix getFactorLoading() { return mFactorLoading; }

	double getFactorLoading_ij(size_t i, size_t j) { return mFactorLoading[i][j]; }

    bool isDiscrete() const { return true; }

    double get(double t, size_t p, size_t q);

    DoubleVector getFactors(double t, size_t p);

    double get(size_t s, size_t p, size_t q);

    DoubleVector getFactors(size_t s, size_t p);

protected:
	DoubleMatrix mFactorLoading;
    DoubleMatrix mCorrMat;

private:
};
