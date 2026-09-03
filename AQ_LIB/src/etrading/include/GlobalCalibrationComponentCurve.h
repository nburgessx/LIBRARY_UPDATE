/*
 * @brief			Base class for component curves used in global yield curve calibration
 * @Created:		22 Jan 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LAObject.h"
#include "CurveCalibrationData.h"

class LAInterpolationBase;

/*! 
    @brief Structure for parameters used in solving
*/
struct SolverParams
{
	double delta_;
	double grad_eps_;
	double maxLoop_;
	double eps_;
	bool fastRebuildRequested_;
};


/*! 
    @brief Class of GlobalCalibrationComponentCurve
*/
class GlobalCalibrationComponentCurve
{
public:

	// Constructor
	GlobalCalibrationComponentCurve(const LAString& curveName, const LADate& baseDate, const bool& fastRebuildRequested = false);

	// Destructor
	virtual ~GlobalCalibrationComponentCurve() {}

	// Initialisation
	virtual void initialise() = 0;

	// Calibration routine by pricing instruments
	virtual void priceCalibrationInstruments(DoubleVector& allPVs) = 0;

	// Post processing results once instruments have been consumed in calibration steps
	virtual void postProcessing(LAObject& yieldCurveProEntity) = 0;

	//==============================================================================

	// Perturb a single rate point in the target curve by delta
	void perturbSingleRatePoint(unsigned int pos, double delta);

	// Update the rate of all tenors in target curve
	void updateAllRatePoints(double delta);

	// Update state varibale interpolator
	void update();
	
	// Set DF interpolator	
	void setDFInterpolator(const std::shared_ptr<LAInterpolationBase>& dfInterp);

	// Get state variable interpolator
	std::shared_ptr<LAInterpolationBase> getStateVarInterp() const { return pInter_StateVariable_;}

	// Get the state variables
	void getStateVariableRates(DoubleArray& stateVariables);

	// Get number of calibration instruments in solving
	unsigned int getSolvingInstrumentCount() const	{ return swapCount_; }

	// Get number of instruments that sit before the solving instruments
	unsigned int getPreSolvingInstrumentCount() const	{ return preSwapSize_; }

	// Get curve name
	const LAString getCurveName() const	{ return curveName_;}

	// Get state variable rates
	const DoubleArray& getStateVariableRates() const { return stateVariable_rates_;}

	// Get state variable grid
	const DoubleArray& getStateVariableGrid() const { return stateVariable_grid_;}

	// Set state variable rates
	void setStateVariableRates(const DoubleVector& rates);

protected:
	
	// Update state varibale interpolator
	void update(const DoubleVector& rates);

	//==============================================================================

	std::shared_ptr<LAInterpolationBase> pInter_StateVariable_;
	std::shared_ptr<LAInterpolationBase> pInter_DF_;

	DoubleArray stateVariable_grid_;
	DoubleArray stateVariable_rates_;

	size_t preSwapSize_;		// The number of instruments prior to the swaps that get solved 
	size_t swapCount_;			// The number of swaps that are solved 

	LAString curveName_;
	LADate baseDate_;

	bool fastRebuild_;
};

// Custom object type when used with shared pointer
typedef std::shared_ptr<GlobalCalibrationComponentCurve> GlobalCalibrationComponentCurvePtr;

