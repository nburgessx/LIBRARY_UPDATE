#include "EntityPoolUtilities.h"

#include "ParameterValidation.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CurveCalibrationData.h"
#include "AQLDataVector.h"
#include "AQLDefinitions.h"
#include "TypeHelpers.h"
#include "ParameterValidation.h"
#include "AQLCoreDataService.h"
#include "AQLUpdateStaticDataManager.h"

namespace etrading
{
	/* @brief			Method that returns a boolean indicator, telling us whether the specified curve is registerd in the object pool or not
    *  @param [in]		curveCollection		Name of curve collection
    *  @return	A message showing the output of the action
    */
	const bool isCurveRegistered(const AQLString& curveCollection)
	{
		AQLDataInstance* dataInstance = getDataInstance();
        AQLObjectPool& en = dataInstance->getObjectPool();
        AQLString name = etrading::AQLCurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + curveCollection;
		AQLObjectHolder ehycpro = en.getObject(name);
		return ehycpro.isDefined();
    };


	/* @brief			Method that removes a specified curve from object pool
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool
    *  @return	A message showing the output of the action
    */
	AQLString removeCurveFromEntityPool(const AQLString& curveCollection, const AQLString& curveIndex)
	{
		AQLDataInstance* dataInstance = getDataInstance();
        AQLObjectPool& en = dataInstance->getObjectPool();

		// Get CurveCalibrationData object
		AQLString name = etrading::AQLCurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + curveCollection;
		AQLObjectHolder ehycpro = en.getObject(name);
		if (!ehycpro.isDefined())
		{
			AQLString msg = AQLString("#Error - Curve '") + curveIndex + AQLString("' does not exist in collection '") + curveCollection + AQLString("'");
            throw AQLCoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
		}
		CurveCalibrationData *curveCalibrationData = &dynamic_cast<CurveCalibrationData &>(ehycpro.get());
		AQLString curveName = curveIndex;

		//------------------------------------------------------------------------
		// 1. Remove properties from the property manager singleton object

		AQLUpdateStaticDataManager::setUpForIRServer();
        AQLCoreDataService::setContext( CONTEXT_KEY_ISSETCURVEID, "TRUE" );
        AQLUpdateStaticDataManager::setUpDefaultIRStaticData( *dataInstance );

		//------------------------------------------------------------------------
		// 2. Remove all curve build outputs such as DFs, Fwd rates etc

		// Important Note:
		// Market name and index names that form part of Data names for DFs or forward rates etc 
		// are case sensitive. This is true in both AQLCurvePricingObject and the object pool.

		// Remove DF, Fwd rates and day convention data from AQLCurvePricingObject. 
		AQLCurvePricingObject& yc = etrading::AQLCurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance, curveCollection);
		yc.deleteCurveDataByCurveName(curveName);

		// Remove DFs, terms, FwdRates, Fwd Term Matrix from CurveCollection AQLObject. 
		AQLObjectHolder objHolder = en.getObject(curveCollection, ENCHKTYPE_NOCHECK);
		if(objHolder.isDefined())
		{
			objHolder.remove(CALIBRATION_DATA_TERMS			+ AQLString("_") + curveName);
			objHolder.remove(IR_CALIBRATION_DATA_DFS			+ AQLString("_") + curveName);
			objHolder.remove(IR_CALIBRATION_DATA_DFS2		+ AQLString("_") + curveName);
			objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + AQLString("_") + curveName);
			objHolder.remove(IR_CALIBRATION_DATA_FORWARDRATES + AQLString("_") + curveName);
		}

		// Remove DFs, terms, FwdRates, Fwd Term Matrix for all other index names related to the curve being removed

		// getAssignedCurveMktMap() returns a map whose keys are the index names associated with the current curve.
		// These index names CAN be in lower meanting they are case sensitive.
		std::map<AQLString, AQLString> assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();

		// This block only works with STD and OIS curves
		AQLDataHolder *dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{					
			// Look through all the curves. These strings are all case sensitive.
			AQLStringVector gDFs = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
			for (unsigned int i = 0; i < gDFs.size(); ++i)
			{
				if (curveName != gDFs[i])
				{
					continue;
				}

				for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
				{
					AQLString currentMkt = it->second;
					AQLString dfCurve = gDFs[i];
					AQLString mktForCurve = curveCalibrationData->getMarketForCurve(dfCurve);
					if (currentMkt == mktForCurve)
					{
						// Find an index name that would offer the same DFs and FwdRates etc as 'curveName' does. Remove them.
						// 'suffix_curve' is case sensitive.
						AQLString suffix_curve = "";
						suffix_curve = "_" + it->first;

						objHolder.remove(CALIBRATION_DATA_TERMS			+ suffix_curve);
						objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + suffix_curve);
						objHolder.remove(IR_CALIBRATION_DATA_FORWARDRATES + suffix_curve);
						objHolder.remove(IR_CALIBRATION_DATA_DFS			+ suffix_curve);
						objHolder.remove(IR_CALIBRATION_DATA_DFS2		+ suffix_curve);

						// Remove it from the map
						curveCalibrationData->removeAssignedCurveMktMap(it->first);
					}
				}
			}
		}

		// This block is needed to work with basis curves
		for (std::map<AQLString, AQLString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == assignedCurveMktMap[curveName])
			{
				// Find an index name that would offer the same DFs and FwdRates etc as 'staticDataTable' does. Remove them.
				// 'suffix_curve' is case sensitive.
				AQLString suffix_curve = "";
				suffix_curve = "_" + it->first;

				objHolder.remove(CALIBRATION_DATA_TERMS			+ suffix_curve);
				objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + suffix_curve);
				objHolder.remove(IR_CALIBRATION_DATA_FORWARDRATES + suffix_curve);
				objHolder.remove(IR_CALIBRATION_DATA_DFS			+ suffix_curve);
				objHolder.remove(IR_CALIBRATION_DATA_DFS2		+ suffix_curve);

				// Remove it from the map
				curveCalibrationData->removeAssignedCurveMktMap(it->first);
			}
		}

		//------------------------------------------------------------------------
		// 3. Remove constituent instruments from the object pool
			
		bool curveDeleted = false;
		try
		{
			// suffix        
			AQLString suffix = "";
			if (AQLString(curveName).toUpper() != STD)
			{
				suffix = curveName;
			}
			suffix.toUpper();	

			// Remove curve manager
			name = curveCollection + curveName + MANAGER;
			en.remove(name);

			// Remove MarketData data specific to staticDataTable from the CurveCalibrationData object 
			// This step **MUST** happen before the removal of instituent instrument dataValues.
			//curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + (suffix.size() == 0 ? suffix : AQLString("_") + suffix));
			curveCalibrationData->reset();

			// Remove all curve constituent instruments by name
			AQLStringVector attributeNames = en.getNames();
			for (size_t i = 0; i < attributeNames.size(); ++i)
			{
				name = attributeNames[i];
				AQLStringVector tokens = name.toToken('_');
				
				if (tokens.size() > 0)
				{
					if (AQLString(curveName).toUpper() == STD)
					{
						if (tokens[0] == curveCollection)
						{
							if (tokens[0] == curveCollection && 
								(isNumber(tokens.back()) || tokens.back() == "O_N" || tokens.back() == "T_N") )
							{
								en.removeNoCheck(name);
								curveDeleted = true;
							}
						}
					}
					else
					{					
						if (tokens[0] == curveCollection 
							&& tokens.back() == suffix) // The token that is the same as the curve name is always in upper case
						{
							en.removeNoCheck(name);
							curveDeleted = true;
						}					
					}
				}
			}
		}
		catch(const AQLCoreError& e)	
		{						
			return AQLString("#Error - Failed to remove '") + name + ("' from Object Pool. ") + e.getMsg();			
		}
		
		if (!curveDeleted)
		{
			AQLString msg = AQLString("#Error - Curve '") + curveIndex + AQLString("' does not exist in collection '") + curveCollection + AQLString("'");	
		}

		return AQLString("#Success - Curve '") + curveIndex + AQLString("' in curve collection '") + curveCollection + AQLString("' has been removed from the Object Pool");
	}
}