#pragma once

#include <vector>
#include "LAMathCorrelationLMMDisc.h"
#include "LAMathCorrelationFuncLMM.h"
#include "AQLCoreTemplateType.h"

using namespace std;


class LAMathCorrelationLMMDiscAngle : public LAMathCorrelationLMMDisc
{

public:

	LAMathCorrelationLMMDiscAngle();

    LAMathCorrelationLMMDiscAngle(LAMathCorrelationFuncLMM*	corr,
								const DoubleVector&		T_fix, 
								size_t					no_factors = 0
			);

	LAMathCorrelationLMMDiscAngle(const DoubleMatrix&	corr_mat_,
								const DoubleVector&	T_fix,
								size_t				no_factors,
								bool				full_mat
			);

	LAMathCorrelationLMMDiscAngle(const DoubleMatrix&	factor_loading_,
								const DoubleVector&	T_fix,
								bool				full_mat
			);

	LAMathCorrelationLMMDiscAngle(const DoubleMatrix&	theta_,
								const DoubleVector&	T_fix
			);

	void setTheta(const DoubleMatrix& theta_);

	DoubleMatrix getTheta() const { return mTheta; }
	
	virtual ~LAMathCorrelationLMMDiscAngle() {}

    virtual LAMathCorrelationLMM* clone() const { return new LAMathCorrelationLMMDiscAngle(*this); }

private:

	DoubleMatrix angleFromFactorLoading( const DoubleMatrix& factor_loading_, double eps = 0.1 );

	DoubleMatrix factorLoadingFromAngle( const DoubleMatrix& theta_ );

    DoubleMatrix mTheta;

};


