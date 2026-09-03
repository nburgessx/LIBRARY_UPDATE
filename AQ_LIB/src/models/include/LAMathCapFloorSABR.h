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

#include <AQLDataInstance.h>
#include <AQLCoreTemplateType.h>
#include <AQLString.h>
#include <AQLPriceDataSlidingRule.h>
#include <AQLPriceDataCalendar.h>
#include "LAMathSABR.h"

using namespace std;

class LAMathCapFloorConvention
{
public:
    // constructor
    LAMathCapFloorConvention( const AQLString& freq_, const AQLString& spotLag_, 
                            const AQLString& daycount_, const AQLString& paySlr_, const AQLString& payCal_, 
                            const AQLString& fixCal_ );
    // constructor
    LAMathCapFloorConvention( const AQLString& freq_, const AQLString& spotLag_, 
                            const AQLString& daycount_, const AQLPriceDataSlidingRule& paySlr_, 
                            const AQLPriceDataCalendar& payCal_, const AQLPriceDataCalendar& fixCal_ );
    // destructor
    virtual ~LAMathCapFloorConvention();

    AQLString freq;
    AQLString spotLag;
    AQLString daycount;
    AQLPriceDataSlidingRule paySlr;
    AQLPriceDataCalendar payCal;
    AQLPriceDataCalendar fixCal;
};

class LAMathCapFloorSABR
{
public:
    // constructor
    // maturity is represented by numbers of months
    LAMathCapFloorSABR( const IntVector& expiDate, const vector<LAMathSABR_Hagan>& sabr_, AQLInterpolationBase* pInter_, 
					  const LAMathCapFloorConvention& conv_, const AQLString& curveID, 
					  AQLString foreCurveName = STD, AQLString dfCurveName = STD);
    // destructor
    virtual ~LAMathCapFloorSABR();

    LAMathSABR_Hagan getSABRParam(unsigned int pos);

    double getVol( AQLDataInstance* dataInstance, double fixingTerm, double strike );

    void calibrateToCapVol( AQLDataInstance* dataInstance, LAMathSABRLimiter sabrLimiter, 
                            const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec,
                            const IntVector& capTerm_mon, const DoubleVector& weight, 
                            const AQLString& target, DoubleMatrix& capletVol, DoubleMatrix& capVol );

    void getCapletVolMat( AQLDataInstance* dataInstance, const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec, 
                          const IntVector& capTerm_mon, DoubleMatrix& capletVol, DoubleMatrix& capVol );

    void getCapletVolMat2( AQLDataInstance* dataInstance, const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec, 
                           const IntVector& capTerm_mon, DoubleMatrix& capletVol, DoubleMatrix& capVol );
private:
    //functions for calibration
    void setCalibrationStack(AQLDataInstance* dataInstance);

    double getCapFloorPrem( unsigned int expiPos, double strike, double aveVol );

    void getCapletVol( unsigned int expiPos, unsigned int columPos, 
                       DoubleMatrix& capletVols, double strike, double aveVol );

    double getCapVol( unsigned int expiPos, unsigned int columPos, 
                      const DoubleMatrix& capletVols, double strike );

    IntVector expiDate;
    vector<LAMathSABR_Hagan> sabr;
    AQLInterpolationBase* pInter;
    LAMathCapFloorConvention conv;
	AQLString curveID;
	AQLString foreCurveName;
	AQLString dfCurveName;
    //stack member for calibration
    DoubleArray Nu;
    DoubleArray F;
    DoubleArray Te;
};

class LAMathCapVolCalibrator : public AQLFunctionVector
{
public:
    // constructor
    LAMathCapVolCalibrator( const DoubleArray& capletVol_, const DoubleArray& Te_, const DoubleArray& F_, 
                          const DoubleArray& Nu_, double strike_, double aveVol_, AQLInterpolationBase* pInter_ );
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
    AQLInterpolationBase* pInter;
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