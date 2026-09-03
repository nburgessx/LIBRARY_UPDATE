#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPolyFitBase.h"
#include "LALinearRegression.h"


// ID for LAPolyFitLS
#define FN_POLYFITLS	2502
// Function Name of LAPolyFitLS
#define FN_POLYFITLS_STR	"fn_polyfit_ls"


class LAPolynomialBase;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of polynomial fitting class

*/
class LAPolyFitLS : public LAPolyFitBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPolyFitLS(bool isExcludeOutlier_Y = false, double criteria_Y = 5, bool isExcludeOutlier_X = false, double criteria_X = 5, bool isExcludeOutlier_e = false, double criteria_e = 5, bool isShift_Y = false, bool isScale_Y = false, bool isShift_X = false, bool isScale_X = false);
	//	Copy constructor
//	LAPolyFitLS(const LAPolyFitLS& v);
	// Destructor
	virtual ~LAPolyFitLS();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// execute regression
    virtual void				regress(const DoubleArray& y, const DoubleMatrix& x, LAPolynomialBase& poly, DoubleArray *p_y_t = 0, DoubleMatrix *p_x_t = 0) const;
								//======================================
								// execute regression
    void                        setRegParam(bool isExcludeOutlier_Y,
                                            double criteria_Y,
                                            bool isExcludeOutlier_X,
                                            double criteria_X,
                                            bool isExcludeOutlier_e,
                                            double criteria_e,
											bool isShift_Y, 
											bool isScale_Y,
											bool isShift_X,
											bool isScale_X,
											double SVDTolerance
                                           );

private:

protected:
	LALinearRegression mReg;// regression tool
	bool			mIsShift_Y; // flag of shift y value
	bool			mIsScale_Y; // flag of scale y value
	bool			mIsShift_X; // flag of shift x value
	bool			mIsScale_X; // flag of scale x value

};
