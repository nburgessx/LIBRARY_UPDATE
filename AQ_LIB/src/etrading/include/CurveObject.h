#pragma once

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CurveGenerator.h"
#include "CurveMarketData.h"

namespace etrading
{
	// String constants used only in this file
	extern const std::string CURVE_BUILD_SCHEMA_NAME;
	extern const std::string DATA_NAMES;
	extern const std::string DATA_VALUES;
	extern const std::string OBJECT_NAME;
	extern const std::string CURVE_GENERATOR_NAME;
	extern const std::string CURVE_MARKET_CALIBRATION_DATAAME;
	extern const std::string DOMESTIC_CURVE_COLLECTION;
	extern const std::string FOREIGN_CURVE_COLLECTION;
	extern const std::string CURVE_COUNT;

	/* 
		@brief Base class for all curve objects
	*/
    class CurveObject: public IsLWOObject
    {
	public:
		
		CurveObject( const std::string& objectName);
		
		/* @brief Copy Constructor
		*/
   		CurveObject(const CurveObject& rhs);

		CurveObject()  : IsLWOObject("", CURVE) {}

		virtual ~CurveObject() {}

		/* @brief Used to serialize an instance of this class, including the CurveGenerator and CurveMarketData objects this curve depends on
		*  @param[out] the populated SchemaObject
		*/
		virtual const SchemaObject toSchemaObject() const = 0;

		/* @brief	Function to identify the curve type i.e. SINGLE_CURVE, MULTI_CURVE, DUAL_CURVE
		*/
		virtual CurveCalibrationTypeEnum curveCalibrationType() const = 0;

		/* @brief	The main function which builds an object-pool curve from the specified configuration parameters
		*/
		virtual void calibrateCurve() = 0;

	};


	/* 
		@brief	Structure that keeps curve data of all types that are fed into 
				the respective curve calibration method
	*/
	struct CurveObjectData
	{
        // Constructor and ***Virtual*** Destructor
        CurveObjectData() {}
        virtual ~CurveObjectData() {}
        
        virtual etrading::CurveTypeEnum getCurveType() { return NONE_CURVETYPE;  }

		AQLString curveName_;
		AQLString curveIndex_;

        // Label Value Block Conventions for Lookups
		LabelValueBlock curveConvLVB_;
        
        // Risk Parameters
        double shiftSize_;
		CurveMarketDataEnum riskInstrumentType_;
		size_t nthRiskInstrument_;

        // Virtual Market Data Flat-Shift Methods
        // --------------------------------------

        // Function to apply a flat-shift to outright instruments only
        virtual void applyFlatShift( const double & shiftSize ) = 0;
        
        // Function to remove a flat-shift a clear an existing flat-shift
        virtual void restoreDataAndClearFlatShift() = 0;

		// Function to perturb the nth market data instrument
		virtual void perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize ) = 0;

		// Function to restore market data and clear perturbation shifts
		virtual void restoreDataAndClearPerturbation() = 0;
	};

	// Shared pointer type declaration
	typedef std::shared_ptr<CurveObjectData> CurveObjectDataPtr;


	/* @brief	OIS curve
	*/
	struct OISCurveObjectData : public CurveObjectData
	{
		OISCurveObjectData() {}
		OISCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& marketDataAsOfDate, bool useSwaps = true, const std::string& curveNameIn = "", const std::string& curveIndexIn = "");
		
		virtual etrading::CurveTypeEnum getCurveType() { return OIS_CURVETYPE; }

        // Use Label Value Block for Conventions for Lookups
        LabelValueBlock oisConvLVB_;
        LabelValueBlock loBasisConvLVB_;
        LabelValueBlock swapConvLVB_;

        // String Matrix Data
		AQLStringMatrix oisRates_;
		AQLStringMatrix histRates_;
		AQLStringMatrix loBasisRates_;
		AQLStringMatrix swapRates_;
		StandardStringMatrix forwardAdjustments_;

        // Function to apply a flat-shift to outright instruments only
        void applyFlatShift( const double & shiftSize );
        
		// Function to remove a flat-shift a clear an existing flat-shift
        void restoreDataAndClearFlatShift();

		// Function to perturb the nth market data instrument
		void perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize );

		// Function to restore market data and clear perturbation shifts
		void restoreDataAndClearPerturbation();

		// Function to get the peturbed instrument tenors
		StandardStringVector instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const;

		// Function to identify which instruments are outright instruments to be included in risk totals
		std::vector<bool> isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const;
	};

	/* @brief	ARR curve
	*/
	struct ARRCurveObjectData : public CurveObjectData
	{
		ARRCurveObjectData() {}
		ARRCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& marketDataAsOfDate, bool useSwaps = true, const std::string& curveNameIn = "", const std::string& curveIndexIn = "");

		virtual etrading::CurveTypeEnum getCurveType() { return ARR_CURVETYPE; }

        // Use Label Value Block for Conventions for Lookups
        LabelValueBlock oisConvLVB_;
        LabelValueBlock loBasisConvLVB_;
        LabelValueBlock swapConvLVB_;

        // String Matrix Data
		AQLStringMatrix oisRates_;
		AQLStringMatrix histRates_;
		AQLStringMatrix loBasisRates_;
		AQLStringMatrix swapRates_;
		StandardStringMatrix forwardAdjustments_;

        // Function to apply a flat-shift to outright instruments only
        void applyFlatShift( const double & shiftSize );
        
        // Function to remove a flat-shift a clear an existing flat-shift
        void restoreDataAndClearFlatShift();

		// Function to perturb the nth market data instrument
		void perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize );
		
		// Function to restore market data and clear perturbation shifts
		void restoreDataAndClearPerturbation();

		// Function to get the peturbed instrument tenors
		StandardStringVector instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const;

		// Function to identify which instruments are outright instruments to be included in risk totals
		std::vector<bool> isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const;
	};

	/* @brief	swap curve
	*/
	struct SwapCurveObjectData : public CurveObjectData
	{
		SwapCurveObjectData() {}
		SwapCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& frequency, const std::string& marketDataAsOfDate, const std::string& curveNameIn = "", const std::string& curveIndexIn = "");

		virtual etrading::CurveTypeEnum getCurveType() { return SWAP_CURVETYPE; }

        // Use Label Value Block for Conventions for Lookups
		LabelValueBlock moneyMarketConvLVB_;
		LabelValueBlock liborConvLVB_;
        LabelValueBlock swapConvLVB_;
        LabelValueBlock fraConvLVB_;
        LabelValueBlock futureConvLVB_;
        LabelValueBlock basisAdjConvLVB_;

        // String Matrix Data
        AQLStringMatrix liborRates_;
		AQLStringMatrix swapRates_;
		AQLStringMatrix fraRates_;
		AQLStringMatrix fra3mRates_;
		AQLStringMatrix fra6mRates_;
		AQLStringMatrix futureRates_;
		AQLStringMatrix basisAdjRates_;
		StandardStringMatrix forwardAdjustments_;

        // Function to apply a flat-shift to outright instruments only
        void applyFlatShift( const double & shiftSize );
        
        // Function to remove a flat-shift a clear an existing flat-shift
        void restoreDataAndClearFlatShift();

		// Function to perturb the nth market data instrument
		void perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize );

		// Function to get the peturbed instrument tenors
		StandardStringVector instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const;

		// Function to identify which instruments are outright instruments to be included in risk totals
		std::vector<bool> isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const;

		// Function to restore market data and clear perturbation shifts
		void restoreDataAndClearPerturbation();
	};

	/* @brief	tenor basis curve
	*/
	struct TenorBasisCurveObjectData : public CurveObjectData
	{
		TenorBasisCurveObjectData() {}
		TenorBasisCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string frequency, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn = "", const std::string& curveIndexIn = "");

		virtual etrading::CurveTypeEnum getCurveType() { return TENORBASIS_CURVETYPE; }

        // Use Label Value Block for Conventions for Lookups
        LabelValueBlock basisConvLVB_;
        LabelValueBlock fxFwdConvLVB_;
        LabelValueBlock fraConvLVB_;
        LabelValueBlock liborConvLVB_;
        LabelValueBlock moneyMarketConvLVB_;

        // String Matrix Data
		AQLStringMatrix basisRates_;
		AQLStringMatrix fxFwdRates_;
		AQLStringMatrix spotFxRates_;
		AQLStringMatrix fraRates_;
		AQLStringMatrix liborRates_;
		StandardStringMatrix forwardAdjustments_;

        // Function to apply a flat-shift to outright instruments only
        void applyFlatShift( const double & shiftSize );
        
        // Function to remove a flat-shift a clear an existing flat-shift
        void restoreDataAndClearFlatShift();

		// Function to perturb the nth market data instrument
		void perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize );

		// Function to restore market data and clear perturbation shifts
		void restoreDataAndClearPerturbation();

		// Function to get the peturbed instrument tenors
		StandardStringVector instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const;

		// Function to identify which instruments are outright instruments to be included in risk totals
		std::vector<bool> isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const;
	};

	/* @brief	Xccy basis curve
	*/
	struct XccyBasisCurveObjectData : public CurveObjectData
	{
		XccyBasisCurveObjectData() {}
		XccyBasisCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn = "", const std::string& curveIndexIn = "");

		virtual etrading::CurveTypeEnum getCurveType() { return XCCYBASIS_CURVETYPE; }

        // Use Label Value Block for Conventions for Lookups
        LabelValueBlock basisConvLVB_;
        LabelValueBlock fxFwdConvLVB_;
        LabelValueBlock moneyMarketConvLVB_;
        LabelValueBlock fraConvLVB_;

        // String Matrix Data
		AQLStringMatrix basisRates_;
		AQLStringMatrix fxFwdRates_;
		AQLStringMatrix spotFxRates_;
		AQLStringMatrix fraRates_;
		StandardStringMatrix forwardAdjustments_;

        // Function to apply a flat-shift to outright instruments only
        void applyFlatShift( const double & shiftSize );
        
        // Function to remove a flat-shift a clear an existing flat-shift
        void restoreDataAndClearFlatShift();

		// Function to perturb the nth market data instrument
		void perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize );
		
		// Function to restore market data and clear perturbation shifts
		void restoreDataAndClearPerturbation();

		// Function to get the peturbed instrument tenors
		StandardStringVector instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const;

		// Function to identify which instruments are outright instruments to be included in risk totals
		std::vector<bool> isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const;
	};

	/* @brief	Forward constant curve
	*/
	struct FwdConstantCurveObjectData : public CurveObjectData
	{
		FwdConstantCurveObjectData() {}
		FwdConstantCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection,  const std::string& curveNameIn = "", const std::string& curveIndexIn = "");
		
		virtual etrading::CurveTypeEnum getCurveType() { return FWDFXCONST_CURVETYPE; }

        // Use Label Value Block for Conventions for Lookups
		LabelValueBlock fwdfxconstConvLVB_;
		StandardStringMatrix forwardAdjustments_;

        // Function to apply a flat-shift to outright instruments only
        void applyFlatShift( const double & shiftSize );
        
        // Function to remove a flat-shift a clear an existing flat-shift
        void restoreDataAndClearFlatShift();

		// Function to perturb the nth market data instrument
		void perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize );

		// Function to restore market data and clear perturbation shifts
		void restoreDataAndClearPerturbation();
	};
}
