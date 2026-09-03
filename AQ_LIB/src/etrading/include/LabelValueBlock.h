#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include "AQLCoreTemplateType.h"
#include "KeyValueLookupTable.h"
#include "Variant.h"

namespace etrading
{
    // Class to Create a LabelValueBlock, which is an key-value lookup table made from an unordered map.
    class LabelValueBlock : public KeyValueLookupTable<std::string>
    {
    public:

        // Constructors
        LabelValueBlock() {};
        LabelValueBlock( const AQLStringMatrix& input, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const StandardStringMatrix& input, const bool& makeAllKeysUppercase = true );

        // Single Key-Value Constructors
        LabelValueBlock( const AQLString & key, const AQLString & value, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const StandardString & key, const StandardString & value, const bool& makeAllKeysUppercase = true );
        
        // Multiple Key-Value Constructors
        LabelValueBlock( const AQLStringVector & keys, const AQLStringVector & values, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const StandardStringVector & keys, const StandardStringVector & values, const bool& makeAllKeysUppercase = true );
        
        // Append Constructors
        LabelValueBlock( const LabelValueBlock& originalLVB, const LabelValueBlock& appendLVB, const bool& makeAllKeysUppercase = true ); 
        LabelValueBlock( const LabelValueBlock& originalLVB, const AQLString& appendKey, const AQLString& appendValue, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const LabelValueBlock& originalLVB, const AQLStringVector& appendKeys, const AQLStringVector& appendValues, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const LabelValueBlock& originalLVB, const StandardString& appendKey, const StandardString& appendValue, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const LabelValueBlock& originalLVB, const StandardStringVector& appendKeys, const StandardStringVector& appendValues, const bool& makeAllKeysUppercase = true );

        // Validation Method(s)
        void checkForDuplicateKeys(const std::vector<std::string>& keys) const;
        bool isEmpty() const;

        // Static Helpers
        static LabelValueBlock createLabelValueBlockUsingLAString( const AQLString& key, const AQLString& value, const bool& makeAllKeysUppercase = true );
        static LabelValueBlock createLabelValueBlock( const std::string& key, const std::string& value, const bool& makeAllKeysUppercase = true );

        // Convert LVB to a Matrix Type
        AQLStringMatrix          toLAStringMatrix() const;
        StandardStringMatrix    toStandardStringMatrix() const;
        AnyTypeMatrix           toAnyTypeMatrix() const;
        VariantMatrix           toVariantMatrix() const;

        // Get LVB Key Value
        bool getOptionalValueAsBool( const std::string& key, const bool& defaultVal ) const;
        bool getCompulsoryValueAsBool( const std::string& key, const std::string& labelValueBlockName = "" ) const;

        unsigned int getOptionalValueAsUnsignedInt( const std::string& key, unsigned int defaultVal = 0 ) const;
        
        double getCompulsoryValueAsDouble( const std::string& key, const std::string& labelValueBlockName = "" ) const;

		int getCompulsoryValueAsInt(const std::string& key, const std::string& labelValueBlockName = "") const;

        double getOptionalValueAsDouble( const std::string& key, double defaultVal = 0.0 ) const;
		
		int getOptionalValueAsInt(const std::string& key, int defaultVal = 0) const;
		
		double getOptionalValueAsDoubleFromKeys( const std::string& key1, const std::string& key2, double defaultVal = 0.0 ) const;
        double getCompulsoryValueAsDoubleFromKeys( const std::string& key1, const std::string& key2, const std::string& labelValueBlockName ) const;
        
        AQLDate getCompulsoryValueAsDate( const std::string& key, const std::string& labelValueBlockName = "" ) const;
        AQLDate getOptionalValueAsDate( const std::string& key, const AQLDate& defaultVal = AQLDate() ) const;
        AQLDate getOptionalDateOrTenorAsDate( const std::string& key, const AQLDate& referenceDate, const AQLDate& defaultVal = AQLDate() ) const;
        AQLDate getCompulsoryDateOrTenorAsDate( const std::string& key, const AQLDate& referenceDate, const std::string& labelValueBlockName = "" ) const;
        
        std::string getCompulsoryValueAsString(const std::string& key, const std::string& labelValueBlockName = "", bool makeKeyUppercase = false) const;
        std::string getOptionalValueAsString(const std::string& key, const std::string& defaultVal = "") const;
        std::string getOptionalValueAsStringFromKeys(const std::string& key1, const std::string& key2, const std::string& defaultVal = "") const;
        std::string getOptionalValueAsStringFromMultipleKeys(const std::vector<std::string>& keys, const std::string& defaultVal = "") const;

        AQLString getCompulsoryValueAsLAString( const std::string& key, const std::string& labelValueBlockName = "", bool makeKeyUppercase = false ) const;
        AQLString getOptionalValueAsLAString( const std::string& key, const AQLString& defaultVal = "") const;
        AQLString getOptionalValueAsLAStringFromKeys( const std::string& key1, const std::string& key2, const AQLString& defaultVal = "") const;
        AQLString getOptionalValueAsLAStringFromMultipleKeys( const std::vector<std::string>& keys, const AQLString& defaultVal = "") const;
        AQLString getCompulsoryValueAsLAStringFromKeys(const std::string& key1, const std::string& key2, const std::string& labelValueBlockName) const;
        AQLString getCompulsoryValueAsLAStringFromMultipleKeys( const std::vector<std::string>& keys, const std::string& labelValueBlockName) const;
        
    private:

        // Private Helper Method to Construct a LVB from a AQLStringMatrix
        void initializefromLAStringMatrix( const AQLStringMatrix & input, const bool& makeAllKeysUppercase = true );
        
        // Private Helper Method to Construct a LVB from a StandardStringMatrix
        void initializefromStandardStringMatrix( const StandardStringMatrix & input, const bool& makeAllKeysUppercase = true );

        // Accessible via the toLaStringMatrix and to toStandardStringMatrix methods
        AQLStringMatrix laStringMatrix_;         
        StandardStringMatrix standardStringMatrix_;

    };

    // Helper Methods - Building LabelValueBlock(s) from AQLStringMatrix
    // -----------------------------------------------------------------------------

    // Method to build a Single LVB from a AQLStringMatrix
    LabelValueBlock buildSingleLabelValueBlock( const AQLStringMatrix& input, const bool& makeAllKeysUppercase = true );
    
    // Method to build a Single LVB from a vector of keys and a vector of values
    LabelValueBlock buildSingleLabelValueBlock( const StandardStringVector& keys, const StandardStringVector& values, const bool& makeAllKeysUppercase = true );
    
    // Method to build a Vector of LVBs from a AQLStringMatrix
    std::vector<LabelValueBlock> buildMultiLabelValueBlock( const AQLStringMatrix& input, const bool& makeAllKeysUppercase = true );
    

    // Helper Methods - Vector and Matrix Conversion to and from LA- and Standard- string types
    // --------------------------------------------------------------------------------
    
    StandardStringVector convertToStandardStringVector( const AQLStringVector& laStringVector );
    AQLStringVector convertToLAStringVector( const StandardStringVector& standardStringVector );
    
    StandardStringMatrix convertToStandardStringMatrix( const AQLStringMatrix & laStringMatrix );
    AQLStringMatrix convertToLAStringMatrix( const StandardStringMatrix & standardStringMatrix );

    StandardStringMatrix combineStandardStringMatrices( const StandardStringMatrix & matrix1, const StandardStringMatrix & matrix2 );
    AQLStringMatrix combineLAStringMatrices( const AQLStringMatrix & matrix1, const AQLStringMatrix & matrix2 );

    // Helper Methods - Getting Values from LVB String Matrices
    // --------------------------------------------------------------------------------
    
    AQLString searchLAStringMatrix( const AQLString & searchKey, const AQLStringMatrix & searchMatrix );
    StandardString searchStandardStringMatrix( const StandardString & searchKey, const StandardStringMatrix & searchMatrix );

}


