// HandleEnumsTest.cpp

// Includes: This Library
#include <HandleEnums.h>

// Include: Google Test Library
#include <gTest/gTest.h>

/*
// Namespaces
using namespace ETrading::Enums;
using std::string;

TEST( HandleEnumsTest, HandleTypeToString )
{
    // Check Valid Enums
    EXPECT_EQ( string("INVALID"),       toString( HandleType( INVALID ) ) );
    EXPECT_EQ( string("GRID"),          toString( HandleType( GRID )    ) );
    EXPECT_EQ( string("LOOKUP"),        toString( HandleType( LOOKUP )  ) );
    EXPECT_EQ( string("TABLE"),         toString( HandleType( TABLE )   ) );
    EXPECT_EQ( string("MATRIX"),        toString( HandleType( MATRIX )  ) );
    EXPECT_EQ( string("CURVE"),         toString( HandleType( CURVE )   ) );
    EXPECT_EQ( string("SWAP"),          toString( HandleType( SWAP )    ) );
}

TEST( HandleEnumsTest, StringToHandleType )
{
    // Check Valid Enums
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("INVALID") ) );
    EXPECT_EQ( HandleType( GRID ),      toHandleType( string("GRID")    ) );
    EXPECT_EQ( HandleType( LOOKUP ),    toHandleType( string("LOOKUP")  ) );
    EXPECT_EQ( HandleType( TABLE ),     toHandleType( string("TABLE")   ) );
    EXPECT_EQ( HandleType( MATRIX ),    toHandleType( string("MATRIX")  ) );
    EXPECT_EQ( HandleType( CURVE ),     toHandleType( string("CURVE")   ) );
    EXPECT_EQ( HandleType( SWAP ),      toHandleType( string("SWAP")    ) );
}

TEST( HandleEnumsTest, ConvertingInvalidStringToHandleType )
{
    // Check Valid Enums
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("invalid") ) );
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("BadType") ) );
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("abcdefg") ) );
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("0123456") ) );
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("@@@@@@@") ) );
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("#######") ) );
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("_______") ) );
    EXPECT_EQ( HandleType( INVALID ),   toHandleType( string("-------") ) );
}

TEST( HandleEnumsTest, TestingListHandleTypes )
{
    // List all the available handle types
    const string expected = string("INVALID, GRID, LOOKUP, TABLE, MATRIX, CURVE, SWAP");
    const string result   = listHandleTypes();
    EXPECT_EQ( expected, result );
}
*/