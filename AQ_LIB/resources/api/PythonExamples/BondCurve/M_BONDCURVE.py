#Import the MLIB CLIENT API
import MLIB_CLIENT_API as m

#Set-Up MLIB
print "Set-Up MLIB"
print "-----------------------------------"
print ( m.setUpMLIB("","") )    #Load Calendars and Config Files
print ( m.meUtilityVersion() )  #Get the MLIB Version
print

#Bond Data
bondMaturitiesInYears = ( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 )
bondYieldsInPercent = ( -0.6, -0.6, -0.6, -0.6, -0.6, -0.6, -0.6, -0.6, -0.6, -0.6 )
print "Bond Data"
print "-----------------------------------"
print ( "Maturities in Years: ", bondMaturitiesInYears )
print ( "Yields in Percent: ", bondYieldsInPercent )
print

# Nelson Siegel / Svensson Parameters - Initial Guess: Use 0 for Betas and 1 for Lambdas
beta0 = 0       # Long Term Yield
beta1 = 0       # Slope
beta2 = 0       # Curvature
beta3 = 0       # Secondary Curvature - Required for Svensson Only
lambda1 = 1     # TimeDecay
lambda2 = 1     # Secondary TimeDecay - Required for Svensson Only
modelParameters = m.NelsonSiegelSvenssonParameters( beta0, beta1, beta2, beta3, lambda1, lambda2 )

# Model Settings
maxInterations = 100
maxStationaryIterations = 10

# For Beta Bounds: Use -50 for lower- and +50 for upperBounds
# For Lambda Bounds: Use 0 for lower- and +50 for upperBounds
# Note size 4 for Nelson-Siegel and size 6 for Svensson
lowerBoundForNelsonParameters = ( -50, -50, -50, 0 )
upperBoundForNeslonParameters = ( 50, 50, 50, 50 )

lowerBoundForSvenssonParameters = ( -50, -50, -50, -50, 0, 0 )
upperBoundForSvenssonParameters = ( 50, 50, 50, 50, 50, 50 )

# Calibrate Nelson-Siegel
calibrateNS = m.meBondCurveNelsonSiegelCalibrate( bondMaturitiesInYears, bondYieldsInPercent, modelParameters, maxInterations, maxStationaryIterations, lowerBoundForNelsonParameters, upperBoundForNeslonParameters )
b0 = calibrateNS.parameters_.beta0_
b1 = calibrateNS.parameters_.beta1_
b2 = calibrateNS.parameters_.beta2_
b3 = calibrateNS.parameters_.beta3_
l1 = calibrateNS.parameters_.lambda1_
l2 = calibrateNS.parameters_.lambda2_ 

# Calibration Results
print "Calibration Parameters - Nelson-Siegel"
print "-----------------------------------"
print ( "Beta0: ", b0 )
print ( "Beta1: ", b1 )
print ( "Beta2: ", b2 )
print ( "Beta3: ", b3 )
print ( "Lambda1: ", l1 )
print ( "Lambda2: ", l2 )
print

# Bond Maturities - Used for testing interpolated / fitted results
fittedBondMaturities = ( 1.0, 2.5, 5, 7.5, 10.0) 

print "Fitted Yields - Nelson-Siegel"
print "-----------------------------------"
print ("Maturities: ", fittedBondMaturities)
print ("Yields: ", m.meBondCurveNelsonSiegelYield( b0, b1, b2, l1, fittedBondMaturities ))


# Calibrate Svensson
calibrateS = m.meBondCurveSvenssonCalibrate( bondMaturitiesInYears, bondYieldsInPercent, modelParameters, maxInterations, maxStationaryIterations, lowerBoundForSvenssonParameters, upperBoundForSvenssonParameters )
b0 = calibrateS.parameters_.beta0_
b1 = calibrateS.parameters_.beta1_
b2 = calibrateS.parameters_.beta2_
b3 = calibrateS.parameters_.beta3_
l1 = calibrateS.parameters_.lambda1_
l2 = calibrateS.parameters_.lambda2_ 

# Calibration Results
print "Calibration Parameters - Svensson"
print "-----------------------------------"
print ( "Beta0: ", b0 )
print ( "Beta1: ", b1 )
print ( "Beta2: ", b2 )
print ( "Beta3: ", b3 )
print ( "Lambda1: ", l1 )
print ( "Lambda2: ", l2 )
print

print "Fitted Yields - Svensson"
print "-----------------------------------"
print ("Maturities: ", fittedBondMaturities)
print ("Yields: ", m.meBondCurveSvenssonYield( b0, b1, b2, b3, l1, l2, fittedBondMaturities ))
