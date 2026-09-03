#pragma once


#include <memory>

#include "CurveData.h"


namespace etrading
{
    struct HasCurveData
    {
        HasCurveData() : ptrMktData_() {};
        void setCurveData( const std::shared_ptr<CurveData>& );
        std::shared_ptr<CurveData> getCurveData();
    private:
        std::shared_ptr<CurveData> ptrMktData_;
    };
};