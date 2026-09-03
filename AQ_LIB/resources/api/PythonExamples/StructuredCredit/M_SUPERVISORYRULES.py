import MLIB_CLIENT_API as m
print ( m.setUpMLIB("","") )    #Load Calendars and Config Files
print ( m.meUtilityVersion() )  #Get the MLIB Version

creditEnhancementLevel = 0.2
trancheThickness = 0.13
kirb = 0.12
elgd = 0.45
nEffectiveExposures = 1

riskWeight = m.meSRTSupervisoryFormula(creditEnhancementLevel, trancheThickness, kirb, elgd, nEffectiveExposures )
riskWeight


poolType="Wholesale"
nEffectiveExposures = 100
rankType="Senior"
kirb = 0.06675
lgd = 0.4
trancheMaturity = 5.
attachmentPoint = 0.15
detatchmentPoint = 1.0
rwFloor = 0.15
riskWeight2 = m.meSRTSecIrbaRiskWeight(  poolType, nEffectiveExposures, rankType, kirb, lgd, trancheMaturity, attachmentPoint, detatchmentPoint, rwFloor )
riskWeight2

poolType="Retail"
nEffectiveExposures = 21
rankType="NonSenior"
kirb = 0.22
lgd = 0.4
trancheMaturity = 5.81
attachmentPoint = 0.15
detatchmentPoint = 1.0
rwFloor = 0.15
riskWeight3 = m.meSRTSecIrbaRiskWeight(  poolType, nEffectiveExposures, rankType, kirb, lgd, trancheMaturity, attachmentPoint, detatchmentPoint, rwFloor )
riskWeight3

poolType="Retail"
nEffectiveExposures = 26
rankType="NonSenior"
kirb = 0.22
lgd = 0.4
trancheMaturity = 5.81
attachmentPoint = 0.15
detatchmentPoint = 1.0
rwFloor = 0.15
riskWeight4 = m.meSRTSecIrbaRiskWeight(  poolType, nEffectiveExposures, rankType, kirb, lgd, trancheMaturity, attachmentPoint, detatchmentPoint, rwFloor )
riskWeight4


poolType="Wholesale"
nEffectiveExposures = 1802
rankType="NonSenior"
kirb = 0.1198
lgd = 0.4
trancheMaturity = 5.
attachmentPoint = 0.0548
detatchmentPoint = 0.13
rwFloor = 0.15
riskWeight5 = m.meSRTSecIrbaRiskWeight(  poolType, nEffectiveExposures, rankType, kirb, lgd, trancheMaturity, attachmentPoint, detatchmentPoint, rwFloor )
riskWeight5


poolType="Wholesale"
nEffectiveExposures = 1802
rankType="NonSenior"
kirb = 0.1198
lgd = 0.
trancheMaturity = 5.
attachmentPoint = 0.0548
detatchmentPoint = 0.13
rwFloor = 0.15
riskWeight6 = m.meSRTSecIrbaRiskWeight(  poolType, nEffectiveExposures, rankType, kirb, lgd, trancheMaturity, attachmentPoint, detatchmentPoint, rwFloor )
riskWeight6



