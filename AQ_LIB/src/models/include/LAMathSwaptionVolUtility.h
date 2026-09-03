#ifndef LAMathSwaptionVolUtility_h
#define LAMathSwaptionVolUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include <AQLDataInstance.h>
#include <AQLString.h>
#include <AQLDate.h>
#include <AQLCoreTemplateType.h>
#include <ConstantDeclarations.h>
#include "AQLPriceDataSlidingRule.h"

class LAMathSABR;
//////////////////////////

#ifndef PRICING_DATA_EXPIRYSTRING
#define PRICING_DATA_EXPIRYSTRING		"ExpiryString"		//  data name of ExpiryString
#endif
#ifndef PRICING_DATA_TENORSTRING
#define PRICING_DATA_TENORSTRING		"TenorString"		//  data name of TenorString
#endif
#ifndef PRICING_DATA_SWAPTIONMATRIX
#define PRICING_DATA_SWAPTIONMATRIX		"SwaptionMatirx"		//  data name of SwaptionMatrix
#endif
#ifndef PRICING_DATA_PREMIUMMATRIX
#define PRICING_DATA_PREMIUMMATRIX		"PremiumMatirx"		//  data name of PremiumMatrix
#endif
#ifndef PRICING_DATA_SWAPTIONSTRIKEMATRIX
#define PRICING_DATA_SWAPTIONSTRIKEMATRIX		"SwaptionStrikeMatrix"		//  data name of SwaptionMatrix
#endif
#ifndef PRICING_DATA_SWAPTIONSIGNMATRIX
#define PRICING_DATA_SWAPTIONSIGNMATRIX		"SwaptionSignMatrix"		//  data name of SwaptionMatrix
#endif
#ifndef PRICING_DATA_EXPIRYVECTOR
#define PRICING_DATA_EXPIRYVECTOR		"ExpiryVector"		//  data name of ExpiryVector
#endif
#ifndef PRICING_DATA_TENORVECTOR
#define PRICING_DATA_TENORVECTOR		"TenorVector"		//  data name of TenorVector
#endif
#ifndef PRICING_DATA_EXPIRYDATEVECTOR
#define PRICING_DATA_EXPIRYDATEVECTOR	"ExpiryDateVector"		//  data name of ExpiryDateVector
#endif
#ifndef PRICING_DATA_CURVETYPEKEYS
#define PRICING_DATA_CURVETYPEKEYS		"CurveTypeKeys"		//  data name of CurveTypeKeys
#endif
#ifndef PRICING_DATA_CURVETYPES
#define PRICING_DATA_CURVETYPES		"CurveTypes"		//  data name of CurveTypes
#endif
#ifndef PRICING_DATA_CALIBRATIONFLAG
#define PRICING_DATA_CALIBRATIONFLAG		"CalibrationFlag"		//  data name of CalibrationFlag
#endif

#ifndef CALIBRATION_DATA_CURVEID
#define CALIBRATION_DATA_CURVEID					"CurveID"				//  [Curve ID] Data
#endif
#ifndef CALIBRATION_DATA_1MLCURVENAME
#define CALIBRATION_DATA_1MLCURVENAME				"1MLiborCurveName"		//  [1 Month Libor Forecast Curve Name] Data
#endif
#ifndef CALIBRATION_DATA_3MLCURVENAME
#define CALIBRATION_DATA_3MLCURVENAME				"3MLiborCurveName"		//  [3 Month Libor Forecast Curve Name] Data
#endif
#ifndef CALIBRATION_DATA_6MLCURVENAME
#define CALIBRATION_DATA_6MLCURVENAME				"6MLiborCurveName"		//  [6 Month Libor Forecast Curve Name] Data
#endif
#ifndef CALIBRATION_DATA_SWAPRATELCURVENAME
#define CALIBRATION_DATA_SWAPRATELCURVENAME		"SwapRateCurveName"		//  [Swap Rate Forecast Curve Name] Data
#endif
#ifndef CALIBRATION_DATA_DISCOUNTCURVENAME
#define CALIBRATION_DATA_DISCOUNTCURVENAME		"DiscountCurveName"		//  [Discount Curve Name] Data
#endif
#ifndef PRICING_DATA_GRIDAPPROXMETHOD
#define PRICING_DATA_GRIDAPPROXMETHOD	"GridApproxMethod"      //  [Grid Approx Method] Data
#endif

#ifndef	CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD
#define CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD		"NON_LINEAR_CONJUGATE_GRADIENT_METHOD"
#endif
#ifndef	CALIB_STEEPEST_DESCENT_METHOD
#define CALIB_STEEPEST_DESCENT_METHOD					"STEEPEST_DESCENT_METHOD"
#endif
#ifndef	CALIB_SIMPLEX_METHOD
#define CALIB_SIMPLEX_METHOD							"SIMPLEX_METHOD"
#endif
#ifndef	CALIB_LEVENBERG_MARQUARDT_METHOD
#define CALIB_LEVENBERG_MARQUARDT_METHOD				"LEVENBERG-MARQUARDT_METHOD"
#endif

#ifndef	APPROXIMATION_HAGAN
#define APPROXIMATION_HAGAN		"HAGAN"
#endif
#ifndef	APPROXIMATION_ANTONOV
#define APPROXIMATION_ANTONOV	"ANTONOV"
#endif
#ifndef	APPROXIMATION_CHAOS_DD
#define APPROXIMATION_CHAOS_DD	"CHAOS_DD"
#endif


#ifndef STD
#define STD     "STD"
#endif



class LAMathSwaptionVolUtility
{
friend class LAMathLeastSquareSABRCapFloorCostFuncQlib;

public:
    static void setUpConvention( AQLDataInstance* dataInstance, const AQLString& convID, AQLStringMatrix& convMat );

	static void setCurveID(AQLDataInstance* dataInstance, const AQLString& name, const AQLString& swapConvID, const AQLString& capConvID);

	static void setCurveID2(AQLDataInstance* dataInstance, const AQLString& name, AQLStringMatrix& curveMat);

	static void setCurveID2(AQLDataInstance* dataInstance, const AQLString& name, const AQLStringVector& sTenor, AQLStringMatrix& curveMat);

	static void setUpSABRGrid( AQLDataInstance* dataInstance, const AQLString& matID, const AQLString& convID, AQLStringMatrix& mat );

    static void setUpSwaptionVol( AQLDataInstance* dataInstance, const AQLString& matID, AQLStringMatrix& volMat, AQLStringMatrix& strikeMat , AQLStringMatrix& sign);

	static void setUpForwardShiftValue(AQLDataInstance* dataInstance, AQLString& currencyID, double forwardshiftvalue = 0.);

	static double getForwardShiftValue(AQLDataInstance* dataInstance, AQLString& CurrencyID );

    static double lookUpSwapGrid(AQLDataInstance* dataInstance, const AQLString& matID, AQLString expPoint, AQLString tenorPoint);

    static double lookUpSwapGrid(AQLDataInstance* dataInstance, const AQLString& matID, AQLDate expDate, AQLString tenorPoint);

    static void outPutSABRGrid( AQLDataInstance* dataInstance, const AQLString& matID, DoubleVector& ret, size_t& row, size_t& colum );
    
    static void calibrateSABRMatrix( AQLDataInstance* dataInstance, const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID, 
									 const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID, 
									 const AQLString& capConvID, const AQLStringVector& swapVolID, /*AQLStringMatrix sabrLimiter,*/
									 const AQLString& target, const DoubleVector& weight, const IntVector& sgn, const AQLString& forwardID, const double forwardShiftValue, 
									 const AQLString& numeraireID, AQLString& msg, const BoolMatrix *calibFlgMtx = 0, bool isLognormal = true);

	static void calibrateSABRMatrix(DoubleMatrix& alphaMat, DoubleMatrix& betaMat, DoubleMatrix& nuMat, DoubleMatrix& rhoMat,
									const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod, 
									const std::vector<DoubleMatrix >& marketVol, const std::vector<DoubleMatrix >& marketStrike, const DoubleVector& expiryTerm,
									const AQLString& target, const DoubleVector& weight, const IntVector& sgn, const double solverEpsilon,
									const DoubleMatrix& forwardMat, const double forwardShiftValue, const DoubleMatrix& numeraireMat,
									const DoubleMatrix& atmMarketVol, const bool alphaFromAtmVol,
									AQLString& msg, const BoolMatrix *calibFlgMtx = 0, const bool isLognormal = true);

	static void calibrateSABRMatrix( AQLDataInstance* dataInstance, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID,
									 const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID, 
									 const AQLString& capConvID, const AQLStringVector& swapVolID, AQLStringMatrix sabrLimiter, 
									 AQLString target, const DoubleVector& weight, const AQLString& forwardID, const double forwardShiftValue );

	static void calibrateSABRMatrixCapFloor(AQLDataInstance* dataInstance, const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID,
									const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID,
									const AQLString& capConvID, const AQLStringVector& swapVolID, const AQLString& target, const DoubleVector& weight,
		                            const IntVector& sgn_in, const double forwardShiftValue, AQLString& msg, const BoolMatrix *calibFlgMtx = 0);

	static DoubleArray calibrateSABR( const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod,
									  double alpha, double beta, double nu, double rho, double forward, AQLString expiryPoint,
									  double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
									  /*AQLStringMatrix sabrLimiter,*/ const AQLString& target, const DoubleVector& weight, const IntVector& sgn );
    static DoubleArray calibrateSABR( double alpha, double beta, double nu, double rho, double forward, AQLString expiryPoint,
                                      double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
                                      AQLStringMatrix sabrLimiter, AQLString target, const DoubleVector& weight );


    static void calibrateSABRATMFix( AQLDataInstance* dataInstance, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID, 
									 const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID, 
									 const AQLString& capConvID, const AQLString& ATMVolID, const AQLStringVector& swapVolID, 
									 AQLStringMatrix sabrLimiter, const AQLString& target, const DoubleVector& weight, 
									 const AQLString& forwardID, const double forwardShiftValue );

    static double getSABRAlpha( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, const AQLString& volID, 
                                const AQLString& forwardID, const AQLString& alphaID, const AQLString& betaID, const AQLString& nuID, 
                                const AQLString& rhoID );

    static double getSABRAlpha2( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double vol, 
                                 const AQLString& alphaID, const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, 
                                 const AQLString& curveID, const AQLString& convID , const AQLString& foreCurveName, 
								 const AQLString& dfCurveName);

    static double getSABRVol( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, 
                              const AQLString& fowardID, const AQLString& alphaID, const AQLString& betaID, const AQLString& nuID, 
                              const AQLString& rhoID, const AQLString& approxMethod = "Antonov", const double shift = 0.0, bool isLognormal = true);

	static double calcSABRVol(const double alpha, const double beta, const double nu, const double rho, const double expiryTerm, const double strike, const double forward, const double shift, const AQLString& approxMethod, bool isLognormal = true);

	static double calcSABRParam(const DoubleMatrix& paramMat, const double expiryTerm, const double tenorTerm, const DoubleVector& expiryVec, const DoubleVector& tenorVec);

	static double getSABRVol2( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, const AQLString& alphaID,
                               const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& curveID, 
							   const AQLString& convID, AQLString foreCurveName = STD, AQLString dfCurveName = STD, const AQLString& approxMethod = "Antonov");

	static double getSABRVol3( AQLDataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double strike, const AQLString& alphaID,
							   const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& approxMethod = "Antonov");

    static double getSABRVol4( AQLDataInstance* dataInstance, double expTerm, const AQLString& tenorPoint, double forward, double strike, const AQLString& alphaID,
                               const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& curveID, 
							   const AQLString& convID, AQLString foreCurveName = STD, AQLString dfCurveName = STD,const AQLString& approxMethod = "Antonov");


    static double getSABRPrem( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, int sgn,
                               const AQLString& fowardID, const AQLString& numeraireID, const AQLString& alphaID, const AQLString& betaID, const AQLString& nuID, 
                               const AQLString& rhoID, const AQLString& approxMethod = "Antonov", const double shift = 0.0, bool isLognormal = true);

	static double getSABRPrem2( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, int sgn, const AQLString& alphaID,
                               const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& curveID, 
							   const AQLString& convID, AQLString foreCurveName = STD, AQLString dfCurveName = STD, const AQLString& approxMethod = "Antonov");
	
	static double getSABRPrem3( AQLDataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double numeraire, double strike, int sgn, const AQLString& alphaID,
							   const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID,const AQLString& approxMethod = "Antonov");



	static DoubleMatrix getSABRVolMatrix( AQLDataInstance* dataInstance, const AQLString& strikeID, const AQLString& forwardID, const AQLString& alphaID,
										  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID );
//private:
	static double getExpiryPoint(AQLString str, const AQLDate& asOfDate, const AQLPriceDataSlidingRule& slr, const AQLPriceDataCalendar& cal);

	static AQLDate getExpiryPoint2(AQLString str, const AQLDate& asOfDate, const AQLPriceDataSlidingRule& slr, const AQLPriceDataCalendar& cal);

	static double getForward(AQLDataInstance* dataInstance, const AQLDate& expiry, const AQLString& tenor, const AQLString& curveID, const AQLString& convID, 
							 AQLString foreCurveName = STD, AQLString dfCurveName = STD, bool isFWDInter = true);

	static double getNumeraire(AQLDataInstance* dataInstance, const AQLDate& expiry, const AQLString& tenor, const AQLString& curveID, const AQLString& convID, 
							   AQLString curveName = STD);

	static AQLDate getLADate(AQLString date);

	static double getTenorPoint(AQLString str);

	static void getSABRParam(DoubleVector& expiry, DoubleVector& tenor, const AQLString& alphaID, const AQLString& betaID, 
							 const AQLString& nuID, const AQLString& rhoID);

    static void matirixCheck( AQLDataInstance* dataInstance, const AQLString& entityName, const AQLString& entityName2, bool isVolMat = false );

private:
    static LAMathSABR* createSABR(const AQLString& approxMethod, double alpha, double beta, double nu, double rho, bool isLognormal = true);

	LAMathSwaptionVolUtility(void);
	~LAMathSwaptionVolUtility(void);
	LAMathSwaptionVolUtility(const LAMathSwaptionVolUtility &rhs);
	LAMathSwaptionVolUtility &operator=(const LAMathSwaptionVolUtility &rhs);
};
#endif 