// swad_date.h: dates

#ifndef _SWAD_DAT
#define _SWAD_DAT
/*
    SWAD (Shared Workspace At a Distance in Spanish),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2015 Antonio Ca�as Vargas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*****************************************************************************/
/********************************* Headers ***********************************/
/*****************************************************************************/

#include <stdio.h>	// For FILE *
#include <time.h>

#include "swad_bool.h"

/*****************************************************************************/
/***************************** Public constants ******************************/
/*****************************************************************************/

#define Dat_SECONDS_IN_ONE_MONTH (30UL*24UL*60UL*60UL)

/*****************************************************************************/
/******************************* Public types ********************************/
/*****************************************************************************/

struct Date
  {
   unsigned Day,Month,Year,Week;
   char YYYYMMDD[4+2+2+1];
  };
struct Time
  {
   unsigned Hour,Minute,Second;
  };
struct Hour
  {
   unsigned Hour,Minute;
  };
struct DateTime
  {
   struct Date Date;
   struct Time Time;
   char YYYYMMDDHHMMSS[4+2+2+2+2+2+1];
  };

/*****************************************************************************/
/***************************** Public prototypes *****************************/
/*****************************************************************************/

void Dat_GetTimeStartExecution (void);
void Dat_GetAndConvertCurrentDateTime (void);

bool Dat_GetDateFromYYYYMMDD (struct Date *Date,const char *YYYYMMDDString);
bool Dat_GetDateTimeFromYYYYMMDDHHMMSS (struct DateTime *DateTime,const char *YYYYMMDDHHMMSS);

void Dat_ShowCurrentDateTime (void);

void Dat_GetLocalTimeFromClock (const time_t *clock);
void Dat_WriteDateTimeFromtblock (void);
void Dat_WriteDateFromtblock (void);
void Dat_ConvDateToDateStr (struct Date *Date,char *DateStr);

void Dat_WriteFormIniEndDates (void);
void Dat_WriteFormDate (unsigned FirstYear,unsigned LastYear,
	                const char *NameSelectDay,const char *NameSelectMonth,const char *NameSelectYear,
		        struct Date *DateSelected,
                        bool SubmitFormOnChange,bool Disabled);
void Dat_WriteFormHourMinute (const char *NameSelectHour,const char *NameSelectMinute,
		              struct Time *TimeSelected,
                              bool SubmitFormOnChange,bool Disabled);
void Dat_GetDateFromForm (const char *NameParamDay,const char *NameParamMonth,const char *NameParamYear,
                          unsigned *Day,unsigned *Month,unsigned *Year);
void Dat_GetHourMinuteFromForm (const char *NameParamHour,const char *NameParamMinute,
                                unsigned *Hour,unsigned *Minute);

void Dat_GetIniEndDatesFromForm (void);

void Dat_WriteDate (const char *YYYYMMDD);
void Dat_WriteHourMinute (const char *HHMM);
void Dat_WriteRFC822DateFromTM (FILE *File,struct tm *tm);

void Dat_GetDateBefore (struct Date *Date,struct Date *PrecedingDate);
void Dat_GetDateAfter (struct Date *Date,struct Date *SubsequentDate);
void Dat_GetWeekBefore (struct Date *Date,struct Date *PrecedingDate);
void Dat_GetMonthBefore (struct Date *Date,struct Date *PrecedingDate);
unsigned Dat_GetNumDaysBetweenDates (struct Date *DateIni,struct Date *DateEnd);
unsigned Dat_GetNumWeeksBetweenDates (struct Date *DateIni,struct Date *DateEnd);
unsigned Dat_GetNumMonthsBetweenDates (struct Date *DateIni,struct Date *DateEnd);
unsigned Dat_GetNumDaysInYear (unsigned Year);
unsigned Dat_GetNumDaysFebruary (unsigned Year);
bool Dat_GetIfLeapYear (unsigned Year);
unsigned Dat_GetNumWeeksInYear (unsigned Year);
unsigned Dat_GetDayOfWeek (unsigned Year,unsigned Month,unsigned Day);
unsigned Dat_GetDayOfYear (struct Date *Date);
void Dat_CalculateWeekOfYear (struct Date *Date);
void Dat_AssignDate (struct Date *DateDst,struct Date *DateSrc);

int Dat_CompareDates (struct Date *Date1,struct Date *Date2);
int Dat_CompareDateTimes (struct DateTime *DateTime1,struct DateTime *DateTime2);

#endif
