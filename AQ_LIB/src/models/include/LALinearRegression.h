#pragma once

#ifdef __GNUG__
#pragma interface
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif


#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include "AQLCoreTemplateType.h"


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class of linear regression analysis

*/
class LALinearRegression
{
public:
//  LIFECYCLE
	// constructor
	LALinearRegression(bool isExcludeOutlier_Y = false, double criteria_Y = 5, bool isExcludeOutlier_X = false, double criteria_X = 5, bool isExcludeOutlier_e = false, double criteria_e = 5, double SVDTolerance = 1.E-13);
	// Destructor
	virtual ~LALinearRegression();
	// get regression coefficient
	/*!
		@brief get regression coefficient
	*/
	const DoubleArray&	getCoefficient() const {return mCoefficient;}
	// set data
	/*!
		@brief set data
		@param[in] y explained variables
		@param[in] x explanatory variables
	*/
	void				setData(const DoubleArray& y, const DoubleMatrix& x) {mY = y; mX = x;}
	//
	/*!
		@brief set zero interception or not
		@param[in] flag true:zero interception, false:not zero interception
	*/
	void				setZeroInterception(bool flag = false) {mIsZeroInterception = flag;}
	// execute regression analysis
	void				executeRegression();
	//======================================
	/*!
        @brief set exclude outlier Y or not
		@param[in] flag true:ExcludeOutlier_Y, false:Not ExcludeOutlier_Y
	*/
	void    			setExcludeOutlier_Y(bool flag) {mIsExcludeOutlier_Y =flag;} 
	//======================================
	/*!
        @brief set exclude outlier X or not
		@param[in] flag true:ExcludeOutlier_X, false:Not ExcludeOutlier_X
	*/
	void    			setExcludeOutlier_X(bool flag) {mIsExcludeOutlier_X =flag;} 
	//======================================
	/*!
        @brief set exclude outlier residual error or not
		@param[in] flag true:ExcludeOutlier_E, false:Not ExcludeOutlier_E
	*/
	void    			setExcludeOutlier_e(bool flag) {mIsExcludeOutlier_e =flag;} 
	//======================================
	/*!
        @brief set value of criteria Y
		@param[in] value criteria Y
        */
	void    			setCriteria_Y(double value) {mCriteria_Y =value;} 
	//======================================
	/*!
        @brief set value of criteria X
		@param[in] value criteria X
        */
	void    			setCriteria_X(double value) {mCriteria_X =value;} 
	//======================================
	/*!
        @brief set value of criteria E
		@param[in] value criteria E
        */
	void    			setCriteria_e(double value) {mCriteria_e =value;} 
	//======================================
	/*!
        @brief set value of SVD Tolerance
		@param[in] value SVD Tolerance
        */
	void    			setSVDTolerance(double value) {mSVDTolerance =value;} 

private:
	
protected:
	DoubleArray		mY;	// value array of explained variables
	DoubleMatrix	mX;	// value matrix of explanatory variables
	DoubleArray		mCoefficient; // regression coefficient
	bool			mIsZeroInterception; // zero interception flag			
	bool			mIsExcludeOutlier_Y; // flag of exclude outlier by y value
	bool			mIsExcludeOutlier_X; // flag of exclude outlier by x value
	bool			mIsExcludeOutlier_e; // flag of exclude outlier by x residual
	double			mCriteria_Y;// criteria of mutiplier of std of y when exlude outlier
	double			mCriteria_X;// criteria of mutiplier of std of x when exlude outlier
	double			mCriteria_e;// criteria of mutiplier of std of residual when exlude outlier
	double          mSVDTolerance;// tolerence ratio for eigen value of matrix

};
