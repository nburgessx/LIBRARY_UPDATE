#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <AQLCoreTemplateType.h>
#include "LAMathBaseFuncUtility.h"
#include "LAMathInterpolationUtilities.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "LAMathSABR.h"
#include "LAMathSwaptionSABR.h"

LAMathSwaptionMatrix::LAMathSwaptionMatrix( const DoubleMatrix& mat_, const DoubleVector& expiryVec_,
                                        const DoubleVector& tenorVec_ )
        :mat(mat_),tenorVec(tenorVec_),expiryVec(expiryVec_) {}

LAMathSwaptionMatrix::~LAMathSwaptionMatrix(){}

double
LAMathSwaptionMatrix::lookUpMatrix( double expiry, double tenor )
{
	if(expiry<0. || tenor<0.) throw AQLCoreInvalidData("expiry or tenor is negative!",__FILE__,__LINE__);

    size_t size_te = tenorVec.size();
    size_t size_ex = expiryVec.size();

	unsigned int index_te;
	if(tenorVec.front() >= tenor) index_te = 0;
	else if(tenorVec.back() <= tenor) index_te = size_te;
	else index_te = LAMathInterpolationUtilities::searchIndex(tenorVec, tenor);

	unsigned int index_ex;
	if(expiryVec.front()>=expiry) index_ex = 0;
	else if(expiryVec.back()<=expiry) index_ex = size_ex;
	else index_ex = LAMathInterpolationUtilities::searchIndex(expiryVec, expiry);

    double ret;
	if	   (index_ex==0		  && index_te==0	  ) { ret = mat[0][0];					}
	else if(index_ex==size_ex && index_te==0	  ) { ret = mat[size_ex-1][0];			}
	else if(index_ex==size_ex && index_te==size_te) { ret = mat[size_ex-1][size_te-1];	}
	else if(index_ex==0		  && index_te==size_te) { ret = mat[0][size_te-1];			}
	else if(index_ex==0) 
	{
		ret = LAMathInterpolationUtilities::getLinearInterpolation(tenorVec[index_te-1],tenorVec[index_te],
					mat[0][index_te-1],mat[0][index_te],tenor);
	}
	else if(index_ex==size_ex) 
	{
		ret = LAMathInterpolationUtilities::getLinearInterpolation(tenorVec[index_te-1],tenorVec[index_te],
					mat[size_ex-1][index_te-1],mat[size_ex-1][index_te],tenor);
	}
	else if(index_te==0) 
	{
		ret = LAMathInterpolationUtilities::getLinearInterpolation(expiryVec[index_ex-1],expiryVec[index_ex],
					mat[index_ex-1][0],mat[index_ex][0],expiry);
	}
	else if(index_te==size_te) 
	{
		ret = LAMathInterpolationUtilities::getLinearInterpolation(expiryVec[index_ex-1],expiryVec[index_ex],
					mat[index_ex-1][size_te-1],mat[index_ex][size_te-1],expiry);
	}
	else
	{
		double point1 = LAMathInterpolationUtilities::getLinearInterpolation(tenorVec[index_te-1],tenorVec[index_te],
								mat[index_ex-1][index_te-1],mat[index_ex-1][index_te],tenor);
		double point2 = LAMathInterpolationUtilities::getLinearInterpolation(tenorVec[index_te-1],tenorVec[index_te],
								mat[index_ex][index_te-1],mat[index_ex][index_te],tenor);
		ret =  LAMathInterpolationUtilities::getLinearInterpolation(expiryVec[index_ex-1],expiryVec[index_ex],point1,point2,expiry);
	}
    return ret; 
}

//LAMathSABRAlphaMatrix::LAMathSABRAlphaMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                                          const DoubleVector& tenorVec )
//                                          : LAMathSwaptionMatrix(mat, expiryVec, tenorVec)
//{
//    for(size_t i=0; i<mat.size(); i++)
//        for(size_t j=0; j<mat[0].size(); j++)
//            if(mat[i][j]<-0.00001) throw AQLCoreInvalidData("alpha is negative.",__FILE__,__LINE__);
//}
//LAMathSABRAlphaMatrix::~LAMathSABRAlphaMatrix(){}
//
//LAMathSABRBetaMatrix::LAMathSABRBetaMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                                        const DoubleVector& tenorVec )
//                     : LAMathSwaptionMatrix(mat, expiryVec, tenorVec)
//{
//    for(size_t i=0; i<mat.size(); i++)
//        for(size_t j=0; j<mat[0].size(); j++)
//            if(mat[i][j]<-0.0000001 || mat[i][j]>1.0000001) 
//                throw AQLCoreInvalidData("beta is in [0:1].",__FILE__,__LINE__);
//}
//LAMathSABRBetaMatrix::~LAMathSABRBetaMatrix(){}
//
//LAMathSABRNuMatrix::LAMathSABRNuMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                                    const DoubleVector& tenorVec )
//                                    : LAMathSwaptionMatrix(mat, expiryVec, tenorVec)
//{
//    for(size_t i=0; i<mat.size(); i++)
//        for(size_t j=0; j<mat[0].size(); j++)
//            if(mat[i][j]<-0.0000001 ) throw AQLCoreInvalidData("gamma is negative.",__FILE__,__LINE__);
//}
//LAMathSABRNuMatrix::~LAMathSABRNuMatrix(){}
//
//LAMathSABRRhoMatrix::LAMathSABRRhoMatrix( const DoubleMatrix& mat, const DoubleVector& expiryVec,
//                                      const DoubleVector& tenorVec )
//                                      : LAMathSwaptionMatrix(mat, expiryVec, tenorVec)
//{
//    for(size_t i=0; i<mat.size(); i++)
//        for(size_t j=0; j<mat[0].size(); j++)
//            if(mat[i][j]<-1.0000001 || mat[i][j]>1.0000001) 
//                throw AQLCoreInvalidData("beta is in [^-1:1].",__FILE__,__LINE__);
//}
//LAMathSABRRhoMatrix::~LAMathSABRRhoMatrix(){}
