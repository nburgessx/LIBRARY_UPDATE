/*! @file
    @brief DataInstance setup class for Risk
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLDataInstanceConfigurationRisk.cpp
//
//  DESCRIPTION :        DataInstance setupper for PV
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLDataInstanceConfigurationRisk.h"
#include "AQLMathFXEntity.h"
#include "AQLMarketData.h"
#include "AQLRiskConfigurationManager.h"

// constructor
/*!

*/
AQLDataInstanceConfigurationRisk::AQLDataInstanceConfigurationRisk()
: AQLDataInstanceConfigurationPV()
{
}

// destructor
/*!

*/
AQLDataInstanceConfigurationRisk::~AQLDataInstanceConfigurationRisk(void)
{
}


// 
/*!
	@brief setup

*/
void 
AQLDataInstanceConfigurationRisk::setUp(void)
{
	AQLDataInstanceConfigurationPV::setUp();
}



// 
/*!
    @brief setup entities

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationRisk::setUpEntityes(AQLDataInstance &dataInstance) const
{
	AQLDataInstanceConfigurationPV::setUpEntityes(dataInstance);

//	AQLRiskConfiguration *riskSetUpper = 0;
//
//////////////////////////////////////////////////////////////// for XLL plus
///*
//	AQLString riskName = AQLCoreDataService::getContext(ARG_KEY_OFFICIALRISK);
//	if (riskName != AQ_NO_DATA)
//	{
//		riskSetUpper = AQLRiskConfigurationManager::getInstance()->createRiskSetUpper(riskName, true);
//	}
//	else
//	{
//		riskName = AQLCoreDataService::getContext(ARG_KEY_FRONTRISK);
//		riskSetUpper = AQLRiskConfigurationManager::getInstance()->createRiskSetUpper(riskName, false);
//	}
//
//	riskSetUpper->setUpRiskEntity(dataInstance);
//
//	delete riskSetUpper;
//*/
//
//	bool isOfficial = true;
//	AQLString riskName = AQLCoreDataService::getContext(ARG_KEY_OFFICIALRISK);
//	if (riskName == AQ_NO_DATA)
//	{
//		// front risk
//		isOfficial = false;
//		riskName = AQLCoreDataService::getContext(ARG_KEY_FRONTRISK);
//	}
//
//	// risk setup(multiple)
//	std::vector<AQLString> risks = riskName.toToken(',');
//	AQLString curveType = AQLCoreDataService::getContext(ARG_KEY_CURVETYPE);
//	std::vector<AQLString> curveTypes = curveType.toToken(',');
//	AQLString baseSceNum = AQLCoreDataService::getContext(ARG_KEY_BASESCENARIONUM);
//	std::vector<AQLString> baseSceNums = baseSceNum.toToken(',');
//
//	AQLString risk = risks[0];
//	riskSetUpper = AQLRiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
//	if ( riskSetUpper == NULL )
//		throw AQLCoreInvalidData( "riskName is odd! AQLDataInstanceConfigurationRisk::setUpEntityes", __FILE__, __LINE__ );
//
//	if (curveType != AQ_NO_DATA && curveTypes[0] != "")
//	{
//		AQLStringVector ccyCurves = curveTypes[0].toToken(':');
//		for (unsigned int i = 0; i < ccyCurves.size(); ++i)
//		{
//			AQLStringVector ccy_curve = ccyCurves[i].toToken('_');
//			riskSetUpper->setCurveType(ccy_curve[0], ccy_curve[1]);
//		}
//	}
//
//	if (baseSceNum != AQ_NO_DATA && baseSceNums[0] != "")
//	{
//		AQLStringVector ccyBaseSceNums = baseSceNums[0].toToken(':');
//		for (unsigned int i = 0; i < ccyBaseSceNums.size(); ++i)
//		{
//			AQLStringVector ccy_baseSceNums = ccyBaseSceNums[i].toToken('_');
//			riskSetUpper->setBaseScenarioTarget(ccy_baseSceNums[0], ccy_baseSceNums[1]);
//		}
//	}
//
//	riskSetUpper->setUpRiskEntity(dataInstance);
//	delete riskSetUpper;
//
//	for (unsigned int i=1; i<risks.size(); i++)
//	{
//		risk = risks[i];
//		riskSetUpper = AQLRiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
//		if (curveType != AQ_NO_DATA && curveTypes.size() > i  && curveTypes[i] != "")
//		{
//			AQLStringVector ccyCurves = curveTypes[i].toToken(':');
//			for (unsigned int j = 0; j < ccyCurves.size(); ++j)
//			{
//				AQLStringVector ccy_curve = ccyCurves[j].toToken('_');
//				riskSetUpper->setCurveType(ccy_curve[0], ccy_curve[1]);
//			}
//		}
//
//		if (baseSceNum != AQ_NO_DATA && baseSceNums.size() > i && baseSceNums[i] != "")
//		{
//			AQLStringVector ccyBaseSceNums = baseSceNums[i].toToken(':');
//			for (unsigned int j = 0; j < ccyBaseSceNums.size(); ++j)
//			{
//				AQLStringVector ccy_baseSceNums = ccyBaseSceNums[j].toToken('_');
//				riskSetUpper->setBaseScenarioTarget(ccy_baseSceNums[0], ccy_baseSceNums[1]);
//			}
//		}
//
//		riskSetUpper->setUpRiskEntity(dataInstance,false);
//		delete riskSetUpper;
//	}
//////////////////////////////////////////////////////////////// for XLL plus
}


// 
/*!
    @brief setup risk object

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationRisk::setUpRiskEntityes(AQLDataInstance &dataInstance) const
{
	AQLRiskConfiguration *riskSetUpper = 0;
	bool isOfficial = true;
	AQLString riskName = AQLCoreDataService::getContext(ARG_KEY_OFFICIALRISK);
	if (riskName == AQ_NO_DATA)
	{
		// front risk
		isOfficial = false;
		riskName = AQLCoreDataService::getContext(ARG_KEY_FRONTRISK);
	}

	// risk setup(multiple)
	std::vector<AQLString> risks = riskName.toToken(',');
	AQLString curveType = AQLCoreDataService::getContext(ARG_KEY_CURVETYPE);
	std::vector<AQLString> curveTypes = curveType.toToken(',');
	AQLString baseSceNum = AQLCoreDataService::getContext(ARG_KEY_BASESCENARIONUM);
	std::vector<AQLString> baseSceNums = baseSceNum.toToken(',');

	AQLString risk = risks[0];
	riskSetUpper = AQLRiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
	if ( riskSetUpper == NULL )
		throw AQLCoreInvalidData( "riskName is odd! AQLDataInstanceConfigurationRisk::setUpEntityes", __FILE__, __LINE__ );

	if (curveType != AQ_NO_DATA && curveTypes[0] != "")
	{
		AQLStringVector ccyCurves = curveTypes[0].toToken(':');
		for (unsigned int i = 0; i < ccyCurves.size(); ++i)
		{
			AQLStringVector ccy_curve = ccyCurves[i].toToken('_');
			riskSetUpper->setCurveType(ccy_curve[0], ccy_curve[1]);
		}
	}

	if (baseSceNum != AQ_NO_DATA && baseSceNums[0] != "")
	{
		AQLStringVector ccyBaseSceNums = baseSceNums[0].toToken(':');
		for (unsigned int i = 0; i < ccyBaseSceNums.size(); ++i)
		{
			AQLStringVector ccy_baseSceNums = ccyBaseSceNums[i].toToken('_');
			riskSetUpper->setBaseScenarioTarget(ccy_baseSceNums[0], ccy_baseSceNums[1]);
		}
	}

	riskSetUpper->setUpRiskEntity(dataInstance);
	delete riskSetUpper;

	for (unsigned int i=1; i<risks.size(); i++)
	{
		risk = risks[i];
		riskSetUpper = AQLRiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
		if (curveType != AQ_NO_DATA && curveTypes.size() > i  && curveTypes[i] != "")
		{
			AQLStringVector ccyCurves = curveTypes[i].toToken(':');
			for (unsigned int j = 0; j < ccyCurves.size(); ++j)
			{
				AQLStringVector ccy_curve = ccyCurves[j].toToken('_');
				riskSetUpper->setCurveType(ccy_curve[0], ccy_curve[1]);
			}
		}

		if (baseSceNum != AQ_NO_DATA && baseSceNums.size() > i && baseSceNums[i] != "")
		{
			AQLStringVector ccyBaseSceNums = baseSceNums[i].toToken(':');
			for (unsigned int j = 0; j < ccyBaseSceNums.size(); ++j)
			{
				AQLStringVector ccy_baseSceNums = ccyBaseSceNums[j].toToken('_');
				riskSetUpper->setBaseScenarioTarget(ccy_baseSceNums[0], ccy_baseSceNums[1]);
			}
		}

		riskSetUpper->setUpRiskEntity(dataInstance,false);
		delete riskSetUpper;
	}
////////////////////////////////////////////////////////////// for XLL plus
}
