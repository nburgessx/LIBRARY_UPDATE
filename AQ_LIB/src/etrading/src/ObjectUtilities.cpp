
#include <boost/assign.hpp>

#include "ObjectUtilities.h"

namespace etrading
{
	FreeObject CreateFreeObject(const rapidjson::Value& jsonDoc )
	{
        checkObjectLabels(jsonDoc);

        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();
        FreeObject freeObject( objectName );

        std::vector<DataSchema> dataSchemas;
        std::map<std::string, std::vector<std::vector<Variant>> > dataPerKey;

        const Value& skArray = jsonDoc[DATASCHEMAS];
        // assert(skArray.IsArray());
        for ( SizeType skCounter = 0; skCounter < skArray.Size(); skCounter++ ) // Uses SizeType instead of size_t
        {
            // typedef std::map<std::string, std::vector<std::vector<Variant>>>::const_iterator It_type;
            std::pair<DataSchema, std::vector<std::vector<Variant>>> skResult =
                createDataSchemaWithDataFromJSON( skArray[skCounter] );

            dataSchemas.push_back( skResult.first );
            dataPerKey.insert( std::map<std::string, std::vector<std::vector<Variant>>>::value_type
                               ( skResult.first.getName(), skResult.second ) );
        }

        freeObject.setDataSchemas( dataSchemas );
        for( auto iterator = dataPerKey.cbegin(); iterator != dataPerKey.cend(); iterator++ )
        {
            unsigned int numberOfColumnsForSK = ( *iterator ).second.size();
            for( unsigned int colCounter = 0u; colCounter < numberOfColumnsForSK; colCounter++ )
            {
                freeObject.setColumnData( ( *iterator ).first, colCounter, ( *iterator ).second[ colCounter ] );
            }
        }

        return freeObject;	
	
	};

	FreeObject createFreeObjectFromGrid( const std::string& objectName,
										 const std::vector<std::string>& columnNames,
										 const std::vector<ContainedTypeEnum>& colTypes,
										 const VariantMatrix& rangeData,
										 const std::string& schemaName,
										 const bool allowJaggedData )
	{
        if( !allowJaggedData )
        {
			// Perform sanity check on rangeData
			unsigned int colCounter = 0;
			unsigned int numberOfRows = 0;
            
            if( rangeData.size() != 0 )
            {
                // Prevent access violation when rangeData is empty
                numberOfRows = rangeData[colCounter].size();
                
			    // Loop over remaining columns to ensure they have the same dimensions as column zero
	            while( colCounter < ( rangeData.size() - 1 ) )
	            {
	                int numberOfRowsHere = rangeData[ ++colCounter ].size();
	                if( numberOfRowsHere  != numberOfRows )
	                {
	                    // +1 because this display is not zero-indexed
	                    { std::ostringstream aqCoreMsg23;
aqCoreMsg23 << "Number of data points in data range is not equal for every column. For column " << ( colCounter + 1 ) << " there are " << numberOfRowsHere << " data points, but for column " << colCounter << " there are " << numberOfRows << " data points"; AQ_THROW( aqCoreMsg23.str() ); }
	                }
                }
            }
		}

        std::vector<DataSchema> dataSchema;
		dataSchema.push_back( DataSchema( schemaName, colTypes.size(), colTypes, columnNames ) );

		FreeObject fo( objectName );
		fo.setDataSchemas( dataSchema );
		fo.setDataForSchema( schemaName, rangeData, colTypes );

		return fo;

	};


    void checkObjectLabel(const rapidjson::Value& jsonDoc, const char* objectLabelName )
    {
        if(!jsonDoc.HasMember(objectLabelName))
        {
            { std::ostringstream aqMsg148;
aqMsg148 << "JSON document does not contain a '" << objectLabelName << "' member "; AQ_THROW( aqMsg148.str() ); }
        }
    };

    void checkObjectLabels(const rapidjson::Value& jsonDoc)
    {
        checkObjectLabel(jsonDoc, DataHolder::OBJECT_NAME_LABEL.c_str());
        checkObjectLabel(jsonDoc, DATASCHEMAS);
        if(!jsonDoc[DATASCHEMAS].IsArray())
        {
            { std::ostringstream aqMsg149;
aqMsg149 << "Error reading file: JSON Element '" << DATASCHEMAS << "' is not an array "; AQ_THROW( aqMsg149.str() ); }                    
        }
        if(jsonDoc[DATASCHEMAS].Size()<1)
        {
            { std::ostringstream aqMsg150;
aqMsg150 << "Error reading file: JSON Element '" << DATASCHEMAS << "' is an empty array "; AQ_THROW( aqMsg150.str() ); }                    
        }
    };

    void checkValidDataSchema( const rapidjson::Value& inputValue )
    {
        static const char*  SK_NAME = "SK_NAME";
        static const char*  SK_NUMBER_OF_COLUMNS = "SK_NUMBER_OF_COLUMNS";
        static const char*  SK_COLUMN_TYPES = "SK_COLUMN_TYPES";

        if(!inputValue.HasMember(SK_NAME))
        {
            { std::ostringstream aqMsg151;
aqMsg151 << "JSON Element does not have a '" << SK_NAME << "' member: '" << createStringFromJSONValue(inputValue) << "' "; AQ_THROW( aqMsg151.str() ); }
        }
        rapidjson::Type nameType = inputValue[SK_NAME].GetType();
        if(nameType != rapidjson::kStringType)
        {
            { std::ostringstream aqMsg152;
aqMsg152 << "JSON Element '" << SK_NAME << "' should hold a string: '" << createStringFromJSONValue(inputValue[SK_NAME]) << "' "; AQ_THROW( aqMsg152.str() ); }            
        }
        if(!inputValue.HasMember(SK_NUMBER_OF_COLUMNS))
        {
            { std::ostringstream aqMsg153;
aqMsg153 << "JSON Element does not have a '" << SK_NUMBER_OF_COLUMNS << "' member: '" << createStringFromJSONValue(inputValue) << "' "; AQ_THROW( aqMsg153.str() ); }
        }
        if(!inputValue.HasMember(SK_COLUMN_TYPES))
        {
            { std::ostringstream aqMsg154;
aqMsg154 << "JSON Element does not have a '" << SK_COLUMN_TYPES << "' member: '" << createStringFromJSONValue(inputValue) << "' "; AQ_THROW( aqMsg154.str() ); }
        }
        rapidjson::Type type = inputValue[SK_NUMBER_OF_COLUMNS].GetType();
        if(type != rapidjson::kNumberType)
        {
            { std::ostringstream aqMsg155;
aqMsg155 << "JSON Element '" << SK_NUMBER_OF_COLUMNS << "' should hold an integer: '" << createStringFromJSONValue(inputValue[SK_NUMBER_OF_COLUMNS]) << "' "; AQ_THROW( aqMsg155.str() ); }
        }
        const int numberOfColumns =  inputValue[SK_NUMBER_OF_COLUMNS].GetInt();
        if(memberCount(inputValue) < (3 + numberOfColumns))
        {
            { std::ostringstream aqMsg156;
aqMsg156 << "JSON Element has less than " << (3 + numberOfColumns) << " members: '" << createStringFromJSONValue(inputValue) << "' "; AQ_THROW( aqMsg156.str() ); }
        }
        if(inputValue[SK_COLUMN_TYPES].Size() != numberOfColumns)
        {
            { std::ostringstream aqMsg157;
aqMsg157 << "JSON Error: number of column types does correspond to number of columns ('" << numberOfColumns << "') : '" << createStringFromJSONValue(inputValue[SK_COLUMN_TYPES]) << "' "; AQ_THROW( aqMsg157.str() ); }                    
        }
    };

    void checkArrayIndexLabel(  const rapidjson::Value& inputValue, 
                                const rapidjson::SizeType& minSize )
    {
        if(!inputValue.IsArray())
        {
            { std::ostringstream aqMsg158;
aqMsg158 << "Error reading file: JSON Element '" << createStringFromJSONValue(inputValue) << "' is not an array "; AQ_THROW( aqMsg158.str() ); }                    
        }

        if(minSize > inputValue.Size())
        {
            { std::ostringstream aqMsg159;
aqMsg159 << "Error reading file: JSON Array Size is less than " << minSize << " : '" << createStringFromJSONValue(inputValue) << "'"; AQ_THROW( aqMsg159.str() ); }                            
        }
    };

    const int memberCount(const rapidjson::Value& inputValue )
    {
        int count = 0;
        for (rapidjson::Value::ConstMemberIterator itr = inputValue.MemberBegin();    
            itr != inputValue.MemberEnd(); ++itr)
        {
            count++;
            //printf("Type of member %s is %s\n",itr->name.GetString(), kTypeNames[itr->value.GetType()]);
        }  
        return count;
    };    
    
    template<>
    typename to_cached_object_type<SCHEDULE>::type createCacheableObject<SCHEDULE>( const rapidjson::Value& jsonDoc )
    {
        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();

        FreeObject freeObject = CreateFreeObject(jsonDoc);

        auto scheduleMap = createSchedulesFromFreeObject(freeObject, objectName, false);
        if (scheduleMap.size() != 1)
        {
          { std::ostringstream aqMsg160;
aqMsg160 << "A Schedule jason file must have one schedule section " << objectName; AQ_THROW( aqMsg160.str() ); }
        }
        auto mySchedule = scheduleMap.begin()->second;
        return mySchedule;
	};	

    template<>
    typename to_cached_object_type<LEG>::type createCacheableObject<LEG>( const rapidjson::Value& jsonDoc )
    {
        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();
        FreeObject freeObject = CreateFreeObject(jsonDoc);

		//Get leg's input parameters
		auto legPropertiesMap = createLegInputParametersFromFreeObject(freeObject, objectName, false);
		if (legPropertiesMap.size() != 1)
		{
			{ std::ostringstream aqMsg161;
aqMsg161 << "A Leg jason file must have a legProperties section " << objectName; AQ_THROW( aqMsg161.str() ); }
		}

		//Get Bespoke schedule
		auto bespokeScheduleMap = createBespokeSchedulesFromFreeObject(freeObject, objectName, false);

		size_t bespokeScheduleSize = bespokeScheduleMap.size();

		if (bespokeScheduleSize != 0 && bespokeScheduleSize != 1)
        {
          { std::ostringstream aqMsg162;
aqMsg162 << "A Leg jason file must have a schedule section " << objectName; AQ_THROW( aqMsg162.str() ); }
        }

		SchedulePtr schedule = (bespokeScheduleSize != 0) ? bespokeScheduleMap.begin()->second : SchedulePtr();

		//Create leg object
		LegPtr myLeg = createLegByLVB(legPropertiesMap.begin()->second, objectName, schedule);

        return myLeg;
	};

    template<>
    typename to_cached_object_type<BOND>::type createCacheableObject<BOND>( const rapidjson::Value& jsonDoc )
    {
        std::string objectName  = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();
        FreeObject freeObject   = CreateFreeObject(jsonDoc);
        
        LabelValueBlock bondLVB      = createBondLVBFromFreeObject(freeObject, objectName);  
        
        if (bondLVB.size() == 0 )
        {
          { std::ostringstream aqMsg163;
aqMsg163 << "The Bond file for " << objectName << " does not contain a Bond Description Label Value Block, which is required."; AQ_THROW( aqMsg163.str() ); }
        }

		BondPtr myBond = createBondFromSingleLVB( objectName, bondLVB);
        
        return myBond;
	};

	template<>
    typename to_cached_object_type<BOND_GENERATOR>::type createCacheableObject<BOND_GENERATOR>( const rapidjson::Value& jsonDoc )
    {
        FreeObject freeObject = CreateFreeObject( jsonDoc );
        const std::string objectName = freeObject.getRefToName();
		BondGenerator bondGenerator( objectName, freeObject );
		return bondGenerator;
    };

    template<>
	typename to_cached_object_type<SWAP_OBJECT>::type createCacheableObject<SWAP_OBJECT>( const rapidjson::Value& jsonDoc )
    {
      
        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();

        FreeObject freeObject = CreateFreeObject(jsonDoc);

        //Get swapProperties
        LabelValueBlock swapProperties; 
        const auto schemaNames = freeObject.keyNames();
        for(unsigned int i =0; i < schemaNames.size(); i++)
		{
            //Get Leg Static Data
            auto schemaName = schemaNames[i];
            std::string index = getIndexFromSchemaName(schemaName, false);
            std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, false);
            if(schemaNameWithoutIndex == toString(SWAP_OBJECT))
		    {
			    auto keys = freeObject.getValuesCopy(schemaName,0);
			    auto values = freeObject.getValuesCopy(schemaName,1);
                swapProperties = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
                break;
            }    
        }

        auto legPropertiesMap = createLegInputParametersFromFreeObject(freeObject, objectName, true);

 		auto scheduleMap = createSchedulesFromFreeObject(freeObject, objectName, true);
		
		size_t scheduleMapSize = scheduleMap.size();

        if (legPropertiesMap.size() < 2 )
        {
          { std::ostringstream aqMsg164;
aqMsg164 << "A Swap jason file must have at least two pairs of legProperties " << objectName; AQ_THROW( aqMsg164.str() ); }
        }

		//If bespoke schedule is not empty, it must have the same side as leg's properties map
        if (scheduleMapSize > 0 && legPropertiesMap.size() != scheduleMapSize)
        {
          { std::ostringstream aqMsg165;
aqMsg165 << "A Swap jason file must have same numbers of legProperties and schedules " << objectName; AQ_THROW( aqMsg165.str() ); }
        }

        std::vector<LegPtr> legs;

		for(auto iter = legPropertiesMap.begin(); iter != legPropertiesMap.end(); iter++) 
        {
			auto key = iter->first;

			auto legProperties = legPropertiesMap[key];

			SchedulePtr schedule;

			if (scheduleMapSize > 0 && scheduleMap.find(key) != scheduleMap.end())
			{
	            schedule = scheduleMap[key];
			}
            auto leg = createLegByLVB(legProperties, objectName, schedule);

            legs.push_back(leg);
        }

        std::shared_ptr<Swap> mySwap = createSwapFromLegs(objectName, legs, swapProperties);

		return mySwap;
    };

    template<>
	typename to_cached_object_type<SWAP_GENERATOR>::type createCacheableObject<SWAP_GENERATOR>( const rapidjson::Value& jsonDoc )
    {

        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
        auto legGeneratorsMap = createLegGeneratorFromFreeObject(freeObject, objectName, true);
        
        if (legGeneratorsMap.size() != 2 )
        {
          { std::ostringstream aqMsg166;
aqMsg166 << "A SwapGenerator jason file must have two of legGenerators" << objectName; AQ_THROW( aqMsg166.str() ); }
        }

        std::vector<LegGenerator> legGenerators;
        for(auto iter = legGeneratorsMap.begin(); iter != legGeneratorsMap.end(); iter++) 
        {
            auto legGen = legGeneratorsMap[iter->first];
            legGenerators.push_back(legGen);
        }

        SwapGenerator swapGen(objectName, legGenerators.at(0), legGenerators.at(1));
		return swapGen;
    };

    template<>
	typename to_cached_object_type<CURVE_DATA>::type createCacheableObject<CURVE_DATA>( const rapidjson::Value& jsonDoc )
    {
		FreeObject freeObject = CreateFreeObject(jsonDoc);
		std::shared_ptr<CurveData> ptrMktData;
		const auto schemaNames = freeObject.keyNames();
		for(unsigned int i =0u; i < schemaNames.size(); i++)
		{
			if(schemaNames[i] == SwapQuotes::getSchemaName())
			{
				auto information = freeObject.getValuesCopy(schemaNames[i],0);
				if(information.size() != 4)
				{
					{ std::ostringstream aqMsg167;
aqMsg167 << "DataSchema with name " << schemaNames[i] << " in object named " << freeObject.getRefToName() << " (read from JSON) is not a Swap Quote"; AQ_THROW( aqMsg167.str() ); }
				};
				const CurveTenorEnum curveTenor = etrading::toCurveTenorEnum(information[1].toString());
				if(ptrMktData == nullptr)
				{
					ptrMktData = std::make_shared<CurveData>(freeObject.getRefToName(),curveTenor);
				};
				
				SwapQuotes sqts( Variant::createValueVector<std::string>(freeObject.getValuesCopy(schemaNames[i],1)) ,
								 Variant::createValueVector<double>(freeObject.getValuesCopy(schemaNames[i],2)),
								 toSwapQuoteTypeEnum(information[3].toString()),
								 toCCYEnum(information[0].toString()),
								 curveTenor );
				ptrMktData->setSwapQuotes(std::move(sqts));

			} else if(schemaNames[i] == BasisSwapQuotes::getSchemaName())
			{
				// first column: CCY, CurveTenor, ToCurveTenor, mdType, SwapType // second & third = maturity,quote
				auto information = freeObject.getValuesCopy(schemaNames[i],0);
				if(information.size() != 5)
				{
					{ std::ostringstream aqMsg168;
aqMsg168 << "DataSchema with name " << schemaNames[i] << " in object named " << freeObject.getRefToName() << " (read from JSON) is not a Basis Swap Quote"; AQ_THROW( aqMsg168.str() ); }
				};
				const CurveTenorEnum curveTenor = etrading::toCurveTenorEnum(information[1].toString());
				const CurveTenorEnum toCurveTenor = etrading::toCurveTenorEnum(information[2].toString());
				if(ptrMktData == nullptr)
				{
					ptrMktData = std::make_shared<CurveData>(freeObject.getRefToName(),curveTenor);
				};
				BasisSwapQuotes bsqts(Variant::createValueVector<std::string>(freeObject.getValuesCopy(schemaNames[i],1)) ,
								 Variant::createValueVector<double>(freeObject.getValuesCopy(schemaNames[i],2)),
								 toCCYEnum(information[0].toString()),
								 curveTenor,
								 toCurveTenor);
				ptrMktData->setSwapQuotes(std::move(bsqts));
			} else if(schemaNames[i] == CentralBankSwapQuotes::getSchemaName())
			{
				// first column: CCY, CurveTenor, mdType, CentralBankType // second & third = maturity,quote  // fourth & fifth = to- and fromDates
				auto information = freeObject.getValuesCopy(schemaNames[i],0);
				if(information.size() != 4)
				{
					{ std::ostringstream aqMsg169;
aqMsg169 << "DataSchema with name " << schemaNames[i] << " in object named " << freeObject.getRefToName() << " (read from JSON) is not a Central Bank Swap Quote"; AQ_THROW( aqMsg169.str() ); }
				};
				const CurveTenorEnum curveTenor = etrading::toCurveTenorEnum(information[1].toString());
				if(ptrMktData == nullptr)
				{
					ptrMktData = std::make_shared<CurveData>(freeObject.getRefToName(),curveTenor);
				};
				CentralBankSwapQuotes cbsqts(
					Variant::createValueVector<std::string>(freeObject.getValuesCopy(schemaNames[i],1)) ,
					Variant::createValueVector<double>(freeObject.getValuesCopy(schemaNames[i],2)),
					Variant::createValueVector<boost::gregorian::date>(freeObject.getValuesCopy(schemaNames[i],3)),
					Variant::createValueVector<boost::gregorian::date>(freeObject.getValuesCopy(schemaNames[i],4)),
					toCentralBankTypeEnum(information[3].toString()),
					toCCYEnum(information[0].toString()),
					curveTenor );  // should we allow for non 1D tenor central bank swaps??
				ptrMktData->setCBSwapQuotes(std::move(cbsqts));
			} else if(schemaNames[i] == FRAQuotes::getSchemaName())
			{
				auto information = freeObject.getValuesCopy(schemaNames[i],0);
				if(information.size() != 3)
				{
					{ std::ostringstream aqMsg170;
aqMsg170 << "DataSchema with name " << schemaNames[i] << " in object named " << freeObject.getRefToName() << " (read from JSON) is not a FRA Quote"; AQ_THROW( aqMsg170.str() ); }
				};
				const CurveTenorEnum curveTenor = etrading::toCurveTenorEnum(information[1].toString());
				if(ptrMktData == nullptr)
				{
					ptrMktData = std::make_shared<CurveData>(freeObject.getRefToName(),curveTenor);
				};
				FRAQuotes fra_qts( Variant::createValueVector<std::string>(freeObject.getValuesCopy(schemaNames[i],1)) ,
								 Variant::createValueVector<double>(freeObject.getValuesCopy(schemaNames[i],2)),
								 toFRAPeriodEnum(curveTenor),
								 toCCYEnum(information[0].toString()));
				ptrMktData->setFRAQuotes(std::move(fra_qts));
			} else if(schemaNames[i] == IRFuturesQuotes::getSchemaName())
			{
				auto information = freeObject.getValuesCopy(schemaNames[i],0);
				if(information.size() != 3)
				{
					{ std::ostringstream aqMsg171;
aqMsg171 << "DataSchema with name " << schemaNames[i] << " in object named " << freeObject.getRefToName() << " (read from JSON) is not an IR Futures Quote"; AQ_THROW( aqMsg171.str() ); }
				};
				const CurveTenorEnum curveTenor = etrading::toCurveTenorEnum(information[1].toString());
				if(ptrMktData == nullptr)
				{
					ptrMktData = std::make_shared<CurveData>(freeObject.getRefToName(),curveTenor);
				};
				IRFuturesQuotes irfutures_qts(
					Variant::createValueVector<std::string>(freeObject.getValuesCopy(schemaNames[i],1)) ,
					Variant::createValueVector<double>(freeObject.getValuesCopy(schemaNames[i],2)),
					Variant::createValueVector<boost::gregorian::date>(freeObject.getValuesCopy(schemaNames[i],3)),
					Variant::createValueVector<boost::gregorian::date>(freeObject.getValuesCopy(schemaNames[i],4)),
					toCCYEnum(information[0].toString()),
					toFRAPeriodEnum(curveTenor),
					Variant::createValueVector<double>(freeObject.getValuesCopy(schemaNames[i],5))
					);
				ptrMktData->setIRFuturesQuotes(std::move(irfutures_qts));
				// first column: CCY, CurveTenor (same as IR Period), mdType 
				// second & third = termtickers,quote  
				// fourth & fifth = to- and fromDates 
				// sixth=convextiyModelParams
			} else if(schemaNames[i] == FXQuotes::getSchemaName())
			{
				auto information = freeObject.getValuesCopy(schemaNames[i],0);
				if(information.size() != 5)
				{
					{ std::ostringstream aqMsg172;
aqMsg172 << "DataSchema with name " << schemaNames[i] << " in object named " << freeObject.getRefToName() << " (read from JSON) is not an FX Quote"; AQ_THROW( aqMsg172.str() ); }
				};
				const CurveTenorEnum curveTenor = etrading::toCurveTenorEnum(information[3].toString());
				if(ptrMktData == nullptr)
				{
					ptrMktData = std::make_shared<CurveData>(freeObject.getRefToName(),curveTenor);
				};
				FXQuotes fx_qts(
					Variant::createValueVector<std::string>(freeObject.getValuesCopy(schemaNames[i],1)) ,
					Variant::createValueVector<double>(freeObject.getValuesCopy(schemaNames[i],2)),
					toCCYEnum(information[0].toString()),
					toCCYEnum(information[1].toString()),
					information[2].getValue<bool>()
					);
				ptrMktData->setFXQuotes(std::move(fx_qts));
			} 		
		} // for(int i =0; i < schemaNames.size(); i++)

		if(ptrMktData == nullptr)
		{
			{ std::ostringstream aqMsg173;
aqMsg173 << "No Market Data Quotes were found in the file, available schema names were: " << etrading::containerAsString(schemaNames); AQ_THROW( aqMsg173.str() ); }
		}
			
		return CurveData(*ptrMktData.get());
	};	


    
    template<>
    typename to_cached_object_type<CURVE_DEPRECATED>::type createCacheableObject<CURVE_DEPRECATED>( const rapidjson::Value& jsonDoc )
    {
        // TODO: we could also create a FreeObject and simply query the FreeObject to get
        // the data for building an AQObjCurve instead of using rapidjson (and having to check the formation of the file all the time).

        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();
        std::vector<DataSchema> dataSchemas = StaticStructureStore::getInstance().getCachedObjectStructure( CURVE_DEPRECATED );

        const bool hasCurveBuildProperties = ( jsonDoc[DATASCHEMAS].Size() > 1 );

        checkArrayIndexLabel(jsonDoc[DATASCHEMAS],2);

        SizeType skIndex = hasCurveBuildProperties ? 1 : 0;
        const std::string& dateColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[0];
        const std::string& discountFactorColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[1];
        const std::string& forwardRatesColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[2];

        checkValidDataSchema(jsonDoc[DATASCHEMAS][skIndex]);
        const Value& skDateArray = jsonDoc[DATASCHEMAS][skIndex][dateColumnName.c_str()];
        const Value& skDiscountFactorsArray = jsonDoc[DATASCHEMAS][skIndex][discountFactorColumnName.c_str()];
        const Value& skForwardRatesArray = jsonDoc[DATASCHEMAS][skIndex][forwardRatesColumnName.c_str()];

        std::vector<boost::gregorian::date> dates = populateDateVector( skDateArray );
        std::vector<double> discountFactors = populateDoubleVector( skDiscountFactorsArray );
        std::vector<double> forwardRates = populateDoubleVector( skForwardRatesArray );

        if( hasCurveBuildProperties )
        {
            skIndex = 0;
            const std::string& variableNamesColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[0];
            const std::string& variableValuesColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[1];

            checkValidDataSchema(jsonDoc[DATASCHEMAS][skIndex]);
            const Value& skVariableNamesArray = jsonDoc[DATASCHEMAS][skIndex][variableNamesColumnName.c_str()];
            const Value& skVariableValuesArray = jsonDoc[DATASCHEMAS][skIndex][variableValuesColumnName.c_str()];

            const std::string cbp_objectName = std::string( "CURVE_BUILD_PROPERTIES_FOR_" ) + objectName;

            const CurveBuildProperties cbp = createCurveBuildPropertiesFromJsonArray( skVariableValuesArray, &jsonDoc[DATASCHEMAS][skIndex], cbp_objectName ) ;
            return std::move( AQObjCurve(	objectName, dates, discountFactors, cbp, forwardRates ) );
        }
        else
        {
            { std::ostringstream aqMsg174;
aqMsg174 << "JSON file is missing CurveBuildProperties when attempting to load curve named " << objectName; AQ_THROW( aqMsg174.str() ); }
        };
    };

    template<>
    typename to_cached_object_type<CURVE_BUILD_PROPERTIES>::type createCacheableObject<CURVE_BUILD_PROPERTIES>( const rapidjson::Value& jsonDoc )
    {
        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();
        std::vector<DataSchema> dataSchemas = StaticStructureStore::getInstance().getCachedObjectStructure( CURVE_DEPRECATED );

        checkArrayIndexLabel(jsonDoc[DATASCHEMAS],2);

        SizeType skIndex = 0;
        const std::string& variableNamesColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[0];
        const std::string& variableValuesColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[1];

        checkValidDataSchema(jsonDoc[DATASCHEMAS][skIndex]);
        const Value& skVariableNamesArray = jsonDoc[DATASCHEMAS][skIndex][variableNamesColumnName.c_str()];
        const Value& skVariableValuesArray = jsonDoc[DATASCHEMAS][skIndex][variableValuesColumnName.c_str()];

        if( jsonDoc[DATASCHEMAS].Size() > ( skIndex + 1 ) )
        {
            // jsonDoc[DATASCHEMAS][skIndex+1]["SK_NAME"].GetString()
            return createCurveBuildPropertiesFromJsonArray( skVariableValuesArray, &jsonDoc[DATASCHEMAS][skIndex + 1], objectName );
        }
        else
        {
            return createCurveBuildPropertiesFromJsonArray( skVariableValuesArray, nullptr, objectName );
        }
        
    };

    template<>
    typename to_cached_object_type<EXAMPLE_STAND_ALONE>::type createCacheableObject<EXAMPLE_STAND_ALONE>( const rapidjson::Value& jsonDoc )
    {
        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();
        StandAlone toReturn( objectName );
        std::vector<DataSchema> dataSchemas = StaticStructureStore::getInstance().getCachedObjectStructure( EXAMPLE_STAND_ALONE );
        SizeType skIndex = 0;
        const std::string& numberColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[0];
        checkValidDataSchema(jsonDoc[DATASCHEMAS][skIndex]);
        const Value& skDoubleArray = jsonDoc[DATASCHEMAS][skIndex][numberColumnName.c_str()];
        std::vector<double> numbers = populateDoubleVector( skDoubleArray );
        toReturn.d_ = numbers[ 0 ];
        return  toReturn;
    };

    template<>
    typename to_cached_object_type<TABLE>::type createCacheableObject<TABLE>( const rapidjson::Value& jsonDoc )
    {
        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();
        std::vector<DataSchema> dataSchemas = StaticStructureStore::getInstance().getCachedObjectStructure( TABLE );

        SizeType skIndex = 0;
        const std::string& dateColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[0];
        const std::string& valuesColumnName = dataSchemas[ skIndex ].getColumnNamesRef()[1];

        checkValidDataSchema(jsonDoc[DATASCHEMAS][skIndex]);
        const Value& skDateArray = jsonDoc[DATASCHEMAS][skIndex][dateColumnName.c_str()];
        const Value& skDoubleArray = jsonDoc[DATASCHEMAS][skIndex][valuesColumnName.c_str()];

        std::vector<boost::gregorian::date> dates;
        for ( SizeType skCounter = 0; skCounter < skDateArray.Size(); skCounter++ )
        {
            const std::string dateStr = skDateArray[skCounter].GetString();
            dates.emplace_back( boost::gregorian::from_undelimited_string( dateStr.c_str() ) );
        }

        std::vector<double> values;
        for ( SizeType skCounter = 0; skCounter < skDoubleArray.Size(); skCounter++ )
        {
            const double value = skDoubleArray[skCounter].GetDouble();
            values.emplace_back( value );
        }
        return TableDateDouble( objectName, dates, values );
    };

	

    //
    // Deserialize the 'FIXING_TABLE' Object
    //
    template<>
	typename to_cached_object_type<FIXING_TABLE>::type createCacheableObject<FIXING_TABLE>( const rapidjson::Value& jsonDoc )
    {
        // Load Data as a Free Object
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();

        // This copies one column at a time ... in the static structure store we persist the schema as a column of Fixing Dates followed by a column of Fixing Values
        auto fixingDates =  Variant::createValueVector< boost::gregorian::date >( freeObject.getValuesCopy( FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE, 0, true ) );
        auto fixingValues = Variant::createValueVector< double >( freeObject.getValuesCopy(FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE, 1, true ) );

		// Check if there is a PARAMETER LVB to deserialize ...
		LabelValueBlock parameterLVB;
		const auto schemaNames = freeObject.keyNames();
		for (unsigned int i = 0; i < schemaNames.size(); i++)
		{
			// Search for the Fixing Table Parameters LVB Key
			auto schemaName = schemaNames[i];
			if ( schemaName == FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE_PARAMETERLVB )
			{
				auto keys   = freeObject.getValuesCopy( schemaName, 0 );
				auto values = freeObject.getValuesCopy( schemaName, 1 );
				parameterLVB = buildSingleLabelValueBlock( fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values) );
			}
		}
		
		if (parameterLVB.size() == 0)
		{
			// Not using LabelValueBlock; Call the FixingTable constructor specific to interest rate fixings only

			// Fixing Parameters consist of Currency and Curve Tenor.
			// Column 0 contains the Keys, here we are updating Column 1 the Values
			auto fixingParameters = freeObject.getValuesCopy( FIXING_TABLE_SERIALIZATION_KEYS::FIXING_TABLE_PARAMETERS, 1, true );

			auto currency = toCCYEnum( fixingParameters.at(0).toString() );             // Column 0: Currency
			auto curveTenor = toCurveTenorEnum( fixingParameters.at(1).toString() );    // Column 1: CurveTenor

			FixingTable fixingTable( objectName, fixingDates, fixingValues, curveTenor, currency );
			return fixingTable;
		}
		else
		{
			// Using ParameterLVB. Call the constructor which takes the LVB ( suitable for INTERESTRATE, INFLATION and FX fixings ).
			FixingTable fixingTable( objectName, fixingDates, fixingValues, parameterLVB );
			return fixingTable;
		}
    };

	//
    // Deserialize the 'CURVE_GENERATOR' Object
    //
	template<>
	typename to_cached_object_type<CURVE_GENERATOR>::type createCacheableObject<CURVE_GENERATOR>( const rapidjson::Value& jsonDoc )
    {
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
		CurveGenerator curveGenerator(objectName, freeObject);
		return curveGenerator;
    };

	//
    // Deserialize the 'CURVE_MARKETDATA' Object
    //
	template<>
	typename to_cached_object_type<CURVE_MARKETDATA>::type createCacheableObject<CURVE_MARKETDATA>( const rapidjson::Value& jsonDoc )
    {
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
		CurveMarketData curveMarketData(objectName, freeObject);
		return curveMarketData;
    };

	//
    // Deserialize the 'CURVE' Object
    //
	template<>
	typename to_cached_object_type<CURVE>::type createCacheableObject<CURVE>( const rapidjson::Value& jsonDoc )
    {
		// Deserialize the nested dependencies of the SingleCurveObject
		if ( jsonDoc.HasMember( NESTED_OBJECTS ) )
		{
			const Value& nestedJSONDocs = jsonDoc[NESTED_OBJECTS];
			if ( ! jsonDoc[DATASCHEMAS].IsArray() )
			{
				{ std::ostringstream aqMsg175;
aqMsg175 << "Error deserialising CURVE: JSON Element '" << NESTED_OBJECTS << "' is not an array "; AQ_THROW( aqMsg175.str() ); }                    
			}

			for ( SizeType counter = 0; counter < nestedJSONDocs.Size(); counter++ )
			{
				etrading::Environment& env = etrading::Environment::defaultEnv();
				std::pair<std::string, CachedObjectEnum> result = env.deSerializeJSONDoc( nestedJSONDocs[counter] );
				if ( result.second != CURVE_GENERATOR && result.second != CURVE_MARKETDATA )
				{
					{ std::ostringstream aqCoreMsg24;
aqCoreMsg24 << "Unexpected nested object when deserializing Curve Object. Expecting CURVE_GENERATOR or CURVE_MARKETDATA, but found object handle `" << result.first << "` of type `" << toString( result.second ) << "` "; AQ_THROW( aqCoreMsg24.str() ); }
				}
			}
		}

		// Now deserialize the SingleCurveObject itself
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
		SingleCurveObject curveObject(objectName, freeObject);
		curveObject.calibrateCurve();
		return curveObject;
    };

	//
	// Deserialize the 'MULTICURVE' Object
	//
	template<>
	typename to_cached_object_type<MULTICURVE>::type createCacheableObject<MULTICURVE>(const rapidjson::Value& jsonDoc)
	{
		// Deserialize the nested dependencies of the SingleCurveObject
		if (jsonDoc.HasMember(NESTED_OBJECTS))
		{
			const Value& nestedJSONDocs = jsonDoc[NESTED_OBJECTS];
			if (!jsonDoc[DATASCHEMAS].IsArray())
			{
				{ std::ostringstream aqMsg176;
aqMsg176 << "Error deserialising CURVE: JSON Element '" << NESTED_OBJECTS << "' is not an array "; AQ_THROW( aqMsg176.str() ); }
			}

			for (SizeType counter = 0; counter < nestedJSONDocs.Size(); counter++)
			{
				etrading::Environment& env = etrading::Environment::defaultEnv();
				std::pair<std::string, CachedObjectEnum> result = env.deSerializeJSONDoc(nestedJSONDocs[counter]);
				if (result.second != CURVE_GENERATOR && result.second != CURVE_MARKETDATA)
				{
					{ std::ostringstream aqCoreMsg25;
aqCoreMsg25 << "Unexpected nested object when deserializing Curve Object. Expecting CURVE_GENERATOR or CURVE_MARKETDATA, but found object handle `" << result.first << "` of type `" << toString(result.second) << "` "; AQ_THROW( aqCoreMsg25.str() ); }
				}
			}
		}

		// Now deserialize the MultiCurveObject itself
		FreeObject freeObject = CreateFreeObject(jsonDoc);
		const std::string objectName = freeObject.getRefToName();
		MultiCurveObject curveObject(objectName, freeObject);
		curveObject.calibrateCurve();
		return curveObject;
	};

	// Deserialize the 'FX_CURVE' Object
    template<>
	typename to_cached_object_type<FX_CURVE>::type createCacheableObject<FX_CURVE>( const rapidjson::Value& jsonDoc )
    {
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
        
        auto curveGenerator = createCacheableObject<CURVE_GENERATOR>(jsonDoc);
        auto curveMarketData = createCacheableObject<CURVE_MARKETDATA>(jsonDoc);

        auto curveGeneratorPtr = std::make_shared<CurveGenerator>(curveGenerator);

        auto curveMarketDataPtr = std::make_shared<CurveMarketData>(curveMarketData);
        
		FXCurve fxCurve(objectName, curveGeneratorPtr, curveMarketDataPtr);
		return fxCurve;
    };

	// Deserialize the 'OPTION' Object
	template<>
	typename to_cached_object_type<OPTION>::type createCacheableObject<OPTION>(const rapidjson::Value& jsonDoc)
	{

		std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();

		FreeObject freeObject = CreateFreeObject(jsonDoc);

		//Get swapProperties
		LabelValueBlock optionLVB;
		const auto schemaNames = freeObject.keyNames();
		for (unsigned int i = 0; i < schemaNames.size(); i++)
		{
			//Get Leg Static Data
			auto schemaName = schemaNames[i];
			std::string index = getIndexFromSchemaName(schemaName, false);
			std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, false);
			if (schemaNameWithoutIndex == toString(OPTION))
			{
				auto keys = freeObject.getValuesCopy(schemaName, 0);
				auto values = freeObject.getValuesCopy(schemaName, 1);
				optionLVB = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
				break;
			}
		}

		if (optionLVB.size() == 0 )
		{
			{ std::ostringstream aqMsg177;
aqMsg177 << "An Option jason file must have option properties LVB " << objectName; AQ_THROW( aqMsg177.str() ); }
		}

		OptionPtr option = createOption(objectName, optionLVB, false);

		return option;
	};

	// Deserialize the 'VOLATILITY' Object
	template<>
	typename to_cached_object_type<VOLATILITY>::type createCacheableObject<VOLATILITY>(const rapidjson::Value& jsonDoc)
	{

		std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();

		FreeObject freeObject = CreateFreeObject(jsonDoc);

		//Get swapProperties
		LabelValueBlock volLVB;
		const auto schemaNames = freeObject.keyNames();
		for (unsigned int i = 0; i < schemaNames.size(); i++)
		{
			//Get Leg Static Data
			auto schemaName = schemaNames[i];
			std::string index = getIndexFromSchemaName(schemaName, false);
			std::string schemaNameWithoutIndex = getSchemaNameWithoutIndex(schemaName, false);
			if (schemaNameWithoutIndex == toString(VOLATILITY))
			{
				auto keys = freeObject.getValuesCopy(schemaName, 0);
				auto values = freeObject.getValuesCopy(schemaName, 1);
				volLVB = buildSingleLabelValueBlock(fromVariantToStdStringVector(keys), fromVariantToStdStringVector(values));
				break;
			}
		}

		if (volLVB.size() == 0 )
		{
			{ std::ostringstream aqMsg178;
aqMsg178 << "An Volatility jason file must have vol properties LVB " << objectName; AQ_THROW( aqMsg178.str() ); }
		}

		Volatility volObject(volLVB, objectName);

		return std::make_shared<Volatility>(volObject);
	};

	//
    // Deserialize the 'CREDIT_MODEL' Object
    //
	template<>
	typename to_cached_object_type<CREDIT_MODEL>::type createCacheableObject<CREDIT_MODEL>( const rapidjson::Value& jsonDoc )
    {
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
		CreditModel creditModel(objectName, freeObject);
		return creditModel;
    };

	//
    // Deserialize the 'CREDIT_BASKET_MODEL' Object
    //
	template<>
	typename to_cached_object_type<CREDIT_BASKET_MODEL>::type createCacheableObject<CREDIT_BASKET_MODEL>( const rapidjson::Value& jsonDoc )
    {
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
		CreditBasketModel creditBasketModel(objectName, freeObject);
		return creditBasketModel;
    };

	//
    // Deserialize the 'BOND_CURVE' Object
    //
	template<>
	typename to_cached_object_type<BOND_CURVE>::type createCacheableObject<BOND_CURVE>( const rapidjson::Value& jsonDoc )
    {
        FreeObject freeObject = CreateFreeObject(jsonDoc);
        const std::string objectName = freeObject.getRefToName();
		BondCurve bondCurve(objectName, freeObject);
		return bondCurve;
    };

	//
	// Deserialize the 'INFLATION_CURVE' Object
	//
	template<>
	typename to_cached_object_type<INFLATION_CURVE>::type createCacheableObject<INFLATION_CURVE>( const rapidjson::Value& jsonDoc )
	{
		FreeObject freeObject = CreateFreeObject(jsonDoc);
		const std::string objectName = freeObject.getRefToName();
		InflationCurve inflationCurve( objectName, freeObject);
		return inflationCurve;
	};

	//
	// Deserialize the 'SABR_MODEL' Object
	//
	template<>
	typename to_cached_object_type<SABR_MODEL>::type createCacheableObject<SABR_MODEL>(const rapidjson::Value& jsonDoc)
	{
		FreeObject freeObject = CreateFreeObject(jsonDoc);
		const std::string objectName = freeObject.getRefToName();
		SabrModel sabrModel(objectName, freeObject);
		return sabrModel;
	};

	//
	// Deserialize the 'SABR_MARKETDATA' Object
	//
	template<>
	typename to_cached_object_type<SABR_MARKETDATA>::type createCacheableObject<SABR_MARKETDATA>(const rapidjson::Value& jsonDoc)
	{
		FreeObject freeObject = CreateFreeObject(jsonDoc);
		const std::string objectName = freeObject.getRefToName();
		SabrMarketData sabrData(objectName, freeObject);
		return sabrData;
	};

}


