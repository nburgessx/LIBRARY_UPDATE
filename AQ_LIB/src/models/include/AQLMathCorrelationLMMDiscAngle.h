#pragma once

#include <vector>
#include "AQLMathCorrelationLMMDisc.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLCoreTemplateType.h"

using namespace std;


class AQLMathCorrelationLMMDiscAngle : public AQLMathCorrelationLMMDisc
{

public:

	AQLMathCorrelationLMMDiscAngle();

    AQLMathCorrelationLMMDiscAngle(AQLMathCorrelationFuncLMM*	corr,
								const DoubleVector&		T_fix, 
								size_t					no_factors = 0
			);

	AQLMathCorrelationLMMDiscAngle(const DoubleMatrix&	corr_mat_,
								const DoubleVector&	T_fix,
								size_t				no_factors,
								bool				full_mat
			);

	AQLMathCorrelationLMMDiscAngle(const DoubleMatrix&	factor_loading_,
								const DoubleVector&	T_fix,
								bool				full_mat
			);

	AQLMathCorrelationLMMDiscAngle(const DoubleMatrix&	theta_,
								const DoubleVector&	T_fix
			);

	void setTheta(const DoubleMatrix& theta_);

	DoubleMatrix getTheta() const { return mTheta; }
	
	virtual ~AQLMathCorrelationLMMDiscAngle() {}

    virtual AQLMathCorrelationLMM* clone() const { return new AQLMathCorrelationLMMDiscAngle(*this); }

private:

	DoubleMatrix angleFromFactorLoading( const DoubleMatrix& factor_loading_, double eps = 0.1 );

	DoubleMatrix factorLoadingFromAngle( const DoubleMatrix& theta_ );

    DoubleMatrix mTheta;

};


