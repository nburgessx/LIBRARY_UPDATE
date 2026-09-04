/*!
    @file AQLMathCorrelationLMMDiscAngle.cpp
*/

#include "AQLMathCorrelationLMMDiscAngle.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLModelUtilities.h"
#include "AQLEigenSystems.h"

//
//////////////////////////////////////////////////////////////////
////////// Reduced-rank formulations: Rebonato's angles  /////////
AQLMathCorrelationLMMDiscAngle::AQLMathCorrelationLMMDiscAngle()
: AQLMathCorrelationLMMDisc()
{
}

AQLMathCorrelationLMMDiscAngle::AQLMathCorrelationLMMDiscAngle(AQLMathCorrelationFuncLMM*	corr,
														 const DoubleVector&		T_fix,
														 size_t						no_factors
		)
:
AQLMathCorrelationLMMDisc( corr, T_fix, no_factors )
{
    if( no_factors > 1 ) mTheta = angleFromFactorLoading(mFactorLoading);
}

AQLMathCorrelationLMMDiscAngle::AQLMathCorrelationLMMDiscAngle(const DoubleMatrix&	corr_mat_,
														 const DoubleVector&	T_fix,
														 size_t					no_factors,
														 bool					full_mat
		)
:
AQLMathCorrelationLMMDisc( corr_mat_, T_fix, no_factors, full_mat )
{
    if( no_factors > 1 ) mTheta = angleFromFactorLoading(mFactorLoading);
    if (full_mat)
    {
        //if (T_fix.size() != corr_mat_.size())
        //{
        //    throw invalid_argument("full-length corr_mat is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc");
        //}
        //take_col_row(corr_mat_, corr_mat, -static_cast<int>(T_fix.size()) + 1, -static_cast<int>(T_fix.size()) + 1 );
        //factor_loading = Factor_loading_from_Corr( corr_mat, no_factors);
        //corr_mat = Corr_from_Factor_loading( factor_loading );

        //for (size_t i = 0; i < corr_mat.size(); i++ )
        //{
        //    corr_mat[i].insert(corr_mat[i].begin(),0);
        //}
        //corr_mat.insert(corr_mat.begin(), corr_mat_.front() );
        //factor_loading.insert(factor_loading.begin(), DoubleVector(no_factors, 0));
    }
    else
    {
        //if (T_fix.size() != corr_mat_.size() + 1)
        //{
        //    throw invalid_argument("reduced-length corr_mat is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc");
        //}
        //factor_loading = Factor_loading_from_Corr( corr_mat_, no_factors );
        //corr_mat = Corr_from_Factor_loading( factor_loading );
        //for (size_t i = 0; i < corr_mat.size(); i++ )
        //{
        //    corr_mat[i].insert(corr_mat[i].begin(),0);
        //}
        //corr_mat.insert(corr_mat.begin(), DoubleVector(corr_mat[0].size(), 0) );
        //corr_mat[0][0] = 1;
        //factor_loading.insert(factor_loading.begin(), DoubleVector(no_factors, 0) );
    }
}

AQLMathCorrelationLMMDiscAngle::AQLMathCorrelationLMMDiscAngle(const DoubleMatrix&	factor_loading_,
														 const DoubleVector&	T_fix,
														 bool					full_mat
		)
:
AQLMathCorrelationLMMDisc( factor_loading_, T_fix, full_mat)
{
    if( factor_loading_[0].size() > 1 ) mTheta = angleFromFactorLoading(mFactorLoading);

    if (full_mat)
    {
        //if (T_fix.size() != factor_loading.size())
        //{
        //    throw invalid_argument("full-length factor loading is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc");
        //} 
        //DoubleMatrix factor_loading_temp( factor_loading.begin() + 1, factor_loading.end() );
        //corr_mat = Corr_from_Factor_loading( factor_loading_temp );
        //for (size_t i = 0; i < corr_mat.size(); i++ )
        //{
        //    corr_mat[i].insert(corr_mat[i].begin(),0);
        //}
        //corr_mat.insert(corr_mat.begin(), DoubleVector(corr_mat[0].size(), 0) );
        //corr_mat[0][0] = 1;
    }
    else
    {
        //if (T_fix.size() != factor_loading.size() + 1)
        //{
        //    throw invalid_argument("reduced-length factors loading is incompatible with T_fix : AQLMathCorrelationLMMDisc::AQLMathCorrelationLMMDisc");
        //}
        //corr_mat = Corr_from_Factor_loading( factor_loading );
        //for (size_t i = 0; i < corr_mat.size(); i++ )
        //{
        //    corr_mat[i].insert(corr_mat[i].begin(),0);
        //}
        //corr_mat.insert(corr_mat.begin(), DoubleVector(corr_mat[0].size(), 0) );
        //corr_mat[0][0] = 1;
        //factor_loading.insert(factor_loading.begin(), DoubleVector(no_factors, 0) );
    }
}

AQLMathCorrelationLMMDiscAngle::AQLMathCorrelationLMMDiscAngle(const DoubleMatrix& theta_,
														 const DoubleVector& T_fix
		)
:
AQLMathCorrelationLMMDisc( factorLoadingFromAngle(theta_), T_fix ),
mTheta(theta_)
{
}

void AQLMathCorrelationLMMDiscAngle::setTheta(const DoubleMatrix& theta_)
{ 
	mTheta = theta_;
	mFactorLoading = factorLoadingFromAngle(mTheta);
	mCorrMat = AQLEigenSystems::factorloading2corre(mFactorLoading);
}

DoubleMatrix AQLMathCorrelationLMMDiscAngle::angleFromFactorLoading( const DoubleMatrix& factor_loading_, double eps )
{
	//error check
	if( mFactorLoading.size() < 2)
	{
		AQLString msg = "mFactorLoading.size() must bigger than 2.";
		AQLString msg_file = " : AQLMathCorrelationLMMDiscAngle::angleFromFactorLoading";
		throw AQLCoreInvalidData((msg + msg_file).getCString(), __FILE__, __LINE__);
	}
    if( mFactorLoading[0].size() < 2)
	{
		AQLString msg = "mFactorLoading[0].size() must bigger than 2.";
		AQLString msg_file = " : AQLMathCorrelationLMMDiscAngle::angleFromFactorLoading";
		throw AQLCoreInvalidData((msg + msg_file).getCString(), __FILE__, __LINE__);
	}
	
	double tol = fabs(eps);

	size_t n = factor_loading_.size();
	size_t m = factor_loading_[0].size();
	
	DoubleMatrix theta_( n, DoubleVector(m - 1, 0.0) );
	
	for(size_t i = 0; i < n; ++i)
	{
		
		if( fabs(mFactorLoading[i][0]) > 1.0 + tol )
		{
			//furuya//AQLString msg =  "Element of mFactorLoading is " + AQLModelUtilities::n2s(1.0 + tol) + "> 1!.";
			AQLString msg =  "Element of mFactorLoading is " + AQLString(AQLModelUtilities::n2s(1.0 + tol).c_str()) + "> 1!.";
			AQLString msg_file = " : AQLMathCorrelationLMMDiscAngle::angleFromFactorLoading";
			throw AQLCoreInvalidData( (msg + msg_file).getCString(), __FILE__, __LINE__ );
		}

		theta_[i][0] = fabs( mFactorLoading[i][0] ) > 1.0 ? M_PI / 2.0 : acos(mFactorLoading[i][0]);
		double sin_ = sin(theta_[i][0]);
		
		for(size_t j = 1; j < m - 1; ++j)
		{
			if( sin_ == 0.0)
			{
				theta_[i][j] = 0.0;
			}
			else
			{
				double cos_theta_ij = mFactorLoading[i][j] / sin_;
				if( fabs(cos_theta_ij) > 1.0 + tol)
				{
					//furuya//AQLString msg =  "Element of mFactorLoading is bigger than : " + AQLModelUtilities::n2s(1.0 + tol) + "!.";
					AQLString msg =  "Element of mFactorLoading is bigger than : " + AQLString(AQLModelUtilities::n2s(1.0 + tol).c_str()) + "!.";
					AQLString msg_file = " : AQLMathCorrelationLMMDiscAngle::angleFromFactorLoading";
					throw AQLCoreInvalidData( (msg + msg_file).getCString(), __FILE__, __LINE__ );
				}
				if( fabs(cos_theta_ij) > 1) cos_theta_ij = 1.0;
				theta_[i][j] = acos(cos_theta_ij);
				sin_ *= sin(theta_[i][j]); 
			}
		}
		
		if( AQLModelUtilities::sign(sin_) != AQLModelUtilities::sign(mFactorLoading[i][m-1]) )
		{
			theta_[i][m-2] *= -1.0;
			sin_ *= -1.0;
		}

		if( fabs( sin_ - mFactorLoading[i][m - 1] ) > tol )
		{
			AQLString msg =  "sin_ = mFactorLoading[i][m - 1] is not satisfied!";
			AQLString msg_file = " : AQLMathCorrelationLMMDiscAngle::angleFromFactorLoading";
			throw AQLCoreInvalidData( (msg + msg_file).getCString(), __FILE__, __LINE__ );
		}
	}
	
	// -PI < theta[i][j] <= PI 
	for(size_t i = 0; i < n; i++)
	{
		for(size_t j = 0; j < m - 1; j++)
		{
			for (;;)
			{
				if (theta_[i][j] > M_PI) theta_[i][j] -= M_PI;
				else if (theta_[i][j] < -M_PI) theta_[i][j] += M_PI;
				else if (theta_[i][j] == -M_PI)
				{
					theta_[i][j] = M_PI;
					break;
				}
				else break;
			}
		}		
	}

	return theta_;
}

DoubleMatrix AQLMathCorrelationLMMDiscAngle::factorLoadingFromAngle( const DoubleMatrix& theta_ )
{
	size_t n = theta_.size();
	size_t m = theta_[0].size();

	DoubleMatrix factor_loading_(n, DoubleVector(m + 1, 0.0));

	double sin_;
	for(size_t i = 0; i < n; ++i)
	{
		factor_loading_[i][0] = cos(theta_[i][0]);
		sin_ = sin(theta_[i][0]);

		for(size_t j = 1; j < m; ++j)
		{
			factor_loading_[i][j] = sin_ * cos(theta_[i][j]);
			sin_ *= sin(theta_[i][j]); 
		}
		factor_loading_[i][m] = sin_;
	}

	return factor_loading_;
}

