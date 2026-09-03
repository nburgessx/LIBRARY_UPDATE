#ifndef LAMathCapFloorVolUtility_h
#define LAMathCapFloorVolUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include<AQLDataInstance.h>
#include<AQLCoreTemplateType.h>
#include<AQLString.h>

class LAMathCapFloorVolUtility
{
public:
	static void setUpCapFloorVolSurf( AQLDataInstance* dataInstance, const AQLString& capFloorID,
        AQLStringMatrix convMat, const DoubleMatrix& volMat, const DoubleVector& strike,
        AQLStringVector capTerm, const DoubleMatrix& sabrParamMat, AQLStringMatrix sabrLimiter,
        AQLString target, AQLString interpo, const DoubleVector& weight, bool isSABR,
		const AQLString& curveID, AQLString foreCurveName = STD, 
		AQLString dfCurveName = STD);

    static void outPutCapFloor( AQLDataInstance* dataInstance, const AQLString& capFloorID, AQLString capFloorFlag, 
        DoubleVector& ret, size_t& row, size_t& colum);

    static IntVector changeDateFromStringToMonth( const AQLStringVector& expiryDate );

    static AQLStringVector changeDateFromMonthToString(const IntVector& expiryMonth);

    static double lookUpCapFloorVol( AQLDataInstance* dataInstance, const AQLString& capFloorID,double strike,
        const AQLDate& fixingDate );

private:
    LAMathCapFloorVolUtility(void);
    ~LAMathCapFloorVolUtility(void);
    LAMathCapFloorVolUtility(const LAMathCapFloorVolUtility &rhs);
    LAMathCapFloorVolUtility &operator=(const LAMathCapFloorVolUtility &rhs);
};

#endif