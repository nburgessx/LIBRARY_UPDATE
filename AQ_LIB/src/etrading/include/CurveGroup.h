// CurveGroup.h

/*
 * @brief			Curve Group Class
 * @Created:		4th February 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once
#include <vector>
#include <string>
#include <memory>

namespace etrading
{
    // =============================== CURVE GROUP CLASS ==================================================================

    // Class to Manage a Group of Curve Handle Objects for the Single-Curve and Multi-Curve Framework
    class CurveGroup
    {
    
    public:
        CurveGroup() {};
        ~CurveGroup() {};
        
        // Copy Constructor
        CurveGroup( const CurveGroup& rhs );

		// Clone
		std::shared_ptr<CurveGroup> clone() const;

        // Assignment Operator
        CurveGroup & operator=( const CurveGroup & rhs );

        // Main Constructor
		CurveGroup( const std::vector<std::string> & curveHandles );
		
        // Accessors
		std::vector<std::string> curveHandles()	const		{ return curveHandles_; }
		std::string curveCollection()	const				{ return curveCollection_; }

    private:

		std::vector<std::string> curveHandles_;
		std::string curveCollection_;
    };

}
