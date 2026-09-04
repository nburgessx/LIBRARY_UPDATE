#define _HAS_STD_BYTE 0

/*! @file
    @brief DataInstance setup class for calc PV
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLDataInstanceConfigurationPV.cpp
//
//  DESCRIPTION :       DataInstance setup class for calc PV
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


#include "AQLDataInstanceConfigurationPV.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreReferencePool.h"
#include "AQLRatesSDEBase.h"
#include "AQLDataCSVFileLoader.h"
#include "AQLLinearRatesOptionValue.h"
#include "AQLCoreDataService.h"
#include "AQLDealUtils.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLModelSetupBase.h"
#include "AQLModelManager.h"
#include "AQLDataInstanceConfiguration.h"
#include "AQLModelConfiguration.h"
#include "AQLCalibrateModel.h"
#include "AQLCalibrateModelIR.h"
#include "AQLObjectConfigurationManager.h"
#include "AQLObjectConfiguration.h"
#include "AQLPriceDataFunction.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLPriceLSMCTradeValue.h"
#include "AQLPricePortfolioValue.h"
#include "AQLDataReference.h"
#include "AQLDataMatrix.h"
#include "AQLFunctionUtilities.h"
#include "AQLDefinitionsIRSABR.h"
#include "AQLMathValuableEntity.h"


using namespace std;

#define MARKETPARAM	"marketparam1"
#define PATH1	"path1"
#define FXSDE	"fx_sde"
#define FORWARDFX "FORWARDFX"



// constructor
/*!

*/
AQLDataInstanceConfigurationPV::AQLDataInstanceConfigurationPV()
: AQLDataInstanceConfiguration()
{

}

// destructor
/*!

*/
AQLDataInstanceConfigurationPV::~AQLDataInstanceConfigurationPV(void)
{
}

// 
/*!
	@brief setup

*/
void 
AQLDataInstanceConfigurationPV::setUp(void)
{
	mSDECurrencys = AQLDealUtils::getSDECurrencys(true);
	int ccySize = mSDECurrencys.size();
	AQLString calc = AQLCoreDataService::getContext(ARG_KEY_CALC).toUpper();
	mPathEntityName = calc == "VANILLA" ? MARKETPARAM : PATH1;

	// set model
	mSDEModels.resize(ccySize);
	for (int i = 0; i < ccySize; ++i)
	{
		mSDEModels[i] = AQLMarketData::getModelName(mSDECurrencys[i]).toUpper();
		if (mSDEModels[i] == MODEL_IRSABR) mPathEntityName = MARKETPARAM;
	}

	//set maxterm
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString maxFileName = staticData.getStaticData(KEY_DEAL_MAXTERM_FILE);
	if (maxFileName != AQ_NO_DATA)
	{
		maxFileName = AQLMarketData::getNumFileName(maxFileName, AQLID);
		AQLFileAccessor file(maxFileName);
		AQLStringMatrix dataMtx;
		file.readAllData(MARKET_DATA_DELIMITER, dataMtx);
		file.close();
		const AQLString asofStr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		if (asofStr == AQ_NO_DATA)
		{
			throw AQLCoreInvalidData("asofdate does not given in arguments", __FILE__, __LINE__);
		}
		AQLDate asofDate(asofStr.getCString());
		AQLDate maxDate = asofDate;
		AQLDate endDate;
		for (unsigned int i = 0; i < dataMtx.size(); ++i)
		{
			if (dataMtx[i].size() != 2)
			{
				throw AQLCoreInvalidData("maxterm file format is wrong", __FILE__ , __LINE__);
			}
			if(!(dataMtx[i][0].toUpper()==""))
			{
				endDate.setDate(dataMtx[i][0].getCString());
			}
			else
			{
				endDate=asofDate;
			}
			const AQLStringVector addYearsVec = dataMtx[i][1].toToken(':');
			for (unsigned int j = 0; j < addYearsVec.size(); ++j)
			{
				const AQLDate date = AQLMathDateCalculations::getDate(endDate, addYearsVec[j], true);
				if (date > maxDate)
				{
					maxDate = date;
				}
			}
		}
		int maxTerm = asofDate.intervalYears(maxDate);
		if (maxTerm < 1)
		{
			maxTerm = 1;
		}
		else
		{
			int addYears = 1;
			maxTerm += addYears;
		}
		AQLCoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, AQLString(maxTerm));
		AQLCoreDataService::setContext(CONTEXT_KEY_MAXTERM, AQLString(maxTerm));
	}
	

}

// 
/*!
    @brief setup masters

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::setUpMasters(AQLDataInstance &dataInstance) const
{
	// regist master data for base ccy
	AQLModelSetupBase *reg = AQLMasterRegistManager::getInstance()->createRegister(mSDEModels[0]);
	if (!reg)
	{
		AQLString msg;
		if (mSDEModels[0] == AQ_NO_DATA)
		{
			msg = "IR SED model is not registered !!";
		}
		else
		{
            msg = mSDEModels[0] + " is not supported !!";
		}
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	reg->registMaster(dataInstance);
	delete reg;
}

// 
/*!
    @brief setup sde

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::setUpSDE(AQLDataInstance &dataInstance) const
{
	map<AQLString, AQLCalibrateModel *> gMap;
	unsigned int ccySize = mSDECurrencys.size();
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		if (gMap.find(mSDEModels[i]) == gMap.end())
		{
			AQLCalibrateModel *generator = AQLModelConfiguration::getInstance()->createSDEGenerator(mSDEModels[i]);
			if (!generator) throw AQLCoreInvalidData("SDE Model is not set",__FILE__, __LINE__);
			gMap.insert(make_pair(mSDEModels[i], generator));
		}
	}
	// generate sde
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		gMap[mSDEModels[i]]->generateSDE(mSDECurrencys[i], dataInstance);
	}
	// generate fwdfx constant curve
 	for (unsigned int i = 0; i < ccySize; ++i)
	{
		gMap[mSDEModels[i]]->loadFwdFXConstCurveDataAndCalibrate(mSDECurrencys[i], dataInstance);
	}
	// generate last sde
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		gMap[mSDEModels[i]]->generateSDE(mSDECurrencys[i], dataInstance, true, false);
	}
	map<AQLString, AQLCalibrateModel *>::iterator it = gMap.begin();
	while (it != gMap.end())
	{
		delete it->second;
		++it;
	}
	gMap.clear();
}


// 
/*!
    @brief create path object

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::createPathEntity(AQLDataInstance &dataInstance) const
{
	// get EntityPool
	AQLObjectPool& objPool = dataInstance.getObjectPool();

	if (mPathEntityName == MARKETPARAM)
	{
		//new plainvanilla object
		AQLMathPlainVanillaEntity* pPlain = NULL;
		AQLObjectHolder ehpath = objPool.getObject(MARKETPARAM, ENCHKTYPE_NOCHECK);
		if (!ehpath.isDefined())
		{
			pPlain = new AQLMathPlainVanillaEntity(&dataInstance);
			objPool.set(MARKETPARAM,pPlain);
		
		}
		else
		{
			dynamic_cast<AQLMathPlainVanillaEntity &>(objPool.getObject(MARKETPARAM).get()).reset();
			pPlain = &dynamic_cast<AQLMathPlainVanillaEntity &>(objPool.getObject(PATH1).get());
		}
		pPlain->getName().convertFromString(MARKETPARAM);
		pPlain->getDayCount().setDayCount(ACT_365_ISDA);
		AQLString asofstr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		pPlain->getAsOfDate().convertFromString(asofstr);
		AQLCoreDataService::setContext(CONTEXT_KEY_PLAINVANILLAENTITY_NAME, MARKETPARAM);
		AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	}
	else
	{
		//new path object
		AQLMathPathEntity* pPath = NULL;
		AQLObjectHolder ehpath = objPool.getObject(PATH1, ENCHKTYPE_NOCHECK);
		if (!ehpath.isDefined())
		{
			pPath = new AQLMathPathEntity(&dataInstance);
			objPool.set(PATH1,pPath);
		
		}
		else
		{
			dynamic_cast<AQLMathPathEntity &>(objPool.getObject(PATH1).get()).reset();
			pPath = &dynamic_cast<AQLMathPathEntity &>(objPool.getObject(PATH1).get());
		}
		pPath->getName().convertFromString(PATH1);
		pPath->getDayCount().setDayCount(ACT_365_ISDA);
		AQLString asofstr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		pPath->getAsOfDate().convertFromString(asofstr);
		AQLCoreDataService::setContext(CONTEXT_KEY_PATHENTITY_NAME, PATH1);
		
		////AQLDate asofDate = AQLMarketData::getAsofDate(objPool);
		AQLString dayCount = AQLMarketData::getTimeGridDayCount(objPool);
		AQLCoreDataService::setContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT,dayCount);
		AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	}
}


// 
/*!
    @brief create fx object

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::createFXEntity(AQLDataInstance &dataInstance) const
{
	// get EntityPool
	AQLObjectPool& objPool = dataInstance.getObjectPool();
	AQLString calc = AQLCoreDataService::getContext(ARG_KEY_CALC).toUpper();
	
	if (mSDECurrencys.size() > 1)
	{
		//new fxentity
		AQLMathFXEntity* pFwd = NULL;
		AQLObjectHolder ehfx = objPool.getObject(FORWARDFX, ENCHKTYPE_NOCHECK);
		if (!ehfx.isDefined())
		{
			pFwd = new AQLMathFXEntity(&dataInstance);
			objPool.set(FORWARDFX,pFwd);
		}
		else
		{
			dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FORWARDFX).get()).reset();
			pFwd = &dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FORWARDFX).get());
		}
		pFwd->getName().convertFromString(FORWARDFX);
		pFwd->getFXType().convertFromString("FORWARDRATE");
		AQLMarketData::setUpMarket2FXEntity(*pFwd);

		//exo case add the usemodel fx object
		AQLMathFXEntity* pFxsde = NULL;
		if (mPathEntityName == PATH1)
		{
			
			AQLObjectHolder ehfx = objPool.getObject(FXSDE, ENCHKTYPE_NOCHECK);
			if (!ehfx.isDefined())
			{
				pFxsde = new AQLMathFXEntity(&dataInstance);
				objPool.set(FXSDE,pFxsde);
			}
			else
			{
				dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FXSDE).get()).reset();
				pFxsde = &dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FXSDE).get());
			}

			pFxsde->getName().convertFromString(FXSDE);
			pFxsde->getFXType().convertFromString("USEMODEL");
			AQLMarketData::setUpMarket2FXEntity(*pFxsde);
			pFxsde->getPathEntity().convertFromString(PATH1);
		
		}

		//AQLMathFXEntity* pFX = AQLMarketData::getFXEntity(objPool, "FORWARDRATE");
		if (!pFwd)
			throw AQLCoreInvalidData("ForwardRate FX Object does not exist",__FILE__,__LINE__);
		AQLString fxName = pFwd->getName();
		AQLCoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, fxName);


		//pFX = AQLMarketData::getFXEntity(objPool, "USEMODEL");
		if (pFxsde)
		{
			fxName = pFxsde->getName();
			AQLCoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FXSDE, fxName);
		}

        if(calc=="VANILLA"){
            AQLMathPlainVanillaEntity* vanilla = &dynamic_cast<AQLMathPlainVanillaEntity&>(objPool.getObject(MARKETPARAM, ENCHKTYPE_ISDEFINED).get());
            vanilla->getFXEntity().convertFromString(FORWARDFX);
        }
	}
}


// 
/*!
    @brief load data

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::loadEntities(AQLDataInstance &dataInstance) const
{
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString fileName = staticData.getStaticData(KEY_DEAL_FILE);

// update for XLL Plus //////////////////////////////////// 
/*
	// csv loader
	AQLCSVFileLoader fileLoader;
	fileLoader.setDataInstance(&dataInstance);

	AQLObject* pEntity = new AQLObject;
	AQLStringVector strVec(1, AQLMarketData::getNumFileName(fileName));
	pEntity->add(CALIBRATION_DATA_MD_FILEPATHS, new AQLDataStrings(strVec));
	Records_var ret = fileLoader.get(AQLObjectHolder(pEntity, true));

	// get object pool
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	std::vector<AQLObjectHolder>::iterator it = ret->begin();
	while (it != ret->end())
	{
		objPool.set(it->getName(), &(it->get()));
		++it;
	}
*/
    // get AttributeMaster
    AQLPriceDataManager& dm = dataInstance.getDataMaster();
    // get EntityMaster
    AQLObjectMaster& em = dataInstance.getObjectMaster();
	// get EntityPool
	AQLObjectPool& objPool = dataInstance.getObjectPool();

	AQLString calc = AQLCoreDataService::getContext(ARG_KEY_CALC).toUpper();
	//if (calc == "VANILLA")
	//{

	//	//new plainvanilla object
	//	AQLMathPlainVanillaEntity* pPlain = NULL;
	//	AQLObjectHolder ehpath = objPool.getObject(MARKETPARAM, ENCHKTYPE_NOCHECK);
	//	if (!ehpath.isDefined())
	//	{
	//		pPlain = new AQLMathPlainVanillaEntity(&dataInstance);
	//		objPool.set(MARKETPARAM,pPlain);
	//	
	//	}
	//	else
	//	{
	//		dynamic_cast<AQLMathPlainVanillaEntity &>(objPool.getObject(MARKETPARAM).get()).reset();
	//		pPlain = &dynamic_cast<AQLMathPlainVanillaEntity &>(objPool.getObject(PATH1).get());
	//	}
	//	pPlain->getName().convertFromString(MARKETPARAM);
	//	pPlain->getDayCount().setDayCount(ACT_365_ISDA);
	//	AQLString asofstr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//	pPlain->getAsOfDate().convertFromString(asofstr);
	//	AQLCoreDataService::setContext(CONTEXT_KEY_PLAINVANILLAENTITY_NAME, MARKETPARAM);
	//	AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	//}
	//else
	//{
	//	//new path object
	//	AQLMathPathEntity* pPath = NULL;
	//	AQLObjectHolder ehpath = objPool.getObject(PATH1, ENCHKTYPE_NOCHECK);
	//	if (!ehpath.isDefined())
	//	{
	//		pPath = new AQLMathPathEntity(&dataInstance);
	//		objPool.set(PATH1,pPath);
	//	
	//	}
	//	else
	//	{
	//		dynamic_cast<AQLMathPathEntity &>(objPool.getObject(PATH1).get()).reset();
	//		pPath = &dynamic_cast<AQLMathPathEntity &>(objPool.getObject(PATH1).get());
	//	}
	//	pPath->getName().convertFromString(PATH1);
	//	pPath->getDayCount().setDayCount(ACT_365_ISDA);
	//	AQLString asofstr = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//	pPath->getAsOfDate().convertFromString(asofstr);
	//	AQLCoreDataService::setContext(CONTEXT_KEY_PATHENTITY_NAME, PATH1);
	//	
	//	////AQLDate asofDate = AQLMarketData::getAsofDate(objPool);
	//	AQLString dayCount = AQLMarketData::getTimeGridDayCount(objPool);
	//	AQLCoreDataService::setContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT,dayCount);
	//	AQLCoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	//}
	//
	//if (mSDECurrencys.size() > 1)
	//{
	//	//new fxentity
	//	AQLMathFXEntity* pFwd = NULL;
	//	AQLObjectHolder ehfx = objPool.getObject(FORWARDFX, ENCHKTYPE_NOCHECK);
	//	if (!ehfx.isDefined())
	//	{
	//		pFwd = new AQLMathFXEntity(&dataInstance);
	//		objPool.set(FORWARDFX,pFwd);
	//	}
	//	else
	//	{
	//		dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FORWARDFX).get()).reset();
	//		pFwd = &dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FORWARDFX).get());
	//	}
	//	pFwd->getName().convertFromString(FORWARDFX);
	//	pFwd->getFXType().convertFromString("FORWARDRATE");
	//	AQLMarketData::setUpMarket2FXEntity(*pFwd);

	//	//exo case add the usemodel fx object
	//	AQLMathFXEntity* pFxsde = NULL;
	//	if (calc != "VANILLA")
	//	{
	//		
	//		AQLObjectHolder ehfx = objPool.getObject(FXSDE, ENCHKTYPE_NOCHECK);
	//		if (!ehfx.isDefined())
	//		{
	//			pFxsde = new AQLMathFXEntity(&dataInstance);
	//			objPool.set(FXSDE,pFxsde);
	//		}
	//		else
	//		{
	//			dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FXSDE).get()).reset();
	//			pFxsde = &dynamic_cast<AQLMathFXEntity &>(objPool.getObject(FXSDE).get());
	//		}

	//		pFxsde->getName().convertFromString(FXSDE);
	//		pFxsde->getFXType().convertFromString("USEMODEL");
	//		AQLMarketData::setUpMarket2FXEntity(*pFxsde);
	//		pFxsde->getPathEntity().convertFromString(PATH1);
	//	
	//	}

	//	//AQLMathFXEntity* pFX = AQLMarketData::getFXEntity(objPool, "FORWARDRATE");
	//	if (!pFwd)
	//		throw AQLCoreInvalidData("ForwardRate FX Object does not exist",__FILE__,__LINE__);
	//	AQLString fxName = pFwd->getName();
	//	AQLCoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, fxName);


	//	//pFX = AQLMarketData::getFXEntity(objPool, "USEMODEL");
	//	if (pFxsde)
	//	{
	//		fxName = pFxsde->getName();
	//		AQLCoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FXSDE, fxName);
	//	}

 //       if(calc=="VANILLA"){
 //           AQLMathPlainVanillaEntity* vanilla = &dynamic_cast<AQLMathPlainVanillaEntity&>(objPool.getObject(MARKETPARAM, ENCHKTYPE_ISDEFINED).get());
 //           vanilla->getFXEntity().convertFromString(FORWARDFX);
 //       }
	//}

	AQLStringMatrix tradeMatrix;
	AQLFileAccessor tradeFile(AQLMarketData::getNumFileName(fileName, AQLID));
	tradeFile.readAllData(',',tradeMatrix);
    insertContext(tradeMatrix, dataInstance);
	unsigned int row = 0, col;
	unsigned int size = tradeMatrix.size();	
	AQLString valueFuncName = staticData.getStaticData(KEY_DEAL_VALUEFUNCTION);
	if (size>0)
	{
		while(true)
		{
			const AQLStringVector &header = tradeMatrix[row];

			// check header
			if (header.empty() || header[0] != "object_t")
			{
				AQLString msg("Invalid Format : First item is not object_t [");
				msg += header[0] + " ]";
				throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
			}

			const AQLStringVector *befVal = &tradeMatrix[row];
			row++;
			if (tradeMatrix[row].empty())
			{
				break;
			}
			int m_type = tradeMatrix[row][0].getIntValue();
            const AQLObjectHolder& objHolder = em.getObject(m_type);
            if (! objHolder.isDefined())
            {
                AQLString msg("Invalid Format : Not exist object_t (");
                msg += m_type + ") [ line-";
                msg += AQLString((int)row) + "]";
                throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
            }

			AQLObject* e = objHolder.get().clone();	
			unsigned int headerSize = header.size();
			while(row<size)
			{
				const AQLStringVector &body = tradeMatrix[row];

				// is exists object type?
				if (body[0]=="") {row++;continue;}

				// check object type
				int type = body[0].getIntValue();
				if (type==0)
				{
					// not numeric -> header rec
					delete e;
					e = NULL;
					break;
				}
				if (m_type != type)
				{
				    AQLString msg("Invalid Format : object_t is different from upper line (");
					msg += body[0] + ") [ line-";
					msg += AQLString((int)row) + "]";
					throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
				}

                // copy Object
                AQLString name("");
                for (col = 1; col < headerSize; ++col)
				{
					if (col>=body.size()) continue;
					if (body[col]=="") 
					{
						if (befVal->size() > col && (*befVal)[col] != "")
						{
							e->remove(header[col]);
						}
						continue;
					}

					if (befVal->size() <= col || (*befVal)[col] != body[col])
					{

						// get data
						AQLDataHolder* dh = &(e->getData(header[col]));
						if (!dh->isDefined() || dh->getType() == DATA_FUNCTION)
						{
							e->remove(header[col]);
							const AQLDataHolder& att = dm.getData(header[col]);
							dh = &(e->add(header[col], att));
						}

						if (header[col] == CALIBRATION_DATA_VALUE && body[col] != FN_IR_PORTFOLIOVALUE_STR && valueFuncName != AQ_NO_DATA)
						{
							dh->convertFromString(valueFuncName);
						}
						else
						{
							dh->convertFromString(body[col]);
						}

						if (header[col] == CALIBRATION_DATA_NAME)
						{
							name = dynamic_cast<AQLDataString&>(dh->get()).get();
						}
					}
				}
				objPool.set(name, e);
				e = e->clone();
				befVal = &body;
				row++;
			}
			if (e) 
			{
				delete e;
			}

			if (row>=tradeMatrix.size()) break;
		}
	}
    
    dataInstance.getReferencePool().completeDependency();
    setupFundingChangeInfo(dataInstance);
    dataInstance.getReferencePool().completeDependency();
	
	AQLDate asofDate = AQLMarketData::getAsofDate(objPool);
	//after data in set deal info (maxterm)
	if (calc == "VANILLA")
	{
		const int maxTerm = AQLDealUtils::getMaxTerm(objPool, asofDate, AQLString("VANILLA"));
		AQLCoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, AQLString(maxTerm));
		AQLCoreDataService::setContext(CONTEXT_KEY_MAXTERM, AQLString(maxTerm));
	}
	else
	{
		// set maxterm
		AQLString request = AQLCoreDataService::getContext(ARG_KEY_REQUEST);
		const int maxTerm = AQLDealUtils::getMaxTerm(objPool, asofDate);
		AQLCoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, AQLString(maxTerm));
		if (request == "EOD")
		{
			AQLCoreDataService::setContext(CONTEXT_KEY_MAXTERM, staticData.getStaticData(KEY_SIMULATION_TERM_MAX));

		}
		else
		{
			const int buffer = staticData.getStaticData(KEY_SIMULATION_TERM_BUFFER).getIntValue();
			AQLCoreDataService::setContext(CONTEXT_KEY_MAXTERM, AQLString(maxTerm + buffer));
		}
	}


	// set calibration target flag of ir volatility matrix
	// get object pool
	AQLStaticData &calibProp = AQLCoreDataService::getStaticDataManager().getCalibStaticData();
	AQLStringVector ccys = AQLDealUtils::getAllSingleCurrencys();
	lowervec(ccys);
	for (size_t i = 0; i < ccys.size(); ++i)
	{
		AQLString model = AQLMarketData::getModelName(ccys[i]);
		model.toUpper();
		if (model == MODEL_IRSABR)
		{
			//get underlying
			AQLString underlying = calibProp.getStaticData(ccys[i] + STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING);
			if (underlying == AQ_NO_DATA) continue;
			underlying.toLower();
			AQLStringVector underlyings = underlying.toToken(':');
			//get a flag to calibrate all grids
			bool isAllGridsCalibrate = false;
			AQLString strIsAllCalib= calibProp.getStaticData(ccys[i] + STATIC_DATA_KEY_CALIB_IRSABR_ISALLGRIDSCALIBRATE);
			if (strIsAllCalib != AQ_NO_DATA)
			{
				AQLDataBool tmpAttrBool;
				tmpAttrBool.convertFromString(strIsAllCalib);
				isAllGridsCalibrate = tmpAttrBool.get();
			}

			for (size_t j = 0; j < underlyings.size(); ++j)
			{
				AQLString key = ccys[i] + "." CONTEXT_KEY_DEAL_IRVOL + "." + underlyings[j];
				const AQLString calibTarget = AQLCoreDataService::getContext(key);
				if (calibTarget != AQ_NO_DATA && !isAllGridsCalibrate)
				{
					continue;
				}
				if (isAllGridsCalibrate)
				{
					AQLCoreDataService::setContext(key, AQ_NO_DATA);
				}
				else
				{
					BoolMatrix calibTarget = AQLDealUtils::getCalibTargetIRVolGrids(objPool, asofDate, ccys[i], underlyings[j]);
					AQLDataBoolMatrix tmp(calibTarget);
					AQLCoreDataService::setContext(key, tmp.convertToString());
				}
			}
		}
	}
}


// 
/*!
    @brief load data

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::loadCSV(AQLDataInstance &dataInstance) const
{
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString fileName = staticData.getStaticData(KEY_DEAL_FILE);

    // get AttributeMaster
    AQLPriceDataManager& dm = dataInstance.getDataMaster();
    // get EntityMaster
    AQLObjectMaster& em = dataInstance.getObjectMaster();
	// get EntityPool
	AQLObjectPool& objPool = dataInstance.getObjectPool();

	AQLString calc = AQLCoreDataService::getContext(ARG_KEY_CALC).toUpper();

	AQLStringMatrix tradeMatrix;
	AQLFileAccessor tradeFile(AQLMarketData::getNumFileName(fileName, AQLID));
	tradeFile.readAllData(',',tradeMatrix);
    insertContext(tradeMatrix, dataInstance);
	unsigned int row = 0, col;
	unsigned int size = tradeMatrix.size();	
	if (size>0)
	{
		while(true)
		{
			const AQLStringVector &header = tradeMatrix[row];

			// check header
			if (header.empty() || header[0] != "object_t")
			{
				AQLString msg("Invalid Format : First item is not object_t [");
				msg += header[0] + " ]";
				throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
			}

			const AQLStringVector *befVal = &tradeMatrix[row];
			row++;
			if (tradeMatrix[row].empty())
			{
				break;
			}
			int m_type = tradeMatrix[row][0].getIntValue();
            const AQLObjectHolder& objHolder = em.getObject(m_type);
            if (! objHolder.isDefined())
            {
                AQLString msg("Invalid Format : Not exist object_t (");
                msg += m_type + ") [ line-";
                msg += AQLString((int)row) + "]";
                throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
            }

			AQLObject* e = objHolder.get().clone();	
			unsigned int headerSize = header.size();
			while(row<size)
			{
				const AQLStringVector &body = tradeMatrix[row];

				// is exists object type?
				if (body[0]=="") {row++;continue;}

				// check object type
				int type = body[0].getIntValue();
				if (type==0)
				{
					// not numeric -> header rec
					delete e;
					e = NULL;
					break;
				}
				if (m_type != type)
				{
				    AQLString msg("Invalid Format : object_t is different from upper line (");
					msg += body[0] + ") [ line-";
					msg += AQLString((int)row) + "]";
					throw AQLCoreInvalidData(msg.getCString(),__FILE__, __LINE__);
				}

                // copy Object
                AQLString name("");
                for (col = 1; col < headerSize; ++col)
				{
					if (col>=body.size()) continue;
					if (body[col]=="") 
					{
						if (befVal->size() > col && (*befVal)[col] != "")
						{
							e->remove(header[col]);
						}
						continue;
					}

					if (befVal->size() <= col || (*befVal)[col] != body[col])
					{

						// get data
						AQLDataHolder* dh = &(e->getData(header[col]));
						if (!dh->isDefined() || dh->getType() == DATA_FUNCTION)
						{
							e->remove(header[col]);
							const AQLDataHolder& att = dm.getData(header[col]);
							dh = &(e->add(header[col], att));
						}
						dh->convertFromString(body[col]);
						if (header[col] == CALIBRATION_DATA_NAME)
						{
							name = dynamic_cast<AQLDataString&>(dh->get()).get();
						}
					}
				}
				objPool.set(name, e);
				e = e->clone();
				befVal = &body;
				row++;
			}
			if (e) 
			{
				delete e;
			}

			if (row>=tradeMatrix.size()) break;
		}
	}
	AQLMathFXEntity* pFxsde = NULL;
	pFxsde = new AQLMathFXEntity(&dataInstance);
	objPool.set(FXSDE,pFxsde);

    dataInstance.getReferencePool().completeDependency();
    setupFundingChangeInfo(dataInstance);
    dataInstance.getReferencePool().completeDependency();

}



// 
/*!
    @brief setup fx object

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::setUpFXEntity(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = AQLDealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = AQLDealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	AQLString mainModel = AQLMarketData::getModelName(baseCurrency);
	// setup entities
	AQLObjectConfiguration *setUpper = AQLObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
	setUpper->setUpFXEntity(dataInstance.getObjectPool());
	// complete depenency
	dataInstance.getReferencePool().completeDependency();
	delete setUpper;
}


// 
/*!
    @brief setup fx object

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::setUpPathEntity(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = AQLDealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = AQLDealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	AQLString mainModel = AQLMarketData::getModelName(baseCurrency);
	// setup entities
	AQLObjectConfiguration *setUpper = AQLObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
	setUpper->setUpPathEntity(dataInstance.getObjectPool());
	// complete depenency
	dataInstance.getReferencePool().completeDependency();
	delete setUpper;
}

// 
/*!
    @brief setup entities

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::setUpEntityes(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = AQLDealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = AQLDealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

    AQLStaticData& staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
    if(staticData.getStaticData(CONTEXT_KEY_ISINCLUDECASH)=="true"){
        shiftPaymentDate(dataInstance);
    }

	AQLString mainModel = AQLMarketData::getModelName(baseCurrency);
	// setup entities
	AQLObjectConfiguration *setUpper = AQLObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
	setUpper->setUpEntityes(dataInstance);
	// complete depenency
	dataInstance.getReferencePool().completeDependency();
	delete setUpper;
}

// 
/*!
    @brief setup entities

	@param[out] dataInstance
*/
void
AQLDataInstanceConfigurationPV::setUpRiskInfo(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = AQLDealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = AQLDealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	AQLString mainModel = AQLMarketData::getModelName(baseCurrency);
	// setup entities
	AQLObjectConfiguration *setUpper = AQLObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
	setUpper->setUpRiskInfo(dataInstance);
	// complete depenency
	dataInstance.getReferencePool().completeDependency();
	delete setUpper;
}


void AQLDataInstanceConfigurationPV::shiftPaymentDate(AQLObject& trade, const AQLDate& asof1, const AQLDate& asof2) const 
{
	AQLDataHolder* dh = &(trade.getData(PRICING_DATA_PREMIUMPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		AQLDataDate& data_paydate = dynamic_cast<AQLDataDate &>(dh->get());
		const AQLDate paydate = dynamic_cast<AQLDataDate &>(dh->get()).get();
		if (paydate > asof1 && paydate <= asof2)
		{
			AQLPriceDataCalendar cal;
			const AQLDate shiftDate = AQLMathDateCalculations::getDate(asof2, "1D", SLIDING_RULE_FOLLOWING, &cal, true);
			data_paydate.set(shiftDate);
		}
	}

	dh = &(trade.getData(PRICING_DATA_CASHSETTLEMENTPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		AQLDataDate& data_paydate = dynamic_cast<AQLDataDate &>(dh->get());
		const AQLDate paydate = dynamic_cast<AQLDataDate &>(dh->get()).get();
		if (paydate > asof1 && paydate <= asof2)
		{
			AQLPriceDataCalendar cal;
			const AQLDate shiftDate = AQLMathDateCalculations::getDate(asof2, "1D", SLIDING_RULE_FOLLOWING, &cal, true);
			data_paydate.set(shiftDate);
		}
	}
	
	const AQLDataValuation& valuemehod = dynamic_cast<const AQLDataValuation &>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

    if (valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) || valuemehod.isTypeOf(FN_IR_TRADEVALUE) || valuemehod.isTypeOf(FN_IR_LSMCTRADEVALUE))
    {
        dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
        if (dh->isDefined() && !dh->isNull())
        {
            AQLDataMultiReference &legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
            unsigned int legSize = legs.getSize();
            for (unsigned int j = 0; j < legSize; j++)
            {
                AQLObject &eleg = legs.get(j).get();
                //get cashlet
                dh = &(eleg.getData(PRICING_DATA_CASHLETS, NOCHECK));
                if (dh->isDefined() && !dh->isNull())
                {
                    AQLDataMultiReference &cashlets = dynamic_cast<AQLDataMultiReference &>(dh->get());
                    unsigned int cashletSize = cashlets.getSize();
                    for (unsigned int k = 0; k < cashletSize; k++)
                    {
                        AQLObject &ecashlet = cashlets.get(k).get();
                        dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
                        AQLDataDate& data_paydate = dynamic_cast<AQLDataDate &>(dh->get());
                        const AQLDate& paydate = data_paydate.get();
                        if (paydate > asof1 && paydate <= asof2)
                        {
                            AQLPriceDataCalendar cal;
                            if((dh = &ecashlet.getData(CALIBRATION_DATA_CALENDAR))->isDefined() && !dh->isNull()){
                                cal = dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
                            }
                            else if((dh = &eleg.getData(CALIBRATION_DATA_CALENDAR))->isDefined() && !dh->isNull()){
                                cal = dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
                            }
                            const AQLDate shiftDate = AQLMathDateCalculations::getDate(asof2, "1D", SLIDING_RULE_FOLLOWING, &cal, true);
                            data_paydate.set(shiftDate);
                        }
                    }
                }
            }
        }
    }
}

AQLDate AQLDataInstanceConfigurationPV::getAsOfDate(AQLDataInstance& dataInstance) const
{
    AQLObjectPool& objPool = dataInstance.getObjectPool();
    AQLMathPathEntity* path = AQLMarketData::getPathEnitty(objPool);
    if (path != NULL){
        return path->getAsOfDate();
    }
    else{
        AQLMathPlainVanillaEntity* pvanilla = AQLMarketData::getPlainVanillaEntity(objPool);
        if (!pvanilla)
        {
            throw AQLCoreInvalidData("Neither AQLMathPathEntity nor AQLMathPlainVanillaEntity exists.", __FILE__, __LINE__);
        }
        return pvanilla->getAsOfDate();
    }
}

void AQLDataInstanceConfigurationPV::shiftPaymentDate(AQLDataInstance& dataInstance) const
{
    AQLObjectPool& objPool = dataInstance.getObjectPool();

    AQLStaticData& staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
    const AQLDate asof1 = AQLDate(staticData.getStaticData(CONTEXT_KEY_INCLUDECASH_FROM).getCString());
    const AQLDate asof2 = getAsOfDate(dataInstance);


    const AQLString mainTradeName = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
    AQLObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);
    if(dynamic_cast<const AQLDataValuation &>(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE){
        AQLDataMultiReference &unders = dynamic_cast<AQLDataMultiReference &>(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
        const unsigned int tradeSize = unders.getSize();
        for (unsigned int i = 0; i < tradeSize; i++){
            shiftPaymentDate(unders.get(i).get(), asof1, asof2);
        }
    }
    else{
        shiftPaymentDate(objHolder.get(), asof1, asof2);
    }
}

void AQLDataInstanceConfigurationPV::insertContext(AQLStringMatrix& m, AQLDataInstance& dataInstance) const
{
    const AQLStringVector* header;
    const bool is_vanilla = AQLCoreDataService::getContext(ARG_KEY_CALC).toUpper() == "VANILLA";
    const AQLString fx_entity_name = is_vanilla ? FORWARDFX : FXSDE;
    const bool has_fx_entity = dataInstance.getObjectPool().getObject(fx_entity_name).isDefined();
    for(size_t i = 0; i < m.size(); i++){
        if(m[i][0] == "object_t"){
            header = &m[i];
        }
        else{
            for(size_t j = 0; j < header->size() && j < m[i].size(); j++){
                if(header->at(j) == CALIBRATION_DATA_ASOFDATE){
                    m[i][j] = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
                    continue;
                }
                if(header->at(j) == PRICING_DATA_TODAY){
                    if(m[i][j]!="") continue;
                    m[i][j] = AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
                    continue;
                }
                if(header->at(j) == PRICING_DATA_SETTLEDATE){
                    if(m[i][j]!="") continue;
                    const AQLString& temp = AQLCoreDataService::getContext(ARG_KEY_SETTLEDATE);
                    m[i][j] = temp==AQ_NO_DATA ? AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE) : temp;
                    continue;
                }
                if(header->at(j) == PRICING_DATA_VALUEDATE){
                    const AQLString& temp = AQLCoreDataService::getContext(ARG_KEY_VALUEDATE);
                    m[i][j] = temp==AQ_NO_DATA ? AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE) : temp;
                    continue;
                }
                if(header->at(j) == PRICING_DATA_PATHENTITY){
                    m[i][j] = mPathEntityName;
                    continue;
                }
                if(header->at(j) == PRICING_DATA_MARKETPARAM){
                    m[i][j] = mPathEntityName;
                    continue;
                }
                if(has_fx_entity){
                    if(header->at(j) == PRICING_DATA_FXRATE){
                        if(m[i][j]!="") continue;
                        m[i][j] = fx_entity_name;
                        continue;
                    }
                    if(header->at(j) == PRICING_DATA_EXTRACFFXRATE){
                        m[i][j] = fx_entity_name;
                        continue;
                    }
                }
            }
        }
    }
} 

void AQLDataInstanceConfigurationPV::setupFundingChangeInfo(AQLDataInstance& dataInstance) const
{
    typedef map<AQLString, AQLString> FchMap;

    const AQLString fname = AQLCoreDataService::getStaticDataManager().getStaticData().getStaticData(CONTEXT_KEY_FUNDINGCHANGE_FILE);
    if(fname == AQ_NO_DATA) return;
    FchMap trade_fchinfo;
    AQLFileAccessor fch_file(AQLMarketData::getNumFileName(fname));
    AQLStringMatrix mat; 
    fch_file.readAllData(',', mat);
    for(size_t i = 0; i < mat.size(); i++) trade_fchinfo[mat[i][0]] = mat[i][1];




    AQLObjectPool& objPool = dataInstance.getObjectPool();
    const AQLString port_name = AQLCoreDataService::getContext(ARG_KEY_MAINTRADE);
    AQLDataMultiReference& trades = dynamic_cast<AQLDataMultiReference&>(objPool.getObject(port_name, ENCHKTYPE_ISDEFINED).getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    for(size_t i = 0; i < trades.getSize(); i++){
        AQLObjectHolder& objHolder = trades.get(i);
        const AQLString name = dynamic_cast<const AQLDataString&>(objHolder.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
        FchMap::const_iterator it = trade_fchinfo.find(name);
        if(it==trade_fchinfo.end()) continue;
        objHolder.remove(PRICING_DATA_FUNDINGCHANGEINFO);
        objHolder.add(PRICING_DATA_FUNDINGCHANGEINFO, new AQLDataReference()).convertFromString(it->second);
    }
}

// 
/*!
    @brief setup risk object

	@param[out] dataInstance
*/
void AQLDataInstanceConfigurationPV::setUpRiskEntityes(AQLDataInstance& dataInstance) const
{
	dataInstance;
	// do nothing
}