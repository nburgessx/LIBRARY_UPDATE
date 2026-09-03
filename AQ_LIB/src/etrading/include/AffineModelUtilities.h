/*
 * @brief			Hull White Model
 * @Created:		13th Octt 2017
 * @Author:			Yongyan Zheng
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include <string>
#include <memory>
#include "LACoreTemplateType.h"

namespace etrading
{
	//Store inputDate info
	struct InputDateInfo
	{
		LADate tDate; 
		LADate TDate;
		double t; // yearFraction from asOfDate to tDate
		double tao; // T-t, yearFraction from tDate to TDate
		double T; // yearFraction of from asOfDate to TDate, derived from (t + tao)
	};

	/* @brief			Calculate input date information
	*  @param [in]		fixingDates			Input fixing dates
	*  @param [in]		curveCollection	    Curve Collection
	*  @param [in]		curveIndex			Curve Index name
	*  @return			A list of InputDateInfos
	*/
	std::vector<InputDateInfo> calculateInputDateInfos(const DateVector& fixingDates, const std::string& curveCollection, const std::string& curveIndex);

	/* @brief			Get forward rates from discount factors via Discrete Formula: F(t,T) = (P(0,t)/P(0,T) - 1) / (T- t) = (1/P(t,T) - 1) / (T- t) 
	*  @param [in]		inputDateInfos	    Input date information (t, T, T-t)
	*  @param [in]		P_t_Ts				Discount factors P(t,T)s
	*  @return			A list of F(t,T)s
	*/
	DoubleVector forwardRates(const std::vector<InputDateInfo>& inputDateInfos, const std::vector<double>& P_t_Ts);

	/* @brief			Check if the valuation is the same or later than asOfDate, throw error if it's smaller earlier than asOfdDate
	*  @param [in]		curveAsOfDate	    curveAsOfDate
	*  @param [in]		P_t_Ts				Discount factors P(t,T)s
	*  @return			True to indicate it's future curve (i.e. valuationDate > asOfDate), false to indicate it's today's curve (i.e. valuationDate == asOfDate)
	*/
	bool isFutureValuation(const LADate& curveAsOfDate, const LADate& valuationDate);

	/* @brief			Get the default short rate at asOfDate if its not provided 
	*  @param [in]		curveAsOfDate	    curveAsOfDate
	*  @param [in]		curveCollection	    Curve Collection
	*  @param [in]		curveIndex			Curve Index name
    *  @param [in]		futureValuation		False to indicate it's valuated at asOfDate, True to indicate it's valuated at a later date
	*  @return			default short rate at asOfDate if its not provided
	*/
	double getDefaultShortRate(const LADate& curveAsOfDate, const std::string& curveCollection, const std::string& curveIndex, const double& rt, const bool& futureValuation);

}
