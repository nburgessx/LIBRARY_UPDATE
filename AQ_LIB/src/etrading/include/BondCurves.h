#pragma once

#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "FreeObject.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include "BondEnumerations.h"
#include "AQLCoreTemplateType.h"

namespace etrading
{

	// Structure containing the Nelson-Siegel-Svensson model parameters
	struct NelsonSiegelSvenssonParameters
	{
		double beta0_;	    // Long term yield
		double beta1_;	    // Slope
		double beta2_;	    // Curvature
		double lambda1_;	// Time decay

		// The following used by Svensson interpolation only
		double beta3_;      // Secondary curvature
		double lambda2_;    // Secondary time decay
	};

	/* @brief Nelson-Siegel interpolation. Given a set of calibration parameters and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	parameters	NelsonSiegel parameters structure: beta0, beta1, beta2, lambda1 are used.
	*  @param[in]	tau			Bond maturity in years
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	double NelsonSiegelInterpolator( const NelsonSiegelSvenssonParameters& parameters, double tau );

	/* @brief Nelson-Siegel-Svensson interpolation. Given a set of calibration parameters and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	parameters	NelsonSiegel parameters structure: beta0, beta1, beta2, beta3, lambda1, lambda2 are used.
	*  @param[in]	tau			Bond maturity in years
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	double SvenssonInterpolator( const NelsonSiegelSvenssonParameters& parameters, double tau );

	/* @brief Polynomial interpolation. Given a vector of polynomial coefficients and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	coefficients	Polynomial function coefficients: a_0 x^n + a_1 x^(n-1) + ... +a_n i.e. highest order coefficient is first in the vector.
	*  @param[in]	tau				Bond maturity in years
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	double PolynomialInterpolator( const DoubleVector& coefficients, double tau );


	// Bond Curve Stripping

	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, VariantMatrix>  TableInfo;

	/* @brief	Represents a BondCurve calibrated from bond yield quotes
	*/
	class BondCurve : public IsLWOObject
	{
	public:
			
		/* @brief Main Constructor
		 * @param[in] objectName    The name of this BondCurve instance
		 * @param[in] propertyKeys  A vector containing the names of each configuration block
		 * @param[in] infoBlocks    A vector containing the configuation blocks
		 */
		BondCurve( const std::string& objectName,
                   const std::vector<std::string>& propertyKeys,
                   const std::vector<TableInfo>& infoBlocks );

		/* @brief Constructor used by deserialization
		 * @param[in] objectName    The name of this BondCurve instance
		 * @param[in] freeObject    A freeObject constructed from the serialized data
		 */
		BondCurve( const std::string& objectName, const FreeObject& freeObject );

		/* @brief Copy Constructor
		 */
   		BondCurve(const BondCurve& rhs);

		virtual ~BondCurve() {}

		std::shared_ptr<BondCurve> clone() const;

		/* @brief Used to serialize an instance of this class
		 * @param[out] the populated SchemaObject
		 */
		virtual const SchemaObject toSchemaObject() const;

        // This is a helper method to populate the supplied SchemaObject
        virtual void const toSchemaObject(SchemaObject& schemaObject) const;

		// ------- Accessor methods --------

		/* @brief Returns the yield interpolated from the BondCurve for the specified couponDate
		*				Note: the method uses piecewise-constant interpolation.
		* @param[in]	couponDate	The date for which the yield is required
		* @returns		The interpolated yield
		*/
		double getYield( const AQLDate& couponDate ) const;

		/* @brief Updates the yield calibration stored in the curve by adding a yield point for the specified pillarDate.
		*        This method intended to be used by the calibration process when fitting the curve to input bond quotes.
		* 
		* @param [in]   bondMaturityDate	The date corresponding to this coupon yield
		* @param [in]   yield				The estimate of the yield for this curve pillar date
		*/
		void setCalibrationPoint( const AQLDate& bondMaturityDate, const double& yield );

		/* @brief Updates the calibration stored in the curve by adding a discountFactor point for the specified pillar date.
		*         This method intended to be used by the calibration process when fitting the curve to input bond quotes.
		*
		* @param [in]   bondMaturityDate	The date corresponding to this coupon yield
		* @param [in]   discountFactor		The discountFactor at the bond curve pillar date
		*/
		void setDiscountFactorAtCalibrationPoint(const AQLDate& bondMaturityDate, const double& discountFactor);

		/* @brief	Returns the bond curve calibration as a matrix.
		*			Column 0 contains curve pillar dates
		*			Column 1 contains the calibrated yield points 
		*/
		AnyTypeMatrix displayBondCurve() const;

		// Simple data getters
		AQLDate getSettlementDate() const;

		YieldCalculationTypeEnum getYieldCalculationTypeEnum() const;

		bool getYieldQuoteInPercent() const;

		std::string getInterpolationMethod() const;

		std::string getExtrapolationMethod() const;

	private:

		/* @brief		Called by constructor to calibrate yields to maturity from the provided bond quotes
		*/
		void calibrate();
		
		/*
		*  @brief  Utility method which validates the property keys of this BondCurve, to verify that all are recognized key names.
		*          Will throw if one of the keys is not expected.
		*/
		void validateKeys() const ;
		
		/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
		 * @param [in]   propertyKey   The property to be displayed
	     * @param [out]  A LabelValue block containing the properties
		 */
		LabelValueBlock toLabelValueBlock( const std::string& propertyKey ) const;


		// The main data store of this object. Allows easy serialization
		FreeObject freeObject_;

		// The settlementDate to use for bond valuation
		AQLDate settlementDate_;

		// The yield Calculation Type ( see BondEnumerations )
		YieldCalculationTypeEnum yieldCalculationTypeEnum_;

		// Whether the input yield quotes are expressed in percent or in decimal.
		// If TRUE it means the yield quotes are in percent units, and the BondCurve will divide through by 100 to get a decimal number.
		// i.e. "2.0" is mapped to "0.02".
		bool yieldQuoteInPercent_;

		// Used when interpolating yields from the bond curve
		std::string interpolationMethod_;
		std::string extrapolationMethod_;

		double spread_;

		// The output from calibration: A map of payment dates and corresponding yields
		std::map<AQLDate, double> calibratedYields_;

		std::map<AQLDate, double> calibratedDiscountFactors_;

		static std::vector<std::string> bond_curve_properties_lvbKeys()
		{
			const std::string arr[] =
			{
				BONDCURVE_PROPERTIES_KEY::SETTLEMENT_DATE
				, BONDCURVE_PROPERTIES_KEY::YIElD_CALCULATION_TYPE
				, BONDCURVE_PROPERTIES_KEY::YIELD_QUOTE_IN_PERCENT
				, BONDCURVE_PROPERTIES_KEY::INTERPOLATION
				, BONDCURVE_PROPERTIES_KEY::EXTRAPOLATION
			};

			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		static std::vector<std::string> bond_spread_curve_properties_lvbKeys()
		{
			const std::string arr[] =
			{
				BONDSPREADCURVE_PROPERTIES_KEY::SPREAD
				, BONDSPREADCURVE_PROPERTIES_KEY::BENCHMARK_BOND_CURVE
			};

			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}
	};

	typedef std::shared_ptr< BondCurve > BondCurvePtr;
}


