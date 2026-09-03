/*
 * @brief			Class acting as an engine that provides simultaneous global yield curve calibration capabilities
 * @Created:		22 Jan 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#include "GlobalCurveCalibrationEngine.h"
#include "LABasic.h"
#include "LAObjectHolder.h"


/*!
	@brief Default constructor
*/
GlobalCurveCalibrationEngine::GlobalCurveCalibrationEngine(const CurveCalibrationData& curveCalibrationData, const LAString& engineName)
	: totalCalibCount_(0), 
	numComponentCurves_(0),
	engineName_(engineName),
	previousJacobianAvailable_(false),
	jacobianLabelsCreated_(false),
	newJacobianAvailable_(false)
{
	engineSuffix_ = "_" + engineName;
	engineSuffix_.toUpper();

	// Yield curve data as a reference
	yieldDataRef_ = curveCalibrationData.getYieldData();
	LAObjectHolder objHolder = yieldDataRef_.get();

	calibSwapCounts_.clear();
	componentCurves_.clear();
	jacobian_.clearValues();
	inverseJacobian_.clearValues();
	jacobianLabels_.clear();

	eps_ = 1.0e-9;
	const LADataHolder *dh = &objHolder.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON + engineSuffix_, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		eps_ = dynamic_cast<const LADataDouble &>(dh->get()).get();
	}

	grad_eps_ = 1.0e-15;
	dh = &objHolder.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON + engineSuffix_, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		grad_eps_ = dynamic_cast<const LADataDouble &>(dh->get()).get();
	}

	delta_ = 1.0e-10;
	dh = &objHolder.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA + engineSuffix_, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		delta_ = dynamic_cast<const LADataDouble &>(dh->get()).get();
	}

	max_loop_ = 1000;
	dh = &objHolder.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP + engineSuffix_, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		max_loop_ = dynamic_cast<const LADataInt &>(dh->get()).get();
	}

	fastRebuild_ = true;
	dh = &objHolder.getData(IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD + engineSuffix_, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		fastRebuild_ = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
}

/*!
	@brief Start the calibration process on all the curves
*/
void GlobalCurveCalibrationEngine::calibrate()
{
	previousInverseJacobian_.clear();
	if (fastRebuild_)
	{
		// Now attempt to fetch the jacobian (gradient matrix)
		LAObjectHolder objHolder = yieldDataRef_.get();
		const LADataHolder* ahJacobian = &(objHolder.getData(IR_CALIBRATION_DATA_INVERSE_ENGINE_JACOBIAN + engineSuffix_, NOCHECK));
		if (ahJacobian->isDefined() && !ahJacobian->isNull())
		{
			previousInverseJacobian_ = dynamic_cast<const LADataDoubleMatrix &>(ahJacobian->get()).get();
			// Sanity check: Verify the previous jacobian has the correct size
			if ((previousInverseJacobian_.size() == totalCalibCount_) && (previousInverseJacobian_[0].size() == totalCalibCount_))
			{
				previousJacobianAvailable_ = true;
			}
		}
	}

	DoubleVector allPVs_old;
	DoubleVector allPVs_new;

	if (!previousJacobianAvailable_)		
	{
		// Calculate old PVs of all curves
		for (unsigned int i = 0; i < numComponentCurves_; ++i)
		{
			DoubleVector allPVs;
			componentCurves_[i]->priceCalibrationInstruments(allPVs);
			allPVs_old.insert(allPVs_old.end(), allPVs.begin(), allPVs.end());
		}

		// Bump all curves
		for (unsigned int i = 0; i < numComponentCurves_; ++i)
		{
			componentCurves_[i]->updateAllRatePoints(delta_);
		}
	}

	// Calculate new PVs of all curves
	for(unsigned int i = 0; i <numComponentCurves_; ++i)
	{
		DoubleVector allPVs;
		componentCurves_[i]->priceCalibrationInstruments(allPVs);
		allPVs_new.insert(allPVs_new.end(), allPVs.begin(), allPVs.end());		
	}

	// Start multi curve solving
	int loop = max_loop_;
	bool solutionFound = false;	
	while(loop--)
	{
		unsigned int columnShift = 0;
		bool isEnd = true;
		inverseJacobian_.resize(totalCalibCount_, totalCalibCount_);
		jacobian_.resize(totalCalibCount_, totalCalibCount_);

		if (loop == max_loop_ - 1 && previousJacobianAvailable_)
		{
			// Retrieve the initialInverseJacobian matrix if we have been requested to use it.
			// A small change in input variables can often be solved for using the initial estimate of the gradient
			// and a single matrix-multiply, without needing to re-run the full Newton-Raphson loop.
			for (unsigned int i = 0; i < totalCalibCount_; ++i)
			{
				const DoubleVector& dataVec = previousInverseJacobian_[i];
				for (unsigned int j = 0; j < totalCalibCount_; ++j)
				{
					const double value = dataVec[j];
					inverseJacobian_.setValue(j, i, value);
				}
			}
		}
		else
		{

			// -----------------------------------------------------------------------------------------
			// Check whether solving can no longer produce meaningful difference to the output. 
			for (size_t i = 0; i < totalCalibCount_; ++i)
			{
				double diff = LAMath::abs(allPVs_new[i] - allPVs_old[i]);
				if (diff >= grad_eps_)
				{
					isEnd = false;
					break;
				}
			}

			if (isEnd)
			{
				break;
			}

			// -----------------------------------------------------------------------------------------
			// Perturb x by amount 'delta' in order to calculate y_after_bump to obtain Jacobian matrix		
			jacobian_.clearValues();

			// Loop through each instrument of each component curve; Calculate the sensitivity of the PV 
			// of each instrument (of all the curves) against the price move of each other 
			// instrument (of all the curves).
			
			for (size_t i = 0; i < numComponentCurves_; ++i)
			{
				// Keep the unbumped values
				DoubleArray unbumpedRates = componentCurves_[i]->getStateVariableRates();
				const LAString curveName = componentCurves_[i]->getCurveName();
				unsigned int preSwapSize = componentCurves_[i]->getPreSolvingInstrumentCount();
				
				// Calculate PV sensitivities of every instrument of every curve against the current curve's instruments 			
				unsigned int calibIntCount = calibSwapCounts_[i];
				for (size_t j = 0; j < calibIntCount; ++j)
				{
					LAString temp = "/d(rate_" + curveName + "_" + LAString(static_cast<int>(j)) + ")";

					// Update rate at the jth point of the ith component curve 
					DoubleArray tempRates(unbumpedRates);
					tempRates[preSwapSize + j] += delta_;
					componentCurves_[i]->setStateVariableRates(tempRates);

					// Reprice all the curves after rate adjustment
					DoubleVector allPVs_bumped;
					LAStringVector labels;
					for (size_t k = 0; k < numComponentCurves_; ++k)
					{
						DoubleArray allPVs;
						componentCurves_[k]->priceCalibrationInstruments(allPVs);						
						allPVs_bumped.insert(allPVs_bumped.end(), allPVs.begin(), allPVs.end());

						if (!jacobianLabelsCreated_)
						{
							// Set up one label for each matrix element
							LAString curve = componentCurves_[k]->getCurveName();
							for (size_t s = 0; s < allPVs.size(); ++s)
							{
								LAString elemetLabel = "d(PV(" + curve + "_" + LAString(static_cast<int>(s)) + "))" + temp;
								labels.push_back(elemetLabel);
							}
						}
					}

					// Calculate Jacobian matrix
					for (size_t h = 0; h < allPVs_bumped.size(); ++h)
					{
						const double firstOrderDerivative = (allPVs_bumped[h] - allPVs_new[h]) / delta_;
						jacobian_.setValue(h, columnShift + j, firstOrderDerivative);
						
						if (!jacobianLabelsCreated_)
						{
							LAString label = labels[h];
							jacobianLabels_.set(columnShift + j, h, label);
						}
					}
				}

				columnShift += calibIntCount;

				// Reset the ith component curve
				componentCurves_[i]->setStateVariableRates(unbumpedRates);
			}

			#ifdef _DEBUG			
			DoubleMatrix tempJacobian;
			for (size_t i = 0; i < jacobian_.row(); ++i)
			{
				DoubleArray temp;
				for (size_t j = 0; j < jacobian_.row(); ++j)
				{
					temp.push_back(jacobian_.getValue(j,i));
				}
				tempJacobian.push_back(temp);
			}
			#endif
						
			inverseJacobian_ = jacobian_.inverseMatrix();
			jacobianLabelsCreated_ = true;
			newJacobianAvailable_ = true;
		}

		// -----------------------------------------------------------------------
		// Update x as in y = f(x) according to Newton Raphson
		LAMatrix valMat(allPVs_new);

		LAMatrix deltaMat = inverseJacobian_ * valMat;

		#ifdef _DEBUG
		DoubleMatrix tempInverseJ;
		for (size_t i = 0; i < inverseJacobian_.row() ; ++i)
		{
			DoubleArray temp;
			for (size_t j = 0; j < inverseJacobian_.row() ; ++j)
			{
				temp.push_back(inverseJacobian_.getValue(j,i));
			}
			tempInverseJ.push_back(temp);
		}
		
		DoubleArray temp;
		for (size_t i = 0; i < deltaMat.row(); ++i)
		{
			temp.push_back(deltaMat.getValue(i, 0));
		}
		#endif
				
		columnShift = 0;
		for(size_t i = 0; i < numComponentCurves_; ++i)
		{
			unsigned int calibIntCount = calibSwapCounts_[i];
			for(size_t j = 0; j < calibIntCount; ++j)
			{
				componentCurves_[i]->perturbSingleRatePoint(j, -1 * deltaMat.getValue(columnShift + j, 0));
			}

			componentCurves_[i]->update();

			columnShift += calibIntCount;
		}

		// -----------------------------------------------------------------------
		// Calculate new y as in y = f(x) now that we've obtained a new x
		allPVs_old = allPVs_new;
		allPVs_new.clear();
		
		for(unsigned int i = 0; i <numComponentCurves_; ++i)
		{
			DoubleVector allPVs;
			componentCurves_[i]->priceCalibrationInstruments(allPVs);
			allPVs_new.insert(allPVs_new.end(), allPVs.begin(), allPVs.end());		
		}
	
		// Check if solution is found
		solutionFound = true;
		for (size_t i = 0; i < totalCalibCount_; ++i)		
		{
			// A solution is considered acceptable only when all calibration instruments reprice to zero
			if ( LAMath::abs( allPVs_new[i] ) >= eps_ )
			{
				solutionFound = false;
				break;
			}
		}

		if (solutionFound)
		{
			break;
		}
	}

	if (!solutionFound)
	{
		throw LACoreInvalidData("#Error: Global yield curve calibration engine can't converge in newton raphson method!", __FILE__, __LINE__);
	}

}

/*!
	@brief Add component curve to the calibration engine
	@param[in] curve Component curve
*/
void GlobalCurveCalibrationEngine::addComponentCurve(const GlobalCalibrationComponentCurvePtr& curve) 
{
	// Keep curve
	componentCurves_.push_back(curve); 
	numComponentCurves_++;

	// Keep calibration instrument count
	unsigned int calibIntCount = curve->getSolvingInstrumentCount();
	calibSwapCounts_.push_back(calibIntCount);
	
	totalCalibCount_ += calibIntCount;
}

/*!
	@brief Post processing calibration results on each curve
	@param[in] yieldCurveProEntity	Object object for CurveCalibrationData
*/
void GlobalCurveCalibrationEngine::postProcessing(LAObject& yieldCurveProEntity)
{
	// Store inverse jacobian
	DoubleMatrix inverseJacobianAtSolution;
	DoubleMatrix jacobianAtSolution;
	for (unsigned int i = 0; i < totalCalibCount_; ++i)
	{
		std::vector<double> dataVecInv;
		std::vector<double> dataVec;
		for (unsigned int j = 0; j < totalCalibCount_; ++j)
		{
			dataVecInv.push_back(inverseJacobian_.getValue(j, i));
			dataVec.push_back(jacobian_.getValue(j, i));
		}
		inverseJacobianAtSolution.push_back(dataVecInv);
		jacobianAtSolution.push_back(dataVec);
	}

	LAObjectHolder objHolder = yieldDataRef_.get();
	if (newJacobianAvailable_)
	{
		objHolder.remove(IR_CALIBRATION_DATA_INVERSE_ENGINE_JACOBIAN + engineSuffix_);
		objHolder.add(IR_CALIBRATION_DATA_INVERSE_ENGINE_JACOBIAN + engineSuffix_, new LADataDoubleMatrix(inverseJacobianAtSolution));

		objHolder.remove(IR_CALIBRATION_DATA_ENGINE_JACOBIAN + engineSuffix_);
		objHolder.add(IR_CALIBRATION_DATA_ENGINE_JACOBIAN + engineSuffix_, new LADataDoubleMatrix(jacobianAtSolution));
	}

	if (jacobianLabelsCreated_)
	{
		objHolder.remove(IR_CALIBRATION_DATA_ENGINE_JACOBIAN_LABEL + engineSuffix_);
		objHolder.add(IR_CALIBRATION_DATA_ENGINE_JACOBIAN_LABEL + engineSuffix_, new LADataStringMatrix(jacobianLabels_));
	}

	// Loop through all component curves and delegate 
	for(unsigned int i = 0; i <numComponentCurves_; ++i)
	{
		componentCurves_[i]->postProcessing(yieldCurveProEntity);
	}
}
