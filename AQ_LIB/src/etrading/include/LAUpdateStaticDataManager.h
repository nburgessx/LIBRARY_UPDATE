//
// LAUpdateStaticDataManager.h
// This file was previously called YieldCurveFactory.h and before that LACurveSetup.h
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

#include <LADate.h>
#include <LACoreAppError.h>
#include <LACoreTemplateType.h>
#include <LADataInstance.h>
#include <LAString.h>
#include "CurveObject.h"

// Forward Declarations
class LAStaticData;

namespace etrading
{
    class LAUpdateStaticDataManager
    {
    public:

		//==============================================================================
        static void SetUpArbFreeCurve(LADataInstance* dataInstance,
                                      const LAString& curveID,
                                      const LAStringMatrix& generateProp,
                                      const LAStringMatrix& moneyConv,
                                      const LAStringMatrix& liborRates,
                                      const LAStringMatrix& liborConv,
                                      const LAStringMatrix& swapRates,
                                      const LAStringMatrix& swapConv,
                                      const LAStringMatrix& fra3mRates,
                                      const LAStringMatrix& fra6mRates,
                                      const LAStringMatrix& fraConv,
                                      const LAStringMatrix& xccyBasisRates,
                                      const LAStringMatrix& xccyBasisConv,
                                      const LAStringMatrix& threeSixRates,
                                      const LAStringMatrix& threeSixConv,
                                      const LAStringMatrix& futureRates,
                                      const LAStringMatrix& futureConv,
                                      const LAStringMatrix& adjustData,
                                      const LAString& curveNames_3ML,
                                      const LAString& curveNames_6ML,
                                      const LAString& curveNames_DF,
                                      const LAString& curveName_DF2);

		//==============================================================================
        static void setUpFloater(LADataInstance* dataInstance,
                                 const LAString& curveID,
                                 const LAString& discountCurveName,
                                 const LAString& forecastCurveName,
                                 const LAStringMatrix& generateProp,
                                 const LAStringMatrix& basisMkt,
                                 const LAStringMatrix& basisConv,
                                 const LAStringMatrix& swapConv,
                                 const LAStringMatrix& adjustData);

		//==============================================================================
        static void loadStaticDataOISCurve( LADataInstance* dataInstance,
											const LAString& curveID,
											const LAString& marketName,
											const LAStringMatrix& generateProp,
											const LAStringMatrix& oisRates,
											const LAStringMatrix& oisConv,
											const LAString& curveNames,
											const LAStringMatrix& histRates,
											const LAStringMatrix& lobasisRates = LAStringMatrix(),
											const LAStringMatrix& lobasisConv = LAStringMatrix(),
											const LAStringMatrix& swapRates = LAStringMatrix(),
											const LAStringMatrix& swapConv = LAStringMatrix() );
		
		static void calibrateOISCurve( LADataInstance* dataInstance,
									   const LAString& curveID,
									   const LAString& marketName,
									   const LAStringMatrix& generateProp );

		static void setUpOISCurve(LADataInstance* dataInstance,
                                  const LAString& curveID,
                                  const LAString& marketName,
                                  const LAStringMatrix& generateProp,
                                  const LAStringMatrix& oisRates,
                                  const LAStringMatrix& oisConv,
                                  const LAString& curveNames,
                                  const LAStringMatrix& histRates,
                                  const LAStringMatrix& lobasisRates = LAStringMatrix(),
                                  const LAStringMatrix& lobasisConv = LAStringMatrix(),
                                  const LAStringMatrix& swapRates = LAStringMatrix(),
                                  const LAStringMatrix& swapConv = LAStringMatrix());

		

		//==============================================================================
		static void loadStaticDataSwapCurve(LADataInstance* dataInstance,
											const LAString& curveID,
											const LAString& marketName,
											const LAStringMatrix& generateProp,
											const LAStringMatrix& moneyConv,
											const LAStringMatrix& liborRates,
											const LAStringMatrix& liborConv,
											const LAStringMatrix& swapRates,
											const LAStringMatrix& swapConv,
											const LAStringMatrix& fra3mRates,
											const LAStringMatrix& fra6mRates,
											const LAStringMatrix& fraConv,
											const LAStringMatrix& futureRates,
											const LAStringMatrix& futureConv,
											const LAStringMatrix& adjustSwapConv,
											const LAStringMatrix& adjustSwapRates,
											const LAString& curveNames,
											const LAString& curveName_DF2);

		static void calibrateSwapCurve( LADataInstance* dataInstance,
									    const LAString& curveID,
									    const LAString& marketName,
									    const LAStringMatrix& generateProp );

		static void setUpSwapCurve(LADataInstance* dataInstance,
                                   const LAString& curveID,
                                   const LAString& marketName,
                                   const LAStringMatrix& generateProp,
                                   const LAStringMatrix& moneyConv,
                                   const LAStringMatrix& liborRates,
                                   const LAStringMatrix& liborConv,
                                   const LAStringMatrix& swapRates,
                                   const LAStringMatrix& swapConv,
                                   const LAStringMatrix& fra3mRates,
                                   const LAStringMatrix& fra6mRates,
                                   const LAStringMatrix& fraConv,
                                   const LAStringMatrix& futureRates,
                                   const LAStringMatrix& futureConv,
                                   const LAStringMatrix& adjustSwapConv,
                                   const LAStringMatrix& adjustSwapRates,
                                   const LAString& curveNames,
                                   const LAString& curveName_DF2);

        //==============================================================================
		static void loadStaticDataBasisCurve(LADataInstance* dataInstance,
                                             const LAString& curveID,
                                             const LAString& marketName,
                                             const LAStringMatrix& basisRates,
                                             const LAStringMatrix& basisConv,
                                             const LAStringMatrix& fwdFXs,
                                             const LAStringMatrix& fwdConv,
                                             const LAStringMatrix& spotFXs,
                                             const LAStringMatrix& generateProp,
                                             const LAStringMatrix& moneyConv,
                                             const LAString& curveNames,
                                             const LAStringMatrix& fraConv = LAStringMatrix(),
                                             const LAStringMatrix& fraRates = LAStringMatrix(),
                                             const LAStringMatrix& liborConv = LAStringMatrix(),
                                             const LAStringMatrix& liborRates = LAStringMatrix());

		static void calibrateBasisCurve( LADataInstance* dataInstance,
									     const LAString& curveID,
									     const LAString& marketName,
									     const LAStringMatrix& generateProp );

		static void setUpBasisCurve(LADataInstance* dataInstance,
                                    const LAString& curveID,
                                    const LAString& marketName,
                                    const LAStringMatrix& basisRates,
                                    const LAStringMatrix& basisConv,
                                    const LAStringMatrix& fwdFXs,
                                    const LAStringMatrix& fwdConv,
                                    const LAStringMatrix& spotFXs,
                                    const LAStringMatrix& generateProp,
                                    const LAStringMatrix& moneyConv,
                                    const LAString& curveNames,
                                    const LAStringMatrix& fraConv = LAStringMatrix(),
                                    const LAStringMatrix& fraRates = LAStringMatrix(),
                                    const LAStringMatrix& liborConv = LAStringMatrix(),
                                    const LAStringMatrix& liborRates = LAStringMatrix());

        //==============================================================================
		static void loadStaticDataFwdFXConstantCurve(LADataInstance* dataInstance,
                                                     const LAString& curveID,
                                                     const LAString& marketName,
                                                     const LAStringMatrix& fwdfxconstConv,
                                                     const LAStringMatrix& generateProp,
                                                     const LAString& curveNames);
		
		static void calibrateFwdFXConstantCurve( LADataInstance* dataInstance,
									             const LAString& curveID,
									             const LAString& marketName,
									             const LAStringMatrix& generateProp );
		
		static void setUpFwdFXConstantCurve(LADataInstance* dataInstance,
                                            const LAString& curveID,
                                            const LAString& marketName,
                                            const LAStringMatrix& fwdfxconstConv,
                                            const LAStringMatrix& generateProp,
                                            const LAString& curveNames);
        
		//==============================================================================
        static void setUpDualBootstrapCurve(LADataInstance* dataInstance,
                                            const LAString& curveID,
                                            const LAString& curveName_db,
                                            const LAString& curveName_OIS,
                                            const LAString& curveName_swap,
                                            const LAString& curveNames_OIS,
                                            const LAString& curveNames_swap,
                                            const LAStringMatrix& commonParams,
                                            const LAStringMatrix& generateProp_OIS,
                                            const LAStringMatrix& oisRates_OIS,
                                            const LAStringMatrix& oisConv_OIS,
                                            const LAStringMatrix& histRates_OIS,
                                            const LAStringMatrix& lobasisRates_OIS,
                                            const LAStringMatrix& lobasisConv_OIS,
                                            const LAStringMatrix& swapConv_OIS,
                                            const LAStringMatrix& generateProp_swap,
                                            const LAStringMatrix& moneyConv_swap,
                                            const LAStringMatrix& liborRates_swap,
                                            const LAStringMatrix& liborConv_swap,
                                            const LAStringMatrix& swapRates_swap,
                                            const LAStringMatrix& swapConv_swap,
                                            const LAStringMatrix& fra3mRates_swap,
                                            const LAStringMatrix& fra6mRates_swap,
                                            const LAStringMatrix& fraConv_swap,
                                            const LAStringMatrix& futureRates_swap,
                                            const LAStringMatrix& futureConv_swap,
                                            const LAStringMatrix& adjustSwapConv_swap,
                                            const LAStringMatrix& adjustSwapRates_swap
        );
        //==============================================================================

		static void setUpGlobalEngineCurves(LADataInstance* dataInstance,
                                            const LAString& engineName,
                                            const LAString& curveCollectionID,
                                            const LAStringMatrix& commonParams,
                                            const std::vector<etrading::CurveObjectDataPtr>& curveDataCollection);

        //==============================================================================
        static void setUpCheapestToDeliverCurve(LADataInstance* dataInstance,
                                                const LAString& curveCollection,
                                                const LAString& curveName,
                                                const LAString& curveIndex,
                                                const LAStringMatrix& generateProp,
                                                const LAStringVector& collateralCurves);

        //==============================================================================
        static void setUpForIRServer(void);
        //==============================================================================
        
        static void setUpDefaultIRStaticData(LADataInstance& dataInstance);
        static void setUpDefaultIRStaticData(LADataInstance& dataInstance, const LAString& filePath);
        static void setUpDefaultCalibStaticData(LADataInstance& dataInstance);
        static void setUpDefaultCalibStaticData(LADataInstance& dataInstance, LAString filePath);
        
        //==============================================================================
        static void resetService();
        //==============================================================================
        
        static void SetUpFundingSpread(LADataInstance* dataInstance,const LAStringMatrix &fundingSpread);

        static void CalcMeanAndCovariance(const DoubleMatrix& data, DoubleArray& mean, DoubleMatrix& covar);

        static void SetUpPCA(LADataInstance* dataInstance, const DoubleMatrix& corr, const size_t no_factors, const LAString& id);

        static DoubleMatrix GetPCAResult(LADataInstance* dataInstance, const LAString& type, const LAString& id);

        static void populateStaticDataManagerForSwapCurve(LAStaticData &irprop,
                                                          const LAString& useMarkets,
                                                          const LAString& currency,
                                                          const LAString& curveNames_swap,
                                                          const LAString& marketName_swap,
                                                          const LAString& generateCurveName_swap,
                                                          const LAStringMatrix& generateProp_swap,
                                                          const LAStringMatrix& moneyConv_swap,
                                                          const LAStringMatrix& liborRates_swap,
                                                          const LAStringMatrix& liborConv_swap,
                                                          const LAStringMatrix& swapRates_swap,
                                                          const LAStringMatrix& swapConv_swap,
                                                          const LAStringMatrix& fra3mRates_swap,
                                                          const LAStringMatrix& fra6mRates_swap,
                                                          const LAStringMatrix& fraConv_swap,
                                                          const LAStringMatrix& futureRates_swap,
                                                          const LAStringMatrix& futureConv_swap,
                                                          const LAStringMatrix& adjustSwapConv_swap,
                                                          const LAStringMatrix& adjustSwapRates_swap);

        static void populateStaticDataManagerForOISCurve(LAStaticData &irprop,
                                                         const LAString& useMarkets,
                                                         const LAString& currency,
                                                         const LAString& curveNames_OIS,
                                                         const LAString& marketName_OIS,
                                                         const LAString& generateCurveName_OIS,
                                                         const LAStringMatrix& generateProp_OIS,
                                                         const LAStringMatrix& oisRates_OIS,
                                                         const LAStringMatrix& oisConv_OIS,
                                                         const LAStringMatrix& histRates_OIS,
                                                         const LAStringMatrix& lobasisRates_OIS,
                                                         const LAStringMatrix& lobasisConv_OIS,
                                                         const LAStringMatrix& swapConv_OIS);

        static void populateStaticDataManagerForTenorBasisCurve(LAStaticData &irprop,
                                                          const LAString& curveName,
                                                          const LAString& curveNames,
                                                          const LAString& currency,
                                                          const LAStringMatrix& basisRates,
                                                          const LAStringMatrix& basisConv,
                                                          const LAStringMatrix& fwdFXs,
                                                          const LAStringMatrix& fwdConv,
                                                          const LAStringMatrix& spotFXs,
                                                          const LAStringMatrix& generateProp,
                                                          const LAStringMatrix& moneyConv,
                                                          const LAStringMatrix& fraConv,
                                                          const LAStringMatrix& fraRates,
                                                          const LAStringMatrix& liborConv,
                                                          const LAStringMatrix& liborRates);

    protected:
        static std::istringstream* createFutureStream(const LAStringMatrix& future_rates, LAString& usegrid_future);
        static void setStaticDataValue(LAStaticData &prop, const LAString &key, const LAString &val, const bool is_override = false);
    };
}