#pragma once
#include "CapletFloorlet.h"

namespace validation
{
    // Calculate the Price
    const double tryAqMathCapletFloorletPrice( const std::string& capletOrFloorlet,
                                               const double & annuityFactor,
										       const double & liborRate,
										       const double & strike,
                                               const double & vol,
                                               const double & time,
										       const double & shift = 0.0,
                                               const std::string & volatilityType = "LOGNORMAL" );
    
	// Calculate the Price - optimize = true uses OMP threading
    const std::vector<double> tryAqMathCapletFloorletPrices( const std::vector<std::string> & capletOrFloorlet,
													         const std::vector<double> & annuityFactor,
													         const std::vector<double> & liborRate,
                                                             const std::vector<double> & strike,
                                                             const std::vector<double> & vol,
                                                             const std::vector<double> & time,
													         const std::vector<double> & shift = std::vector<double>(),
                                                             const std::vector<std::string> & volatilityType = std::vector<std::string>(),
                                                             const bool optimize = false );

	// Calculate the tryAqMathCapletFloorletImpliedVol
    const double tryAqMathCapletFloorletImpliedVol( const double & price,
                                                    const std::string& capletOrFloorlet,
											        const double & annuityFactor,
											        const double & liborRate,
											        const double & strike,
                                                    const double & time,
											        const double & shift = 0.0,
                                                    const std::string & volatilityType = "LOGNORMAL" );

    // Calculate the tryAqMathCapletFloorletImpliedVols - optimize = true uses OMP threading
    const std::vector<double> tryAqMathCapletFloorletImpliedVols( const std::vector<double> & price,
                                                                  const std::vector<std::string> & capletOrFloorlet,
															      const std::vector<double> & annuityFactor,
															      const std::vector<double> & liborRate,
                                                                  const std::vector<double> & strike,
                                                                  const std::vector<double> & time,
															      const std::vector<double> & shift = std::vector<double>(),
                                                                  const std::vector<std::string> & volatilityType = std::vector<std::string>(),
                                                                  const bool optimize = false );


}
