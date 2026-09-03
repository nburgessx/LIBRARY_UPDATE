#define _HAS_STD_BYTE 0

/*! @file
    @brief DataInstance setup class for calc PV
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADataInstanceConfigurationPV.cpp
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


#include "LADataInstanceConfigurationPV.h"
#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreReferencePool.h"
#include "AQLRatesSDEBase.h"
#include "AQLDataCSVFileLoader.h"
#include "AQLLinearRatesOptionValue.h"
#include "LACoreDataService.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LAModelSetupBase.h"
#include "LAModelManager.h"
#include "LADataInstanceConfiguration.h"
#include "LAModelConfiguration.h"
#include "LACalibrateModel.h"
#include "LACalibrateModelIR.h"
#include "LAObjectConfigurationManager.h"
#include "LAObjectConfiguration.h"
#include "AQLPriceDataFunction.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLPriceLSMCTradeValue.h"
#include "AQLPricePortfolioValue.h"
#include "AQLDataReference.h"
#include "AQLDataMatrix.h"
#include "AQLFunctionUtilities.h"
#include "LADefinitionsIRSABR.h"
#include "AQLMathValuableEntity.h"


using namespace std;

#define MARKETPARAM	"marketparam1"
#define PATH1	"path1"
#define FXSDE	"fx_sde"
#define FORWARDFX "FORWARDFX"



// constructor
/*!

*/
LADataInstanceConfigurationPV::LADataInstanceConfigurationPV()
: LADataInstanceConfiguration()
{

}

// destructor
/*!

*/
LADataInstanceConfigurationPV::~LADataInstanceConfigurationPV(void)
{
}

// 
/*!
	@brief setup

*/
void 
LADataInstanceConfigurationPV::setUp(void)
{
	mSDECurrencys = MADealUtils::getSDECurrencys(true);
	int ccySize = mSDECurrencys.size();
	AQLString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();
	mPathEntityName = calc == "VANILLA" ? MARKETPARAM : PATH1;

	// set model
	mSDEModels.resize(ccySize);
	for (int i = 0; i < ccySize; ++i)
	{
		mSDEModels[i] = LAMarketData::getModelName(mSDECurrencys[i]).toUpper();
		if (mSDEModels[i] == MODEL_IRSABR) mPathEntityName = MARKETPARAM;
	}

	//set maxterm
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLString maxFileName = staticData.getStaticData(KEY_DEAL_MAXTERM_FILE);
	if (maxFileName != AQ_NO_DATA)
	{
		maxFileName = LAMarketData::getNumFileName(maxFileName, MLIBID);
		MAFileAccessor file(maxFileName);
		AQLStringMatrix dataMtx;
		file.readAllData(MARKET_DATA_DELIMITER, dataMtx);
		file.close();
		const AQLString asofStr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
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
		LACoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, AQLString(maxTerm));
		LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, AQLString(maxTerm));
	}
	

}

// 
/*!
    @brief setup masters

	@param[out] dataInstance
*/
void
LADataInstanceConfigurationPV::setUpMasters(AQLDataInstance &dataInstance) const
{
	// regist master data for base ccy
	LAModelSetupBase *reg = MAMasterRegistManager::getInstance()->createRegister(mSDEModels[0]);
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
LADataInstanceConfigurationPV::setUpSDE(AQLDataInstance &dataInstance) const
{
	map<AQLString, LACalibrateModel *> gMap;
	unsigned int ccySize = mSDECurrencys.size();
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		if (gMap.find(mSDEModels[i]) == gMap.end())
		{
			LACalibrateModel *generator = LAModelConfiguration::getInstance()->createSDEGenerator(mSDEModels[i]);
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
	map<AQLString, LACalibrateModel *>::iterator it = gMap.begin();
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
LADataInstanceConfigurationPV::createPathEntity(AQLDataInstance &dataInstance) const
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
		AQLString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		pPlain->getAsOfDate().convertFromString(asofstr);
		LACoreDataService::setContext(CONTEXT_KEY_PLAINVANILLAENTITY_NAME, MARKETPARAM);
		LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
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
		AQLString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		pPath->getAsOfDate().convertFromString(asofstr);
		LACoreDataService::setContext(CONTEXT_KEY_PATHENTITY_NAME, PATH1);
		
		////AQLDate asofDate = LAMarketData::getAsofDate(objPool);
		AQLString dayCount = LAMarketData::getTimeGridDayCount(objPool);
		LACoreDataService::setContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT,dayCount);
		LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	}
}


// 
/*!
    @brief create fx object

	@param[out] dataInstance
*/
void
LADataInstanceConfigurationPV::createFXEntity(AQLDataInstance &dataInstance) const
{
	// get EntityPool
	AQLObjectPool& objPool = dataInstance.getObjectPool();
	AQLString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();
	
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
		LAMarketData::setUpMarket2FXEntity(*pFwd);

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
			LAMarketData::setUpMarket2FXEntity(*pFxsde);
			pFxsde->getPathEntity().convertFromString(PATH1);
		
		}

		//AQLMathFXEntity* pFX = LAMarketData::getFXEntity(objPool, "FORWARDRATE");
		if (!pFwd)
			throw AQLCoreInvalidData("ForwardRate FX Object does not exist",__FILE__,__LINE__);
		AQLString fxName = pFwd->getName();
		LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, fxName);


		//pFX = LAMarketData::getFXEntity(objPool, "USEMODEL");
		if (pFxsde)
		{
			fxName = pFxsde->getName();
			LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FXSDE, fxName);
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
LADataInstanceConfigurationPV::loadEntities(AQLDataInstance &dataInstance) const
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLString fileName = staticData.getStaticData(KEY_DEAL_FILE);

// update for XLL Plus //////////////////////////////////// 
/*
	// csv loader
	MDCSVFileLoader fileLoader;
	fileLoader.setDataInstance(&dataInstance);

	AQLObject* pEntity = new AQLObject;
	AQLStringVector strVec(1, LAMarketData::getNumFileName(fileName));
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

	AQLString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();
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
	//	AQLString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//	pPlain->getAsOfDate().convertFromString(asofstr);
	//	LACoreDataService::setContext(CONTEXT_KEY_PLAINVANILLAENTITY_NAME, MARKETPARAM);
	//	LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
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
	//	AQLString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//	pPath->getAsOfDate().convertFromString(asofstr);
	//	LACoreDataService::setContext(CONTEXT_KEY_PATHENTITY_NAME, PATH1);
	//	
	//	////AQLDate asofDate = LAMarketData::getAsofDate(objPool);
	//	AQLString dayCount = LAMarketData::getTimeGridDayCount(objPool);
	//	LACoreDataService::setContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT,dayCount);
	//	LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
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
	//	LAMarketData::setUpMarket2FXEntity(*pFwd);

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
	//		LAMarketData::setUpMarket2FXEntity(*pFxsde);
	//		pFxsde->getPathEntity().convertFromString(PATH1);
	//	
	//	}

	//	//AQLMathFXEntity* pFX = LAMarketData::getFXEntity(objPool, "FORWARDRATE");
	//	if (!pFwd)
	//		throw AQLCoreInvalidData("ForwardRate FX Object does not exist",__FILE__,__LINE__);
	//	AQLString fxName = pFwd->getName();
	//	LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, fxName);


	//	//pFX = LAMarketData::getFXEntity(objPool, "USEMODEL");
	//	if (pFxsde)
	//	{
	//		fxName = pFxsde->getName();
	//		LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FXSDE, fxName);
	//	}

 //       if(calc=="VANILLA"){
 //           AQLMathPlainVanillaEntity* vanilla = &dynamic_cast<AQLMathPlainVanillaEntity&>(objPool.getObject(MARKETPARAM, ENCHKTYPE_ISDEFINED).get());
 //           vanilla->getFXEntity().convertFromString(FORWARDFX);
 //       }
	//}

	AQLStringMatrix tradeMatrix;
	MAFileAccessor tradeFile(LAMarketData::getNumFileName(fileName, MLIBID));
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
	
	AQLDate asofDate = LAMarketData::getAsofDate(objPool);
	//after data in set deal info (maxterm)
	if (calc == "VANILLA")
	{
		const int maxTerm = MADealUtils::getMaxTerm(objPool, asofDate, AQLString("VANILLA"));
		LACoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, AQLString(maxTerm));
		LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, AQLString(maxTerm));
	}
	else
	{
		// set maxterm
		AQLString request = LACoreDataService::getContext(ARG_KEY_REQUEST);
		const int maxTerm = MADealUtils::getMaxTerm(objPool, asofDate);
		LACoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, AQLString(maxTerm));
		if (request == "EOD")
		{
			LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, staticData.getStaticData(KEY_SIMULATION_TERM_MAX));

		}
		else
		{
			const int buffer = staticData.getStaticData(KEY_SIMULATION_TERM_BUFFER).getIntValue();
			LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, AQLString(maxTerm + buffer));
		}
	}


	// set calibration target flag of ir volatility matrix
	// get object pool
	LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();
	AQLStringVector ccys = MADealUtils::getAllSingleCurrencys();
	lowervec(ccys);
	for (size_t i = 0; i < ccys.size(); ++i)
	{
		AQLString model = LAMarketData::getModelName(ccys[i]);
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
				const AQLString calibTarget = LACoreDataService::getContext(key);
				if (calibTarget != AQ_NO_DATA && !isAllGridsCalibrate)
				{
					continue;
				}
				if (isAllGridsCalibrate)
				{
					LACoreDataService::setContext(key, AQ_NO_DATA);
				}
				else
				{
					BoolMatrix calibTarget = MADealUtils::getCalibTargetIRVolGrids(objPool, asofDate, ccys[i], underlyings[j]);
					AQLDataBoolMatrix tmp(calibTarget);
					LACoreDataService::setContext(key, tmp.convertToString());
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
LADataInstanceConfigurationPV::loadCSV(AQLDataInstance &dataInstance) const
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLString fileName = staticData.getStaticData(KEY_DEAL_FILE);

    // get AttributeMaster
    AQLPriceDataManager& dm = dataInstance.getDataMaster();
    // get EntityMaster
    AQLObjectMaster& em = dataInstance.getObjectMaster();
	// get EntityPool
	AQLObjectPool& objPool = dataInstance.getObjectPool();

	AQLString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();

	AQLStringMatrix tradeMatrix;
	MAFileAccessor tradeFile(LAMarketData::getNumFileName(fileName, MLIBID));
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
LADataInstanceConfigurationPV::setUpFXEntity(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = MADealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	AQLString mainModel = LAMarketData::getModelName(baseCurrency);
	// setup entities
	LAObjectConfiguration *setUpper = LAObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
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
LADataInstanceConfigurationPV::setUpPathEntity(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = MADealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	AQLString mainModel = LAMarketData::getModelName(baseCurrency);
	// setup entities
	LAObjectConfiguration *setUpper = LAObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
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
LADataInstanceConfigurationPV::setUpEntityes(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = MADealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

    LAStaticData& staticData = LACoreDataService::getStaticDataManager().getStaticData();
    if(staticData.getStaticData(CONTEXT_KEY_ISINCLUDECASH)=="true"){
        shiftPaymentDate(dataInstance);
    }

	AQLString mainModel = LAMarketData::getModelName(baseCurrency);
	// setup entities
	LAObjectConfiguration *setUpper = LAObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
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
LADataInstanceConfigurationPV::setUpRiskInfo(AQLDataInstance &dataInstance) const
{
	// get model name for base currency
	AQLString baseCurrency = MADealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	AQLString mainModel = LAMarketData::getModelName(baseCurrency);
	// setup entities
	LAObjectConfiguration *setUpper = LAObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
	setUpper->setUpRiskInfo(dataInstance);
	// complete depenency
	dataInstance.getReferencePool().completeDependency();
	delete setUpper;
}


void LADataInstanceConfigurationPV::shiftPaymentDate(AQLObject& trade, const AQLDate& asof1, const AQLDate& asof2) const 
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

AQLDate LADataInstanceConfigurationPV::getAsOfDate(AQLDataInstance& dataInstance) const
{
    AQLObjectPool& objPool = dataInstance.getObjectPool();
    AQLMathPathEntity* path = LAMarketData::getPathEnitty(objPool);
    if (path != NULL){
        return path->getAsOfDate();
    }
    else{
        AQLMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
        if (!pvanilla)
        {
            throw AQLCoreInvalidData("Neither AQLMathPathEntity nor AQLMathPlainVanillaEntity exists.", __FILE__, __LINE__);
        }
        return pvanilla->getAsOfDate();
    }
}

void LADataInstanceConfigurationPV::shiftPaymentDate(AQLDataInstance& dataInstance) const
{
    AQLObjectPool& objPool = dataInstance.getObjectPool();

    LAStaticData& staticData = LACoreDataService::getStaticDataManager().getStaticData();
    const AQLDate asof1 = AQLDate(staticData.getStaticData(CONTEXT_KEY_INCLUDECASH_FROM).getCString());
    const AQLDate asof2 = getAsOfDate(dataInstance);


    const AQLString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
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

void LADataInstanceConfigurationPV::insertContext(AQLStringMatrix& m, AQLDataInstance& dataInstance) const
{
    const AQLStringVector* header;
    const bool is_vanilla = LACoreDataService::getContext(ARG_KEY_CALC).toUpper() == "VANILLA";
    const AQLString fx_entity_name = is_vanilla ? FORWARDFX : FXSDE;
    const bool has_fx_entity = dataInstance.getObjectPool().getObject(fx_entity_name).isDefined();
    for(size_t i = 0; i < m.size(); i++){
        if(m[i][0] == "object_t"){
            header = &m[i];
        }
        else{
            for(size_t j = 0; j < header->size() && j < m[i].size(); j++){
                if(header->at(j) == CALIBRATION_DATA_ASOFDATE){
                    m[i][j] = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
                    continue;
                }
                if(header->at(j) == PRICING_DATA_TODAY){
                    if(m[i][j]!="") continue;
                    m[i][j] = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
                    continue;
                }
                if(header->at(j) == PRICING_DATA_SETTLEDATE){
                    if(m[i][j]!="") continue;
                    const AQLString& temp = LACoreDataService::getContext(ARG_KEY_SETTLEDATE);
                    m[i][j] = temp==AQ_NO_DATA ? LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE) : temp;
                    continue;
                }
                if(header->at(j) == PRICING_DATA_VALUEDATE){
                    const AQLString& temp = LACoreDataService::getContext(ARG_KEY_VALUEDATE);
                    m[i][j] = temp==AQ_NO_DATA ? LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE) : temp;
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

void LADataInstanceConfigurationPV::setupFundingChangeInfo(AQLDataInstance& dataInstance) const
{
    typedef map<AQLString, AQLString> FchMap;

    const AQLString fname = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(CONTEXT_KEY_FUNDINGCHANGE_FILE);
    if(fname == AQ_NO_DATA) return;
    FchMap trade_fchinfo;
    MAFileAccessor fch_file(LAMarketData::getNumFileName(fname));
    AQLStringMatrix mat; 
    fch_file.readAllData(',', mat);
    for(size_t i = 0; i < mat.size(); i++) trade_fchinfo[mat[i][0]] = mat[i][1];




    AQLObjectPool& objPool = dataInstance.getObjectPool();
    const AQLString port_name = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
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
void LADataInstanceConfigurationPV::setUpRiskEntityes(AQLDataInstance& dataInstance) const
{
	dataInstance;
	// do nothing
}