/*
 * @brief			Base class for component curves used in global yield curve calibration
 * @Created:		22 Jan 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#include "GlobalCalibrationComponentCurve.h"
#include "LAPriceDataInterpolation.h"

// constructor
GlobalCalibrationComponentCurve::GlobalCalibrationComponentCurve(const LAString& curveName,
																const LADate& baseDate,
																const bool& fastRebuildRequested)
		: curveName_(curveName),
		  baseDate_(baseDate),
		  preSwapSize_(0), swapCount_(0), fastRebuild_(fastRebuildRequested)
{
	stateVariable_grid_.clear();
	stateVariable_rates_.clear();

	pInter_StateVariable_.reset();
	pInter_DF_.reset();
}

// Perturb a single rate point in the target curve by delta
void GlobalCalibrationComponentCurve::perturbSingleRatePoint(unsigned int pos, double delta)
{
	stateVariable_rates_[preSwapSize_ + pos] += delta;	
}

// Update the rate of all tenors in target curve
void GlobalCalibrationComponentCurve::updateAllRatePoints(double delta)
{
	for(size_t i = 0; i < swapCount_; ++i)
	{
		stateVariable_rates_[preSwapSize_ + i] += delta;
	}
	update();
}

// Set DF interpolator	
void GlobalCalibrationComponentCurve::setDFInterpolator(const std::shared_ptr<LAInterpolationBase>& dfInterp) 
{
	//Don't use reset or clone method here as that is not the intended behavior
	pInter_DF_ = dfInterp;
}	

// Get the state variables
void GlobalCalibrationComponentCurve::getStateVariableRates(DoubleArray& stateVariables)
{
	stateVariables = stateVariable_rates_;
}

// Update state varibale interpolator
void GlobalCalibrationComponentCurve::update()
{
	pInter_StateVariable_->set(stateVariable_grid_, stateVariable_rates_);
}

// Update state varibale interpolator
void GlobalCalibrationComponentCurve::update(const DoubleVector& rates)
{
	pInter_StateVariable_->set(stateVariable_grid_, rates);
}

// Set state variable rates
void GlobalCalibrationComponentCurve::setStateVariableRates(const DoubleVector& rates) 
{
	update(rates);
}