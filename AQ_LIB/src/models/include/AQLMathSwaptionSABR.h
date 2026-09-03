/*! @file
    @brief Class declaration to AQLMathSABR.

*/
//  2009, AlgoQuantHub.
#ifndef __LAMATHSWAPTIONSABR_H__
#define __LAMATHSWAPTIONSABR_H__

#ifdef __GNUG__
#pragma interface
#endif

#include <AQLCoreTemplateType.h>
#include "AQLInterpolationBase.h"

class AQLMathSwaptionMatrix
{
	friend class AQLMathSwaptionVolUtility;

public:
    // constructor
    AQLMathSwaptionMatrix( const DoubleMatrix& mat_, const DoubleVector& expiryVec_,
                        const DoubleVector& tenorVec_ );
    // destructor
    virtual ~AQLMathSwaptionMatrix();

    double lookUpMatrix( double expiry, double tenor );

private:
    DoubleMatrix mat;
    DoubleVector tenorVec;
    DoubleVector expiryVec;
};

//class AQLMathSABRAlphaMatrix : public AQLMathSwaptionMatrix
//{
//public:
//     // constructor
//    AQLMathSABRAlphaMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                         const DoubleVector& tenorVec );
//    // destructor
//    virtual ~AQLMathSABRAlphaMatrix();
//};
//
//class AQLMathSABRBetaMatrix : public AQLMathSwaptionMatrix
//{
//public:
//     // constructor
//    AQLMathSABRBetaMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                         const DoubleVector& tenorVec );
//    // destructor
//    virtual ~AQLMathSABRBetaMatrix();
//};
//
//class AQLMathSABRNuMatrix : public AQLMathSwaptionMatrix
//{
//public:
//     // constructor
//    AQLMathSABRNuMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                         const DoubleVector& tenorVec );
//    // destructor
//    virtual ~AQLMathSABRNuMatrix();
//};
//
//class AQLMathSABRRhoMatrix : public AQLMathSwaptionMatrix
//{
//public:
//    // constructor
//    AQLMathSABRRhoMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                       const DoubleVector& tenorVec );
//    // destructor
//    virtual ~AQLMathSABRRhoMatrix();
//};

#endif