// swad_match.h: matches in games using remote control

#ifndef _SWAD_MCH
#define _SWAD_MCH
/*
    SWAD (Shared Workspace At a Distance in Spanish),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2019 Antonio Ca�as Vargas

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

#include "swad_scope.h"

/*****************************************************************************/
/************************** Public types and constants ***********************/
/*****************************************************************************/

#define Mch_NEW_MATCH_SECTION_ID	"new_match"

#define Mch_AFTER_LAST_QUESTION	((unsigned)((1UL << 31) - 1))	// 2^31 - 1, don't change this number because it is used in database to indicate that a match is finished

#define Mch_NUM_SHOWING 4
typedef enum
  {
   Mch_NOTHING,	// Don't show anything
   Mch_STEM,	// Showing only the question stem
   Mch_ANSWERS,	// Showing the question stem and the answers
   Mch_RESULTS,	// Showing the results
  } Mch_Showing_t;
#define Mch_SHOWING_DEFAULT Mch_NOTHING

struct Match
  {
   long MchCod;
   long GamCod;
   long UsrCod;
   time_t TimeUTC[Dat_NUM_START_END_TIME];
   char Title[Gam_MAX_BYTES_TITLE + 1];
   struct
     {
      unsigned QstInd;	// 0 means that the game has not started. First question has index 1.
      long QstCod;
      time_t QstStartTimeUTC;
      Mch_Showing_t Showing;	// What is shown on teacher's screen
      bool ShowQstResults;	// Show global results of current question while playing
      bool ShowUsrResults;	// Show exam with results of all questions for the student
      bool Playing;		// Is being played now?
      unsigned NumPlayers;
     } Status;			// Status related to match playing
  };

struct Mch_UsrAnswer
  {
   int NumOpt;	// < 0 ==> no answer selected
   int AnsInd;	// < 0 ==> no answer selected
  };

/*****************************************************************************/
/***************************** Public prototypes *****************************/
/*****************************************************************************/

void Mch_ListMatches (struct Game *Game,bool PutFormNewMatch);
void Mch_GetDataOfMatchByCod (struct Match *Match);

void Mch_ToggleVisibilResultsMchUsr (void);

void Mch_RequestRemoveMatchTch (void);
void Mch_RemoveMatchTch (void);

void Mch_RemoveMatchesInGameFromAllTables (long GamCod);
void Mch_RemoveMatchInCourseFromAllTables (long CrsCod);

void Mch_PutParamMchCod (long MchCod);
long Mch_GetParamMchCod (void);

void Mch_CreateNewMatchTch (void);
void Mch_RequestStartResumeMatchTch (void);
void Mch_GetIndexes (long MchCod,unsigned QstInd,
		     unsigned Indexes[Tst_MAX_OPTIONS_PER_QUESTION]);

void Mch_RemoveGroup (long GrpCod);
void Mch_RemoveGroupsOfType (long GrpTypCod);

void Mch_PauseMatchTch (void);
void Mch_ResumeMatchTch (void);
void Mch_ToggleVisibilResultsMchQst (void);
void Mch_BackMatchTch (void);
void Mch_ForwardMatchTch (void);

unsigned Mch_GetNumMchsInGame (long GamCod);

void Mch_GetMatchBeingPlayed (void);
void Mch_ShowMatchToMeAsStd (void);
void Mch_RefreshMatchTch (void);
void Mch_RefreshMatchStd (void);

void Mch_GetQstAnsFromDB (long MchCod,long UsrCod,unsigned QstInd,
		          struct Mch_UsrAnswer *UsrAnswer);
void Mch_ReceiveQstAnsFromStd (void);

void Mch_GetAndDrawBarNumUsrsWhoHaveChosenAns (long MchCod,unsigned QstInd,unsigned AnsInd,
					       unsigned NumAnswerersQst,bool Correct);
unsigned Mch_GetNumUsrsWhoHaveAnswerQst (long MchCod,unsigned QstInd);

#endif