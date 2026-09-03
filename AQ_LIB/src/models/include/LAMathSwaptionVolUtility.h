#ifndef LAMathSwaptionVolUtility_h
#define LAMathSwaptionVolUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include <LADataInstance.h>
#include <LAString.h>
#include <LADate.h>
#include <LACoreTemplateType.h>
#include <ConstantDeclarations.h>
#include "LAPriceDataSlidingRule.h"

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
    static void setUpConvention( LADataInstance* dataInstance, const LAString& convID, LAStringMatrix& convMat );

	static void setCurveID(LADataInstance* dataInstance, const LAString& name, const LAString& swapConvID, const LAString& capConvID);

	static void setCurveID2(LADataInstance* dataInstance, const LAString& name, LAStringMatrix& curveMat);

	static void setCurveID2(LADataInstance* dataInstance, const LAString& name, const LAStringVector& sTenor, LAStringMatrix& curveMat);

	static void setUpSABRGrid( LADataInstance* dataInstance, const LAString& matID, const LAString& convID, LAStringMatrix& mat );

    static void setUpSwaptionVol( LADataInstance* dataInstance, const LAString& matID, LAStringMatrix& volMat, LAStringMatrix& strikeMat , LAStringMatrix& sign);

	static void setUpForwardShiftValue(LADataInstance* dataInstance, LAString& currencyID, double forwardshiftvalue = 0.);

	static double getForwardShiftValue(LADataInstance* dataInstance, LAString& CurrencyID );

    static double lookUpSwapGrid(LADataInstance* dataInstance, const LAString& matID, LAString expPoint, LAString tenorPoint);

    static double lookUpSwapGrid(LADataInstance* dataInstance, const LAString& matID, LADate expDate, LAString tenorPoint);

    static void outPutSABRGrid( LADataInstance* dataInstance, const LAString& matID, DoubleVector& ret, size_t& row, size_t& colum );
    
    static void calibrateSABRMatrix( LADataInstance* dataInstance, const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID, 
									 const LAString& nuID, const LAString& rhoID, const LAString& swapConvID, 
									 const LAString& capConvID, const LAStringVector& swapVolID, /*LAStringMatrix sabrLimiter,*/
									 const LAString& target, const DoubleVector& weight, const IntVector& sgn, const LAString& forwardID, const double forwardShiftValue, 
									 const LAString& numeraireID, LAString& msg, const BoolMatrix *calibFlgMtx = 0, bool isLognormal = true);

	static void calibrateSABRMatrix(DoubleMatrix& alphaMat, DoubleMatrix& betaMat, DoubleMatrix& nuMat, DoubleMatrix& rhoMat,
									const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod, 
									const std::vector<DoubleMatrix >& marketVol, const std::vector<DoubleMatrix >& marketStrike, const DoubleVector& expiryTerm,
									const LAString& target, const DoubleVector& weight, const IntVector& sgn, const double solverEpsilon,
									const DoubleMatrix& forwardMat, const double forwardShiftValue, const DoubleMatrix& numeraireMat,
									const DoubleMatrix& atmMarketVol, const bool alphaFromAtmVol,
									LAString& msg, const BoolMatrix *calibFlgMtx = 0, const bool isLognormal = true);

	static void calibrateSABRMatrix( LADataInstance* dataInstance, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID,
									 const LAString& nuID, const LAString& rhoID, const LAString& swapConvID, 
									 const LAString& capConvID, const LAStringVector& swapVolID, LAStringMatrix sabrLimiter, 
									 LAString target, const DoubleVector& weight, const LAString& forwardID, const double forwardShiftValue );

	static void calibrateSABRMatrixCapFloor(LADataInstance* dataInstance, const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID,
									const LAString& nuID, const LAString& rhoID, const LAString& swapConvID,
									const LAString& capConvID, const LAStringVector& swapVolID, const LAString& target, const DoubleVector& weight,
		                            const IntVector& sgn_in, const double forwardShiftValue, LAString& msg, const BoolMatrix *calibFlgMtx = 0);

	static DoubleArray calibrateSABR( const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod,
									  double alpha, double beta, double nu, double rho, double forward, LAString expiryPoint,
									  double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
									  /*LAStringMatrix sabrLimiter,*/ const LAString& target, const DoubleVector& weight, const IntVector& sgn );
    static DoubleArray calibrateSABR( double alpha, double beta, double nu, double rho, double forward, LAString expiryPoint,
                                      double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
                                      LAStringMatrix sabrLimiter, LAString target, const DoubleVector& weight );


    static void calibrateSABRATMFix( LADataInstance* dataInstance, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID, 
									 const LAString& nuID, const LAString& rhoID, const LAString& swapConvID, 
									 const LAString& capConvID, const LAString& ATMVolID, const LAStringVector& swapVolID, 
									 LAStringMatrix sabrLimiter, const LAString& target, const DoubleVector& weight, 
									 const LAString& forwardID, const double forwardShiftValue );

    static double getSABRAlpha( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, const LAString& volID, 
                                const LAString& forwardID, const LAString& alphaID, const LAString& betaID, const LAString& nuID, 
                                const LAString& rhoID );

    static double getSABRAlpha2( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double vol, 
                                 const LAString& alphaID, const LAString& betaID, const LAString& nuID, const LAString& rhoID, 
                                 const LAString& curveID, const LAString& convID , const LAString& foreCurveName, 
								 const LAString& dfCurveName);

    static double getSABRVol( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, 
                              const LAString& fowardID, const LAString& alphaID, const LAString& betaID, const LAString& nuID, 
                              const LAString& rhoID, const LAString& approxMethod = "Antonov", const double shift = 0.0, bool isLognormal = true);

	static double calcSABRVol(const double alpha, const double beta, const double nu, const double rho, const double expiryTerm, const double strike, const double forward, const double shift, const LAString& approxMethod, bool isLognormal = true);

	static double calcSABRParam(const DoubleMatrix& paramMat, const double expiryTerm, const double tenorTerm, const DoubleVector& expiryVec, const DoubleVector& tenorVec);

	static double getSABRVol2( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, const LAString& alphaID,
                               const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& curveID, 
							   const LAString& convID, LAString foreCurveName = STD, LAString dfCurveName = STD, const LAString& approxMethod = "Antonov");

	static double getSABRVol3( LADataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double strike, const LAString& alphaID,
							   const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& approxMethod = "Antonov");

    static double getSABRVol4( LADataInstance* dataInstance, double expTerm, const LAString& tenorPoint, double forward, double strike, const LAString& alphaID,
                               const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& curveID, 
							   const LAString& convID, LAString foreCurveName = STD, LAString dfCurveName = STD,const LAString& approxMethod = "Antonov");


    static double getSABRPrem( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, int sgn,
                               const LAString& fowardID, const LAString& numeraireID, const LAString& alphaID, const LAString& betaID, const LAString& nuID, 
                               const LAString& rhoID, const LAString& approxMethod = "Antonov", const double shift = 0.0, bool isLognormal = true);

	static double getSABRPrem2( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, int sgn, const LAString& alphaID,
                               const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& curveID, 
							   const LAString& convID, LAString foreCurveName = STD, LAString dfCurveName = STD, const LAString& approxMethod = "Antonov");
	
	static double getSABRPrem3( LADataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double numeraire, double strike, int sgn, const LAString& alphaID,
							   const LAString& betaID, const LAString& nuID, const LAString& rhoID,const LAString& approxMethod = "Antonov");



	static DoubleMatrix getSABRVolMatrix( LADataInstance* dataInstance, const LAString& strikeID, const LAString& forwardID, const LAString& alphaID,
										  const LAString& betaID, const LAString& nuID, const LAString& rhoID );
//private:
	static double getExpiryPoint(LAString str, const LADate& asOfDate, const LAPriceDataSlidingRule& slr, const LAPriceDataCalendar& cal);

	static LADate getExpiryPoint2(LAString str, const LADate& asOfDate, const LAPriceDataSlidingRule& slr, const LAPriceDataCalendar& cal);

	static double getForward(LADataInstance* dataInstance, const LADate& expiry, const LAString& tenor, const LAString& curveID, const LAString& convID, 
							 LAString foreCurveName = STD, LAString dfCurveName = STD, bool isFWDInter = true);

	static double getNumeraire(LADataInstance* dataInstance, const LADate& expiry, const LAString& tenor, const LAString& curveID, const LAString& convID, 
							   LAString curveName = STD);

	static LADate getLADate(LAString date);

	static double getTenorPoint(LAString str);

	static void getSABRParam(DoubleVector& expiry, DoubleVector& tenor, const LAString& alphaID, const LAString& betaID, 
							 const LAString& nuID, const LAString& rhoID);

    static void matirixCheck( LADataInstance* dataInstance, const LAString& entityName, const LAString& entityName2, bool isVolMat = false );

private:
    static LAMathSABR* createSABR(const LAString& approxMethod, double alpha, double beta, double nu, double rho, bool isLognormal = true);

	LAMathSwaptionVolUtility(void);
	~LAMathSwaptionVolUtility(void);
	LAMathSwaptionVolUtility(const LAMathSwaptionVolUtility &rhs);
	LAMathSwaptionVolUtility &operator=(const LAMathSwaptionVolUtility &rhs);
};
#endif 