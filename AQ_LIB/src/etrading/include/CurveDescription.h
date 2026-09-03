// CurveDescription.h

/*
 * @brief			Curve Description Class
 * @Created:		15th July 2017
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once
#include <vector>
#include <memory>

#include "AQLDate.h"
#include "CoreEnumerations.h"
#include "AQLInterpolationBase.h"
#include "Variant.h"

namespace etrading
{
    // =============================== CURVE DESCRIPTION CLASS ==================================================================

    // Class to Manage the Curve Conventions and also LA Curve Naming Convention(s)
    class CurveDescription
    {
    
    public:
        CurveDescription() {};
        ~CurveDescription() {};
        
        // Copy Constructor
        CurveDescription( const CurveDescription& rhs );

		// Clone
		std::shared_ptr<CurveDescription> clone() const;

        // Assignment Operator
        CurveDescription & operator=( const CurveDescription & rhs );

        // Main Constructor
        CurveDescription( const CurveTypeEnum & curveTypeEnum, 
                          const CurveTenorEnum & curveTenorEnum,
                          const std::string & curveCollection,
                          const std::string & indexName,
                          const std::string & objectPoolLookupTable = std::string() );
        
        // Accessors
        CurveTypeEnum curveTypeEnum() const             { return curveTypeEnum_; }
        std::string curveType() const                   { return curveType_; }

        CurveTenorEnum curveTenorEnum() const           { return curveTenorEnum_; }
        std::string curveTenor() const                  { return curveTenor_; }

        std::string curveCollection() const             { return curveCollection_; }
        std::string curveIndexList() const              { return indexName_; }
        std::string firstCurveIndex() const;
        std::string objectPoolLookupTable() const       { return objectPoolLookupTable_; }
        
    private:

        CurveTypeEnum curveTypeEnum_; 
        std::string curveType_;

        CurveTenorEnum curveTenorEnum_;
        std::string curveTenor_;

        std::string curveCollection_;
        std::string indexName_;
        std::string objectPoolLookupTable_;
        
    };

}
