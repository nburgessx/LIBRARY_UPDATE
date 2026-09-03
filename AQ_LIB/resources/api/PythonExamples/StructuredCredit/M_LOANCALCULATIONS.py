import MLIB_CLIENT_API as m
print ( m.setUpMLIB("","") )    #Load Calendars and Config Files
print ( m.meUtilityVersion() )  #Get the MLIB Version


# Loan Calculations

term1 = 50
loanRate =0.05
originalBalance = 100.
loanType = "Bullet"
calculatedValue1 = m.meSRTLoanPayment( term1, loanRate, originalBalance, loanType )
calculatedValue1

term2 = 50
loanType2 = "Amortizing"
calculatedValue2 = m.meSRTLoanPayment( term2, loanRate, originalBalance, loanType2 )
calculatedValue2

term3 = 100
loanType3 = "Amortizing"
calculatedValue3 = m.meSRTLoanPayment( term3, loanRate, originalBalance, loanType3 )
calculatedValue3

period1 = 0
loanPrincipal1 = m.meSRTLoanPrincipal( term1, loanRate, originalBalance, period1, loanType )
loanPrincipal1

period2 = 50
loanPrincipal2 = m.meSRTLoanPrincipal( term1, loanRate, originalBalance, period2, loanType )
loanPrincipal2

period3 = 0
loanPrincipal3 = m.meSRTLoanPrincipal( term1, loanRate, originalBalance, period3, loanType3 )
loanPrincipal3

period4 = 50
loanType4 = "Amortizing"
loanPrincipal4 = m.meSRTLoanPrincipal( term1, loanRate, originalBalance, period4, loanType4 )
loanPrincipal4

period5 = 25
loanType5 = "Amortizing"
loanPrincipal5 = m.meSRTLoanPrincipal( term1, loanRate, originalBalance, period5, loanType5 )
loanPrincipal5


