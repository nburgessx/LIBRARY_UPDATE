// BondFactory.cpp

/*
 * @brief			Bond Factory to Create Bond AQObj objects
 * @Created:		3rd February 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "BondFactory.h"
#include "BondUtilities.h"
#include "FixedBond.h"
#include "FloatingBond.h"
#include "CommonConstants.h"
#include "LabelValueBlockValidation.h"
#include "AQObjUtilities.h"

#include <string>
#include <vector>
#include <boost/foreach.hpp>

namespace etrading
{

    /* @brief			Create a Bond based on the Bond Label Value Blocks and Schedule
    *  @param [in]		bondObjectName          Bond cached name
    *  @param [in]		bondDescriptionLVB      Bond Description Label Value Block
    *  @param [in]		bondScheduleLVB         Bond Schedule Label Value Block
    *  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
    *  @return			Bond Pointer
    */
	BondPtr createBond( const std::string& bondObjectName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB, const bool& validateKeys )
	{
        // Will throw if the bond type is invalid
		BondTypeEnum bondType = etrading::toBondTypeEnum( bondLVB.getCompulsoryValueAsAQLString( BOND_KEY::BOND_TYPE, "bondLVB" ).getCString() );
	
        // Validate the Bond Description and Schedule Label Value Blocks
        // ---------------------------------------------------------
        etrading::validateBondDescriptionLVBKeys( bondLVB, validateKeys, "bondLVB" );
        
		BondPtr ret; 
		switch( bondType )
		{
            case FIXED_BOND:
		        etrading::validateKeysForLVB( etrading::FixedSchedule::lvbKeys(), scheduleLVB.getKeys(), validateKeys );
                ret = BondPtr( new FixedBond( bondObjectName, bondLVB, scheduleLVB ) );
	            break;

            case FLOATER_BOND:
                etrading::validateKeysForLVB( etrading::FloatSchedule::lvbKeys(), scheduleLVB.getKeys(), validateKeys );
                ret = BondPtr( new FloatingBond( bondObjectName, bondLVB, scheduleLVB ) );
                break;

            case PERPETUAL_BOND:
                throw AQLCoreInvalidData("#Error: 'PERPETUAL' Bonds are currently not supported.",__FILE__,__LINE__);
                break;

            case CALLABLE_BOND:
                throw AQLCoreInvalidData("#Error: 'CALLABLE' Bonds are currently not supported.",__FILE__,__LINE__);
                break;

            case INFLATION_LINKED_BOND:
                throw AQLCoreInvalidData("#Error: 'INFLATION-LINKED' Bonds are currently not supported.",__FILE__,__LINE__);
                break;

            case CONVERTIBLE_BOND:
                throw AQLCoreInvalidData("#Error: 'CONVERTIBLE' Bonds are currently not supported.",__FILE__,__LINE__);
                break;

            case STRUCTURED_BOND:
                throw AQLCoreInvalidData("#Error: 'STRUCTURED' Bonds are currently not supported.",__FILE__,__LINE__);
                break;

			default:
		        throw AQLCoreInvalidData("#Error: Invalid Bond Type.",__FILE__,__LINE__);
                break;
        }

		return ret;
	}

	/* @brief			Create a Bond based on the Bond Label Value Blocks and Schedule
	*  @param [in]		bondObjectName          Bond cached name
	*  @param [in]		bondDescriptionLVB      Bond Description Label Value Block
	*  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
	*  @return			Bond Pointer
	*/
	BondPtr createBondFromSingleLVB(const std::string& bondObjectName, const LabelValueBlock& bondLVB, const bool& validateKeys)
	{
		// Will throw if the bond type is invalid
		BondTypeEnum bondType = etrading::toBondTypeEnum(bondLVB.getCompulsoryValueAsAQLString(BOND_KEY::BOND_TYPE, "bondLVB").getCString());

		BondPtr ret;
		switch (bondType)
		{
		case FIXED_BOND:
			validateKeysForLVB(fixedBondLVBKeys(), bondLVB.getKeys(), validateKeys);
			ret = BondPtr(new FixedBond(bondObjectName, bondLVB ));
			break;
		case FLOATER_BOND:
			validateKeysForLVB( floatBondLVBKeys(), bondLVB.getKeys(), validateKeys );  
            ret = BondPtr( new FloatingBond( bondObjectName, bondLVB ) );
			break;
		case PERPETUAL_BOND:
			throw AQLCoreInvalidData("#Error: 'PERPETUAL' Bonds are currently not supported.", __FILE__, __LINE__);
			break;

		case CALLABLE_BOND:
			throw AQLCoreInvalidData("#Error: 'CALLABLE' Bonds are currently not supported.", __FILE__, __LINE__);
			break;

		case INFLATION_LINKED_BOND:
			throw AQLCoreInvalidData("#Error: 'INFLATION-LINKED' Bonds are currently not supported.", __FILE__, __LINE__);
			break;

		case CONVERTIBLE_BOND:
			throw AQLCoreInvalidData("#Error: 'CONVERTIBLE' Bonds are currently not supported.", __FILE__, __LINE__);
			break;

		case STRUCTURED_BOND:
			throw AQLCoreInvalidData("#Error: 'STRUCTURED' Bonds are currently not supported.", __FILE__, __LINE__);
			break;

		default:
			throw AQLCoreInvalidData("#Error: Invalid Bond Type.", __FILE__, __LINE__);
			break;
		}

		return ret;

	}

   /* @brief			Create a Bond based on a BondGenerator
    *  @param [in]		bondObjectName          Bond cached name to use
	*  @param [in]		bondGeneratorName       Bond Generator name to use
    *  @param [in]		bondExpressionLVB       A Label Value Block containing dataValues specific to this bond
    *  @param [in]		validateKeys            Validate the bond and schedule LVB keys, defaults to true
    *  @return			Bond Pointer
    */
	BondPtr createBondFromGenerator( const std::string& bondObjectName, const std::string& bondGeneratorName, const LabelValueBlock& bondExpressionLVB, const bool& validateKeys )
	{
		auto bondGen = getBondGenerator( bondGeneratorName );

		/* Iterate through keys / values in the bondExpressionLVB and copy to the bondLVB and scheduleLVB.
		 * Many of the scheduleLVB dataValues are optional, so we simply iterate through the keys / values we have been given
		 * and defer to the createBond() method to perform further validation
		 */
		const std::string lvbName = "bondExpressionLVB";
		const std::set<std::string> expressionKeys = bondExpressionLVB.getKeysAsSet();
		
        StandardStringVector bondKeys;
        StandardStringVector bondValues;

        StandardStringVector scheduleKeys;
        StandardStringVector scheduleValues;


		// Reserve Vector sizes
		bondKeys.reserve( expressionKeys.size() );
		bondValues.reserve( expressionKeys.size() );
		scheduleKeys.reserve( expressionKeys.size() );
		scheduleValues.reserve( expressionKeys.size() );

        BOOST_FOREACH( auto key, expressionKeys )
		{
			// Handle specific bondLVB keys
			if ( key == BONDGENERATOR_EXPRESSION_KEY::ISIN || key == BONDGENERATOR_EXPRESSION_KEY::BOND_DESCRIPTION || key == BONDGENERATOR_EXPRESSION_KEY::QUOTED_MARGIN )
			{
				auto value = bondExpressionLVB.getCompulsoryValue( key, lvbName );
                bondKeys.push_back( key );
                bondValues.push_back( value );
			}
			else
			{
				// Everything else belongs in the Bond scheduleLVB
				auto value = bondExpressionLVB.getCompulsoryValue( key, lvbName );
				scheduleKeys.push_back( key );
                scheduleValues.push_back( value );
			}
		}
        
        LabelValueBlock bondLVB( bondGen->toLabelValueBlock( BONDGENERATOR_COMPONENTS::KEY_BOND_STATICDATA ), bondKeys, bondValues );
		LabelValueBlock scheduleLVB( bondGen->toLabelValueBlock( BONDGENERATOR_COMPONENTS::KEY_BOND_SCHEDULE ), scheduleKeys, scheduleValues );

		BondPtr ret = createBond( bondObjectName, bondLVB, scheduleLVB, true );
		return ret;
	}
  
}