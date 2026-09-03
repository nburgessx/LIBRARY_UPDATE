/*! @file
    @brief DataInstance setup class for Risk
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADataInstanceConfigurationRisk.cpp
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


#include "LADataInstanceConfigurationRisk.h"
#include "LAMathFXEntity.h"
#include "LAMarketData.h"
#include "LARiskConfigurationManager.h"

// constructor
/*!

*/
LADataInstanceConfigurationRisk::LADataInstanceConfigurationRisk()
: LADataInstanceConfigurationPV()
{
}

// destructor
/*!

*/
LADataInstanceConfigurationRisk::~LADataInstanceConfigurationRisk(void)
{
}


// 
/*!
	@brief setup

*/
void 
LADataInstanceConfigurationRisk::setUp(void)
{
	LADataInstanceConfigurationPV::setUp();
}



// 
/*!
    @brief setup entities

	@param[out] dataInstance
*/
void
LADataInstanceConfigurationRisk::setUpEntityes(AQLDataInstance &dataInstance) const
{
	LADataInstanceConfigurationPV::setUpEntityes(dataInstance);

//	LARiskConfiguration *riskSetUpper = 0;
//
//////////////////////////////////////////////////////////////// for XLL plus
///*
//	AQLString riskName = LACoreDataService::getContext(ARG_KEY_OFFICIALRISK);
//	if (riskName != AQ_NO_DATA)
//	{
//		riskSetUpper = LARiskConfigurationManager::getInstance()->createRiskSetUpper(riskName, true);
//	}
//	else
//	{
//		riskName = LACoreDataService::getContext(ARG_KEY_FRONTRISK);
//		riskSetUpper = LARiskConfigurationManager::getInstance()->createRiskSetUpper(riskName, false);
//	}
//
//	riskSetUpper->setUpRiskEntity(dataInstance);
//
//	delete riskSetUpper;
//*/
//
//	bool isOfficial = true;
//	AQLString riskName = LACoreDataService::getContext(ARG_KEY_OFFICIALRISK);
//	if (riskName == AQ_NO_DATA)
//	{
//		// front risk
//		isOfficial = false;
//		riskName = LACoreDataService::getContext(ARG_KEY_FRONTRISK);
//	}
//
//	// risk setup(multiple)
//	std::vector<AQLString> risks = riskName.toToken(',');
//	AQLString curveType = LACoreDataService::getContext(ARG_KEY_CURVETYPE);
//	std::vector<AQLString> curveTypes = curveType.toToken(',');
//	AQLString baseSceNum = LACoreDataService::getContext(ARG_KEY_BASESCENARIONUM);
//	std::vector<AQLString> baseSceNums = baseSceNum.toToken(',');
//
//	AQLString risk = risks[0];
//	riskSetUpper = LARiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
//	if ( riskSetUpper == NULL )
//		throw AQLCoreInvalidData( "riskName is odd! LADataInstanceConfigurationRisk::setUpEntityes", __FILE__, __LINE__ );
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
//		riskSetUpper = LARiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
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
LADataInstanceConfigurationRisk::setUpRiskEntityes(AQLDataInstance &dataInstance) const
{
	LARiskConfiguration *riskSetUpper = 0;
	bool isOfficial = true;
	AQLString riskName = LACoreDataService::getContext(ARG_KEY_OFFICIALRISK);
	if (riskName == AQ_NO_DATA)
	{
		// front risk
		isOfficial = false;
		riskName = LACoreDataService::getContext(ARG_KEY_FRONTRISK);
	}

	// risk setup(multiple)
	std::vector<AQLString> risks = riskName.toToken(',');
	AQLString curveType = LACoreDataService::getContext(ARG_KEY_CURVETYPE);
	std::vector<AQLString> curveTypes = curveType.toToken(',');
	AQLString baseSceNum = LACoreDataService::getContext(ARG_KEY_BASESCENARIONUM);
	std::vector<AQLString> baseSceNums = baseSceNum.toToken(',');

	AQLString risk = risks[0];
	riskSetUpper = LARiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
	if ( riskSetUpper == NULL )
		throw AQLCoreInvalidData( "riskName is odd! LADataInstanceConfigurationRisk::setUpEntityes", __FILE__, __LINE__ );

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
		riskSetUpper = LARiskConfigurationManager::getInstance()->createRiskSetUpper(risk, isOfficial);
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
