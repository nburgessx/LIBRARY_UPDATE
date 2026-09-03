#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLString.h"
#include "ConstantDeclarations.h"
#include "AQLTime.h"
#include "AQLInterpolationBase.h"
#include "AQLParabolicInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include <memory>
#include "CoreEnumerations.h"

class AQLMathInterpolationUtilities
{
public:
	
	// Consolidated Interpolation Methods
	static double interpolate(const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const StandardString& interpolation, const double & joinXValue = 0.0);
	static double interpolate(const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue = 0.0);
	static std::vector<double> interpolate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const StandardString& interpolation, const double & joinXValue = 0.0);
	static std::vector<double> interpolate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue = 0.0);

	static std::shared_ptr<AQLInterpolationBase> buildInterpolator(const DoubleArray& xValues, const DoubleArray& yValues, const etrading::InterpolationEnum& interpolationEnum, const double & joinXValue = 0.0 );

	// Instantaneous Differentiation at a single xPoint
	static double differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const StandardString& interpolation, const double & joinXValue = 0.0);
	static double differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const double & xPoint, const etrading::InterpolationEnum& interpolation, const double & joinXValue = 0.0);
	static std::vector<double> differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const StandardString& interpolation, const double & joinXValue = 0.0);
	static std::vector<double> differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & xPoints, const etrading::InterpolationEnum& interpolation, const double & joinXValue = 0.0);

	// Discrete Differentiation - Over the range (fromXPoint - toXPoint)
	static double differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const double & fromXPoint, const double & toXPoint, const StandardString& interpolation, const double & joinXValue = 0.0);
	static double differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const double & fromXPoint, const double & toXPoint, const etrading::InterpolationEnum& interpolation, const double & joinXValue = 0.0);
	static std::vector<double> differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & fromXPoints, const std::vector<double> & toXPoints, const StandardString& interpolation, const double & joinXValue = 0.0);
	static std::vector<double> differentiate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & fromXPoints, const std::vector<double> & toXPoints, const etrading::InterpolationEnum& interpolation, const double & joinXValue = 0.0);

	static double integrate(const DoubleArray& xValues, const DoubleArray& yValues, const double & lowerBound, const double & upperBound, const StandardString& interpolation, const double & joinXValue = 0.0);
	static double integrate(const DoubleArray& xValues, const DoubleArray& yValues, const double & lowerBound, const double & upperBound, const etrading::InterpolationEnum& interpolation, const double & joinXValue = 0.0);
	static std::vector<double> integrate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds, const StandardString& interpolation, const double & joinXValue = 0.0);
	static std::vector<double> integrate(const DoubleArray& xValues, const DoubleArray& yValues, const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds, const etrading::InterpolationEnum& interpolation, const double & joinXValue = 0.0);

	// Specific Interpolation Methods
	static double monotoneconvex(DoubleVector& xValues, DoubleVector& yValues, double xPoint, double lambda, bool isAllowedNegative, int inputMode, int outputMode);
	static double linear(DoubleVector& xValues, DoubleVector& yValues, double xPoint, bool isExtraConstFlag=true );
    static double linearReverse(DoubleVector& xValues, DoubleVector& yValues, double xPoint, bool isExtraConstFlag=true );//when xValues is decreasing
	static double spline(DoubleArray& xValues, DoubleArray& yValues, double xPoint);
    static double splineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint);//when xValues is decreasing
	static double monotoneSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint);
    static double monotoneSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint);//when xValues is decreasing
	static double monotoneParabolic(DoubleArray& xValues, DoubleArray& yValues, double xPoint);
    static double monotoneParabolicReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint);//when xValues is decreasing
    static double parabolic(DoubleArray& xValues, DoubleArray& yValues, double xPoint);
    static double parabolicReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint); //when xValues is decreasing
    static double linearSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble );
    static double linearSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble ); //when xValues is decreasing
    static double linearMonotoneSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble );
    static double linearMonotoneSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble ); //when xValues is decreasing
	static double linearMonotoneParabolic(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble );
    static double linearMonotoneParabolicReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint, double joinDateAsDouble ); //when xValues is decreasing
    static double constrainedSpline(DoubleArray& xValues, DoubleArray& yValues, double xPoint);
    static double constrainedSplineReverse(DoubleArray& xValues, DoubleArray& yValues, double xPoint);//when xValues is decreasing
	static double step(DoubleArray& xValues, DoubleArray& yValues, double xPoint);
	static double getLinearInterpolation(double x1, double x2, double y1, double y2, double xPoint);
    static unsigned int searchIndex(const DoubleArray& X, double xPoint);
	static std::shared_ptr<AQLInterpolationBase> createInterpolation(const AQLString &method);

	// max-min methods
	static double gmax(double a, double b){return a > b ? a : b;}
	static double gmin(double a, double b){return a < b ? a : b;}
    
    static DoubleArray solve_cubic_equation(double a1, double a2, double a3);
	static DoubleArray solve_quadratic_equation(double a1, double a2);

private:
    AQLMathInterpolationUtilities(void) {};
    ~AQLMathInterpolationUtilities(void) {}; 
	AQLMathInterpolationUtilities(const AQLMathInterpolationUtilities &rhs);
	AQLMathInterpolationUtilities &operator=(const AQLMathInterpolationUtilities &rhs);
};

