#define _HAS_STD_BYTE 0

/*! @file
    @brief DataInstance setup class for calc PV
*/
//  2007, Mizuho International London.
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
#include "LAString.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAObjectPool.h"
#include "LACoreReferencePool.h"
#include "LARatesSDEBase.h"
#include "LADataCSVFileLoader.h"
#include "LALinearRatesOptionValue.h"
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
#include "LAPriceDataFunction.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LAPriceLSMCTradeValue.h"
#include "LAPricePortfolioValue.h"
#include "LADataReference.h"
#include "LADataMatrix.h"
#include "LAFunctionUtilities.h"
#include "LADefinitionsIRSABR.h"
#include "LAMathValuableEntity.h"


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
	LAString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();
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
	LAString maxFileName = staticData.getStaticData(KEY_DEAL_MAXTERM_FILE);
	if (maxFileName != MLIB_NO_DATA)
	{
		maxFileName = LAMarketData::getNumFileName(maxFileName, MLIBID);
		MAFileAccessor file(maxFileName);
		LAStringMatrix dataMtx;
		file.readAllData(MARKET_DATA_DELIMITER, dataMtx);
		file.close();
		const LAString asofStr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		if (asofStr == MLIB_NO_DATA)
		{
			throw LACoreInvalidData("asofdate does not given in arguments", __FILE__, __LINE__);
		}
		LADate asofDate(asofStr.getCString());
		LADate maxDate = asofDate;
		LADate endDate;
		for (unsigned int i = 0; i < dataMtx.size(); ++i)
		{
			if (dataMtx[i].size() != 2)
			{
				throw LACoreInvalidData("maxterm file format is wrong", __FILE__ , __LINE__);
			}
			if(!(dataMtx[i][0].toUpper()==""))
			{
				endDate.setDate(dataMtx[i][0].getCString());
			}
			else
			{
				endDate=asofDate;
			}
			const LAStringVector addYearsVec = dataMtx[i][1].toToken(':');
			for (unsigned int j = 0; j < addYearsVec.size(); ++j)
			{
				const LADate date = LAMathDateCalculations::getDate(endDate, addYearsVec[j], true);
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
		LACoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, LAString(maxTerm));
		LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, LAString(maxTerm));
	}
	

}

// 
/*!
    @brief setup masters

	@param[out] dataInstance
*/
void
LADataInstanceConfigurationPV::setUpMasters(LADataInstance &dataInstance) const
{
	// regist master data for base ccy
	LAModelSetupBase *reg = MAMasterRegistManager::getInstance()->createRegister(mSDEModels[0]);
	if (!reg)
	{
		LAString msg;
		if (mSDEModels[0] == MLIB_NO_DATA)
		{
			msg = "IR SED model is not registered !!";
		}
		else
		{
            msg = mSDEModels[0] + " is not supported !!";
		}
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
LADataInstanceConfigurationPV::setUpSDE(LADataInstance &dataInstance) const
{
	map<LAString, LACalibrateModel *> gMap;
	unsigned int ccySize = mSDECurrencys.size();
	for (unsigned int i = 0; i < ccySize; ++i)
	{
		if (gMap.find(mSDEModels[i]) == gMap.end())
		{
			LACalibrateModel *generator = LAModelConfiguration::getInstance()->createSDEGenerator(mSDEModels[i]);
			if (!generator) throw LACoreInvalidData("SDE Model is not set",__FILE__, __LINE__);
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
	map<LAString, LACalibrateModel *>::iterator it = gMap.begin();
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
LADataInstanceConfigurationPV::createPathEntity(LADataInstance &dataInstance) const
{
	// get EntityPool
	LAObjectPool& objPool = dataInstance.getObjectPool();

	if (mPathEntityName == MARKETPARAM)
	{
		//new plainvanilla object
		LAMathPlainVanillaEntity* pPlain = NULL;
		LAObjectHolder ehpath = objPool.getObject(MARKETPARAM, ENCHKTYPE_NOCHECK);
		if (!ehpath.isDefined())
		{
			pPlain = new LAMathPlainVanillaEntity(&dataInstance);
			objPool.set(MARKETPARAM,pPlain);
		
		}
		else
		{
			dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(MARKETPARAM).get()).reset();
			pPlain = &dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(PATH1).get());
		}
		pPlain->getName().convertFromString(MARKETPARAM);
		pPlain->getDayCount().setDayCount(ACT_365_ISDA);
		LAString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		pPlain->getAsOfDate().convertFromString(asofstr);
		LACoreDataService::setContext(CONTEXT_KEY_PLAINVANILLAENTITY_NAME, MARKETPARAM);
		LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	}
	else
	{
		//new path object
		LAMathPathEntity* pPath = NULL;
		LAObjectHolder ehpath = objPool.getObject(PATH1, ENCHKTYPE_NOCHECK);
		if (!ehpath.isDefined())
		{
			pPath = new LAMathPathEntity(&dataInstance);
			objPool.set(PATH1,pPath);
		
		}
		else
		{
			dynamic_cast<LAMathPathEntity &>(objPool.getObject(PATH1).get()).reset();
			pPath = &dynamic_cast<LAMathPathEntity &>(objPool.getObject(PATH1).get());
		}
		pPath->getName().convertFromString(PATH1);
		pPath->getDayCount().setDayCount(ACT_365_ISDA);
		LAString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
		pPath->getAsOfDate().convertFromString(asofstr);
		LACoreDataService::setContext(CONTEXT_KEY_PATHENTITY_NAME, PATH1);
		
		////LADate asofDate = LAMarketData::getAsofDate(objPool);
		LAString dayCount = LAMarketData::getTimeGridDayCount(objPool);
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
LADataInstanceConfigurationPV::createFXEntity(LADataInstance &dataInstance) const
{
	// get EntityPool
	LAObjectPool& objPool = dataInstance.getObjectPool();
	LAString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();
	
	if (mSDECurrencys.size() > 1)
	{
		//new fxentity
		LAMathFXEntity* pFwd = NULL;
		LAObjectHolder ehfx = objPool.getObject(FORWARDFX, ENCHKTYPE_NOCHECK);
		if (!ehfx.isDefined())
		{
			pFwd = new LAMathFXEntity(&dataInstance);
			objPool.set(FORWARDFX,pFwd);
		}
		else
		{
			dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get()).reset();
			pFwd = &dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get());
		}
		pFwd->getName().convertFromString(FORWARDFX);
		pFwd->getFXType().convertFromString("FORWARDRATE");
		LAMarketData::setUpMarket2FXEntity(*pFwd);

		//exo case add the usemodel fx object
		LAMathFXEntity* pFxsde = NULL;
		if (mPathEntityName == PATH1)
		{
			
			LAObjectHolder ehfx = objPool.getObject(FXSDE, ENCHKTYPE_NOCHECK);
			if (!ehfx.isDefined())
			{
				pFxsde = new LAMathFXEntity(&dataInstance);
				objPool.set(FXSDE,pFxsde);
			}
			else
			{
				dynamic_cast<LAMathFXEntity &>(objPool.getObject(FXSDE).get()).reset();
				pFxsde = &dynamic_cast<LAMathFXEntity &>(objPool.getObject(FXSDE).get());
			}

			pFxsde->getName().convertFromString(FXSDE);
			pFxsde->getFXType().convertFromString("USEMODEL");
			LAMarketData::setUpMarket2FXEntity(*pFxsde);
			pFxsde->getPathEntity().convertFromString(PATH1);
		
		}

		//LAMathFXEntity* pFX = LAMarketData::getFXEntity(objPool, "FORWARDRATE");
		if (!pFwd)
			throw LACoreInvalidData("ForwardRate FX Object does not exist",__FILE__,__LINE__);
		LAString fxName = pFwd->getName();
		LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, fxName);


		//pFX = LAMarketData::getFXEntity(objPool, "USEMODEL");
		if (pFxsde)
		{
			fxName = pFxsde->getName();
			LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FXSDE, fxName);
		}

        if(calc=="VANILLA"){
            LAMathPlainVanillaEntity* vanilla = &dynamic_cast<LAMathPlainVanillaEntity&>(objPool.getObject(MARKETPARAM, ENCHKTYPE_ISDEFINED).get());
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
LADataInstanceConfigurationPV::loadEntities(LADataInstance &dataInstance) const
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString fileName = staticData.getStaticData(KEY_DEAL_FILE);

// update for XLL Plus //////////////////////////////////// 
/*
	// csv loader
	MDCSVFileLoader fileLoader;
	fileLoader.setDataInstance(&dataInstance);

	LAObject* pEntity = new LAObject;
	LAStringVector strVec(1, LAMarketData::getNumFileName(fileName));
	pEntity->add(CALIBRATION_DATA_MD_FILEPATHS, new LADataStrings(strVec));
	Records_var ret = fileLoader.get(LAObjectHolder(pEntity, true));

	// get object pool
	LAObjectPool &objPool = dataInstance.getObjectPool();
	std::vector<LAObjectHolder>::iterator it = ret->begin();
	while (it != ret->end())
	{
		objPool.set(it->getName(), &(it->get()));
		++it;
	}
*/
    // get AttributeMaster
    LAPriceDataManager& dm = dataInstance.getDataMaster();
    // get EntityMaster
    LAObjectMaster& em = dataInstance.getObjectMaster();
	// get EntityPool
	LAObjectPool& objPool = dataInstance.getObjectPool();

	LAString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();
	//if (calc == "VANILLA")
	//{

	//	//new plainvanilla object
	//	LAMathPlainVanillaEntity* pPlain = NULL;
	//	LAObjectHolder ehpath = objPool.getObject(MARKETPARAM, ENCHKTYPE_NOCHECK);
	//	if (!ehpath.isDefined())
	//	{
	//		pPlain = new LAMathPlainVanillaEntity(&dataInstance);
	//		objPool.set(MARKETPARAM,pPlain);
	//	
	//	}
	//	else
	//	{
	//		dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(MARKETPARAM).get()).reset();
	//		pPlain = &dynamic_cast<LAMathPlainVanillaEntity &>(objPool.getObject(PATH1).get());
	//	}
	//	pPlain->getName().convertFromString(MARKETPARAM);
	//	pPlain->getDayCount().setDayCount(ACT_365_ISDA);
	//	LAString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//	pPlain->getAsOfDate().convertFromString(asofstr);
	//	LACoreDataService::setContext(CONTEXT_KEY_PLAINVANILLAENTITY_NAME, MARKETPARAM);
	//	LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	//}
	//else
	//{
	//	//new path object
	//	LAMathPathEntity* pPath = NULL;
	//	LAObjectHolder ehpath = objPool.getObject(PATH1, ENCHKTYPE_NOCHECK);
	//	if (!ehpath.isDefined())
	//	{
	//		pPath = new LAMathPathEntity(&dataInstance);
	//		objPool.set(PATH1,pPath);
	//	
	//	}
	//	else
	//	{
	//		dynamic_cast<LAMathPathEntity &>(objPool.getObject(PATH1).get()).reset();
	//		pPath = &dynamic_cast<LAMathPathEntity &>(objPool.getObject(PATH1).get());
	//	}
	//	pPath->getName().convertFromString(PATH1);
	//	pPath->getDayCount().setDayCount(ACT_365_ISDA);
	//	LAString asofstr = LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE);
	//	pPath->getAsOfDate().convertFromString(asofstr);
	//	LACoreDataService::setContext(CONTEXT_KEY_PATHENTITY_NAME, PATH1);
	//	
	//	////LADate asofDate = LAMarketData::getAsofDate(objPool);
	//	LAString dayCount = LAMarketData::getTimeGridDayCount(objPool);
	//	LACoreDataService::setContext(CONTEXT_KEY_TIMEGRID_DAYCOUNT,dayCount);
	//	LACoreDataService::setContext(CONTEXT_KEY_ASOFDATE, asofstr);
	//}
	//
	//if (mSDECurrencys.size() > 1)
	//{
	//	//new fxentity
	//	LAMathFXEntity* pFwd = NULL;
	//	LAObjectHolder ehfx = objPool.getObject(FORWARDFX, ENCHKTYPE_NOCHECK);
	//	if (!ehfx.isDefined())
	//	{
	//		pFwd = new LAMathFXEntity(&dataInstance);
	//		objPool.set(FORWARDFX,pFwd);
	//	}
	//	else
	//	{
	//		dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get()).reset();
	//		pFwd = &dynamic_cast<LAMathFXEntity &>(objPool.getObject(FORWARDFX).get());
	//	}
	//	pFwd->getName().convertFromString(FORWARDFX);
	//	pFwd->getFXType().convertFromString("FORWARDRATE");
	//	LAMarketData::setUpMarket2FXEntity(*pFwd);

	//	//exo case add the usemodel fx object
	//	LAMathFXEntity* pFxsde = NULL;
	//	if (calc != "VANILLA")
	//	{
	//		
	//		LAObjectHolder ehfx = objPool.getObject(FXSDE, ENCHKTYPE_NOCHECK);
	//		if (!ehfx.isDefined())
	//		{
	//			pFxsde = new LAMathFXEntity(&dataInstance);
	//			objPool.set(FXSDE,pFxsde);
	//		}
	//		else
	//		{
	//			dynamic_cast<LAMathFXEntity &>(objPool.getObject(FXSDE).get()).reset();
	//			pFxsde = &dynamic_cast<LAMathFXEntity &>(objPool.getObject(FXSDE).get());
	//		}

	//		pFxsde->getName().convertFromString(FXSDE);
	//		pFxsde->getFXType().convertFromString("USEMODEL");
	//		LAMarketData::setUpMarket2FXEntity(*pFxsde);
	//		pFxsde->getPathEntity().convertFromString(PATH1);
	//	
	//	}

	//	//LAMathFXEntity* pFX = LAMarketData::getFXEntity(objPool, "FORWARDRATE");
	//	if (!pFwd)
	//		throw LACoreInvalidData("ForwardRate FX Object does not exist",__FILE__,__LINE__);
	//	LAString fxName = pFwd->getName();
	//	LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD, fxName);


	//	//pFX = LAMarketData::getFXEntity(objPool, "USEMODEL");
	//	if (pFxsde)
	//	{
	//		fxName = pFxsde->getName();
	//		LACoreDataService::setContext(CONTEXT_KEY_FXENTIY_NAME_FXSDE, fxName);
	//	}

 //       if(calc=="VANILLA"){
 //           LAMathPlainVanillaEntity* vanilla = &dynamic_cast<LAMathPlainVanillaEntity&>(objPool.getObject(MARKETPARAM, ENCHKTYPE_ISDEFINED).get());
 //           vanilla->getFXEntity().convertFromString(FORWARDFX);
 //       }
	//}

	LAStringMatrix tradeMatrix;
	MAFileAccessor tradeFile(LAMarketData::getNumFileName(fileName, MLIBID));
	tradeFile.readAllData(',',tradeMatrix);
    insertContext(tradeMatrix, dataInstance);
	unsigned int row = 0, col;
	unsigned int size = tradeMatrix.size();	
	LAString valueFuncName = staticData.getStaticData(KEY_DEAL_VALUEFUNCTION);
	if (size>0)
	{
		while(true)
		{
			const LAStringVector &header = tradeMatrix[row];

			// check header
			if (header.empty() || header[0] != "object_t")
			{
				LAString msg("Invalid Format : First item is not object_t [");
				msg += header[0] + " ]";
				throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
			}

			const LAStringVector *befVal = &tradeMatrix[row];
			row++;
			if (tradeMatrix[row].empty())
			{
				break;
			}
			int m_type = tradeMatrix[row][0].getIntValue();
            const LAObjectHolder& objHolder = em.getObject(m_type);
            if (! objHolder.isDefined())
            {
                LAString msg("Invalid Format : Not exist object_t (");
                msg += m_type + ") [ line-";
                msg += LAString((int)row) + "]";
                throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
            }

			LAObject* e = objHolder.get().clone();	
			unsigned int headerSize = header.size();
			while(row<size)
			{
				const LAStringVector &body = tradeMatrix[row];

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
				    LAString msg("Invalid Format : object_t is different from upper line (");
					msg += body[0] + ") [ line-";
					msg += LAString((int)row) + "]";
					throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
				}

                // copy Object
                LAString name("");
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
						LADataHolder* dh = &(e->getData(header[col]));
						if (!dh->isDefined() || dh->getType() == DATA_FUNCTION)
						{
							e->remove(header[col]);
							const LADataHolder& att = dm.getData(header[col]);
							dh = &(e->add(header[col], att));
						}

						if (header[col] == CALIBRATION_DATA_VALUE && body[col] != FN_IR_PORTFOLIOVALUE_STR && valueFuncName != MLIB_NO_DATA)
						{
							dh->convertFromString(valueFuncName);
						}
						else
						{
							dh->convertFromString(body[col]);
						}

						if (header[col] == CALIBRATION_DATA_NAME)
						{
							name = dynamic_cast<LADataString&>(dh->get()).get();
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
	
	LADate asofDate = LAMarketData::getAsofDate(objPool);
	//after data in set deal info (maxterm)
	if (calc == "VANILLA")
	{
		const int maxTerm = MADealUtils::getMaxTerm(objPool, asofDate, LAString("VANILLA"));
		LACoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, LAString(maxTerm));
		LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, LAString(maxTerm));
	}
	else
	{
		// set maxterm
		LAString request = LACoreDataService::getContext(ARG_KEY_REQUEST);
		const int maxTerm = MADealUtils::getMaxTerm(objPool, asofDate);
		LACoreDataService::setContext(CONTEXT_KEY_DEAL_MAXTERM, LAString(maxTerm));
		if (request == "EOD")
		{
			LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, staticData.getStaticData(KEY_SIMULATION_TERM_MAX));

		}
		else
		{
			const int buffer = staticData.getStaticData(KEY_SIMULATION_TERM_BUFFER).getIntValue();
			LACoreDataService::setContext(CONTEXT_KEY_MAXTERM, LAString(maxTerm + buffer));
		}
	}


	// set calibration target flag of ir volatility matrix
	// get object pool
	LAStaticData &calibProp = LACoreDataService::getStaticDataManager().getCalibStaticData();
	LAStringVector ccys = MADealUtils::getAllSingleCurrencys();
	lowervec(ccys);
	for (size_t i = 0; i < ccys.size(); ++i)
	{
		LAString model = LAMarketData::getModelName(ccys[i]);
		model.toUpper();
		if (model == MODEL_IRSABR)
		{
			//get underlying
			LAString underlying = calibProp.getStaticData(ccys[i] + STATIC_DATA_KEY_CALIB_IRSABR_UNDERLYING);
			if (underlying == MLIB_NO_DATA) continue;
			underlying.toLower();
			LAStringVector underlyings = underlying.toToken(':');
			//get a flag to calibrate all grids
			bool isAllGridsCalibrate = false;
			LAString strIsAllCalib= calibProp.getStaticData(ccys[i] + STATIC_DATA_KEY_CALIB_IRSABR_ISALLGRIDSCALIBRATE);
			if (strIsAllCalib != MLIB_NO_DATA)
			{
				LADataBool tmpAttrBool;
				tmpAttrBool.convertFromString(strIsAllCalib);
				isAllGridsCalibrate = tmpAttrBool.get();
			}

			for (size_t j = 0; j < underlyings.size(); ++j)
			{
				LAString key = ccys[i] + "." CONTEXT_KEY_DEAL_IRVOL + "." + underlyings[j];
				const LAString calibTarget = LACoreDataService::getContext(key);
				if (calibTarget != MLIB_NO_DATA && !isAllGridsCalibrate)
				{
					continue;
				}
				if (isAllGridsCalibrate)
				{
					LACoreDataService::setContext(key, MLIB_NO_DATA);
				}
				else
				{
					BoolMatrix calibTarget = MADealUtils::getCalibTargetIRVolGrids(objPool, asofDate, ccys[i], underlyings[j]);
					LADataBoolMatrix tmp(calibTarget);
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
LADataInstanceConfigurationPV::loadCSV(LADataInstance &dataInstance) const
{
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	LAString fileName = staticData.getStaticData(KEY_DEAL_FILE);

    // get AttributeMaster
    LAPriceDataManager& dm = dataInstance.getDataMaster();
    // get EntityMaster
    LAObjectMaster& em = dataInstance.getObjectMaster();
	// get EntityPool
	LAObjectPool& objPool = dataInstance.getObjectPool();

	LAString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();

	LAStringMatrix tradeMatrix;
	MAFileAccessor tradeFile(LAMarketData::getNumFileName(fileName, MLIBID));
	tradeFile.readAllData(',',tradeMatrix);
    insertContext(tradeMatrix, dataInstance);
	unsigned int row = 0, col;
	unsigned int size = tradeMatrix.size();	
	if (size>0)
	{
		while(true)
		{
			const LAStringVector &header = tradeMatrix[row];

			// check header
			if (header.empty() || header[0] != "object_t")
			{
				LAString msg("Invalid Format : First item is not object_t [");
				msg += header[0] + " ]";
				throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
			}

			const LAStringVector *befVal = &tradeMatrix[row];
			row++;
			if (tradeMatrix[row].empty())
			{
				break;
			}
			int m_type = tradeMatrix[row][0].getIntValue();
            const LAObjectHolder& objHolder = em.getObject(m_type);
            if (! objHolder.isDefined())
            {
                LAString msg("Invalid Format : Not exist object_t (");
                msg += m_type + ") [ line-";
                msg += LAString((int)row) + "]";
                throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
            }

			LAObject* e = objHolder.get().clone();	
			unsigned int headerSize = header.size();
			while(row<size)
			{
				const LAStringVector &body = tradeMatrix[row];

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
				    LAString msg("Invalid Format : object_t is different from upper line (");
					msg += body[0] + ") [ line-";
					msg += LAString((int)row) + "]";
					throw LACoreInvalidData(msg.getCString(),__FILE__, __LINE__);
				}

                // copy Object
                LAString name("");
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
						LADataHolder* dh = &(e->getData(header[col]));
						if (!dh->isDefined() || dh->getType() == DATA_FUNCTION)
						{
							e->remove(header[col]);
							const LADataHolder& att = dm.getData(header[col]);
							dh = &(e->add(header[col], att));
						}
						dh->convertFromString(body[col]);
						if (header[col] == CALIBRATION_DATA_NAME)
						{
							name = dynamic_cast<LADataString&>(dh->get()).get();
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
	LAMathFXEntity* pFxsde = NULL;
	pFxsde = new LAMathFXEntity(&dataInstance);
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
LADataInstanceConfigurationPV::setUpFXEntity(LADataInstance &dataInstance) const
{
	// get model name for base currency
	LAString baseCurrency = MADealUtils::getSDECurrencys()[0];
	LAStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	LAString mainModel = LAMarketData::getModelName(baseCurrency);
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
LADataInstanceConfigurationPV::setUpPathEntity(LADataInstance &dataInstance) const
{
	// get model name for base currency
	LAString baseCurrency = MADealUtils::getSDECurrencys()[0];
	LAStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	LAString mainModel = LAMarketData::getModelName(baseCurrency);
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
LADataInstanceConfigurationPV::setUpEntityes(LADataInstance &dataInstance) const
{
	// get model name for base currency
	LAString baseCurrency = MADealUtils::getSDECurrencys()[0];
	LAStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

    LAStaticData& staticData = LACoreDataService::getStaticDataManager().getStaticData();
    if(staticData.getStaticData(CONTEXT_KEY_ISINCLUDECASH)=="true"){
        shiftPaymentDate(dataInstance);
    }

	LAString mainModel = LAMarketData::getModelName(baseCurrency);
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
LADataInstanceConfigurationPV::setUpRiskInfo(LADataInstance &dataInstance) const
{
	// get model name for base currency
	LAString baseCurrency = MADealUtils::getSDECurrencys()[0];
	LAStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	LAString mainModel = LAMarketData::getModelName(baseCurrency);
	// setup entities
	LAObjectConfiguration *setUpper = LAObjectConfigurationManager::getInstance()->createEntitySetUpper(mainModel);
	setUpper->setUpRiskInfo(dataInstance);
	// complete depenency
	dataInstance.getReferencePool().completeDependency();
	delete setUpper;
}


void LADataInstanceConfigurationPV::shiftPaymentDate(LAObject& trade, const LADate& asof1, const LADate& asof2) const 
{
	LADataHolder* dh = &(trade.getData(PRICING_DATA_PREMIUMPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LADataDate& data_paydate = dynamic_cast<LADataDate &>(dh->get());
		const LADate paydate = dynamic_cast<LADataDate &>(dh->get()).get();
		if (paydate > asof1 && paydate <= asof2)
		{
			LAPriceDataCalendar cal;
			const LADate shiftDate = LAMathDateCalculations::getDate(asof2, "1D", SLIDING_RULE_FOLLOWING, &cal, true);
			data_paydate.set(shiftDate);
		}
	}

	dh = &(trade.getData(PRICING_DATA_CASHSETTLEMENTPAYMENTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LADataDate& data_paydate = dynamic_cast<LADataDate &>(dh->get());
		const LADate paydate = dynamic_cast<LADataDate &>(dh->get()).get();
		if (paydate > asof1 && paydate <= asof2)
		{
			LAPriceDataCalendar cal;
			const LADate shiftDate = LAMathDateCalculations::getDate(asof2, "1D", SLIDING_RULE_FOLLOWING, &cal, true);
			data_paydate.set(shiftDate);
		}
	}
	
	const LADataValuation& valuemehod = dynamic_cast<const LADataValuation &>(trade.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get());

    if (valuemehod.isTypeOf(FN_IR_PLAINVANILLASWAPTRADEVALUE) || valuemehod.isTypeOf(FN_IR_TRADEVALUE) || valuemehod.isTypeOf(FN_IR_LSMCTRADEVALUE))
    {
        dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, NOCHECK));
        if (dh->isDefined() && !dh->isNull())
        {
            LADataMultiReference &legs = dynamic_cast<LADataMultiReference &>(dh->get());
            unsigned int legSize = legs.getSize();
            for (unsigned int j = 0; j < legSize; j++)
            {
                LAObject &eleg = legs.get(j).get();
                //get cashlet
                dh = &(eleg.getData(PRICING_DATA_CASHLETS, NOCHECK));
                if (dh->isDefined() && !dh->isNull())
                {
                    LADataMultiReference &cashlets = dynamic_cast<LADataMultiReference &>(dh->get());
                    unsigned int cashletSize = cashlets.getSize();
                    for (unsigned int k = 0; k < cashletSize; k++)
                    {
                        LAObject &ecashlet = cashlets.get(k).get();
                        dh = &(ecashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
                        LADataDate& data_paydate = dynamic_cast<LADataDate &>(dh->get());
                        const LADate& paydate = data_paydate.get();
                        if (paydate > asof1 && paydate <= asof2)
                        {
                            LAPriceDataCalendar cal;
                            if((dh = &ecashlet.getData(CALIBRATION_DATA_CALENDAR))->isDefined() && !dh->isNull()){
                                cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
                            }
                            else if((dh = &eleg.getData(CALIBRATION_DATA_CALENDAR))->isDefined() && !dh->isNull()){
                                cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
                            }
                            const LADate shiftDate = LAMathDateCalculations::getDate(asof2, "1D", SLIDING_RULE_FOLLOWING, &cal, true);
                            data_paydate.set(shiftDate);
                        }
                    }
                }
            }
        }
    }
}

LADate LADataInstanceConfigurationPV::getAsOfDate(LADataInstance& dataInstance) const
{
    LAObjectPool& objPool = dataInstance.getObjectPool();
    LAMathPathEntity* path = LAMarketData::getPathEnitty(objPool);
    if (path != NULL){
        return path->getAsOfDate();
    }
    else{
        LAMathPlainVanillaEntity* pvanilla = LAMarketData::getPlainVanillaEntity(objPool);
        if (!pvanilla)
        {
            throw LACoreInvalidData("Neither LAMathPathEntity nor LAMathPlainVanillaEntity exists.", __FILE__, __LINE__);
        }
        return pvanilla->getAsOfDate();
    }
}

void LADataInstanceConfigurationPV::shiftPaymentDate(LADataInstance& dataInstance) const
{
    LAObjectPool& objPool = dataInstance.getObjectPool();

    LAStaticData& staticData = LACoreDataService::getStaticDataManager().getStaticData();
    const LADate asof1 = LADate(staticData.getStaticData(CONTEXT_KEY_INCLUDECASH_FROM).getCString());
    const LADate asof2 = getAsOfDate(dataInstance);


    const LAString mainTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
    LAObjectHolder objHolder = objPool.getObject(mainTradeName, ENCHKTYPE_ISDEFINED);
    if(dynamic_cast<const LADataValuation &>(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE){
        LADataMultiReference &unders = dynamic_cast<LADataMultiReference &>(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
        const unsigned int tradeSize = unders.getSize();
        for (unsigned int i = 0; i < tradeSize; i++){
            shiftPaymentDate(unders.get(i).get(), asof1, asof2);
        }
    }
    else{
        shiftPaymentDate(objHolder.get(), asof1, asof2);
    }
}

void LADataInstanceConfigurationPV::insertContext(LAStringMatrix& m, LADataInstance& dataInstance) const
{
    const LAStringVector* header;
    const bool is_vanilla = LACoreDataService::getContext(ARG_KEY_CALC).toUpper() == "VANILLA";
    const LAString fx_entity_name = is_vanilla ? FORWARDFX : FXSDE;
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
                    const LAString& temp = LACoreDataService::getContext(ARG_KEY_SETTLEDATE);
                    m[i][j] = temp==MLIB_NO_DATA ? LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE) : temp;
                    continue;
                }
                if(header->at(j) == PRICING_DATA_VALUEDATE){
                    const LAString& temp = LACoreDataService::getContext(ARG_KEY_VALUEDATE);
                    m[i][j] = temp==MLIB_NO_DATA ? LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE) : temp;
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

void LADataInstanceConfigurationPV::setupFundingChangeInfo(LADataInstance& dataInstance) const
{
    typedef map<LAString, LAString> FchMap;

    const LAString fname = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(CONTEXT_KEY_FUNDINGCHANGE_FILE);
    if(fname == MLIB_NO_DATA) return;
    FchMap trade_fchinfo;
    MAFileAccessor fch_file(LAMarketData::getNumFileName(fname));
    LAStringMatrix mat; 
    fch_file.readAllData(',', mat);
    for(size_t i = 0; i < mat.size(); i++) trade_fchinfo[mat[i][0]] = mat[i][1];




    LAObjectPool& objPool = dataInstance.getObjectPool();
    const LAString port_name = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
    LADataMultiReference& trades = dynamic_cast<LADataMultiReference&>(objPool.getObject(port_name, ENCHKTYPE_ISDEFINED).getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());
    for(size_t i = 0; i < trades.getSize(); i++){
        LAObjectHolder& objHolder = trades.get(i);
        const LAString name = dynamic_cast<const LADataString&>(objHolder.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
        FchMap::const_iterator it = trade_fchinfo.find(name);
        if(it==trade_fchinfo.end()) continue;
        objHolder.remove(PRICING_DATA_FUNDINGCHANGEINFO);
        objHolder.add(PRICING_DATA_FUNDINGCHANGEINFO, new LADataReference()).convertFromString(it->second);
    }
}

// 
/*!
    @brief setup risk object

	@param[out] dataInstance
*/
void LADataInstanceConfigurationPV::setUpRiskEntityes(LADataInstance& dataInstance) const
{
	dataInstance;
	// do nothing
}