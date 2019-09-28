// swad_cookies.c: user's preferences about cookies

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

#include "swad_box.h"
#include "swad_cookie.h"
#include "swad_database.h"
#include "swad_form.h"
#include "swad_global.h"
#include "swad_layout.h"
#include "swad_setting.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

static void Coo_PutIconsCookies (void);

/*****************************************************************************/
/********************* Edit my preferences on cookies ************************/
/*****************************************************************************/

void Coo_EditMyPrefsOnCookies (void)
  {
   extern const char *Hlp_PROFILE_Settings_cookies;
   extern const char *Txt_Cookies;
   extern const char *Txt_Accept_third_party_cookies_to_view_multimedia_content_from_other_websites;

   /***** Start section with preferences about cookies *****/
   Lay_StartSection (Coo_COOKIES_ID);

   /***** Start box and table *****/
   Box_StartBoxTable (NULL,Txt_Cookies,Coo_PutIconsCookies,
                      Hlp_PROFILE_Settings_cookies,Box_NOT_CLOSABLE,2);

   /***** Edit my preference about cookies *****/
   /* Start form */
   Frm_StartFormAnchor (ActChgCooPrf,Coo_COOKIES_ID);

   /* Start container */
   fprintf (Gbl.F.Out,"<div class=\"%s\">",
	    (Gbl.Usrs.Me.UsrDat.Prefs.AcceptThirdPartyCookies) ? "DAT_N LIGHT_BLUE" :
								 "DAT");
   /* Check box */
   fprintf (Gbl.F.Out,"<label>"
	              "<input type=\"checkbox\""
		      " name=\"cookies\" value=\"Y\"");
   if (Gbl.Usrs.Me.UsrDat.Prefs.AcceptThirdPartyCookies)
      fprintf (Gbl.F.Out," checked=\"checked\"");
   fprintf (Gbl.F.Out," onclick=\"document.getElementById('%s').submit();\" />"
	              "%s"
		      "</label>",
	    Gbl.Form.Id,
	    Txt_Accept_third_party_cookies_to_view_multimedia_content_from_other_websites);

   /* End container */
   fprintf (Gbl.F.Out,"</div>");

   /***** End table and box *****/
   Box_EndBoxTable ();

   /***** End section with preferences about cookies *****/
   Lay_EndSection ();
  }

/*****************************************************************************/
/***************** Put contextual icons in cookies preference ****************/
/*****************************************************************************/

static void Coo_PutIconsCookies (void)
  {
   /***** Put icon to show a figure *****/
   Gbl.Figures.FigureType = Fig_COOKIES;
   Fig_PutIconToShowFigure ();
  }

/*****************************************************************************/
/************** Change my preference about third party cookies ***************/
/*****************************************************************************/

void Coo_ChangeMyPrefsCookies (void)
  {
   /***** Get param with preference about third party cookies *****/
   Gbl.Usrs.Me.UsrDat.Prefs.AcceptThirdPartyCookies = Par_GetParToBool ("cookies");

   /***** Store preference in database *****/
   if (Gbl.Usrs.Me.Logged)
      DB_QueryUPDATE ("can not update your preference about cookies",
		      "UPDATE usr_data SET ThirdPartyCookies='%c'"
		      " WHERE UsrCod=%ld",
                      Gbl.Usrs.Me.UsrDat.Prefs.AcceptThirdPartyCookies ? 'Y' :
                	                                                 'N',
		      Gbl.Usrs.Me.UsrDat.UsrCod);

   /***** Show forms again *****/
   Set_EditSettings ();
  }