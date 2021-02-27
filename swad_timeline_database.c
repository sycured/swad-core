// swad_timeline_database.c: social timeline operations with database

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2021 Antonio Ca�as Vargas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General 3 License as
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
/*********************************** Headers *********************************/
/*****************************************************************************/

#include "swad_database.h"
#include "swad_timeline.h"
#include "swad_timeline_database.h"
#include "swad_timeline_publication.h"

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/************************* Private constants and types ***********************/
/*****************************************************************************/

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

/*****************************************************************************/
/************************* Private global variables **************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Create a new note *****************************/
/*****************************************************************************/
// Returns code of note just created

long TL_DB_CreateNewNote (TL_Not_NoteType_t NoteType,long Cod,
                          long PublisherCod,long HieCod)
  {
   return
   DB_QueryINSERTandReturnCode ("can not create new note",
				"INSERT INTO tl_notes"
				" (NoteType,Cod,UsrCod,HieCod,Unavailable,TimeNote)"
				" VALUES"
				" (%u,%ld,%ld,%ld,'N',NOW())",
				(unsigned) NoteType,	// Post, file, exam, notice, forum
				Cod,			// Post, file, exam, notice, forum code
				PublisherCod,		// Publisher code
				HieCod);		// Where in hierarchy
  }

/*****************************************************************************/
/************************* Mark a note as unavailable ************************/
/*****************************************************************************/

void TL_DB_MarkNoteAsUnavailable (TL_Not_NoteType_t NoteType,long Cod)
  {
   /***** Mark the note as unavailable *****/
   DB_QueryUPDATE ("can not mark note as unavailable",
		   "UPDATE tl_notes SET Unavailable='Y'"
		   " WHERE NoteType=%u AND Cod=%ld",
		   (unsigned) NoteType,Cod);
  }

/*****************************************************************************/
/**** Insert note in temporary tables used to not get notes already shown ****/
/*****************************************************************************/

void TL_DB_InsertNoteInJustRetrievedNotes (long NotCod)
  {
   /* Insert note in temporary table with just retrieved notes.
      This table will be used to not get notes already shown */
   DB_QueryINSERT ("can not store note code",
		   "INSERT IGNORE INTO tl_tmp_just_retrieved_notes"
		   " SET NotCod=%ld",
		   NotCod);
  }

void TL_DB_InsertNoteInVisibleTimeline (long NotCod)
  {
   /* Insert note in temporary table with visible timeline.
      This table will be used to not get notes already shown */
   DB_QueryINSERT ("can not store note code",
		   "INSERT IGNORE INTO tl_tmp_visible_timeline"
		   " SET NotCod=%ld",
		   NotCod);
  }

/*****************************************************************************/
/********************* Get number of comments in a note **********************/
/*****************************************************************************/

unsigned TL_DB_GetNumCommentsInNote (long NotCod)
  {
   return (unsigned)
   DB_QueryCOUNT ("can not get number of comments in a note",
		  "SELECT COUNT(*) FROM tl_pubs"
		  " WHERE NotCod=%ld AND PubType=%u",
		  NotCod,(unsigned) TL_Pub_COMMENT_TO_NOTE);
  }

/*****************************************************************************/
/************** Get initial comments of a note from database *****************/
/*****************************************************************************/
// Returns the number of rows got

unsigned TL_DB_GetInitialComments (long NotCod,
				   unsigned NumInitialCommentsToGet,
				   MYSQL_RES **mysql_res)
  {
   return (unsigned)
   DB_QuerySELECT (mysql_res,"can not get comments",
		   "SELECT tl_pubs.PubCod,"			// row[0]
			  "tl_pubs.PublisherCod,"		// row[1]
			  "tl_pubs.NotCod,"			// row[2]
			  "UNIX_TIMESTAMP(tl_pubs.TimePublish),"// row[3]
			  "tl_comments.Txt,"			// row[4]
			  "tl_comments.MedCod"			// row[5]
		   " FROM tl_pubs,tl_comments"
		   " WHERE tl_pubs.NotCod=%ld"
		   " AND tl_pubs.PubType=%u"
		   " AND tl_pubs.PubCod=tl_comments.PubCod"
		   " ORDER BY tl_pubs.PubCod"
		   " LIMIT %lu",
		   NotCod,(unsigned) TL_Pub_COMMENT_TO_NOTE,
		   NumInitialCommentsToGet);
  }

/*****************************************************************************/
/*************** Get final comments of a note from database ******************/
/*****************************************************************************/
// Returns the number of rows got

unsigned TL_DB_GetFinalComments (long NotCod,
				 unsigned NumFinalCommentsToGet,
				 MYSQL_RES **mysql_res)
  {
   /***** Get final comments of a note from database *****/
   return (unsigned)
   DB_QuerySELECT (mysql_res,"can not get comments",
		   "SELECT * FROM "
		   "("
		   "SELECT tl_pubs.PubCod,"			// row[0]
			  "tl_pubs.PublisherCod,"		// row[1]
			  "tl_pubs.NotCod,"			// row[2]
			  "UNIX_TIMESTAMP(tl_pubs.TimePublish),"// row[3]
			  "tl_comments.Txt,"			// row[4]
			  "tl_comments.MedCod"			// row[5]
	          " FROM tl_pubs,tl_comments"
		  " WHERE tl_pubs.NotCod=%ld"
		  " AND tl_pubs.PubType=%u"
		  " AND tl_pubs.PubCod=tl_comments.PubCod"
		  " ORDER BY tl_pubs.PubCod DESC LIMIT %u"
		  ") AS comments"
		  " ORDER BY PubCod",
		  NotCod,(unsigned) TL_Pub_COMMENT_TO_NOTE,
		  NumFinalCommentsToGet);
  }

/*****************************************************************************/
/******************* Get data of comment using its code **********************/
/*****************************************************************************/
// Returns the number of rows got

unsigned TL_DB_GetDataOfCommByCod (long PubCod,MYSQL_RES **mysql_res)
  {
   /***** Trivial check: publication code should be > 0 *****/
   if (PubCod <= 0)
      return 0;

   /***** Get data of comment from database *****/
   return (unsigned)
   DB_QuerySELECT (mysql_res,"can not get data of comment",
		   "SELECT tl_pubs.PubCod,"			// row[0]
			  "tl_pubs.PublisherCod,"		// row[1]
			  "tl_pubs.NotCod,"			// row[2]
			  "UNIX_TIMESTAMP(tl_pubs.TimePublish),"// row[3]
			  "tl_comments.Txt,"			// row[4]
			  "tl_comments.MedCod"			// row[5]
		   " FROM tl_pubs,tl_comments"
		   " WHERE tl_pubs.PubCod=%ld"
		   " AND tl_pubs.PubType=%u"
		   " AND tl_pubs.PubCod=tl_comments.PubCod",
		   PubCod,(unsigned) TL_Pub_COMMENT_TO_NOTE);
  }

/*****************************************************************************/
/******************* Insert comment content in database **********************/
/*****************************************************************************/

void TL_DB_InsertCommentContent (long PubCod,
				 const struct TL_Pst_PostContent *Content)
  {
   /***** Insert comment content in database *****/
   DB_QueryINSERT ("can not store comment content",
		   "INSERT INTO tl_comments"
		   " (PubCod,Txt,MedCod)"
		   " VALUES"
		   " (%ld,'%s',%ld)",
		   PubCod,
		   Content->Txt,
		   Content->Media.MedCod);
  }

/*****************************************************************************/
/**************** Get code of media associated to comment ********************/
/*****************************************************************************/

long TL_DB_GetMedCodFromComment (long PubCod)
  {
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   long MedCod = -1L;	// Default value

   /***** Get code of media associated to comment *****/
   if (DB_QuerySELECT (&mysql_res,"can not get media code",
		       "SELECT MedCod"	// row[0]
		       " FROM tl_comments"
		       " WHERE PubCod=%ld",
		       PubCod) == 1)   // Result should have a unique row
     {
      /* Get media code */
      row = mysql_fetch_row (mysql_res);
      MedCod = Str_ConvertStrCodToLongCod (row[0]);
     }

   /* Free structure that stores the query result */
   DB_FreeMySQLResult (&mysql_res);

   return MedCod;
  }

/*****************************************************************************/
/****************** Remove favs for comment from database ********************/
/*****************************************************************************/

void TL_DB_RemoveCommentFavs (long PubCod)
  {
   /***** Remove favs for comment *****/
   DB_QueryDELETE ("can not remove favs for comment",
		   "DELETE FROM tl_comments_fav"
		   " WHERE PubCod=%ld",
		   PubCod);
  }

/*****************************************************************************/
/***************** Remove content of comment from database *******************/
/*****************************************************************************/

void TL_DB_RemoveCommentContent (long PubCod)
  {
   /***** Remove content of comment *****/
   DB_QueryDELETE ("can not remove comment content",
		   "DELETE FROM tl_comments"
		   " WHERE PubCod=%ld",
		   PubCod);
  }

/*****************************************************************************/
/***************** Remove comment publication from database ******************/
/*****************************************************************************/

void TL_DB_RemoveCommentPub (long PubCod,long PublisherCod)
  {
   /***** Remove comment publication *****/
   DB_QueryDELETE ("can not remove comment",
		   "DELETE FROM tl_pubs"
	           " WHERE PubCod=%ld"
	           " AND PublisherCod=%ld"	// Extra check: author
	           " AND PubType=%u",		// Extra check: it's a comment
		   PubCod,
		   PublisherCod,
		   (unsigned) TL_Pub_COMMENT_TO_NOTE);
  }