#pragma once

#include "GlobalCalibrationComponentCurve.h"
#include "AQLDataReference.h"
#include "AQLDataMatrix.h"
#include "AQLMatrix.h"
#include <vector>
#include <map>

/*! 
    @brief Class of GlobalCurveCalibrationEngine
*/
class GlobalCurveCalibrationEngine
{
public:
		
	// constructor
	GlobalCurveCalibrationEngine(const CurveCalibrationData& curveCalibrationData, const AQLString& engineName);

	// destructor
	~GlobalCurveCalibrationEngine() {}
    
	// Add component curves
	void addComponentCurve(const GlobalCalibrationComponentCurvePtr& curve); 

	// Calibrate curves
	void calibrate();

	// Post-process results
	void postProcessing(AQLObject& yieldCurveProEntity);
	
private:

	// All component curves are simply the observables in this observer pattern
	std::vector<GlobalCalibrationComponentCurvePtr> componentCurves_;

	std::vector<unsigned int> calibSwapCounts_;

	unsigned int numComponentCurves_;

	unsigned int totalCalibCount_;
		
	SolverParams solverParams_;

	AQLDataReference yieldDataRef_;

	AQLString engineName_;
	AQLString engineSuffix_;

	// Solver params
	double eps_;
	double delta_;
	double grad_eps_;
	int max_loop_;
	bool fastRebuild_;

	DoubleMatrix previousInverseJacobian_;
	bool previousJacobianAvailable_;
	AQLMatrix inverseJacobian_;
	AQLMatrix jacobian_;

	AQLDataStringMatrix jacobianLabels_;

	bool jacobianLabelsCreated_;

	bool newJacobianAvailable_;
};


// Custom object type when used with shared pointer
typedef std::shared_ptr<GlobalCurveCalibrationEngine> GlobalCurveCalibrationEnginePtr;


