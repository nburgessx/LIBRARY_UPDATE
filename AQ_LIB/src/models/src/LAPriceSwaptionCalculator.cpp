#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceSwaptionCalculator.h"
#include "LAPriceSABRSwaptionCalculator.h"
#include "LAPriceThresholdSABRSwaptionCalculator.h"
#include "LAPriceFlatSABRSwaptionCalculator.h"
#include "LAPriceBenaimSABRSwaptionCalculator.h"

//================ LAPriceSwaptionCalculator ===================================
LAPriceSwaptionCalculator::LAPriceSwaptionCalculator(double S0_)
{
    mS0 = S0_;
}

LAPriceSwaptionCalculator::~LAPriceSwaptionCalculator()
{
}

//================ Utilities ===================================
LAPriceSwaptionCalculator* GetSwaptionCalculator(string type, double fwd, const vector<double>& sabrParameters,
                                            const vector<double>& extraParameters)
{
    LAPriceSwaptionCalculator* calculator = 0;
    if (type == "Original SABR")
        calculator = new MVSABRSwaptionCalculator(fwd, sabrParameters);
    else if (type == "Threshold SABR")
        calculator = new MVThresholdSABRSwaptionCalculator(fwd, sabrParameters, extraParameters);
    else if (type == "Flat SABR")
        calculator = new MVFlatSABRSwaptionCalculator(fwd, sabrParameters, extraParameters);
    else if (type == "Benaim SABR")
        calculator = new MVBenaimSABRSwaptionCalculator(fwd, sabrParameters, extraParameters, true);
    else if (type == "Right Benaim SABR")
        calculator = new MVBenaimSABRSwaptionCalculator(fwd, sabrParameters, extraParameters, false);
    else
        throw AQLCoreInvalidData(("Unknown swaption calculator type: " + type).c_str(),__FILE__,__LINE__);

    return calculator;
}
