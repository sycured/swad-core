// swad_degree.c: degrees

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2016 Antonio Ca�as Vargas

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

#include <ctype.h>		// For isprint, isspace, etc.
#include <linux/stddef.h>	// For NULL
#include <stdbool.h>		// For boolean type
#include <stdio.h>		// For fprintf, etc.
#include <stdlib.h>		// For exit, system, calloc, free, etc.
#include <string.h>		// For string functions
#include <mysql/mysql.h>	// To access MySQL databases

#include "swad_changelog.h"
#include "swad_config.h"
#include "swad_database.h"
#include "swad_degree.h"
#include "swad_degree_type.h"
#include "swad_exam.h"
#include "swad_global.h"
#include "swad_help.h"
#include "swad_indicator.h"
#include "swad_info.h"
#include "swad_logo.h"
#include "swad_notification.h"
#include "swad_parameter.h"
#include "swad_QR.h"
#include "swad_RSS.h"
#include "swad_string.h"
#include "swad_tab.h"
#include "swad_text.h"
#include "swad_theme.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/*************************** Public constants ********************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private types *********************************/
/*****************************************************************************/

typedef enum
  {
   Deg_FIRST_YEAR,
   Deg_LAST_YEAR,
  } Deg_FirstOrLastYear_t;

/*****************************************************************************/
/**************************** Private constants ******************************/
/*****************************************************************************/

#define Deg_MAX_LENGTH_SHORT_NAME_DEGREE_ON_PAGE_HEAD	20	// Adjust depending on the size of the style used for the degree on the page head
#define Deg_MAX_LENGTH_SHORT_NAME_COURSE_ON_PAGE_HEAD	20	// Adjust depending on the size of the style used for the degree on the page head

/*****************************************************************************/
/**************************** Private prototypes *****************************/
/*****************************************************************************/

static void Deg_Configuration (bool PrintView);
static void Deg_PutIconsToPrintAndUpload (void);

static void Deg_WriteSelectorOfDegree (void);

static void Deg_ListDegreesForEdition (void);
static bool Deg_CheckIfICanEdit (struct Degree *Deg);
static Deg_StatusTxt_t Deg_GetStatusTxtFromStatusBits (Deg_Status_t Status);
static Deg_Status_t Deg_GetStatusBitsFromStatusTxt (Deg_StatusTxt_t StatusTxt);
static void Deg_PutFormToCreateDegree (void);
static void Deg_PutHeadDegreesForSeeing (void);
static void Deg_PutHeadDegreesForEdition (void);
static void Deg_CreateDegree (struct Degree *Deg,unsigned Status);

static void Deg_ListDegrees (void);
static void Deg_PutIconToEditDegrees (void);
static void Deg_ListOneDegreeForSeeing (struct Degree *Deg,unsigned NumDeg);

static void Deg_GetListDegsOfCurrentCtr (void);
static void Deg_FreeListDegsOfCurrentCtr (void);
static void Deg_RecFormRequestOrCreateDeg (unsigned Status);
static void Deg_PutParamOtherDegCod (long DegCod);

static void Deg_GetDataOfDegreeFromRow (struct Degree *Deg,MYSQL_ROW row);
static bool Deg_RenameDegree (struct Degree *Deg,Cns_ShortOrFullName_t ShortOrFullName);
static bool Deg_CheckIfDegreeNameExists (long CtrCod,const char *FieldName,const char *Name,long DegCod);

/*****************************************************************************/
/********** List pending institutions, centres, degrees and courses **********/
/*****************************************************************************/

void Deg_SeePending (void)
  {
   /***** List countries with pending institutions *****/
   Cty_SeeCtyWithPendingInss ();

   /***** List institutions with pending centres *****/
   Ins_SeeInsWithPendingCtrs ();

   /***** List centres with pending degrees *****/
   Ctr_SeeCtrWithPendingDegs ();

   /***** List degrees with pending courses *****/
   Deg_SeeDegWithPendingCrss ();
  }

/*****************************************************************************/
/******************* List degrees with pending courses ***********************/
/*****************************************************************************/

void Deg_SeeDegWithPendingCrss (void)
  {
   extern const char *Txt_Degrees_with_pending_courses;
   extern const char *Txt_Degree;
   extern const char *Txt_Courses_ABBREVIATION;
   extern const char *Txt_There_are_no_degrees_with_requests_for_courses_to_be_confirmed;
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumDegs;
   unsigned NumDeg;
   struct Degree Deg;
   const char *BgColor;

   /***** Get degrees with pending courses *****/
   switch (Gbl.Usrs.Me.LoggedRole)
     {
      case Rol_DEG_ADM:
         sprintf (Query,"SELECT courses.DegCod,COUNT(*)"
                        " FROM admin,courses,degrees"
                        " WHERE admin.UsrCod='%ld' AND admin.Scope='Deg'"
                        " AND admin.Cod=courses.DegCod"
                        " AND (courses.Status & %u)<>0"
                        " AND courses.DegCod=degrees.DegCod"
                        " GROUP BY courses.DegCod ORDER BY degrees.ShortName",
                  Gbl.Usrs.Me.UsrDat.UsrCod,(unsigned) Crs_STATUS_BIT_PENDING);
         break;
      case Rol_SYS_ADM:
         sprintf (Query,"SELECT courses.DegCod,COUNT(*)"
                        " FROM courses,degrees"
                        " WHERE (courses.Status & %u)<>0"
                        " AND courses.DegCod=degrees.DegCod"
                        " GROUP BY courses.DegCod ORDER BY degrees.ShortName",
                  (unsigned) Crs_STATUS_BIT_PENDING);
         break;
      default:	// Forbidden for other users
	 return;
     }

   /***** Get degrees *****/
   if ((NumDegs = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get degrees with pending courses")))
     {
      /***** Write heading *****/
      Lay_StartRoundFrameTable (NULL,2,Txt_Degrees_with_pending_courses);
      fprintf (Gbl.F.Out,"<tr>"
                         "<th class=\"LEFT_MIDDLE\">"
                         "%s"
                         "</th>"
                         "<th class=\"RIGHT_MIDDLE\">"
                         "%s"
                         "</th>"
                         "</tr>",
               Txt_Degree,
               Txt_Courses_ABBREVIATION);

      /***** List the degrees *****/
      for (NumDeg = 0;
	   NumDeg < NumDegs;
	   NumDeg++)
        {
         /* Get next degree */
         row = mysql_fetch_row (mysql_res);

         /* Get degree code (row[0]) */
         Deg.DegCod = Str_ConvertStrCodToLongCod (row[0]);
         BgColor = (Deg.DegCod == Gbl.CurrentDeg.Deg.DegCod) ? "LIGHT_BLUE" :
                                                               Gbl.ColorRows[Gbl.RowEvenOdd];

         /* Get data of degree */
         Deg_GetDataOfDegreeByCod (&Deg);

         /* Degree logo and full name */
         fprintf (Gbl.F.Out,"<tr>"
	                    "<td class=\"LEFT_MIDDLE %s\">",
                  BgColor);
         Deg_DrawDegreeLogoAndNameWithLink (&Deg,ActSeeCrs,
                                            "DAT_NOBR","CENTER_MIDDLE");
         fprintf (Gbl.F.Out,"</td>");

         /* Number of pending courses (row[1]) */
         fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE %s\">"
	                    "%s"
	                    "</td>"
	                    "</tr>",
                  BgColor,row[1]);

         Gbl.RowEvenOdd = 1 - Gbl.RowEvenOdd;
        }

      Lay_EndRoundFrameTable ();
     }
   else
      Lay_ShowAlert (Lay_INFO,Txt_There_are_no_degrees_with_requests_for_courses_to_be_confirmed);

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/******************** Draw degree logo and name with link ********************/
/*****************************************************************************/

void Deg_DrawDegreeLogoAndNameWithLink (struct Degree *Deg,Act_Action_t Action,
                                        const char *ClassLink,const char *ClassLogo)
  {
   extern const char *Txt_Go_to_X;

   /***** Start form *****/
   Act_FormGoToStart (Action);
   Deg_PutParamDegCod (Deg->DegCod);

   /***** Link to action *****/
   sprintf (Gbl.Title,Txt_Go_to_X,Deg->FullName);
   Act_LinkFormSubmit (Gbl.Title,ClassLink,NULL);

   /***** Draw degree logo *****/
   Log_DrawLogo (Sco_SCOPE_DEG,Deg->DegCod,Deg->ShortName,20,ClassLogo,true);

   /***** End link *****/
   fprintf (Gbl.F.Out,"&nbsp;%s</a>",Deg->FullName);

   /***** End form *****/
   Act_FormEnd ();
  }

/*****************************************************************************/
/****************** Show information of the current degree *******************/
/*****************************************************************************/

void Deg_ShowConfiguration (void)
  {
   Deg_Configuration (false);

   /***** Show help to enroll me *****/
   Hlp_ShowHelpWhatWouldYouLikeToDo ();
  }

/*****************************************************************************/
/****************** Print information of the current degree ******************/
/*****************************************************************************/

void Deg_PrintConfiguration (void)
  {
   Deg_Configuration (true);
  }

/*****************************************************************************/
/******************* Information of the current degree ***********************/
/*****************************************************************************/

static void Deg_Configuration (bool PrintView)
  {
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_Centre;
   extern const char *Txt_Degree;
   extern const char *Txt_Short_name;
   extern const char *Txt_Web;
   extern const char *Txt_Shortcut;
   extern const char *Txt_STR_LANG_ID[1+Txt_NUM_LANGUAGES];
   extern const char *Txt_Courses;
   extern const char *Txt_Courses_of_DEGREE_X;
   extern const char *Txt_QR_code;
   extern const char *Txt_ROLES_PLURAL_Abc[Rol_NUM_ROLES][Usr_NUM_SEXS];
   unsigned NumCtr;
   bool PutLink = !PrintView && Gbl.CurrentDeg.Deg.WWW[0];

   if (Gbl.CurrentDeg.Deg.DegCod > 0)
     {
      /***** Start frame *****/
      Lay_StartRoundFrame (NULL,NULL,PrintView ? NULL :
	                                         Deg_PutIconsToPrintAndUpload);

      /***** Title *****/
      fprintf (Gbl.F.Out,"<div class=\"TITLE_LOCATION\">");
      if (PutLink)
	 fprintf (Gbl.F.Out,"<a href=\"%s\" target=\"_blank\""
	                    " class=\"TITLE_LOCATION\" title=\"%s\">",
		  Gbl.CurrentDeg.Deg.WWW,
		  Gbl.CurrentDeg.Deg.FullName);
      Log_DrawLogo (Sco_SCOPE_DEG,Gbl.CurrentDeg.Deg.DegCod,
                    Gbl.CurrentDeg.Deg.ShortName,64,NULL,true);
      fprintf (Gbl.F.Out,"<br />%s",
               Gbl.CurrentDeg.Deg.FullName);
      if (PutLink)
	 fprintf (Gbl.F.Out,"</a>");
      fprintf (Gbl.F.Out,"</div>");

      /***** Start table *****/
      fprintf (Gbl.F.Out,"<table class=\"FRAME_TABLE CELLS_PAD_2\">");

      /***** Centre *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"%s RIGHT_MIDDLE\">"
			 "%s:"
			 "</td>"
			 "<td class=\"DAT LEFT_MIDDLE\">",
	       The_ClassForm[Gbl.Prefs.Theme],
	       Txt_Centre);

      if (!PrintView &&
	  Gbl.Usrs.Me.LoggedRole >= Rol_INS_ADM)	// Only institution admins and system admin can move a degree to another centre
	{
	 /* Get list of centres of the current institution */
	 Ctr_GetListCentres (Gbl.CurrentIns.Ins.InsCod);

	 /* Put form to select centre */
	 Act_FormStart (ActChgDegCtrCfg);
	 fprintf (Gbl.F.Out,"<select name=\"OthCtrCod\""
			    " class=\"INPUT_CENTRE\""
			    " onchange=\"document.getElementById('%s').submit();\">",
		  Gbl.Form.Id);
	 for (NumCtr = 0;
	      NumCtr < Gbl.Ctrs.Num;
	      NumCtr++)
	    fprintf (Gbl.F.Out,"<option value=\"%ld\"%s>%s</option>",
		     Gbl.Ctrs.Lst[NumCtr].CtrCod,
		     Gbl.Ctrs.Lst[NumCtr].CtrCod == Gbl.CurrentCtr.Ctr.CtrCod ? " selected=\"selected\"" :
										"",
		     Gbl.Ctrs.Lst[NumCtr].ShortName);
	 fprintf (Gbl.F.Out,"</select>");
	 Act_FormEnd ();

	 /* Free list of centres */
	 Ctr_FreeListCentres ();
	}
      else	// I can not edit centre
	 fprintf (Gbl.F.Out,"%s",Gbl.CurrentCtr.Ctr.FullName);

      fprintf (Gbl.F.Out,"</td>"
			 "</tr>");

      /***** Degree full name *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"%s RIGHT_MIDDLE\">"
	                 "%s:"
	                 "</td>"
			 "<td class=\"DAT_N LEFT_MIDDLE\">",
	       The_ClassForm[Gbl.Prefs.Theme],
	       Txt_Degree);
      if (PutLink)
	 fprintf (Gbl.F.Out,"<a href=\"%s\" target=\"_blank\""
	                    " class=\"DAT_N\" title=\"%s\">",
		  Gbl.CurrentDeg.Deg.WWW,
		  Gbl.CurrentDeg.Deg.FullName);
      fprintf (Gbl.F.Out,"%s",
	       Gbl.CurrentDeg.Deg.FullName);
      if (PutLink)
	 fprintf (Gbl.F.Out,"</a>");
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");

      /***** Degree short name *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"%s RIGHT_MIDDLE\">"
			 "%s:"
			 "</td>"
			 "<td class=\"DAT LEFT_MIDDLE\">"
			 "%s"
			 "</td>"
			 "</tr>",
	       The_ClassForm[Gbl.Prefs.Theme],
	       Txt_Short_name,
	       Gbl.CurrentDeg.Deg.ShortName);

      /***** Degree WWW *****/
      if (Gbl.CurrentDeg.Deg.WWW[0])
	{
	 fprintf (Gbl.F.Out,"<tr>"
			    "<td class=\"%s RIGHT_MIDDLE\">"
			    "%s:"
			    "</td>"
			    "<td class=\"DAT LEFT_MIDDLE\">"
			    "<a href=\"%s\" target=\"_blank\" class=\"DAT\">",
		  The_ClassForm[Gbl.Prefs.Theme],
		  Txt_Web,
		  Gbl.CurrentDeg.Deg.WWW);
	 Str_LimitLengthHTMLStr (Gbl.CurrentDeg.Deg.WWW,20);
	 fprintf (Gbl.F.Out,"%s"
			    "</a>"
			    "</td>"
			    "</tr>",
		  Gbl.CurrentDeg.Deg.WWW);
	}

      /***** Shortcut to the degree *****/
      fprintf (Gbl.F.Out,"<tr>"
			 "<td class=\"%s RIGHT_MIDDLE\">"
	                 "%s:"
	                 "</td>"
			 "<td class=\"DAT LEFT_MIDDLE\">"
			 "<a href=\"%s/%s?deg=%ld\" class=\"DAT\" target=\"_blank\">"
			 "%s/%s?deg=%ld"
			 "</a>"
			 "</td>"
			 "</tr>",
	       The_ClassForm[Gbl.Prefs.Theme],
	       Txt_Shortcut,
	       Cfg_URL_SWAD_CGI,
	       Txt_STR_LANG_ID[Gbl.Prefs.Language],
	       Gbl.CurrentDeg.Deg.DegCod,
	       Cfg_URL_SWAD_CGI,
	       Txt_STR_LANG_ID[Gbl.Prefs.Language],
	       Gbl.CurrentDeg.Deg.DegCod);

      if (PrintView)
	{
	 /***** QR code with link to the degree *****/
	 fprintf (Gbl.F.Out,"<tr>"
			    "<td class=\"%s RIGHT_MIDDLE\">"
	                    "%s:"
	                    "</td>"
			    "<td class=\"DAT LEFT_MIDDLE\">",
		  The_ClassForm[Gbl.Prefs.Theme],
		  Txt_QR_code);
	 QR_LinkTo (250,"deg",Gbl.CurrentDeg.Deg.DegCod);
	 fprintf (Gbl.F.Out,"</td>"
			    "</tr>");
	}
      else
	{
	 /***** Number of courses *****/
	 fprintf (Gbl.F.Out,"<tr>"
			    "<td class=\"%s RIGHT_MIDDLE\">"
	                    "%s:"
	                    "</td>"
			    "<td class=\"LEFT_MIDDLE\">",
		  The_ClassForm[Gbl.Prefs.Theme],
		  Txt_Courses);

	 /* Form to go to see courses of this degree */
	 Act_FormGoToStart (ActSeeCrs);
	 Deg_PutParamDegCod (Gbl.CurrentDeg.Deg.DegCod);
	 sprintf (Gbl.Title,Txt_Courses_of_DEGREE_X,
	          Gbl.CurrentDeg.Deg.ShortName);
	 Act_LinkFormSubmit (Gbl.Title,"DAT",NULL);
	 fprintf (Gbl.F.Out,"%u</a>",
		  Crs_GetNumCrssInDeg (Gbl.CurrentDeg.Deg.DegCod));
	 Act_FormEnd ();

	 fprintf (Gbl.F.Out,"</td>"
			    "</tr>");

	 /***** Number of teachers *****/
	 fprintf (Gbl.F.Out,"<tr>"
			    "<td class=\"%s RIGHT_MIDDLE\">"
	                    "%s:"
	                    "</td>"
			    "<td class=\"DAT LEFT_MIDDLE\">"
	                    "%u"
	                    "</td>"
			    "</tr>",
		  The_ClassForm[Gbl.Prefs.Theme],
		  Txt_ROLES_PLURAL_Abc[Rol_TEACHER][Usr_SEX_UNKNOWN],
		  Usr_GetNumUsrsInCrssOfDeg (Rol_TEACHER,Gbl.CurrentDeg.Deg.DegCod));

	 /***** Number of students *****/
	 fprintf (Gbl.F.Out,"<tr>"
			    "<td class=\"%s RIGHT_MIDDLE\">"
	                    "%s:"
	                    "</td>"
			    "<td class=\"DAT LEFT_MIDDLE\">"
	                    "%u"
	                    "</td>"
			    "</tr>",
		  The_ClassForm[Gbl.Prefs.Theme],
		  Txt_ROLES_PLURAL_Abc[Rol_STUDENT][Usr_SEX_UNKNOWN],
		  Usr_GetNumUsrsInCrssOfDeg (Rol_STUDENT,Gbl.CurrentDeg.Deg.DegCod));
	}

      /***** End table *****/
      fprintf (Gbl.F.Out,"</table>");

      /***** End frame *****/
      Lay_EndRoundFrame ();
     }
  }

/*****************************************************************************/
/************ Put contextual icons in configuration of a degree **************/
/*****************************************************************************/

static void Deg_PutIconsToPrintAndUpload (void)
  {
   extern const char *Txt_Print;

   /***** Link to print info about degree *****/
   Lay_PutContextualLink (ActPrnDegInf,NULL,
                          "print64x64.png",
                          Txt_Print,NULL,
                          NULL);

   if (Gbl.Usrs.Me.LoggedRole >= Rol_DEG_ADM)
      /***** Link to upload logo of degree *****/
      Log_PutIconToChangeLogo (Sco_SCOPE_DEG);
  }

/*****************************************************************************/
/*** Write menu to select country, institution, centre, degree and course ****/
/*****************************************************************************/

void Deg_WriteMenuAllCourses (void)
  {
   extern const char *The_ClassForm[The_NUM_THEMES];
   extern const char *Txt_Country;
   extern const char *Txt_Institution;
   extern const char *Txt_Centre;
   extern const char *Txt_Degree;
   extern const char *Txt_Course;

   /***** Start of table *****/
   fprintf (Gbl.F.Out,"<table class=\"CELLS_PAD_2\""
	              " style=\"margin:0 auto 12px auto;\">");

   /***** Write a 1st selector
          with all the countries *****/
   fprintf (Gbl.F.Out,"<tr>"
                      "<td class=\"%s RIGHT_MIDDLE\">"
                      "%s:"
                      "</td>"
                      "<td class=\"LEFT_MIDDLE\">",
            The_ClassForm[Gbl.Prefs.Theme],Txt_Country);
   Cty_WriteSelectorOfCountry ();
   fprintf (Gbl.F.Out,"</td>"
	              "</tr>");

   if (Gbl.CurrentCty.Cty.CtyCod > 0)
     {
      /***** Write a 2nd selector
             with the institutions of selected country *****/
      fprintf (Gbl.F.Out,"<tr>"
                         "<td class=\"%s RIGHT_MIDDLE\">"
                         "%s:"
                         "</td>"
                         "<td class=\"LEFT_MIDDLE\">",
               The_ClassForm[Gbl.Prefs.Theme],Txt_Institution);
      Ins_WriteSelectorOfInstitution ();
      fprintf (Gbl.F.Out,"</td>"
	                 "</tr>");

      if (Gbl.CurrentIns.Ins.InsCod > 0)
        {
         /***** Write a 3rd selector
                with all the centres of selected institution *****/
         fprintf (Gbl.F.Out,"<tr>"
                            "<td class=\"%s RIGHT_MIDDLE\">"
                            "%s:"
                            "</td>"
                            "<td class=\"LEFT_MIDDLE\">",
                  The_ClassForm[Gbl.Prefs.Theme],Txt_Centre);
         Ctr_WriteSelectorOfCentre ();
         fprintf (Gbl.F.Out,"</td>"
                            "</tr>");

         if (Gbl.CurrentCtr.Ctr.CtrCod > 0)
           {
            /***** Write a 4th selector
                   with all the degrees of selected centre *****/
            fprintf (Gbl.F.Out,"<tr>"
                               "<td class=\"%s RIGHT_MIDDLE\">"
                               "%s:"
                               "</td>"
                               "<td class=\"LEFT_MIDDLE\">",
                     The_ClassForm[Gbl.Prefs.Theme],Txt_Degree);
            Deg_WriteSelectorOfDegree ();
            fprintf (Gbl.F.Out,"</td>"
        	               "</tr>");

	    if (Gbl.CurrentDeg.Deg.DegCod > 0)
	      {
	       /***** Write a 5th selector
		      with all the courses of selected degree *****/
	       fprintf (Gbl.F.Out,"<tr>"
				  "<td class=\"%s RIGHT_MIDDLE\">"
				  "%s:"
				  "</td>"
				  "<td class=\"LEFT_MIDDLE\">",
			The_ClassForm[Gbl.Prefs.Theme],Txt_Course);
	       Crs_WriteSelectorOfCourse ();
	       fprintf (Gbl.F.Out,"</td>"
				  "</tr>");
	      }
           }
        }
     }

   /***** End of table *****/
   fprintf (Gbl.F.Out,"</table>");
  }

/*****************************************************************************/
/*************************** Write selector of degree ************************/
/*****************************************************************************/

static void Deg_WriteSelectorOfDegree (void)
  {
   extern const char *Txt_Degree;
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumDegs;
   unsigned NumDeg;
   long DegCod;

   /***** Start form *****/
   Act_FormGoToStart (ActSeeCrs);
   fprintf (Gbl.F.Out,"<select name=\"deg\" style=\"width:175px;\"");
   if (Gbl.CurrentCtr.Ctr.CtrCod > 0)
      fprintf (Gbl.F.Out," onchange=\"document.getElementById('%s').submit();\"",
               Gbl.Form.Id);
   else
      fprintf (Gbl.F.Out," disabled=\"disabled\"");
   fprintf (Gbl.F.Out,"><option value=\"\"");
   if (Gbl.CurrentDeg.Deg.DegCod < 0)
      fprintf (Gbl.F.Out," selected=\"selected\"");
   fprintf (Gbl.F.Out," disabled=\"disabled\">[%s]</option>",
            Txt_Degree);

   if (Gbl.CurrentCtr.Ctr.CtrCod > 0)
     {
      /***** Get degrees belonging to the current centre from database *****/
      sprintf (Query,"SELECT DegCod,ShortName FROM degrees"
                     " WHERE CtrCod='%ld'"
                     " ORDER BY ShortName",
               Gbl.CurrentCtr.Ctr.CtrCod);
      NumDegs = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get degrees of a centre");

      /***** Get degrees of this centre *****/
      for (NumDeg = 0;
	   NumDeg < NumDegs;
	   NumDeg++)
        {
         /* Get next degree */
         row = mysql_fetch_row (mysql_res);

         /* Get degree code (row[0]) */
         if ((DegCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
            Lay_ShowErrorAndExit ("Wrong degree.");

         /* Write option */
         fprintf (Gbl.F.Out,"<option value=\"%ld\"",DegCod);
         if (Gbl.CurrentDeg.Deg.DegCod > 0 &&
             (DegCod == Gbl.CurrentDeg.Deg.DegCod))
	    fprintf (Gbl.F.Out," selected=\"selected\"");
         fprintf (Gbl.F.Out,">%s</option>",row[1]);
        }

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }

   /***** End form *****/
   fprintf (Gbl.F.Out,"</select>");
   Act_FormEnd ();
  }

/*****************************************************************************/
/************* Write hierarchy breadcrumb in the top of the page *************/
/*****************************************************************************/

void Deg_WriteHierarchyBreadcrumb (void)
  {
   extern const char *The_ClassBreadcrumb[The_NUM_THEMES];
   extern const char *Txt_System;
   extern const char *Txt_Country;
   extern const char *Txt_Institution;
   extern const char *Txt_Centre;
   extern const char *Txt_Degree;
   char DegreeShortName[Deg_MAX_LENGTH_DEGREE_FULL_NAME+1];	// Full name of degree
   char ClassOn[64];
   char ClassSemiOff[64];
   char ClassOff[64];

   /***** CSS classes *****/
   strcpy (ClassOn,The_ClassBreadcrumb[Gbl.Prefs.Theme]);
   sprintf (ClassSemiOff,"BC_SEMIOFF %s",The_ClassBreadcrumb[Gbl.Prefs.Theme]);
   sprintf (ClassOff,"BC_OFF %s",The_ClassBreadcrumb[Gbl.Prefs.Theme]);

   /***** Form to go to the system *****/
   Act_FormGoToStart (ActMnu);
   Par_PutHiddenParamUnsigned ("NxtTab",(unsigned) TabSys);
   Act_LinkFormSubmit (Txt_System,ClassOn,NULL);
   fprintf (Gbl.F.Out,"%s</a>",Txt_System);
   Act_FormEnd ();

   if (Gbl.CurrentCty.Cty.CtyCod > 0)		// Country selected...
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassOn);

      /***** Form to go to see institutions of this country *****/
      Act_FormGoToStart (ActSeeIns);
      Cty_PutParamCtyCod (Gbl.CurrentCty.Cty.CtyCod);
      Act_LinkFormSubmit (Gbl.CurrentCty.Cty.Name[Gbl.Prefs.Language],ClassOn,NULL);
      fprintf (Gbl.F.Out,"%s</a>",
               Gbl.CurrentCty.Cty.Name[Gbl.Prefs.Language]);
      Act_FormEnd ();
     }
   else
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassSemiOff);

      /***** Form to go to select countries *****/
      Act_FormGoToStart (ActSeeCty);
      Act_LinkFormSubmit (Txt_Country,ClassSemiOff,NULL);
      fprintf (Gbl.F.Out,"%s</a>",Txt_Country);
      Act_FormEnd ();
     }

   if (Gbl.CurrentIns.Ins.InsCod > 0)		// Institution selected...
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassOn);

      /***** Form to see centres of this institution *****/
      Act_FormGoToStart (ActSeeCtr);
      Ins_PutParamInsCod (Gbl.CurrentIns.Ins.InsCod);
      Act_LinkFormSubmit (Gbl.CurrentIns.Ins.FullName,ClassOn,NULL);
      fprintf (Gbl.F.Out,"%s</a>",
	       Gbl.CurrentIns.Ins.ShortName);
      Act_FormEnd ();
     }
   else if (Gbl.CurrentCty.Cty.CtyCod > 0)
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassSemiOff);

      /***** Form to go to select institutions *****/
      Act_FormGoToStart (ActSeeIns);
      Act_LinkFormSubmit (Txt_Institution,ClassSemiOff,NULL);
      fprintf (Gbl.F.Out,"%s</a>",Txt_Institution);
      Act_FormEnd ();
     }
   else
      /***** Separator and hidden institution *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; %s</span>",
	       ClassOff,Txt_Institution);

   if (Gbl.CurrentCtr.Ctr.CtrCod > 0)	// Centre selected...
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassOn);

      /***** Form to see degrees of this centre *****/
      Act_FormGoToStart (ActSeeDeg);
      Ctr_PutParamCtrCod (Gbl.CurrentCtr.Ctr.CtrCod);
      Act_LinkFormSubmit (Gbl.CurrentCtr.Ctr.FullName,ClassOn,NULL);
      fprintf (Gbl.F.Out,"%s</a>",
	       Gbl.CurrentCtr.Ctr.ShortName);
      Act_FormEnd ();
     }
   else if (Gbl.CurrentIns.Ins.InsCod > 0)
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassSemiOff);

      /***** Form to go to select centres *****/
      Act_FormGoToStart (ActSeeCtr);
      Act_LinkFormSubmit (Txt_Centre,ClassSemiOff,NULL);
      fprintf (Gbl.F.Out,"%s</a>",Txt_Centre);
      Act_FormEnd ();
     }
   else
      /***** Separator and hidden centre *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; %s</span>",
	       ClassOff,Txt_Centre);

   if (Gbl.CurrentDeg.Deg.DegCod > 0)	// Degree selected...
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassOn);

      /***** Form to go to see courses of this degree *****/
      Act_FormGoToStart (ActSeeCrs);
      Deg_PutParamDegCod (Gbl.CurrentDeg.Deg.DegCod);
      Act_LinkFormSubmit (Gbl.CurrentDeg.Deg.FullName,ClassOn,NULL);
      strcpy (DegreeShortName,Gbl.CurrentDeg.Deg.ShortName);
      Str_LimitLengthHTMLStr (DegreeShortName,
			      Deg_MAX_LENGTH_SHORT_NAME_DEGREE_ON_PAGE_HEAD);
      fprintf (Gbl.F.Out,"%s</a>",
	       DegreeShortName);
      Act_FormEnd ();
     }
   else if (Gbl.CurrentCtr.Ctr.CtrCod > 0)
     {
      /***** Separator *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",ClassSemiOff);

      /***** Form to go to select degrees *****/
      Act_FormGoToStart (ActSeeDeg);
      Act_LinkFormSubmit (Txt_Degree,ClassSemiOff,NULL);
      fprintf (Gbl.F.Out,"%s</a>",Txt_Degree);
      Act_FormEnd ();
     }
   else
      /***** Separator and hidden degree *****/
      fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; %s</span>",
	       ClassOff,Txt_Degree);

   /***** Separator *****/
   fprintf (Gbl.F.Out,"<span class=\"%s\"> &gt; </span>",
	     (Gbl.CurrentCrs.Crs.CrsCod > 0) ? ClassOn :
            ((Gbl.CurrentDeg.Deg.DegCod > 0) ? ClassSemiOff :
		                               ClassOff));
  }

/*****************************************************************************/
/*************** Write course full name in the top of the page ***************/
/*****************************************************************************/

void Deg_WriteBigNameCtyInsCtrDegCrs (void)
  {
   extern const char *The_ClassCourse[The_NUM_THEMES];
   extern const char *Txt_TAGLINE;

   fprintf (Gbl.F.Out,"<h1 id=\"main_title\" class=\"%s\">",
	    The_ClassCourse[Gbl.Prefs.Theme]);

   /***** Logo *****/
   if (Gbl.CurrentCrs.Crs.CrsCod > 0 ||
       Gbl.CurrentDeg.Deg.DegCod > 0)
      Log_DrawLogo (Sco_SCOPE_DEG,Gbl.CurrentDeg.Deg.DegCod,
		    Gbl.CurrentDeg.Deg.ShortName,40,"TOP_LOGO",false);
   else if (Gbl.CurrentCtr.Ctr.CtrCod > 0)
      Log_DrawLogo (Sco_SCOPE_CTR,Gbl.CurrentCtr.Ctr.CtrCod,
		    Gbl.CurrentCtr.Ctr.ShortName,40,"TOP_LOGO",false);
   else if (Gbl.CurrentIns.Ins.InsCod > 0)
      Log_DrawLogo (Sco_SCOPE_INS,Gbl.CurrentIns.Ins.InsCod,
		    Gbl.CurrentIns.Ins.ShortName,40,"TOP_LOGO",false);
   else if (Gbl.CurrentCty.Cty.CtyCod > 0)
      Cty_DrawCountryMap (&Gbl.CurrentCty.Cty,"COUNTRY_MAP_TITLE");
   else
      fprintf (Gbl.F.Out,"<img src=\"%s/%s\""
                         " alt=\"%s\" title=\"%s\""
                         " class=\"TOP_LOGO\" />",
               Gbl.Prefs.IconsURL,Cfg_PLATFORM_LOGO_SMALL_FILENAME,
               Cfg_PLATFORM_SHORT_NAME,Cfg_PLATFORM_FULL_NAME);

   /***** Text *****/
   fprintf (Gbl.F.Out,"<div id=\"big_name_container\">");
   if (Gbl.CurrentCty.Cty.CtyCod > 0)
      fprintf (Gbl.F.Out,"<div id=\"big_full_name\">"
			 "%s"	// Full name
			 "</div>"
			 "<div class=\"NOT_SHOWN\">"
			 " / "	// To separate
			 "</div>"
			 "<div id=\"big_short_name\">"
			 "%s"	// Short name
			 "</div>",
		(Gbl.CurrentCrs.Crs.CrsCod > 0) ? Gbl.CurrentCrs.Crs.FullName :
	       ((Gbl.CurrentDeg.Deg.DegCod > 0) ? Gbl.CurrentDeg.Deg.FullName :
	       ((Gbl.CurrentCtr.Ctr.CtrCod > 0) ? Gbl.CurrentCtr.Ctr.FullName :
	       ((Gbl.CurrentIns.Ins.InsCod > 0) ? Gbl.CurrentIns.Ins.FullName :
	                                          Gbl.CurrentCty.Cty.Name[Gbl.Prefs.Language]))),
		(Gbl.CurrentCrs.Crs.CrsCod > 0) ? Gbl.CurrentCrs.Crs.ShortName :
	       ((Gbl.CurrentDeg.Deg.DegCod > 0) ? Gbl.CurrentDeg.Deg.ShortName :
	       ((Gbl.CurrentCtr.Ctr.CtrCod > 0) ? Gbl.CurrentCtr.Ctr.ShortName :
	       ((Gbl.CurrentIns.Ins.InsCod > 0) ? Gbl.CurrentIns.Ins.ShortName :
	                                          Gbl.CurrentCty.Cty.Name[Gbl.Prefs.Language]))));
   else	// No country specified ==> home page
      fprintf (Gbl.F.Out,"<div id=\"big_full_name\">"
			 "%s: %s"	// Full name
			 "</div>"
			 "<div class=\"NOT_SHOWN\">"
			 " / "		// To separate
			 "</div>"
			 "<div id=\"big_short_name\">"
			 "%s"		// Short name
			 "</div>",
	       Cfg_PLATFORM_SHORT_NAME,Txt_TAGLINE,
	       Cfg_PLATFORM_SHORT_NAME);
   fprintf (Gbl.F.Out,"</div>"
	              "</h1>");
  }

/*****************************************************************************/
/**************** Initialize values related to current course ****************/
/*****************************************************************************/

void Deg_InitCurrentCourse (void)
  {
   /***** If numerical course code is available, get course data *****/
   if (Gbl.CurrentCrs.Crs.CrsCod > 0)
     {
      if (Crs_GetDataOfCourseByCod (&Gbl.CurrentCrs.Crs))		// Course found
         Gbl.CurrentDeg.Deg.DegCod = Gbl.CurrentCrs.Crs.DegCod;
      else
        {
         Gbl.CurrentIns.Ins.InsCod =
         Gbl.CurrentCtr.Ctr.CtrCod =
         Gbl.CurrentDeg.Deg.DegCod =
         Gbl.CurrentCrs.Crs.CrsCod = -1L;
        }
     }

   /***** If numerical degree code is available, get degree data *****/
   if (Gbl.CurrentDeg.Deg.DegCod > 0)
     {
      if (Deg_GetDataOfDegreeByCod (&Gbl.CurrentDeg.Deg))	// Degree found
	{
	 Gbl.CurrentCtr.Ctr.CtrCod          = Gbl.CurrentDeg.Deg.CtrCod;
         Gbl.CurrentDegTyp.DegTyp.DegTypCod = Gbl.CurrentDeg.Deg.DegTypCod;
         Gbl.CurrentIns.Ins.InsCod = Deg_GetInsCodOfDegreeByCod (Gbl.CurrentDeg.Deg.DegCod);

         /***** Degree type is available, so get degree type data *****/
         if (!DT_GetDataOfDegreeTypeByCod (&Gbl.CurrentDegTyp.DegTyp))	// Degree type not found
           {
	    Gbl.CurrentIns.Ins.InsCod =
	    Gbl.CurrentCtr.Ctr.CtrCod =
	    Gbl.CurrentDeg.Deg.DegTypCod =
	    Gbl.CurrentDeg.Deg.DegCod =
	    Gbl.CurrentCrs.Crs.CrsCod = -1L;
           }
	}
      else
        {
         Gbl.CurrentIns.Ins.InsCod =
         Gbl.CurrentCtr.Ctr.CtrCod =
         Gbl.CurrentDeg.Deg.DegCod =
         Gbl.CurrentCrs.Crs.CrsCod = -1L;
        }
     }

   /***** If centre code is available, get centre data *****/
   if (Gbl.CurrentCtr.Ctr.CtrCod > 0)
     {
      if (Ctr_GetDataOfCentreByCod (&Gbl.CurrentCtr.Ctr))	// Centre found
         Gbl.CurrentIns.Ins.InsCod = Gbl.CurrentCtr.Ctr.InsCod;
      else
         Gbl.CurrentCtr.Ctr.CtrCod = -1L;
     }

   /***** If numerical institution code is available, get institution data *****/
   if (Gbl.CurrentIns.Ins.InsCod > 0)
     {
      if (Ins_GetDataOfInstitutionByCod (&Gbl.CurrentIns.Ins,Ins_GET_BASIC_DATA))	// Institution found
	 Gbl.CurrentCty.Cty.CtyCod = Gbl.CurrentIns.Ins.CtyCod;
      else
        {
         Gbl.CurrentCty.Cty.CtyCod =
         Gbl.CurrentIns.Ins.InsCod =
         Gbl.CurrentCtr.Ctr.CtrCod =
         Gbl.CurrentDeg.Deg.DegCod =
         Gbl.CurrentCrs.Crs.CrsCod = -1L;
        }
     }

   /***** If numerical country code is available, get country data *****/
   if (Gbl.CurrentCty.Cty.CtyCod > 0)
     {
      if (!Cty_GetDataOfCountryByCod (&Gbl.CurrentCty.Cty,Cty_GET_BASIC_DATA))	// Country not found
        {
         Gbl.CurrentCty.Cty.CtyCod =
         Gbl.CurrentIns.Ins.InsCod =
         Gbl.CurrentCtr.Ctr.CtrCod =
         Gbl.CurrentDeg.Deg.DegCod =
         Gbl.CurrentCrs.Crs.CrsCod = -1L;
        }
     }

   /***** Initialize default fields for edition to current values *****/
   Gbl.Inss.EditingIns.CtyCod    = Gbl.CurrentCty.Cty.CtyCod;
   Gbl.Ctrs.EditingCtr.InsCod    =
   Gbl.Dpts.EditingDpt.InsCod    = Gbl.CurrentIns.Ins.InsCod;
   Gbl.Degs.EditingDeg.CtrCod    = Gbl.CurrentCtr.Ctr.CtrCod;
   Gbl.Degs.EditingDeg.DegTypCod = Gbl.CurrentDegTyp.DegTyp.DegTypCod;

   /***** Initialize paths *****/
   if (Gbl.CurrentCrs.Crs.CrsCod > 0)
     {
      /***** Paths of course directories *****/
      sprintf (Gbl.CurrentCrs.PathPriv,"%s/%s/%ld",
	       Cfg_PATH_SWAD_PRIVATE,Cfg_FOLDER_CRS,Gbl.CurrentCrs.Crs.CrsCod);
      sprintf (Gbl.CurrentCrs.PathRelPubl,"%s/%s/%ld",
	       Cfg_PATH_SWAD_PUBLIC ,Cfg_FOLDER_CRS,Gbl.CurrentCrs.Crs.CrsCod);
      sprintf (Gbl.CurrentCrs.PathURLPubl,"%s/%s/%ld",
	       Cfg_URL_SWAD_PUBLIC,Cfg_FOLDER_CRS,Gbl.CurrentCrs.Crs.CrsCod);

      /***** If any of the course directories does not exist, create it *****/
      if (!Fil_CheckIfPathExists (Gbl.CurrentCrs.PathPriv))
	 Fil_CreateDirIfNotExists (Gbl.CurrentCrs.PathPriv);
      if (!Fil_CheckIfPathExists (Gbl.CurrentCrs.PathRelPubl))
	 Fil_CreateDirIfNotExists (Gbl.CurrentCrs.PathRelPubl);

      /***** Count number of groups in current course (used only in some actions) *****/
      Gbl.CurrentCrs.Grps.NumGrps = Grp_CountNumGrpsInCurrentCrs ();
     }
  }

/*****************************************************************************/
/************* Show the degrees belonging to the current centre **************/
/*****************************************************************************/

void Deg_ShowDegsOfCurrentCtr (void)
  {
   if (Gbl.CurrentCtr.Ctr.CtrCod > 0)
     {
      /***** Get list of centres and degrees *****/
      Ctr_GetListCentres (Gbl.CurrentIns.Ins.InsCod);
      Deg_GetListDegsOfCurrentCtr ();

      /***** Write menu to select country, institution and centre *****/
      Deg_WriteMenuAllCourses ();

      /***** Show list of degrees *****/
      Deg_ListDegrees ();

      /***** Free list of degrees and centres *****/
      Deg_FreeListDegsOfCurrentCtr ();
      Ctr_FreeListCentres ();
     }
  }

/*****************************************************************************/
/********************* List current degrees for edition **********************/
/*****************************************************************************/

#define Deg_MAX_LENGTH_WWW_ON_SCREEN 10

static void Deg_ListDegreesForEdition (void)
  {
   extern const char *Txt_Degrees_of_CENTRE_X;
   extern const char *Txt_DEGREE_STATUS[Deg_NUM_STATUS_TXT];
   unsigned NumDeg;
   struct DegreeType *DegTyp;
   struct Degree *Deg;
   unsigned NumCtr;
   unsigned NumDegTyp;
   char WWW[Deg_MAX_LENGTH_WWW_ON_SCREEN+1];
   struct UsrData UsrDat;
   bool ICanEdit;
   Deg_StatusTxt_t StatusTxt;
   unsigned NumCrss;

   /***** Initialize structure with user's data *****/
   Usr_UsrDataConstructor (&UsrDat);

   /***** Write heading *****/
   sprintf (Gbl.Title,Txt_Degrees_of_CENTRE_X,
            Gbl.CurrentCtr.Ctr.ShortName);
   Lay_StartRoundFrameTable (NULL,2,Gbl.Title);
   Deg_PutHeadDegreesForEdition ();

   /***** List the degrees *****/
   for (NumDeg = 0;
	NumDeg < Gbl.CurrentCtr.Ctr.NumDegs;
	NumDeg++)
     {
      Deg = &(Gbl.CurrentCtr.LstDegs[NumDeg]);

      NumCrss = Crs_GetNumCrssInDeg (Deg->DegCod);

      ICanEdit = Deg_CheckIfICanEdit (Deg);

      /* Put icon to remove degree */
      fprintf (Gbl.F.Out,"<tr>"
	                 "<td class=\"BM\">");
      if (NumCrss ||	// Degree has courses ==> deletion forbidden
	  !ICanEdit)
         Lay_PutIconRemovalNotAllowed ();
      else
        {
         Act_FormStart (ActRemDeg);
         Deg_PutParamOtherDegCod (Deg->DegCod);
         Lay_PutIconRemove ();
         Act_FormEnd ();
        }
      fprintf (Gbl.F.Out,"</td>");

      /* Degree code */
      fprintf (Gbl.F.Out,"<td class=\"DAT CODE\">"
	                 "%ld"
	                 "</td>",
               Deg->DegCod);

      /* Degree logo */
      fprintf (Gbl.F.Out,"<td title=\"%s LEFT_MIDDLE\" style=\"width:25px;\">",
               Deg->FullName);
      Log_DrawLogo (Sco_SCOPE_DEG,Deg->DegCod,Deg->ShortName,20,NULL,true);
      fprintf (Gbl.F.Out,"</td>");

      /* Centre */
      fprintf (Gbl.F.Out,"<td class=\"DAT LEFT_MIDDLE\">");
      if (Gbl.Usrs.Me.LoggedRole >= Rol_INS_ADM)	// I can select centre
	{
	 Act_FormStart (ActChgDegCtr);
	 Deg_PutParamOtherDegCod (Deg->DegCod);
	 fprintf (Gbl.F.Out,"<select name=\"OthCtrCod\" style=\"width:62px;\""
			    " onchange=\"document.getElementById('%s').submit();\">",
		  Gbl.Form.Id);
	 for (NumCtr = 0;
	      NumCtr < Gbl.Ctrs.Num;
	      NumCtr++)
	    fprintf (Gbl.F.Out,"<option value=\"%ld\"%s>%s</option>",
		     Gbl.Ctrs.Lst[NumCtr].CtrCod,
		     (Gbl.Ctrs.Lst[NumCtr].CtrCod == Deg->CtrCod) ? " selected=\"selected\"" :
			                                            "",
		     Gbl.Ctrs.Lst[NumCtr].ShortName);
	 fprintf (Gbl.F.Out,"</select>");
	 Act_FormEnd ();
	}
      else
	 fprintf (Gbl.F.Out,"%s",Gbl.CurrentCtr.Ctr.ShortName);
      fprintf (Gbl.F.Out,"</td>");

      /* Degree short name */
      fprintf (Gbl.F.Out,"<td class=\"DAT LEFT_MIDDLE\">");
      if (ICanEdit)
	{
	 Act_FormStart (ActRenDegSho);
	 Deg_PutParamOtherDegCod (Deg->DegCod);
	 fprintf (Gbl.F.Out,"<input type=\"text\" name=\"ShortName\""
	                    " maxlength=\"%u\" value=\"%s\""
                            " class=\"INPUT_SHORT_NAME\""
			    " onchange=\"document.getElementById('%s').submit();\" />",
		  Deg_MAX_LENGTH_DEGREE_SHORT_NAME,Deg->ShortName,Gbl.Form.Id);
	 Act_FormEnd ();
	}
      else
	 fprintf (Gbl.F.Out,"%s",Deg->ShortName);
      fprintf (Gbl.F.Out,"</td>");

      /* Degree full name */
      fprintf (Gbl.F.Out,"<td class=\"DAT LEFT_MIDDLE\">");
      if (ICanEdit)
	{
	 Act_FormStart (ActRenDegFul);
	 Deg_PutParamOtherDegCod (Deg->DegCod);
	 fprintf (Gbl.F.Out,"<input type=\"text\" name=\"FullName\""
	                    " maxlength=\"%u\" value=\"%s\""
                            " class=\"INPUT_FULL_NAME\""
			    " onchange=\"document.getElementById('%s').submit();\" />",
		  Deg_MAX_LENGTH_DEGREE_FULL_NAME,Deg->FullName,Gbl.Form.Id);
	 Act_FormEnd ();
	}
      else
	 fprintf (Gbl.F.Out,"%s",Deg->FullName);
      fprintf (Gbl.F.Out,"</td>");

      /* Degree type */
      fprintf (Gbl.F.Out,"<td class=\"DAT LEFT_MIDDLE\">");
      if (ICanEdit)
	{
	 Act_FormStart (ActChgDegTyp);
	 Deg_PutParamOtherDegCod (Deg->DegCod);
	 fprintf (Gbl.F.Out,"<select name=\"OthDegTypCod\""
	                    " style=\"width:62px;\""
			    " onchange=\"document.getElementById('%s').submit();\">",
		  Gbl.Form.Id);
	 for (NumDegTyp = 0;
	      NumDegTyp < Gbl.Degs.DegTypes.Num;
	      NumDegTyp++)
	   {
	    DegTyp = &Gbl.Degs.DegTypes.Lst[NumDegTyp];
	    fprintf (Gbl.F.Out,"<option value=\"%ld\"%s>%s</option>",
		     DegTyp->DegTypCod,
		     (DegTyp->DegTypCod == Deg->DegTypCod) ? " selected=\"selected\"" :
			                                     "",
		     DegTyp->DegTypName);
	   }
	 fprintf (Gbl.F.Out,"</select>");
	 Act_FormEnd ();
	}
      else
	 for (NumDegTyp = 0;
	      NumDegTyp < Gbl.Degs.DegTypes.Num;
	      NumDegTyp++)
	    if (Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypCod == Deg->DegTypCod)
	       fprintf (Gbl.F.Out,"%s",Gbl.Degs.DegTypes.Lst[NumDegTyp].DegTypName);
      fprintf (Gbl.F.Out,"</td>");

      /* Degree WWW */
      fprintf (Gbl.F.Out,"<td class=\"DAT LEFT_MIDDLE\">");
      if (ICanEdit)
	{
	 Act_FormStart (ActChgDegWWW);
	 Deg_PutParamOtherDegCod (Deg->DegCod);
	 fprintf (Gbl.F.Out,"<input type=\"text\" name=\"WWW\""
	                    " maxlength=\"%u\" value=\"%s\""
                            " class=\"INPUT_WWW\""
			    " onchange=\"document.getElementById('%s').submit();\" />",
		  Cns_MAX_LENGTH_WWW,Deg->WWW,Gbl.Form.Id);
	 Act_FormEnd ();
	}
      else
	{
         strncpy (WWW,Deg->WWW,Deg_MAX_LENGTH_WWW_ON_SCREEN);
         WWW[Deg_MAX_LENGTH_WWW_ON_SCREEN] = '\0';
         fprintf (Gbl.F.Out,"<a href=\"%s\" target=\"_blank\""
                            " class=\"DAT\" title=\"%s\">%s",
                  Deg->WWW,Deg->WWW,WWW);
         if (strlen (Deg->WWW) > Deg_MAX_LENGTH_WWW_ON_SCREEN)
            fprintf (Gbl.F.Out,"&hellip;");
         fprintf (Gbl.F.Out,"</a>");
	}
      fprintf (Gbl.F.Out,"</td>");

      /* Current number of courses in this degree */
      fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
	                 "%u"
	                 "</td>",
               NumCrss);

      /* Degree status */
      StatusTxt = Deg_GetStatusTxtFromStatusBits (Deg->Status);
      fprintf (Gbl.F.Out,"<td class=\"DAT STATUS\">");
      if (Gbl.Usrs.Me.LoggedRole >= Rol_CTR_ADM &&
	  StatusTxt == Deg_STATUS_PENDING)
	{
	 Act_FormStart (ActChgDegSta);
	 Deg_PutParamOtherDegCod (Deg->DegCod);
	 fprintf (Gbl.F.Out,"<select name=\"Status\" class=\"INPUT_STATUS\""
			    " onchange=\"document.getElementById('%s').submit();\">"
			    "<option value=\"%u\" selected=\"selected\">%s</option>"
			    "<option value=\"%u\">%s</option>"
			    "</select>",
		  Gbl.Form.Id,
		  (unsigned) Deg_GetStatusBitsFromStatusTxt (Deg_STATUS_PENDING),
		  Txt_DEGREE_STATUS[Deg_STATUS_PENDING],
		  (unsigned) Deg_GetStatusBitsFromStatusTxt (Deg_STATUS_ACTIVE),
		  Txt_DEGREE_STATUS[Deg_STATUS_ACTIVE]);
	 Act_FormEnd ();
	}
      else
	 fprintf (Gbl.F.Out,"%s",Txt_DEGREE_STATUS[StatusTxt]);
      fprintf (Gbl.F.Out,"</td>");

      /* Degree requester */
      UsrDat.UsrCod = Deg->RequesterUsrCod;
      Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (&UsrDat);
      fprintf (Gbl.F.Out,"<td class=\"INPUT_REQUESTER LEFT_TOP\">"
			 "<table class=\"INPUT_REQUESTER CELLS_PAD_2\">"
			 "<tr>");
      Msg_WriteMsgAuthor (&UsrDat,100,6,"DAT",true,NULL);
      fprintf (Gbl.F.Out,"</tr>"
			 "</table>"
			 "</td>"
			 "</tr>");
     }

   /***** End table *****/
   Lay_EndRoundFrameTable ();

   /***** Free memory used for user's data *****/
   Usr_UsrDataDestructor (&UsrDat);
  }

/*****************************************************************************/
/************** Check if I can edit, remove, etc. a degree *******************/
/*****************************************************************************/

static bool Deg_CheckIfICanEdit (struct Degree *Deg)
  {
   return (bool) (Gbl.Usrs.Me.LoggedRole >= Rol_CTR_ADM ||		// I am a centre administrator or higher
                  ((Deg->Status & Deg_STATUS_BIT_PENDING) != 0 &&		// Degree is not yet activated
                   Gbl.Usrs.Me.UsrDat.UsrCod == Deg->RequesterUsrCod));		// I am the requester
  }

/*****************************************************************************/
/******************* Set StatusTxt depending on status bits ******************/
/*****************************************************************************/
// Deg_STATUS_UNKNOWN = 0	// Other
// Deg_STATUS_ACTIVE  = 1	// 00 (Status == 0)
// Deg_STATUS_PENDING = 2	// 01 (Status == Deg_STATUS_BIT_PENDING)
// Deg_STATUS_REMOVED = 3	// 1- (Status & Deg_STATUS_BIT_REMOVED)

static Deg_StatusTxt_t Deg_GetStatusTxtFromStatusBits (Deg_Status_t Status)
  {
   if (Status == 0)
      return Deg_STATUS_ACTIVE;
   if (Status == Deg_STATUS_BIT_PENDING)
      return Deg_STATUS_PENDING;
   if (Status & Deg_STATUS_BIT_REMOVED)
      return Deg_STATUS_REMOVED;
   return Deg_STATUS_UNKNOWN;
  }

/*****************************************************************************/
/******************* Set status bits depending on StatusTxt ******************/
/*****************************************************************************/
// Deg_STATUS_UNKNOWN = 0	// Other
// Deg_STATUS_ACTIVE  = 1	// 00 (Status == 0)
// Deg_STATUS_PENDING = 2	// 01 (Status == Deg_STATUS_BIT_PENDING)
// Deg_STATUS_REMOVED = 3	// 1- (Status & Deg_STATUS_BIT_REMOVED)

static Deg_Status_t Deg_GetStatusBitsFromStatusTxt (Deg_StatusTxt_t StatusTxt)
  {
   switch (StatusTxt)
     {
      case Deg_STATUS_UNKNOWN:
      case Deg_STATUS_ACTIVE:
	 return (Deg_Status_t) 0;
      case Deg_STATUS_PENDING:
	 return Deg_STATUS_BIT_PENDING;
      case Deg_STATUS_REMOVED:
	 return Deg_STATUS_BIT_REMOVED;
     }
   return (Deg_Status_t) 0;
  }

/*****************************************************************************/
/*********************** Put a form to create a new degree *******************/
/*****************************************************************************/

static void Deg_PutFormToCreateDegree (void)
  {
   extern const char *Txt_New_degree_of_CENTRE_X;
   extern const char *Txt_DEGREE_STATUS[Deg_NUM_STATUS_TXT];
   extern const char *Txt_Create_degree;
   struct Degree *Deg;
   struct DegreeType *DegTyp;
   unsigned NumDegTyp;

   /***** Start form *****/
   if (Gbl.Usrs.Me.LoggedRole >= Rol_CTR_ADM)
      Act_FormStart (ActNewDeg);
   else if (Gbl.Usrs.Me.MaxRole >= Rol__GUEST_)
      Act_FormStart (ActReqDeg);
   else
      Lay_ShowErrorAndExit ("You can not edit degrees.");

   /***** Degree data *****/
   Deg = &Gbl.Degs.EditingDeg;

   /***** Start of frame *****/
   sprintf (Gbl.Title,Txt_New_degree_of_CENTRE_X,
            Gbl.CurrentCtr.Ctr.ShortName);
   Lay_StartRoundFrameTable (NULL,2,Gbl.Title);

   /***** Write heading *****/
   Deg_PutHeadDegreesForEdition ();

   /***** Put disabled icon to remove degree *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"BM\">");
   Lay_PutIconRemovalNotAllowed ();
   fprintf (Gbl.F.Out,"</td>");

   /***** Degree code *****/
   fprintf (Gbl.F.Out,"<td class=\"CODE\"></td>");

   /***** Degree logo *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">");
   Log_DrawLogo (Sco_SCOPE_DEG,-1L,"",20,NULL,true);
   fprintf (Gbl.F.Out,"</td>");

   /***** Centre *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">"
                      "<select name=\"OthCtrCod\""
                      " style=\"width:62px;\" disabled=\"disabled\">"
                      "<option value=\"%ld\" selected=\"selected\">"
                      "%s"
                      "</option>"
                      "</select>"
                      "</td>",
            Gbl.CurrentCtr.Ctr.CtrCod,
	    Gbl.CurrentCtr.Ctr.ShortName);

   /***** Degree short name *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"text\" name=\"ShortName\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_SHORT_NAME\" />"
                      "</td>",
            Deg_MAX_LENGTH_DEGREE_SHORT_NAME,Deg->ShortName);

   /***** Degree full name *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"text\" name=\"FullName\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_FULL_NAME\" />"
                      "</td>",
            Deg_MAX_LENGTH_DEGREE_FULL_NAME,Deg->FullName);

   /***** Degree type *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">"
                      "<select name=\"OthDegTypCod\" style=\"width:62px;\">");
   for (NumDegTyp = 0;
	NumDegTyp < Gbl.Degs.DegTypes.Num;
	NumDegTyp++)
     {
      DegTyp = &Gbl.Degs.DegTypes.Lst[NumDegTyp];
      fprintf (Gbl.F.Out,"<option value=\"%ld\"%s>%s</option>",
	       DegTyp->DegTypCod,
	       DegTyp->DegTypCod == Deg->DegTypCod ? " selected=\"selected\"" :
		                                     "",
	       DegTyp->DegTypName);
     }
   fprintf (Gbl.F.Out,"</select>"
	              "</td>");

   /***** Degree WWW *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE\">"
                      "<input type=\"text\" name=\"WWW\""
                      " maxlength=\"%u\" value=\"%s\""
                      " class=\"INPUT_WWW\" />"
                      "</td>",
            Cns_MAX_LENGTH_WWW,Deg->WWW);

   /***** Current number of courses in this degree *****/
   fprintf (Gbl.F.Out,"<td class=\"DAT RIGHT_MIDDLE\">"
	              "0"
	              "</td>");

   /***** Degree status *****/
   fprintf (Gbl.F.Out,"<td class=\"DAT STATUS\">"
	              "%s"
	              "</td>",
            Txt_DEGREE_STATUS[Deg_STATUS_PENDING]);

   /***** Degree requester *****/
   fprintf (Gbl.F.Out,"<td class=\"INPUT_REQUESTER LEFT_TOP\">"
		      "<table class=\"INPUT_REQUESTER CELLS_PAD_2\">"
		      "<tr>");
   Msg_WriteMsgAuthor (&Gbl.Usrs.Me.UsrDat,100,6,"DAT",true,NULL);
   fprintf (Gbl.F.Out,"</tr>"
		      "</table>"
		      "</td>"
		      "</tr>");

   /***** Send button and end frame *****/
   Lay_EndRoundFrameTableWithButton (Lay_CREATE_BUTTON,Txt_Create_degree);

   /***** End form *****/
   Act_FormEnd ();
  }

/*****************************************************************************/
/******************** Write header with fields of a degree *******************/
/*****************************************************************************/

static void Deg_PutHeadDegreesForSeeing (void)
  {
   extern const char *Txt_Degree;
   extern const char *Txt_Type;
   extern const char *Txt_Courses_ABBREVIATION;
   extern const char *Txt_Status;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"BM\"></th>"
                      "<th></th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Degree,
            Txt_Type,
            Txt_Courses_ABBREVIATION,
            Txt_Status);
  }

/*****************************************************************************/
/******************** Write header with fields of a degree *******************/
/*****************************************************************************/

static void Deg_PutHeadDegreesForEdition (void)
  {
   extern const char *Txt_Code;
   extern const char *Txt_Centre;
   extern const char *Txt_Short_name_of_the_degree;
   extern const char *Txt_Full_name_of_the_degree;
   extern const char *Txt_Type;
   extern const char *Txt_WWW;
   extern const char *Txt_Courses_ABBREVIATION;
   extern const char *Txt_Status;
   extern const char *Txt_Requester;

   fprintf (Gbl.F.Out,"<tr>"
                      "<th class=\"BM\"></th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th></th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"RIGHT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "<th class=\"LEFT_MIDDLE\">"
                      "%s"
                      "</th>"
                      "</tr>",
            Txt_Code,
            Txt_Centre,
            Txt_Short_name_of_the_degree,
            Txt_Full_name_of_the_degree,
            Txt_Type,
            Txt_WWW,
            Txt_Courses_ABBREVIATION,
            Txt_Status,
            Txt_Requester);
  }

/*****************************************************************************/
/******************** Convert string to year of a degree *********************/
/*****************************************************************************/

unsigned Deg_ConvStrToYear (const char *StrYear)
  {
   int Year;

   if (sscanf (StrYear,"%d",&Year) != 1)
      return 0;
   if (Year < 0)
      return 0;
   if (Year > Deg_MAX_YEARS_PER_DEGREE)
      return Deg_MAX_YEARS_PER_DEGREE;
   return (unsigned) Year;
  }

/*****************************************************************************/
/***************************** Create a new degree ***************************/
/*****************************************************************************/

static void Deg_CreateDegree (struct Degree *Deg,unsigned Status)
  {
   extern const char *Txt_Created_new_degree_X;
   char Query[1024];

   /***** Create a new degree *****/
   sprintf (Query,"INSERT INTO degrees (CtrCod,DegTypCod,Status,"
	          "RequesterUsrCod,ShortName,FullName,WWW)"
                  " VALUES ('%ld','%ld','%u',"
                  "'%ld','%s','%s','%s')",
            Deg->CtrCod,Deg->DegTypCod,Status,
            Gbl.Usrs.Me.UsrDat.UsrCod,Deg->ShortName,Deg->FullName,Deg->WWW);
   Deg->DegCod = DB_QueryINSERTandReturnCode (Query,"can not create a new degree");

   /***** Write success message *****/
   sprintf (Gbl.Message,Txt_Created_new_degree_X,
            Deg->FullName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);

   /***** Put button to go to degree created *****/
   Deg_PutButtonToGoToDeg (Deg);
  }

/*****************************************************************************/
/************** List degrees belonging to the current centre *****************/
/*****************************************************************************/

static void Deg_ListDegrees (void)
  {
   extern const char *Txt_Degrees_of_CENTRE_X;
   extern const char *Txt_No_degrees;
   extern const char *Txt_Create_another_degree;
   extern const char *Txt_Create_degree;
   unsigned NumDeg;
   bool ICanEdit = (Gbl.Usrs.Me.LoggedRole >= Rol__GUEST_);

   /***** Start frame *****/
   sprintf (Gbl.Title,Txt_Degrees_of_CENTRE_X,Gbl.CurrentCtr.Ctr.ShortName);
   Lay_StartRoundFrame (NULL,Gbl.Title,ICanEdit ? Deg_PutIconToEditDegrees :
                                                  NULL);

   if (Gbl.CurrentCtr.Ctr.NumDegs)	// There are degrees in the current centre
     {
      /***** Start table *****/
      fprintf (Gbl.F.Out,"<table class=\"FRAME_TABLE_MARGIN CELLS_PAD_2\">");
      Deg_PutHeadDegreesForSeeing ();

      /***** List the degrees *****/
      for (NumDeg = 0;
	   NumDeg < Gbl.CurrentCtr.Ctr.NumDegs;
	   NumDeg++)
	 Deg_ListOneDegreeForSeeing (&(Gbl.CurrentCtr.LstDegs[NumDeg]),NumDeg + 1);

      /***** End table *****/
      fprintf (Gbl.F.Out,"</table>");
     }
   else	// No degrees created in the current centre
      Lay_ShowAlert (Lay_INFO,Txt_No_degrees);

   /***** Button to create degree *****/
   if (ICanEdit)
     {
      Act_FormStart (ActEdiDeg);
      Lay_PutConfirmButton (Gbl.CurrentCtr.Ctr.NumDegs ? Txt_Create_another_degree :
	                                                 Txt_Create_degree);
      Act_FormEnd ();
     }

   /***** End frame *****/
   Lay_EndRoundFrame ();
  }

/*****************************************************************************/
/********************** Put link (form) to edit degrees **********************/
/*****************************************************************************/

static void Deg_PutIconToEditDegrees (void)
  {
   extern const char *Txt_Edit;

   Lay_PutContextualLink (ActEdiDeg,NULL,
                          "edit64x64.png",
                          Txt_Edit,NULL,
                          NULL);
  }

/*****************************************************************************/
/************************ List one degree for seeing *************************/
/*****************************************************************************/

static void Deg_ListOneDegreeForSeeing (struct Degree *Deg,unsigned NumDeg)
  {
   extern const char *Txt_DEGREE_With_courses;
   extern const char *Txt_DEGREE_Without_courses;
   extern const char *Txt_DEGREE_STATUS[Deg_NUM_STATUS_TXT];
   struct DegreeType DegTyp;
   const char *TxtClassNormal;
   const char *TxtClassStrong;
   const char *BgColor;
   Crs_StatusTxt_t StatusTxt;
   unsigned NumCrss;

   /***** Get number of courses in this degree *****/
   NumCrss = Crs_GetNumCrssInDeg (Deg->DegCod);

   /***** Get data of type of degree of this degree *****/
   DegTyp.DegTypCod = Deg->DegTypCod;
   if (!DT_GetDataOfDegreeTypeByCod (&DegTyp))
      Lay_ShowErrorAndExit ("Code of type of degree not found.");

   if (Deg->Status & Deg_STATUS_BIT_PENDING)
     {
      TxtClassNormal = "DAT_LIGHT";
      TxtClassStrong = "DAT_LIGHT";
     }
   else
     {
      TxtClassNormal = "DAT";
      TxtClassStrong = "DAT_N";
     }
   BgColor = (Deg->DegCod == Gbl.CurrentDeg.Deg.DegCod) ? "LIGHT_BLUE" :
                                                          Gbl.ColorRows[Gbl.RowEvenOdd];

   /***** Put green tip if degree has courses *****/
   fprintf (Gbl.F.Out,"<tr>"
		      "<td class=\"CENTER_MIDDLE %s\">"
		      "<img src=\"%s/%s16x16.gif\""
		      " alt=\"%s\" title=\"%s\""
		      " class=\"ICON20x20\" />"
		      "</td>",
	    BgColor,
	    Gbl.Prefs.IconsURL,
	    NumCrss ? "ok_green" :
		      "tr",
	    NumCrss ? Txt_DEGREE_With_courses :
		      Txt_DEGREE_Without_courses,
	    NumCrss ? Txt_DEGREE_With_courses :
		      Txt_DEGREE_Without_courses);

   /***** Number of degree in this list *****/
   fprintf (Gbl.F.Out,"<td class=\"%s RIGHT_MIDDLE %s\">"
                      "%u"
                      "</td>",
	    TxtClassNormal,BgColor,
            NumDeg);

   /***** Degree logo and name *****/
   fprintf (Gbl.F.Out,"<td class=\"LEFT_MIDDLE %s\">",BgColor);
   Deg_DrawDegreeLogoAndNameWithLink (Deg,ActSeeCrs,
                                      TxtClassStrong,"CENTER_MIDDLE");
   fprintf (Gbl.F.Out,"</td>");

   /***** Type of degree *****/
   fprintf (Gbl.F.Out,"<td class=\"%s LEFT_MIDDLE %s\">"
	              "%s"
	              "</td>",
	    TxtClassNormal,BgColor,DegTyp.DegTypName);

   /***** Current number of courses in this degree *****/
   fprintf (Gbl.F.Out,"<td class=\"%s RIGHT_MIDDLE %s\">"
	              "%u"
	              "</td>",
	    TxtClassNormal,BgColor,NumCrss);

   /***** Degree status *****/
   StatusTxt = Deg_GetStatusTxtFromStatusBits (Deg->Status);
   fprintf (Gbl.F.Out,"<td class=\"%s LEFT_MIDDLE %s\">"
	              "%s"
	              "</td>"
		      "</tr>",
	    TxtClassNormal,BgColor,Txt_DEGREE_STATUS[StatusTxt]);

   Gbl.RowEvenOdd = 1 - Gbl.RowEvenOdd;
  }

/*****************************************************************************/
/*************************** Put forms to edit degrees ***********************/
/*****************************************************************************/

void Deg_EditDegrees (void)
  {
   extern const char *Txt_There_is_no_list_of_centres;
   extern const char *Txt_You_must_create_at_least_one_centre_before_creating_degrees;
   extern const char *Txt_There_is_no_list_of_types_of_degree;
   extern const char *Txt_You_must_create_at_least_one_type_of_degree_before_creating_degrees;

   /***** Get list of degrees in the current centre *****/
   Deg_GetListDegsOfCurrentCtr ();

   /***** Get list of centres of the current institution *****/
   Ctr_GetListCentres (Gbl.CurrentIns.Ins.InsCod);

   if (Gbl.Ctrs.Num)
     {
      /***** Get list of degree types *****/
      DT_GetListDegreeTypes ();

      if (Gbl.Degs.DegTypes.Num)
	{
	 /***** Put a form to create a new degree *****/
	 Deg_PutFormToCreateDegree ();

	 /***** Forms to edit current degrees *****/
	 if (Gbl.CurrentCtr.Ctr.NumDegs)
	   {
	    if (Gbl.Ctrs.Num)
	       Deg_ListDegreesForEdition ();
	    else
	       Lay_ShowAlert (Lay_WARNING,Txt_There_is_no_list_of_centres);
	   }
	}
      else	// No degree types
	{
	 Lay_ShowAlert (Lay_WARNING,Txt_There_is_no_list_of_types_of_degree);
	 Lay_ShowAlert (Lay_INFO,Txt_You_must_create_at_least_one_type_of_degree_before_creating_degrees);
	}

      /***** Free list of degree types *****/
      DT_FreeListDegreeTypes ();
     }
   else	// No centres
     {
      Lay_ShowAlert (Lay_WARNING,Txt_There_is_no_list_of_centres);
      Lay_ShowAlert (Lay_INFO,Txt_You_must_create_at_least_one_centre_before_creating_degrees);
     }

   /***** Free list of centres of the current institution *****/
   Ctr_FreeListCentres ();

   /***** Free list of degrees in the current centre *****/
   Deg_FreeListDegsOfCurrentCtr ();
  }

/*****************************************************************************/
/********** Create a list with all the degrees that have students ************/
/*****************************************************************************/

void Deg_GetListAllDegsWithStds (struct ListDegrees *Degs)
  {
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumDeg;

   /***** Get degrees admin by me from database *****/
   sprintf (Query,"SELECT DISTINCTROW degrees.DegCod,degrees.CtrCod,"
	          "degrees.DegTypCod,degrees.Status,degrees.RequesterUsrCod,"
                  "degrees.ShortName,degrees.FullName,degrees.WWW"
                  " FROM degrees,courses,crs_usr"
                  " WHERE degrees.DegCod=courses.DegCod"
                  " AND courses.CrsCod=crs_usr.CrsCod"
                  " AND crs_usr.Role='%u'"
                  " ORDER BY degrees.ShortName",
            (unsigned) Rol_STUDENT);
   Degs->Num = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get degrees admin by you");

   if (Degs->Num) // Degrees found...
     {
      /***** Create list with degrees *****/
      if ((Degs->Lst = (struct Degree *) calloc (Degs->Num,sizeof (struct Degree))) == NULL)
         Lay_ShowErrorAndExit ("Not enough memory to store degrees admin by you.");

      /***** Get the degrees *****/
      for (NumDeg = 0;
	   NumDeg < Degs->Num;
	   NumDeg++)
        {
         /* Get next degree */
         row = mysql_fetch_row (mysql_res);
         Deg_GetDataOfDegreeFromRow (&(Degs->Lst[NumDeg]),row);
        }
     }
   else
      Degs->Lst = NULL;

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/*********************** Free list of all the degrees ************************/
/*****************************************************************************/

void Deg_FreeListDegs (struct ListDegrees *Degs)
  {
   if (Degs->Lst)
     {
      free ((void *) Degs->Lst);
      Degs->Lst = NULL;
      Degs->Num = 0;
     }
  }

/*****************************************************************************/
/************ Get a list with the degrees of the current centre **************/
/*****************************************************************************/

static void Deg_GetListDegsOfCurrentCtr (void)
  {
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;
   unsigned NumDeg;

   /***** Get degrees of the current centre from database *****/
   sprintf (Query,"SELECT DegCod,CtrCod,DegTypCod,Status,RequesterUsrCod,"
                  "ShortName,FullName,WWW"
                  " FROM degrees WHERE CtrCod='%ld' ORDER BY FullName",
            Gbl.CurrentCtr.Ctr.CtrCod);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get degrees of a centre");

   /***** Count number of rows in result *****/
   if (NumRows) // Degrees found...
     {
      Gbl.CurrentCtr.Ctr.NumDegs = (unsigned) NumRows;

      /***** Create list with degrees of this centre *****/
      if ((Gbl.CurrentCtr.LstDegs = (struct Degree *) calloc (Gbl.CurrentCtr.Ctr.NumDegs,sizeof (struct Degree))) == NULL)
         Lay_ShowErrorAndExit ("Not enough memory to store degrees of a centre.");

      /***** Get the degrees of this centre *****/
      for (NumDeg = 0;
	   NumDeg < Gbl.CurrentCtr.Ctr.NumDegs;
	   NumDeg++)
        {
         /* Get next degree */
         row = mysql_fetch_row (mysql_res);
         Deg_GetDataOfDegreeFromRow (&(Gbl.CurrentCtr.LstDegs[NumDeg]),row);
        }
     }
   else	// Error: degrees should be found, but really they haven't be found. This never should happen.
      Gbl.CurrentCtr.Ctr.NumDegs = 0;

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/***************** Free list of degrees of the current centre ****************/
/*****************************************************************************/

static void Deg_FreeListDegsOfCurrentCtr (void)
  {
   if (Gbl.CurrentCtr.LstDegs)
     {
      free ((void *) Gbl.CurrentCtr.LstDegs);
      Gbl.CurrentCtr.LstDegs = NULL;
      Gbl.CurrentCtr.Ctr.NumDegs = 0;
     }
  }

/*****************************************************************************/
/*********** Create a list with the degrees administrated by me **************/
/*****************************************************************************/

void Deg_GetListDegsAdminByMe (void)
  {
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumDeg;

   /***** Set default list *****/
   Gbl.Usrs.Me.MyAdminDegs.Num = 0;
   Gbl.Usrs.Me.MyAdminDegs.Lst = NULL;

   /***** Get degrees admin by me from database *****/
   switch (Gbl.Usrs.Me.LoggedRole)
     {
      case Rol_CTR_ADM:
      case Rol_INS_ADM:
      case Rol_SYS_ADM:
	 sprintf (Query,"SELECT DegCod,CtrCod,DegTypCod,Status,RequesterUsrCod,"
			"ShortName,FullName,WWW"
			" FROM degrees"
			" WHERE CtrCod='%ld'"
			" ORDER BY ShortName",
		  Gbl.CurrentCtr.Ctr.CtrCod);
	 break;
      case Rol_DEG_ADM:
	 sprintf (Query,"SELECT degrees.DegCod,degrees.CtrCod,degrees.DegTypCod,degrees.Status,degrees.RequesterUsrCod,"
			"degrees.ShortName,degrees.FullName,degrees.WWW"
			" FROM admin,degrees"
			" WHERE admin.UsrCod='%ld' AND admin.Scope='Deg'"
			" AND admin.Cod=degrees.DegCod"
			" ORDER BY degrees.ShortName",
		  Gbl.Usrs.Me.UsrDat.UsrCod);
	 break;
      default:
	 /* I can not admin any degree */
	 return;
     }

   Gbl.Usrs.Me.MyAdminDegs.Num = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get degrees admin by you");

   /***** Count number of rows in result *****/
   if (Gbl.Usrs.Me.MyAdminDegs.Num) // Degrees found...
     {
      /***** Create list with degrees of this type *****/
      if ((Gbl.Usrs.Me.MyAdminDegs.Lst = (struct Degree *) calloc (Gbl.Usrs.Me.MyAdminDegs.Num,
                                                                   sizeof (struct Degree))) == NULL)
         Lay_ShowErrorAndExit ("Nout enough memory to store degrees admin by you.");

      /***** Get the degrees *****/
      for (NumDeg = 0;
	   NumDeg < Gbl.Usrs.Me.MyAdminDegs.Num;
	   NumDeg++)
        {
         /* Get next degree */
         row = mysql_fetch_row (mysql_res);
         Deg_GetDataOfDegreeFromRow (&(Gbl.Usrs.Me.MyAdminDegs.Lst[NumDeg]),row);
        }
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/***************** Free list of degrees administrated by me ******************/
/*****************************************************************************/

void Deg_FreeListMyAdminDegs (void)
  {
   if (Gbl.Usrs.Me.MyAdminDegs.Lst)
     {
      free ((void *) Gbl.Usrs.Me.MyAdminDegs.Lst);
      Gbl.Usrs.Me.MyAdminDegs.Lst = NULL;
      Gbl.Usrs.Me.MyAdminDegs.Num = 0;
     }
  }

/*****************************************************************************/
/****************** Receive form to request a new degree *********************/
/*****************************************************************************/

void Deg_RecFormReqDeg (void)
  {
   Deg_RecFormRequestOrCreateDeg ((unsigned) Deg_STATUS_BIT_PENDING);
  }

/*****************************************************************************/
/******************* Receive form to create a new degree *********************/
/*****************************************************************************/

void Deg_RecFormNewDeg (void)
  {
   Deg_RecFormRequestOrCreateDeg (0);
  }

/*****************************************************************************/
/******************* Receive form to create a new degree *********************/
/*****************************************************************************/

static void Deg_RecFormRequestOrCreateDeg (unsigned Status)
  {
   extern const char *Txt_The_degree_X_already_exists;
   extern const char *Txt_You_must_specify_the_web_address_of_the_new_degree;
   extern const char *Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_degree;
   struct Degree *Deg;

   Deg = &Gbl.Degs.EditingDeg;

   /***** Get parameters from form *****/
   /* Set degree centre */
   Deg->CtrCod = Gbl.CurrentCtr.Ctr.CtrCod;

   /* Get degree short name */
   Par_GetParToText ("ShortName",Deg->ShortName,Deg_MAX_LENGTH_DEGREE_SHORT_NAME);

   /* Get degree full name */
   Par_GetParToText ("FullName",Deg->FullName,Deg_MAX_LENGTH_DEGREE_FULL_NAME);

   /* Get degree type */
   if ((Deg->DegTypCod = DT_GetParamOtherDegTypCod ()) <= 0)
      Lay_ShowAlert (Lay_ERROR,"Wrong type of degree.");

   /* Get degree WWW */
   Par_GetParToText ("WWW",Deg->WWW,Cns_MAX_LENGTH_WWW);

   if (Deg->ShortName[0] && Deg->FullName[0])	// If there's a degree name
     {
      if (Deg->WWW[0])
	{
	 /***** If name of degree was in database... *****/
	 if (Deg_CheckIfDegreeNameExists (Deg->CtrCod,"ShortName",Deg->ShortName,-1L))
	   {
	    sprintf (Gbl.Message,Txt_The_degree_X_already_exists,
		     Deg->ShortName);
	    Lay_ShowAlert (Lay_WARNING,Gbl.Message);
	   }
	 else if (Deg_CheckIfDegreeNameExists (Deg->CtrCod,"FullName",Deg->FullName,-1L))
	   {
	    sprintf (Gbl.Message,Txt_The_degree_X_already_exists,
		     Deg->FullName);
	    Lay_ShowAlert (Lay_WARNING,Gbl.Message);
	   }
	 else	// Add new degree to database
	    Deg_CreateDegree (Deg,Status);
	}
      else	// If there is not a degree logo or web
	{
	 sprintf (Gbl.Message,"%s",Txt_You_must_specify_the_web_address_of_the_new_degree);
	 Lay_ShowAlert (Lay_WARNING,Gbl.Message);
	}
     }
   else	// If there is not a degree name
     {
      sprintf (Gbl.Message,"%s",Txt_You_must_specify_the_short_name_and_the_full_name_of_the_new_degree);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }

   /***** Show the form again *****/
   Deg_EditDegrees ();
  }

/*****************************************************************************/
/************************ Request removing of a degree ***********************/
/*****************************************************************************/

void Deg_RemoveDegree (void)
  {
   extern const char *Txt_To_remove_a_degree_you_must_first_remove_all_courses_in_the_degree;
   extern const char *Txt_Degree_X_removed;
   struct Degree Deg;

   /***** Get degree code *****/
   if ((Deg.DegCod = Deg_GetParamOtherDegCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of degree is missing.");

   /***** Get data of degree *****/
   Deg_GetDataOfDegreeByCod (&Deg);

   /***** Check if this degree has courses *****/
   if (Crs_GetNumCrssInDeg (Deg.DegCod))	// Degree has courses ==> don't remove
      Lay_ShowAlert (Lay_WARNING,Txt_To_remove_a_degree_you_must_first_remove_all_courses_in_the_degree);
   else	// Degree has no courses ==> remove it
     {
      /***** Remove degree *****/
      Deg_RemoveDegreeCompletely (Deg.DegCod);

      /***** Write message to show the change made *****/
      sprintf (Gbl.Message,Txt_Degree_X_removed,
               Deg.FullName);
      Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);
     }

   /***** Show the form again *****/
   Deg_EditDegrees ();
  }

/*****************************************************************************/
/******************** Write parameter with code of degree ********************/
/*****************************************************************************/

void Deg_PutParamDegCod (long DegCod)
  {
   Par_PutHiddenParamLong ("deg",DegCod);
  }

/*****************************************************************************/
/******************** Write parameter with code of degree ********************/
/*****************************************************************************/

static void Deg_PutParamOtherDegCod (long DegCod)
  {
   Par_PutHiddenParamLong ("OthDegCod",DegCod);
  }

/*****************************************************************************/
/********************* Get parameter with code of degree *********************/
/*****************************************************************************/

long Deg_GetParamOtherDegCod (void)
  {
   char LongStr[1+10+1];

   /***** Get parameter with code of degree *****/
   Par_GetParToText ("OthDegCod",LongStr,1+10);
   return Str_ConvertStrCodToLongCod (LongStr);
  }

/*****************************************************************************/
/********************* Get data of a degree from its code ********************/
/*****************************************************************************/
// Returns true if degree found

bool Deg_GetDataOfDegreeByCod (struct Degree *Deg)
  {
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRows;
   bool DegFound = false;

   if (Deg->DegCod <= 0)
     {
      Deg->DegCod = -1L;
      Deg->CtrCod = -1L;
      Deg->DegTypCod = -1L;
      Deg->Status = (Deg_Status_t) 0;
      Deg->RequesterUsrCod = -1L;
      Deg->ShortName[0] = '\0';
      Deg->FullName[0] = '\0';
      Deg->WWW[0] = '\0';
      Deg->LstCrss = NULL;
      return false;
     }

   /***** Get data of a degree from database *****/
   sprintf (Query,"SELECT DegCod,CtrCod,DegTypCod,Status,RequesterUsrCod,"
                  "ShortName,FullName,WWW"
                  " FROM degrees WHERE DegCod ='%ld'",
            Deg->DegCod);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get data of a degree");

   if (NumRows == 1)
     {
      /***** Get data of degree *****/
      row = mysql_fetch_row (mysql_res);
      Deg_GetDataOfDegreeFromRow (Deg,row);

      DegFound = true;
     }
   else if (NumRows == 0)
     {
      Deg->DegCod = -1L;
      Deg->CtrCod = -1L;
      Deg->DegTypCod = -1L;
      Deg->Status = (Deg_Status_t) 0;
      Deg->RequesterUsrCod = -1L;
      Deg->ShortName[0] = '\0';
      Deg->FullName[0] = '\0';
      Deg->WWW[0] = '\0';
      Deg->LstCrss = NULL;
      return false;
     }
   else if (NumRows > 1)
      Lay_ShowErrorAndExit ("Degree repeated in database.");

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);

   return DegFound;
  }

/*****************************************************************************/
/********** Get data of a degree from a row resulting of a query *************/
/*****************************************************************************/

static void Deg_GetDataOfDegreeFromRow (struct Degree *Deg,MYSQL_ROW row)
  {
   /***** Get degree code (row[0]) *****/
   if ((Deg->DegCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
      Lay_ShowErrorAndExit ("Wrong code of degree.");

   /***** Get centre code (row[1]) *****/
   Deg->CtrCod = Str_ConvertStrCodToLongCod (row[1]);

   /***** Get the code of the degree type (row[2]) *****/
   Deg->DegTypCod = Str_ConvertStrCodToLongCod (row[2]);

   /* Get course status (row[3]) */
   if (sscanf (row[3],"%u",&(Deg->Status)) != 1)
      Lay_ShowErrorAndExit ("Wrong degree status.");

   /* Get requester user's code (row[4]) */
   Deg->RequesterUsrCod = Str_ConvertStrCodToLongCod (row[4]);

   /***** Get degree short name (row[5]) *****/
   strcpy (Deg->ShortName,row[5]);

   /***** Get degree full name (row[6]) *****/
   strcpy (Deg->FullName,row[6]);

   /***** Get WWW (row[7]) *****/
   strcpy (Deg->WWW,row[7]);
  }

/*****************************************************************************/
/************* Get the short name of a degree from its code ******************/
/*****************************************************************************/

void Deg_GetShortNameOfDegreeByCod (struct Degree *Deg)
  {
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;

   Deg->ShortName[0] = '\0';
   if (Deg->DegCod > 0)
     {
      /***** Get the short name of a degree from database *****/
      sprintf (Query,"SELECT ShortName FROM degrees"
		     " WHERE DegCod ='%ld'",
	       Deg->DegCod);
      if (DB_QuerySELECT (Query,&mysql_res,"can not get the short name of a degree") == 1)
	{
	 /***** Get the short name of this degree *****/
	 row = mysql_fetch_row (mysql_res);
	 strcpy (Deg->ShortName,row[0]);
	}

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }
  }

/*****************************************************************************/
/************* Get the centre code of a degree from its code *****************/
/*****************************************************************************/

long Deg_GetCtrCodOfDegreeByCod (long DegCod)
  {
   char Query[128];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   long CtrCod = -1L;

   if (DegCod > 0)
     {
      /***** Get the centre code of a degree from database *****/
      sprintf (Query,"SELECT CtrCod FROM degrees WHERE DegCod ='%ld'",
	       DegCod);
      if (DB_QuerySELECT (Query,&mysql_res,"can not get the centre of a degree") == 1)
	{
	 /***** Get the centre code of this degree *****/
	 row = mysql_fetch_row (mysql_res);
	 CtrCod = Str_ConvertStrCodToLongCod (row[0]);
	}

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }

   return CtrCod;
  }

/*****************************************************************************/
/********** Get the institution code of a degree from its code ***************/
/*****************************************************************************/

long Deg_GetInsCodOfDegreeByCod (long DegCod)
  {
   char Query[256];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   long InsCod = -1L;

   if (DegCod > 0)
     {
      /***** Get the institution code of a degree from database *****/
      sprintf (Query,"SELECT centres.InsCod FROM degrees,centres"
		     " WHERE degrees.DegCod='%ld' AND degrees.CtrCod=centres.CtrCod",
	       DegCod);
      if (DB_QuerySELECT (Query,&mysql_res,"can not get the institution of a degree") == 1)
	{
	 /***** Get the institution code of this degree *****/
	 row = mysql_fetch_row (mysql_res);
	 InsCod = Str_ConvertStrCodToLongCod (row[0]);
	}

      /***** Free structure that stores the query result *****/
      DB_FreeMySQLResult (&mysql_res);
     }

   return InsCod;
  }

/*****************************************************************************/
/***************************** Remove a degree *******************************/
/*****************************************************************************/

void Deg_RemoveDegreeCompletely (long DegCod)
  {
   char Query[512];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned long NumRow,NumRows;
   long CrsCod;
   char PathDeg[PATH_MAX+1];

   /***** Get courses of a degree from database *****/
   sprintf (Query,"SELECT CrsCod FROM courses"
                  " WHERE DegCod='%ld'",DegCod);
   NumRows = DB_QuerySELECT (Query,&mysql_res,"can not get courses of a degree");

   /* Get courses in this degree */
   for (NumRow = 0;
	NumRow < NumRows;
	NumRow++)
     {
      /* Get next course */
      row = mysql_fetch_row (mysql_res);

      /* Get course code (row[0]) */
      if ((CrsCod = Str_ConvertStrCodToLongCod (row[0])) < 0)
         Lay_ShowErrorAndExit ("Wrong code of course.");

      /* Remove course */
      Crs_RemoveCourseCompletely (CrsCod);
     }

   /* Free structure that stores the query result */
   DB_FreeMySQLResult (&mysql_res);

   /***** Remove surveys of the degree (not including surveys of courses in degree, already removed) *****/
   Svy_RemoveDegSurveys (DegCod);

   /***** Remove all the threads and posts in degree forums *****/
   /* Remove disabled posts */
   sprintf (Query,"DELETE FROM forum_disabled_post"
	          " USING forum_thread,forum_post,forum_disabled_post"
                  " WHERE (forum_thread.ForumType='%u' OR forum_thread.ForumType='%u')"
                  " AND forum_thread.Location='%ld' AND forum_thread.ThrCod=forum_post.ThrCod AND forum_post.PstCod=forum_disabled_post.PstCod",
            For_FORUM_DEGREE_USRS,For_FORUM_DEGREE_TCHS,DegCod);
   DB_QueryDELETE (Query,"can not remove the disabled posts in forums of a degree");

   /* Remove posts */
   sprintf (Query,"DELETE FROM forum_post"
	          " USING forum_thread,forum_post"
                  " WHERE (forum_thread.ForumType='%u' OR forum_thread.ForumType='%u')"
                  " AND forum_thread.Location='%ld' AND forum_thread.ThrCod=forum_post.ThrCod",
            For_FORUM_DEGREE_USRS,For_FORUM_DEGREE_TCHS,DegCod);
   DB_QueryDELETE (Query,"can not remove posts in forums of a degree");

   /* Remove threads read */
   sprintf (Query,"DELETE FROM forum_thr_read"
	          " USING forum_thread,forum_thr_read"
                  " WHERE (forum_thread.ForumType='%u' OR forum_thread.ForumType='%u')"
                  " AND forum_thread.Location='%ld' AND forum_thread.ThrCod=forum_thr_read.ThrCod",
            For_FORUM_DEGREE_USRS,For_FORUM_DEGREE_TCHS,DegCod);
   DB_QueryDELETE (Query,"can not remove read threads in forums of a degree");

   /* Remove threads */
   sprintf (Query,"DELETE FROM forum_thread"
                  " WHERE (forum_thread.ForumType='%u' OR forum_thread.ForumType='%u')"
                  " AND Location='%ld'",
            For_FORUM_DEGREE_USRS,For_FORUM_DEGREE_TCHS,DegCod);
   DB_QueryDELETE (Query,"can not remove threads in forums of a degree");

   /***** Remove information related to files in degree *****/
   Brw_RemoveDegFilesFromDB (DegCod);

   /***** Remove directories of the degree *****/
   sprintf (PathDeg,"%s/%s/%02u/%u",
	    Cfg_PATH_SWAD_PUBLIC,Cfg_FOLDER_DEG,
	    (unsigned) (DegCod % 100),
	    (unsigned) DegCod);
   Fil_RemoveTree (PathDeg);

   /***** Remove administrators of this degree *****/
   sprintf (Query,"DELETE FROM admin WHERE Scope='Deg' AND Cod='%ld'",
            DegCod);
   DB_QueryDELETE (Query,"can not remove administrators of a degree");

   /***** Remove the degree *****/
   sprintf (Query,"DELETE FROM degrees WHERE DegCod='%ld'",
            DegCod);
   DB_QueryDELETE (Query,"can not remove a degree");

   /***** Delete all the degrees in sta_degrees table not present in degrees table *****/
   Pho_RemoveObsoleteStatDegrees ();
  }

/*****************************************************************************/
/********************* Change the short name of a degree *********************/
/*****************************************************************************/

void Deg_RenameDegreeShort (void)
  {
   struct Degree *Deg;

   Deg = &Gbl.Degs.EditingDeg;

   if (Deg_RenameDegree (Deg,Cns_SHORT_NAME))
      if (Deg->DegCod == Gbl.CurrentDeg.Deg.DegCod)	// If renaming current degree...
         strcpy (Gbl.CurrentDeg.Deg.ShortName,Deg->ShortName);	// Overwrite current degree name in order to show correctly in page title and heading
  }

/*****************************************************************************/
/********************* Change the full name of a degree **********************/
/*****************************************************************************/

void Deg_RenameDegreeFull (void)
  {
   struct Degree *Deg;

   Deg = &Gbl.Degs.EditingDeg;

   if (Deg_RenameDegree (Deg,Cns_FULL_NAME))
      if (Deg->DegCod == Gbl.CurrentDeg.Deg.DegCod)	// If renaming current degree...
         strcpy (Gbl.CurrentDeg.Deg.FullName,Deg->FullName);	// Overwrite current degree name in order to show correctly in page title and heading
  }

/*****************************************************************************/
/************************ Change the name of a degree ************************/
/*****************************************************************************/
// Returns true if the degree is renamed
// Returns false if the degree is not renamed

static bool Deg_RenameDegree (struct Degree *Deg,Cns_ShortOrFullName_t ShortOrFullName)
  {
   extern const char *Txt_You_can_not_leave_the_name_of_the_degree_X_empty;
   extern const char *Txt_The_degree_X_already_exists;
   extern const char *Txt_The_name_of_the_degree_X_has_changed_to_Y;
   extern const char *Txt_The_name_of_the_degree_X_has_not_changed;
   char Query[512];
   const char *ParamName = NULL;	// Initialized to avoid warning
   const char *FieldName = NULL;	// Initialized to avoid warning
   unsigned MaxLength = 0;		// Initialized to avoid warning
   char *CurrentDegName = NULL;		// Initialized to avoid warning
   char NewDegName[Deg_MAX_LENGTH_DEGREE_FULL_NAME+1];
   bool DegreeHasBeenRenamed = false;

   switch (ShortOrFullName)
     {
      case Cns_SHORT_NAME:
         ParamName = "ShortName";
         FieldName = "ShortName";
         MaxLength = Deg_MAX_LENGTH_DEGREE_SHORT_NAME;
         CurrentDegName = Deg->ShortName;
         break;
      case Cns_FULL_NAME:
         ParamName = "FullName";
         FieldName = "FullName";
         MaxLength = Deg_MAX_LENGTH_DEGREE_FULL_NAME;
         CurrentDegName = Deg->FullName;
         break;
     }

   /***** Get parameters from form *****/
   /* Get the code of the degree */
   if ((Deg->DegCod = Deg_GetParamOtherDegCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of degree is missing.");

   /* Get the new name for the degree */
   Par_GetParToText (ParamName,NewDegName,MaxLength);

   /***** Get data of degree *****/
   Deg_GetDataOfDegreeByCod (Deg);

   /***** Check if new name is empty *****/
   if (!NewDegName[0])
     {
      sprintf (Gbl.Message,Txt_You_can_not_leave_the_name_of_the_degree_X_empty,
               CurrentDegName);
      Gbl.Error = true;
     }
   else
     {
      /***** Check if old and new names are the same (this happens when user press enter with no changes in the form) *****/
      if (strcmp (CurrentDegName,NewDegName))	// Different names
        {
         /***** If degree was in database... *****/
         if (Deg_CheckIfDegreeNameExists (Deg->CtrCod,ParamName,NewDegName,Deg->DegCod))
           {
            sprintf (Gbl.Message,Txt_The_degree_X_already_exists,
                     NewDegName);
            Gbl.Error = true;
           }
         else
           {
            /* Update the table changing old name by new name */
            sprintf (Query,"UPDATE degrees SET %s='%s' WHERE DegCod='%ld'",
                     FieldName,NewDegName,Deg->DegCod);
            DB_QueryUPDATE (Query,"can not update the name of a degree");

            /* Write message to show the change made */
            sprintf (Gbl.Message,Txt_The_name_of_the_degree_X_has_changed_to_Y,
                     CurrentDegName,NewDegName);

	    /* Change current degree name in order to display it properly */
	    strncpy (CurrentDegName,NewDegName,MaxLength);
	    CurrentDegName[MaxLength] = '\0';

	    DegreeHasBeenRenamed = true;
           }
        }
      else	// The same name
         sprintf (Gbl.Message,Txt_The_name_of_the_degree_X_has_not_changed,
                  CurrentDegName);
     }

   return DegreeHasBeenRenamed;
  }

/*****************************************************************************/
/********************* Check if the name of degree exists ********************/
/*****************************************************************************/

static bool Deg_CheckIfDegreeNameExists (long CtrCod,const char *FieldName,const char *Name,long DegCod)
  {
   char Query[512];

   /***** Get number of degrees with a type and a name from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM degrees"
                  " WHERE CtrCod='%ld' AND %s='%s' AND DegCod<>'%ld'",
            CtrCod,FieldName,Name,DegCod);
   return (DB_QueryCOUNT (Query,"can not check if the name of a degree already existed") != 0);
  }

/*****************************************************************************/
/************************ Change the centre of a degree **********************/
/*****************************************************************************/

void Deg_ChangeDegreeCtrInConfig (void)
  {
   extern const char *Txt_The_degree_X_has_been_moved_to_the_centre_Y;
   struct Centre NewCtr;
   char Query[128];

   /***** Get parameters from form *****/
   /* Get parameter with centre code */
   NewCtr.CtrCod = Ctr_GetParamOtherCtrCod ();

   /***** Get data of new centre *****/
   Ctr_GetDataOfCentreByCod (&NewCtr);

   /***** Update centre in table of degrees *****/
   sprintf (Query,"UPDATE degrees SET CtrCod='%ld' WHERE DegCod='%ld'",
            NewCtr.CtrCod,Gbl.CurrentDeg.Deg.DegCod);
   DB_QueryUPDATE (Query,"can not update the centre of a degree");
   Gbl.CurrentDeg.Deg.CtrCod =
   Gbl.CurrentCtr.Ctr.CtrCod = NewCtr.CtrCod;

   /***** Initialize again current course, degree, centre... *****/
   Deg_InitCurrentCourse ();

   /***** Create message to show the change made *****/
   sprintf (Gbl.Message,Txt_The_degree_X_has_been_moved_to_the_centre_Y,
	    Gbl.CurrentDeg.Deg.FullName,
	    Gbl.CurrentCtr.Ctr.FullName);
  }

/*****************************************************************************/
/** Show message of success after changing a course in course configuration **/
/*****************************************************************************/

void Deg_ContEditAfterChgDegInConfig (void)
  {
   /***** Write error/success message *****/
   Lay_ShowAlert (Gbl.Error ? Lay_WARNING :
			      Lay_SUCCESS,
		  Gbl.Message);

   /***** Show the form again *****/
   Deg_ShowConfiguration ();
  }

/*****************************************************************************/
/************************ Change the centre of a degree **********************/
/*****************************************************************************/

void Deg_ChangeDegreeCtr (void)
  {
   extern const char *Txt_The_degree_X_has_been_moved_to_the_centre_Y;
   struct Degree *Deg;
   struct Centre NewCtr;
   char Query[128];

   Deg = &Gbl.Degs.EditingDeg;

   /***** Get parameters from form *****/
   /* Get degree code */
   if ((Deg->DegCod = Deg_GetParamOtherDegCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of degree is missing.");

   /* Get parameter with centre code */
   NewCtr.CtrCod = Ctr_GetParamOtherCtrCod ();

   /***** Get data of degree and new centre *****/
   Deg_GetDataOfDegreeByCod (Deg);
   Ctr_GetDataOfCentreByCod (&NewCtr);

   /***** Update centre in table of degrees *****/
   sprintf (Query,"UPDATE degrees SET CtrCod='%ld' WHERE DegCod='%ld'",
            NewCtr.CtrCod,Deg->DegCod);
   DB_QueryUPDATE (Query,"can not update the centre of a degree");
   Deg->CtrCod = NewCtr.CtrCod;

   /***** Write message to show the change made *****/
   sprintf (Gbl.Message,Txt_The_degree_X_has_been_moved_to_the_centre_Y,
	    Deg->FullName,NewCtr.FullName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);

   /***** Put button to go to degree changed *****/
   Deg_PutButtonToGoToDeg (Deg);

   /***** Show the form again *****/
   Deg_EditDegrees ();
  }

/*****************************************************************************/
/************************* Change the WWW of a degree ************************/
/*****************************************************************************/

void Deg_ChangeDegWWW (void)
  {
   extern const char *Txt_The_new_web_address_is_X;
   extern const char *Txt_You_can_not_leave_the_web_address_empty;
   struct Degree *Deg;
   char Query[256+Cns_MAX_LENGTH_WWW];
   char NewWWW[Cns_MAX_LENGTH_WWW+1];

   Deg = &Gbl.Degs.EditingDeg;

   /***** Get parameters from form *****/
   /* Get the code of the degree */
   if ((Deg->DegCod = Deg_GetParamOtherDegCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of degree is missing.");

   /* Get the new WWW for the degree */
   Par_GetParToText ("WWW",NewWWW,Cns_MAX_LENGTH_WWW);

   /***** Get data of degree *****/
   Deg_GetDataOfDegreeByCod (Deg);

   /***** Check if new WWW is empty *****/
   if (NewWWW[0])
     {
      /***** Update the table changing old WWW by new WWW *****/
      sprintf (Query,"UPDATE degrees SET WWW='%s' WHERE DegCod='%ld'",
               NewWWW,Deg->DegCod);
      DB_QueryUPDATE (Query,"can not update the web of a degree");

      /***** Write message to show the change made *****/
      sprintf (Gbl.Message,Txt_The_new_web_address_is_X,
               NewWWW);
      Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);

      /***** Put button to go to degree changed *****/
      Deg_PutButtonToGoToDeg (Deg);
     }
   else
     {
      sprintf (Gbl.Message,"%s",Txt_You_can_not_leave_the_web_address_empty);
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
     }

   /***** Show the form again *****/
   strcpy (Deg->WWW,NewWWW);
   Deg_EditDegrees ();
  }

/*****************************************************************************/
/*********************** Change the status of a degree ***********************/
/*****************************************************************************/

void Deg_ChangeDegStatus (void)
  {
   extern const char *Txt_The_status_of_the_degree_X_has_changed;
   struct Degree *Deg;
   char Query[256];
   char UnsignedNum[10+1];
   Deg_Status_t Status;
   Deg_StatusTxt_t StatusTxt;

   Deg = &Gbl.Degs.EditingDeg;

   /***** Get parameters from form *****/
   /* Get degree code */
   if ((Deg->DegCod = Deg_GetParamOtherDegCod ()) == -1L)
      Lay_ShowErrorAndExit ("Code of degree is missing.");

   /* Get parameter with status */
   Par_GetParToText ("Status",UnsignedNum,1);
   if (sscanf (UnsignedNum,"%u",&Status) != 1)
      Lay_ShowErrorAndExit ("Wrong status.");
   StatusTxt = Deg_GetStatusTxtFromStatusBits (Status);
   Status = Deg_GetStatusBitsFromStatusTxt (StatusTxt);	// New status

   /***** Get data of degree *****/
   Deg_GetDataOfDegreeByCod (Deg);

   /***** Update status in table of degrees *****/
   sprintf (Query,"UPDATE degrees SET Status='%u' WHERE DegCod='%ld'",
            (unsigned) Status,Deg->DegCod);
   DB_QueryUPDATE (Query,"can not update the status of a degree");

   Deg->Status = Status;

   /***** Write message to show the change made *****/
   sprintf (Gbl.Message,Txt_The_status_of_the_degree_X_has_changed,
            Deg->ShortName);
   Lay_ShowAlert (Lay_SUCCESS,Gbl.Message);

   /***** Put button to go to degree changed *****/
   Deg_PutButtonToGoToDeg (Deg);

   /***** Show the form again *****/
   Deg_EditDegrees ();
  }

/*****************************************************************************/
/************* Show message of success after changing a degree ***************/
/*****************************************************************************/

void Deg_ContEditAfterChgDeg (void)
  {
   if (Gbl.Error)
      /***** Write error message *****/
      Lay_ShowAlert (Lay_WARNING,Gbl.Message);
   else
     {
      /***** Write success message showing the change made *****/
      Lay_ShowAlert (Lay_INFO,Gbl.Message);

      /***** Put button to go to degree changed *****/
      Deg_PutButtonToGoToDeg (&Gbl.Degs.EditingDeg);
     }

   /***** Show the form again *****/
   Deg_EditDegrees ();
  }

/*****************************************************************************/
/************************ Put button to go to degree *************************/
/*****************************************************************************/

void Deg_PutButtonToGoToDeg (struct Degree *Deg)
  {
   extern const char *Txt_Go_to_X;

   // If the degree is different to the current one...
   if (Deg->DegCod != Gbl.CurrentDeg.Deg.DegCod)
     {
      fprintf (Gbl.F.Out,"<div class=\"BUTTONS_AFTER_ALERT\">");
      Act_FormStart (ActSeeCrs);
      Deg_PutParamDegCod (Deg->DegCod);
      sprintf (Gbl.Title,Txt_Go_to_X,Deg->ShortName);
      Lay_PutConfirmButtonInline (Gbl.Title);
      Act_FormEnd ();
      fprintf (Gbl.F.Out,"</div>");
     }
  }

/*****************************************************************************/
/*********** Show a form for sending a logo of the current degree ************/
/*****************************************************************************/

void Deg_RequestLogo (void)
  {
   Log_RequestLogo (Sco_SCOPE_DEG);
  }

/*****************************************************************************/
/***************** Receive the logo of the current degree ********************/
/*****************************************************************************/

void Deg_ReceiveLogo (void)
  {
   Log_ReceiveLogo (Sco_SCOPE_DEG);
  }

/*****************************************************************************/
/****************** Remove the logo of the current degree ********************/
/*****************************************************************************/

void Deg_RemoveLogo (void)
  {
   Log_RemoveLogo (Sco_SCOPE_DEG);
  }

/*****************************************************************************/
/*********************** Get total number of degrees *************************/
/*****************************************************************************/

unsigned Deg_GetNumDegsTotal (void)
  {
   char Query[256];

   /***** Get total number of degrees from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM degrees");
   return (unsigned) DB_QueryCOUNT (Query,"can not get the total number of degrees");
  }

/*****************************************************************************/
/********************* Get number of degrees in a country ********************/
/*****************************************************************************/

unsigned Deg_GetNumDegsInCty (long InsCod)
  {
   char Query[512];

   /***** Get number of degrees in a country from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM institutions,centres,degrees"
	          " WHERE institutions.CtyCod='%ld'"
	          " AND institutions.InsCod=centres.InsCod"
	          " AND centres.CtrCod=degrees.CtrCod",
	    InsCod);
   return (unsigned) DB_QueryCOUNT (Query,"can not get the number of degrees in a country");
  }

/*****************************************************************************/
/****************** Get number of degrees in an institution ******************/
/*****************************************************************************/

unsigned Deg_GetNumDegsInIns (long InsCod)
  {
   char Query[512];

   /***** Get number of degrees in an institution from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM centres,degrees"
	          " WHERE centres.InsCod='%ld'"
	          " AND centres.CtrCod=degrees.CtrCod",
	    InsCod);
   return (unsigned) DB_QueryCOUNT (Query,"can not get the number of degrees in an institution");
  }

/*****************************************************************************/
/******************** Get number of degrees in a centre **********************/
/*****************************************************************************/

unsigned Deg_GetNumDegsInCtr (long CtrCod)
  {
   char Query[512];

   /***** Get number of degrees in a centre from database *****/
   sprintf (Query,"SELECT COUNT(*) FROM degrees"
	          " WHERE CtrCod='%ld'",
	    CtrCod);
   return (unsigned) DB_QueryCOUNT (Query,"can not get the number of degrees in a centre");
  }

/*****************************************************************************/
/********************* Get number of centres with courses ********************/
/*****************************************************************************/

unsigned Deg_GetNumDegsWithCrss (const char *SubQuery)
  {
   char Query[512];

   /***** Get number of degrees with courses from database *****/
   sprintf (Query,"SELECT COUNT(DISTINCT degrees.DegCod)"
                  " FROM institutions,centres,degrees,courses"
                  " WHERE %sinstitutions.InsCod=centres.InsCod"
                  " AND centres.CtrCod=degrees.CtrCod"
                  " AND degrees.DegCod=courses.DegCod",
            SubQuery);
   return (unsigned) DB_QueryCOUNT (Query,"can not get number of degrees with courses");
  }

/*****************************************************************************/
/********************* Get number of degrees with users **********************/
/*****************************************************************************/

unsigned Deg_GetNumDegsWithUsrs (Rol_Role_t Role,const char *SubQuery)
  {
   char Query[512];

   /***** Get number of degrees with users from database *****/
   sprintf (Query,"SELECT COUNT(DISTINCT degrees.DegCod)"
                  " FROM institutions,centres,degrees,courses,crs_usr"
                  " WHERE %sinstitutions.InsCod=centres.InsCod"
                  " AND centres.CtrCod=degrees.CtrCod"
                  " AND degrees.DegCod=courses.DegCod"
                  " AND courses.CrsCod=crs_usr.CrsCod"
                  " AND crs_usr.Role='%u'",
            SubQuery,(unsigned) Role);
   return (unsigned) DB_QueryCOUNT (Query,"can not get number of degrees with users");
  }

/*****************************************************************************/
/***** Write institutions, centres and degrees administrated by an admin *****/
/*****************************************************************************/

void Deg_GetAndWriteInsCtrDegAdminBy (long UsrCod,unsigned ColSpan)
  {
   extern const char *Txt_all_degrees;
   char Query[1024];
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumRow;
   unsigned NumRows;
   struct Institution Ins;
   struct Centre Ctr;
   struct Degree Deg;

   /***** Get institutions, centres, degrees admin by user from database *****/
   sprintf (Query,"(SELECT '%u' AS S,'-1' AS Cod,'' AS FullName"
	          " FROM admin"
	          " WHERE UsrCod='%ld'"
	          " AND Scope='Sys')"
                  " UNION "
                  "(SELECT '%u' AS S,admin.Cod,institutions.FullName"
                  " FROM admin,institutions"
                  " WHERE admin.UsrCod='%ld'"
                  " AND admin.Scope='Ins'"
                  " AND admin.Cod=institutions.InsCod)"
                  " UNION "
                  "(SELECT '%u' AS S,admin.Cod,centres.FullName"
                  " FROM admin,centres"
                  " WHERE admin.UsrCod='%ld'"
                  " AND admin.Scope='Ctr'"
                  " AND admin.Cod=centres.CtrCod)"
                  " UNION "
                  "(SELECT '%u' AS S,admin.Cod,degrees.FullName"
                  " FROM admin,degrees"
                  " WHERE admin.UsrCod='%ld'"
                  " AND admin.Scope='Deg'"
                  " AND admin.Cod=degrees.DegCod)"
                  " ORDER BY S,FullName",
            (unsigned) Sco_SCOPE_SYS,UsrCod,
            (unsigned) Sco_SCOPE_INS,UsrCod,
            (unsigned) Sco_SCOPE_CTR,UsrCod,
            (unsigned) Sco_SCOPE_DEG,UsrCod);
   if ((NumRows = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get institutions, centres, degrees admin by a user")))
      /***** Get the list of degrees *****/
      for (NumRow = 1;
	   NumRow <= NumRows;
	   NumRow++)
	{
         /***** Indent *****/
         fprintf (Gbl.F.Out,"<tr>"
                            "<td class=\"RIGHT_TOP COLOR%u\">"
                            "<img src=\"%s/%s20x20.gif\""
                            " alt=\"\" title=\"\""
                            " class=\"ICON25x25\" />"
                            "</td>",
                  Gbl.RowEvenOdd,Gbl.Prefs.IconsURL,
                  NumRow == NumRows ? "subend" :
                	              "submid");

         /***** Write institution, centre, degree *****/
         fprintf (Gbl.F.Out,"<td colspan=\"%u\""
                            " class=\"DAT_SMALL_NOBR LEFT_TOP COLOR%u\">",
                  ColSpan - 1,Gbl.RowEvenOdd);

         /* Get next institution, centre, degree */
         row = mysql_fetch_row (mysql_res);

	 /* Get scope */
	 switch (Sco_GetScopeFromUnsignedStr (row[0]))
	   {
	    case Sco_SCOPE_SYS:	// System
	       fprintf (Gbl.F.Out,"<img src=\"%s/swad64x64.gif\""
        	                  " alt=\"%s\" title=\"%s\""
                                  " class=\"ICON20x20\" />"
                                  "&nbsp;%s",
                     Gbl.Prefs.IconsURL,
                     Txt_all_degrees,
                     Txt_all_degrees,
                     Txt_all_degrees);
	       break;
	    case Sco_SCOPE_INS:	// Institution
	       Ins.InsCod = Str_ConvertStrCodToLongCod (row[1]);
	       if (Ins.InsCod > 0)
		 {
		  /* Get data of institution */
		  Ins_GetDataOfInstitutionByCod (&Ins,Ins_GET_BASIC_DATA);

		  /* Write institution logo and name */
		  Ins_DrawInstitutionLogoAndNameWithLink (&Ins,ActSeeInsInf,
						          "DAT_SMALL_NOBR","LEFT_TOP");
		 }
	       break;
	    case Sco_SCOPE_CTR:	// Centre
	       Ctr.CtrCod = Str_ConvertStrCodToLongCod (row[1]);
	       if (Ctr.CtrCod > 0)
		 {
		  /* Get data of centre */
		  Ctr_GetDataOfCentreByCod (&Ctr);

		  /* Write centre logo and name */
		  Ctr_DrawCentreLogoAndNameWithLink (&Ctr,ActSeeCtrInf,
						     "DAT_SMALL_NOBR","LEFT_TOP");
		 }
	       break;
	    case Sco_SCOPE_DEG:	// Degree
	       Deg.DegCod = Str_ConvertStrCodToLongCod (row[1]);
	       if (Deg.DegCod > 0)
		 {
		  /* Get data of degree */
		  Deg_GetDataOfDegreeByCod (&Deg);

		  /* Write degree logo and name */
		  Deg_DrawDegreeLogoAndNameWithLink (&Deg,ActSeeDegInf,
						     "DAT_SMALL_NOBR","LEFT_TOP");
		 }
	       break;
	    default:	// There are no administrators in other scopes
	       Lay_ShowErrorAndExit ("Wrong scope.");
	       break;
           }
         fprintf (Gbl.F.Out,"</td>"
                            "</tr>");
        }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/****************************** List degrees found ***************************/
/*****************************************************************************/
// Returns number of degrees found

unsigned Deg_ListDegsFound (const char *Query)
  {
   extern const char *Txt_degree;
   extern const char *Txt_degrees;
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumDegs;
   unsigned NumDeg;
   struct Degree Deg;

   /***** Query database *****/
   if ((NumDegs = (unsigned) DB_QuerySELECT (Query,&mysql_res,"can not get degrees")))
     {
      /***** Write heading *****/
      /* Number of degrees found */
      sprintf (Gbl.Title,"%u %s",
               NumDegs,(NumDegs == 1) ? Txt_degree :
        	                        Txt_degrees);
      Lay_StartRoundFrameTable (NULL,2,Gbl.Title);
      Deg_PutHeadDegreesForSeeing ();

      /***** List the degrees (one row per degree) *****/
      for (NumDeg = 1;
	   NumDeg <= NumDegs;
	   NumDeg++)
	{
	 /* Get next degree */
	 row = mysql_fetch_row (mysql_res);

	 /* Get degree code (row[0]) */
	 Deg.DegCod = Str_ConvertStrCodToLongCod (row[0]);

	 /* Get data of degree */
	 Deg_GetDataOfDegreeByCod (&Deg);

	 /* Write data of this degree */
	 Deg_ListOneDegreeForSeeing (&Deg,NumDeg);
	}

      /***** End table *****/
      Lay_EndRoundFrameTable ();
     }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);

   return NumDegs;
  }
