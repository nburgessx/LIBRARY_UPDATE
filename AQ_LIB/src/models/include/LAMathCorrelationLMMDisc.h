#pragma once

#include <vector>
#include "LAMathCorrelationLMM.h"
#include "LAMathCorrelationFuncLMM.h"
#include "LACoreTemplateType.h"

using namespace std;


class LAMathCorrelationLMMDisc : public LAMathCorrelationLMM
{

public:

    LAMathCorrelationLMMDisc();

	LAMathCorrelationLMMDisc( LAMathCorrelationFuncLMM*	corr,
							const DoubleVector&		T_fix,
							size_t					no_factors = 0
			);

	LAMathCorrelationLMMDisc(	const DoubleMatrix&	corr_mat_,
							const DoubleVector&	T_fix,
							size_t				no_factors,
							bool				full_mat
			);

	LAMathCorrelationLMMDisc(	const DoubleMatrix&	factor_loading_,
							const DoubleVector&	T_fix,
							bool				full_mat
			);

	LAMathCorrelationLMMDisc(	const DoubleMatrix& factor_loading_,
							const DoubleVector&	T_fix
			);

    LAMathCorrelationLMMDisc( const LAMathCorrelationLMMDisc& rhs );

    virtual ~LAMathCorrelationLMMDisc() {}

    virtual LAMathCorrelationLMM* clone() const { return new LAMathCorrelationLMMDisc(*this); }

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
