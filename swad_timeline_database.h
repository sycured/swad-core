// swad_timeline_database.h: social timeline operations with database

#ifndef _SWAD_TL_DB
#define _SWAD_TL_DB
/*
    SWAD (Shared Workspace At a Distance in Spanish),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2021 Antonio Ca�as Vargas

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
/********************************** Headers **********************************/
/*****************************************************************************/

#include "swad_database.h"
#include "swad_timeline_note.h"
#include "swad_timeline_post.h"

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/******************************** Public types *******************************/
/*****************************************************************************/

/*****************************************************************************/
/****************************** Public prototypes ****************************/
/*****************************************************************************/

/* Notes */
long TL_DB_CreateNewNote (TL_Not_NoteType_t NoteType,long Cod,
                          long PublisherCod,long HieCod);
void TL_DB_MarkNoteAsUnavailable (TL_Not_NoteType_t NoteType,long Cod);
void TL_DB_InsertNoteInJustRetrievedNotes (long NotCod);
void TL_DB_InsertNoteInVisibleTimeline (long NotCod);

/* Comments */
unsigned TL_DB_GetNumCommentsInNote (long NotCod);
unsigned TL_DB_GetInitialComments (long NotCod,
				   unsigned NumInitialCommentsToGet,
				   MYSQL_RES **mysql_res);
unsigned TL_DB_GetFinalComments (long NotCod,
				 unsigned NumFinalCommentsToGet,
				 MYSQL_RES **mysql_res);
unsigned TL_DB_GetDataOfCommByCod (long PubCod,MYSQL_RES **mysql_res);
void TL_DB_InsertCommentContent (long PubCod,
				 const struct TL_Pst_PostContent *Content);
long TL_DB_GetMedCodFromComment (long PubCod);
void TL_DB_RemoveCommentFavs (long PubCod);
void TL_DB_RemoveCommentContent (long PubCod);
void TL_DB_RemoveCommentPub (long PubCod,long PublisherCod);

#endif