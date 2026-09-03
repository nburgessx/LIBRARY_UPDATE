/*
 * @brief			Hull White Model
 * @Created:		13th Octt 2017
 * @Author:			Yongyan Zheng
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "VasicekModel.h"
#include "CurveUtilities.h"
#include "ExceptionMacros.h"

namespace etrading
{
    // Alternative Constructor
    VasicekModel::VasicekModel(const double & alpha, const double & theta, const double & sigma) : alpha_( alpha ), theta_( theta ), sigma_( sigma )
    {
		// Vasicek SDE: dr = alpha* (theta - r)*dt + sigma * dW, where theta, alpha, and sigma are constant
		
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( alpha ),    "VasicekModel alpha (mean reversion spread) parameter must be greater than zero" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( theta ),    "VasicekModel theta (mean reversion level) parameter must be greater than zero" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( sigma ),    "VasicekModel sigma (volatility) parameter cannot be negative" );
    }

    // Copy Constructor
    VasicekModel::VasicekModel( const VasicekModel & rhs ) : alpha_( rhs.alpha_ ), theta_( rhs.theta_ ), sigma_( rhs.sigma_ )
    {
    }

    // Assignment Operator
    VasicekModel& VasicekModel::operator=( const VasicekModel & rhs )
    {
        // 1. Make a temp copy
        VasicekModel temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( alpha_,      temp.alpha_ );
        std::swap( theta_,      temp.theta_ );
        std::swap( sigma_,      temp.sigma_ );

		return *this;
    }

	std::shared_ptr<VasicekModel> VasicekModel::clone()
    {
        std::shared_ptr<VasicekModel> cf = std::make_shared<VasicekModel>( VasicekModel( *this ) );
        return cf;
    }

	/* @brief			Formula:  B(t,T) = 1/a * (1 - exp(-a*(T-t))
	*  @param [in]		tao	    Year fraction (T-t)
	*  @return			B(t,T)
	*/
	double VasicekModel::formulaB(const double& tao) const
	{
		// Formula: B(t,T) = 1/a * (1 - exp(-a*(T-t))
		// ref: (3.8) on page 59 of Brigo book 

		double temp = 1.0 - exp(-alpha_* tao);

		double B_t_T = temp / alpha_;

		return B_t_T;
	}

	/* @brief			Formula: A(t,T) = exp( (theta - sigma^2/(2a^2))*[B(t,T) - (T-t)] - sigma^2/(4a) * B(t,T)^2 )
	*  @param [in]		tao			 Year fraction (T-t)
	*  @param [in]		B_t_T	     Formula B(t,T)
	*  @return			A(t,T)
	*/
	double VasicekModel::formulaA(const double& tao, const double& B_t_T) const
	{
		// Formula: A(t,T) = exp( (theta - sigma^2/(2a^2))*[B(t,T) - (T-t)] - sigma^2/(4a) * B(t,T)^2 )
		// ref: (3.8) on page 59 of Brigo book

		//(theta - sigma^2/(2a^2))
		const double temp = theta_ - sigma_ * sigma_/ (2 * alpha_ * alpha_);

		// A(t,T) 
		const double A_t_T = exp( temp * (B_t_T  - tao) - sigma_ * sigma_ /(4 * alpha_) * B_t_T * B_t_T);

		return A_t_T;
	}

	/* @brief			Get Zero Coupon Bond/ Discount Factor P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
	*  @param [in]		inputDateInfos	 Date/year fraction info derived from input fixing dates
	*  @param [in]		r_t				 Short rate at time t from the model
    *  @param [in]		futureValuation	 False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
	*  @return			A list of P(t,T)s
	*/
	DoubleVector VasicekModel::zeroCouponBondPricesAnalytical(const std::vector<InputDateInfo>& inputDateInfos, const double& r_t, const bool& futureValuation) const
	{
		// Formula: P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
		// ref: (3.8) on page 59 of Brigo book 

		const size_t expectedSize = inputDateInfos.size();
		DoubleVector P_t_Ts (expectedSize);

		for( size_t i = 0; i < expectedSize; ++i )
		{
			const auto inputDateInfo = inputDateInfos[i];
				
			double P_t_T = 0.0;

			// Valuate at asOfDate
			// For today's curve, we do not simulate r(t), we use r(0) by using : P(t,T) = P(0,T)/P(0,t) = [A(0,T) * exp (-B(0,T) * r(0))]/[A(0,t) * exp (-B(0,t) * r(0))] 
			if (!futureValuation)
			{
				// --- 1) Calculate P(0,T) ---

				// B(0,T)
				const double B_0_T = formulaB(inputDateInfo.T);

				// A(0,T)
				const double A_0_T = formulaA(inputDateInfo.T, B_0_T);

				// P(0,T) = A(0,T) * exp (-B(0,T) * r(0))
				const double P_0_T = A_0_T * exp(-B_0_T * r_t);

				// --- 2) Calculate P(0,t) ---

				// B(0,t)
				const double B_0_t = formulaB(inputDateInfo.t);

				// A(0,t)
				const double A_0_t = formulaA(inputDateInfo.t, B_0_t);

				// P(0,t) = A(0,t) * exp (-B(0,t) * r(0))
				const double P_0_t = A_0_t * exp(-B_0_t * r_t);

				// --- 3) P(t,T) = P(0,T)/P(0,t) ---
				P_t_T = P_0_T / P_0_t;

			}
			// Valuate at a future date
			else
			{
				// B(t,T)
				const double B_t_T = formulaB(inputDateInfo.tao);

				// A(t,T)
				const double A_t_T = formulaA(inputDateInfo.tao, B_t_T);

				// P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
				P_t_T = A_t_T * exp(-B_t_T * r_t);

			}

			P_t_Ts[i] = P_t_T;
		}

		return P_t_Ts;
	}

	/* @brief			Get forward rates from Vasicek Bond Analytical formua
	*  @param [in]		fixingDates			Input fixing dates
	*  @param [in]		curveCollection	    Curve Collection
	*  @param [in]		curveIndex			Curve Index name
	*  @param [in]		r_t					Short rate at time t from the model
    *  @param [in]		futureValuation			False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
	*  @return			A list of F(t,T)s
	*/
	DoubleVector VasicekModel::forwardRatesAnalytical(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex, const double& r_t, const bool& futureValuation) const
	{
		std::vector<InputDateInfo> inputDateInfos = calculateInputDateInfos(fixingDates, curveCollection, curveIndex);

		const std::vector<double> P_t_Ts = zeroCouponBondPricesAnalytical(inputDateInfos, r_t, futureValuation);

		// 3) Get fitted forward rates given a sigma:
		DoubleVector fittedForwardRates = forwardRates(inputDateInfos, P_t_Ts);

		return fittedForwardRates;
	}

	// Get Accessors
    double VasicekModel::getAlpha() const					{ return alpha_; };
    double VasicekModel::getSigma() const					{ return sigma_; };
    double VasicekModel::getTheta() const					{ return theta_; };

	// Set Accessors
    void VasicekModel::setAlpha( const double & alpha )   { alpha_ = alpha; };
    void VasicekModel::setSigma( const double & sigma )   { sigma_ = sigma; };
    void VasicekModel::setTheta( const double & theta )   { theta_ = theta; };


}
