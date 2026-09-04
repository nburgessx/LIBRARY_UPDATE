#pragma once

#include "AQLCoreTemplateType.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "BondEnumerations.h"
#include "BondYields.h"
#include "LabelValueBlock.h"
#include "ScheduleValidation.h"
#include "IsAQObject.h"
#include "SchemaObject.h"
#include "Cashflow.h"
#include "DataProvider.h"
#include <boost/lexical_cast.hpp>

namespace etrading
{
	
	class Schedule : public IsAQObject
    {
	public:

		Schedule(const std::string& instanceName);
		Schedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName);

        //Bespoke Schedule, cashflowLVBs will be updated by bespokeScheduleProperties
        Schedule(const std::string& instanceName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs, const BespokeScheduleTypeEnum& bespokeScheduleType);

		Schedule(const Schedule& rhs);
        virtual ~Schedule() {};
        virtual std::shared_ptr<Schedule> clone();

        // For Swaps: Update the Cashflow Discount Factors and Forward Rates
        virtual void initializeDataProviderWithCurveData( DataProvider& dataProvider, const AQLString& discountCurve, const std::vector< FloatRateData >& floatRates = std::vector< FloatRateData >() ) const;

		//This is required when the Swap is back dated, user can choose if the accrued interest is included in the PV or not.
		bool hasAccruedInterest(const AQLDate& settleDate) const;
		virtual double calculateAccruedInterest( const DataProvider& dataProvider, bool nativeCurrencyPV ) const;

        AQLDate getEffectiveDate() const;
        virtual AQLDate getMaturityDate() const;
		AQLDate getUnadjustedMaturityDate() const;

		bool isBespoke() const;

        //Check if the cashflows have variable notionals
        bool isVariableNotional() const;

        //Check if the the schedule has irregular stub
        bool isIrregularStub() const;

        //Check if the schedule is front or back stub, default as front stub.
        bool isFrontStub() const;

        double getYearFractionFromEffectiveToMaturityDates() const;

        //data serialization related
        virtual const SchemaObject toSchemaObject() const;
  		void addInputParametersToSchemaObject(SchemaObject& schemaObject, const std::string& schemaName) const; //Helper method for caching
		void addBespokeCashflowsToSchemaObject(SchemaObject& schemaObject, const std::string& bespokeCashflowSchemaName) const; //Helper method for caching

		virtual const std::string getDataSchemaName() const; //Helper method for caching
		virtual const DataSchema generateDataSchema(const std::string& schemaName="") const;
		std::map<std::string, Variant> getDataMap() const;//Helper method for caching
        
		// To save bespoke/variant notional swap leg's cashflows:
        virtual const std::string getCashflowSchemaName() const; //Helper method for caching
        virtual const DataSchema generateCashflowSchema(const std::string& schemaName="") const; //Helper method for caching
        virtual std::map<std::string, std::vector<std::string>> getCashflowDataMap() const; //Helper method for caching

		virtual std::unordered_set<CashflowHeaderEnum, EnumClassHash> allowedColumns() const;

		// populate Header matrix and Body matrix
		virtual void populateHeaderAndBody(AnyTypeVector& headers, std::vector<AnyTypeVector>& bodyBlock, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum, EnumClassHash>& columnList, const bool convertDatesToExcelFormat = true ) const;

        // return a pair of Header matrix and Body matrix 
        std::pair<AnyTypeMatrix, AnyTypeMatrix> view(bool showBespokeProperties=false, bool showColumnHeaders=true, const std::unordered_set<CashflowHeaderEnum, EnumClassHash>& columnList=std::unordered_set<CashflowHeaderEnum, EnumClassHash>(), const bool convertDatesToExcelFormat=true) const;

        //this method is only for backward compatibility with me methods (none AQObj)	
		AQLStringMatrix display(bool showColumnHeaders=true, const std::unordered_set<CashflowHeaderEnum, EnumClassHash>& columnList=std::unordered_set<CashflowHeaderEnum, EnumClassHash>(), const bool convertDatesToExcelFormat=true) const;

		//Schedule input parameters
		void setInputParameters(const LabelValueBlock& inputParameters);
		LabelValueBlock getInputParameters() const;
		
		double getNotional() const;
		void setNotional(double notional);
        double getFutureValueNotional() const;

        NotionalExchangeEnum getNotionalExchangeEnum() const;	
		double getLeverage() const;
		double getCouponMultiplier() const;
		ScheduleTypeEnum getScheduleType() const;
		BespokeScheduleTypeEnum getBespokeScheduleType() const;

		AQLString getRollDayInput() const;			
		AQLString getAccrualCalendar() const;		
		AQLString getFxFixingCalendar() const;
		BusinessDayAdjustmentEnum getAccrualbusinessDayAdj() const;
		BusinessDayAdjustmentEnum getFxFixingBusinessDayAdj() const;
		DayCountEnum getAccrualDaycount() const;
        FrequencyEnum getAccrualFrequency() const;	
        FrequencyEnum getPaymentFrequency() const;	
        StubTypeEnum getStubType() const;
		AQLString getFixingCalendar() const;		
		AQLString getFixLag() const;
		BusinessDayAdjustmentEnum getFixingBusinessDayAdj() const;
		FixingTypeEnum getFixingAdvanceOrArrears() const;
		AQLString getFirstStub() const;
		AQLString getLastStub() const;		
        bool isAccrualFreqLessThanPaymentFreq() const;

        FrequencyEnum getPaymentFreqEnum() const;
        bool isPaymentFreqEnumAtMaturity() const;
        
		//Schedule outputs
		DateVector		getFixingDates() const;
		DateVector		getFixingEndDates() const;
		DateVector		getPaymentDates() const;
		//Payment dates including upfront cashflow
		DateVector		getAllPaymentDates() const;
		DateVector		getAccrualStartDates() const;
		DateVector		getAccrualEndDates() const;
		DoubleVector	getAccrualYearFractions() const;
		IntVector		getAccrualDays() const;
		AQLDate			getFixingDate(size_t i) const;

		CashflowPtr getCashflow(size_t i) const;
		size_t getCashflowSize() const;
        //Get all cashflows including the upfront cashflow
		std::vector<CashflowPtr> getAllCashflows() const;
        std::vector<CashflowPtr> getAllCashflowsExcludingUpfrontNotional() const;
        std::vector<CashflowPtr> getAllCashflowsIncludingUpfrontNotional() const;
        CashflowPtr getUpfrontCashflow() const;

  		PayReceiveEnum getPayerReceiver() const;
        void flipPayerReceiver();
        int getPayRecIndicator() const;

		//dummy methods, just to avoid downcasting
   		virtual double getFixedRate() const;
   		virtual double getSpread() const;
        virtual bool isVariableSpread() const;

        virtual void updateCashflowsCompoundingCoupons( const DataProvider& dataProvider );

        void updateNotionalExchange();

        DateVector calculateFxFixingDates();

		/*  @brief		This is for calculation of OIS float rates.
						This function calculates fixingEndDates specifically for OIS Swaps.
						The OIS fixingEndDate is used for the OIS average rate calculation,
						which is different from the normal Libor fixingEndDate.
						It is calculated by applying a fixingLag to accrualEndDate.
			@returns	A vector of fixing end dates
		*/
        DateVector calculateAndPopulateOISFixingEndDates();

        static std::vector<std::string> bespokeLVBKeys(const BespokeScheduleTypeEnum& bespokeScheduleType)
		{
			switch (bespokeScheduleType)
			{
			case BESPOKE_SCHEDULE_WITH_PROPERTIES:
			{
				const std::vector<std::string>  expectedKeys{
					IRS_KEY::SCHEDULE_TYPE
					,IRS_KEY::PAY_RECEIVE
					,IRS_KEY::PAYER_RECEIVER
					,IRS_KEY::NOTIONAL_EXCHANGE
					,IRS_KEY::ACCRUALDAYCOUNT
					,IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT
					,IRS_KEY::ACCRUALCALENDAR
					,IRS_KEY::ACCRUALFREQUENCY
					,IRS_KEY::ROLLDAY
					,IRS_KEY::STUBTYPE
					,IRS_KEY::FIXINGADVANCEORARREAR
					,IRS_KEY::PAYMENTFREQUENCY

					,IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT
					,IRS_KEY::FIXINGCALENDAR

					,IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT
					,IRS_KEY::FXFIXINGCALENDAR
					,IRS_KEY::FXFIXINGLAG
				};

				return expectedKeys;
				break;
			}
			case BESPOKE_SCHEDULE:
			{
				const std::vector<std::string>  expectedKeys{
					IRS_KEY::SCHEDULE_TYPE
					,IRS_KEY::PAY_RECEIVE
					,IRS_KEY::PAYER_RECEIVER
					,IRS_KEY::NOTIONAL_EXCHANGE
					,IRS_KEY::ACCRUALDAYCOUNT
					,IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT
					,IRS_KEY::ACCRUALCALENDAR
					,IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT
					,IRS_KEY::FIXINGCALENDAR
				};

				return expectedKeys;
				break;
			}
			default:
				throw AQLCoreInvalidData("#Error: bespokeScheduleType can only be either 'BESPOKE_SCHEDULE_WITH_PROPERTIES' or 'BESPOKE_SCHEDULE'", __FILE__, __LINE__);
				break;
			}
		};


		static std::vector<std::string> lvbKeys()
		{
			const std::string arr[] =
			{
				IRS_KEY::NOTIONAL
				, IRS_KEY::AMORTIZATION
				, IRS_KEY::AMORT_FREQUENCY

                //Future Value Notional, Compulsory for Zero Coupon Swap:
                , IRS_KEY::FV_NOTIONAL

                , IRS_KEY::NOTIONAL_EXCHANGE
				, IRS_KEY::LEVERAGE
				, IRS_KEY::COUPONMULTIPLIER
                
                // PayReceive is required for schedule for getting the right sign for notionalExchange               
                , IRS_KEY::PAY_RECEIVE
				, IRS_KEY::PAYER_RECEIVER //relax PayerReceiver key

                , IRS_KEY::SCHEDULE_TYPE
                , IRS_KEY::EFFECTIVE_DATE
				, IRS_KEY::MATURITY_DATE
				, IRS_KEY::ACCRUALFREQUENCY
				, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT
				, IRS_KEY::ACCRUALCALENDAR
				, IRS_KEY::ACCRUALDAYCOUNT
				, IRS_KEY::PAYMENTFREQUENCY
				, IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT
				, IRS_KEY::PAYMENTCALENDAR
				, IRS_KEY::PAYMENTLAG
				, IRS_KEY::FIRSTSTUBDATE
				, IRS_KEY::LASTSTUBDATE
				, IRS_KEY::ROLLDAY
				, IRS_KEY::STUBTYPE
				, IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT
				, IRS_KEY::FIXINGCALENDAR
				, IRS_KEY::FIXINGLAG
				, IRS_KEY::FIRSTFIXINGLAG
				, IRS_KEY::FIXINGADVANCEORARREAR
				, IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT
				, IRS_KEY::FXFIXINGCALENDAR
				, IRS_KEY::FXFIXINGLAG

				// These keys are for relaxing the prefix "ACCRUAL"	
				, IRS_KEY::BUSINESSDAYADJUSTMENT
				, IRS_KEY::CALENDAR
				, IRS_KEY::DAYCOUNT
				, IRS_KEY::FREQUENCY

                // fields for fixed/float 
                , IRS_KEY::FIXED_RATE
				, IRS_KEY::FLOAT_SPREAD

                // fields for Bond Schedules
                , BOND_KEY::FACE_VALUE
                , BOND_KEY::COUPON
                , BOND_KEY::ISSUE_DATE
                , BOND_KEY::ACCRUAL_START_DATE
                , BOND_KEY::FIRST_COUPON_DATE
                , BOND_KEY::LAST_COUPON_DATE
                , BOND_KEY::CALCULATION_TYPE
                , BOND_KEY::ISSUE_PRICE
                , BOND_KEY::TAX_RATE
                , BOND_KEY::YIELD_FREQUENCY
				, BOND_KEY::EX_DIVIDEND_TENOR
				, BOND_KEY::EX_DIVIDEND_BUSINESSDAYADJUSTMENT
				, BOND_KEY::QUOTED_MARGIN

            };

            std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
			return expectedKeys;
		};

	protected:

        //Schedule input parameters
		ScheduleTypeEnum scheduleType_;
		BespokeScheduleTypeEnum bespokeScheduleType_;
		PayReceiveEnum payerReceiver_;

   	    double notional_;
        NotionalExchangeEnum notionalExchangeEnum_;

        //Amortization, set to zero for no amortization
        double amortization_; 
        //Amortization, set to zero for no amortization
   	    unsigned int amortFrequency_; 

        //For Zero Coupon Swap:
        double futureValueNotional_;
        FrequencyEnum paymentFreqEnum_;

		double leverage_;
		double couponMultiplier_;
		AQLString accrualStartDate_;			
        AQLString accrualEndDateOrTenor_;		
		BusinessDayAdjustmentEnum accrualbusinessDayAdj_;		
        AQLString accrualCalendar_;			
        FrequencyEnum accrualFrequency_;			
        DayCountEnum accrualDaycount_;			
		BusinessDayAdjustmentEnum paymentbusinessDayAdj_;		
        AQLString paymentCalendar_;			
    	FrequencyEnum paymentFrequency_;			
		AQLString payLag_;						
		AQLString rollDayInput_;				
		StubTypeEnum stubType_;
	    AQLString firstStub_;					
        AQLString lastStub_;					
        AQLString fixingAdvanceOrArrears_;
		BusinessDayAdjustmentEnum fixingbusinessDayAdj_;		
        AQLString fixingCalendar_;			
        AQLString fixLag_;	
        AQLString firstFixLag_;	
        AQLString fxFixingLag_;	
        BusinessDayAdjustmentEnum fxFixingbusinessDayAdj_;	
        AQLString fxFixingCalendar_;	

        //Schedule outputs
		DateVector		fixingDates_; //Fixing start dates
		DateVector		fixingEndDates_;
		DateVector		paymentDates_;
		DateVector		accrualStartDates_;
		DateVector		accrualEndDates_;
		DoubleVector	accrualYearFractions_;
		IntVector		accrualDays_;

		AQLDate			effectiveDate_;
		AQLDate			unadjustedMaturityDate_;

		std::vector<CashflowPtr> cashflows_;
        //the first cashflow when the notional exchange is type START or START_END    
        CashflowPtr upfrontCashflow_;
		LabelValueBlock inputParameters_;

        virtual void populateAccrualStartDates(const LabelValueBlock& scheduleLVB);

		//return all accrualDates with accrualStart and accrualEnd
		virtual DateVector generateAccrualAndPaymentSchedules();
		
		virtual void calculateScheduleDates();

        virtual void createUpfrontCashflow(const AQLDate& paymentDate, double leverage);
        virtual void createCashflows();
        virtual void createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs); 
		
		virtual void populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB=LabelValueBlock());

		virtual double getFinalCashflowNotionalExchange(const double& notional) const;
        std::vector<double> getCashflowNotionals(const size_t cashflowSize); 

        // Check if the the schedule has irregular stub
        void determineIsIrregularStub();

   protected:
        
        bool isIrregularStub_;

	};

	typedef std::shared_ptr<Schedule> SchedulePtr;

}

