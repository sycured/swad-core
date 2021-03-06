// swad_hierarchy.c: hierarchy (system, institution, center, degree, course)

/*
    SWAD (Shared Workspace At a Distance),
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
/********************************* Headers ***********************************/
/*****************************************************************************/

#define _GNU_SOURCE 		// For asprintf
#include <stdio.h>		// For asprintf
#include <stdlib.h>		// For free

#include "swad_database.h"
#include "swad_error.h"
#include "swad_form.h"
#include "swad_global.h"
#include "swad_hierarchy.h"
#include "swad_HTML.h"
#include "swad_logo.h"

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

/*****************************************************************************/
/*************************** Private constants *******************************/
/*****************************************************************************/

/*****************************************************************************/
/*************************** Private prototypes ******************************/
/*****************************************************************************/

/*****************************************************************************/
/********** List pending institutions, centers, degrees and courses **********/
/*****************************************************************************/

void Hie_SeePending (void)
  {
   /***** List countries with pending institutions *****/
   Cty_SeeCtyWithPendingInss ();

   /***** List institutions with pending centers *****/
   Ins_SeeInsWithPendingCtrs ();

   /***** List centers with pending degrees *****/
   Ctr_SeeCtrWithPendingDegs ();

   /***** List degrees with pending courses *****/
   Deg_SeeDegWithPendingCrss ();
  }

/*****************************************************************************/
/*** Write menu to select country, institution, center, degree and course ****/
/*****************************************************************************/

void Hie_WriteMenuHierarchy (void)
  {
   extern const char *Txt_Country;
   extern const char *Txt_Institution;
   extern const char *Txt_Center;
   extern const char *Txt_Degree;
   extern const char *Txt_Course;

   /***** Begin table *****/
   HTM_TABLE_BeginCenterPadding (2);

   /***** Write a 1st selector
          with all the countries *****/
   HTM_TR_Begin (NULL);

   /* Label */
   Frm_LabelColumn ("RT","cty",Txt_Country);

   /* Data */
   HTM_TD_Begin ("class=\"LT\"");
   Cty_WriteSelectorOfCountry ();
   HTM_TD_End ();

   HTM_TR_End ();

   if (Gbl.Hierarchy.Cty.CtyCod > 0)
     {
      /***** Write a 2nd selector
             with the institutions of selected country *****/
      HTM_TR_Begin (NULL);

      /* Label */
      Frm_LabelColumn ("RT","ins",Txt_Institution);

      /* Data */
      HTM_TD_Begin ("class=\"LT\"");
      Ins_WriteSelectorOfInstitution ();
      HTM_TD_End ();

      HTM_TR_End ();

      if (Gbl.Hierarchy.Ins.InsCod > 0)
        {
         /***** Write a 3rd selector
                with all the centers of selected institution *****/
         HTM_TR_Begin (NULL);

         /* Label */
         Frm_LabelColumn ("RT","ctr",Txt_Center);

         /* Data */
         HTM_TD_Begin ("class=\"LT\"");
         Ctr_WriteSelectorOfCenter ();
         HTM_TD_End ();

         HTM_TR_End ();

         if (Gbl.Hierarchy.Ctr.CtrCod > 0)
           {
            /***** Write a 4th selector
                   with all the degrees of selected center *****/
            HTM_TR_Begin (NULL);

            /* Label */
            Frm_LabelColumn ("RT","deg",Txt_Degree);

            /* Data */
            HTM_TD_Begin ("class=\"LT\"");
            Deg_WriteSelectorOfDegree ();
            HTM_TD_End ();

            HTM_TR_End ();

	    if (Gbl.Hierarchy.Deg.DegCod > 0)
	      {
	       /***** Write a 5th selector
		      with all the courses of selected degree *****/
	       HTM_TR_Begin (NULL);

	       /* Label */
               Frm_LabelColumn ("RT","crs",Txt_Course);

               /* Data */
	       HTM_TD_Begin ("class=\"LT\"");
	       Crs_WriteSelectorOfCourse ();
	       HTM_TD_End ();

	       HTM_TR_End ();
	      }
           }
        }
     }

   /***** End table *****/
   HTM_TABLE_End ();
  }

/*****************************************************************************/
/************* Write hierarchy breadcrumb in the top of the page *************/
/*****************************************************************************/

void Hie_WriteHierarchyInBreadcrumb (void)
  {
   extern const char *The_ClassBreadcrumb[The_NUM_THEMES];
   extern const char *Txt_System;
   extern const char *Txt_Country;
   extern const char *Txt_Institution;
   extern const char *Txt_Center;
   extern const char *Txt_Degree;
   const char *ClassTxt = The_ClassBreadcrumb[Gbl.Prefs.Theme];
   char *ClassLink;

   /***** Create CSS class of links *****/
   if (asprintf (&ClassLink,"BT_LINK %s",ClassTxt) < 0)
      Err_NotEnoughMemoryExit ();

   /***** Form to go to the system *****/
   HTM_DIV_Begin ("class=\"BC %s\"",ClassTxt);
   HTM_NBSP ();

   Frm_BeginFormGoTo (ActMnu);
   Par_PutHiddenParamUnsigned (NULL,"NxtTab",(unsigned) TabSys);
   HTM_BUTTON_SUBMIT_Begin (Txt_System,ClassLink,NULL);
   HTM_Txt (Txt_System);
   HTM_BUTTON_End ();
   Frm_EndForm ();

   HTM_DIV_End ();

   if (Gbl.Hierarchy.Cty.CtyCod > 0)		// Country selected...
     {
      HTM_DIV_Begin ("class=\"BC %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to go to see institutions of this country *****/
      Frm_BeginFormGoTo (ActSeeIns);
      Cty_PutParamCtyCod (Gbl.Hierarchy.Cty.CtyCod);
      HTM_BUTTON_SUBMIT_Begin (Gbl.Hierarchy.Cty.Name[Gbl.Prefs.Language],ClassLink,NULL);
      HTM_Txt (Gbl.Hierarchy.Cty.Name[Gbl.Prefs.Language]);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }
   else
     {
      HTM_DIV_Begin ("class=\"BC BC_SEMIOFF %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to go to select countries *****/
      Frm_BeginFormGoTo (ActSeeCty);
      HTM_BUTTON_SUBMIT_Begin (Txt_Country,ClassLink,NULL);
      HTM_Txt (Txt_Country);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }

   if (Gbl.Hierarchy.Ins.InsCod > 0)		// Institution selected...
     {
      HTM_DIV_Begin ("class=\"BC %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to see centers of this institution *****/
      Frm_BeginFormGoTo (ActSeeCtr);
      Ins_PutParamInsCod (Gbl.Hierarchy.Ins.InsCod);
      HTM_BUTTON_SUBMIT_Begin (Gbl.Hierarchy.Ins.FullName,ClassLink,NULL);
      HTM_Txt (Gbl.Hierarchy.Ins.ShrtName);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }
   else if (Gbl.Hierarchy.Cty.CtyCod > 0)
     {
      HTM_DIV_Begin ("class=\"BC BC_SEMIOFF %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to go to select institutions *****/
      Frm_BeginFormGoTo (ActSeeIns);
      HTM_BUTTON_SUBMIT_Begin (Txt_Institution,ClassLink,NULL);
      HTM_Txt (Txt_Institution);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }
   else
     {
      HTM_DIV_Begin ("class=\"BC BC_OFF %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Hidden institution *****/
      HTM_Txt (Txt_Institution);

      HTM_DIV_End ();
     }

   if (Gbl.Hierarchy.Ctr.CtrCod > 0)	// Center selected...
     {
      HTM_DIV_Begin ("class=\"BC %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to see degrees of this center *****/
      Frm_BeginFormGoTo (ActSeeDeg);
      Ctr_PutParamCtrCod (Gbl.Hierarchy.Ctr.CtrCod);
      HTM_BUTTON_SUBMIT_Begin (Gbl.Hierarchy.Ctr.FullName,ClassLink,NULL);
      HTM_Txt (Gbl.Hierarchy.Ctr.ShrtName);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }
   else if (Gbl.Hierarchy.Ins.InsCod > 0)
     {
      HTM_DIV_Begin ("class=\"BC BC_SEMIOFF %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to go to select centers *****/
      Frm_BeginFormGoTo (ActSeeCtr);
      HTM_BUTTON_SUBMIT_Begin (Txt_Center,ClassLink,NULL);
      HTM_Txt (Txt_Center);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }
   else
     {
      HTM_DIV_Begin ("class=\"BC BC_OFF %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Hidden center *****/
      HTM_Txt (Txt_Center);

      HTM_DIV_End ();
     }

   if (Gbl.Hierarchy.Deg.DegCod > 0)	// Degree selected...
     {
      HTM_DIV_Begin ("class=\"BC %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to go to see courses of this degree *****/
      Frm_BeginFormGoTo (ActSeeCrs);
      Deg_PutParamDegCod (Gbl.Hierarchy.Deg.DegCod);
      HTM_BUTTON_SUBMIT_Begin (Gbl.Hierarchy.Deg.FullName,ClassLink,NULL);
      HTM_Txt (Gbl.Hierarchy.Deg.ShrtName);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }
   else if (Gbl.Hierarchy.Ctr.CtrCod > 0)
     {
      HTM_DIV_Begin ("class=\"BC BC_SEMIOFF %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Form to go to select degrees *****/
      Frm_BeginFormGoTo (ActSeeDeg);
      HTM_BUTTON_SUBMIT_Begin (Txt_Degree,ClassLink,NULL);
      HTM_Txt (Txt_Degree);
      HTM_BUTTON_End ();
      Frm_EndForm ();

      HTM_DIV_End ();
     }
   else
     {
      HTM_DIV_Begin ("class=\"BC BC_OFF %s\"",ClassTxt);

      /***** Separator *****/
      HTM_Txt ("&nbsp;&gt;&nbsp;");

      /***** Hidden degree *****/
      HTM_Txt (Txt_Degree);

      HTM_DIV_End ();
     }

   HTM_DIV_Begin ("class=\"BC%s %s\"",
		   (Gbl.Hierarchy.Level == Hie_Lvl_CRS) ? "" :
		  ((Gbl.Hierarchy.Deg.DegCod > 0) ? " BC_SEMIOFF" :
						    " BC_OFF"),
		  ClassTxt);

   /***** Separator *****/
   HTM_Txt ("&nbsp;&gt;&nbsp;");

   HTM_DIV_End ();

   /***** Free memory used for CSS class of links *****/
   free (ClassLink);
  }

/*****************************************************************************/
/*************** Write course full name in the top of the page ***************/
/*****************************************************************************/

void Hie_WriteBigNameCtyInsCtrDegCrs (void)
  {
   extern const char *The_ClassCourse[The_NUM_THEMES];
   extern const char *Txt_TAGLINE;

   HTM_TxtF ("<h1 id=\"main_title\" class=\"%s\">",
	     The_ClassCourse[Gbl.Prefs.Theme]);

   /***** Logo *****/
   switch (Gbl.Hierarchy.Level)
     {
      case Hie_Lvl_SYS:	// System
	 Ico_PutIcon ("swad64x64.png",Cfg_PLATFORM_FULL_NAME,"ICO40x40 TOP_LOGO");
         break;
      case Hie_Lvl_CTY:	// Country
         Cty_DrawCountryMap (&Gbl.Hierarchy.Cty,"COUNTRY_MAP_TITLE");
         break;
      case Hie_Lvl_INS:	// Institution
	 Lgo_DrawLogo (Hie_Lvl_INS,Gbl.Hierarchy.Ins.InsCod,
		       Gbl.Hierarchy.Ins.ShrtName,40,"TOP_LOGO",false);
         break;
      case Hie_Lvl_CTR:	// Center
	 Lgo_DrawLogo (Hie_Lvl_CTR,Gbl.Hierarchy.Ctr.CtrCod,
		       Gbl.Hierarchy.Ctr.ShrtName,40,"TOP_LOGO",false);
         break;
      case Hie_Lvl_DEG:	// Degree
      case Hie_Lvl_CRS:	// Course
	 Lgo_DrawLogo (Hie_Lvl_DEG,Gbl.Hierarchy.Deg.DegCod,
		       Gbl.Hierarchy.Deg.ShrtName,40,"TOP_LOGO",false);
         break;
      default:
	 break;
     }

   /***** Text *****/
   HTM_DIV_Begin ("id=\"big_name_container\"");
   if (Gbl.Hierarchy.Cty.CtyCod > 0)
     {
      HTM_DIV_Begin ("id=\"big_full_name\"");
      HTM_Txt (	(Gbl.Hierarchy.Level == Hie_Lvl_CRS) ? Gbl.Hierarchy.Crs.FullName :// Full name
	       ((Gbl.Hierarchy.Level == Hie_Lvl_DEG) ? Gbl.Hierarchy.Deg.FullName :
	       ((Gbl.Hierarchy.Level == Hie_Lvl_CTR) ? Gbl.Hierarchy.Ctr.FullName :
	       ((Gbl.Hierarchy.Level == Hie_Lvl_INS) ? Gbl.Hierarchy.Ins.FullName :
	                                           Gbl.Hierarchy.Cty.Name[Gbl.Prefs.Language]))));
      HTM_DIV_End ();

      HTM_DIV_Begin ("class=\"NOT_SHOWN\"");
      HTM_Txt (" / ");	// To separate
      HTM_DIV_End ();

      HTM_DIV_Begin ("id=\"big_short_name\"");
      HTM_Txt (	(Gbl.Hierarchy.Level == Hie_Lvl_CRS) ? Gbl.Hierarchy.Crs.ShrtName :// Short name
	       ((Gbl.Hierarchy.Level == Hie_Lvl_DEG) ? Gbl.Hierarchy.Deg.ShrtName :
	       ((Gbl.Hierarchy.Level == Hie_Lvl_CTR) ? Gbl.Hierarchy.Ctr.ShrtName :
	       ((Gbl.Hierarchy.Level == Hie_Lvl_INS) ? Gbl.Hierarchy.Ins.ShrtName :
	                                           Gbl.Hierarchy.Cty.Name[Gbl.Prefs.Language]))));
      HTM_DIV_End ();
     }
   else	// No country specified ==> home page
     {
      HTM_DIV_Begin ("id=\"big_full_name\"");	// Full name
      HTM_TxtF ("%s:&nbsp;%s",Cfg_PLATFORM_SHORT_NAME,Txt_TAGLINE);
      HTM_DIV_End ();

      HTM_DIV_Begin ("class=\"NOT_SHOWN\"");
      HTM_Txt (" / ");	// To separate
      HTM_DIV_End ();

      HTM_DIV_Begin ("id=\"big_short_name\"");	// Short name
      HTM_Txt (Cfg_PLATFORM_SHORT_NAME);
      HTM_DIV_End ();
     }
   HTM_DIV_End ();
   HTM_TxtF ("</h1>");
  }

/*****************************************************************************/
/**************** Copy last hierarchy to current hierarchy *******************/
/*****************************************************************************/

void Hie_SetHierarchyFromUsrLastHierarchy (void)
  {
   /***** Initialize all codes to -1 *****/
   Hie_ResetHierarchy ();

   /***** Copy last hierarchy scope and code to current hierarchy *****/
   switch (Gbl.Usrs.Me.UsrLast.LastHie.Scope)
     {
      case Hie_Lvl_CTY:	// Country
         Gbl.Hierarchy.Cty.CtyCod = Gbl.Usrs.Me.UsrLast.LastHie.Cod;
	 break;
      case Hie_Lvl_INS:	// Institution
         Gbl.Hierarchy.Ins.InsCod = Gbl.Usrs.Me.UsrLast.LastHie.Cod;
	 break;
      case Hie_Lvl_CTR:	// Center
         Gbl.Hierarchy.Ctr.CtrCod = Gbl.Usrs.Me.UsrLast.LastHie.Cod;
	 break;
      case Hie_Lvl_DEG:	// Degree
         Gbl.Hierarchy.Deg.DegCod = Gbl.Usrs.Me.UsrLast.LastHie.Cod;
	 break;
      case Hie_Lvl_CRS:	// Course
         Gbl.Hierarchy.Crs.CrsCod = Gbl.Usrs.Me.UsrLast.LastHie.Cod;
	 break;
      default:
	 break;
     }

   /****** Initialize again current course, degree, center... ******/
   Hie_InitHierarchy ();
  }

/*****************************************************************************/
/**** Initialize current country, institution, center, degree and course *****/
/*****************************************************************************/

void Hie_InitHierarchy (void)
  {
   /***** If course code is available, get course data *****/
   if (Gbl.Hierarchy.Crs.CrsCod > 0)
     {
      if (Crs_GetDataOfCourseByCod (&Gbl.Hierarchy.Crs))	// Course found
         Gbl.Hierarchy.Deg.DegCod = Gbl.Hierarchy.Crs.DegCod;
      else
         Hie_ResetHierarchy ();
     }

   /***** If degree code is available, get degree data *****/
   if (Gbl.Hierarchy.Deg.DegCod > 0)
     {
      if (Deg_GetDataOfDegreeByCod (&Gbl.Hierarchy.Deg))	// Degree found
	{
	 Gbl.Hierarchy.Ctr.CtrCod = Gbl.Hierarchy.Deg.CtrCod;
         Gbl.Hierarchy.Ins.InsCod = Deg_GetInsCodOfDegreeByCod (Gbl.Hierarchy.Deg.DegCod);
	}
      else
         Hie_ResetHierarchy ();
     }

   /***** If center code is available, get center data *****/
   if (Gbl.Hierarchy.Ctr.CtrCod > 0)
     {
      if (Ctr_GetDataOfCenterByCod (&Gbl.Hierarchy.Ctr))	// Center found
         Gbl.Hierarchy.Ins.InsCod = Gbl.Hierarchy.Ctr.InsCod;
      else
         Hie_ResetHierarchy ();
     }

   /***** If institution code is available, get institution data *****/
   if (Gbl.Hierarchy.Ins.InsCod > 0)
     {
      if (Ins_GetDataOfInstitutionByCod (&Gbl.Hierarchy.Ins))	// Institution found
	 Gbl.Hierarchy.Cty.CtyCod = Gbl.Hierarchy.Ins.CtyCod;
      else
         Hie_ResetHierarchy ();
     }

   /***** If country code is available, get country data *****/
   if (Gbl.Hierarchy.Cty.CtyCod > 0)
      if (!Cty_GetDataOfCountryByCod (&Gbl.Hierarchy.Cty))		// Country not found
         Hie_ResetHierarchy ();

   /***** Set current hierarchy level and code
          depending on course code, degree code, etc. *****/
   if      (Gbl.Hierarchy.Crs.CrsCod > 0)	// Course selected
     {
      Gbl.Hierarchy.Level = Hie_Lvl_CRS;
      Gbl.Hierarchy.Cod = Gbl.Hierarchy.Crs.CrsCod;
     }
   else if (Gbl.Hierarchy.Deg.DegCod > 0)	// Degree selected
     {
      Gbl.Hierarchy.Level = Hie_Lvl_DEG;
      Gbl.Hierarchy.Cod = Gbl.Hierarchy.Deg.DegCod;
     }
   else if (Gbl.Hierarchy.Ctr.CtrCod > 0)	// Center selected
     {
      Gbl.Hierarchy.Level = Hie_Lvl_CTR;
      Gbl.Hierarchy.Cod = Gbl.Hierarchy.Ctr.CtrCod;
     }
   else if (Gbl.Hierarchy.Ins.InsCod > 0)	// Institution selected
     {
      Gbl.Hierarchy.Level = Hie_Lvl_INS;
      Gbl.Hierarchy.Cod = Gbl.Hierarchy.Ins.InsCod;
     }
   else if (Gbl.Hierarchy.Cty.CtyCod > 0)	// Country selected
     {
      Gbl.Hierarchy.Level = Hie_Lvl_CTY;
      Gbl.Hierarchy.Cod = Gbl.Hierarchy.Cty.CtyCod;
     }
   else
     {
      Gbl.Hierarchy.Level = Hie_Lvl_SYS;
      Gbl.Hierarchy.Cod = -1L;
     }

   /***** Initialize paths *****/
   if (Gbl.Hierarchy.Level == Hie_Lvl_CRS)	// Course selected
     {
      /***** Paths of course directories *****/
      snprintf (Gbl.Crs.PathPriv,sizeof (Gbl.Crs.PathPriv),"%s/%ld",
	        Cfg_PATH_CRS_PRIVATE,Gbl.Hierarchy.Crs.CrsCod);
      snprintf (Gbl.Crs.PathRelPubl,sizeof (Gbl.Crs.PathRelPubl),"%s/%ld",
	        Cfg_PATH_CRS_PUBLIC,Gbl.Hierarchy.Crs.CrsCod);
      snprintf (Gbl.Crs.PathURLPubl,sizeof (Gbl.Crs.PathURLPubl),"%s/%ld",
	        Cfg_URL_CRS_PUBLIC,Gbl.Hierarchy.Crs.CrsCod);

      /***** If any of the course directories does not exist, create it *****/
      if (!Fil_CheckIfPathExists (Gbl.Crs.PathPriv))
	 Fil_CreateDirIfNotExists (Gbl.Crs.PathPriv);
      if (!Fil_CheckIfPathExists (Gbl.Crs.PathRelPubl))
	 Fil_CreateDirIfNotExists (Gbl.Crs.PathRelPubl);

      /***** Count number of groups in current course
             (used in some actions) *****/
      Gbl.Crs.Grps.NumGrps = Grp_CountNumGrpsInCurrentCrs ();
     }
  }

/*****************************************************************************/
/******* Reset current country, institution, center, degree and course *******/
/*****************************************************************************/

void Hie_ResetHierarchy (void)
  {
   /***** Country *****/
   Gbl.Hierarchy.Cty.CtyCod = -1L;

   /***** Institution *****/
   Gbl.Hierarchy.Ins.InsCod = -1L;

   /***** Center *****/
   Gbl.Hierarchy.Ctr.CtrCod = -1L;
   Gbl.Hierarchy.Ctr.InsCod = -1L;
   Gbl.Hierarchy.Ctr.PlcCod = -1L;

   /***** Degree *****/
   Gbl.Hierarchy.Deg.DegCod = -1L;

   /***** Course *****/
   Gbl.Hierarchy.Crs.CrsCod = -1L;

   /***** Hierarchy level and code *****/
   Gbl.Hierarchy.Level = Hie_Lvl_UNK;
   Gbl.Hierarchy.Cod   = -1L;
  }

/*****************************************************************************/
/***** Write institutions, centers and degrees administrated by an admin *****/
/*****************************************************************************/

void Hie_GetAndWriteInsCtrDegAdminBy (long UsrCod,unsigned ColSpan)
  {
   extern const char *Txt_all_degrees;
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   unsigned NumRow;
   unsigned NumRows;
   struct Hie_Hierarchy Hie;

   /***** Get institutions, centers, degrees admin by user from database *****/
   NumRows = (unsigned)
   DB_QuerySELECT (&mysql_res,"can not get institutions, centers, degrees"
			      " admin by a user",
		   "(SELECT %u AS S,"			// row[0]
		           "-1 AS Cod,"			// row[1]
		           "'' AS FullName"		// row[2]
		     " FROM usr_admins"
		    " WHERE UsrCod=%ld"
		      " AND Scope='%s')"
		   " UNION "
		   "(SELECT %u AS S,"			// row[0]
			   "usr_admins.Cod,"		// row[1]
			   "ins_instits.FullName"	// row[2]
		     " FROM usr_admins,"
			   "ins_instits"
		    " WHERE usr_admins.UsrCod=%ld"
		      " AND usr_admins.Scope='%s'"
		      " AND usr_admins.Cod=ins_instits.InsCod)"
		   " UNION "
		   "(SELECT %u AS S,"			// row[0]
			   "usr_admins.Cod,"		// row[1]
			   "ctr_centers.FullName"	// row[2]
		     " FROM usr_admins,"
			   "ctr_centers"
		    " WHERE usr_admins.UsrCod=%ld"
		      " AND usr_admins.Scope='%s'"
		      " AND usr_admins.Cod=ctr_centers.CtrCod)"
		   " UNION "
		   "(SELECT %u AS S,"			// row[0]
			   "usr_admins.Cod,"		// row[1]
			   "deg_degrees.FullName"	// row[2]
		     " FROM usr_admins,"
		           "deg_degrees"
		    " WHERE usr_admins.UsrCod=%ld"
		      " AND usr_admins.Scope='%s'"
		      " AND usr_admins.Cod=deg_degrees.DegCod)"
		   " ORDER BY S,"
		             "FullName",
		   (unsigned) Hie_Lvl_SYS,UsrCod,Sco_GetDBStrFromScope (Hie_Lvl_SYS),
		   (unsigned) Hie_Lvl_INS,UsrCod,Sco_GetDBStrFromScope (Hie_Lvl_INS),
		   (unsigned) Hie_Lvl_CTR,UsrCod,Sco_GetDBStrFromScope (Hie_Lvl_CTR),
		   (unsigned) Hie_Lvl_DEG,UsrCod,Sco_GetDBStrFromScope (Hie_Lvl_DEG));
   if (NumRows)
      /***** Get the list of degrees *****/
      for (NumRow = 1;
	   NumRow <= NumRows;
	   NumRow++)
	{
         HTM_TR_Begin (NULL);

         /***** Indent *****/
         HTM_TD_Begin ("class=\"RT COLOR%u\"",Gbl.RowEvenOdd);
         Ico_PutIcon (NumRow == NumRows ? "subend20x20.gif" :
                	                  "submid20x20.gif",
		      "","ICO25x25");
         HTM_TD_End ();

         /***** Write institution, center, degree *****/
         HTM_TD_Begin ("colspan=\"%u\" class=\"DAT_SMALL_NOBR LT COLOR%u\"",
                       ColSpan - 1,Gbl.RowEvenOdd);

         /* Get next institution, center, degree */
         row = mysql_fetch_row (mysql_res);

	 /* Get scope */
	 switch (Sco_GetScopeFromUnsignedStr (row[0]))
	   {
	    case Hie_Lvl_SYS:	// System
	       Ico_PutIcon ("swad64x64.png",Txt_all_degrees,"ICO16x16");
	       HTM_TxtF ("&nbsp;%s",Txt_all_degrees);
	       break;
	    case Hie_Lvl_INS:	// Institution
	       Hie.Ins.InsCod = Str_ConvertStrCodToLongCod (row[1]);
	       if (Hie.Ins.InsCod > 0)
		 {
		  /* Get data of institution */
		  Ins_GetDataOfInstitutionByCod (&Hie.Ins);

		  /* Write institution logo and name */
		  Ins_DrawInstitutionLogoAndNameWithLink (&Hie.Ins,ActSeeInsInf,
						          "BT_LINK DAT_SMALL_NOBR","LT");
		 }
	       break;
	    case Hie_Lvl_CTR:	// Center
	       Hie.Ctr.CtrCod = Str_ConvertStrCodToLongCod (row[1]);
	       if (Hie.Ctr.CtrCod > 0)
		 {
		  /* Get data of center */
		  Ctr_GetDataOfCenterByCod (&Hie.Ctr);

		  /* Write center logo and name */
		  Ctr_DrawCenterLogoAndNameWithLink (&Hie.Ctr,ActSeeCtrInf,
						     "BT_LINK DAT_SMALL_NOBR","LT");
		 }
	       break;
	    case Hie_Lvl_DEG:	// Degree
	       Hie.Deg.DegCod = Str_ConvertStrCodToLongCod (row[1]);
	       if (Hie.Deg.DegCod > 0)
		 {
		  /* Get data of degree */
		  Deg_GetDataOfDegreeByCod (&Hie.Deg);

		  /* Write degree logo and name */
		  Deg_DrawDegreeLogoAndNameWithLink (&Hie.Deg,ActSeeDegInf,
						     "BT_LINK DAT_SMALL_NOBR","LT");
		 }
	       break;
	    default:	// There are no administrators in other scopes
	       Err_WrongScopeExit ();
	       break;
           }
         HTM_TD_End ();

         HTM_TR_End ();
        }

   /***** Free structure that stores the query result *****/
   DB_FreeMySQLResult (&mysql_res);
  }

/*****************************************************************************/
/********************* Build a "Go to <where>" message ***********************/
/*****************************************************************************/
// Where is a hierarchy member (country, institution, center, degree or course
// Hie_FreeGoToMsg() must be called after calling this function

char *Hie_BuildGoToMsg (const char *Where)
  {
   extern const char *Txt_Go_to_X;

   return Str_BuildStringStr (Txt_Go_to_X,Where);
  }

void Hie_FreeGoToMsg (void)
  {
   Str_FreeString ();
  }
