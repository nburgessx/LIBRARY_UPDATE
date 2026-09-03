#ifndef LAMathCapFloorVolUtility_h
#define LAMathCapFloorVolUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include<LADataInstance.h>
#include<LACoreTemplateType.h>
#include<LAString.h>

class LAMathCapFloorVolUtility
{
public:
	static void setUpCapFloorVolSurf( LADataInstance* dataInstance, const LAString& capFloorID,
        LAStringMatrix convMat, const DoubleMatrix& volMat, const DoubleVector& strike,
        LAStringVector capTerm, const DoubleMatrix& sabrParamMat, LAStringMatrix sabrLimiter,
        LAString target, LAString interpo, const DoubleVector& weight, bool isSABR,
		const LAString& curveID, LAString foreCurveName = STD, 
		LAString dfCurveName = STD);

    static void outPutCapFloor( LADataInstance* dataInstance, const LAString& capFloorID, LAString capFloorFlag, 
        DoubleVector& ret, size_t& row, size_t& colum);

    static IntVector changeDateFromStringToMonth( const LAStringVector& expiryDate );

    static LAStringVector changeDateFromMonthToString(const IntVector& expiryMonth);

    static double lookUpCapFloorVol( LADataInstance* dataInstance, const LAString& capFloorID,double strike,
        const LADate& fixingDate );

private:
    LAMathCapFloorVolUtility(void);
    ~LAMathCapFloorVolUtility(void);
    LAMathCapFloorVolUtility(const LAMathCapFloorVolUtility &rhs);
    LAMathCapFloorVolUtility &operator=(const LAMathCapFloorVolUtility &rhs);
};

#endif