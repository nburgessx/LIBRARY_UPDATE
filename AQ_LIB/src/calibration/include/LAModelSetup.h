#ifndef LA_MODEL_SETUP_H
#define LA_MODEL_SETUP_H

#ifdef __GNUG__
#pragma interface
#endif

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


#include "LACurveSetup.h"

class LAModelSetup : public LACurveSetup
{
public:
	//==============================================================================
	static void setUpCalibMktSkewVol(LADataInstance& dataInstance,
								const LADate& asofDate,
								const LAString& fx,
								const double spotFX,
								const LAString& curveType_d,
								const LAString& curveType_f,
								const LAStringVector& terms,
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
								const LAString& type,
								const double maxTerm,
								const LAStringVector& spotOrFwd,
								const LAStringVector& isFitATM,
								const LAStringVector& termBeta,
								const LAStringVector& isDeltaNeutral);
	//==============================================================================
	static void setUpCalibHybridHWDDProcess(LADataInstance& dataInstance,
								const LADate& asofDate,
								const LAString& fx,
								const double spotFX,
								const LAString& curveType_d,
								const LAString& curveType_f,
								const DoubleMatrix& hwParams_d,
								const DoubleMatrix& hwParams_f,
								const DoubleMatrix& ddParams,
								const DoubleVector& corVec,
								const LAStringVector& terms,
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
								const LAString& type);
	//==============================================================================
protected:
	static void setUpCalibPreparation(LADataInstance &dataInstance, const LAString& fx);
};
#endif