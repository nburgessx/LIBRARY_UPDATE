#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <AQLDataInstance.h>
#include <AQLDataBasics.h>
#include <AQLCoreTemplateType.h>
#include "AQLInterpolationBase.h"
#include "AQLNl2sol.h"
#include <LAMathYieldCurve.h>
#include "AQLSplineInterpolation.h"
#include <AQLMathDefine.h>
#include <LAMathDateCalculations.h>
#include "ConstantDeclarations.h"
#include "LAMathCapFloorSABR.h"
//#include "LAMathAnalyticalBlack.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathCurveFuncUtility.h"

#include "AQLCoreComponentManager.h"
#include "LAAnalyticFormula.h"
#include "LAMathSABR.h"

/*!
	@brief Default constructor
*/
LAMathCapFloorConvention::LAMathCapFloorConvention
    ( const AQLString& freq_,
      const AQLString& spotLag_, const AQLString& daycount_, const AQLString& paySlr_,
      const AQLString& payCal_, const AQLString& fixCal_ )
:freq(freq_), spotLag(spotLag_), daycount(daycount_)
{
    paySlr.convertFromString(paySlr_);
    payCal.convertFromString(payCal_);
    fixCal.convertFromString(fixCal_);
}
/*!
	@brief Default constructor
*/
LAMathCapFloorConvention::LAMathCapFloorConvention
    ( const AQLString& freq_,
      const AQLString& spotLag_, const AQLString& daycount_, const AQLPriceDataSlidingRule& paySlr_,
      const AQLPriceDataCalendar& payCal_, const AQLPriceDataCalendar& fixCal_ )
:freq(freq_), spotLag(spotLag_), daycount(daycount_), paySlr(paySlr_), payCal(payCal_), fixCal(fixCal_) {}
/*!
	@brief Default destructor
*/
LAMathCapFloorConvention::~LAMathCapFloorConvention(){}

/*!
	@brief Default constructor
*/
LAMathCapFloorSABR::LAMathCapFloorSABR
    ( const IntVector& expiDate_, const vector<LAMathSABR_Hagan>& sabr_, 
      AQLInterpolationBase* pInter_, const LAMathCapFloorConvention& conv_, const AQLString& curveID_,
	  AQLString foreCurveName_, AQLString dfCurveName_)
:expiDate(expiDate_), sabr(sabr_), pInter(pInter_), conv(conv_), curveID(curveID_), foreCurveName(foreCurveName_),
 dfCurveName(dfCurveName_)
{
    if( expiDate_.size() != sabr_.size() )
    {
        throw AQLCoreInvalidData("sizes are not same.",__FILE__,__LINE__);
    }
}
                                   
LAMathCapFloorSABR::~LAMathCapFloorSABR(){}          

LAMathSABR_Hagan 
LAMathCapFloorSABR::getSABRParam(unsigned int pos)
{
    if( pos >= sabr.size() ) throw AQLCoreInvalidData("position is over.",__FILE__,__LINE__);

    return sabr[pos];
}

void                                 
LAMathCapFloorSABR::calibrateToCapVol
    ( AQLDataInstance* dataInstance, LAMathSABRLimiter sabrLimiter, 
      const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec, 
      const IntVector& capTerm_mon, const DoubleVector& weight, 
      const AQLString& target, DoubleMatrix& capletVol, DoubleMatrix& capVol )
{
    capletVol.clear(); capVol.clear();
    const double eps = 0.000000001;

    setCalibrationStack(dataInstance);
    unsigned int span = LAMathYieldCurve::setSpanFromFrequency( conv.freq );

    if( capVol_mk.size() != capTerm_mon.size() || capVol_mk[0].size() != strikeVec.size() )
    {
        throw AQLCoreInvalidData("sizes are not same.",__FILE__,__LINE__);
    }

    size_t rowPos_mk,colPos;
    IntVector expiVec;
    for(size_t i=1; i*span < capTerm_mon.back(); i++)      
    {
        expiVec.push_back( i*span );
    }

    capVol.resize(expiVec.size()); capletVol.resize(expiVec.size());
    for(size_t i=0;i<expiVec.size();i++)
    {
        capVol[i].resize(strikeVec.size()); capletVol[i].resize(strikeVec.size());
    }

    IntVector staPos(expiVec.size(),0);
    size_t colum,row_mk,row=0;
    for(row_mk=0; row_mk<capTerm_mon.size(); row_mk++)
    {
        for(colum=0; colum<strikeVec.size(); colum++)
        {
            if(capVol_mk[row_mk][colum] >= eps || capVol_mk[row_mk][colum] <= -eps) break;
        }
        for(; (row+1)*span <= capTerm_mon[row_mk]-span; row++) 
        {
            staPos[row] = colum;
        }
    }
    
    rowPos_mk=0; colPos=0;
    size_t rowPos_caplet=0;
    DoubleArray x,y; //for variable of interpolation 
    bool isMap = false;
    while( rowPos_mk<capTerm_mon.size() && colPos<=strikeVec.size() )
    {
		double capMkVol = 0.;
		if(colPos<strikeVec.size()) capMkVol = capVol_mk[rowPos_mk][colPos];
		int capTerm = 0;
		if(rowPos_mk < capTerm_mon.size() - 1) capTerm = capTerm_mon[rowPos_mk+1];
        if( capMkVol<eps )
        {
			for(; span*(rowPos_caplet+1)<=capTerm-span; rowPos_caplet++)
			{
				if( rowPos_mk+1>=capTerm_mon.size() ) break;
				for(size_t j=staPos[rowPos_caplet]; j<colPos; j++)
				{
					getCapletVol(rowPos_caplet,j,capletVol,strikeVec[j],capVol[rowPos_caplet][j]);
				}
				DoubleArray strike_calib = getPartialVector(strikeVec,staPos[rowPos_caplet],colPos-1);
				DoubleArray vol_calib = getPartialVector(capletVol[rowPos_caplet],staPos[rowPos_caplet],colPos-1);
				DoubleArray forward_calib = getConstantVector(F[rowPos_caplet],colPos-staPos[rowPos_caplet]);
				DoubleArray expiry_calib = getConstantVector(Te[rowPos_caplet],colPos-staPos[rowPos_caplet]);
				DoubleArray numeraire_calib = getConstantVector(Nu[rowPos_caplet],colPos-staPos[rowPos_caplet]);
				DoubleArray weight_calib = getPartialVector(weight,staPos[rowPos_caplet],colPos-1);

				LAMathSABRCalibrator calibrator(sabr[rowPos_caplet], strike_calib, vol_calib, forward_calib,
					expiry_calib, numeraire_calib, weight_calib, target, isMap);
				calibrator.setLimiter( sabrLimiter );

				DoubleArray x(sabrLimiter.getParamNum());
				sabrLimiter.getArgument( sabr[rowPos_caplet], x, isMap );
				NL2SOL solver( calibrator );
				solver.tryToSolve( x );
				sabr[rowPos_caplet] = sabrLimiter.getSABR( sabr[rowPos_caplet], x , isMap );

				for(size_t j=0; j<staPos[rowPos_caplet]; j++)
				{
					capletVol[rowPos_caplet][j] = 
						sabr[rowPos_caplet].getSABRVol( Te[rowPos_caplet], F[rowPos_caplet], strikeVec[j] );
				}

				for(size_t j=colPos; j<strikeVec.size(); j++)
				{
					capletVol[rowPos_caplet][j] = 
						sabr[rowPos_caplet].getSABRVol( Te[rowPos_caplet], F[rowPos_caplet], strikeVec[j] );
				}
			}
			rowPos_mk++;
		}
        else 
        {
            for(row_mk=rowPos_mk; row_mk<capTerm_mon.size(); row_mk++)
            {
                if(capVol_mk[row_mk][colPos]>eps)
                {
                    double term = static_cast<double > (capTerm_mon[row_mk]-span) / 12;
                    x.push_back( term );
                    y.push_back( capVol_mk[row_mk][colPos] * capVol_mk[row_mk][colPos] * term );
                }
                else 
                {
                    break;
                }
            }           
            pInter->set(x,y);
            size_t interpoSta;
            if( rowPos_mk == 0 )
            {
                interpoSta = 0;
            }
            else
            {
                interpoSta = capTerm_mon[rowPos_mk]/span-1;
            }

            for(row=interpoSta; span*(row+1)<=capTerm_mon[row_mk-1]-span; row++)
            {
                double term = static_cast<double > (span*(row+1)) / 12;
                double stdDev = pInter->value( term );
                capVol[row][colPos] = AQLMath::sqrt(stdDev / term);
            }
            x.clear();
            y.clear();
            colPos++;
        }
    }

    for(size_t i=0;i<capVol.size();i++)
        for(size_t j=0;j<strikeVec.size();j++)
        {
            capletVol[i][j] = sabr[i].getSABRVol( Te[i], F[i], strikeVec[j] ); 
        }

    for(size_t i=0;i<capVol.size();i++)
        for(size_t j=0;j<strikeVec.size();j++)
        {
            capVol[i][j] = getCapVol(i,j,capletVol,strikeVec[j]); 
        }
}

void
LAMathCapFloorSABR::getCapletVol( unsigned int expiPos, unsigned int columPos, 
                                DoubleMatrix& capletVols, double strike, double aveVol )
{
    if( expiPos>=capletVols.size() ) throw AQLCoreInvalidData("the number of caplet vol is shortage.",__FILE__,__LINE__);
    if( expiPos>=expiDate.size() ) throw AQLCoreInvalidData("expity position is over.",__FILE__,__LINE__);
    if( expiPos<0 ) throw AQLCoreInvalidData("expity position is too small.",__FILE__,__LINE__);
    if( aveVol<=0. ) throw AQLCoreInvalidData("average vol is negative.",__FILE__,__LINE__);

    if( expiPos==0 ) 
    {
        capletVols[expiPos][columPos] = aveVol;
    }
    else
    {
        double capPrem = getCapFloorPrem( expiPos, strike, aveVol );
        double capletPrem = capPrem;

        map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
        AQLString bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
        map<AQLString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
	    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	    LABlackScholesBase* p1 = it->second;
        AnalyticBKParam param; param.K = strike; 
        for(size_t i=0; i<expiPos; i++)
        {
            param.F = F[i]; param.Nu = Nu[i]; param.Te = Te[i]; param.Vol = capletVols[i][columPos];
            if( param.Vol == 0. ) throw AQLCoreInvalidData("caplet vol is 0.",__FILE__,__LINE__); 
            capletPrem -= p1->calc(param);
        }
        
        //Parameter
	    param.F   = F[expiPos];
	    param.K	  = strike;
	    param.Te  = Te[expiPos];
	    param.Nu  = Nu[expiPos];
	    param.ErrorCheck();

	    //Ready for p->calc method2
	    bscomponent = AQLString(BK) + AQLString(VEGA)  + AQLString(CALL) ;
	    var = AQLCoreComponentManager::getBlackComponentMap();
	    it = var.find(bscomponent);
	    LABlackScholesBase* p2 = it->second;

        param.Vol = 0.000001; double lowPrem = p1->calc(param);;
        param.Vol = 10.; double highPrem = p1->calc(param);;
        if( lowPrem>capletPrem )
        {
            AQLString msg = "strike " + AQLString(strike) + " term " + AQLString(expiDate[expiPos]) + "M cap vol is too small.";
            throw AQLCoreInvalidData( msg.getCString(),__FILE__,__LINE__);
        }

        if( highPrem<capletPrem )
        {
            AQLString msg = "strike " + AQLString(strike) + " term " + AQLString(expiDate[expiPos]) + "M cap vol is too big.";
            throw AQLCoreInvalidData( msg.getCString(),__FILE__,__LINE__);
        }
	    //Optimize
	    capletVols[expiPos][columPos]  
        = LAMathIRVanillaFuncUtility::optimize(0.000001, 10., capletPrem, param, p1, p2);//Bisection Method
    }
}

double
LAMathCapFloorSABR::getCapVol( unsigned int expiPos, unsigned int columPos, 
                             const DoubleMatrix& capletVols, double strike )
{
    if( expiPos>=capletVols.size() ) throw AQLCoreInvalidData("the number of caplet vol is shortage.",__FILE__,__LINE__);
    if( expiPos>=expiDate.size() ) throw AQLCoreInvalidData("expity position is over.",__FILE__,__LINE__);
    if( expiPos<0 ) throw AQLCoreInvalidData("expity position is too small.",__FILE__,__LINE__);

    double ret;
    if( expiPos==0 ) 
    {
        ret = capletVols[expiPos][columPos];
    }
    else
    {
        double capPrem = 0.;

        map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
        AQLString bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
        map<AQLString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
	    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	    LABlackScholesBase* p1 = it->second;
        AnalyticBKParam param; param.K = strike; 
        for(size_t i=0; i<=expiPos; i++)
        {
            param.F = F[i]; param.Nu = Nu[i]; param.Te = Te[i]; param.Vol = capletVols[i][columPos];
            if( param.Vol == 0. ) throw AQLCoreInvalidData("caplet vol is 0.",__FILE__,__LINE__); 
            capPrem += p1->calc(param);
        }

        bscomponent = AQLString(CF)+AQLString(PREM)+AQLString(CALL);
        it = var.find(bscomponent); 
	    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
        LABlackScholesBase* p2 = it->second;
        bscomponent = AQLString(CF)+AQLString(VEGA)+AQLString(CALL);
        it = var.find(bscomponent); 
	    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
        LABlackScholesBase* p3 = it->second;

        AnalyticCFParam param2; 
        param2.K = strike; 
        param2.F.push_back(0.0); 
        param2.Nu.push_back(0.0); 
        param2.Te.push_back(0.0);
        for(size_t i=0; i<=expiPos; i++)
        {
            param2.F.push_back( F[i] ); 
            param2.Nu.push_back( Nu[i] ); 
            param2.Te.push_back( Te[i] );
        }

        param2.Vol = 0.000001; double lowPrem = p2->calc(param2);
        param2.Vol = 10.; double highPrem = p2->calc(param2);
        if( lowPrem>capPrem )
        {
            AQLString msg = "strike " + AQLString(strike) + " term " + AQLString(expiDate[expiPos]) + "M cap vol is too small.";
            throw AQLCoreInvalidData( msg.getCString(),__FILE__,__LINE__);
        }

        if( highPrem<capPrem )
        {
            AQLString msg = "strike " + AQLString(strike) + " term " + AQLString(expiDate[expiPos]) + "M cap vol is too big.";
            throw AQLCoreInvalidData( msg.getCString(),__FILE__,__LINE__);
        }

        ret = LAMathIRVanillaFuncUtility::optimize(0.000001, 10., capPrem, param2, p2, p3);
    }

    return ret;
}

void
LAMathCapFloorSABR::setCalibrationStack(AQLDataInstance* dataInstance)
{
    Nu.clear(); F.clear(); Te.clear();

    unsigned int span = LAMathYieldCurve::setSpanFromFrequency( conv.freq );

    for(size_t i=0; i<static_cast<int>(expiDate.back()/span); i++)
        if( expiDate[i] != span * (i+1) ) throw AQLCoreInvalidData("frequency and expiry date are inconsistent.",__FILE__,__LINE__);
    
    AQLPriceDataSlidingRule slr_Pre; slr_Pre.convertFromString(PRE);
    AQLPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);
    const AQLDate asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(curveID,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
    const AQLDate spotDate = LAMathDateCalculations::getDate(asOfDate, conv.spotLag, slr_Fol, &conv.payCal, true);
    const AQLString maxDate = AQLString( static_cast<int>(expiDate.back() + span) ) + AQLString("M");
    const AQLDate endDate = LAMathDateCalculations::getDate(spotDate, maxDate, conv.paySlr, &conv.payCal, true);
    DateVector payVec;
    LAMathDateCalculations::generateSchedule(spotDate, endDate, conv.freq,true,NULL,NULL,0,payVec,&conv.paySlr,&conv.payCal);
    DateVector fixVec(payVec.size());
    for(size_t i =0; i<payVec.size(); i++) fixVec[i]= LAMathDateCalculations::getDate(payVec[i],conv.spotLag,slr_Pre,&conv.payCal,false); 
    //LAMathYieldCurve
	LAMathYieldCurve& yc = LAMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveID);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(conv.daycount);
    yc.getSlidingRule().convertFromString(NO_CH);
    yc.getCalendar() = conv.payCal;
	yc.getFrequency().convertFromString("SIMPLE");

    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLPriceDataDayCount daycount; daycount.convertFromString(conv.daycount);
    
	LAMathYieldCurve* yc2 = dynamic_cast<LAMathYieldCurve* >(yc.clone());
	yc.setCurveType(foreCurveName);
	yc2->setCurveType(dfCurveName);
	for(size_t i=0; i<static_cast<int>(expiDate.back()/span); i++)
    {
		
        F.push_back( yc.getZeroRate(payVec[i],payVec[i+1]) );
        double accruTerm = daycount.getTerm(payVec[i],payVec[i+1]);
		yc.setCurveType(dfCurveName);
        Nu.push_back( yc2->getDF(spotDate,payVec[i+1]) * accruTerm );
        Te.push_back( dc_act365.getTerm(asOfDate,fixVec[i]) );
    }
	delete yc2;
	/*for(size_t i=0; i<static_cast<int>(expiDate.back()/span); i++)
    {
		yc.setCurveType(foreCurveName);
        F.push_back( yc.getZeroRate(payVec[i],payVec[i+1]) );
        double accruTerm = daycount.getTerm(payVec[i],payVec[i+1]);
		yc.setCurveType(dfCurveName);
        Nu.push_back( yc.getDF(spotDate,payVec[i+1]) * accruTerm );
        Te.push_back( dc_act365.getTerm(asOfDate,fixVec[i]) );
    }*/
}

double
LAMathCapFloorSABR::getCapFloorPrem( unsigned int expiPos, double strike, double aveVol )
{
    //error check
    if( expiPos >= F.size() || expiPos <= 0 ) throw AQLCoreInvalidData("expiry position is bad.",__FILE__,__LINE__);

    map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	AQLString bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
    map<AQLString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	LABlackScholesBase* p = it->second;
    
    double ret=0.;
    AnalyticBKParam param; param.Vol = aveVol; param.K = strike; 
    for(size_t i=0; i<=expiPos; i++)
    {
         param.F = F[i]; param.Nu = Nu[i]; param.Te = Te[i];
         ret += p->calc(param);
    }
    return ret;
}

void                                 
LAMathCapFloorSABR::getCapletVolMat
    ( AQLDataInstance* dataInstance, const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec, 
      const IntVector& capTerm_mon, DoubleMatrix& capletVol, DoubleMatrix& capVol )
{
    capletVol.clear(); capVol.clear();
    const double eps = 0.000000001;

    setCalibrationStack(dataInstance);
    unsigned int span = LAMathYieldCurve::setSpanFromFrequency( conv.freq );

    if( capVol_mk.size() != capTerm_mon.size() || capVol_mk[0].size() != strikeVec.size() )
    {
        throw AQLCoreInvalidData("sizes are not same.",__FILE__,__LINE__);
    }
    for(size_t i=0; i<capVol_mk.size(); i++)
    {
        for(size_t j=0; j<strikeVec.size(); j++)
        {
            if(capVol_mk[i][j]<eps) 
                throw AQLCoreInvalidData("market vol is too small .",__FILE__,__LINE__);
        }
    }

    size_t rowPos_mk,colPos;
    IntVector expiVec;
    for(size_t i=1; i*span <= capTerm_mon.back()-span; i++)      
    {
        expiVec.push_back( i*span );
    }

    capVol.resize(expiVec.size()); capletVol.resize(expiVec.size());
    for(size_t i=0;i<expiVec.size();i++)
    {
        capVol[i].resize(strikeVec.size()); capletVol[i].resize(strikeVec.size());
    }
    for(size_t i=0;i<strikeVec.size();i++)
    {
        capVol[0][i] = 0.; capletVol[0][i] = 0.;
    }

    size_t row_mk,row=0;
    
    rowPos_mk=0; colPos=0;
    size_t rowPos_caplet=1;
    DoubleArray x,y; //for variable of interpolation 

    for(colPos=0; colPos<strikeVec.size(); colPos++)
    {
        for(row_mk=rowPos_mk; row_mk<capTerm_mon.size(); row_mk++)
        {
            double term = static_cast<double > (capTerm_mon[row_mk]-span) / 12;
            x.push_back( term );
            y.push_back( capVol_mk[row_mk][colPos] * capVol_mk[row_mk][colPos] * term );
        }           
        pInter->set(x,y);
        for(row=0; row<expiVec.size(); row++)
        {
            double term = static_cast<double > (span*(row+1)) / 12;
            //double term = static_cast<double > (span*row) / 12;
            double stdDev = pInter->value( term );
            capVol[row][colPos] = AQLMath::sqrt(stdDev / term);
        }
        x.clear();
        y.clear();
    }
    
    for(row=0; row<expiVec.size(); row++)
    {
        for(colPos=0; colPos<strikeVec.size(); colPos++)
        {
            getCapletVol(row,colPos,capletVol,strikeVec[colPos],capVol[row][colPos]);
        }
    }
}

double
LAMathCapFloorSABR::getVol( AQLDataInstance* dataInstance, double fixingTerm, double strike )
{
    unsigned int span = LAMathYieldCurve::setSpanFromFrequency( conv.freq );

    DoubleArray x,y;
    setCalibrationStack(dataInstance);
    for(size_t i=0;i<expiDate.size();i++)
    {
        double term = static_cast<double > (expiDate[i]-span) / 12;
        double vol = sabr[i].getSABRVol(term, F[i], strike);
        x.push_back( term );
        y.push_back( vol * vol * term );
    }

    pInter->set(x,y);
    double stdDev =  pInter->value(fixingTerm);

    return AQLMath::sqrt( stdDev / fixingTerm );
}

void                                 
LAMathCapFloorSABR::getCapletVolMat2
    ( AQLDataInstance* dataInstance, const DoubleMatrix& capVol_mk, const DoubleVector& strikeVec, 
      const IntVector& capTerm_mon, DoubleMatrix& capletVol, DoubleMatrix& capVol )
{
    const double eps = 0.000000001;

    setCalibrationStack(dataInstance);
    unsigned int span = LAMathYieldCurve::setSpanFromFrequency( conv.freq );

    if( capVol_mk.size() != capTerm_mon.size() || capVol_mk[0].size() != strikeVec.size() )
    {
        throw AQLCoreInvalidData("sizes are not same.",__FILE__,__LINE__);
    }
    for(size_t i=0; i<capVol_mk.size(); i++)
        for(size_t j=0; j<strikeVec.size(); j++)
        {
            if(capVol_mk[i][j]<eps) 
                throw AQLCoreInvalidData("market vol is too small .",__FILE__,__LINE__);
        }
    if( capTerm_mon[0] != span * 2 )
        throw AQLCoreInvalidData("first cap term is long.",__FILE__,__LINE__);

    DoubleMatrix capletVol_(strikeVec.size(), DoubleVector(1));
    for(size_t j=0; j<strikeVec.size(); j++)
    {
        capletVol_[j][0] = capVol_mk[0][j];
    }
    capletVol.push_back(capVol_mk[0]); capVol.push_back(capVol_mk[0]);
    DoubleArray x(1);
    for(size_t i=1; i<capVol_mk.size(); i++)
    {
        DoubleArray Te_ = Te; Te_.resize(capTerm_mon[i]/span - 1);
        DoubleArray F_ = F; F_.resize(capTerm_mon[i]/span - 1);
        DoubleArray Nu_ = Nu; Nu_.resize(capTerm_mon[i]/span - 1);
        for(size_t j=0; j<strikeVec.size(); j++)
        {
            LAMathCapVolCalibrator calibrator(capletVol_[j],Te_,F_,Nu_,strikeVec[j],capVol_mk[i][j],pInter);
            x[0] = capVol_mk[i][j];
            NL2SOL solver( calibrator );
            solver.tryToSolve( x );
            capletVol_[j] = calibrator.getCapletVol(x);
        }
    }
    //transpose
    capletVol.resize(Te.size(), DoubleVector(strikeVec.size())); 
    for(size_t i=0; i<Te.size(); i++)
    {
        for(size_t j=0; j<strikeVec.size(); j++)
        {
            capletVol[i][j] = capletVol_[j][i]; 
        }
    }
    capVol.resize(Te.size(), DoubleVector(strikeVec.size())); 
    for(size_t i=0; i<Te.size(); i++)
    {
        for(size_t j=0; j<strikeVec.size(); j++)
        {
            capVol[i][j] = getCapVol(i,j,capletVol,strikeVec[j]); 
        }
    }
}

LAMathCapVolCalibrator::LAMathCapVolCalibrator
( const DoubleArray& capletVol_, const DoubleArray& Te_, const DoubleArray& F_, 
  const DoubleArray& Nu_, double strike_, double aveVol_, AQLInterpolationBase* pInter_ )
 : capletVol(capletVol_), Te(Te_), F(F_), Nu(Nu_), strike(strike_),
   aveVol(aveVol_), pInter(pInter_)
{
    volNum = capletVol.size();
    size_t N = Te.size();
    if( volNum >= N ) 
        throw AQLCoreInvalidData("caplet vol size is more than expiry size.",__FILE__,__LINE__);
    if( N != Te.size() || N != F.size() || N != Nu.size() )
        throw AQLCoreInvalidData("parameter sizes are not same.",__FILE__,__LINE__);

    map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	AQLString bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
    map<AQLString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	LABlackScholesBase* p = it->second;

    AnalyticBKParam param; param.K = strike;
    premSum=0.;
    for(size_t i=0; i<volNum; i++)
    {
         param.Vol = capletVol[i]; param.F = F[i]; param.Nu = Nu[i]; param.Te = Te[i];
         premSum += p->calc(param);
    }
    capPrem=0.;
    param.Vol = aveVol;
    for(size_t i=0; i<Te.size(); i++)
    {
         param.F = F[i]; param.Nu = Nu[i]; param.Te = Te[i];
         capPrem += p->calc(param);
    }

    Te_inter.resize(volNum+1); capletVol_inter.resize(volNum+1);
    for(size_t i=0; i<volNum; i++)
    {
         Te_inter[i] = Te[i]; 
         capletVol_inter[i] = capletVol[i];
    }
    Te_inter[volNum] = Te[N-1];
    capletVol_inter[volNum] = aveVol;
    capletVol.resize(N);
}

void
LAMathCapVolCalibrator::operator()(DoubleArray& f, const DoubleArray& x)
{
    capletVol = getCapletVol(x);
    size_t N = Te.size();
    map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	AQLString bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
    map<AQLString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	LABlackScholesBase* p = it->second;
    
    double prem=premSum;
    AnalyticBKParam param;  param.K = strike; 
    for(size_t i=volNum; i<N; i++)
    {
         param.Vol = capletVol[i]; param.F = F[i]; param.Nu = Nu[i]; param.Te = Te[i];
         prem += p->calc(param);
    }
    f[0] = (prem - capPrem) * 10000.;
}

DoubleArray 
LAMathCapVolCalibrator::getCapletVol(const DoubleArray& x)
{
    size_t N = Te.size();
    capletVol_inter[volNum] = x[0];
    pInter->set(Te_inter, capletVol_inter);
    
    for(size_t i=volNum; i<N; i++)
    {
        capletVol[i] = pInter->value(Te[i]);
    }

    return capletVol;
}
bool 
LAMathCapVolCalibrator::constraintsAreViolated(const DoubleArray& x)
{
    if( x[0] <= 0.000001 )  return true;
    if( x[0] >= 10. )  return true;

    return false;
}