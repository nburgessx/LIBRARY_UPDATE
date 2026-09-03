/*! @file
    @brief Class declaration to LAMathSABR.

*/
//  2009, AlgoQuantHub.
#ifndef __LAMATHCAPLETMATRIX_H__
#define __LAMATHCAPLETMATRIX_H__

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>

#include <LADataInstance.h>
#include <LACoreTemplateType.h>
#include <LAString.h>
#include <LAPriceDataSlidingRule.h>
#include <LAPriceDataCalendar.h>
#include "LAMathSABR.h"

using namespace std;

class LAMathCapFloorConvention
{
public:
    // constructor
    LAMathCapFloorConvention( const LAString& freq_, const LAString& spotLag_, 
                            const LAString& daycount_, const LAString& paySlr_, const LAString& payCal_, 
                            const LAString& fixCal_ );
    // constructor
    LAMathCapFloorConvention( const LAString& freq_, const LAString& spotLag_, 
                            const LAString& daycount_, const LAPriceDataSlidingRule& paySlr_, 
                            const LAPriceDataCalendar& payCal_, const LAPriceDataCalendar& fixCal_ );
    // destructor
    virtual ~LAMathCapFloorConvention();

    LAString freq;
    LAString spotLag;
    LAString daycount;
    LAPriceDataSlidingRule paySlr;
    LAPriceDataCalendar payCal;
    LAPriceDataCalendar fixCal;
};

class LAMathCapFloorSABR
{
public:
    // constructor
    // maturity is represented by numbers of months
    LAMathCapFloorSABR( const IntVector& expiDate, const vector<LAMathSABR_Hagan>& sabr_, LAInterpolationBase* pInter_, 
					  const LAMathCapFloorConvention& conv_, const LAString& curveID, 
					  LAString foreCurveName = STD, LAString dfCurveName = STD);
    // destructor
    virtual ~LAMathCapFloorSABR();

    LAMathSABR_Hagan getSABRParam(unsigned int pos);

    double getVol( LADataInstance* dataInstance, double fixingTerm, double strike );

    void calibrateToCapVol( LADataInstance* dataInstance, LAMathSABRLimiter sabrLimiter, 
                            const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec,
                            const IntVector& capTerm_mon, const DoubleVector& weight, 
                            const LAString& target, DoubleMatrix& capletVol, DoubleMatrix& capVol );

    void getCapletVolMat( LADataInstance* dataInstance, const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec, 
                          const IntVector& capTerm_mon, DoubleMatrix& capletVol, DoubleMatrix& capVol );

    void getCapletVolMat2( LADataInstance* dataInstance, const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec, 
                           const IntVector& capTerm_mon, DoubleMatrix& capletVol, DoubleMatrix& capVol );
private:
    //functions for calibration
    void setCalibrationStack(LADataInstance* dataInstance);

    double getCapFloorPrem( unsigned int expiPos, double strike, double aveVol );

    void getCapletVol( unsigned int expiPos, unsigned int columPos, 
                       DoubleMatrix& capletVols, double strike, double aveVol );

    double getCapVol( unsigned int expiPos, unsigned int columPos, 
                      const DoubleMatrix& capletVols, double strike );

    IntVector expiDate;
    vector<LAMathSABR_Hagan> sabr;
    LAInterpolationBase* pInter;
    LAMathCapFloorConvention conv;
	LAString curveID;
	LAString foreCurveName;
	LAString dfCurveName;
    //stack member for calibration
    DoubleArray Nu;
    DoubleArray F;
    DoubleArray Te;
};

class LAMathCapVolCalibrator : public LAFunctionVector
{
public:
    // constructor
    LAMathCapVolCalibrator( const DoubleArray& capletVol_, const DoubleArray& Te_, const DoubleArray& F_, 
                          const DoubleArray& Nu_, double strike_, double aveVol_, LAInterpolationBase* pInter_ );
    // destructor
    virtual ~LAMathCapVolCalibrator(){}

    virtual unsigned long lengthOfArgumentVector() {return 1;}

    virtual unsigned long lengthOfFunctionVector() {return 1;}

    virtual unsigned long maximumNumberOfIterations() {return 150;}

    virtual void operator()(DoubleArray& f, const DoubleArray& x);

    virtual bool constraintsAreViolated(const DoubleArray& x);

    DoubleArray getCapletVol(const DoubleArray& x);

private:
    DoubleArray capletVol;
    DoubleArray Te;
    DoubleArray F;
    DoubleArray Nu;
    double strike;
    double aveVol;
    LAInterpolationBase* pInter;
    size_t volNum;

    double premSum;
    double capPrem;
    DoubleArray Te_inter;
    DoubleArray capletVol_inter;
};

inline DoubleArray getPartialVector(const DoubleArray& vector, unsigned int starPos, unsigned int endPos)
{
    DoubleArray ret;
    for(size_t i=starPos; i<=endPos; i++)
    {
        ret.push_back( vector[i] );
    }
    return ret;
};

inline DoubleArray getConstantVector(double element, size_t size)
{
    DoubleArray ret;
    for(size_t i=0;i<size; i++)
    {
        ret.push_back( element );
    }
    return ret;
};
#endif