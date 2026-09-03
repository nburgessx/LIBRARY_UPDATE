#ifndef AQL_MODEL_SETUP_H
#define AQL_MODEL_SETUP_H

#ifdef __GNUG__
#pragma interface
#endif

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


#include "AQLCurveSetup.h"

class AQLModelSetup : public AQLCurveSetup
{
public:
	//==============================================================================
	static void setUpCalibMktSkewVol(AQLDataInstance& dataInstance,
								const AQLDate& asofDate,
								const AQLString& fx,
								const double spotFX,
								const AQLString& curveType_d,
								const AQLString& curveType_f,
								const AQLStringVector& terms,
								const DoubleMatrix& weightMtx,
								const DoubleMatrix& volMtx,
								const DoubleArray& initialValues,
								const double boundaryMaxSkew,
								const double boundaryMinSkew,
								const double boundaryMaxVol,
								const double boundaryMinVol,
								unsigned int maxIteration,
								unsigned int maxStationaryStateIteration,
								const double rootEpsilon,
								const double functionEpsilon,
								const double gradientNormEpsilon,
								const AQLString& type,
								const double maxTerm,
								const AQLStringVector& spotOrFwd,
								const AQLStringVector& isFitATM,
								const AQLStringVector& termBeta,
								const AQLStringVector& isDeltaNeutral);
	//==============================================================================
	static void setUpCalibHybridHWDDProcess(AQLDataInstance& dataInstance,
								const AQLDate& asofDate,
								const AQLString& fx,
								const double spotFX,
								const AQLString& curveType_d,
								const AQLString& curveType_f,
								const DoubleMatrix& hwParams_d,
								const DoubleMatrix& hwParams_f,
								const DoubleMatrix& ddParams,
								const DoubleVector& corVec,
								const AQLStringVector& terms,
								unsigned int numSteps,
								const double boundaryMaxSigma,
								const double boundaryMinSigma,
								const double boundaryMaxBeta,
								const double boundaryMinBeta,
								unsigned int maxIteration,
								unsigned int maxStationaryStateIteration,
								const double rootEpsilon,
								const double functionEpsilon,
								const double gradientNormEpsilon,
								const bool isCalib,
								const AQLString& type);
	//==============================================================================
protected:
	static void setUpCalibPreparation(AQLDataInstance &dataInstance, const AQLString& fx);
};
#endif