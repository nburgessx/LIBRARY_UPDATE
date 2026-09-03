/*
 * @brief			Utility methods addressing issues to do with object pool
 * @Created:		09 August 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "EntityPoolUtilities.h"

#include "ParameterValidation.h"
#include "LACurveForwardRateHelpers.h"
#include "CurveCalibrationData.h"
#include "LADataVector.h"
#include "LADefinitions.h"
#include "TypeHelpers.h"
#include "ParameterValidation.h"
#include "LACoreDataService.h"
#include "LAUpdateStaticDataManager.h"

namespace etrading
{
	/* @brief			Method that returns a boolean indicator, telling us whether the specified curve is registerd in the object pool or not
    *  @param [in]		curveCollection		Name of curve collection
    *  @return	A message showing the output of the action
    */
	const bool isCurveRegistered(const LAString& curveCollection)
	{
		LADataInstance* dataInstance = getDataInstance();
        LAObjectPool& en = dataInstance->getObjectPool();
        LAString name = etrading::LACurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + curveCollection;
		LAObjectHolder ehycpro = en.getObject(name);
		return ehycpro.isDefined();
    };


	/* @brief			Method that removes a specified curve from object pool
    *  @param [in]		curveCollection		Name of curve collection
    *  @param [in]		curveIndex			Name of the curve to be removed from object pool
    *  @return	A message showing the output of the action
    */
	LAString removeCurveFromEntityPool(const LAString& curveCollection, const LAString& curveIndex)
	{
		LADataInstance* dataInstance = getDataInstance();
        LAObjectPool& en = dataInstance->getObjectPool();

		// Get CurveCalibrationData object
		LAString name = etrading::LACurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + curveCollection;
		LAObjectHolder ehycpro = en.getObject(name);
		if (!ehycpro.isDefined())
		{
			LAString msg = LAString("#Error - Curve '") + curveIndex + LAString("' does not exist in collection '") + curveCollection + LAString("'");
            throw LACoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
		}
		CurveCalibrationData *curveCalibrationData = &dynamic_cast<CurveCalibrationData &>(ehycpro.get());
		LAString curveName = curveIndex;

		//------------------------------------------------------------------------
		// 1. Remove properties from the property manager singleton object

		LAUpdateStaticDataManager::setUpForIRServer();
        LACoreDataService::setContext( CONTEXT_KEY_ISSETCURVEID, "TRUE" );
        LAUpdateStaticDataManager::setUpDefaultIRStaticData( *dataInstance );

		//------------------------------------------------------------------------
		// 2. Remove all curve build outputs such as DFs, Fwd rates etc

		// Important Note:
		// Market name and index names that form part of Data names for DFs or forward rates etc 
		// are case sensitive. This is true in both LACurvePricingObject and the object pool.

		// Remove DF, Fwd rates and day convention data from LACurvePricingObject. 
		LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID(dataInstance, curveCollection);
		yc.deleteCurveDataByCurveName(curveName);

		// Remove DFs, terms, FwdRates, Fwd Term Matrix from CurveCollection LAObject. 
		LAObjectHolder objHolder = en.getObject(curveCollection, ENCHKTYPE_NOCHECK);
		if(objHolder.isDefined())
		{
			objHolder.remove(CALIBRATION_DATA_TERMS			+ LAString("_") + curveName);
			objHolder.remove(IR_CALIBRATION_DATA_DFS			+ LAString("_") + curveName);
			objHolder.remove(IR_CALIBRATION_DATA_DFS2		+ LAString("_") + curveName);
			objHolder.remove(CALIBRATION_DATA_FWDTERMSMATRIX + LAString("_") + curveName);
			objHolder.remove(IR_CALIBRATION_DATA_FORWARDRATES + LAString("_") + curveName);
		}

		// Remove DFs, terms, FwdRates, Fwd Term Matrix for all other index names related to the curve being removed

		// getAssignedCurveMktMap() returns a map whose keys are the index names associated with the current curve.
		// These index names CAN be in lower meanting they are case sensitive.
		std::map<LAString, LAString> assignedCurveMktMap = curveCalibrationData->getAssignedCurveMktMap();

		// This block only works with STD and OIS curves
		LADataHolder *dh = &curveCalibrationData->getData(IR_CALIBRATION_DATA_GENERATEDFS, NOCHECK);
		if (dh->isDefined() && !dh->isNull())
		{					
			// Look through all the curves. These strings are all case sensitive.
			LAStringVector gDFs = dynamic_cast<const LADataStrings &>(dh->get()).get();
			for (unsigned int i = 0; i < gDFs.size(); ++i)
			{
				if (curveName != gDFs[i])
				{
					continue;
				}

				for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
				{
					LAString currentMkt = it->second;
					LAString dfCurve = gDFs[i];
					LAString mktForCurve = curveCalibrationData->getMarketForCurve(dfCurve);
					if (currentMkt == mktForCurve)
					{
						// Find an index name that would offer the same DFs and FwdRates etc as 'curveName' does. Remove them.
						// 'suffix_curve' is case sensitive.
						LAString suffix_curve = "";
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
		for (std::map<LAString, LAString>::const_iterator it = assignedCurveMktMap.begin(); it != assignedCurveMktMap.end(); it++)
		{
			if (it->second == assignedCurveMktMap[curveName])
			{
				// Find an index name that would offer the same DFs and FwdRates etc as 'staticDataTable' does. Remove them.
				// 'suffix_curve' is case sensitive.
				LAString suffix_curve = "";
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
			LAString suffix = "";
			if (LAString(curveName).toUpper() != STD)
			{
				suffix = curveName;
			}
			suffix.toUpper();	

			// Remove curve manager
			name = curveCollection + curveName + MANAGER;
			en.remove(name);

			// Remove MarketData data specific to staticDataTable from the CurveCalibrationData object 
			// This step **MUST** happen before the removal of instituent instrument dataValues.
			//curveCalibrationData->LAObject::remove(CALIBRATION_DATA_MARKETDATA + (suffix.size() == 0 ? suffix : LAString("_") + suffix));
			curveCalibrationData->reset();

			// Remove all curve constituent instruments by name
			LAStringVector attributeNames = en.getNames();
			for (size_t i = 0; i < attributeNames.size(); ++i)
			{
				name = attributeNames[i];
				LAStringVector tokens = name.toToken('_');
				
				if (tokens.size() > 0)
				{
					if (LAString(curveName).toUpper() == STD)
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
		catch(const LACoreError& e)	
		{						
			return LAString("#Error - Failed to remove '") + name + ("' from Object Pool. ") + e.getMsg();			
		}
		
		if (!curveDeleted)
		{
			LAString msg = LAString("#Error - Curve '") + curveIndex + LAString("' does not exist in collection '") + curveCollection + LAString("'");	
		}

		return LAString("#Success - Curve '") + curveIndex + LAString("' in curve collection '") + curveCollection + LAString("' has been removed from the Object Pool");
	}
}