// swad_timeline_favourite.h: social timeline favourites

#ifndef _SWAD_TML_FAV
#define _SWAD_TML_FAV
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

/*****************************************************************************/
/************************ Public constants and types *************************/
/*****************************************************************************/

#define Tml_Fav_ICON_FAV		"heart.svg"
#define Tml_Fav_ICON_FAVED	"heart-red.svg"

/*****************************************************************************/
/***************************** Public prototypes *****************************/
/*****************************************************************************/

void Tml_Fav_ShowAllFaversNoteUsr (void);
void Tml_Fav_ShowAllFaversNoteGbl (void);
void Tml_Fav_FavNoteUsr (void);
void Tml_Fav_FavNoteGbl (void);
void Tml_Fav_UnfNoteUsr (void);
void Tml_Fav_UnfNoteGbl (void);

void Tml_Fav_ShowAllFaversComUsr (void);
void Tml_Fav_ShowAllFaversComGbl (void);
void Tml_Fav_FavCommUsr (void);
void Tml_Fav_FavCommGbl (void);
void Tml_Fav_UnfCommUsr (void);
void Tml_Fav_UnfCommGbl (void);

#endif
