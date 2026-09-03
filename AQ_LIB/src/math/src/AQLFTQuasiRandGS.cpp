/*! @file
    @brief Generalized Sobol'
*/
//#define __MUTEST_DEBUG__

//64bit
#ifdef __GNUG__					
	typedef unsigned long long LONG_Integer;	
#endif							
#ifdef _MSC_VER					
	typedef unsigned _int64 LONG_Integer;	
#endif							
#ifdef __SUNPRO_CC
	typedef unsigned long long LONG_Integer;
#endif


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLFTQuasiRandGS.cpp
//
//  SYNOPSIS    :       AQLFTQuasiRandGS
//  DESCRIPTION :
//
//
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include "AQLFTQuasiRandGS.h"
unsigned long AQLFTQuasiRandGS::mIrreduciblePoly[1500]  = {	
		2,		3,		7,		11,		13,		19,		25,		31,		37,		41,
		47,		55,		59,		61,		67,		73,		87,		91,		97,		103,
		109,	115,	117,	131,	137,	143,	145,	157,	167,	171,
		185,	191,	193,	203,	211,	213,	229,	239,	241,	247,
		253,	283,	285,	299,	301,	313,	319,	333,	351,	355,
		357,	361,	369,	375,	379,	391,	395,	397,	415,	419,
		425,	433,	445,	451,	463,	471,	477,	487,	499,	501,
		505,	515,	529,	535,	539,	545,	557,	563,	587,	601,
		607,	613,	617,	623,	631,	637,	647,	661,	665,	675,
		677,	687,	695,	701,	719,	721,	731,	757,	761,	769,
		787,	789,	799,	803,	817,	827,	841,	847,	859,	865,
		875,	877,	883,	895,	901,	911,	929,	949,	953,	967,
		971,	973,	981,	985,	995,	1001,	1019,	1033,	1039,	1051,
		1053,	1063,	1069,	1077,	1095,	1107,	1123,	1125,	1135,	1153,
		1163,	1177,	1193,	1199,	1221,	1225,	1239,	1255,	1261,	1267,
		1279,	1291,	1293,	1305,	1311,	1315,	1329,	1341,	1347,	1367,
		1377,	1383,	1387,	1413,	1423,	1431,	1435,	1441,	1451,	1465,
		1473,	1479,	1509,	1527,	1531,	1555,	1557,	1571,	1573,	1585,
		1591,	1603,	1615,	1617,	1627,	1657,	1663,	1669,	1673,	1703,
		1709,	1717,	1727,	1729,	1741,	1747,	1759,	1783,	1789,	1807,
		1809,	1815,	1821,	1825,	1835,	1845,	1849,	1863,	1869,	1877,
		1881,	1891,	1915,	1917,	1921,	1927,	1933,	1939,	1961,	1969,
		1989,	2011,	2027,	2035,	2041,	2047,	2053,	2071,	2091,	2093,
		2119,	2147,	2149,	2161,	2171,	2189,	2197,	2207,	2217,	2225,
		2243,	2255,	2257,	2273,	2279,	2283,	2293,	2317,	2323,	2341,
		2345,	2359,	2363,	2365,	2373,	2377,	2385,	2395,	2419,	2421,
		2431,	2435,	2447,	2475,	2477,	2489,	2503,	2521,	2533,	2543,
		2551,	2561,	2567,	2579,	2581,	2601,	2633,	2657,	2669,	2681,
		2687,	2693,	2705,	2717,	2727,	2731,	2739,	2741,	2773,	2783,
		2787,	2793,	2799,	2801,	2811,	2819,	2825,	2833,	2867,	2879,
		2881,	2891,	2905,	2911,	2917,	2927,	2941,	2951,	2955,	2963,
		2965,	2991,	2999,	3005,	3017,	3035,	3037,	3047,	3053,	3083,
		3085,	3097,	3103,	3121,	3159,	3169,	3179,	3187,	3189,	3205,
		3209,	3223,	3227,	3229,	3251,	3263,	3271,	3277,	3283,	3285,
		3299,	3305,	3319,	3331,	3343,	3357,	3367,	3373,	3393,	3399,
		3413,	3417,	3427,	3439,	3441,	3475,	3487,	3497,	3515,	3517,
		3529,	3543,	3547,	3553,	3559,	3573,	3583,	3589,	3613,	3617,
		3623,	3627,	3635,	3641,	3655,	3659,	3669,	3679,	3697,	3707,
		3709,	3713,	3731,	3743,	3747,	3771,	3785,	3791,	3805,	3827,
		3833,	3851,	3865,	3889,	3895,	3933,	3947,	3949,	3957,	3961,
		3971,	3985,	3991,	3995,	4007,	4013,	4021,	4045,	4051,	4069,
		4073,	4091,	4105,	4119,	4129,	4147,	4149,	4159,	4173,	4179,
		4201,	4215,	4219,	4221,	4225,	4235,	4249,	4259,	4261,	4303,
		4305,	4331,	4333,	4351,	4359,	4383,	4387,	4401,	4407,	4411,
		4431,	4439,	4449,	4459,	4461,	4473,	4483,	4485,	4497,	4523,
		4531,	4569,	4575,	4579,	4591,	4593,	4609,	4621,	4627,	4633,
		4645,	4663,	4667,	4669,	4675,	4677,	4711,	4717,	4723,	4735,
		4789,	4793,	4801,	4811,	4873,	4879,	4891,	4893,	4897,	4915,
		4921,	4927,	4941,	4965,	4977,	5017,	5023,	5027,	5033,	5039,
		5051,	5059,	5073,	5079,	5085,	5107,	5109,	5127,	5139,	5169,
		5175,	5193,	5199,	5211,	5213,	5223,	5227,	5237,	5247,	5257,
		5281,	5287,	5293,	5301,	5325,	5331,	5337,	5343,	5349,	5391,
		5405,	5451,	5453,	5505,	5523,	5541,	5545,	5563,	5573,	5591,
		5597,	5611,	5625,	5635,	5641,	5659,	5695,	5697,	5703,	5707,
		5717,	5721,	5731,	5733,	5743,	5779,	5797,	5821,	5827,	5833,
		5841,	5857,	5863,	5875,	5887,	5899,	5909,	5913,	5949,	5955,
		5957,	5967,	5975,	5981,	6003,	6005,	6009,	6025,	6031,	6039,
		6045,	6061,	6067,	6079,	6081,	6111,	6139,	6151,	6157,	6175,
		6179,	6193,	6199,	6217,	6231,	6237,	6253,	6265,	6271,	6275,
		6289,	6295,	6305,	6329,	6347,	6349,	6383,	6385,	6395,	6405,
		6409,	6427,	6445,	6453,	6465,	6475,	6495,	6501,	6511,	6523,
		6529,	6539,	6553,	6577,	6583,	6589,	6601,	6607,	6621,	6631,
		6637,	6649,	6683,	6685,	6689,	6699,	6707,	6733,	6739,	6741,
		6751,	6755,	6761,	6779,	6795,	6833,	6853,	6865,	6881,	6887,
		6891,	6901,	6923,	6925,	6931,	6937,	6943,	6959,	6981,	6999,
		7049,	7055,	7057,	7079,	7093,	7097,	7103,	7105,	7115,	7123,
		7139,	7165,	7171,	7173,	7183,	7185,	7191,	7207,	7245,	7263,
		7303,	7327,	7333,	7351,	7355,	7365,	7369,	7375,	7383,	7403,
		7405,	7411,	7425,	7431,	7459,	7471,	7485,	7491,	7505,	7515,
		7517,	7527,	7541,	7545,	7555,	7557,	7561,	7569,	7591,	7603,
		7617,	7663,	7687,	7701,	7705,	7727,	7739,	7741,	7749,	7761,
		7773,	7777,	7783,	7795,	7823,	7831,	7835,	7865,	7871,	7885,
		7891,	7907,	7921,	7927,	7939,	7953,	7963,	7975,	7993,	8007,
		8011,	8019,	8037,	8049,	8061,	8065,	8077,	8089,	8111,	8123,
		8125,	8131,	8133,	8137,	8161,	8173,	8191,	8219,	8231,	8245,
		8275,	8293,	8303,	8331,	8333,	8351,	8357,	8367,	8379,	8381,
		8387,	8393,	8417,	8435,	8461,	8469,	8489,	8495,	8507,	8515,
		8551,	8555,	8569,	8585,	8599,	8605,	8639,	8641,	8647,	8653,
		8671,	8675,	8689,	8699,	8729,	8741,	8759,	8765,	8771,	8795,
		8797,	8825,	8831,	8841,	8855,	8859,	8883,	8895,	8909,	8943,
		8951,	8955,	8965,	8999,	9003,	9031,	9045,	9049,	9071,	9073,
		9085,	9095,	9101,	9109,	9123,	9129,	9137,	9143,	9147,	9185,
		9197,	9209,	9227,	9235,	9247,	9253,	9257,	9277,	9297,	9303,
		9313,	9325,	9343,	9347,	9371,	9373,	9397,	9407,	9409,	9415,
		9419,	9443,	9481,	9495,	9501,	9505,	9517,	9529,	9555,	9557,
		9571,	9585,	9591,	9607,	9611,	9621,	9625,	9631,	9647,	9661,
		9669,	9679,	9687,	9707,	9731,	9733,	9745,	9773,	9791,	9803,
		9811,	9817,	9833,	9847,	9851,	9863,	9875,	9881,	9905,	9911,
		9917,	9923,	9963,	9973,	10003,	10025,	10043,	10063,	10071,	10077,
		10091,	10099,	10105,	10115,	10129,	10145,	10169,	10183,	10187,	10207,
		10223,	10225,	10247,	10265,	10271,	10275,	10289,	10299,	10301,	10309,
		10343,	10357,	10373,	10411,	10413,	10431,	10445,	10453,	10463,	10467,
		10473,	10491,	10505,	10511,	10513,	10523,	10539,	10549,	10559,	10561,
		10571,	10581,	10615,	10621,	10625,	10643,	10655,	10671,	10679,	10685,
		10691,	10711,	10739,	10741,	10755,	10767,	10781,	10785,	10803,	10805,
		10829,	10857,	10863,	10865,	10875,	10877,	10917,	10921,	10929,	10949,
		10967,	10971,	10987,	10995,	11009,	11029,	11043,	11045,	11055,	11063,
		11075,	11081,	11117,	11135,	11141,	11159,	11163,	11181,	11187,	11225,
		11237,	11261,	11279,	11297,	11307,	11309,	11327,	11329,	11341,	11377,
		11403,	11405,	11413,	11427,	11439,	11453,	11461,	11473,	11479,	11489,
		11495,	11499,	11533,	11545,	11561,	11567,	11575,	11579,	11589,	11611,
		11623,	11637,	11657,	11663,	11687,	11691,	11701,	11747,	11761,	11773,
		11783,	11795,	11797,	11817,	11849,	11855,	11867,	11869,	11873,	11883,
		11919,	11921,	11927,	11933,	11947,	11955,	11961,	11999,	12027,	12029,
		12037,	12041,	12049,	12055,	12095,	12097,	12107,	12109,	12121,	12127,
		12133,	12137,	12181,	12197,	12207,	12209,	12239,	12253,	12263,	12269,
		12277,	12287,	12295,	12309,	12313,	12335,	12361,	12367,	12391,	12409,
		12415,	12433,	12449,	12469,	12479,	12481,	12499,	12505,	12517,	12527,
		12549,	12559,	12597,	12615,	12621,	12639,	12643,	12657,	12667,	12707,
		12713,	12727,	12741,	12745,	12763,	12769,	12779,	12781,	12787,	12799,
		12809,	12815,	12829,	12839,	12857,	12875,	12883,	12889,	12901,	12929,
		12947,	12953,	12959,	12969,	12983,	12987,	12995,	13015,	13019,	13031,
		13063,	13077,	13103,	13137,	13149,	13173,	13207,	13211,	13227,	13241,
		13249,	13255,	13269,	13283,	13285,	13303,	13307,	13321,	13339,	13351,
		13377,	13389,	13407,	13417,	13431,	13435,	13447,	13459,	13465,	13477,
		13501,	13513,	13531,	13543,	13561,	13581,	13599,	13605,	13617,	13623,
		13637,	13647,	13661,	13677,	13683,	13695,	13725,	13729,	13753,	13773,
		13781,	13785,	13795,	13801,	13807,	13825,	13835,	13855,	13861,	13871,
		13883,	13897,	13905,	13915,	13939,	13941,	13969,	13979,	13981,	13997,
		14027,	14035,	14037,	14051,	14063,	14085,	14095,	14107,	14113,	14125,
		14137,	14145,	14151,	14163,	14193,	14199,	14219,	14229,	14233,	14243,
		14277,	14287,	14289,	14295,	14301,	14305,	14323,	14339,	14341,	14359,
		14365,	14375,	14387,	14411,	14425,	14441,	14449,	14499,	14513,	14523,
		14537,	14543,	14561,	14579,	14585,	14593,	14599,	14603,	14611,	14641,
		14671,	14695,	14701,	14723,	14725,	14743,	14753,	14759,	14765,	14795,
		14797,	14803,	14831,	14839,	14845,	14855,	14889,	14895,	14909,	14929,
		14941,	14945,	14951,	14963,	14965,	14985,	15033,	15039,	15053,	15059,
		15061,	15071,	15077,	15081,	15099,	15121,	15147,	15149,	15157,	15167,
		15187,	15193,	15203,	15205,	15215,	15217,	15223,	15243,	15257,	15269,
		15273,	15287,	15291,	15313,	15335,	15347,	15359,	15373,	15379,	15381,
		15391,	15395,	15397,	15419,	15439,	15453,	15469,	15491,	15503,	15517,
		15527,	15531,	15545,	15559,	15593,	15611,	15613,	15619,	15639,	15643,
		15649,	15661,	15667,	15669,	15681,	15693,	15717,	15721,	15741,	15745,
		15765,	15793,	15799,	15811,	15825,	15835,	15847,	15851,	15865,	15877,
		15881,	15887,	15899,	15915,	15935,	15937,	15955,	15973,	15977,	16011,
		16035,	16061,	16069,	16087,	16093,	16097,	16121,	16141,	16153,	16159,
		16165,	16183,	16189,	16195,	16197,	16201,	16209,	16215,	16225,	16259,
		16265,	16273,	16299,	16309,	16355,	16375,	16381,	16417,	16427,	16435,
		16441,	16447,	16467,	16479,	16485,	16507,	16519,	16553,	16559,	16571,
		16573,	16591,	16599,	16619,	16627,	16633,	16651,  16653,	16659,	16699,
		16707,	16713,	16727,	16743,	16749,	16785,	16795,	16797,	16807,	16811,
		16813,	16821,	16853,	16857,	16881,	16897,	16909,	16965,	16969,	16983,
		16993,	17011,	17017,	17023,	17027,	17029,	17053,	17057,	17095,	17099,
		17101,	17123,	17129,	17135,	17155,	17161,	17179,	17185,	17191,	17215,
		17257,	17275,	17277,	17287,	17301,	17327,	17353,	17373,	17387,	17389,
		17407,	17419,	17421,	17475,	17501,	17523,	17545,	17601,	17619,	17621,
		17631,	17635,	17649,	17659,	17667,	17673,	17679,	17707,	17721,	17753,
		17775,	17783,	17789,	17805,	17817,	17823,	17829,	17847,	17861,	17865,
		17873,	17879,	17895,	17907,	17919,	17935,	17949,	17959,	17973,	17991,
		18009,	18019,	18033,	18043,	18061,	18067,	18069,	18083,	18085,	18117,};

using namespace std;

//---------------------------------------------------
/*!
	@brief FTQuasiRandGSdefault constructor
	Rand()

*/

AQLFTQuasiRandGS::AQLFTQuasiRandGS() 
: AQLRandBase(),mSeedSet(false),mDimSet(false),mMax(0)
{
    ftholdrand = 1L;//Rand()'s Default Seed
	UintArray dim(1);
	setDim(dim);
}

/*!
	@brief FTQuasiRandGSdestructor

*/
AQLFTQuasiRandGS::~AQLFTQuasiRandGS()
{
}

/*!
	@brief deep copy of this object
*/
AQLCoreFunctionBase*
AQLFTQuasiRandGS::clone() const
{
    try
	{
		return new AQLFTQuasiRandGS(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
	@brief check whether this class derives from base class with type id
	@param[in] id function type to be checked
	@return True or false
*/
bool
AQLFTQuasiRandGS::isTypeOf(function_t id) const
{
	return (id == FN_RAND_QUASIGS ? true : AQLRandBase::isTypeOf(id));
}

/*!
	@brief get type of the function
    @return function type
*/
function_t
AQLFTQuasiRandGS::getType() const
{
	return FN_RAND_QUASIGS;
}
/*!
	@brief  \n
	2
	@param[in] variates ValarrayVector
*/
void
AQLFTQuasiRandGS::getUniforms(DoubleArray& variates)
{

	/*!		*/	
	unsigned int valRow;					//
	unsigned int valDim;					//
	unsigned int valCount;					//
	unsigned int sDim = getDim().size() == 2 ? getDim()[1] : getDim()[0];				//	
	unsigned int tDim=getDim()[0];				//
	unsigned long multMtrx;					//
	unsigned int cnt;						//1(mod2)

	//variates0
	for (valDim=0;valDim<sDim;valDim++)
	{
		variates[valDim]=0;
	}


	//
	for (valDim=0;valDim<sDim;valDim++)
	{
		//
		for (valRow=0;valRow<mMaxDigit;valRow++)
		{
	
			//(2)
			multMtrx=mGeneMatrix[valDim*mMaxDigit+valRow]&mState;	//&
			cnt=0;
			
			//(11,0)
			for (valCount=0;valCount<mMaxDigit;valCount++)
			{
				if ((multMtrx>>valCount)==0)break;
				cnt=cnt^(unsigned int)((multMtrx>>valCount)&0x01);
			}

			//2
			variates[valDim]=variates[valDim]*2+cnt;
		}
		variates[valDim]=variates[valDim]/AQLMath::pow(2,mMaxDigit);		
		if (variates[valDim]==0)variates[valDim]=1/AQLMath::pow(2,mMaxDigit+1);	
	}

	//
	if ((tDim-sDim)!=0){
		DoubleArray variatesTmp(tDim-sDim);
		mInner.getUniforms(variatesTmp);

		for (valDim=0;valDim<(tDim-sDim);valDim++){
			variates[ sDim+valDim ] = variatesTmp[valDim];
		}
	}

#ifdef __MUTEST_DEBUG__ //20051017add
	FILE *fp_GS;
		fp_GS = fopen("GS.csv","a");

		for (unsigned int i=0;i<tDim;i++)
		{
			fprintf(fp_GS,"%20.18f\t",variates[i]);
		}
		fprintf(fp_GS,"\n");
		fclose(fp_GS);
#endif

	//
	mState = mState + 1;
	
	// counter increments it by one, if exceeds the Max, back to the first
	if (mState==mMax){
		mState=getSeed()[0];
	}

	return;

}

/*!
	@brief Seed
	@param[in] seed SeedValarrayVector\n
		seed[0]=,\n
			seed[1]=Sequence,\n
			seed[2]=Seed For Generator Matrix,\n
			seed[3]=Seed For Mersennne Twister
*/
void
AQLFTQuasiRandGS::setSeed(const UlongArray& _seed)
{
	UlongArray seed(4);
	if (_seed.size() == 2)
	{
		seed[0] = 2^20;
		seed[1] = _seed[1];
		seed[2] = _seed[0];
		seed[3] = _seed[0];
	}
	else if (_seed.size() == 4) seed = _seed;
	else
	{
		AQLString msg =  "Number-of-Seed Error";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		//error. wrong Seed setting
	}
	AQLRandBase::setSeed(seed);
	mMax=seed[0]+seed[1];
	mMaxDigit=culcFig(seed[0]+seed[1]);
	mState=seed[0];
	mMSeed=seed[2];
	mSeedSet=true;

	//Mersennne Twisterseed
	UlongArray seedtmp(1);
	seedtmp[0]=seed[3];
	(mInner).setSeed(seedtmp);

	if (mDimSet && mSeedSet) getGM();
	return;
}
/*!
	@brief  \n
			()

	@param[in] dimValue ValarrayVector\n
		dimValue[0]=,\n
	dimValue[1]=
*/
void
AQLFTQuasiRandGS::setDim(const UintArray& _dimValue)
{
	UintArray dimValue;
	if (_dimValue.size() == 1 && _dimValue[0] > 1500)
	{
		dimValue.resize(2);
        dimValue[0] = _dimValue[0];
		dimValue[1] = 1500;
	}
	else dimValue = _dimValue;
	
	if (dimValue.size() != 1 && dimValue.size() != 2) 
	{
		AQLString msg =  "Number-of-Dim Error";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		//error. wrong Dim setting
	}
	AQLRandBase::setDim(dimValue);
//    mDimSet=true;//20051220(A)
	
	//Mersennne TwisterDim
	UintArray dimtmp(1);
	if (dimValue.size() == 2)
	{
		if ((dimValue[0]-dimValue[1])<0){
			AQLString msg =  "TotalDim<QuasiRandDim Error";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		// 
		}
		dimtmp[0]=dimValue[0]-dimValue[1];
		if (dimValue[1]>1500){
			AQLString msg =  "QuasiRandDim>1500 Error";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			// M-Library1500
		}
	}
	mDimSet=true;

	if (dimtmp[0] != 0) (mInner).setDim(dimtmp);

	if (mDimSet && mSeedSet) getGM();

#ifdef __MUTEST_DEBUG__ //20051017add
	FILE *fp_GS;
		fp_GS = fopen("GS.csv","w");
		fclose(fp_GS);
#endif


	return;
}
/*!
	@brief Get generator Matrix recorded in UlongArray "mGeneMatrix"
*/
void
AQLFTQuasiRandGS::getGM(void)
{
	unsigned int valDim;					//
	unsigned int valRow;					//
	unsigned int sDim = getDim().size() == 2 ? getDim()[1] : getDim()[0];				//	
	unsigned long powerIPoly;				//
	unsigned int digitIPoly;				//
	unsigned int Power;						//
	unsigned int randShift;					//

	mGeneMatrix.resize(sDim*mMaxDigit);	//mGeneMatrix

	//
	ftsrand(mMSeed);

	for (valDim=0;valDim<sDim;valDim++)
	{
		//
		digitIPoly=culcFig(mIrreduciblePoly[valDim])-1;
		for (valRow=0;valRow<mMaxDigit;valRow++)
		{
			//
			Power=valRow/digitIPoly+1;				

			//
			randShift=digitIPoly-1-valRow%digitIPoly;	

			//
			powerIPoly=IPPower(mIrreduciblePoly[valDim],Power);

			//
			mGeneMatrix[valDim*mMaxDigit+valRow]=getEG(powerIPoly,randShift);
		}
	}

#ifdef __MUTEST_DEBUG__ //20051017add
	FILE *fp_G;
		fp_G = fopen("G.csv","w");
		for (valDim=0;valDim<sDim;valDim++)
		{
			for (valRow=0;valRow<mMaxDigit;valRow++)
			{
				fprintf(fp_G,"MM[%d][%d]=\t%ld\n",valDim,valRow,mGeneMatrix[valDim*mMaxDigit+valRow]);
			}
		}
		fprintf(fp_G,"\n");
		fclose(fp_G);
#endif


}

/*!
	@brief Power of Irreducible Polynomial
	@param[in] IPoly Irreducible Polynomial
	@param[in] Power Power Number
	@return Powered Polynomial
*/
unsigned long
AQLFTQuasiRandGS::IPPower(unsigned long IPoly,unsigned int Power)
{
	/*!		*/
	unsigned long out_polynomial;				//
	unsigned long temp_polynomial;				//
	unsigned int val_power;						//
	unsigned int val_digit;						//
	unsigned int	digit;						//

	//
	digit=culcFig(IPoly);

	//
	out_polynomial=IPoly;

	/*!		*/
	//
	for (val_power=1;val_power<Power;val_power++)
	{
		//
		temp_polynomial=0;
		//
		for (val_digit=digit;val_digit>=1;val_digit--)
		{
			//1
			if (((IPoly>>(val_digit-1))&0x00000001)==1)
			{
				//1,XOR
				temp_polynomial=temp_polynomial^(out_polynomial<<(val_digit-1));
			}
		}
		out_polynomial=temp_polynomial;
	}

	//
	return out_polynomial;
}

/*!
	@brief Getting an element of Generator Matrix
	@param[in] IPoly Irreducible Polynomial
	@return an element of Generator Matrix
*/
unsigned long
AQLFTQuasiRandGS::getEG(unsigned long IPoly,unsigned int randShift)
{
	/*!		*/
	unsigned int degreeLaurent;						//
	unsigned int digitIPoly=culcFig(IPoly);			//
	LONG_Integer laurent;							//
	LONG_Integer temp_polynomial;					//
	LONG_Integer elementGeneratorMtrx;				//
	unsigned int val_number,val_digit,val_shift;	//
	unsigned long rand_polynomial;					//
	unsigned long min_rand;							//

	/*!*******************/
	/*!			*/
	/*!*******************/

	//
	//	=(n_digit)+(shift)
	degreeLaurent=digitIPoly+mMaxDigit;

	//
	laurent=(unsigned long)AQLMath::pow(2,digitIPoly-2);

	//
	if (degreeLaurent>64)degreeLaurent=64;

	/*!		*/
    for (val_number=digitIPoly;val_number<degreeLaurent;val_number++)
    {
        temp_polynomial=0x00;
        for (val_digit=0;val_digit<digitIPoly;val_digit++)
        {
            if (((IPoly>>val_digit)&0x0001)==1)
            {
                //laurent*polynomial
                temp_polynomial=temp_polynomial^(laurent>>val_digit);       
            }
        }
        //laurent*polynomiallaurent
        laurent=laurent^(temp_polynomial<<(digitIPoly-1));
    }

	/*!*******************************/
	/*!				*/
	/*!*******************************/

	/*!	digit	*/
	//
	min_rand=(0x00000001<<randShift);
	
	//digit
	rand_polynomial=ftrand()&(min_rand-1);

	//digit
	rand_polynomial=rand_polynomial|min_rand;


	/*!***************************************/
	/*!				*/
	/*!***************************************/
	
	//
	elementGeneratorMtrx=0;
	for (val_shift=0;val_shift<degreeLaurent;val_shift++)
	{
		if (((rand_polynomial>>val_shift)&0x00000001)==1)
		{
			elementGeneratorMtrx=elementGeneratorMtrx^(laurent>>val_shift);
		}
	}

	elementGeneratorMtrx=(unsigned long)(elementGeneratorMtrx)&((LONG_Integer)AQLMath::pow(2,mMaxDigit)-1);
	return (unsigned long)elementGeneratorMtrx;
}

/*!
	@brief calculate figure of number by 2
	@param[in] IPoly Irreducible Polynomial
	@return Figure number
*/
unsigned int
AQLFTQuasiRandGS::culcFig(unsigned long IPoly)
{

	/*!		*/
	int digit;	//

	/*!		*/
	digit=0;

	/*!		*/
	while(((IPoly>>digit)!=0))
    {
        //digit=digit++;
		digit++;
    }

    //
	return digit;
}

/*!
	@brief set seed of rand function
	@param[in] seed 1-dim long
*/
void
AQLFTQuasiRandGS::ftsrand (unsigned long seed)
{
	ftholdrand  = (long)seed;
}
/*!
	@brief the inner rand() function only used by this class
	@return random numbers
*/
int
AQLFTQuasiRandGS::ftrand (void)
{
	return(((ftholdrand = ftholdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}
