#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceSwaptionCalculator.h"
#include "AQLPriceSABRSwaptionCalculator.h"
#include "AQLPriceThresholdSABRSwaptionCalculator.h"
#include "AQLPriceFlatSABRSwaptionCalculator.h"
#include "AQLPriceBenaimSABRSwaptionCalculator.h"

//================ AQLPriceSwaptionCalculator ===================================
AQLPriceSwaptionCalculator::AQLPriceSwaptionCalculator(double S0_)
{
    mS0 = S0_;
}

AQLPriceSwaptionCalculator::~AQLPriceSwaptionCalculator()
{
}

//================ Utilities ===================================
AQLPriceSwaptionCalculator* GetSwaptionCalculator(string type, double fwd, const vector<double>& sabrParameters,
                                            const vector<double>& extraParameters)
{
    AQLPriceSwaptionCalculator* calculator = 0;
    if (type == "Original SABR")
        calculator = new AQLSABRSwaptionCalculator(fwd, sabrParameters);
    else if (type == "Threshold SABR")
        calculator = new AQLThresholdSABRSwaptionCalculator(fwd, sabrParameters, extraParameters);
    else if (type == "Flat SABR")
        calculator = new AQLFlatSABRSwaptionCalculator(fwd, sabrParameters, extraParameters);
    else if (type == "Benaim SABR")
        calculator = new AQLBenaimSABRSwaptionCalculator(fwd, sabrParameters, extraParameters, true);
    else if (type == "Right Benaim SABR")
        calculator = new AQLBenaimSABRSwaptionCalculator(fwd, sabrParameters, extraParameters, false);
    else
        throw AQLCoreInvalidData(("Unknown swaption calculator type: " + type).c_str(),__FILE__,__LINE__);

    return calculator;
}
