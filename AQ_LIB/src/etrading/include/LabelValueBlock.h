#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include "LACoreTemplateType.h"
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
        LabelValueBlock( const LAStringMatrix& input, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const StandardStringMatrix& input, const bool& makeAllKeysUppercase = true );

        // Single Key-Value Constructors
        LabelValueBlock( const LAString & key, const LAString & value, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const StandardString & key, const StandardString & value, const bool& makeAllKeysUppercase = true );
        
        // Multiple Key-Value Constructors
        LabelValueBlock( const LAStringVector & keys, const LAStringVector & values, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const StandardStringVector & keys, const StandardStringVector & values, const bool& makeAllKeysUppercase = true );
        
        // Append Constructors
        LabelValueBlock( const LabelValueBlock& originalLVB, const LabelValueBlock& appendLVB, const bool& makeAllKeysUppercase = true ); 
        LabelValueBlock( const LabelValueBlock& originalLVB, const LAString& appendKey, const LAString& appendValue, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const LabelValueBlock& originalLVB, const LAStringVector& appendKeys, const LAStringVector& appendValues, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const LabelValueBlock& originalLVB, const StandardString& appendKey, const StandardString& appendValue, const bool& makeAllKeysUppercase = true );
        LabelValueBlock( const LabelValueBlock& originalLVB, const StandardStringVector& appendKeys, const StandardStringVector& appendValues, const bool& makeAllKeysUppercase = true );

        // Validation Method(s)
        void checkForDuplicateKeys(const std::vector<std::string>& keys) const;
        bool isEmpty() const;

        // Static Helpers
        static LabelValueBlock createLabelValueBlockUsingLAString( const LAString& key, const LAString& value, const bool& makeAllKeysUppercase = true );
        static LabelValueBlock createLabelValueBlock( const std::string& key, const std::string& value, const bool& makeAllKeysUppercase = true );

        // Convert LVB to a Matrix Type
        LAStringMatrix          toLAStringMatrix() const;
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
        
        LADate getCompulsoryValueAsDate( const std::string& key, const std::string& labelValueBlockName = "" ) const;
        LADate getOptionalValueAsDate( const std::string& key, const LADate& defaultVal = LADate() ) const;
        LADate getOptionalDateOrTenorAsDate( const std::string& key, const LADate& referenceDate, const LADate& defaultVal = LADate() ) const;
        LADate getCompulsoryDateOrTenorAsDate( const std::string& key, const LADate& referenceDate, const std::string& labelValueBlockName = "" ) const;
        
        std::string getCompulsoryValueAsString(const std::string& key, const std::string& labelValueBlockName = "", bool makeKeyUppercase = false) const;
        std::string getOptionalValueAsString(const std::string& key, const std::string& defaultVal = "") const;
        std::string getOptionalValueAsStringFromKeys(const std::string& key1, const std::string& key2, const std::string& defaultVal = "") const;
        std::string getOptionalValueAsStringFromMultipleKeys(const std::vector<std::string>& keys, const std::string& defaultVal = "") const;

        LAString getCompulsoryValueAsLAString( const std::string& key, const std::string& labelValueBlockName = "", bool makeKeyUppercase = false ) const;
        LAString getOptionalValueAsLAString( const std::string& key, const LAString& defaultVal = "") const;
        LAString getOptionalValueAsLAStringFromKeys( const std::string& key1, const std::string& key2, const LAString& defaultVal = "") const;
        LAString getOptionalValueAsLAStringFromMultipleKeys( const std::vector<std::string>& keys, const LAString& defaultVal = "") const;
        LAString getCompulsoryValueAsLAStringFromKeys(const std::string& key1, const std::string& key2, const std::string& labelValueBlockName) const;
        LAString getCompulsoryValueAsLAStringFromMultipleKeys( const std::vector<std::string>& keys, const std::string& labelValueBlockName) const;
        
    private:

        // Private Helper Method to Construct a LVB from a LAStringMatrix
        void initializefromLAStringMatrix( const LAStringMatrix & input, const bool& makeAllKeysUppercase = true );
        
        // Private Helper Method to Construct a LVB from a StandardStringMatrix
        void initializefromStandardStringMatrix( const StandardStringMatrix & input, const bool& makeAllKeysUppercase = true );

        // Accessible via the toLaStringMatrix and to toStandardStringMatrix methods
        LAStringMatrix laStringMatrix_;         
        StandardStringMatrix standardStringMatrix_;

    };

    // Helper Methods - Building LabelValueBlock(s) from LAStringMatrix
    // -----------------------------------------------------------------------------

    // Method to build a Single LVB from a LAStringMatrix
    LabelValueBlock buildSingleLabelValueBlock( const LAStringMatrix& input, const bool& makeAllKeysUppercase = true );
    
    // Method to build a Single LVB from a vector of keys and a vector of values
    LabelValueBlock buildSingleLabelValueBlock( const StandardStringVector& keys, const StandardStringVector& values, const bool& makeAllKeysUppercase = true );
    
    // Method to build a Vector of LVBs from a LAStringMatrix
    std::vector<LabelValueBlock> buildMultiLabelValueBlock( const LAStringMatrix& input, const bool& makeAllKeysUppercase = true );
    

    // Helper Methods - Vector and Matrix Conversion to and from LA- and Standard- string types
    // --------------------------------------------------------------------------------
    
    StandardStringVector convertToStandardStringVector( const LAStringVector& laStringVector );
    LAStringVector convertToLAStringVector( const StandardStringVector& standardStringVector );
    
    StandardStringMatrix convertToStandardStringMatrix( const LAStringMatrix & laStringMatrix );
    LAStringMatrix convertToLAStringMatrix( const StandardStringMatrix & standardStringMatrix );

    StandardStringMatrix combineStandardStringMatrices( const StandardStringMatrix & matrix1, const StandardStringMatrix & matrix2 );
    LAStringMatrix combineLAStringMatrices( const LAStringMatrix & matrix1, const LAStringMatrix & matrix2 );

    // Helper Methods - Getting Values from LVB String Matrices
    // --------------------------------------------------------------------------------
    
    LAString searchLAStringMatrix( const LAString & searchKey, const LAStringMatrix & searchMatrix );
    StandardString searchStandardStringMatrix( const StandardString & searchKey, const StandardStringMatrix & searchMatrix );

}


