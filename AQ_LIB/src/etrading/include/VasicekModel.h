/*
 * @brief			Hull White Model
 * @Created:		13th Octt 2017
 * @Author:			Yongyan Zheng
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include <string>
#include <memory>
#include "LACoreTemplateType.h"
#include "AffineModelUtilities.h"

namespace etrading
{

    class VasicekModel 
    {
    public:

        // Constructor / Destructor
        VasicekModel() {};
        ~VasicekModel() {};

        // Alternative Constructor
        VasicekModel( const double & alpha, const double & theta, const double & sigma); 

        // Copy Constructor
        VasicekModel( const VasicekModel & rhs );

		// Assignment Operator
        VasicekModel & operator=( const VasicekModel & rhs );

		// Clone for shared pointer
		std::shared_ptr<VasicekModel> clone();

        // Get Accessors
        double getAlpha() const;
        double getSigma() const;
        double getTheta() const;

		// Set Accessors
        void setAlpha( const double & alpha );
        void setSigma( const double & sigma );
        void setTheta( const double & theta );

		/* @brief			Get forward rates from Vasicek Bond Analytical formua
		*  @param [in]		fixingDates			Input fixing dates
		*  @param [in]		curveCollection	    Curve Collection
		*  @param [in]		curveIndex			Curve Index name
		*  @param [in]		r_t					Short rate at time t from the model
	    *  @param [in]		futureValuation			False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
		*  @return			A list of F(t,T)s
		*/
		DoubleVector forwardRatesAnalytical(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex, const double& r_t, const bool& futureValuation) const;

    protected:

		/* @brief			Formula:  B(t,T) = 1/a * (1 - exp(-a*(T-t))
		*  @param [in]		tao	    Year fraction (T-t)
		*  @return			B(t,T)
		*/
		double formulaB(const double& tao) const;

		/* @brief			Formula: A(t,T) = exp( (theta - sigma^2/(2a^2))*[B(t,T) - (T-t)] - sigma^2/(4a) * B(t,T)^2 )
		*  @param [in]		tao			 Year fraction (T-t)
		*  @param [in]		B_t_T	     Formula B(t,T)
		*  @return			A(t,T)
		*/
		double formulaA(const double& tao, const double& B_t_T) const;

		/* @brief			Get Zero Coupon Bond/ Discount Factor P(t,T) = A(t,T) * exp (-B(t,T) * r(t))
		*  @param [in]		inputDateInfos		Date/year fraction info derived from input fixing dates
		*  @param [in]		r_t					Short rate at time t from the model
	    *  @param [in]		futureValuation			False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
		*  @return			A list of P(t,T)s
		*/
		DoubleVector zeroCouponBondPricesAnalytical(const std::vector<InputDateInfo>& inputDateInfos, const double& r_t, const bool& futureValuation) const;

    private:
        
        // Parameters
        double   alpha_; 	// constant meanReversionRate
        double   sigma_;    // constant volatility
        double   theta_;    // constant meanReversionLevel
    };
}
