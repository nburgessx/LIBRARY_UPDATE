#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include <boost/lexical_cast.hpp>
#include "LabelValueBlock.h"
#include "Swap.h"
#include "Schedule.h"
#include "CoreEnumerations.h"
#include "Cashflow.h"
#include "FreeObject.h"
#include "LegGenerator.h"

using etrading::LabelValueBlock;

namespace etrading
{
    /* @brief			Get the schedule type enum from the given legName
    * @param [in]		legName  Leg name, e.g. leg1:fixed
    * @output			Schedule type enum
    */
    ScheduleTypeEnum getScheduleTypeFromLegName( const AQLString& legName);

	/* @brief			Return a leg pointer based on the Leg Label Value Block and Schedule
    *  @param [in]		legLVB              Leg label value block
    *  @param [in]		legObjectName Leg   object name
    *  @param [in]		schedule            Schedule of the leg
    *  @return			Leg pointer
    */
	LegPtr createLegByLVB(const LabelValueBlock& legLVB, const std::string& legObjectName="legObjectName", const SchedulePtr& schedule={});


	/* @brief Create Schedule object based on the label value block
    *  @param [in] scheduleName	   Schedule Name
    *  @param [in] swapScheduleLVB Schedule Label Value Block
    *  @Return     a pointer to the schedule object
    */
	std::shared_ptr<Schedule> createSchedule(const std::string& scheduleName, const LabelValueBlock& swapScheduleLVB);

	/* @brief Create a Bespoke Schedule object based on the label value blocks of schedule properties and schedule cashflows
	*  @param [in]		scheduleName				Bespoke schedule name
	*  @param [in]		bespokeScheduleProperties	Bespoke schedule properties label value block
	*  @param [in]		cashflowLVBs				Bespoke schedule cashflow label value block
	*  @param [in]		bespokeScheduleType			Bespoke type - internally used
	*  @Return     a pointer to the schedule object
	*/
	std::shared_ptr<Schedule> createScheduleBespoke(const std::string& scheduleName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs, const BespokeScheduleTypeEnum& bespokeScheduleType);

	/* @brief Create Swap object  on the leg label value blocks
	*  @param [in]	swapName		    Swap name
	*  @param [in]	leg1			    Leg1
	*  @param [in]	leg2			    Leg2 
	*  @param [in]	swapPropertiesLVB	Swap level properties
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwapFromLegs(const std::string& swapName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB);

    /* @brief Create Swap object from leg objects
    *  @param [in]	swapName		Swap name
	*  @param [in]	legs			Legs
	*  @param [in]	swapPropertiesLVB	Swap level properties
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwapFromLegs(const std::string& swapName, const std::vector<LegPtr>& legs, const LabelValueBlock& swapPropertiesLVB);

	/* @brief Create Swap object based on the leg label value blocks
	*  @param [in]	swapName		Swap name
	*  @param [in]	leg1LVB			Leg1 label value block 
	*  @param [in]	leg2LVB			Leg2 label value block 
	*  @param [in]	swapPropertiesLVB	Swap level properties
	*  @param [in]	schedule1		Schedule1	
	*  @param [in]	schedule2		Schedule2	
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwap(const std::string& swapName, const LabelValueBlock& leg1LVB, const LabelValueBlock& leg2LVB, const LabelValueBlock& swapPropertiesLVB, const SchedulePtr& schedule1={}, const SchedulePtr& schedule2={});

	/* @brief Create Swap object based on the legs' label value block
	*  @param [in]	swapName		Swap name
	*  @param [in]	legsLVB			The label value block for multiple legs
	*  @param [in]	swapPropertiesLVB	Swap level properties
	*  @param [in]	schedules		Schedules for multiple legs 
    *  @Return		a pointer to the Swap object
    */
	std::shared_ptr<Swap> createSwap(const std::string& swapName, const std::vector<LabelValueBlock>& legsLVB, const LabelValueBlock& swapPropertiesLVB, const std::vector<SchedulePtr>& schedules = std::vector<SchedulePtr>());

	/* @brief Create Swap object based on a swapGenerator config
	*  @param [in]	swapName			Swap name
	*  @param [in]	swapGeneratorName	Name of the AQObj SwapGenerator to use
	*  @param [in]	expressionLVB		Additional swap config
	*  @param [in]	swapPropertiesLVB	Swap level properties
	*  @param [in]	isXccySwap			Boolean flag which specifies whether this is an ordinary IRS or a Xccy swap
    *  @Return		a pointer to the Swap object
	*/
	std::shared_ptr<Swap> createSwapFromGenerator(const std::string& swapName, const std::string& swapGeneratorName, const LabelValueBlock& expressionLVB, const LabelValueBlock& swapPropertiesLVB, bool isXccySwap );


	/* @brief Create a swap that is back to back to the original swap
	*  @param [in]	originalSwap The original swap
	*  @param [in]	swapName	 The new swap name

    *  @Return		a pointer to a new Swap object, 
    */
	std::shared_ptr<Swap> createBackToBackSwap(const std::shared_ptr<Swap> originalSwap, const std::string& swapName);

	/* @brief label value block keys for the swap leg
    *  @param [in]	legName				Leg name, e.g. leg1:fixed
	*  @param [in]	withScheduleKeys	True to include keys from Schedule (Fixed/Float). Default to true
    *  @Return     label value block keys for the swap leg
    */
	std::vector<std::string> getSwapLegLVBKeys(const AQLString& legName, bool withScheduleKeys=true); 

	/* @brief label value block keys for the swap schedule
    *  @param [in] scheduleType	   Schedule type, e.g. fixed/float/fee
    *  @Return     label value block keys for the swap schedule
    */
	std::vector<std::string> getScheduleLVBKeys(const AQLString& scheduleType=""); 

	/* @brief			Validate if the swap keys, with or without prefix "fixed"/"float" 
	*  @param [in]		legName				Leg name, e.g. leg1:fixed
	*  @param [in]		keysFromUser		A list of keys from user input
	*  @param [in]		validateKeys		True to do the verification. Default to true
	*  @param [in]		withScheduleKeys	True to include keys from Schedule (Fixed/Float). Default to true
	*/
	void validateSwapLegLVBKeys(const AQLString& legName, const std::vector<std::string>& keysFromUser, bool validateKeys=true, bool withScheduleKeys=true);


	/* @brief Create a Bespoke Schedule object based on the label value blocks of schedule properties and schedule cashflows
	*  @param [in]		scheduleName				Fee schedule name
	*  @param [in]		cashflowLVBs				Fee cashflow label value blocks
	*  @Return     a pointer to the schedule object
	*/
	std::shared_ptr<Schedule> createFeeSchedule(const std::string& scheduleName, const std::vector<LabelValueBlock>& cashflowLVBs);

    /* @brief Create a Bespoke Schedule object based on the label value blocks of schedule properties and schedule cashflows 
	*  @param [in]		feeName				        Fee leg name
	*  @param [in]		feeProperties			    Fee properties label value block
	*  @param [in]		cashflowLVBs				Fee cashflow label value blocks
    *  @Return     a pointer to the schedule object
    */
	LegPtr createFeeLeg(const std::string& feeName, const LabelValueBlock& feeProperties, const std::vector<LabelValueBlock>& cashflowLVBs);

    /* @brief Create a map of leg's inputParameters
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a map of Leg inputParameters LVB
    */
    std::map<std::string, LabelValueBlock> createLegInputParametersFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex);

    /* @brief Create a LegGenerator
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a pointer to the LegGenerator
    */
    std::map<std::string, LegGenerator> createLegGeneratorFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex);

    /* @brief Create a Schedule map
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a map of schedule object
    */
    std::map<std::string, SchedulePtr> createSchedulesFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex);

    /* @brief Create a Schedule map
    *  @param [in]		freeObject				freeObject
	*  @param [in]		objectName			    objectName
	*  @param [in]		schemaNameHasIndex		True to indicate the schema name has index
    *  @Return     a map of schedule object
    */
    std::map<std::string, SchedulePtr> createBespokeSchedulesFromFreeObject(const FreeObject& freeObject, const std::string& objectName, bool schemaNameHasIndex);

    //Helper methods for createXXXFromFreeObject
    std::string getIndexFromSchemaName(const std::string& schemaName, bool schemaNameHasIndex);
    std::string getSchemaNameWithoutIndex(const std::string& schemaName, bool schemaNameHasIndex);
	std::vector<std::string> getMatchingSchemaNames(const std::vector<std::string>& schemaNames, const std::unordered_set<std::string> searchNames, bool schemaNameHasIndex);
	
	bool hasBespokeSchedule(const FreeObject& freeObject, bool schemaNameHasIndex);


    void validateSwapStaticDataObject(const LabelValueBlock& swapPropertiesLVB, bool isXccySwap, bool validateKeys);

    /* @brief			Function to check if a swap input LVB is equal to an existing swap LVB registered on the cache
    * @param [in]		swapName			Existing Swap Name
    * @param [in]		leg1ScheduleType	Leg1 Schedule Type: FIXED, FLOAT, FEE
    * @param [in]		leg1LVB		        Leg1 Label Value Block
    * @param [in]		leg2ScheduleType	Leg2 Schedule Type: FIXED, FLOAT, FEE
    * @param [in]		leg2LVB		        Leg2 Label Value Block
    * @output			Returns swap shared pointer if the swap exists already and it's inputs are unchanged
    */
    std::shared_ptr<Swap> useExistingSwapIfGeneratorInputsUnchanged( const std::string& swapName, const ScheduleTypeEnum& leg1Type, const LabelValueBlock& leg1LVB, const ScheduleTypeEnum& leg2Type, const LabelValueBlock& leg2LVB );

	/* @brief Finds the leg of the specified type within a provided LegCollection.
	*         If the legName is provided, attempts to find that leg and verifies the legType
	*         If the legName is empty and more than leg of the specified type exists, the method throws an exception
	*
	* @param[in] legName	Optional, allowed to be an empty string
	* @param[in] legType	Mandatory, specifies the desired leg type
	* @param[in] legs		Mandatory, specifies a collection of legs to search for the required legType
	* @returns The validated leg name matching the specified legType
	*/
	AQLString validateLegName( const AQLString& legName, ScheduleTypeEnum legType, const LegCollection& legs );

	/* @brief Given an accrual frequency, returns the corresponding number of coupons per year.
	*
	* @param[in] accrualFrequency	Accrual frequency
	* @returns   the number of coupons per year. For example SEMI_ANNUAL_FREQUENCY returns 2.
	*/
	size_t convertFrequenyToCouponsPerYear( const FrequencyEnum accrualFrequency );

	// Check if a swap is a fixed float swap
	bool isFixedFloatSwap(const SwapPtr& swap);

	// Check if a swap is a fixed float swap
	LegPtr getFixedLeg(const SwapPtr& swap);

	LegPtr getFloatLeg(const SwapPtr& swap);

}


