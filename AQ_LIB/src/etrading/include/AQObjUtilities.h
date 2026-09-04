#pragma once

#include "Swap.h"
#include "Schedule.h"
#include "SwapGenerator.h"
#include "FixingTable.h"

#include "CurveGenerator.h"
#include "CurveMarketData.h"

#include "Bond.h"
#include "BondGenerator.h"
#include "FXCurve.h"
#include "Option.h"
#include "Volatility.h"
#include "CreditModel.h"
#include "CreditBasketModel.h"
#include "BondCurves.h"
#include "SingleCurveObject.h"
#include "MultiCurveObject.h"
#include "DualBootstrappedCurveObject.h"
#include "InflationCurve.h"
#include "SabrModel.h"
#include "SabrMarketData.h"

namespace etrading
{
	/*
	For some strange VS2010 specific reason std::vector members on moved or copied instances 
	of classes sometimes give potential CPU allocated sizes instead of actual sizes unless they 
	go through a template ??! Not a problem in VS2015	
	*/
	/*
	template <typename S>
    int getVectorSize( const std::vector<S>& inputColumnData )
	{ return inputColumnData.size(); }
	*/

	SwapPtr getSwap( const std::string& objectName );
	SchedulePtr getSchedule( const std::string& objectName );
    LegPtr getLeg( const std::string& objectName );
    BondPtr getBond( const std::string& objectName );
	CreditModelPtr getCreditModel( const std::string& objectName, bool throwIfMissing=true );
	CreditBasketModelPtr getCreditBasketModel( const std::string& objectName, bool throwIfMissing=true );
	BondCurvePtr getBondCurve( const std::string& objectName, bool throwIfMissing=true );
	InflationCurvePtr getInflationCurve( const std::string& objectName, bool throwIfMissing = true );
	SabrModelPtr getSabrModel(const std::string& objectName, bool throwIfMissing = true);
	SabrMarketDataPtr getSabrMarketData(const std::string& objectName, bool throwIfMissing = true);

    bool doesAQObjExist( const std::string& objectName, const etrading::CachedObjectEnum& objectTypeEnum );
    bool doesAQObjExist( const std::string& objectName, const std::string& objectTypeAsString );
    
    SwapGeneratorPtr getSwapGenerator( const std::string& objectName );
    FixingTablePtr getFixingTable( const std::string& objectName, bool throwIfMissing = true);

	CurveGeneratorPtr getCurveGenerator( const std::string& objectName );
	CurveMarketDataPtr getCurveMarketData( const std::string& objectName );
	
    FXCurvePtr getFxCurve( const std::string& objectName );

	BondGeneratorPtr getBondGenerator( const std::string& objectName );

	OptionPtr getOption(const std::string& objectName);

	VolatilityPtr getVolatility(const std::string& objectName, bool throwIfMissing = true);

	std::shared_ptr<FreeObject> getFreeObject(const std::string& objectName);

	// Curve Object Methods
    SingleCurveObjectPtr getSingleCurveObject( const std::string& objectName, const bool throwIfMissing = true );
	MultiCurveObjectPtr getMultiCurveObject( const std::string& objectName, const bool throwIfMissing = true );
	DualBootstrappedCurveObjectPtr getDualCurveObject( const std::string& objectName, const bool throwIfMissing = true );

	// Handle Methods
	// ---------------------
	std::string undecorateHandle( const std::string & handle );

	// Method to Get a Curve Collection from a Curve Handle or Curve Collection
	// We can optionally throw if the input is a curveCollection string
	// Object methods accept handles only, but we need to be backward compatible for legacy methods which require curve collection strings
	std::string getCurveCollectionFromHandle( const std::string & curveName, const bool allowCurveCollections = true );
};

