#ifndef MFParameterUtility_h
#define MFParameterUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include <LADataInstance.h>
#include "LAPriceDataSlidingRule.h"

#ifndef MLIB_DATAMATRIX
#define MLIB_DATAMATRIX "DataMatrix"
#endif
#ifndef MLIB_EXPIRYTERMS
#define MLIB_EXPIRYTERMS "ExpiryTerms"
#endif
#ifndef MLIB_EXPIRYDATES
#define MLIB_EXPIRYDATES "ExpiryDates"
#endif
#ifndef MLIB_EXPIRYVECTOR
#define MLIB_EXPIRYVECTOR "ExpiryVector"
#endif
#ifndef MLIB_INDEXVECTOR
#define MLIB_INDEXVECTOR "IndexVector"
#endif

using namespace std;

class LAMathParameterObject
{
public:
    static void SetParameterMatrix(LADataInstance* dataInstance, const LAString& matID, const LAString& convID, LAStringMatrix& mat);

    static LAStringMatrix ParameterMatrix(LADataInstance* dataInstance, const LAString& matID);

    // By expiry term
    static double LookUpParameterMatrix(LADataInstance* dataInstance, const LAString& matID, const LAString& expTerm, const LAString& idxPoint,
                                        LAString interpolationType = "Linear");

    // By expiry date
    static double LookUpParameterMatrix(LADataInstance* dataInstance, const LAString& matID, LADate expDate, const LAString& idxPoint,
                                        LAString interpolationType = "Linear");

    static LAStringVector ParameterMatrixTerms(LADataInstance* dataInstance, const LAString& matID);

    static LAStringVector ParameterMatrixIndexes(LADataInstance* dataInstance, const LAString& matID);

    static LADate GetDate(const LADate& asOfDate, LAString term, const LAPriceDataSlidingRule& slr, const LAPriceDataCalendar& cal);

    static void SetMatrixAxis(const LAString& title, const LAStringVector& terms, const LAStringVector& indexes,
                              LAStringMatrix& matrix);

    static void SetMatrixData(const DoubleMatrix& data, LAStringMatrix& matrix);

private:
    static double Lookup(LADate asOfDate, LADate expDate, const LAString& idxPoint,
                                             const DoubleVector& expiryVec, const LAStringVector& indexVec,
                                             const DoubleMatrix& dataMatrix, LAString interpolationType);

};

class MFParameterMatrix
{
public:
    MFParameterMatrix(const DoubleMatrix& mat_, const DoubleVector& expiryVec_, const LAStringVector& idxVec_);

    double LookUp(double expiry, const LAString& idxID, LAString interpolationType);

private:
    DoubleMatrix mMatrix;
    DoubleVector mExpiries;
    LAStringVector mIndexes;
};

#endif 
