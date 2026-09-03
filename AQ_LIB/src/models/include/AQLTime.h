#pragma once

#include <float.h>
#include <cmath>
#include "ConstantDeclarations.h"
#include <cstring>
#include <vector>
#include <ctime>
#include <cstdio>
#include "AQLDataFile.h"


#define TIMELEN             (20+1) 
class AQLTime
{
public:
	//error check 
	static int isError(double b)
	{
#if defined(WIN32) || defined(WIN64)
		if(!_finite(b))
			return ERMATHINF_;
		if(_isnan(b))
			return ERMATHNAN_;
		return 0;
#else
		if (std::isinf(b))
			return ERMATHINF_;
		if(std::isnan(b))
			return ERMATHNAN_;
		return 0;
#endif
	}

	static AQLString now(void)
	{
		tm*     nowtime;
		time_t  local_t;
		char    strTime[TIMELEN] = "\0";

		time(&local_t); 
		nowtime = localtime(&local_t);
		sprintf(strTime, "[%04d/%02d/%02d %02d:%02d]",
                    nowtime->tm_year + 1900,
                    nowtime->tm_mon + 1,
                    nowtime->tm_mday,
                    nowtime->tm_hour,
                    nowtime->tm_min);
		AQLString ret;
		for(int i=0;i<TIMELEN;i++)
			ret+= strTime[i];
		return ret;
	}

	//cut down the log file's data to n lines. 
	static void logFileCheck(unsigned int n, const char* logfile)
	{

		MDFile mdf;
		OPENMODE mode;

		FILE* fp = fopen(logfile, "r");
		if (fp)
		{
			fclose(fp);
			mode = MODE_R;

		}
		else
		{
			mode = MODE_W;
		}

		mdf.openFile(logfile, mode);
		
		unsigned int logrow = mdf.getRowCounts();//get row numbers
		if(logrow>n)
		{
			AQLStringVector logrec;
			for(unsigned int i=0;i<n;i++)
			{
				logrec.push_back(mdf.getRecord(logrow-n+i));
			}//set the newer n data to the string.

			mdf.closeFile();

			MDFile mdf_w(logfile,MODE_W);//reopen the file as writable mode which initializes the file.
			for(unsigned int i=0;i<n;i++)
			{
				AQLString addrec = logrec[i];
				mdf_w.addRecord(addrec);
			}
			mdf_w.closeFile();
		}
		else
		{
			mdf.closeFile();
		}
	}

};
