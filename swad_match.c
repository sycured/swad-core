// swad_match.c: matches in games using remote control

/*
    SWAD (Shared Workspace At a Distance),
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

#define _GNU_SOURCE 		// For asprintf
#include <linux/limits.h>	// For PATH_MAX
#include <linux/stddef.h>	// For NULL
#include <stdio.h>		// For asprintf
#include <stdlib.h>		// For calloc
#include <string.h>		// For string functions

#include "swad_alert.h"
#include "swad_box.h"
#include "swad_database.h"
#include "swad_form.h"
#include "swad_game.h"
#include "swad_global.h"
#include "swad_group.h"
#include "swad_ID.h"
#include "swad_match.h"
#include "swad_match_result.h"
#include "swad_pagination.h"
#include "swad_parameter.h"
#include "swad_role.h"
#include "swad_setting.h"
#include "swad_table.h"
#include "swad_test.h"
#include "swad_user.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

#define Mch_ICON_CLOSE		"fas fa-times"
#define Mch_ICON_PLAY		"fas fa-play"
#define Mch_ICON_PAUSE		"fas fa-pause"
#define Mch_ICON_PREVIOUS	"fas fa-step-backward"
#define Mch_ICON_NEXT		"fas fa-step-forward"
#define Mch_ICON_RESULTS	"fas fa-chart-bar"

/*****************************************************************************/
/******************************* Private types *******************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

const char *Mch_ShowingStringsDB[Mch_NUM_SHOWING] =
  {
   "nothing",
   "stem",
   "answers",
   "results",
  };

/*
mysql> SELECT table_name FROM information_schema.tables WHERE table_name LIKE 'mch%';
*/
const char *MatchSecondaryTables[] =
  {
   "mch_players",	// match players
   "mch_playing",	// matches being played
   "mch_results",	// matches results
   "mch_answers",	// students' answers to matches
   "mch_times",		// times associated to matches
   "mch_groups",	// groups associated to matches
   "mch_indexes",	// indexes associated to matches
   // "mch_matches"	// the matches themselves, this table is treated separately
  };
#define Mch_NUM_SECONDARY_TABLES	(sizeof (MatchSecondaryTables) / sizeof (MatchSecondaryTables[0]))

/*****************************************************************************/
/***************************** Private variables *****************************/
/*****************************************************************************/

long Mch_CurrentMchCod = -1L;	// Used as parameter in contextual links

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

static void Mch_PutIconToPlayNewMatch (void);

static void Mch_ListOneOrMoreMatches (struct Game *Game,
				      unsigned NumMatches,
                                      MYSQL_RES *mysql_res);
static void Mch_ListOneOrMoreMatchesHeading (void);
static void Mch_ListOneOrMoreMatchesIcons (const struct Match *Match);
static void Mch_ListOneOrMoreMatchesNumMatch (unsigned NumMatch);
static void Mch_ListOneOrMoreMatchesAuthor (const struct Match *Match);
static void Mch_ListOneOrMoreMatchesTimes (const struct Match *Match,unsigned UniqueId);
static void Mch_ListOneOrMoreMatchesTitleGrps (const struct Match *Match);
static void Mch_GetAndWriteNamesOfGrpsAssociatedToMatch (const struct Match *Match);
static void Mch_ListOneOrMoreMatchesNumPlayers (const struct Match *Match);
static void Mch_ListOneOrMoreMatchesStatus (const struct Match *Match,unsigned NumQsts);
static void Mch_ListOneOrMoreMatchesResult (const struct Match *Match);

static void Mch_GetMatchDataFromRow (MYSQL_RES *mysql_res,
				     struct Match *Match);
static Mch_Showing_t Mch_GetShowingFromStr (const char *Str);

static void Mch_RemoveMatchFromAllTables (long MchCod);
static void Mch_RemoveMatchFromTable (long MchCod,const char *TableName);
static void Mch_RemoveMatchesInGameFromTable (long GamCod,const char *TableName);
static void Mch_RemoveMatchInCourseFromTable (long CrsCod,const char *TableName);

static void Mch_PutParamCurrentMchCod (void);

static void Mch_PutButtonNewMatch (long GamCod);

static void Mch_PutFormNewMatch (struct Game *Game);
static void Mch_ShowLstGrpsToCreateMatch (void);

static long Mch_CreateMatch (long GamCod,char Title[Gam_MAX_BYTES_TITLE + 1]);
static void Mch_CreateIndexes (long GamCod,long MchCod);
static void Mch_ReorderAnswer (long MchCod,unsigned QstInd,
			       long QstCod,bool Shuffle);
static void Mch_CreateGrps (long MchCod);
static void Mch_UpdateMatchStatusInDB (struct Match *Match);

static void Mch_UpdateElapsedTimeInQuestion (struct Match *Match);
static void Mch_GetElapsedTimeInQuestion (struct Match *Match,
				          struct Time *Time);
static void Mch_GetElapsedTimeInMatch (struct Match *Match,
				       struct Time *Time);
static void Mch_GetElapsedTime (unsigned NumRows,MYSQL_RES *mysql_res,
				struct Time *Time);

static void Mch_SetMatchStatusToPrev (struct Match *Match);
static void Mch_SetMatchStatusToPrevQst (struct Match *Match);
static void Mch_SetMatchStatusToStart (struct Match *Match);

static void Mch_SetMatchStatusToNext (struct Match *Match);
static void Mch_SetMatchStatusToNextQst (struct Match *Match);
static void Mch_SetMatchStatusToEnd (struct Match *Match);

static void Mch_ShowMatchStatusForTch (struct Match *Match);
static void Mch_ShowMatchStatusForStd (struct Match *Match);
static bool Mch_CheckIfIPlayThisMatchBasedOnGrps (long MchCod);

static void Mch_ShowLeftColumnTch (struct Match *Match);
static void Mch_ShowInsideLeftColumnTch (struct Match *Match);
static void Mch_ShowRightColumnTch (struct Match *Match);
static void Mch_ShowLeftColumnStd (struct Match *Match);
static void Mch_ShowRightColumnStd (struct Match *Match);

static void Mch_ShowNumQstInMatch (struct Match *Match);
static void Mch_PutMatchControlButtons (struct Match *Match);
static void Mch_PutCheckboxResult (struct Match *Match);
static void Mch_ShowNumPlayers (struct Match *Match);
static void Mch_ShowMatchTitle (struct Match *Match);
static void Mch_ShowQuestionAndAnswersTch (struct Match *Match);
static void Mch_ShowQuestionAndAnswersStd (struct Match *Match);

static void Mch_PutParamNumOpt (unsigned NumOpt);
static unsigned Mch_GetParamNumOpt (void);

static void Mch_PutBigButton (Act_Action_t NextAction,long MchCod,
			      const char *Icon,const char *Txt);
static void Mch_PutBigButtonOff (const char *Icon);
static void Mch_PutBigButtonClose (void);

static void Mch_ShowWaitImage (const char *Txt);

static void Mch_RemoveOldPlayers (void);
static void Mch_UpdateMatchAsBeingPlayed (long MchCod);
static void Mch_SetMatchAsNotBeingPlayed (long MchCod);
static bool Mch_GetIfMatchIsBeingPlayed (long MchCod);
static void Mch_RegisterMeAsPlayerInMatch (long MchCod);
static void Mch_GetNumPlayers (struct Match *Match);

static double Mch_ComputeScore (unsigned NumQsts);

static unsigned Mch_GetNumUsrsWhoHaveChosenAns (long MchCod,unsigned QstInd,unsigned AnsInd);
static unsigned Mch_GetNumUsrsWhoHaveAnswerMch (long MchCod);
static void Mch_DrawBarNumUsrs (unsigned NumAnswerersAns,unsigned NumAnswerersQst,bool Correct);

/*****************************************************************************/
/************************* List the matches of a game ************************/
/*****************************************************************************/

void Mch_ListMatches (struct Game *Game,bool PutFormNewMatch)
  {
   extern const char *Hlp_ASSESSMENT_Games_matches;
   extern const char *Txt_Matches;
   extern long Gam_CurrentGamCod;	// Used as parameter in contextual links;
   char *SubQuery;
   MYSQL_RES *mysql_res;
   unsigned NumMatches;

   /***** Get data of matches from database *****/
   /* Fill subquery for game */
   if (Gbl.Crs.Grps.WhichGrps == Grp_ONLY_MY_GROUPS)
     {
      if (asprintf (&SubQuery," AND"
			      "(MchCod NOT IN"
			      " (SELECT MchCod FROM mch_groups)"
			      " OR"
			      " MchCod IN"
			      " (SELECT mch_groups.MchCod"
			      " FROM mch_groups,crs_grp_usr"
			      " WHERE crs_grp_usr.UsrCod=%ld"
			      " AND mch_groups.GrpCod=crs_grp_usr.GrpCod))",
		     Gbl.Usrs.Me.UsrDat.UsrCod) < 0)
	  Lay_NotEnoughMemoryExit ();
      }
    else	// Gbl.Crs.Grps.WhichGrps == Grp_ALL_GROUPS
       if (asprintf (&SubQuery,"%s","") < 0)
	  Lay_NotEnoughMemoryExit ();

   /* Make query */
   NumMatches = (unsigned) DB_QuerySELECT (&mysql_res,"can not get matches",
					   "SELECT MchCod,"				// row[ 0]
						  "GamCod,"				// row[ 1]
						  "UsrCod,"				// row[ 2]
						  "UNIX_TIMESTAMP(StartTime),"		// row[ 3]
						  "UNIX_TIMESTAMP(EndTime),"		// row[ 4]
						  "Title,"				// row[ 5]
						  "QstInd,"				// row[ 6]
						  "QstCod,"				// row[ 7]
						  "Showing,"				// row[ 8]
					          "ShowQstResults,"			// row[ 9]
					          "ShowUsrResults"			// row[10]
					   " FROM mch_matches"
					   " WHERE GamCod=%ld%s"
					   " ORDER BY MchCod",
					   Game->GamCod,
					   SubQuery);

   /* Free allocated memory for subquery */
   free ((void *) SubQuery);

   /***** Start box *****/
   Gam_CurrentGamCod = Game->GamCod;
   Box_StartBox (NULL,Txt_Matches,Mch_PutIconToPlayNewMatch,
                 Hlp_ASSESSMENT_Games_matches,Box_NOT_CLOSABLE);

   if (NumMatches)
      /***** Show the table with the matches *****/
      Mch_ListOneOrMoreMatches (Game,NumMatches,mysql_res);

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);

   /***** Put button to play a new match in this game *****/
   switch (Gbl.Usrs.Me.Role.Logged)
     {
      case Rol_NET:
      case Rol_TCH:
      case Rol_SYS_ADM:
	 if (PutFormNewMatch)
	    Mch_PutFormNewMatch (Game);			// Form to fill in data and start playing a new match
	 else
	    Mch_PutButtonNewMatch (Game->GamCod);	// Button to create a new match
	 break;
      default:
	 break;
     }

   /***** End box *****/
   Box_EndBox ();
  }

/*****************************************************************************/
/********************** Get match data using its code ************************/
/*****************************************************************************/

void Mch_GetDataOfMatchByCod (struct Match *Match)
  {
   MYSQL_RES *mysql_res;
   unsigned long NumRows;
   Dat_StartEndTime_t StartEndTime;

   /***** Get data of match from database *****/
   NumRows = (unsigned) DB_QuerySELECT (&mysql_res,"can not get matches",
					"SELECT MchCod,"			// row[ 0]
					       "GamCod,"			// row[ 1]
					       "UsrCod,"			// row[ 2]
					       "UNIX_TIMESTAMP(StartTime),"	// row[ 3]
					       "UNIX_TIMESTAMP(EndTime),"	// row[ 4]
					       "Title,"				// row[ 5]
					       "QstInd,"			// row[ 6]
					       "QstCod,"			// row[ 7]
					       "Showing,"			// row[ 8]
					       "ShowQstResults,"		// row[ 9]
					       "ShowUsrResults"			// row[10]
					" FROM mch_matches"
					" WHERE MchCod=%ld"
					" AND GamCod IN"		// Extra check
					" (SELECT GamCod FROM gam_games"
					" WHERE CrsCod='%ld')",
					Match->MchCod,
					Gbl.Hierarchy.Crs.CrsCod);
   if (NumRows) // Match found...
      /***** Get match data from row *****/
      Mch_GetMatchDataFromRow (mysql_res,Match);
   else
     {
      /* Initialize to empty match */
      Match->MchCod                  = -1L;
      Match->GamCod                  = -1L;
      Match->UsrCod                  = -1L;
      for (StartEndTime = (Dat_StartEndTime_t) 0;
	   StartEndTime <= (Dat_StartEndTime_t) (Dat_NUM_START_END_TIME - 1);
	   StartEndTime++)
         Match->TimeUTC[StartEndTime] = (time_t) 0;
      Match->Title[0]                = '\0';
      Match->Status.QstInd           = 0;
      Match->Status.QstCod           = -1L;
      Match->Status.QstStartTimeUTC  = (time_t) 0;
      Match->Status.Showing          = Mch_STEM;
      Match->Status.Playing          = false;
      Match->Status.NumPlayers       = 0;
      Match->Status.ShowQstResults   = false;
      Match->Status.ShowUsrResults   = false;
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/***************** Put icon to add a new questions to game *******************/
/*****************************************************************************/

static void Mch_PutIconToPlayNewMatch (void)
  {
   extern const char *Txt_New_match;

   /***** Put form to create a new question *****/
   Ico_PutContextualIconToAdd (ActReqNewMchTch,Mch_NEW_MATCH_SECTION_ID,Gam_PutParams,
			       Txt_New_match);
  }

/*****************************************************************************/
/*********************** List game matches for edition ***********************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatches (struct Game *Game,
				      unsigned NumMatches,
                                      MYSQL_RES *mysql_res)
  {
   unsigned NumMatch;
   unsigned UniqueId;
   struct Match Match;

   /***** Write the heading *****/
   Tbl_StartTableWideMargin (2);
   Mch_ListOneOrMoreMatchesHeading ();

   /***** Write rows *****/
   for (NumMatch = 0, UniqueId = 1;
	NumMatch < NumMatches;
	NumMatch++, UniqueId++)
     {
      Gbl.RowEvenOdd = NumMatch % 2;

      /***** Get match data from row *****/
      Mch_GetMatchDataFromRow (mysql_res,&Match);

      /***** Write row for this match ****/
      fprintf (Gbl.F.Out,"<tr>");

      /* Icons */
      Mch_ListOneOrMoreMatchesIcons (&Match);

      /* Number of match **/
      Mch_ListOneOrMoreMatchesNumMatch (NumMatch);

      /* Match player */
      Mch_ListOneOrMoreMatchesAuthor (&Match);

      /* Start/end date/time */
      Mch_ListOneOrMoreMatchesTimes (&Match,UniqueId);

      /* Title and groups */
      Mch_ListOneOrMoreMatchesTitleGrps (&Match);

      /* Number of players who have answered any question in the match */
      Mch_ListOneOrMoreMatchesNumPlayers (&Match);

      /* Match status */
      Mch_ListOneOrMoreMatchesStatus (&Match,Game->NumQsts);

      /* Match result visible? */
      Mch_ListOneOrMoreMatchesResult (&Match);

      fprintf (Gbl.F.Out,"</tr>");
     }

   /***** End table *****/
   Tbl_EndTable ();
  }

/*****************************************************************************/
/***************** Put a column for match start and end times ****************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesHeading (void)
  {
   extern const char *Txt_No_INDEX;
   extern const char *Txt_ROLES_SINGUL_Abc[Rol_NUM_ROLES][Usr_NUM_SEXS];
   extern const char *Txt_START_END_TIME[Dat_NUM_START_END_TIME];
   extern const char *Txt_Match;
   extern const char *Txt_Players;
   extern const char *Txt_Status;
   extern const char *Txt_Result;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th></th>"
                      "<th class=\"RIGHT_TOP\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_TOP\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_TOP\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_TOP\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_TOP\">"
                      "%s"
                      "</th>"
                      "<th class=\"RIGHT_TOP\">"
                      "%s"
                      "</th>"
                      "<th class=\"CENTER_TOP\">"
                      "%s"
                      "</th>"
                      "<th class=\"CENTER_TOP\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_No_INDEX,
            Txt_ROLES_SINGUL_Abc[Rol_TCH][Usr_SEX_UNKNOWN],
	    Txt_START_END_TIME[Gam_ORDER_BY_START_DATE],
	    Txt_START_END_TIME[Gam_ORDER_BY_END_DATE],
            Txt_Match,
	    Txt_Players,
            Txt_Status,
            Txt_Result);
  }

/*****************************************************************************/
/************************* Put a column for icons ****************************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesIcons (const struct Match *Match)
  {
   fprintf (Gbl.F.Out,"<td class=\"BT%u\">",Gbl.RowEvenOdd);

   /* Put icon to remove the match */
   Frm_StartForm (ActReqRemMchTch);
   Mch_PutParamMchCod (Match->MchCod);
   Ico_PutIconRemove ();
   Frm_EndForm ();

   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/********************* Put a column for number of match **********************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesNumMatch (unsigned NumMatch)
  {
   fprintf (Gbl.F.Out,"<td class=\"BIG_INDEX RIGHT_TOP COLOR%u\">%u</td>",
	    Gbl.RowEvenOdd,NumMatch + 1);
  }

/*****************************************************************************/
/************* Put a column for teacher who created the match ****************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesAuthor (const struct Match *Match)
  {
   /***** Match author (teacher) *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_TOP COLOR%u\">",Gbl.RowEvenOdd);
   Usr_WriteAuthor1Line (Match->UsrCod,false);
   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/***************** Put a column for match start and end times ****************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesTimes (const struct Match *Match,unsigned UniqueId)
  {
   extern const char *Txt_Today;
   Dat_StartEndTime_t StartEndTime;

   for (StartEndTime = (Dat_StartEndTime_t) 0;
	StartEndTime <= (Dat_StartEndTime_t) (Dat_NUM_START_END_TIME - 1);
	StartEndTime++)
     {
      fprintf (Gbl.F.Out,"<td id=\"mch_time_%u_%u\""
			 " class=\"%s LEFT_TOP COLOR%u\">",
	       (unsigned) StartEndTime,UniqueId,
	       Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION ? "DATE_RED" :
								"DATE_GREEN",
	       Gbl.RowEvenOdd);
      fprintf (Gbl.F.Out,"<script type=\"text/javascript\">"
			 "writeLocalDateHMSFromUTC('mch_time_%u_%u',"
			 "%ld,%u,'<br />','%s',true,true,0x7);"
			 "</script>"
			 "</td>",
	       (unsigned) StartEndTime,UniqueId,
	       Match->TimeUTC[StartEndTime],
	       (unsigned) Gbl.Prefs.DateFormat,Txt_Today);
     }
  }

/*****************************************************************************/
/***************** Put a column for match title and grous ********************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesTitleGrps (const struct Match *Match)
  {
   fprintf (Gbl.F.Out,"<td class=\"LEFT_TOP COLOR%u\">",Gbl.RowEvenOdd);

   /***** Title *****/
   fprintf (Gbl.F.Out,"<span class=\"ASG_TITLE\">%s</span>",Match->Title);

   /***** Groups whose students can answer this match *****/
   if (Gbl.Crs.Grps.NumGrps)
      Mch_GetAndWriteNamesOfGrpsAssociatedToMatch (Match);

   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/************* Get and write the names of the groups of a match **************/
/*****************************************************************************/

static void Mch_GetAndWriteNamesOfGrpsAssociatedToMatch (const struct Match *Match)
  {
   extern const char *Txt_Group;
   extern const char *Txt_Groups;
   extern const char *Txt_and;
   extern const char *Txt_The_whole_course;
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRow;
   unsigned long NumRows;

   /***** Get groups associated to a match from database *****/
   NumRows = DB_QuerySELECT (&mysql_res,"can not get groups of a match",
			     "SELECT crs_grp_types.GrpTypName,crs_grp.GrpName"
			     " FROM mch_groups,crs_grp,crs_grp_types"
			     " WHERE mch_groups.MchCod=%ld"
			     " AND mch_groups.GrpCod=crs_grp.GrpCod"
			     " AND crs_grp.GrpTypCod=crs_grp_types.GrpTypCod"
			     " ORDER BY crs_grp_types.GrpTypName,crs_grp.GrpName",
			     Match->MchCod);

   /***** Write heading *****/
   fprintf (Gbl.F.Out,"<div class=\"ASG_GRP\">%s: ",
            NumRows == 1 ? Txt_Group  :
                           Txt_Groups);

   /***** Write groups *****/
   if (NumRows) // Groups found...
     {
      /* Get and write the group types and names */
      for (NumRow = 0;
	   NumRow < NumRows;
	   NumRow++)
        {
         /* Get next group */
         row = mysql_fetch_row (mysql_res);

         /* Write group type name and group name */
         fprintf (Gbl.F.Out,"%s %s",row[0],row[1]);

         if (NumRows >= 2)
           {
            if (NumRow == NumRows-2)
               fprintf (Gbl.F.Out," %s ",Txt_and);
            if (NumRows >= 3)
              if (NumRow < NumRows-2)
                  fprintf (Gbl.F.Out,", ");
           }
        }
     }
   else
      fprintf (Gbl.F.Out,"%s %s",
               Txt_The_whole_course,Gbl.Hierarchy.Crs.ShrtName);

   fprintf (Gbl.F.Out,"</div>");

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/******************* Put a column for number of players **********************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesNumPlayers (const struct Match *Match)
  {
   /***** Number of players who have answered any question in the match ******/
   fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_TOP COLOR%u\">"
		      "%u"
		      "</td>",
	    Gbl.RowEvenOdd,
	    Mch_GetNumUsrsWhoHaveAnswerMch (Match->MchCod));
  }

/*****************************************************************************/
/********************** Put a column for match status ************************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesStatus (const struct Match *Match,unsigned NumQsts)
  {
   extern const char *Txt_Play;
   extern const char *Txt_Resume;

   fprintf (Gbl.F.Out,"<td class=\"DAT CENTER_TOP COLOR%u\">",Gbl.RowEvenOdd);

   if (Match->Status.QstInd < Mch_AFTER_LAST_QUESTION)	// Unfinished match
      /* Current question index / total of questions */
      fprintf (Gbl.F.Out,"<div class=\"DAT\">%u/%u</div>",
	       Match->Status.QstInd,NumQsts);

   switch (Gbl.Usrs.Me.Role.Logged)
     {
      case Rol_STD:
	 /* Icon to play as student */
	 Mch_CurrentMchCod = Match->MchCod;
	 Lay_PutContextualLinkOnlyIcon (ActPlyMchStd,NULL,
					Mch_PutParamCurrentMchCod,
					Match->Status.QstInd < Mch_AFTER_LAST_QUESTION ? "play.svg" :
											 "flag-checkered.svg",
					Txt_Play);
	 break;
      case Rol_NET:
      case Rol_TCH:
      case Rol_DEG_ADM:
      case Rol_CTR_ADM:
      case Rol_INS_ADM:
      case Rol_SYS_ADM:
	 /* Icon to resume */
	 Mch_CurrentMchCod = Match->MchCod;
	 Lay_PutContextualLinkOnlyIcon (ActResMchTch,NULL,
					Mch_PutParamCurrentMchCod,
					Match->Status.QstInd < Mch_AFTER_LAST_QUESTION ? "play.svg" :
											 "flag-checkered.svg",
					Txt_Resume);
	 break;
      default:
	 break;
     }

   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/**************** Put a column for visibility of match result ****************/
/*****************************************************************************/

static void Mch_ListOneOrMoreMatchesResult (const struct Match *Match)
  {
   extern const char *Txt_Match_result;
   extern const char *Txt_Hidden_result;
   extern const char *Txt_Visible_result;

   fprintf (Gbl.F.Out,"<td class=\"DAT CENTER_TOP COLOR%u\">",Gbl.RowEvenOdd);

   switch (Gbl.Usrs.Me.Role.Logged)
     {
      case Rol_STD:
	 /* Match result visible or hidden? */
	 if (Match->Status.ShowUsrResults)
	   {
	    Frm_StartForm (ActSeeOneMchResMe);
	    Mch_PutParamMchCod (Match->MchCod);
	    Ico_PutIconLink ("tasks.svg",Txt_Match_result);
	    Frm_EndForm ();
	   }
	 else
	    Ico_PutIconOff ("eye-slash.svg",Txt_Hidden_result);
	 break;
      case Rol_NET:
      case Rol_TCH:
      case Rol_DEG_ADM:
      case Rol_CTR_ADM:
      case Rol_INS_ADM:
      case Rol_SYS_ADM:
	 /* Match result visible or hidden? */
	 Mch_CurrentMchCod = Match->MchCod;
	 Lay_PutContextualLinkOnlyIcon (ActChgVisResMchUsr,NULL,
					Mch_PutParamCurrentMchCod,
					Match->Status.ShowUsrResults ? "eye.svg" :
							               "eye-slash.svg",
					Match->Status.ShowUsrResults ? Txt_Visible_result :
							               Txt_Hidden_result);
	 break;
      default:
	 break;
     }

   fprintf (Gbl.F.Out,"</td>");
  }

/*****************************************************************************/
/******************** Toggle visibility of match results *********************/
/*****************************************************************************/

void Mch_ToggleVisibilResultsMchUsr (void)
  {
   struct Match Match;

   /***** Get game code *****/
   if ((Match.MchCod = Mch_GetParamMchCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of match is missing.");

   /***** Get data of the match and the game from database *****/
   Mch_GetDataOfMatchByCod (&Match);

   /***** Toggle visibility of match results *****/
   Match.Status.ShowUsrResults = !Match.Status.ShowUsrResults;
   DB_QueryUPDATE ("can not toggle visibility of match results",
		   "UPDATE mch_matches"
		   " SET ShowUsrResults='%c'"
		   " WHERE MchCod=%ld",
		   Match.Status.ShowUsrResults ? 'Y' :
			                         'N',
		   Match.MchCod);

   /***** Show current game *****/
   Gam_ShowOneGame (Match.GamCod,
                    true,	// Show only this game
                    false,	// Do not list game questions
		    false);	// Do not put form to start new match
  }

/*****************************************************************************/
/******************** Get game data from a database row **********************/
/*****************************************************************************/

static void Mch_GetMatchDataFromRow (MYSQL_RES *mysql_res,
				     struct Match *Match)
  {
   MYSQL_ROW row;
   Dat_StartEndTime_t StartEndTime;

   /***** Get match data *****/
   row = mysql_fetch_row (mysql_res);
   /*
   row[ 0]	MchCod
   row[ 1]	GamCod
   row[ 2]	UsrCod
   row[ 3]	UNIX_TIMESTAMP(StartTime)
   row[ 4]	UNIX_TIMESTAMP(EndTime)
   row[ 5]	Title
   */
   /***** Get match data *****/
   /* Code of the match (row[0]) */
   if ((Match->MchCod = Str_ConvertStrCodToLongCod (row[0])) <= 0)
      Lay_ShowErrorAndExit ("Wrong code of match.");

   /* Code of the game (row[1]) */
   if ((Match->GamCod = Str_ConvertStrCodToLongCod (row[1])) <= 0)
      Lay_ShowErrorAndExit ("Wrong code of game.");

   /* Get match teacher (row[2]) */
   Match->UsrCod = Str_ConvertStrCodToLongCod (row[2]);

   /* Get start/end times (row[3], row[4] hold start/end UTC times) */
   for (StartEndTime = (Dat_StartEndTime_t) 0;
	StartEndTime <= (Dat_StartEndTime_t) (Dat_NUM_START_END_TIME - 1);
	StartEndTime++)
      Match->TimeUTC[StartEndTime] = Dat_GetUNIXTimeFromStr (row[3 + StartEndTime]);

   /* Get the title of the game (row[5]) */
   if (row[5])
      Str_Copy (Match->Title,row[5],
		Gam_MAX_BYTES_TITLE);
   else
      Match->Title[0] = '\0';

   /***** Get current match status *****/
   /*
   row[ 6]	QstInd
   row[ 7]	QstCod
   row[ 8]	Showing
   row[ 9]	ShowQstResults
   row[10]	ShowUsrResults
   */
   /* Current question index (row[6]) */
   Match->Status.QstInd = Gam_GetQstIndFromStr (row[6]);

   /* Current question code (row[7]) */
   Match->Status.QstCod = Str_ConvertStrCodToLongCod (row[7]);

   /* Get what to show (stem, answers, results) (row(8)) */
   Match->Status.Showing = Mch_GetShowingFromStr (row[8]);

   /* Get whether to show question results or not (row(9)) */
   Match->Status.ShowQstResults = (row[9][0] == 'Y');

   /* Get whether to show user results or not (row(10)) */
   Match->Status.ShowUsrResults = (row[10][0] == 'Y');

   /***** Get whether the match is being played or not *****/
   if (Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION)	// Finished
      Match->Status.Playing = false;
   else							// Unfinished
      Match->Status.Playing = Mch_GetIfMatchIsBeingPlayed (Match->MchCod);
  }

/*****************************************************************************/
/****************** Get parameter with what is being shown *******************/
/*****************************************************************************/

static Mch_Showing_t Mch_GetShowingFromStr (const char *Str)
  {
   Mch_Showing_t Showing;

   for (Showing = (Mch_Showing_t) 0;
	Showing <= (Mch_Showing_t) (Mch_NUM_SHOWING - 1);
	Showing++)
      if (!strcmp (Str,Mch_ShowingStringsDB[Showing]))
         return Showing;

   return (Mch_Showing_t) Mch_SHOWING_DEFAULT;
  }

/*****************************************************************************/
/************** Request the removal of a match (game instance) ***************/
/*****************************************************************************/

void Mch_RequestRemoveMatchTch (void)
  {
   extern const char *Txt_Do_you_really_want_to_remove_the_match_X;
   extern const char *Txt_Remove_match;
   struct Match Match;

   /***** Get parameters *****/
   /* Get match code */
   if ((Match.MchCod = Mch_GetParamMchCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of match is missing.");

   /***** Get data of the match from database *****/
   Mch_GetDataOfMatchByCod (&Match);

   /***** Show question and button to remove question *****/
   Mch_CurrentMchCod = Match.MchCod;
   Ale_ShowAlertAndButton (ActRemMchTch,NULL,NULL,Mch_PutParamCurrentMchCod,
			   Btn_REMOVE_BUTTON,Txt_Remove_match,
			   Ale_QUESTION,Txt_Do_you_really_want_to_remove_the_match_X,
	                   Match.Title);

   /***** Show current game *****/
   Gam_ShowOneGame (Match.GamCod,
                    true,	// Show only this game
                    false,	// Do not list game questions
		    false);	// Do not put form to start new match
  }

/*****************************************************************************/
/********************** Remove a match (game instance) ***********************/
/*****************************************************************************/

void Mch_RemoveMatchTch (void)
  {
   extern const char *Txt_Match_X_removed;
   struct Match Match;
   struct Game Game;

   /***** Get parameters *****/
   /* Get match code */
   if ((Match.MchCod = Mch_GetParamMchCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of match is missing.");

   /***** Get data of the match from database *****/
   Mch_GetDataOfMatchByCod (&Match);
   if (Match.MchCod < 0)
      Lay_ShowErrorAndExit ("The match to be removed does not exist.");

   /***** Ensure that the match belongs to this course *****/
   Game.GamCod = Match.GamCod;
   Gam_GetDataOfGameByCod (&Game);
   if (Game.CrsCod != Gbl.Hierarchy.Crs.CrsCod)
      Lay_ShowErrorAndExit ("Match does not belong to this course.");

   /***** Remove the match from all database tables *****/
   Mch_RemoveMatchFromAllTables (Match.MchCod);

   /***** Write message *****/
   Ale_ShowAlert (Ale_SUCCESS,Txt_Match_X_removed,
		  Match.Title);

   /***** Show current game *****/
   Gam_ShowOneGame (Match.GamCod,
                    true,	// Show only this game
                    false,	// Do not list game questions
		    false);	// Do not put form to start new match
  }

/*****************************************************************************/
/********************** Remove match from all tables *************************/
/*****************************************************************************/

static void Mch_RemoveMatchFromAllTables (long MchCod)
  {
   unsigned NumTable;

   /***** Remove match from secondary tables *****/
   for (NumTable = 0;
	NumTable < Mch_NUM_SECONDARY_TABLES;
	NumTable++)
      Mch_RemoveMatchFromTable (MchCod,MatchSecondaryTables[NumTable]);

   /***** Remove match from main table *****/
   DB_QueryDELETE ("can not remove match",
		   "DELETE FROM mch_matches WHERE MchCod=%ld",
		   MchCod);
  }

/*****************************************************************************/
/******************** Remove match in game from all tables *******************/
/*****************************************************************************/

void Mch_RemoveMatchesInGameFromAllTables (long GamCod)
  {
   unsigned NumTable;

   /***** Remove matches from secondary tables *****/
   for (NumTable = 0;
	NumTable < Mch_NUM_SECONDARY_TABLES;
	NumTable++)
      Mch_RemoveMatchesInGameFromTable (GamCod,MatchSecondaryTables[NumTable]);

   /***** Remove matches from main table *****/
   DB_QueryDELETE ("can not remove matches of a game",
		   "DELETE FROM mch_matches WHERE GamCod=%ld",
		   GamCod);
  }

/*****************************************************************************/
/******************* Remove match in course from all tables ******************/
/*****************************************************************************/

void Mch_RemoveMatchInCourseFromAllTables (long CrsCod)
  {
   unsigned NumTable;

   /***** Remove matches from secondary tables *****/
   for (NumTable = 0;
	NumTable < Mch_NUM_SECONDARY_TABLES;
	NumTable++)
      Mch_RemoveMatchInCourseFromTable (CrsCod,MatchSecondaryTables[NumTable]);

   /***** Remove matches from main table *****/
   DB_QueryDELETE ("can not remove matches of a course from table",
		   "DELETE FROM %s"
		   " USING gam_games,mch_matches"
		   " WHERE gam_games.CrsCod=%ld"
		   " AND gam_games.GamCod=mch_matches.GamCod",
		   CrsCod);
  }

/*****************************************************************************/
/******************** Remove match from secondary table **********************/
/*****************************************************************************/

static void Mch_RemoveMatchFromTable (long MchCod,const char *TableName)
  {
   /***** Remove match from secondary table *****/
   DB_QueryDELETE ("can not remove match from table",
		   "DELETE FROM %s WHERE MchCod=%ld",
		   TableName,
		   MchCod);
  }

/*****************************************************************************/
/************* Remove all matches in game from secondary table ***************/
/*****************************************************************************/

static void Mch_RemoveMatchesInGameFromTable (long GamCod,const char *TableName)
  {
   /***** Remove matches in game from secondary table *****/
   DB_QueryDELETE ("can not remove matches of a game from table",
		   "DELETE FROM %s"
		   " USING mch_matches,%s"
		   " WHERE mch_matches.GamCod=%ld"
		   " AND mch_matches.MchCod=%s.MchCod",
		   TableName,
		   TableName,
		   GamCod,
		   TableName);
  }

/*****************************************************************************/
/*********** Remove all matches in course from secondary table ***************/
/*****************************************************************************/

static void Mch_RemoveMatchInCourseFromTable (long CrsCod,const char *TableName)
  {
   /***** Remove matches in course from secondary table *****/
   DB_QueryDELETE ("can not remove matches of a course from table",
		   "DELETE FROM %s"
		   " USING gam_games,mch_matches,%s"
		   " WHERE gam_games.CrsCod=%ld"
		   " AND gam_games.GamCod=mch_matches.GamCod"
		   " AND mch_matches.MchCod=%s.MchCod",
		   TableName,
		   TableName,
		   CrsCod,
		   TableName);
  }

/*****************************************************************************/
/***************** Put parameter with current match code *********************/
/*****************************************************************************/

static void Mch_PutParamCurrentMchCod (void)
  {
   if (Mch_CurrentMchCod > 0)
      Mch_PutParamMchCod (Mch_CurrentMchCod);
  }

/*****************************************************************************/
/******************** Write parameter with code of match **********************/
/*****************************************************************************/

void Mch_PutParamMchCod (long MchCod)
  {
   Par_PutHiddenParamLong ("MchCod",MchCod);
  }

/*****************************************************************************/
/********************* Get parameter with code of match **********************/
/*****************************************************************************/

long Mch_GetParamMchCod (void)
  {
   /***** Get code of match *****/
   return Par_GetParToLong ("MchCod");
  }

/*****************************************************************************/
/********************* Put button to create a new match **********************/
/*****************************************************************************/

static void Mch_PutButtonNewMatch (long GamCod)
  {
   extern const char *Txt_New_match;

   Frm_StartFormAnchor (ActReqNewMchTch,Mch_NEW_MATCH_SECTION_ID);
   Gam_PutParamGameCod (GamCod);
   Btn_PutConfirmButton (Txt_New_match);
   Frm_EndForm ();
  }

/*****************************************************************************/
/****** Put a big button to play match (start a new match) as a teacher ******/
/*****************************************************************************/

static void Mch_PutFormNewMatch (struct Game *Game)
  {
   extern const char *Hlp_ASSESSMENT_Games_new_match;
   extern const char *The_ClassFormInBox[The_NUM_THEMES];
   extern const char *Txt_New_match;
   extern const char *Txt_Title;
   extern const char *Txt_Play;

   /***** Start section for a new match *****/
   Lay_StartSection (Mch_NEW_MATCH_SECTION_ID);

   /***** Start form *****/
   Frm_StartForm (ActNewMchTch);
   Gam_PutParamGameCod (Game->GamCod);
   Gam_PutParamQstInd (0);	// Start by first question in game

   /***** Start box and table *****/
   Box_StartBoxTable (NULL,Txt_New_match,NULL,
		      Hlp_ASSESSMENT_Games_new_match,Box_NOT_CLOSABLE,2);

   /***** Match title *****/
   fprintf (Gbl.F.Out,"<tr>"
	              "<td class=\"RIGHT_MIDDLE\">"
	              "<label for=\"Title\" class=\"%s\">%s:</label>"
	              "</td>"
                      "<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"text\" id=\"Title\" name=\"Title\""
                      " size=\"45\" maxlength=\"%u\" value=\"%s\""
                      " required=\"required\" />"
                      "</td>"
                      "</tr>",
            The_ClassFormInBox[Gbl.Prefs.Theme],
            Txt_Title,
            Gam_MAX_CHARS_TITLE,Game->Title);

   /***** Groups *****/
   Mch_ShowLstGrpsToCreateMatch ();

   /***** End table *****/
   Tbl_EndTable ();

   /***** Put icon with link *****/
   Frm_LinkFormSubmit (Txt_Play,NULL,NULL);
   fprintf (Gbl.F.Out,"<img src=\"%s/play.svg\""
		      " alt=\"%s\" title=\"%s\""
	              " class=\"CONTEXT_OPT ICO_HIGHLIGHT ICO64x64\" />",
            Cfg_URL_ICON_PUBLIC,Txt_Play,Txt_Play);
   fprintf (Gbl.F.Out,"</a>");

   /***** End box *****/
   Box_EndBox ();

   /***** End form *****/
   Frm_EndForm ();

   /***** End section for a new match *****/
   Lay_EndSection ();
  }

/*****************************************************************************/
/***************** Show list of groups to create a new match *****************/
/*****************************************************************************/

static void Mch_ShowLstGrpsToCreateMatch (void)
  {
   extern const char *The_ClassFormInBox[The_NUM_THEMES];
   extern const char *Txt_Groups;
   extern const char *Txt_The_whole_course;
   unsigned NumGrpTyp;

   /***** Get list of groups types and groups in this course *****/
   Grp_GetListGrpTypesAndGrpsInThisCrs (Grp_ONLY_GROUP_TYPES_WITH_GROUPS);

   if (Gbl.Crs.Grps.GrpTypes.Num)
     {
      /***** Start box and table *****/
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"%s RIGHT_TOP\">"
	                 "%s:"
	                 "</td>"
                         "<td class=\"LEFT_TOP\">",
               The_ClassFormInBox[Gbl.Prefs.Theme],
               Txt_Groups);
      Box_StartBoxTable ("95%",NULL,NULL,
                         NULL,Box_NOT_CLOSABLE,0);

      /***** First row: checkbox to select the whole course *****/
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td colspan=\"7\" class=\"DAT LEFT_MIDDLE\">"
                         "<label>"
                         "<input type=\"checkbox\""
                         " id=\"WholeCrs\" name=\"WholeCrs\" value=\"Y\""
                         " checked=\"checked\""
                         " onclick=\"uncheckChildren(this,'GrpCods')\" />"
	                 "%s %s"
	                 "</label>"
	                 "</td>"
	                 "</tr>",
               Txt_The_whole_course,Gbl.Hierarchy.Crs.ShrtName);

      /***** List the groups for each group type *****/
      for (NumGrpTyp = 0;
	   NumGrpTyp < Gbl.Crs.Grps.GrpTypes.Num;
	   NumGrpTyp++)
         if (Gbl.Crs.Grps.GrpTypes.LstGrpTypes[NumGrpTyp].NumGrps)
            Grp_ListGrpsToEditAsgAttSvyMch (&Gbl.Crs.Grps.GrpTypes.LstGrpTypes[NumGrpTyp],
                                            -1L,	// -1 means "New match"
					    Grp_MATCH);

      /***** End table and box *****/
      Box_EndBoxTable ();
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");
     }

   /***** Free list of groups types and groups in this course *****/
   Grp_FreeListGrpTypesAndGrps ();
  }

/*****************************************************************************/
/********************* Create a new match (by a teacher) *********************/
/*****************************************************************************/

void Mch_CreateNewMatchTch (void)
  {
   long GamCod;
   char Title[Gam_MAX_BYTES_TITLE + 1];

   /***** Get form parameters *****/
   /* Get match code */
   if ((GamCod = Gam_GetParamGameCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of game is missing.");

   /* Get match title */
   Par_GetParToText ("Title",Title,Gam_MAX_BYTES_TITLE);

   /* Get groups for this games */
   Grp_GetParCodsSeveralGrps ();

   /***** Create a new match *****/
   Gbl.Games.MchCodBeingPlayed = Mch_CreateMatch (GamCod,Title);

   /***** Free memory for list of selected groups *****/
   Grp_FreeListCodSelectedGrps ();
  }

/*****************************************************************************/
/******* Show button to actually start / resume a match (by a teacher) *******/
/*****************************************************************************/

void Mch_RequestStartResumeMatchTch (void)
  {
   struct Match Match;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Update match status in database *****/
   Mch_UpdateMatchStatusInDB (&Match);

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForTch (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/********************** Create a new match in a game *************************/
/*****************************************************************************/

static long Mch_CreateMatch (long GamCod,char Title[Gam_MAX_BYTES_TITLE + 1])
  {
   long MchCod;

   /***** Insert this new match into database *****/
   MchCod = DB_QueryINSERTandReturnCode ("can not create match",
				         "INSERT mch_matches "
				         "(GamCod,UsrCod,StartTime,EndTime,Title,"
				         "QstInd,QstCod,Showing,"
				         "ShowQstResults,ShowUsrResults)"
				         " VALUES "
				         "(%ld,"	// GamCod
				         "%ld,"		// UsrCod
				         "NOW(),"	// StartTime
				         "NOW(),"	// EndTime
				         "'%s',"	// Title
				         "0,"		// QstInd: Match has not started, so not the first question yet
				         "-1,"		// QstCod: Non-existent question
				         "'%s',"	// Showing: What is being shown
				         "'N',"		// ShowQstResults: Don't show question results initially
				         "'N')",	// ShowUsrResults: Don't show user results initially
				         GamCod,
				         Gbl.Usrs.Me.UsrDat.UsrCod,	// Game creator
				         Title,
					 Mch_ShowingStringsDB[Mch_SHOWING_DEFAULT]);

   /***** Create indexes for answers *****/
   Mch_CreateIndexes (GamCod,MchCod);

   /***** Create groups associated to the match *****/
   if (Gbl.Crs.Grps.LstGrpsSel.NumGrps)
      Mch_CreateGrps (MchCod);

   return MchCod;
  }

/*****************************************************************************/
/*********************** Create indexes for a match **************************/
/*****************************************************************************/
/* Everytime a new match is created,
   the answers of each shufflable question are shuffled.
   The shuffling is stored in a table of indexes
   that will be read when showing a match */

static void Mch_CreateIndexes (long GamCod,long MchCod)
  {
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumQsts;
   unsigned NumQst;
   long QstCod;
   long LongNum;
   unsigned QstInd;
   Tst_AnswerType_t AnswerType;
   bool Shuffle;

   /***** Get questions of the game *****/
   NumQsts = (unsigned)
	     DB_QuerySELECT (&mysql_res,"can not get questions of a game",
			     "SELECT gam_questions.QstCod,"	// row[0]
			            "gam_questions.QstInd,"	// row[1]
			            "tst_questions.AnsType,"	// row[2]
			            "tst_questions.Shuffle"	// row[3]
			     " FROM gam_questions,tst_questions"
			     " WHERE gam_questions.GamCod=%ld"
			     " AND gam_questions.QstCod=tst_questions.QstCod"
			     " ORDER BY gam_questions.QstInd",
			     GamCod);

   /***** For each question in game... *****/
   for (NumQst = 0;
	NumQst < NumQsts;
	NumQst++)
     {
      /***** Get question data *****/
      row = mysql_fetch_row (mysql_res);

      /* Get question code (row[0]) */
      if ((QstCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
	 Lay_ShowErrorAndExit ("Wrong code of question.");

      /* Get question index (row[1]) */
      if ((LongNum = Str_ConvertStrCodToLongCod (row[1])) < 0)
	 Lay_ShowErrorAndExit ("Wrong question index.");
      QstInd = (unsigned) LongNum;

      /* Get answer type (row[2]) */
      AnswerType = Tst_ConvertFromStrAnsTypDBToAnsTyp (row[2]);
      if (AnswerType != Tst_ANS_UNIQUE_CHOICE)
	 Lay_ShowErrorAndExit ("Wrong answer type.");

      /* Get shuffle (row[3]) */
      Shuffle = (row[3][0] == 'Y');

      /***** Reorder answer *****/
      Mch_ReorderAnswer (MchCod,QstInd,QstCod,Shuffle);
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/******************* Reorder answers of a match question *********************/
/*****************************************************************************/

static void Mch_ReorderAnswer (long MchCod,unsigned QstInd,
			       long QstCod,bool Shuffle)
  {
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumAnss;
   unsigned NumAns;
   long LongNum;
   unsigned AnsInd;
   char StrOneAnswer[10 + 1];
   char StrAnswersOneQst[Tst_MAX_BYTES_ANSWERS_ONE_QST + 1];

   /***** Initialize list of answers to empty string *****/
   StrAnswersOneQst[0] = '\0';

   /***** Get questions of the game *****/
   NumAnss = (unsigned)
	     DB_QuerySELECT (&mysql_res,"can not get questions of a game",
			     "SELECT AnsInd"	// row[0]
			     " FROM tst_answers"
			     " WHERE QstCod=%ld"
			     " ORDER BY %s",
			     QstCod,
			     Shuffle ? "RAND()" :	// Use RAND() because is really random; RAND(NOW()) repeats order
				       "AnsInd");

   /***** For each answer in question... *****/
   for (NumAns = 0;
	NumAns < NumAnss;
	NumAns++)
     {
      row = mysql_fetch_row (mysql_res);

      /* Get answer index (row[0]) */
      if ((LongNum = Str_ConvertStrCodToLongCod (row[0])) < 0)
	 Lay_ShowErrorAndExit ("Wrong answer index.");
      AnsInd = (unsigned) LongNum;
      snprintf (StrOneAnswer,sizeof (StrOneAnswer),
		"%u",AnsInd);

      /* Concatenate answer index to list of answers */
      if (NumAns)
         Str_Concat (StrAnswersOneQst,",",
		     Tst_MAX_BYTES_ANSWERS_ONE_QST);
      Str_Concat (StrAnswersOneQst,StrOneAnswer,
		  Tst_MAX_BYTES_ANSWERS_ONE_QST);
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);

   /***** Create entry for this question in table of match indexes *****/
   DB_QueryINSERT ("can not create match indexes",
		   "INSERT INTO mch_indexes"
		   " (MchCod,QstInd,Indexes)"
		   " VALUES"
		   " (%ld,%u,'%s')",
		   MchCod,QstInd,StrAnswersOneQst);
  }

/*****************************************************************************/
/***************** Get indexes for a question from database ******************/
/*****************************************************************************/

void Mch_GetIndexes (long MchCod,unsigned QstInd,
		     unsigned Indexes[Tst_MAX_OPTIONS_PER_QUESTION])
  {
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   char StrIndexesOneQst[Tst_MAX_BYTES_INDEXES_ONE_QST + 1];

   /***** Get indexes for a question from database *****/
   if (!DB_QuerySELECT (&mysql_res,"can not get data of a question",
			"SELECT Indexes"	// row[0]
			" FROM mch_indexes"
			" WHERE MchCod=%ld AND QstInd=%u",
			MchCod,QstInd))
      Lay_ShowErrorAndExit ("No indexes found for a question.");
   row = mysql_fetch_row (mysql_res);

   /* Get indexes (row[0]) */
   Str_Copy (StrIndexesOneQst,row[0],
	     Tst_MAX_BYTES_INDEXES_ONE_QST);

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);

   /***** Get indexes from string *****/
   Par_ReplaceCommaBySeparatorMultiple (StrIndexesOneQst);
   Tst_GetIndexesFromStr (StrIndexesOneQst,Indexes);
  }

/*****************************************************************************/
/******************* Create groups associated to a match *********************/
/*****************************************************************************/

static void Mch_CreateGrps (long MchCod)
  {
   unsigned NumGrpSel;

   /***** Create groups associated to the match *****/
   for (NumGrpSel = 0;
	NumGrpSel < Gbl.Crs.Grps.LstGrpsSel.NumGrps;
	NumGrpSel++)
      /* Create group */
      DB_QueryINSERT ("can not associate a group to a match",
		      "INSERT INTO mch_groups"
		      " (MchCod,GrpCod)"
		      " VALUES"
		      " (%ld,%ld)",
                      MchCod,Gbl.Crs.Grps.LstGrpsSel.GrpCods[NumGrpSel]);
  }

/*****************************************************************************/
/********************* Remove one group from all matches *********************/
/*****************************************************************************/

void Mch_RemoveGroup (long GrpCod)
  {
   /***** Remove group from all the matches *****/
   DB_QueryDELETE ("can not remove group"
	           " from the associations between matches and groups",
		   "DELETE FROM mch_groups WHERE GrpCod=%ld",
		   GrpCod);
  }

/*****************************************************************************/
/***************** Remove groups of one type from all matches ****************/
/*****************************************************************************/

void Mch_RemoveGroupsOfType (long GrpTypCod)
  {
   /***** Remove group from all the matches *****/
   DB_QueryDELETE ("can not remove groups of a type"
	           " from the associations between matches and groups",
		   "DELETE FROM mch_groups"
		   " USING crs_grp,mch_groups"
		   " WHERE crs_grp.GrpTypCod=%ld"
		   " AND crs_grp.GrpCod=mch_groups.GrpCod",
                   GrpTypCod);
  }

/*****************************************************************************/
/***************** Insert/update a game match being played *******************/
/*****************************************************************************/

#define Mch_MAX_BYTES_SUBQUERY 128

static void Mch_UpdateMatchStatusInDB (struct Match *Match)
  {
   char MchSubQuery[Mch_MAX_BYTES_SUBQUERY];

   /***** Update end time only if match is currently being played *****/
   if (Match->Status.Playing)
      Str_Copy (MchSubQuery,"mch_matches.EndTime=NOW(),",
		Mch_MAX_BYTES_SUBQUERY);
   else
      MchSubQuery[0] = '\0';

   /***** Update match status in database *****/
   DB_QueryUPDATE ("can not update match being played",
		   "UPDATE mch_matches,gam_games"
		   " SET %s"
			"mch_matches.QstInd=%u,"
			"mch_matches.QstCod=%ld,"
			"mch_matches.Showing='%s',"
			"mch_matches.ShowQstResults='%c'"
		   " WHERE mch_matches.MchCod=%ld"
		   " AND mch_matches.GamCod=gam_games.GamCod"
		   " AND gam_games.CrsCod=%ld",	// Extra check
		   MchSubQuery,
		   Match->Status.QstInd,Match->Status.QstCod,
		   Mch_ShowingStringsDB[Match->Status.Showing],
		   Match->Status.ShowQstResults ? 'Y' : 'N',
		   Match->MchCod,Gbl.Hierarchy.Crs.CrsCod);

   if (Match->Status.Playing)
      /* Update match as being played */
      Mch_UpdateMatchAsBeingPlayed (Match->MchCod);
   else
      /* Update match as not being played */
      Mch_SetMatchAsNotBeingPlayed (Match->MchCod);
  }

/*****************************************************************************/
/********** Update elapsed time in current question (by a teacher) ***********/
/*****************************************************************************/

static void Mch_UpdateElapsedTimeInQuestion (struct Match *Match)
  {
   /***** Update elapsed time in current question in database *****/
   if (Match->Status.Playing &&
       Match->Status.QstInd > 0 &&
       Match->Status.QstInd < Mch_AFTER_LAST_QUESTION)
      DB_QueryINSERT ("can not update elapsed time in question",
		      "INSERT INTO mch_times (MchCod,QstInd,ElapsedTime)"
		      " VALUES (%ld,%u,SEC_TO_TIME(%u))"
		      " ON DUPLICATE KEY"
		      " UPDATE ElapsedTime=ADDTIME(ElapsedTime,SEC_TO_TIME(%u))",
		      Match->MchCod,Match->Status.QstInd,
		      Cfg_SECONDS_TO_REFRESH_MATCH,
		      Cfg_SECONDS_TO_REFRESH_MATCH);
  }

/*****************************************************************************/
/******************* Get elapsed time in a match question ********************/
/*****************************************************************************/

static void Mch_GetElapsedTimeInQuestion (struct Match *Match,
					  struct Time *Time)
  {
   MYSQL_RES *mysql_res;
   unsigned NumRows;

   /***** Query database *****/
   NumRows = (unsigned) DB_QuerySELECT (&mysql_res,"can not get elapsed time",
				        "SELECT ElapsedTime"
				        " FROM mch_times"
				        " WHERE MchCod=%ld AND QstInd=%u",
				        Match->MchCod,Match->Status.QstInd);

   /***** Get elapsed time from query result *****/
   Mch_GetElapsedTime (NumRows,mysql_res,Time);

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/*********************** Get elapsed time in a match *************************/
/*****************************************************************************/

static void Mch_GetElapsedTimeInMatch (struct Match *Match,
				       struct Time *Time)
  {
   MYSQL_RES *mysql_res;
   unsigned NumRows;

   /***** Query database *****/
   NumRows = (unsigned) DB_QuerySELECT (&mysql_res,"can not get elapsed time",
				        "SELECT SEC_TO_TIME(SUM(TIME_TO_SEC(ElapsedTime)))"
				        " FROM mch_times WHERE MchCod=%ld",
				        Match->MchCod);

   /***** Get elapsed time from query result *****/
   Mch_GetElapsedTime (NumRows,mysql_res,Time);

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/*********************** Get elapsed time in a match *************************/
/*****************************************************************************/

static void Mch_GetElapsedTime (unsigned NumRows,MYSQL_RES *mysql_res,
				struct Time *Time)
  {
   MYSQL_ROW row;
   bool ElapsedTimeGotFromDB = false;

   /***** Get time from H...H:MM:SS string *****/
   if (NumRows)
     {
      row = mysql_fetch_row (mysql_res);

      if (row[0])
	 /* Get the elapsed time (row[0]) */
	 if (sscanf (row[0],"%u:%02u:%02u",&Time->Hour,&Time->Minute,&Time->Second) == 3)
	    ElapsedTimeGotFromDB = true;
     }

   /***** Initialize time to default value (0) *****/
   if (!ElapsedTimeGotFromDB)
      Time->Hour   =
      Time->Minute =
      Time->Second = 0;
  }

/*****************************************************************************/
/********************* Pause current match (by a teacher) ********************/
/*****************************************************************************/

void Mch_PauseMatchTch (void)
  {
   struct Match Match;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Update status *****/
   Match.Status.Playing = false;	// Pause match

   /***** Update match status in database *****/
   Mch_UpdateMatchStatusInDB (&Match);

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForTch (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/** Show current match status (current question, answers...) (by a teacher) **/
/*****************************************************************************/

void Mch_ResumeMatchTch (void)
  {
   struct Match Match;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** If not yet finished, update status *****/
   if (Match.Status.QstInd < Mch_AFTER_LAST_QUESTION)	// Unfinished
     {
      if (Match.Status.QstInd == 0)			// Match has been created, but it has not started
	 Mch_SetMatchStatusToNext (&Match);
      Match.Status.Playing = true;			// Start/resume match
     }

   /***** Update match status in database *****/
   Mch_UpdateMatchStatusInDB (&Match);

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForTch (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/********* Toggle the display of results in a match (by a teacher) ***********/
/*****************************************************************************/

void Mch_ToggleVisibilResultsMchQst (void)
  {
   struct Match Match;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Update status *****/
   Match.Status.ShowQstResults = !Match.Status.ShowQstResults;	// Toggle display
   if (Match.Status.Showing == Mch_RESULTS &&
       !Match.Status.ShowQstResults)
     Match.Status.Showing = Mch_ANSWERS;	// Hide results

   /***** Update match status in database *****/
   Mch_UpdateMatchStatusInDB (&Match);

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForTch (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/************* Show previous question in a match (by a teacher) **************/
/*****************************************************************************/

void Mch_BackMatchTch (void)
  {
   struct Match Match;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Update status *****/
   Mch_SetMatchStatusToPrev (&Match);

   /***** Update match status in database *****/
   Mch_UpdateMatchStatusInDB (&Match);

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForTch (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/*************** Show next question in a match (by a teacher) ****************/
/*****************************************************************************/

void Mch_ForwardMatchTch (void)
  {
   struct Match Match;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Update status *****/
   Mch_SetMatchStatusToNext (&Match);

   /***** Update match status in database *****/
   Mch_UpdateMatchStatusInDB (&Match);

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForTch (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/************** Set match status to previous (backward) status ***************/
/*****************************************************************************/

static void Mch_SetMatchStatusToPrev (struct Match *Match)
  {
   /***** What to show *****/
   if (Match->Status.QstInd == 0)				// Start
      Mch_SetMatchStatusToStart (Match);
   else if (Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION)	// End
      Mch_SetMatchStatusToPrevQst (Match);
   else								// Between start and end
      switch (Match->Status.Showing)
	{
	 case Mch_NOTHING:
	 case Mch_STEM:
	    Mch_SetMatchStatusToPrevQst (Match);
	    break;
	 case Mch_ANSWERS:
	    Match->Status.Showing = Mch_STEM;
	    break;
	 case Mch_RESULTS:
	    Match->Status.Showing = Mch_ANSWERS;
	    break;
	}
  }

static void Mch_SetMatchStatusToPrevQst (struct Match *Match)
  {
   /***** Get index of the previous question *****/
   Match->Status.QstInd = Gam_GetPrevQuestionIndexInGame (Match->GamCod,
							  Match->Status.QstInd);
   if (Match->Status.QstInd == 0)		// Start of questions has been reached
      Mch_SetMatchStatusToStart (Match);
   else
     {
      Match->Status.QstCod = Gam_GetQstCodFromQstInd (Match->GamCod,
						      Match->Status.QstInd);
      Match->Status.Showing = Match->Status.ShowQstResults ? Mch_RESULTS :
							     Mch_ANSWERS;
     }
  }

static void Mch_SetMatchStatusToStart (struct Match *Match)
  {
   Match->Status.QstInd  = 0;				// Before first question
   Match->Status.QstCod  = -1L;
   Match->Status.Playing = false;
   Match->Status.Showing = Mch_NOTHING;
  }

/*****************************************************************************/
/**************** Set match status to next (forward) status ******************/
/*****************************************************************************/

static void Mch_SetMatchStatusToNext (struct Match *Match)
  {
   /***** What to show *****/
   if (Match->Status.QstInd == 0)				// Start
      Mch_SetMatchStatusToNextQst (Match);
   else if (Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION)	// End
      Mch_SetMatchStatusToEnd (Match);
   else								// Between start and end
      switch (Match->Status.Showing)
	{
	 case Mch_NOTHING:
	    Match->Status.Showing = Mch_STEM;
	    break;
	 case Mch_STEM:
	    Match->Status.Showing = Mch_ANSWERS;
	    break;
	 case Mch_ANSWERS:
	    if (Match->Status.ShowQstResults)
	       Match->Status.Showing = Mch_RESULTS;
	    else
	       Mch_SetMatchStatusToNextQst (Match);
	    break;
	 case Mch_RESULTS:
	    Mch_SetMatchStatusToNextQst (Match);
	    break;
	}
  }

static void Mch_SetMatchStatusToNextQst (struct Match *Match)
  {
   /***** Get index of the next question *****/
   Match->Status.QstInd = Gam_GetNextQuestionIndexInGame (Match->GamCod,
							  Match->Status.QstInd);

   /***** Get question code *****/
   if (Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION)	// Finished
      Mch_SetMatchStatusToEnd (Match);
   else							// Unfinished
     {
      Match->Status.QstCod = Gam_GetQstCodFromQstInd (Match->GamCod,
						      Match->Status.QstInd);
      Match->Status.Showing = Mch_STEM;
     }
  }

static void Mch_SetMatchStatusToEnd (struct Match *Match)
  {
   Match->Status.QstInd  = Mch_AFTER_LAST_QUESTION;	// After last question
   Match->Status.QstCod  = -1L;
   Match->Status.Playing = false;
   Match->Status.Showing = Mch_NOTHING;
  }

/*****************************************************************************/
/******* Show current match status (number, question, answers, button) *******/
/*****************************************************************************/

static void Mch_ShowMatchStatusForTch (struct Match *Match)
  {
   /***** Left column *****/
   Mch_ShowLeftColumnTch (Match);

   /***** Right column *****/
   Mch_ShowRightColumnTch (Match);
  }

/*****************************************************************************/
/************ Show current question being played for a student ***************/
/*****************************************************************************/

static void Mch_ShowMatchStatusForStd (struct Match *Match)
  {
   bool IBelongToGroups;

   /***** Do I belong to valid groups to play this match? *****/
   IBelongToGroups = Gbl.Usrs.Me.IBelongToCurrentCrs &&
		     Mch_CheckIfIPlayThisMatchBasedOnGrps (Match->MchCod);
   if (!IBelongToGroups)
      Lay_ShowErrorAndExit ("You can not play this match!");

   /***** Left column *****/
   Mch_ShowLeftColumnStd (Match);

   /***** Right column *****/
   Mch_ShowRightColumnStd (Match);
  }

/*****************************************************************************/
/******************* Get number of questions of a game *********************/
/*****************************************************************************/

unsigned Mch_GetNumMchsInGame (long GamCod)
  {
   /***** Trivial check *****/
   if (GamCod < 0)	// A non-existing game...
      return 0;		// ...has no matches

   /***** Get number of matches in a game from database *****/
   return
   (unsigned) DB_QueryCOUNT ("can not get number of matches of a game",
			     "SELECT COUNT(*) FROM mch_matches"
			     " WHERE GamCod=%ld",
			     GamCod);
  }

/*****************************************************************************/
/************ Check if I belong to any of the groups of a match **************/
/*****************************************************************************/

static bool Mch_CheckIfIPlayThisMatchBasedOnGrps (long MchCod)
  {
   /***** Get if I can play a match from database *****/
   return (DB_QueryCOUNT ("can not check if I can play a match",
			  "SELECT COUNT(*) FROM mch_matches"
			  " WHERE MchCod=%ld"
			  " AND (MchCod NOT IN (SELECT MchCod FROM mch_groups) OR"
			  " MchCod IN (SELECT mch_groups.MchCod FROM mch_groups,crs_grp_usr"
			  " WHERE crs_grp_usr.UsrCod=%ld"
			  " AND mch_groups.GrpCod=crs_grp_usr.GrpCod))",
			  MchCod,Gbl.Usrs.Me.UsrDat.UsrCod) != 0);
  }

/*****************************************************************************/
/*********** Show left column when playing a match (as a teacher) ************/
/*****************************************************************************/

static void Mch_ShowLeftColumnTch (struct Match *Match)
  {
   /***** Start left container *****/
   fprintf (Gbl.F.Out,"<div id=\"match_left\" class=\"MATCH_LEFT\">");

   /***** Show content of div *****/
   Mch_ShowInsideLeftColumnTch (Match);

   /***** End left container *****/
   fprintf (Gbl.F.Out,"</div>");
  }

static void Mch_ShowInsideLeftColumnTch (struct Match *Match)
  {
   extern const char *Txt_MATCH_respond;
   struct Time Time;
   unsigned NumAnswerersQst;

   /***** Top *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_TOP\">");

   /* Write elapsed time in match */
   Mch_GetElapsedTimeInMatch (Match,&Time);
   Dat_WriteHoursMinutesSeconds (&Time);

   fprintf (Gbl.F.Out,"</div>");

   /***** Write number of question *****/
   Mch_ShowNumQstInMatch (Match);

   /***** Write elapsed time in question *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_TIME_QST\">");
   if (Match->Status.QstInd > 0 &&
       Match->Status.QstInd < Mch_AFTER_LAST_QUESTION)
     {
      Mch_GetElapsedTimeInQuestion (Match,&Time);
      Dat_WriteHoursMinutesSeconds (&Time);
     }
   else
      fprintf (Gbl.F.Out,"-");
   fprintf (Gbl.F.Out,"</div>");

   /***** Buttons *****/
   Mch_PutMatchControlButtons (Match);

   /***** Write button to request viewing results *****/
   Mch_PutCheckboxResult (Match);

   /***** Get current number of players *****/
   Mch_GetNumPlayers (Match);

   /***** Number of players *****/
   Mch_ShowNumPlayers (Match);

   /***** Number of users who have answered this question *****/
   if (Match->Status.Playing)
     {
      NumAnswerersQst = Mch_GetNumUsrsWhoHaveAnswerQst (Match->MchCod,
			                                Match->Status.QstInd);
      fprintf (Gbl.F.Out,"<div class=\"MATCH_NUM_ANSWERERS\">"
                         "%s<br />"
                         "<strong>%u/%u</strong>"
	                 "</div>",
	       Txt_MATCH_respond,
	       NumAnswerersQst,Match->Status.NumPlayers);
     }
  }

/*****************************************************************************/
/********** Show right column when playing a match (as a teacher) ************/
/*****************************************************************************/

static void Mch_ShowRightColumnTch (struct Match *Match)
  {
   /***** Start right container *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_RIGHT\">");

   /***** Top row: match title *****/
   Mch_ShowMatchTitle (Match);

   /***** Bottom row: current question and possible answers *****/
   Mch_ShowQuestionAndAnswersTch (Match);

   /***** End right container *****/
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/*********** Show left column when playing a match (as a student) ************/
/*****************************************************************************/

static void Mch_ShowLeftColumnStd (struct Match *Match)
  {
   /***** Start left container *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_LEFT\">");

   /***** Top *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_TOP\"></div>");

   /***** Write number of question *****/
   Mch_ShowNumQstInMatch (Match);

   /***** End left container *****/
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/********** Show right column when playing a match (as a student) ************/
/*****************************************************************************/

static void Mch_ShowRightColumnStd (struct Match *Match)
  {
   extern const char *Txt_Please_wait_;

   /***** Start right container *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_RIGHT\">");

   /***** Top row *****/
   Mch_ShowMatchTitle (Match);

   /***** Bottom row *****/
   if (Match->Status.QstInd < Mch_AFTER_LAST_QUESTION)	// Unfinished
     {
      fprintf (Gbl.F.Out,"<div class=\"MATCH_BOTTOM\">");

      /***** Update players ******/
      Mch_RegisterMeAsPlayerInMatch (Match->MchCod);

      if (Match->Status.Playing)
	 /* Show current question and possible answers */
	 Mch_ShowQuestionAndAnswersStd (Match);
      else	// Not being played
	 Mch_ShowWaitImage (Txt_Please_wait_);

      fprintf (Gbl.F.Out,"</div>");
     }

   /***** End right container *****/
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/********************* Show number of question in game ***********************/
/*****************************************************************************/

static void Mch_ShowNumQstInMatch (struct Match *Match)
  {
   extern const char *Txt_MATCH_Start;
   extern const char *Txt_MATCH_End;
   unsigned NumQsts = Gam_GetNumQstsGame (Match->GamCod);

   fprintf (Gbl.F.Out,"<div class=\"MATCH_NUM_QST\">");
   if (Match->Status.QstInd == 0)				// Not started
      fprintf (Gbl.F.Out,"%s",Txt_MATCH_Start);
   else if (Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION)	// Finished
      fprintf (Gbl.F.Out,"%s",Txt_MATCH_End);
   else
      fprintf (Gbl.F.Out,"%u/%u",Match->Status.QstInd,NumQsts);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/********************** Put buttons to control a match ***********************/
/*****************************************************************************/

static void Mch_PutMatchControlButtons (struct Match *Match)
  {
   extern const char *Txt_Go_back;
   extern const char *Txt_Go_forward;
   extern const char *Txt_Pause;
   extern const char *Txt_Start;
   extern const char *Txt_Resume;

   /***** Start buttons container *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_BUTTONS_CONTAINER\">");

   /***** Left button *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_BUTTON_LEFT_CONTAINER\">");
   if (Match->Status.QstInd == 0)
      /* Put button to close browser tab */
      Mch_PutBigButtonClose ();
   else
      /* Put button to go back */
      Mch_PutBigButton (ActBckMchTch,Match->MchCod,
			Mch_ICON_PREVIOUS,Txt_Go_back);
   fprintf (Gbl.F.Out,"</div>");

   /***** Center button *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_BUTTON_CENTER_CONTAINER\">");
   if (Match->Status.Playing)					// Being played
      /* Put button to pause match */
      Mch_PutBigButton (ActPauMchTch,
			Match->MchCod,
			Mch_ICON_PAUSE,Txt_Pause);
   else								// Paused
     {
      if (Match->Status.QstInd < Mch_AFTER_LAST_QUESTION)	// Not finished
	 /* Put button to play match */
	 Mch_PutBigButton (ActPlyMchTch,
			   Match->MchCod,
			   Mch_ICON_PLAY,Match->Status.QstInd == 0 ? Txt_Start :
								     Txt_Resume);
      else							// Finished
	 /* Put disabled button to play match */
	 Mch_PutBigButtonOff (Mch_ICON_PLAY);
     }
   fprintf (Gbl.F.Out,"</div>");

   /***** Right button *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_BUTTON_RIGHT_CONTAINER\">");
   if (Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION)	// Finished
      /* Put button to close browser tab */
      Mch_PutBigButtonClose ();
   else
      /* Put button to show answers */
      Mch_PutBigButton (ActFwdMchTch,Match->MchCod,
			Mch_ICON_NEXT,Txt_Go_forward);
   fprintf (Gbl.F.Out,"</div>");

   /***** End buttons container *****/
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/***************** Put checkbox to select if show results ********************/
/*****************************************************************************/

static void Mch_PutCheckboxResult (struct Match *Match)
  {
   extern const char *Txt_View_results;

   /***** Start container *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_SHOW_RESULTS\">");

   /***** Start form *****/
   Frm_StartForm (ActChgVisResMchQst);
   Mch_PutParamMchCod (Match->MchCod);	// Current match being played

   /***** Put icon with link *****/
   /* Submitting onmousedown instead of default onclick
      is necessary in order to be fast
      and not lose clicks due to refresh */
   fprintf (Gbl.F.Out,"<div class=\"CONTEXT_OPT\">"
	              "<a href=\"\" class=\"ICO_HIGHLIGHT\""
	              " title=\"%s\" "
	              " onmousedown=\"document.getElementById('%s').submit();"
	              " return false;\">"
	              "<i class=\"%s\"></i>"
	              "&nbsp;%s"
	              "</a>"
	              "</div>",
	    Txt_View_results,
	    Gbl.Form.Id,
	    Match->Status.ShowQstResults ? "fas fa-toggle-on" :
		                           "fas fa-toggle-off",
	    Txt_View_results);

   /***** End form *****/
   Frm_EndForm ();

   /***** End container *****/
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/************************** Show number of players ***************************/
/*****************************************************************************/

static void Mch_ShowNumPlayers (struct Match *Match)
  {
   extern const char *Txt_Players;

   fprintf (Gbl.F.Out,"<div class=\"MATCH_NUM_PLAYERS\">"
	              "%s<br />"
                      "<strong>%u</strong>"
	              "</div>",
	    Txt_Players,Match->Status.NumPlayers);
  }

/*****************************************************************************/
/***************************** Show match title ******************************/
/*****************************************************************************/

static void Mch_ShowMatchTitle (struct Match *Match)
  {
   /***** Match title *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_TOP\">%s</div>",Match->Title);
  }

/*****************************************************************************/
/***** Show question and its answers when playing a match (as a teacher) *****/
/*****************************************************************************/

static void Mch_ShowQuestionAndAnswersTch (struct Match *Match)
  {
   extern const char *Txt_MATCH_Paused;
   extern const char *Txt_View_results;
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;

   /***** Trivial check: question index should be correct *****/
   if (Match->Status.QstInd == 0 ||
       Match->Status.QstInd >= Mch_AFTER_LAST_QUESTION)
      return;

   /***** Get data of question from database *****/
   if (!DB_QuerySELECT (&mysql_res,"can not get data of a question",
			"SELECT AnsType,"	// row[0]
			       "Stem,"		// row[1]
			       "MedCod"		// row[2]
			" FROM tst_questions"
			" WHERE QstCod=%ld",
			Match->Status.QstCod))
      Ale_ShowAlert (Ale_ERROR,"Question doesn't exist.");
   row = mysql_fetch_row (mysql_res);

   /***** Show question *****/
   /* Get answer type (row[0]) */
   Gbl.Test.AnswerType = Tst_ConvertFromStrAnsTypDBToAnsTyp (row[0]);
   if (Gbl.Test.AnswerType != Tst_ANS_UNIQUE_CHOICE)
      Lay_ShowErrorAndExit ("Wrong answer type.");

   fprintf (Gbl.F.Out,"<div class=\"MATCH_BOTTOM\">");	// Bottom

   /* Write stem (row[1]) */
   Tst_WriteQstStem (row[1],"MATCH_TCH_STEM");

   /* Get media (row[2]) */
   Gbl.Test.Media.MedCod = Str_ConvertStrCodToLongCod (row[2]);
   Med_GetMediaDataByCod (&Gbl.Test.Media);

   /* Show media */
   Med_ShowMedia (&Gbl.Test.Media,
		  "TEST_MED_EDIT_LIST_STEM_CONTAINER",
		  "TEST_MED_EDIT_LIST_STEM");

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);

   /***** Write answers? *****/
   switch (Match->Status.Showing)
     {
      case Mch_NOTHING:
      case Mch_STEM:
	 /* Don't write anything */
	 break;
      case Mch_ANSWERS:
	 if (Match->Status.Playing)			// Being played
	    /* Write answers */
	    Tst_WriteAnswersMatchResult (Match->MchCod,
					 Match->Status.QstInd,
					 Match->Status.QstCod,
					 "MATCH_TCH_ANS",
					 false);	// Don't show result
	 else						// Not being played
	    Mch_ShowWaitImage (Txt_MATCH_Paused);
	 break;
      case Mch_RESULTS:
	 /* Write answers with results */
	 Tst_WriteAnswersMatchResult (Match->MchCod,
				      Match->Status.QstInd,
				      Match->Status.QstCod,
				      "MATCH_TCH_ANS",
				      true);		// Show result
	 break;
     }

   fprintf (Gbl.F.Out,"</div>");			// Bottom
  }

/*****************************************************************************/
/***** Show question and its answers when playing a match (as a student) *****/
/*****************************************************************************/

static void Mch_ShowQuestionAndAnswersStd (struct Match *Match)
  {
   struct Mch_UsrAnswer UsrAnswer;
   unsigned NumOptions;
   unsigned NumOpt;

   /***** Show question *****/
   /* Write buttons for answers? */
   if (Match->Status.Showing == Mch_ANSWERS)
     {
      if (Tst_CheckIfQuestionIsValidForGame (Match->Status.QstCod))
	{
	 /***** Get student's answer to this question
		(<0 ==> no answer) *****/
	 Mch_GetQstAnsFromDB (Match->MchCod,
			      Gbl.Usrs.Me.UsrDat.UsrCod,
			      Match->Status.QstInd,
			      &UsrAnswer);

	 /***** Get number of options in this question *****/
	 NumOptions = Tst_GetNumAnswersQst (Match->Status.QstCod);

	 /***** Start table *****/
	 Tbl_StartTableWide (8);

	 for (NumOpt = 0;
	      NumOpt < NumOptions;
	      NumOpt++)
	   {
	    /***** Start row *****/
	    fprintf (Gbl.F.Out,"<tr>");

	    /***** Write letter for this option *****/
	    /* Start table cell */
	    fprintf (Gbl.F.Out,"<td class=\"MATCH_STD_CELL\">");

	    /* Form with button.
	       Sumitting onmousedown instead of default onclick
	       is necessary in order to be fast
	       and not lose clicks due to refresh */
	    Frm_StartForm (ActAnsMchQstStd);
	    Mch_PutParamMchCod (Match->MchCod);	// Current match being played
	    Gam_PutParamQstInd (Match->Status.QstInd);	// Current question index shown
	    Mch_PutParamNumOpt (NumOpt);		// Number of button
	    fprintf (Gbl.F.Out,"<button type=\"submit\""
			       " onmousedown=\"document.getElementById('%s').submit();"
			       "return false;\" class=\"",
		     Gbl.Form.Id);
	    if (UsrAnswer.NumOpt == (int) NumOpt)	// Student's answer
	       fprintf (Gbl.F.Out,"MATCH_STD_ANSWER_SELECTED ");
	    fprintf (Gbl.F.Out,"MATCH_STD_BUTTON BT_%c\">"
			       "%c"
			       "</button>",
		     'A' + (char) NumOpt,
		     'a' + (char) NumOpt);
	    Frm_EndForm ();

	    /* End table cell */
	    fprintf (Gbl.F.Out,"</td>");

	    /***** End row *****/
	    fprintf (Gbl.F.Out,"</tr>");
	   }

	 /***** End table *****/
	 Tbl_EndTable ();
	}
      else
	 Ale_ShowAlert (Ale_ERROR,"Type of answer not valid in a game.");
     }
  }

/*****************************************************************************/
/****** Write parameter with number of option (button) pressed by user *******/
/*****************************************************************************/

static void Mch_PutParamNumOpt (unsigned NumOpt)
  {
   Par_PutHiddenParamUnsigned ("NumOpt",NumOpt);
  }

/*****************************************************************************/
/******* Get parameter with number of option (button) pressed by user ********/
/*****************************************************************************/

static unsigned Mch_GetParamNumOpt (void)
  {
   long NumOpt;

   NumOpt = Par_GetParToLong ("NumOpt");
   if (NumOpt < 0)
      Lay_ShowErrorAndExit ("Wrong number of option.");

   return (unsigned) NumOpt;
  }

/*****************************************************************************/
/*********************** Put a big button to do action ***********************/
/*****************************************************************************/

static void Mch_PutBigButton (Act_Action_t NextAction,long MchCod,
			      const char *Icon,const char *Txt)
  {
   /***** Start form *****/
   Frm_StartForm (NextAction);
   Mch_PutParamMchCod (MchCod);

   /***** Put icon with link *****/
   /* Submitting onmousedown instead of default onclick
      is necessary in order to be fast
      and not lose clicks due to refresh */
   fprintf (Gbl.F.Out,"<div class=\"MATCH_BUTTON_CONTAINER\">"
                      "<a href=\"\" class=\"MATCH_BUTTON_ON\" title=\"%s\" "
	              " onmousedown=\"document.getElementById('%s').submit();"
	              " return false;\">"
	              "<i class=\"%s\"></i>"
	              "</a>"
	              "</div>",
	    Txt,
	    Gbl.Form.Id,
	    Icon);

   /***** End form *****/
   Frm_EndForm ();
  }

static void Mch_PutBigButtonOff (const char *Icon)
  {
   /***** Put inactive icon *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_BUTTON_CONTAINER\">"
                      "<div class=\"MATCH_BUTTON_OFF\">"
	              "<i class=\"%s\"></i>"
	              "</div>"
	              "</div>",
	    Icon);
  }

static void Mch_PutBigButtonClose (void)
  {
   extern const char *Txt_Close;

   /***** Put icon with link *****/
   /* onmousedown instead of default onclick
      is necessary in order to be fast
      and not lose clicks due to refresh */
   fprintf (Gbl.F.Out,"<div class=\"MATCH_BUTTON_CONTAINER\">"
                      "<a href=\"\" class=\"MATCH_BUTTON_ON\" title=\"%s\" "
	              " onmousedown=\"window.close();"
	              " return false;\"\">"
	              "<i class=\"%s\"></i>"
	              "</a>"
	              "</div>",
	    Txt_Close,Mch_ICON_CLOSE);
  }

/*****************************************************************************/
/****************************** Show wait image ******************************/
/*****************************************************************************/

static void Mch_ShowWaitImage (const char *Txt)
  {
   fprintf (Gbl.F.Out,"<div class=\"MATCH_WAIT_CONTAINER\">"
		      "<img src=\"%s/wait.gif\""
		      " alt=\"%s\" title=\"%s\""
		      " class=\"MATCH_WAIT_IMAGE\" />"
		      "</div>",
	    Cfg_URL_ICON_PUBLIC,
	    Txt,
	    Txt);
  }

/*****************************************************************************/
/**************************** Remove old players *****************************/
/*****************************************************************************/

static void Mch_RemoveOldPlayers (void)
  {
   /***** Delete matches not being played *****/
   DB_QueryDELETE ("can not update matches as not being played",
		   "DELETE FROM mch_playing"
		   " WHERE TS<FROM_UNIXTIME(UNIX_TIMESTAMP()-%lu)",
		   Cfg_SECONDS_TO_REFRESH_MATCH*3);

   /***** Delete players who have left matches *****/
   DB_QueryDELETE ("can not update match players",
		   "DELETE FROM mch_players"
		   " WHERE TS<FROM_UNIXTIME(UNIX_TIMESTAMP()-%lu)",
		   Cfg_SECONDS_TO_REFRESH_MATCH*3);
  }

static void Mch_UpdateMatchAsBeingPlayed (long MchCod)
  {
   /***** Insert match as being played *****/
   DB_QueryREPLACE ("can not set match as being played",
		    "REPLACE mch_playing (MchCod) VALUE (%ld)",
		    MchCod);
  }

static void Mch_SetMatchAsNotBeingPlayed (long MchCod)
  {
   /***** Delete all match players ******/
   DB_QueryDELETE ("can not update match players",
		    "DELETE FROM mch_players"
		    " WHERE MchCod=%ld",
		    MchCod);

   /***** Delete match as being played ******/
   DB_QueryDELETE ("can not set match as not being played",
		    "DELETE FROM mch_playing"
		    " WHERE MchCod=%ld",
		    MchCod);
  }

static bool Mch_GetIfMatchIsBeingPlayed (long MchCod)
  {
   /***** Get if a match is being played or not *****/
   return
   (bool) (DB_QueryCOUNT ("can not get if match is being played",
			  "SELECT COUNT(*) FROM mch_playing"
			  " WHERE MchCod=%ld",
			  MchCod) != 0);
  }

static void Mch_RegisterMeAsPlayerInMatch (long MchCod)
  {
   /***** Insert me as match player *****/
   DB_QueryREPLACE ("can not insert match player",
		    "REPLACE mch_players (MchCod,UsrCod) VALUES (%ld,%ld)",
		    MchCod,Gbl.Usrs.Me.UsrDat.UsrCod);
  }

static void Mch_GetNumPlayers (struct Match *Match)
  {
   /***** Get number of players who are playing a match *****/
   Match->Status.NumPlayers =
   (unsigned) DB_QueryCOUNT ("can not get number of players",
			     "SELECT COUNT(*) FROM mch_players"
			     " WHERE MchCod=%ld",
			     Match->MchCod);
  }

/*****************************************************************************/
/********************** Get code of match being played ***********************/
/*****************************************************************************/

void Mch_GetMatchBeingPlayed (void)
  {
   /***** Get match code ****/
   if ((Gbl.Games.MchCodBeingPlayed = Mch_GetParamMchCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of match is missing.");
  }

/*****************************************************************************/
/********* Show game being played to me as student in a new window ***********/
/*****************************************************************************/

void Mch_ShowMatchToMeAsStd (void)
  {
   struct Match Match;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForStd (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/****************** Refresh match for a teacher via AJAX *********************/
/*****************************************************************************/

void Mch_RefreshMatchTch (void)
  {
   struct Match Match;

   if (!Gbl.Session.IsOpen)	// If session has been closed, do not write anything
      return;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Update match status in database *****/
   Mch_UpdateMatchStatusInDB (&Match);

   /***** Update elapsed time in this question *****/
   Mch_UpdateElapsedTimeInQuestion (&Match);

   /***** Show current match status *****/
   // Mch_ShowMatchStatusForTch (&Match);
   Mch_ShowInsideLeftColumnTch (&Match);
  }

/*****************************************************************************/
/*************** Refresh current game for a student via AJAX *****************/
/*****************************************************************************/

void Mch_RefreshMatchStd (void)
  {
   struct Match Match;

   if (!Gbl.Session.IsOpen)	// If session has been closed, do not write anything
      return;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Show current match status *****/
   Mch_ShowMatchStatusForStd (&Match);
  }

/*****************************************************************************/
/**** Receive previous question answer in a match question from database *****/
/*****************************************************************************/

void Mch_GetQstAnsFromDB (long MchCod,long UsrCod,unsigned QstInd,
		          struct Mch_UsrAnswer *UsrAnswer)
  {
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumRows;

   /***** Set default values for number of option and answer index *****/
   UsrAnswer->NumOpt = -1;	// < 0 ==> no answer selected
   UsrAnswer->AnsInd = -1;	// < 0 ==> no answer selected

   /***** Get student's answer *****/
   NumRows = (unsigned) DB_QuerySELECT (&mysql_res,"can not get user's answer to a match question",
					"SELECT NumOpt,"	// row[0]
					       "AnsInd"		// row[1]
					" FROM mch_answers"
					" WHERE MchCod=%ld"
					" AND UsrCod=%ld"
					" AND QstInd=%u",
					MchCod,UsrCod,QstInd);
   if (NumRows) // Answer found...
     {
      row = mysql_fetch_row (mysql_res);

      /***** Get number of option index (row[0]) *****/
      if (sscanf (row[0],"%d",&(UsrAnswer->NumOpt)) != 1)
	 Lay_ShowErrorAndExit ("Error when getting student's answer to a match question.");

      /***** Get answer index (row[1]) *****/
      if (sscanf (row[1],"%d",&(UsrAnswer->AnsInd)) != 1)
	 Lay_ShowErrorAndExit ("Error when getting student's answer to a match question.");
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/********* Receive question answer from student when playing a match *********/
/*****************************************************************************/

void Mch_ReceiveQstAnsFromStd (void)
  {
   struct Match Match;
   unsigned QstInd;
   unsigned Indexes[Tst_MAX_OPTIONS_PER_QUESTION];
   struct Mch_UsrAnswer PreviousUsrAnswer;
   struct Mch_UsrAnswer UsrAnswer;
   unsigned NumQsts;
   unsigned NumQstsNotBlank;
   double TotalScore;

   /***** Remove old players.
          This function must be called before getting match status. *****/
   Mch_RemoveOldPlayers ();

   /***** Get data of the match from database *****/
   Match.MchCod = Gbl.Games.MchCodBeingPlayed;
   Mch_GetDataOfMatchByCod (&Match);

   /***** Get question index from form *****/
   QstInd = Gam_GetParamQstInd ();

   /***** Get indexes for this question from database *****/
   Mch_GetIndexes (Match.MchCod,QstInd,Indexes);

   /***** Check that question index is the current one being played *****/
   if (QstInd == Match.Status.QstInd)	// Receiving an answer
					// to the current question being played
     {
      /***** Get answer index *****/
      /*
      Indexes[4] = {0,3,1,2}
      +-------+--------+----------+---------+
      | Button | Option | Answer   | Correct |
      | letter | number | index    |         |
      | screen | screen | database |         |
      +--------+--------+----------+---------+
      |   a    |    0   |    0     |         |
      |   b    |    1   |    3     |         |
      |   c    |    2   |    1     |    Y    | <---- User press button #2 (index = 1, correct)
      |   d    |    3   |    2     |         |
      +--------+--------+----------+---------+
      UsrAnswer.NumOpt = 2
      UsrAnswer.AnsInd = 1
      */
      UsrAnswer.NumOpt = Mch_GetParamNumOpt ();
      UsrAnswer.AnsInd = Indexes[UsrAnswer.NumOpt];

      /***** Get previous student's answer to this question
	     (<0 ==> no answer) *****/
      Mch_GetQstAnsFromDB (Match.MchCod,Gbl.Usrs.Me.UsrDat.UsrCod,QstInd,
			   &PreviousUsrAnswer);

      /***** Store student's answer *****/
      if (PreviousUsrAnswer.AnsInd == UsrAnswer.AnsInd)
	 DB_QueryDELETE ("can not remove your answer to the match question",
			  "DELETE FROM mch_answers"
			  " WHERE MchCod=%ld AND UsrCod=%ld AND QstInd=%u",
			  Match.MchCod,Gbl.Usrs.Me.UsrDat.UsrCod,QstInd);
      else if (UsrAnswer.NumOpt >= 0 &&
	       UsrAnswer.AnsInd >= 0)
	 DB_QueryREPLACE ("can not register your answer to the match question",
			  "REPLACE mch_answers"
			  " (MchCod,UsrCod,QstInd,NumOpt,AnsInd)"
			  " VALUES"
			  " (%ld,%ld,%u,%d,%d)",
			  Match.MchCod,Gbl.Usrs.Me.UsrDat.UsrCod,QstInd,
			  UsrAnswer.NumOpt,
			  UsrAnswer.AnsInd);

      /***** Update student's match result *****/
      Mch_GetMatchResultQuestionsFromDB (Match.MchCod,Gbl.Usrs.Me.UsrDat.UsrCod,
					 &NumQsts,&NumQstsNotBlank);
      TotalScore = Mch_ComputeScore (NumQsts);

      Str_SetDecimalPointToUS ();	// To print the floating point as a dot
      if (DB_QueryCOUNT ("can not get if match result exists",
			 "SELECT COUNT(*) FROM mch_results"
			 " WHERE MchCod=%ld AND UsrCod=%ld",
			 Match.MchCod,Gbl.Usrs.Me.UsrDat.UsrCod))	// Result exists
	 /* Update result */
	 DB_QueryUPDATE ("can not update match result",
			  "UPDATE mch_results"
			  " SET EndTime=NOW(),"
			       "NumQsts=%u,"
			       "NumQstsNotBlank=%u,"
			       "Score='%lf'"
			  " WHERE MchCod=%ld AND UsrCod=%ld",
			  NumQsts,NumQstsNotBlank,TotalScore,
			  Match.MchCod,Gbl.Usrs.Me.UsrDat.UsrCod);
      else								// Result doesn't exist
	 /* Create result */
	 DB_QueryINSERT ("can not create match result",
			  "INSERT mch_results "
			  "(MchCod,UsrCod,StartTime,EndTime,NumQsts,NumQstsNotBlank,Score)"
			  " VALUES "
			  "(%ld,"	// MchCod
			  "%ld,"	// UsrCod
			  "NOW(),"	// StartTime
			  "NOW(),"	// EndTime
			  "%u,"		// NumQsts
			  "%u,"		// NumQstsNotBlank
			  "'%lf')",	// Score
			  Match.MchCod,Gbl.Usrs.Me.UsrDat.UsrCod,
			  NumQsts,NumQstsNotBlank,TotalScore);
      Str_SetDecimalPointToLocal ();	// Return to local system
     }

   /***** Show current match status *****/
   fprintf (Gbl.F.Out,"<div id=\"match\" class=\"MATCH_CONT\">");
   Mch_ShowMatchStatusForStd (&Match);
   fprintf (Gbl.F.Out,"</div>");
  }

/*****************************************************************************/
/******************** Compute match score for a student **********************/
/*****************************************************************************/

static double Mch_ComputeScore (unsigned NumQsts)
  {
   unsigned NumQst;
   double ScoreThisQst;
   bool AnswerIsNotBlank;
   unsigned Indexes[Tst_MAX_OPTIONS_PER_QUESTION];	// Indexes of all answers of this question
   bool AnswersUsr[Tst_MAX_OPTIONS_PER_QUESTION];
   double TotalScore = 0.0;

   for (NumQst = 0;
	NumQst < NumQsts;
	NumQst++)
     {
      /***** Get indexes for this question from string *****/
      Tst_GetIndexesFromStr (Gbl.Test.StrIndexesOneQst[NumQst],Indexes);

      /***** Get the user's answers for this question from string *****/
      Tst_GetAnswersFromStr (Gbl.Test.StrAnswersOneQst[NumQst],AnswersUsr);

      /***** Get correct answers of test question from database *****/
      Tst_GetCorrectAnswersFromDB (Gbl.Test.QstCodes[NumQst]);

      /***** Compute the total score of this question *****/
      Tst_ComputeScoreQst (Indexes,AnswersUsr,&ScoreThisQst,&AnswerIsNotBlank);

      /***** Compute total score *****/
      TotalScore += ScoreThisQst;
     }

   return TotalScore;
  }

/*****************************************************************************/
/*** Get number of users who selected this answer and draw proportional bar **/
/*****************************************************************************/

void Mch_GetAndDrawBarNumUsrsWhoHaveChosenAns (long MchCod,unsigned QstInd,unsigned AnsInd,
					       unsigned NumAnswerersQst,bool Correct)
  {
   unsigned NumAnswerersAns;

   /***** Get number of users who selected this answer *****/
   NumAnswerersAns = Mch_GetNumUsrsWhoHaveChosenAns (MchCod,QstInd,AnsInd);

   /***** Show stats of this answer *****/
   Mch_DrawBarNumUsrs (NumAnswerersAns,NumAnswerersQst,Correct);
  }

/*****************************************************************************/
/******* Get number of users who have answered a question in a match *********/
/*****************************************************************************/

unsigned Mch_GetNumUsrsWhoHaveAnswerQst (long MchCod,unsigned QstInd)
  {
   /***** Get number of users who have answered
          a question in match from database *****/
   return
   (unsigned) DB_QueryCOUNT ("can not get number of users who have answered a question",
			     "SELECT COUNT(*) FROM mch_answers"
			     " WHERE MchCod=%ld AND QstInd=%u",
			     MchCod,QstInd);
  }

/*****************************************************************************/
/*** Get number of users who have chosen a given answer of a game question ***/
/*****************************************************************************/

static unsigned Mch_GetNumUsrsWhoHaveChosenAns (long MchCod,unsigned QstInd,unsigned AnsInd)
  {
   /***** Get number of users who have chosen
          an answer of a question from database *****/
   return
   (unsigned) DB_QueryCOUNT ("can not get number of users who have chosen an answer",
			     "SELECT COUNT(*) FROM mch_answers"
			     " WHERE MchCod=%ld AND QstInd=%u AND AnsInd=%u",
			     MchCod,QstInd,AnsInd);
  }

/*****************************************************************************/
/****** Get number of users who have answered any question in a match ********/
/*****************************************************************************/

static unsigned Mch_GetNumUsrsWhoHaveAnswerMch (long MchCod)
  {
   /***** Get number of users who have answered
          any question in match from database *****/
   return
   (unsigned) DB_QueryCOUNT ("can not get number of users who have answered a match",
			     "SELECT COUNT(DISTINCT UsrCod) FROM mch_answers"
			     " WHERE MchCod=%ld",
			     MchCod);
  }

/*****************************************************************************/
/***************** Draw a bar with the percentage of answers *****************/
/*****************************************************************************/

// #define Mch_MAX_BAR_WIDTH 400
#define Mch_MAX_BAR_WIDTH 100

static void Mch_DrawBarNumUsrs (unsigned NumAnswerersAns,unsigned NumAnswerersQst,bool Correct)
  {
   extern const char *Txt_of_PART_OF_A_TOTAL;
   unsigned i;
   unsigned BarWidth = 0;

   /***** String with the number of users *****/
   if (NumAnswerersQst)
      snprintf (Gbl.Title,sizeof (Gbl.Title),
	        "%u&nbsp;(%u%%&nbsp;%s&nbsp;%u)",
                NumAnswerersAns,
                (unsigned) ((((float) NumAnswerersAns * 100.0) / (float) NumAnswerersQst) + 0.5),
                Txt_of_PART_OF_A_TOTAL,NumAnswerersQst);
   else
      snprintf (Gbl.Title,sizeof (Gbl.Title),
	        "0&nbsp;(0%%&nbsp;%s&nbsp;%u)",
                Txt_of_PART_OF_A_TOTAL,NumAnswerersQst);

   /***** Start container *****/
   fprintf (Gbl.F.Out,"<div class=\"MATCH_RESULT\">");

   /***** Draw bar with a with proportional to the number of clicks *****/
   if (NumAnswerersAns && NumAnswerersQst)
      BarWidth = (unsigned) ((((float) NumAnswerersAns * (float) Mch_MAX_BAR_WIDTH) /
	                       (float) NumAnswerersQst) + 0.5);

   /***** Bar proportional to number of users *****/
   Tbl_StartTableWide (0);
   fprintf (Gbl.F.Out,"<tr class=\"MATCH_RES_TR\">");
   for (i = 0;
	i < 100;
	i++)
      fprintf (Gbl.F.Out,"<td class=\"%s\"></td>",
	       (i < BarWidth) ? (Correct ? "MATCH_RES_CORRECT" :
					   "MATCH_RES_WRONG") :
				"MATCH_RES_VOID");
   fprintf (Gbl.F.Out,"</tr>");
   Tbl_EndTable ();

   /***** Write the number of users *****/
   fprintf (Gbl.F.Out,"%s",Gbl.Title);

   /***** End container *****/
   fprintf (Gbl.F.Out,"</div>");
  }