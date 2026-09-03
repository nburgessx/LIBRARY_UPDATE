#ifndef MFParameterUtility_h
#define MFParameterUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include <AQLDataInstance.h>
#include "AQLPriceDataSlidingRule.h"

#ifndef AQ_DATAMATRIX
#define AQ_DATAMATRIX "DataMatrix"
#endif
#ifndef AQ_EXPIRYTERMS
#define AQ_EXPIRYTERMS "ExpiryTerms"
#endif
#ifndef AQ_EXPIRYDATES
#define AQ_EXPIRYDATES "ExpiryDates"
#endif
#ifndef AQ_EXPIRYVECTOR
#define AQ_EXPIRYVECTOR "ExpiryVector"
#endif
#ifndef AQ_INDEXVECTOR
#define AQ_INDEXVECTOR "IndexVector"
#endif

using namespace std;

class LAMathParameterObject
{
public:
    static void SetParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID, const AQLString& convID, AQLStringMatrix& mat);

    static AQLStringMatrix ParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID);

    // By expiry term
    static double LookUpParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID, const AQLString& expTerm, const AQLString& idxPoint,
                                        AQLString interpolationType = "Linear");

    // By expiry date
    static double LookUpParameterMatrix(AQLDataInstance* dataInstance, const AQLString& matID, AQLDate expDate, const AQLString& idxPoint,
                                        AQLString interpolationType = "Linear");

    static AQLStringVector ParameterMatrixTerms(AQLDataInstance* dataInstance, const AQLString& matID);

    static AQLStringVector ParameterMatrixIndexes(AQLDataInstance* dataInstance, const AQLString& matID);

    static AQLDate GetDate(const AQLDate& asOfDate, AQLString term, const AQLPriceDataSlidingRule& slr, const AQLPriceDataCalendar& cal);

    static void SetMatrixAxis(const AQLString& title, const AQLStringVector& terms, const AQLStringVector& indexes,
                              AQLStringMatrix& matrix);

    static void SetMatrixData(const DoubleMatrix& data, AQLStringMatrix& matrix);

private:
    static double Lookup(AQLDate asOfDate, AQLDate expDate, const AQLString& idxPoint,
                                             const DoubleVector& expiryVec, const AQLStringVector& indexVec,
                                             const DoubleMatrix& dataMatrix, AQLString interpolationType);

};

class MFParameterMatrix
{
public:
    MFParameterMatrix(const DoubleMatrix& mat_, const DoubleVector& expiryVec_, const AQLStringVector& idxVec_);

    double LookUp(double expiry, const AQLString& idxID, AQLString interpolationType);

private:
    DoubleMatrix mMatrix;
    DoubleVector mExpiries;
    AQLStringVector mIndexes;
};

#endif 
