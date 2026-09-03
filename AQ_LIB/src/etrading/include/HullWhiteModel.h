#pragma once
#include <string>
#include <memory>
#include "AQLCoreTemplateType.h"
#include "AffineModelUtilities.h"

namespace etrading
{

    class HullWhiteModel 
    {
    public:

        // Constructor / Destructor
        HullWhiteModel() {};
        ~HullWhiteModel() {};

        // Alternative Constructor
        HullWhiteModel( const double & alpha, const double & sigma); 

        // Copy Constructor
        HullWhiteModel( const HullWhiteModel & rhs );

		// Assignment Operator
        HullWhiteModel & operator=( const HullWhiteModel & rhs );

		// Clone for shared pointer
		std::shared_ptr<HullWhiteModel> clone();

        // Get Accessors
        double getAlpha() const;
        double getSigma() const;

		// Set Accessors
        void setAlpha( const double & alpha );
        void setSigma( const double & sigma );

		/* @brief			Get forward rates from HullWhite Bond Analytical formua
		*  @param [in]		fixingDates			Input fixing dates
		*  @param [in]		curveCollection	    Curve Collection
		*  @param [in]		curveIndex			Curve Index name
		*  @param [in]		r_t					Short rate at time t from the model
		*  @param [in]		futureValuation		True to indicate it's today's curve, false to indicate it's future curve
		*  @return			A list of F(t,T)s
		*/
		DoubleVector forwardRatesAnalytical(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex, const double& r_t, const bool& futureValuation) const;

    protected:

		/* @brief			Formula:  B(t,T) = 1/a * (1 - exp(-a*(T-t))
		*  @param [in]		tao	    Year fraction (T-t)
		*  @return			B(t,T)
		*/
		double formulaB(const double& tao) const;
		
		/* @brief			Formula: A(t,T) = P(0,T)/P(0,t) * exp( B(t,T)*F(0,t) - sigma^2/(4a) * (1- exp(-2a*t))) * B(t,T)^2 )
		*  @param [in]		t	         Time t in formulaA 
		*  @param [in]		B_t_T	     Formula B(t,T)
		*  @param [in]		Pm_t		 Discount factor P(0,t) from the market
		*  @param [in]		Pm_T		 Discount factor P(0,T) from the market
		*  @param [in]		Fm_t		 FowardRate f(0,t) from the market
		*  @return			A(t,T)
		*/
		double formulaA(const double& t, const double& B_t_T, const double& P_t, const double& P_T, const double& F_t) const;

		/* @brief			Get Zero Coupon Bond/ Discount Factor P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
		*  @param [in]		inputDates	 Date/year fraction info derived from input fixing dates
		*  @param [in]		Pm_ts	     Discount factors P(0,t)s from the market
		*  @param [in]		Pm_Ts		 Discount factors P(0,T)s from the market
		*  @param [in]		fm_ts		 FowardRates f(0,t)s from the market
		*  @param [in]		r_t		     Short rate at time t from the model
		*  @param [in]		futureValuation  True to indicate the valuation date is later than asOfDate, False to indicate the same
		*  @return			A list of P(t,T)s
		*/
		DoubleVector zeroCouponBondPricesAnalytical(const std::vector<InputDateInfo>& inputDateInfos, const DoubleVector& Pm_ts, const DoubleVector& Pm_Ts, const DoubleVector& fm_ts, const double& r_t, const bool& futureValuation) const;
		
		//TODO: to implement
		// Formula: theta(t) = dF(0,t)/dt + a * F(0,t) + sigma^2/(2a) * (1- exp(-2a*t)), where F(0,t) is forward rate from t0 to t, and a is the mean reversion spread
        double calculateTheta(const AQLDate& t, const std::string& curveCollection, const std::string& curveIndex) const;

    private:
        
        // Parameters
        double   alpha_; 	// constant meanReversionRate
        double   sigma_;    // constant volatility
    };
}
