/*
 * @brief			Hull White Model
 * @Created:		13th Octt 2017
 * @Author:			Yongyan Zheng
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "HullWhiteModel.h"
#include "CurveUtilities.h"
#include "ExceptionMacros.h"

namespace etrading
{
    // Alternative Constructor
    HullWhiteModel::HullWhiteModel(const double & alpha, const double & sigma) : alpha_( alpha ), sigma_( sigma )
    {

		// Hull White 1F SDE: dr = (theta - alpha* r)*dt + sigma * dW, where theta is time dependent, alpha and sigma are constant

		MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( alpha ),    "Hull White alpha (mean reversion spread) parameter must be greater than zero" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( sigma ),    "Hull White sigma (volatility) parameter cannot be negative" );
    }

    // Copy Constructor
    HullWhiteModel::HullWhiteModel( const HullWhiteModel & rhs ) : alpha_( rhs.alpha_ ), sigma_( rhs.sigma_ )
    {
    }

    // Assignment Operator
    HullWhiteModel& HullWhiteModel::operator=( const HullWhiteModel & rhs )
    {
        // 1. Make a temp copy
        HullWhiteModel temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( alpha_,      temp.alpha_ );
        std::swap( sigma_,      temp.sigma_ );

		return *this;
    }

	std::shared_ptr<HullWhiteModel> HullWhiteModel::clone()
    {
        std::shared_ptr<HullWhiteModel> cf = std::make_shared<HullWhiteModel>( HullWhiteModel( *this ) );
        return cf;
    }

	/* @brief			Formula:  B(t,T) = 1/a * (1 - exp(-a*(T-t))
    *  @param [in]		tao	    Year fraction (T-t)
    *  @return			B(t,T)
    */
	double HullWhiteModel::formulaB(const double& tao) const
	{
		// Formula: B(t,T) = 1/a * (1 - exp(-a*(T-t))
		// ref: (3.39) on page 75 of Brigo book 

		double temp = 1.0 - exp(-alpha_* tao);

		double B_t_T = temp / alpha_;

		return B_t_T;
	}

	/* @brief			Formula: A(t,T) = P(0,T)/P(0,t) * exp( B(t,T)*F(0,t) - sigma^2/(4a) * (1- exp(-2a*t))) * B(t,T)^2 )
    *  @param [in]		t	         Time t in formulaA 
    *  @param [in]		B_t_T	     Formula B(t,T)
    *  @param [in]		Pm_t		 Discount factor P(0,t) from the market
    *  @param [in]		Pm_T		 Discount factor P(0,T) from the market
    *  @param [in]		Fm_t		 FowardRate f(0,t) from the market
    *  @return			A(t,T)
    */
	double HullWhiteModel::formulaA(const double& t, const double& B_t_T, const double& Pm_t, const double& Pm_T, const double& Fm_t) const
	{
		// Formula: A(t,T) = P(0,T)/P(0,t) * exp( B(t,T)*F(0,t) - sigma^2/(4a) * (1- exp(-2a*t))) * B(t,T)^2 )
		// ref: (3.39) on page 75 of Brigo book 

		// (1- exp(-2a*t))
		const double temp = 1.0 - exp(-2*alpha_*(t));

		// A(t,T) 
		const double A_t_T = Pm_T/Pm_t *+ exp(B_t_T * Fm_t  - sigma_ * sigma_ /(4 * alpha_) * temp * B_t_T * B_t_T);

		return A_t_T;
	}

	
	/* @brief			Get Zero Coupon Bond/ Discount Factor P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
    *  @param [in]		inputDates	 Date/year fraction info derived from input fixing dates
    *  @param [in]		Pm_ts	     Discount factors P(0,t)s from the market
    *  @param [in]		Pm_Ts		 Discount factors P(0,T)s from the market
    *  @param [in]		fm_ts		 FowardRates f(0,t)s from the market
    *  @param [in]		r_t		     Short rate at time t from the model
    *  @param [in]		futureValuation False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
    *  @return			A list of P(t,T)s
    */
	DoubleVector HullWhiteModel::zeroCouponBondPricesAnalytical(const std::vector<InputDateInfo>& inputDates, const DoubleVector& Pm_ts, const DoubleVector& Pm_Ts , const DoubleVector& fm_ts, const double& r_t, const bool& futureValuation) const
	{
		
		// Formula: P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
		// ref: (3.39) on page 75 of Brigo book 
	
		const size_t expectedSize = inputDates.size();
		DoubleVector P_t_Ts (expectedSize);

		for( size_t i = 0; i < expectedSize; ++i )
		{
			const auto inputDateInfo = inputDates[i];
			double P_t_T = 0.0;

			// Pm(0,t) and Pm(0,T)
			const double Pm_t = Pm_ts[i];
			const double Pm_T = Pm_Ts[i];

			// Valuate at asOfDate
			if (!futureValuation)
			{
				// P(t,T) = Pm(0,T)/Pm(0,t), for Hull White, the initial term structure is exactly matched
				P_t_T = Pm_T/Pm_t;
			}
			// Valuate at a Future date
			else
			{
				// B(t,T)
				const double B_t_T = formulaB(inputDateInfo.tao);

				// f(0,t)
				const double fm_t = fm_ts[i];

				// A(t,T)
				const double A_t_T = formulaA(inputDateInfo.t, B_t_T, Pm_t, Pm_T, fm_t);

				// P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
				P_t_T = A_t_T * exp(-B_t_T * r_t);
			}

			P_t_Ts[i] = P_t_T;
		}

		return P_t_Ts;
	}

	/* @brief			Get forward rates from HullWhite Bond Analytical formua
    *  @param [in]		fixingDates			Input fixing dates
    *  @param [in]		curveCollection	    Curve Collection
    *  @param [in]		curveIndex			Curve Index name
    *  @param [in]		r_t					Short rate at time t from the model
    *  @param [in]		futureValuation		False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
    *  @return			A list of F(t,T)s
    */
	DoubleVector HullWhiteModel::forwardRatesAnalytical(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex, const double& r_t, const bool& futureValuation) const
	{
		std::vector<InputDateInfo> inputDateInfos = calculateInputDateInfos(fixingDates, curveCollection, curveIndex);

		const size_t expectedSize = fixingDates.size();
		DateVector tDates (expectedSize);
		DateVector TDates (expectedSize);

		for( size_t i = 0; i < expectedSize; ++i )
		{
			const auto inputDateInfo = inputDateInfos[i];
			tDates[i] = inputDateInfo.tDate;
			TDates[i] = inputDateInfo.TDate;
		}

		const LAString curveId (curveCollection.c_str()); 
		const LAString curveIndx (curveIndex.c_str()); 

		//market forward rates f(0,t)
		const DoubleVector Fm_ts = getCurveForwardRatesFromAsOfDate( tDates, curveId, curveIndx);

		//market discount factors
		const DoubleVector Pm_ts = getCurveDiscountFactors( tDates, curveId, curveIndx);
		const DoubleVector Pm_Ts = getCurveDiscountFactors( TDates, curveId, curveIndx);

		const std::vector<double> P_t_Ts = zeroCouponBondPricesAnalytical(inputDateInfos, Pm_ts, Pm_Ts , Fm_ts, r_t, futureValuation);

		// 3) Get fitted forward rates given a sigma:
		DoubleVector fittedForwardRates = forwardRates(inputDateInfos, P_t_Ts);

		return fittedForwardRates;
	}

	//TODO: to implement
	// Formula: theta(t) = dF(0,t)/dt + a * F(0,t) + sigma^2/(2a) * (1- exp(-2a*t)), where F(0,t) is forward rate from t0 to t, and a is the mean reversion spread
	double HullWhiteModel::calculateTheta(const LADate& t, const std::string& curveCollection, const std::string& curveIndex) const
	{
		// Formula: theta(t) = dF(0,t)/dt + a * F(0,t) + sigma^2/(2a) * (1- exp(-2a*t)), where F(0,t) is forward rate from t0 to t, and a is the mean reversion spread
		// ref: (3.34) on page 73 of Brigo book 

		// theta is required when simulating zero rates, e.g. using Euler discretization: r(i+1) = r(i) + (theta(i) - a*r(i))*dt + sigma*sqrt(dt)*N(0,1)
    	throw LACoreInvalidData( "#Error: HullWhiteModel calculateTheta method is not supported (TODO)", __FILE__, __LINE__ );
	}

	// Get Accessors
    double HullWhiteModel::getAlpha() const					{ return alpha_; };
    double HullWhiteModel::getSigma() const					{ return sigma_; };

	// Set Accessors
    void HullWhiteModel::setAlpha( const double & alpha )   { alpha_ = alpha; };
    void HullWhiteModel::setSigma( const double & sigma )   { sigma_ = sigma; };


}
