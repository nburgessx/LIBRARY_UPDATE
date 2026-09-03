/*! @file
    @brief Class declaration to LAMathSABR.

*/
//  2009, AlgoQuantHub.
#ifndef __LAMATHSWAPTIONSABR_H__
#define __LAMATHSWAPTIONSABR_H__

#ifdef __GNUG__
#pragma interface
#endif

#include <AQLCoreTemplateType.h>
#include "AQLInterpolationBase.h"

class LAMathSwaptionMatrix
{
	friend class LAMathSwaptionVolUtility;

public:
    // constructor
    LAMathSwaptionMatrix( const DoubleMatrix& mat_, const DoubleVector& expiryVec_,
                        const DoubleVector& tenorVec_ );
    // destructor
    virtual ~LAMathSwaptionMatrix();

    double lookUpMatrix( double expiry, double tenor );

private:
    DoubleMatrix mat;
    DoubleVector tenorVec;
    DoubleVector expiryVec;
};

//class LAMathSABRAlphaMatrix : public LAMathSwaptionMatrix
//{
//public:
//     // constructor
//    LAMathSABRAlphaMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                         const DoubleVector& tenorVec );
//    // destructor
//    virtual ~LAMathSABRAlphaMatrix();
//};
//
//class LAMathSABRBetaMatrix : public LAMathSwaptionMatrix
//{
//public:
//     // constructor
//    LAMathSABRBetaMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                         const DoubleVector& tenorVec );
//    // destructor
//    virtual ~LAMathSABRBetaMatrix();
//};
//
//class LAMathSABRNuMatrix : public LAMathSwaptionMatrix
//{
//public:
//     // constructor
//    LAMathSABRNuMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                         const DoubleVector& tenorVec );
//    // destructor
//    virtual ~LAMathSABRNuMatrix();
//};
//
//class LAMathSABRRhoMatrix : public LAMathSwaptionMatrix
//{
//public:
//    // constructor
//    LAMathSABRRhoMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                       const DoubleVector& tenorVec );
//    // destructor
//    virtual ~LAMathSABRRhoMatrix();
//};

#endif