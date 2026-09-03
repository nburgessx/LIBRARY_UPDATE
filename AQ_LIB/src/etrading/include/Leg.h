/*
 * @brief			Class the defines the leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include "Schedule.h"
#include "Cashflow.h"
#include "LegStaticData.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"
#include "LACurvePricingObject.h"

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "DataProvider.h"

namespace etrading
{

	class Leg : public IsLWOObject 
    {
	public:

		Leg(const std::string& instanceName);
		Leg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule={} );
        Leg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule);
		Leg(const Leg& rhs);
		virtual ~Leg() {}

        virtual std::shared_ptr<Leg> clone()=0;

		//Override the abstract class:
        const SchemaObject toSchemaObject() const;
		void toSchemaObject(SchemaObject& schemaObject, const int& index) const; //helper method for caching, to allow the schemaName with index for multi-legs swap
 
		const DataSchema generateDataSchema(const std::string& schemaName) const; //helper method for caching
		std::map<std::string, Variant> getDataMap() const; //helper method for caching


		LegStaticDataPtr getStaticData() const;
		SchedulePtr getSchedule() const;
        virtual void flipPayerReceiver();

        LAString getLegName() const;

		virtual double annuityWithNotional(DataProvider& dataProvider);
		virtual double pv( DataProvider& dataProvider, bool nativeCurrencyPV=false, bool updateCurveData=true);

		virtual ScheduleTypeEnum getType() const=0;
        
		// populate Header matrix and Body matrix
		virtual void populateHeaderAndBody(const DataProvider& dataProvider, AnyTypeVector& headers, std::vector<AnyTypeVector>& bodyBlock, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const;

        // return a pair of Header matrix and Body matrix
		AnyTypeMatrix view(DataProvider& dataProvider, bool showColumnHeaders=true, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList=std::unordered_set<CashflowHeaderEnum,EnumClassHash>());

		virtual LabelValueBlock getInputParameters() const;

		/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
		*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
		*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
		*/
		virtual void initializeDataProvider( DataProvider& dataProvider, bool updateCurveData=true);

		static std::vector<std::string> lvbKeys()
		{
			return LegStaticData::lvbKeys();
		}

		//Helper functions:
		double calculateAnnuityWithNotional( const DataProvider& dataProvider ) const;

		virtual std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const = 0;

		virtual PaymentTriggerEnum getPaymentTrigger() const;
 
	protected:

		LegStaticDataPtr legStaticData_;
		SchedulePtr schedule_;
   		LabelValueBlock inputParameters_;
		
	private:

		/*	@brief	Initialise the DataProvider object from the supplied modelName / curveCollection name parameter, and optional fixngtable
		*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
		*	@param[in]		isFloatRateRequired	Whether to calculate float rates
		*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
		*/
		virtual void initializeDataProviderUsingModel( DataProvider& dataProvider, bool isFloatRateRequired=true, bool updateCurveData=true );

	};

	typedef std::shared_ptr<Leg> LegPtr;

}
