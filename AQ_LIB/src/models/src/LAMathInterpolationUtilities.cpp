#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathInterpolationUtilities.h"
#include "LABasic.h"
#include "LAMonotoneConvexInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAParabolicInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "LAConstrainedSplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LALinearInterpolation.h"
#include "LAInterpolationBase.h"
#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LAPriceDataInterpolation.h"
#include <cmath>
#include <map>
#include "LACoreComponentManager.h"
#include "LAFunctionUtilities.h"
using namespace std;


double LAMathInterpolationUtilities::interpolate( const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::interpolate( xValues, yValues, xPoint, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

double LAMathInterpolationUtilities::interpolate( const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	return LAMathInterpolationUtilities::interpolate( xValues, yValues, std::vector<double>(1, xPoint), interpolationEnum, joinXValue )[0];
}

std::vector<double> LAMathInterpolationUtilities::interpolate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::interpolate( xValues, yValues, xPoints, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

std::vector<double> LAMathInterpolationUtilities::interpolate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	const size_t nPoints = xPoints.size();
	AQ_REQUIRE( nPoints != 0 , "Unable to Interpolate: No interpolation search points were requested, Interpolation xPoints parameter is empty" )
	
	// Build the interpolator object	
	std::shared_ptr<LAInterpolationBase> interpolator = buildInterpolator( xValues, yValues, interpolationEnum, joinXValue );
	
	// Get the interpolated values
	std::vector<double> results( nPoints, 0.0 );
	for( size_t i = 0; i < nPoints; ++i )
	{
		results[i] = interpolator->value(xPoints[i]);
	}
	
	return results;
}

std::shared_ptr<LAInterpolationBase> LAMathInterpolationUtilities::buildInterpolator( const DoubleArray& xValues, const DoubleArray& yValues, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	const size_t nPoints = xValues.size();
	AQ_REQUIRE( nPoints != 0 , "Unable to Interpolate: No interpolation data provided" )
	AQ_REQUIRE( xValues.size() == yValues.size(), "Unable to Interpolate: xValues and yValues must be the same size" )
	std::vector<double> results( nPoints, 0.0 );

	switch( interpolationEnum )
	{
		case etrading::MONOTONE_CONVEX_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAMonotoneConvexInterpolation() );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::SPLINE_INTERPOLATION:
		{
			// The Spline Default Method is a Natural Spline
			std::shared_ptr<LAInterpolationBase> interpolator( new LASplineInterpolation() );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::MONOTONESPLINE_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND) );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::MONOTONEPARABOLIC_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC) );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::NATURAL_SPLINE_INTERPOLATION:
		{
			// True = Use Natural Spline ( Default )
			std::shared_ptr<LAInterpolationBase> interpolator( new LASplineInterpolation(true) );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::CLAMPED_SPLINE_INTERPOLATION:
		{	
			// False = Use Clamped Spline
			std::shared_ptr<LAInterpolationBase> interpolator( new LASplineInterpolation(false) );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::PARABOLIC_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAParabolicInterpolation() );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::CONSTRAINED_SPLINE_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAConstrainedSplineInterpolation() );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::LINEAR_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LALinearInterpolation() );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::LINEAR_WITH_FLAT_EXTRAPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LALinearInterpolation(ExtrapolationType::FLAT_EXTRAPOLATION_TYPE) );
			interpolator->set(xValues, yValues);
			return interpolator;
		}
		case etrading::RIGHT_CONTINUOUS_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAStepInterpolation(StepType::RIGHT_CONTINUOUS) );
			interpolator->set(xValues, yValues);
			return interpolator;
		}
		case etrading::LEFT_CONTINUOUS_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAStepInterpolation(StepType::LEFT_CONTINUOUS) );
			interpolator->set(xValues, yValues);
			return interpolator;
		}
		case etrading::STEP_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LAStepInterpolation() );
			interpolator->set(xValues,yValues);
			return interpolator;
		}
		case etrading::LINEARSPLINE_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LALinearSplineInterpolation() );
			interpolator->set(xValues,yValues,joinXValue);
			return interpolator;
		}
		case etrading::LINEARMONOTONESPLINE_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND) );
			interpolator->set(xValues,yValues,joinXValue);
			return interpolator;
		}
		case etrading::LINEARMONOTONEPARABOLIC_INTERPOLATION:
		{
			std::shared_ptr<LAInterpolationBase> interpolator( new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC) );
			interpolator->set(xValues,yValues,joinXValue);
			return interpolator;
		}
		default:
		{
			AQ_THROW("Invalid Interpolation Scheme: Must be 'LinearSpline', 'MonotoneSpline', 'LinearMonotoneSpline', 'LinearMonotoneParabolic', 'Spline', 'Linear', 'LinearWithFlatExtrapolation' 'RightContinuous', 'LeftContinuous' or 'Step'");
		}
	}
	
	// We should never reach here
	return nullptr;
}

// *** Instantaneous *** Differentiation at a single xPoint
// ---------------------------------------------------------

double LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::differentiate( xValues, yValues, xPoint, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

double LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	return LAMathInterpolationUtilities::differentiate( xValues, yValues, std::vector<double>(1, xPoint), interpolationEnum, joinXValue )[0];
}

std::vector<double> LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::differentiate( xValues, yValues, xPoints, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

std::vector<double> LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	const size_t nPoints = xPoints.size();
	AQ_REQUIRE( xValues.size() == yValues.size(), "Unable to Differentiate: xValues and yValues must be the same size" )
	AQ_REQUIRE( nPoints != 0 , "Unable to Differentiate: No interpolation search points were requested, differentiation xPoints parameter is empty" )
	std::vector<double> results( nPoints, 0.0 );

	switch( interpolationEnum )
	{
		case etrading::MONOTONE_CONVEX_INTERPOLATION:
		{
			LAMonotoneConvexInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator; // The Spline Default Method is a Natural Spline
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::MONOTONESPLINE_INTERPOLATION:
		{
			LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::MONOTONEPARABOLIC_INTERPOLATION:
		{
			LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::NATURAL_SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator(true); // True = Use Natural Spline ( Default )
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::CLAMPED_SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator(false); // False = Use Clamped Spline
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::PARABOLIC_INTERPOLATION:
		{
			LAParabolicInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::CONSTRAINED_SPLINE_INTERPOLATION:
		{
			LAConstrainedSplineInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::LINEAR_INTERPOLATION:
		{
			LALinearInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::LINEAR_WITH_FLAT_EXTRAPOLATION:
		{
			LALinearInterpolation interpolator(ExtrapolationType::FLAT_EXTRAPOLATION_TYPE);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::RIGHT_CONTINUOUS_INTERPOLATION:
		{
			LAStepInterpolation interpolator(StepType::RIGHT_CONTINUOUS);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::LEFT_CONTINUOUS_INTERPOLATION:
		{
			LAStepInterpolation interpolator(StepType::LEFT_CONTINUOUS);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::STEP_INTERPOLATION:
		{
			LAStepInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::LINEARSPLINE_INTERPOLATION:
		{
			LALinearSplineInterpolation interpolator;
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::LINEARMONOTONESPLINE_INTERPOLATION:
		{
			LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		case etrading::LINEARMONOTONEPARABOLIC_INTERPOLATION:
		{
			LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiate(xPoints[i]);
			}
			break;
		}
		default:
		{
			AQ_THROW("Invalid Interpolation Scheme: Must be 'LinearSpline', 'MonotoneSpline', 'LinearMonotoneSpline', 'LinearMonotoneParabolic', 'Spline', 'Linear', 'LinearWithFlatExtrapolation' 'RightContinuous', 'LeftContinuous' or 'Step'");
		}
	}
	
	return results;
}


// *** Discrete Differentiation *** - Over the range (fromXPoint - toXPoint)
// ---------------------------------------------------------

double LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const double & fromXPoint, const double & toXPoint, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::differentiate( xValues, yValues, fromXPoint, toXPoint, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

double LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const double & fromXPoint, const double & toXPoint, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	return LAMathInterpolationUtilities::differentiate( xValues, yValues, std::vector<double>(1, fromXPoint), std::vector<double>(1, toXPoint), interpolationEnum, joinXValue )[0];
}

std::vector<double> LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & fromXPoints, const std::vector<double> & toXPoints, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::differentiate( xValues, yValues, fromXPoints, toXPoints, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

std::vector<double> LAMathInterpolationUtilities::differentiate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & fromXPoints, const std::vector<double> & toXPoints, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	const size_t nPoints = fromXPoints.size();
	AQ_REQUIRE( fromXPoints.size() == toXPoints.size(), "Invalid Discrete Differentiation Parameters: number of differentiation from and toPoints must match" )
	AQ_REQUIRE( xValues.size() == yValues.size(), "Unable to Differentiate: xValues and yValues must be the same size" )
	AQ_REQUIRE( nPoints != 0 , "Unable to Differentiate: No interpolation search points were requested, differentiation xPoints parameter is empty" )
	std::vector<double> results( nPoints, 0.0 );

	switch( interpolationEnum )
	{
		case etrading::MONOTONE_CONVEX_INTERPOLATION:
		{
			LAMonotoneConvexInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator; // The Spline Default Method is a Natural Spline
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::MONOTONESPLINE_INTERPOLATION:
		{
			LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::MONOTONEPARABOLIC_INTERPOLATION:
		{
			LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::NATURAL_SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator(true); // True = Use Natural Spline ( Default )
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::CLAMPED_SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator(false); // False = Use Clamped Spline
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::PARABOLIC_INTERPOLATION:
		{
			LAParabolicInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::CONSTRAINED_SPLINE_INTERPOLATION:
		{
			LAConstrainedSplineInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::LINEAR_INTERPOLATION:
		{
			LALinearInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::LINEAR_WITH_FLAT_EXTRAPOLATION:
		{
			LALinearInterpolation interpolator(ExtrapolationType::FLAT_EXTRAPOLATION_TYPE);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::RIGHT_CONTINUOUS_INTERPOLATION:
		{
			LAStepInterpolation interpolator(StepType::RIGHT_CONTINUOUS);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::LEFT_CONTINUOUS_INTERPOLATION:
		{
			LAStepInterpolation interpolator(StepType::LEFT_CONTINUOUS);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::STEP_INTERPOLATION:
		{
			LAStepInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::LINEARSPLINE_INTERPOLATION:
		{
			LALinearSplineInterpolation interpolator;
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::LINEARMONOTONESPLINE_INTERPOLATION:
		{
			LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		case etrading::LINEARMONOTONEPARABOLIC_INTERPOLATION:
		{
			LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nPoints; ++i )
			{
				results[i] = interpolator.differentiateOverRange(fromXPoints[i], toXPoints[i]);
			}
			break;
		}
		default:
		{
			AQ_THROW("Invalid Interpolation Scheme: Must be 'LinearSpline', 'MonotoneSpline', 'LinearMonotoneSpline', 'LinearMonotoneParabolic', 'Spline', 'Linear', 'LinearWithFlatExtrapolation' 'RightContinuous', 'LeftContinuous' or 'Step'");
		}
	}
	
	return results;
}



double LAMathInterpolationUtilities::integrate( const DoubleArray& xValues, const DoubleArray& yValues, const double & lowerBound, const double & upperBound, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::integrate( xValues, yValues, lowerBound, upperBound, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

double LAMathInterpolationUtilities::integrate( const DoubleArray& xValues, const DoubleArray& yValues, const double & lowerBound, const double & upperBound, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	return LAMathInterpolationUtilities::integrate( xValues, yValues, std::vector<double>(1, lowerBound), std::vector<double>(1, upperBound), interpolationEnum, joinXValue )[0];
}

std::vector<double> LAMathInterpolationUtilities::integrate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds, const StandardString& interpolation, const double & joinXValue )
{
	return LAMathInterpolationUtilities::integrate( xValues, yValues, lowerBounds, upperBounds, etrading::toInterpolationEnum( interpolation ), joinXValue );
}

std::vector<double> LAMathInterpolationUtilities::integrate( const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue )
{
	const size_t nPoints = xValues.size();
	const size_t nBounds = lowerBounds.size();
	AQ_REQUIRE( nPoints == yValues.size(), "Unable to Integrate: xValues and yValues must be the same size" )
	AQ_REQUIRE( nPoints != 0 , "Unable to Integrate: Interpolation data is empty" )
	AQ_REQUIRE( nBounds == upperBounds.size(), "Unable to Integrate: The number of lowerBounds and upperBounds must be the same" )
	AQ_REQUIRE( nBounds != 0 , "Unable to Integrate: No integration abscissae were requested, integration boundary parameters are empty" )

	std::vector<double> results( nBounds, 0.0 );

	switch( interpolationEnum )
	{
		case etrading::MONOTONE_CONVEX_INTERPOLATION:
		{
			LAMonotoneConvexInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator; // The Spline Default Method is a Natural Spline
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::MONOTONESPLINE_INTERPOLATION:
		{
			LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::MONOTONEPARABOLIC_INTERPOLATION:
		{
			LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::NATURAL_SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator(true); // True = Use Natural Spline ( Default )
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::CLAMPED_SPLINE_INTERPOLATION:
		{
			LASplineInterpolation interpolator(false); // False = Use Clamped Spline
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::PARABOLIC_INTERPOLATION:
		{
			LAParabolicInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::CONSTRAINED_SPLINE_INTERPOLATION:
		{
			LAConstrainedSplineInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::LINEAR_INTERPOLATION:
		{
			LALinearInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::LINEAR_WITH_FLAT_EXTRAPOLATION:
		{
			LALinearInterpolation interpolator(ExtrapolationType::FLAT_EXTRAPOLATION_TYPE);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::RIGHT_CONTINUOUS_INTERPOLATION:
		{
			LAStepInterpolation interpolator(StepType::RIGHT_CONTINUOUS);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::LEFT_CONTINUOUS_INTERPOLATION:
		{
			LAStepInterpolation interpolator(StepType::LEFT_CONTINUOUS);
			interpolator.set(xValues, yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::STEP_INTERPOLATION:
		{
			LAStepInterpolation interpolator;
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::LINEARSPLINE_INTERPOLATION:
		{
			LALinearSplineInterpolation interpolator;
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::LINEARMONOTONESPLINE_INTERPOLATION:
		{
			LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		case etrading::LINEARMONOTONEPARABOLIC_INTERPOLATION:
		{
			LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
			interpolator.setJoinDateAsDouble(joinXValue);
			interpolator.set(xValues,yValues);
			for( size_t i = 0; i < nBounds; ++i )
			{
				AQ_REQUIRE( lowerBounds[i] <= upperBounds[i], "Unable to Integrate: the LowerBound must be less than or equal to the UpperBound" )
				if( AQ_IS_EQUAL( lowerBounds[i], upperBounds[i] ) )
				{ 
					results[i] = 0.0;
				}
				else
				{
					results[i] = interpolator.integrate( lowerBounds[i], upperBounds[i] );
				}
			}
			break;
		}
		default:
		{
			AQ_THROW("Invalid Interpolation Scheme: Must be 'LinearSpline', 'MonotoneSpline', 'LinearMonotoneSpline', 'LinearMonotoneParabolic', 'Spline', 'Linear', 'LinearWithFlatExtrapolation' 'RightContinuous', 'LeftContinuous' or 'Step'");
		}
	}
	
	return results;
}

double LAMathInterpolationUtilities::monotoneconvex(DoubleVector& xValues, DoubleVector& yValues, double xPoint, double lambda, bool isAllowedNegative, int inputMode, int outputMode)
{
    LAMonotoneConvexInterpolation interpolator(lambda,isAllowedNegative,inputMode,outputMode);
    interpolator.set(xValues,yValues);
    return interpolator.value(xPoint);
}

double LAMathInterpolationUtilities::linear(DoubleVector& xValues, DoubleVector& yValues, double xPoint, bool isExtraConstFlag)
{
    double result;
    if(isExtraConstFlag==true)
    {	
        LALinearInterpolation interpolator;
        interpolator.set(xValues,yValues);
	    result = interpolator.value(xPoint);
    }
    else if(isExtraConstFlag==false)
    {	
        LALinearInterpolation interpolator(LINEAR_EXTRAPOLATION_TYPE);
        interpolator.set(xValues,yValues);
	    result = interpolator.value(xPoint);
    }
	return result;
}

double 
LAMathInterpolationUtilities::linearReverse(DoubleVector& xValues, DoubleVector& yValues, double xPoint, bool isExtraConstFlag)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	double result;
    if(isExtraConstFlag==true)
    {	
        LALinearInterpolation interpolator;
        interpolator.set(array1_,array2_);
	    result = interpolator.value(xPoint);
    }
    else if(isExtraConstFlag==false)
    {	
        LALinearInterpolation interpolator(LINEAR_EXTRAPOLATION_TYPE);
        interpolator.set(array1_,array2_);
	    result = interpolator.value(xPoint);
    }
	return result;
}

double 
LAMathInterpolationUtilities::spline(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
	LASplineInterpolation interpolator;
	interpolator.set(xValues,yValues);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::splineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LASplineInterpolation interpolator;
	interpolator.set(array1_,array2_);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::monotoneSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
	LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
	interpolator.set(xValues,yValues);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::monotoneSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
	interpolator.set(array1_,array2_);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::monotoneParabolic(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
	LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
	interpolator.set(xValues,yValues);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::monotoneParabolicReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LAMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
	interpolator.set(array1_,array2_);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::parabolic(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
    LAParabolicInterpolation interpolator;
	interpolator.set(xValues,yValues);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::parabolicReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LAParabolicInterpolation interpolator;
	interpolator.set(array1_,array2_);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::linearSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble)
{
    LALinearSplineInterpolation interpolator;
    interpolator.setJoinDateAsDouble(joinDateAsDouble);
    interpolator.set(xValues,yValues);
    double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::linearSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LALinearSplineInterpolation interpolator;
	interpolator.setJoinDateAsDouble(joinDateAsDouble);
    interpolator.set(array1_,array2_);
    double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::linearMonotoneSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble)
{
    LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
    interpolator.setJoinDateAsDouble(joinDateAsDouble);
    interpolator.set(xValues,yValues);
    double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::linearMonotoneSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND);
	interpolator.setJoinDateAsDouble(joinDateAsDouble);
    interpolator.set(array1_,array2_);
    double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::linearMonotoneParabolic(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble)
{
    LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
    interpolator.setJoinDateAsDouble(joinDateAsDouble);
    interpolator.set(xValues,yValues);
    double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::linearMonotoneParabolicReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LALinearMonotoneSplineInterpolation interpolator(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC);
	interpolator.setJoinDateAsDouble(joinDateAsDouble);
    interpolator.set(array1_,array2_);
    double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::constrainedSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
	LAConstrainedSplineInterpolation interpolator;
	interpolator.set(xValues,yValues);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::constrainedSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
    size_t n = xValues.size();
    DoubleArray array1_(n),array2_(n);
    for(size_t i=0;i<n;i++)
    {
        array1_[i]=xValues[n-1-i];
        array2_[i]=yValues[n-1-i];
    }

	LAConstrainedSplineInterpolation interpolator;
	interpolator.set(array1_,array2_);
	double result = interpolator.value(xPoint);
	return result;
}

double 
LAMathInterpolationUtilities::step(DoubleArray& xValues, DoubleArray& yValues, double xPoint)
{
	LAStepInterpolation interpolator;
	interpolator.set(xValues,yValues);
	double result = interpolator.value(xPoint);
	return result;
}

unsigned int 
LAMathInterpolationUtilities::searchIndex(const DoubleArray& X, double xPoint)
{
    size_t size = X.size();
    if(size<=1) throw LACoreInvalidData("size of vector is 1!",__FILE__,__LINE__);
    if(X[0] - 0.0000001 > xPoint || X[size-1] + 0.0000001 < xPoint)
    {
        throw LACoreInvalidData("xPoint is out of range!",__FILE__,__LINE__);
    }
    unsigned int index=0;
    for (index=0;index<size;index++) if(X[index]>=xPoint) break;
    if(index==0) index = 1;
    if(index==size) index = size-1;

    return index;
}

double 
LAMathInterpolationUtilities::getLinearInterpolation(double x1, double x2, double y1, double y2, double xPoint)
{
    DoubleArray X(2); X[0] = x1; X[1] = x2;
    DoubleArray Y(2); Y[0] = y1; Y[1] = y2;
	return LAMathInterpolationUtilities::linear(X,Y,xPoint);
}

DoubleArray 
LAMathInterpolationUtilities::solve_cubic_equation(double a1, double a2, double a3)
// This method is by Viete. Please find the Numerical Recipe C++ Chapter5 Cubic Equations.
// Purpose of this function is to solve cubic equation, x^3 +a1*x^2 +a2*x +a3 = 0
// and get real roots.
{
	DoubleArray result;

	double Q = (a1*a1 - 3.0*a2)/9.0;
	double R = (2.0*a1*a1*a1 - 9.0*a1*a2 + 27.0*a3)/54.0;
	double Q3R2 =  Q*Q*Q - R*R;

	//if(LAMath::abs(Q3R2) > DBL_EPSILON)
	//{
	if (Q3R2 >= 0.0)
	{
		double theta = acos(R/LAMath::sqrt(Q*Q*Q));
		result.resize(3);
		result[0] = -2.0*LAMath::sqrt(Q)*cos(theta/3.0)-a1/3.0;
		result[1] = -2.0*LAMath::sqrt(Q)*cos((theta-2.0*LAMath::pi())/3.0)-a1/3.0;
		result[2] = -2.0*LAMath::sqrt(Q)*cos((theta-4.0*LAMath::pi())/3.0)-a1/3.0;
	}
	else
	{
		result.resize(1);
		double temp = LAMath::pow(LAMath::sqrt(-Q3R2) + fabs(R), 1.0/3.0);
		result[0] = - LAMath::sign(1,R)*(temp + Q/temp) - a1/3.0;
	}
	//}
	//else
	//{
	//	// zero case
	//	result.resize(2);
	//	result[0] = -2.0*LAMath::sqrt(Q)-a1/3.0;
	//	result[1] = -2.0*LAMath::sqrt(Q)*cos(2.0*LAMath::pi()/3.0)-a1/3.0;
	//}

	return result;
}

DoubleArray 
LAMathInterpolationUtilities::solve_quadratic_equation(double a1, double a2)
{
	DoubleArray result;
	double det = a1 * a1 - 4. * a2;
	if(det == 0.)
	{
		result.resize(1);
		result[0] = - a1 / 2.;
	}
	else if(det > 0.)
	{
		result.resize(2);
		result[0] = (- a1 + LAMath::sqrt(det)) / 2.;
		result[1] = (- a1 - LAMath::sqrt(det)) / 2.;
	}
	return result;
}

/*!
		@brief create interpolation

		@param[in] key
		@return LAInterpolationBase

*/
std::shared_ptr<LAInterpolationBase>
LAMathInterpolationUtilities::createInterpolation(const LAString &str)
{
	if (str == FN_STEPINTERPOLATION_STR || str == STEP_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LAStepInterpolation());
	}
	else if (str == FN_RIGHTCONTINUOUS_INTERPOLATION_STR || str == RIGHT_CONTINUOUS_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LAStepInterpolation(StepType::RIGHT_CONTINUOUS));
	}
	else if (str == FN_LEFTCONTINUOUS_INTERPOLATION_STR || str == LEFT_CONTINUOUS_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LAStepInterpolation(StepType::LEFT_CONTINUOUS));
	}
	else if (str == FN_LINEARINTERPOLATION_STR || str == LINEAR_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LALinearInterpolation());
	}
	else if (str == FN_SPLINEINTERPOLATION_STR || str == SPLINE_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LASplineInterpolation());
	}
	else if (str == FN_MONOTONESPLINEINTERPOLATION_STR || str == MONOTONE_SPLINE_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND));
	}
	else if (str == FN_MONOTONEPARABOLICINTERPOLATION_STR || str == MONOTONE_PARABOLIC_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC));
	}
    else if (str == FN_NATURALSPLINEINTERPOLATION_STR || str == NATURAL_SPLINE_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LASplineInterpolation(true)); // UseNaturalSpline = true (default)
	}
    else if (str == FN_CLAMPEDSPLINEINTERPOLATION_STR || str == CLAMPED_SPLINE_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LASplineInterpolation(false)); // UseNaturalSpline = false => Clamped Spline
	}
    else if (str == FN_PARABOLICINTERPOLATION_STR || str == PARABOLIC_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LAParabolicInterpolation());
	}
    else if (str == FN_LINEARSPLINEINTERPOLATION_STR || str == LINEAR_SPLINE_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>( new LALinearSplineInterpolation() );
	}
	else if (str == FN_LINEARMONOTONESPLINEINTERPOLATION_STR || str == LINEAR_MONOTONE_SPLINE_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>( new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND) );
	}
	else if (str == FN_LINEARMONOTONEPARABOLICINTERPOLATION_STR || str == LINEAR_MONOTONE_PARABOLIC_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>( new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC) );
	}
	else if (str == FN_CONSTRAINEDSPLINEINTERPOLATION_STR || str == CONSTRAINED_SPLINE_INTERP)
	{
		return std::shared_ptr<LAInterpolationBase>(new LAConstrainedSplineInterpolation());
	}
	else if (str == FN_MONOTONECONVEXINTERPOLATION_STR || str == MONOTONE_CONVEX_INTERP)
	{
        return std::shared_ptr<LAInterpolationBase>(new LAMonotoneConvexInterpolation());
	}
	else
	{
		LAString msg = "This interpolation method is not supported . interpolation = " + str;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}
