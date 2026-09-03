// JSONInfoBlock.h

/*
 * @brief			Class to Manage the JSON InfoBlock Container for LWO Serialization
 * @Created:		24th September 2018
 * @Author:			Nicholas Burgess
 * @Department:		Mizuho International, Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include <string>
#include <vector>
#include "Variant.h"
#include "LWOUtilities.h"

namespace etrading
{
    // Type Definitions for Brevity
    typedef std::vector<etrading::Variant>                              VariantVector;
    typedef std::vector<std::vector<etrading::Variant> >                VariantMatrix;

    typedef std::vector<std::string>                                    ColumnNames;
    typedef std::vector<etrading::ContainedTypeEnum>                    ColumnDataTypes;
    typedef std::tuple<ColumnNames, ColumnDataTypes, VariantMatrix>     JSONInfoBlockTuple;
    typedef std::vector<JSONInfoBlockTuple>                             JSONInfoBlockTuples;

    typedef VariantMatrix                                               RawInputData;
    typedef VariantMatrix                                               JSONFormattedData; // Transposed
    

    class JSONInfoBlock
    {
    
    public:

        // Constructor / Destructor
        JSONInfoBlock() {};
        virtual ~JSONInfoBlock() {};

        // Alternative Constructor
        JSONInfoBlock(const RawInputData & rawInputData );

        // Copy Constructor
        JSONInfoBlock( const JSONInfoBlock & rhs );
        
        // Assignment Operator
        JSONInfoBlock & operator=( const JSONInfoBlock & rhs );
        
        // Accessors
        ColumnNames         columnNames() const          { return columnNames_;          }
        ColumnDataTypes     columnDataTypes() const      { return columnDataTypes_;      }
        RawInputData        rawInputData() const         { return rawInputData_;         }
        JSONFormattedData   jsonFormattedData() const    { return jsonFormattedData_;    }
        JSONInfoBlockTuple  jsonInfoBlockTuple() const   { return jsonInfoBlockTuple_;   }


        // Static Helper Method
        static JSONInfoBlockTuples trimInfoBlocks( const std::vector<std::string>& infoBlockNames,
                                                   const JSONInfoBlockTuples& infoBlocks,
                                                   const bool checkForErrors = true,
                                                   const bool enforceVariantDataType = false );
    
        // Static Helper Method
        static JSONInfoBlockTuple trimInfoBlock( const JSONInfoBlockTuple& infoBlock,
                                                 const bool checkForErrors = true,
                                                 const bool enforceVariantDataType = false );

        // Static Helper Method
        static JSONInfoBlockTuple trimInfoBlock( const JSONInfoBlockTuple & infoBlock,
                                                 const std::string & infoBlockName = "InfoBlockData",
                                                 const bool checkForErrors = true,
                                                 const bool enforceVariantDataType = false );

        // Static Helper Method
        static JSONInfoBlockTuple createInfoBlock( const VariantMatrix & dataMatrix, const bool enforceVariantDataType = true );

    private:
        
        // Member Data
        ColumnNames         columnNames_;
        ColumnDataTypes     columnDataTypes_;
        RawInputData        rawInputData_;
        JSONFormattedData   jsonFormattedData_;
        JSONInfoBlockTuple  jsonInfoBlockTuple_;
    };

}
