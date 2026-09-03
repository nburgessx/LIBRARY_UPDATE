//
// LAUpdateStaticDataManager.h
// This file was previously called YieldCurveFactory.h and before that AQLCurveSetup.h
//
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <AQLDate.h>
#include <AQLCoreAppError.h>
#include <AQLCoreTemplateType.h>
#include <AQLDataInstance.h>
#include <AQLString.h>
#include "CurveObject.h"

// Forward Declarations
class AQLStaticData;

namespace etrading
{
    class LAUpdateStaticDataManager
    {
    public:

		//==============================================================================
        static void SetUpArbFreeCurve(AQLDataInstance* dataInstance,
                                      const AQLString& curveID,
                                      const AQLStringMatrix& generateProp,
                                      const AQLStringMatrix& moneyConv,
                                      const AQLStringMatrix& liborRates,
                                      const AQLStringMatrix& liborConv,
                                      const AQLStringMatrix& swapRates,
                                      const AQLStringMatrix& swapConv,
                                      const AQLStringMatrix& fra3mRates,
                                      const AQLStringMatrix& fra6mRates,
                                      const AQLStringMatrix& fraConv,
                                      const AQLStringMatrix& xccyBasisRates,
                                      const AQLStringMatrix& xccyBasisConv,
                                      const AQLStringMatrix& threeSixRates,
                                      const AQLStringMatrix& threeSixConv,
                                      const AQLStringMatrix& futureRates,
                                      const AQLStringMatrix& futureConv,
                                      const AQLStringMatrix& adjustData,
                                      const AQLString& curveNames_3ML,
                                      const AQLString& curveNames_6ML,
                                      const AQLString& curveNames_DF,
                                      const AQLString& curveName_DF2);

		//==============================================================================
        static void setUpFloater(AQLDataInstance* dataInstance,
                                 const AQLString& curveID,
                                 const AQLString& discountCurveName,
                                 const AQLString& forecastCurveName,
                                 const AQLStringMatrix& generateProp,
                                 const AQLStringMatrix& basisMkt,
                                 const AQLStringMatrix& basisConv,
                                 const AQLStringMatrix& swapConv,
                                 const AQLStringMatrix& adjustData);

		//==============================================================================
        static void loadStaticDataOISCurve( AQLDataInstance* dataInstance,
											const AQLString& curveID,
											const AQLString& marketName,
											const AQLStringMatrix& generateProp,
											const AQLStringMatrix& oisRates,
											const AQLStringMatrix& oisConv,
											const AQLString& curveNames,
											const AQLStringMatrix& histRates,
											const AQLStringMatrix& lobasisRates = AQLStringMatrix(),
											const AQLStringMatrix& lobasisConv = AQLStringMatrix(),
											const AQLStringMatrix& swapRates = AQLStringMatrix(),
											const AQLStringMatrix& swapConv = AQLStringMatrix() );
		
		static void calibrateOISCurve( AQLDataInstance* dataInstance,
									   const AQLString& curveID,
									   const AQLString& marketName,
									   const AQLStringMatrix& generateProp );

		static void setUpOISCurve(AQLDataInstance* dataInstance,
                                  const AQLString& curveID,
                                  const AQLString& marketName,
                                  const AQLStringMatrix& generateProp,
                                  const AQLStringMatrix& oisRates,
                                  const AQLStringMatrix& oisConv,
                                  const AQLString& curveNames,
                                  const AQLStringMatrix& histRates,
                                  const AQLStringMatrix& lobasisRates = AQLStringMatrix(),
                                  const AQLStringMatrix& lobasisConv = AQLStringMatrix(),
                                  const AQLStringMatrix& swapRates = AQLStringMatrix(),
                                  const AQLStringMatrix& swapConv = AQLStringMatrix());

		

		//==============================================================================
		static void loadStaticDataSwapCurve(AQLDataInstance* dataInstance,
											const AQLString& curveID,
											const AQLString& marketName,
											const AQLStringMatrix& generateProp,
											const AQLStringMatrix& moneyConv,
											const AQLStringMatrix& liborRates,
											const AQLStringMatrix& liborConv,
											const AQLStringMatrix& swapRates,
											const AQLStringMatrix& swapConv,
											const AQLStringMatrix& fra3mRates,
											const AQLStringMatrix& fra6mRates,
											const AQLStringMatrix& fraConv,
											const AQLStringMatrix& futureRates,
											const AQLStringMatrix& futureConv,
											const AQLStringMatrix& adjustSwapConv,
											const AQLStringMatrix& adjustSwapRates,
											const AQLString& curveNames,
											const AQLString& curveName_DF2);

		static void calibrateSwapCurve( AQLDataInstance* dataInstance,
									    const AQLString& curveID,
									    const AQLString& marketName,
									    const AQLStringMatrix& generateProp );

		static void setUpSwapCurve(AQLDataInstance* dataInstance,
                                   const AQLString& curveID,
                                   const AQLString& marketName,
                                   const AQLStringMatrix& generateProp,
                                   const AQLStringMatrix& moneyConv,
                                   const AQLStringMatrix& liborRates,
                                   const AQLStringMatrix& liborConv,
                                   const AQLStringMatrix& swapRates,
                                   const AQLStringMatrix& swapConv,
                                   const AQLStringMatrix& fra3mRates,
                                   const AQLStringMatrix& fra6mRates,
                                   const AQLStringMatrix& fraConv,
                                   const AQLStringMatrix& futureRates,
                                   const AQLStringMatrix& futureConv,
                                   const AQLStringMatrix& adjustSwapConv,
                                   const AQLStringMatrix& adjustSwapRates,
                                   const AQLString& curveNames,
                                   const AQLString& curveName_DF2);

        //==============================================================================
		static void loadStaticDataBasisCurve(AQLDataInstance* dataInstance,
                                             const AQLString& curveID,
                                             const AQLString& marketName,
                                             const AQLStringMatrix& basisRates,
                                             const AQLStringMatrix& basisConv,
                                             const AQLStringMatrix& fwdFXs,
                                             const AQLStringMatrix& fwdConv,
                                             const AQLStringMatrix& spotFXs,
                                             const AQLStringMatrix& generateProp,
                                             const AQLStringMatrix& moneyConv,
                                             const AQLString& curveNames,
                                             const AQLStringMatrix& fraConv = AQLStringMatrix(),
                                             const AQLStringMatrix& fraRates = AQLStringMatrix(),
                                             const AQLStringMatrix& liborConv = AQLStringMatrix(),
                                             const AQLStringMatrix& liborRates = AQLStringMatrix());

		static void calibrateBasisCurve( AQLDataInstance* dataInstance,
									     const AQLString& curveID,
									     const AQLString& marketName,
									     const AQLStringMatrix& generateProp );

		static void setUpBasisCurve(AQLDataInstance* dataInstance,
                                    const AQLString& curveID,
                                    const AQLString& marketName,
                                    const AQLStringMatrix& basisRates,
                                    const AQLStringMatrix& basisConv,
                                    const AQLStringMatrix& fwdFXs,
                                    const AQLStringMatrix& fwdConv,
                                    const AQLStringMatrix& spotFXs,
                                    const AQLStringMatrix& generateProp,
                                    const AQLStringMatrix& moneyConv,
                                    const AQLString& curveNames,
                                    const AQLStringMatrix& fraConv = AQLStringMatrix(),
                                    const AQLStringMatrix& fraRates = AQLStringMatrix(),
                                    const AQLStringMatrix& liborConv = AQLStringMatrix(),
                                    const AQLStringMatrix& liborRates = AQLStringMatrix());

        //==============================================================================
		static void loadStaticDataFwdFXConstantCurve(AQLDataInstance* dataInstance,
                                                     const AQLString& curveID,
                                                     const AQLString& marketName,
                                                     const AQLStringMatrix& fwdfxconstConv,
                                                     const AQLStringMatrix& generateProp,
                                                     const AQLString& curveNames);
		
		static void calibrateFwdFXConstantCurve( AQLDataInstance* dataInstance,
									             const AQLString& curveID,
									             const AQLString& marketName,
									             const AQLStringMatrix& generateProp );
		
		static void setUpFwdFXConstantCurve(AQLDataInstance* dataInstance,
                                            const AQLString& curveID,
                                            const AQLString& marketName,
                                            const AQLStringMatrix& fwdfxconstConv,
                                            const AQLStringMatrix& generateProp,
                                            const AQLString& curveNames);
        
		//==============================================================================
        static void setUpDualBootstrapCurve(AQLDataInstance* dataInstance,
                                            const AQLString& curveID,
                                            const AQLString& curveName_db,
                                            const AQLString& curveName_OIS,
                                            const AQLString& curveName_swap,
                                            const AQLString& curveNames_OIS,
                                            const AQLString& curveNames_swap,
                                            const AQLStringMatrix& commonParams,
                                            const AQLStringMatrix& generateProp_OIS,
                                            const AQLStringMatrix& oisRates_OIS,
                                            const AQLStringMatrix& oisConv_OIS,
                                            const AQLStringMatrix& histRates_OIS,
                                            const AQLStringMatrix& lobasisRates_OIS,
                                            const AQLStringMatrix& lobasisConv_OIS,
                                            const AQLStringMatrix& swapConv_OIS,
                                            const AQLStringMatrix& generateProp_swap,
                                            const AQLStringMatrix& moneyConv_swap,
                                            const AQLStringMatrix& liborRates_swap,
                                            const AQLStringMatrix& liborConv_swap,
                                            const AQLStringMatrix& swapRates_swap,
                                            const AQLStringMatrix& swapConv_swap,
                                            const AQLStringMatrix& fra3mRates_swap,
                                            const AQLStringMatrix& fra6mRates_swap,
                                            const AQLStringMatrix& fraConv_swap,
                                            const AQLStringMatrix& futureRates_swap,
                                            const AQLStringMatrix& futureConv_swap,
                                            const AQLStringMatrix& adjustSwapConv_swap,
                                            const AQLStringMatrix& adjustSwapRates_swap
        );
        //==============================================================================

		static void setUpGlobalEngineCurves(AQLDataInstance* dataInstance,
                                            const AQLString& engineName,
                                            const AQLString& curveCollectionID,
                                            const AQLStringMatrix& commonParams,
                                            const std::vector<etrading::CurveObjectDataPtr>& curveDataCollection);

        //==============================================================================
        static void setUpCheapestToDeliverCurve(AQLDataInstance* dataInstance,
                                                const AQLString& curveCollection,
                                                const AQLString& curveName,
                                                const AQLString& curveIndex,
                                                const AQLStringMatrix& generateProp,
                                                const AQLStringVector& collateralCurves);

        //==============================================================================
        static void setUpForIRServer(void);
        //==============================================================================
        
        static void setUpDefaultIRStaticData(AQLDataInstance& dataInstance);
        static void setUpDefaultIRStaticData(AQLDataInstance& dataInstance, const AQLString& filePath);
        static void setUpDefaultCalibStaticData(AQLDataInstance& dataInstance);
        static void setUpDefaultCalibStaticData(AQLDataInstance& dataInstance, AQLString filePath);
        
        //==============================================================================
        static void resetService();
        //==============================================================================
        
        static void SetUpFundingSpread(AQLDataInstance* dataInstance,const AQLStringMatrix &fundingSpread);

        static void CalcMeanAndCovariance(const DoubleMatrix& data, DoubleArray& mean, DoubleMatrix& covar);

        static void SetUpPCA(AQLDataInstance* dataInstance, const DoubleMatrix& corr, const size_t no_factors, const AQLString& id);

        static DoubleMatrix GetPCAResult(AQLDataInstance* dataInstance, const AQLString& type, const AQLString& id);

        static void populateStaticDataManagerForSwapCurve(AQLStaticData &irprop,
                                                          const AQLString& useMarkets,
                                                          const AQLString& currency,
                                                          const AQLString& curveNames_swap,
                                                          const AQLString& marketName_swap,
                                                          const AQLString& generateCurveName_swap,
                                                          const AQLStringMatrix& generateProp_swap,
                                                          const AQLStringMatrix& moneyConv_swap,
                                                          const AQLStringMatrix& liborRates_swap,
                                                          const AQLStringMatrix& liborConv_swap,
                                                          const AQLStringMatrix& swapRates_swap,
                                                          const AQLStringMatrix& swapConv_swap,
                                                          const AQLStringMatrix& fra3mRates_swap,
                                                          const AQLStringMatrix& fra6mRates_swap,
                                                          const AQLStringMatrix& fraConv_swap,
                                                          const AQLStringMatrix& futureRates_swap,
                                                          const AQLStringMatrix& futureConv_swap,
                                                          const AQLStringMatrix& adjustSwapConv_swap,
                                                          const AQLStringMatrix& adjustSwapRates_swap);

        static void populateStaticDataManagerForOISCurve(AQLStaticData &irprop,
                                                         const AQLString& useMarkets,
                                                         const AQLString& currency,
                                                         const AQLString& curveNames_OIS,
                                                         const AQLString& marketName_OIS,
                                                         const AQLString& generateCurveName_OIS,
                                                         const AQLStringMatrix& generateProp_OIS,
                                                         const AQLStringMatrix& oisRates_OIS,
                                                         const AQLStringMatrix& oisConv_OIS,
                                                         const AQLStringMatrix& histRates_OIS,
                                                         const AQLStringMatrix& lobasisRates_OIS,
                                                         const AQLStringMatrix& lobasisConv_OIS,
                                                         const AQLStringMatrix& swapConv_OIS);

        static void populateStaticDataManagerForTenorBasisCurve(AQLStaticData &irprop,
                                                          const AQLString& curveName,
                                                          const AQLString& curveNames,
                                                          const AQLString& currency,
                                                          const AQLStringMatrix& basisRates,
                                                          const AQLStringMatrix& basisConv,
                                                          const AQLStringMatrix& fwdFXs,
                                                          const AQLStringMatrix& fwdConv,
                                                          const AQLStringMatrix& spotFXs,
                                                          const AQLStringMatrix& generateProp,
                                                          const AQLStringMatrix& moneyConv,
                                                          const AQLStringMatrix& fraConv,
                                                          const AQLStringMatrix& fraRates,
                                                          const AQLStringMatrix& liborConv,
                                                          const AQLStringMatrix& liborRates);

    protected:
        static std::istringstream* createFutureStream(const AQLStringMatrix& future_rates, AQLString& usegrid_future);
        static void setStaticDataValue(AQLStaticData &prop, const AQLString &key, const AQLString &val, const bool is_override = false);
    };
}