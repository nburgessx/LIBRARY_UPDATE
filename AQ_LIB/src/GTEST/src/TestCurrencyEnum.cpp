// TestCurrencyEnum.cpp

// Include: Google Test Library
#include <gTest/gTest.h>

#include "InitializeGoogleTest.h"	// DECLARE_TEST_FIXTURE
#include "CoreEnumerations.h"

namespace google_test
{
	TEST( TestCurrencyEnum, CurrencyToString )
	{
		/* Null Case */
		EXPECT_EQ( "", etrading::toString( etrading::NO_CCY ) );

		/* Majors */
		EXPECT_EQ( "AUD", etrading::toString( etrading::AUD ) );	// Australia Dollar
		EXPECT_EQ( "EUR", etrading::toString( etrading::EUR ) );	// Europe Euro
		EXPECT_EQ( "GBP", etrading::toString( etrading::GBP ) );	// United Kingdom Pound
		EXPECT_EQ( "JPY", etrading::toString( etrading::JPY ) );	// Japan Yen
		EXPECT_EQ( "NZD", etrading::toString( etrading::NZD ) );	// New Zealand Dollar
		EXPECT_EQ( "USD", etrading::toString( etrading::USD ) );	// United States Dollar
		
		/* Minors */
		EXPECT_EQ( "AED", etrading::toString( etrading::AED ) );	// United Arab Emirates Dirham
		EXPECT_EQ( "ARS", etrading::toString( etrading::ARS ) );	// Argentina Peso
		EXPECT_EQ( "BRL", etrading::toString( etrading::BRL ) );	// Brazil Real
		EXPECT_EQ( "CAD", etrading::toString( etrading::CAD ) );	// Canada Dollar
		EXPECT_EQ( "CHF", etrading::toString( etrading::CHF ) );	// Swiss Franc
		EXPECT_EQ( "CNH", etrading::toString( etrading::CNH ) );	// China Yuan (HK Offshore)
		EXPECT_EQ( "CNY", etrading::toString( etrading::CNY ) );	// China Yuan (Onshore)
		EXPECT_EQ( "CLP", etrading::toString( etrading::CLP ) );	// Chile Peso
		EXPECT_EQ( "COP", etrading::toString( etrading::COP ) );	// Colombia Peso
		EXPECT_EQ( "CZK", etrading::toString( etrading::CZK ) );	// Czech Koruna
		EXPECT_EQ( "DKK", etrading::toString( etrading::DKK ) );	// Danish Krone
		EXPECT_EQ( "EGP", etrading::toString( etrading::EGP ) );	// Egyptian Pound
		EXPECT_EQ( "HKD", etrading::toString( etrading::HKD ) );	// Hong Kong Dollar
		EXPECT_EQ( "HRK", etrading::toString( etrading::HRK ) );	// Croatia Kuna
		EXPECT_EQ( "HUF", etrading::toString( etrading::HUF ) );	// Hungary Forint
		EXPECT_EQ( "INR", etrading::toString( etrading::INR ) );	// India Rupee
		EXPECT_EQ( "IDR", etrading::toString( etrading::IDR ) );	// Indonesia Rupiah
		EXPECT_EQ( "ILS", etrading::toString( etrading::ILS ) );	// Israel Shekel
		EXPECT_EQ( "KRW", etrading::toString( etrading::KRW ) );	// South Korea Won
		EXPECT_EQ( "MAD", etrading::toString( etrading::MAD ) );	// Morocco Dirham
		EXPECT_EQ( "MXN", etrading::toString( etrading::MXN ) );	// Mexico Peso
		EXPECT_EQ( "MYR", etrading::toString( etrading::MYR ) );	// Malaysia Ringgit
		EXPECT_EQ( "NGN", etrading::toString( etrading::NGN ) );	// Nigeria Naira
		EXPECT_EQ( "NOK", etrading::toString( etrading::NOK ) );	// Norway Krone
		EXPECT_EQ( "OMR", etrading::toString( etrading::OMR ) );	// Oman Rial
		EXPECT_EQ( "PHP", etrading::toString( etrading::PHP ) );	// Philippines Peso
		EXPECT_EQ( "PLN", etrading::toString( etrading::PLN ) );	// Poland Zloty
		EXPECT_EQ( "QAR", etrading::toString( etrading::QAR ) );	// Qatar Riyal
		EXPECT_EQ( "RON", etrading::toString( etrading::RON ) );	// Romania Leu
		EXPECT_EQ( "RSD", etrading::toString( etrading::RSD ) );	// Serbia Dinar
		EXPECT_EQ( "RUB", etrading::toString( etrading::RUB ) );	// Russia Ruble
		EXPECT_EQ( "SAR", etrading::toString( etrading::SAR ) );	// Saudi Arabia Riyal
        EXPECT_EQ( "SEK", etrading::toString( etrading::SEK ) );	// Sweden Krone
        EXPECT_EQ( "SGD", etrading::toString( etrading::SGD ) );	// Singapore Dollar
		EXPECT_EQ( "TWD", etrading::toString( etrading::TWD ) );	// Taiwan Dollar
		EXPECT_EQ( "THB", etrading::toString( etrading::THB ) );	// Thailand Baht
		EXPECT_EQ( "TRY", etrading::toString( etrading::TRY ) );	// Turkey Lira
		EXPECT_EQ( "UAH", etrading::toString( etrading::UAH ) );	// Ukraine Hryvnia
		EXPECT_EQ( "VND", etrading::toString( etrading::VND ) );	// Vietnam Dong
		EXPECT_EQ( "ZAR", etrading::toString( etrading::ZAR ) );	// South Africa Rand
	}

	TEST( TestCurrencyEnum, CurrencyToEnum )
	{
		/* Null Case */
		EXPECT_EQ( etrading::NO_CCY, etrading::toCCYEnum( "" ) );
		EXPECT_EQ( etrading::NO_CCY, etrading::toCCYEnum( "NONE" ) );

		/* Majors */
		EXPECT_EQ( etrading::AUD, etrading::toCCYEnum( "AUD" ) );	// Australia Dollar
		EXPECT_EQ( etrading::EUR, etrading::toCCYEnum( "EUR" ) );	// Europe Euro
		EXPECT_EQ( etrading::GBP, etrading::toCCYEnum( "GBP" ) );	// United Kingdom Pound
		EXPECT_EQ( etrading::JPY, etrading::toCCYEnum( "JPY" ) );	// Japan Yen
		EXPECT_EQ( etrading::NZD, etrading::toCCYEnum( "NZD" ) );	// New Zealand Dollar
		EXPECT_EQ( etrading::USD, etrading::toCCYEnum( "USD" ) );	// United States Dollar
		
		/* Minors */
		EXPECT_EQ( etrading::AED, etrading::toCCYEnum( "AED" ) );	// United Arab Emirates Dirham
		EXPECT_EQ( etrading::ARS, etrading::toCCYEnum( "ARS" ) );	// Argentina Peso
		EXPECT_EQ( etrading::BRL, etrading::toCCYEnum( "BRL" ) );	// Brazil Real
		EXPECT_EQ( etrading::CAD, etrading::toCCYEnum( "CAD" ) );	// Canada Dollar
		EXPECT_EQ( etrading::CHF, etrading::toCCYEnum( "CHF" ) );	// Swiss Franc
		EXPECT_EQ( etrading::CNH, etrading::toCCYEnum( "CNH" ) );	// China Yuan (HK Offshore)
		EXPECT_EQ( etrading::CNY, etrading::toCCYEnum( "CNY" ) );	// China Yuan (Onshore)
		EXPECT_EQ( etrading::CLP, etrading::toCCYEnum( "CLP" ) );	// Chile Peso
		EXPECT_EQ( etrading::COP, etrading::toCCYEnum( "COP" ) );	// Colombia Peso
		EXPECT_EQ( etrading::CZK, etrading::toCCYEnum( "CZK" ) );	// Czech Koruna
		EXPECT_EQ( etrading::DKK, etrading::toCCYEnum( "DKK" ) );	// Danish Krone
		EXPECT_EQ( etrading::EGP, etrading::toCCYEnum( "EGP" ) );	// Egyptian Pound
		EXPECT_EQ( etrading::HKD, etrading::toCCYEnum( "HKD" ) );	// Hong Kong Dollar
		EXPECT_EQ( etrading::HRK, etrading::toCCYEnum( "HRK" ) );	// Croatia Kuna
		EXPECT_EQ( etrading::HUF, etrading::toCCYEnum( "HUF" ) );	// Hungary Forint
		EXPECT_EQ( etrading::INR, etrading::toCCYEnum( "INR" ) );	// India Rupee
		EXPECT_EQ( etrading::IDR, etrading::toCCYEnum( "IDR" ) );	// Indonesia Rupiah
		EXPECT_EQ( etrading::ILS, etrading::toCCYEnum( "ILS" ) );	// Israel Shekel
		EXPECT_EQ( etrading::KRW, etrading::toCCYEnum( "KRW" ) );	// South Korea Won
		EXPECT_EQ( etrading::MAD, etrading::toCCYEnum( "MAD" ) );	// Morocco Dirham
		EXPECT_EQ( etrading::MXN, etrading::toCCYEnum( "MXN" ) );	// Mexico Peso
		EXPECT_EQ( etrading::MYR, etrading::toCCYEnum( "MYR" ) );	// Malaysia Ringgit
		EXPECT_EQ( etrading::NGN, etrading::toCCYEnum( "NGN" ) );	// Nigeria Naira
		EXPECT_EQ( etrading::NOK, etrading::toCCYEnum( "NOK" ) );	// Norway Krone
		EXPECT_EQ( etrading::OMR, etrading::toCCYEnum( "OMR" ) );	// Oman Rial
		EXPECT_EQ( etrading::PHP, etrading::toCCYEnum( "PHP" ) );	// Philippines Peso
		EXPECT_EQ( etrading::PLN, etrading::toCCYEnum( "PLN" ) );	// Poland Zloty
		EXPECT_EQ( etrading::QAR, etrading::toCCYEnum( "QAR" ) );	// Qatar Riyal
		EXPECT_EQ( etrading::RON, etrading::toCCYEnum( "RON" ) );	// Romania Leu
		EXPECT_EQ( etrading::RSD, etrading::toCCYEnum( "RSD" ) );	// Serbia Dinar
		EXPECT_EQ( etrading::RUB, etrading::toCCYEnum( "RUB" ) );	// Russia Ruble
		EXPECT_EQ( etrading::SAR, etrading::toCCYEnum( "SAR" ) );	// Saudi Arabia Riyal
        EXPECT_EQ( etrading::SEK, etrading::toCCYEnum( "SEK" ) );	// Sweden Krone
        EXPECT_EQ( etrading::SGD, etrading::toCCYEnum( "SGD" ) );	// Singapore Dollar
		EXPECT_EQ( etrading::TWD, etrading::toCCYEnum( "TWD" ) );	// Taiwan Dollar
		EXPECT_EQ( etrading::THB, etrading::toCCYEnum( "THB" ) );	// Thailand Baht
		EXPECT_EQ( etrading::TRY, etrading::toCCYEnum( "TRY" ) );	// Turkey Lira
		EXPECT_EQ( etrading::UAH, etrading::toCCYEnum( "UAH" ) );	// Ukraine Hryvnia
		EXPECT_EQ( etrading::VND, etrading::toCCYEnum( "VND" ) );	// Vietnam Dong
		EXPECT_EQ( etrading::ZAR, etrading::toCCYEnum( "ZAR" ) );	// South Africa Rand
	}

}