/*! @file
    @brief Source code of class of linear regression analysis


*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLLinearRegression.h"
#include "AQLMatrix.h"

using namespace std;


//================ AQLLinearRegression ===================================
/*!
	@brief constructor
    @param[in] isExcludeOutlier_Y	flag of exclude outlier by judging Y(explained variables) value 
    @param[in] criteria_Y			criteria of outlier of judging Y value case
    @param[in] isExcludeOutlier_X	flag of exclude outlier by judging X(explanatory variables) value   
    @param[in] criteria_X			criteria of outlier of judging X value case
    @param[in] isExcludeOutlier_e   flag of exclude outlier by judging residual value
    @param[in] criteria_e			criteria of outlier of judging residual value case
	
*/
AQLLinearRegression::AQLLinearRegression(bool isExcludeOutlier_Y, double criteria_Y, bool isExcludeOutlier_X, double criteria_X, bool isExcludeOutlier_e, double criteria_e, double SVDTolerance)
: mIsZeroInterception(false), mIsExcludeOutlier_Y(isExcludeOutlier_Y), mCriteria_Y(criteria_Y),
mIsExcludeOutlier_X(isExcludeOutlier_X), mCriteria_X(criteria_X),
mIsExcludeOutlier_e(isExcludeOutlier_e), mCriteria_e(criteria_e),
mSVDTolerance(SVDTolerance)
{

}




/*!
	@brief destructor
*/
AQLLinearRegression::~AQLLinearRegression() 
{
	
}
/*!
	@brief execute regression analysis

*/
/*void
AQLLinearRegression::executeRegression()
{
	AQLMatrix x;
	if (mIsZeroInterception)
	{
		x.resize(mX.size(), mX[0].size());
		for (unsigned int i = 0; i < x.row(); i++)
			for (unsigned int j = 0; j < x.column(); j++)
			 x.setValue(i, j, mX[i][j]);	
	}
	else
	{
		x.resize(mX.size(), mX[0].size() + 1);
		for (unsigned int i = 0; i < x.row(); i++)
		{
			x.setValue(i, 0, 1);
			for (unsigned int j = 1; j < x.column(); j++)
			 x.setValue(i, j, mX[i][j - 1]);
		}
	}

	AQLMatrix y(mY);

//	const AQLMatrix& beta = (x.transpose() * x).inverseMatrix() * x.transpose() * y;
	AQLMatrix u, w, v;
	const AQLMatrix& tmp = (x.transpose() * x);
	tmp.svDecomp(u, w, v);
	const double TOL = 1.0E-13;
	double wmax = 0.0;
	for (unsigned int i = 0; i < w.row(); i++)
		if (w[i][i] > wmax) wmax=w[i][i];
	double 	thresh=TOL * wmax;
	for (unsigned int i = 0; i < w.row(); i++)
	{
		if (w[i][i] < thresh) w.setValue(i, i, 0.0);
		else w.setValue(i, i, 1.0/w[i][i]);
	}
	const AQLMatrix& beta = v * w * u.transpose() * (x.transpose() * y);

	mCoefficient.resize(beta.row());
	for (unsigned int i = 0; i < mCoefficient.size(); i++)
		mCoefficient[i] = beta[i][0];





}
*/

/*!
	@brief execute regression analysis
*/
void
AQLLinearRegression::executeRegression()
{
	unsigned int size = mY.size();
	BoolVector flag(mY.size(), true);
	if (mIsExcludeOutlier_Y)
	{
		double ubound_y, lbound_y;
		double sum1 = 0.0, sum2 = 0.0;
		unsigned int size_y = mY.size();
		for (unsigned int i = 0; i < size_y; i++)
		{
			sum1 += mY[i];
			sum2 += mY[i] * mY[i];
		}
		sum1 /= static_cast<double>(size_y);
		sum2 /= static_cast<double>(size_y);
		sum2 -= sum1 * sum1;
		sum2 = AQLMath::sqrt(sum2);
		ubound_y = sum1 + sum2 * mCriteria_Y;
		lbound_y = sum1 - sum2 * mCriteria_Y;
		for (unsigned int i = 0; i < size_y; i++)
		{
			if (mY[i] > ubound_y || mY[i] < lbound_y)
			{
				flag[i] = false;
				size--;
			}			
		}
	}	
	if (mIsExcludeOutlier_X)
	{
		DoubleArray ubound_x(mX[0].size()), lbound_x(mX[0].size());
		double sum1, sum2;
		unsigned int size_x = mX.size();
		for (unsigned int j = 0; j < mX[0].size(); j++)
		{
			sum1 = 0.0;
			sum2 = 0.0;
			for (unsigned int i = 0; i < size_x; i++)
			{
				sum1 += mX[i][j];
				sum2 += mX[i][j] * mX[i][j];
			}
			sum1 /= static_cast<double>(size_x);
			sum2 /= static_cast<double>(size_x);
			sum2 -= sum1 * sum1;
			sum2 = AQLMath::sqrt(sum2);
			ubound_x[j] = sum1 + sum2 * mCriteria_X;
			lbound_x[j] = sum1 - sum2 * mCriteria_X;
		}
		for (unsigned int i = 0; i < size_x; i++)
		{
			for (unsigned int j = 0; j < mX[0].size(); j++)
			{
				if (flag[i] && (mX[i][j] > ubound_x[j] || mX[i][j] < lbound_x[j]))
				{
					flag[i] = false;
					size--;
					break;
				}
			}
		}
	}	
		
	AQLMatrix x, y;
	if (mIsExcludeOutlier_e)
	{
		y.resize(mY.size(), 1);
		if (mIsZeroInterception)
		{
			x.resize(mX.size(), mX[0].size());			
			for (unsigned int i = 0; i < x.row(); i++)
			{
				y.setValue(i, 0, mY[i]);	
				for (unsigned int j = 0; j < x.column(); j++)
					x.setValue(i, j, mX[i][j]);	
			}
		}
		else
		{
			x.resize(mX.size(), mX[0].size() + 1);
			for (unsigned int i = 0; i < x.row(); i++)
			{
				x.setValue(i, 0, 1);
				y.setValue(i, 0, mY[i]);	
				for (unsigned int j = 1; j < x.column(); j++)
					x.setValue(i, j, mX[i][j - 1]);
			}
		}

		AQLMatrix y(mY);

		AQLMatrix u, w, v;
		const AQLMatrix& tmp = (x.transpose() * x);
		tmp.svDecomp(u, w, v);
		const double TOL = 1.0E-13;
		double wmax = 0.0;
		for (unsigned int i = 0; i < w.row(); i++)
			if (w[i][i] > wmax) wmax=w[i][i];
		double 	thresh=TOL * wmax;
		for (unsigned int i = 0; i < w.row(); i++)
		{
			if (w[i][i] < thresh) w.setValue(i, i, 0.0);
			else w.setValue(i, i, 1.0/w[i][i]);
		}
		const AQLMatrix& beta = v * w * u.transpose() * (x.transpose() * y);		

		const AQLMatrix& residual = y - x * beta;
		double sum1 = 0.0, sum2 = 0.0;
		for (unsigned int i = 0; i < residual.row(); i++)
		{
			sum1 += residual[i][0];
			sum2 += residual[i][0] * residual[i][0]; 
		}
		sum1 /= static_cast<double>(residual.row());
		sum2 /= static_cast<double>(residual.row());
		sum2 -= sum1 * sum1;
		sum2 = AQLMath::sqrt(sum2);
		double ubound = sum1 + sum2 * mCriteria_e;
		double lbound = sum1 - sum2 * mCriteria_e;
		unsigned int count = 0;
		for (unsigned int i = 0; i < residual.row(); i++)
		{
			if (flag[i] && (residual[i][0] > ubound || residual[i][0] < lbound))
			{
				flag[i] = false;
				size--;				
			}			
		}	
	}
		
	
	y.resize(size, 1);
	if (mIsZeroInterception)
	{
		x.resize(size, mX[0].size());
		unsigned int count = 0;
		for (unsigned int i = 0; i < mY.size(); i++)
		{
			if (flag[i])
			{
				y.setValue(count, 0, mY[i]);
				for (unsigned int j = 0; j < x.column(); j++)
					x.setValue(count, j, mX[i][j]);				
				count++;
			}
		}	
	}
	else
	{
		x.resize(size, mX[0].size() + 1);		
		unsigned int count = 0;
		for (unsigned int i = 0; i < mY.size(); i++)
		{
			if (flag[i])
			{
				x.setValue(count, 0, 1);
				y.setValue(count, 0, mY[i]);
				for (unsigned int j = 1; j < x.column(); j++)
					x.setValue(count, j, mX[i][j - 1]);				
				count++;
			}
		}
	}

//	const AQLMatrix& beta = (x.transpose() * x).inverseMatrix() * x.transpose() * y;
	AQLMatrix u, w, v;
	const AQLMatrix& tmp = (x.transpose() * x);
	tmp.svDecomp(u, w, v);
	const double TOL = mSVDTolerance;
	double wmax = 0.0;
	for (unsigned int i = 0; i < w.row(); i++)
		if (w[i][i] > wmax) wmax=w[i][i];
	double 	thresh=TOL * wmax;
	for (unsigned int i = 0; i < w.row(); i++)
	{
		if (w[i][i] < thresh) w.setValue(i, i, 0.0);
		else w.setValue(i, i, 1.0/w[i][i]);
	}
	const AQLMatrix& beta = v * w * u.transpose() * (x.transpose() * y);

	mCoefficient.resize(beta.row());
	for (unsigned int i = 0; i < mCoefficient.size(); i++)
		mCoefficient[i] = beta[i][0];

}
