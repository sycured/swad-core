// swad_system_config.c:  configuration of system

/*
    SWAD (Shared Workspace At a Distance),
    is a web platform developed at the University of Granada (Spain),
    and used to support university teaching.

    This file is part of SWAD core.
    Copyright (C) 1999-2020 Antonio Ca�as Vargas

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
#include <stdbool.h>		// For boolean type
#include <stddef.h>		// For NULL
#include <stdio.h>		// For asprintf
#include <stdlib.h>		// For free
#include <string.h>		// For string functions

#include "swad_box.h"
#include "swad_config.h"
#include "swad_course.h"
#include "swad_database.h"
#include "swad_figure_cache.h"
#include "swad_form.h"
#include "swad_help.h"
#include "swad_hierarchy.h"
#include "swad_hierarchy_config.h"
#include "swad_HTML.h"
#include "swad_role.h"
#include "swad_system_config.h"

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/******************************* Private types *******************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private variables *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

static void SysCfg_Configuration (bool PrintView);
static void SysCfg_PutIconToPrint (__attribute__((unused)) void *Args);
static void SysCfg_GetCoordAndZoom (struct Coordinates *Coord,unsigned *Zoom);
static void SysCfg_Map (void);
static void SysCfg_Platform (void);
static void SysCfg_Shortcut (bool PrintView);
static void SysCfg_QR (void);
static void SysCfg_NumCtys (void);
static void SysCfg_NumInss (void);
static void SysCfg_NumDegs (void);
static void SysCfg_NumCrss (void);
static void SysCfg_NumUsrsInCrss (Rol_Role_t Role);

/*****************************************************************************/
/***************** Show information of the current country *******************/
/*****************************************************************************/

void SysCfg_ShowConfiguration (void)
  {
   SysCfg_Configuration (false);

   /***** Show help to enrol me *****/
   Hlp_ShowHelpWhatWouldYouLikeToDo ();
  }

/*****************************************************************************/
/***************** Print information of the current country ******************/
/*****************************************************************************/

void SysCfg_PrintConfiguration (void)
  {
   SysCfg_Configuration (true);
  }

/*****************************************************************************/
/******************** Information of the current country *********************/
/*****************************************************************************/

static void SysCfg_Configuration (bool PrintView)
  {
   extern const char *Hlp_SYSTEM_Information;
   unsigned NumCtrs;
   unsigned NumCtrsWithMap;

   /***** Begin box *****/
   if (PrintView)
      Box_BoxBegin (NULL,Cfg_PLATFORM_SHORT_NAME,
                    NULL,NULL,
		    NULL,Box_NOT_CLOSABLE);
   else
      Box_BoxBegin (NULL,Cfg_PLATFORM_SHORT_NAME,
                    SysCfg_PutIconToPrint,NULL,
		    Hlp_SYSTEM_Information,Box_NOT_CLOSABLE);

   /**************************** Left part ***********************************/
   HTM_DIV_Begin ("class=\"HIE_CFG_LEFT HIE_CFG_WIDTH\"");

   /***** Begin table *****/
   HTM_TABLE_BeginWidePadding (2);

   /***** Platform *****/
   SysCfg_Platform ();

   /***** Shortcut to the country *****/
   SysCfg_Shortcut (PrintView);

   /***** Get number of centres with map *****/
   if (!FigCch_GetFigureFromCache (FigCch_NUM_CTRS_WITH_MAP,Hie_SYS,-1L,
                                   FigCch_Type_UNSIGNED,&NumCtrsWithMap))
     {
      // Not updated recently in cache ==> compute and update it in cache
      NumCtrsWithMap = Ctr_GetNumCtrsWithMapInSys ();
      FigCch_UpdateFigureIntoCache (FigCch_NUM_CTRS_WITH_MAP,Hie_SYS,-1L,
                                    FigCch_Type_UNSIGNED,&NumCtrsWithMap);
     }

   if (PrintView)
      /***** QR code with link to the country *****/
      SysCfg_QR ();
   else
     {
      /***** Get number of centres *****/
      if (!FigCch_GetFigureFromCache (FigCch_NUM_CTRS,Hie_SYS,-1L,
                                      FigCch_Type_UNSIGNED,&NumCtrs))
	{
	 // Not updated recently in cache ==> compute and update it in cache
	 NumCtrs = Ctr_GetNumCtrsInSys ();
	 FigCch_UpdateFigureIntoCache (FigCch_NUM_CTRS,Hie_SYS,-1L,
	                               FigCch_Type_UNSIGNED,&NumCtrs);
	}

      /***** Number of countries,
             number of institutions,
             number of centres,
             number of degrees,
             number of courses *****/
      SysCfg_NumCtys ();
      SysCfg_NumInss ();
      HieCfg_NumCtrs (NumCtrs,
		      false);	// Don't put form
      HieCfg_NumCtrsWithMap (NumCtrs,NumCtrsWithMap);
      SysCfg_NumDegs ();
      SysCfg_NumCrss ();

      /***** Number of users in courses of this country *****/
      SysCfg_NumUsrsInCrss (Rol_TCH);
      SysCfg_NumUsrsInCrss (Rol_NET);
      SysCfg_NumUsrsInCrss (Rol_STD);
      SysCfg_NumUsrsInCrss (Rol_UNK);
     }

   /***** End table *****/
   HTM_TABLE_End ();

   /***** End of left part *****/
   HTM_DIV_End ();

   /**************************** Right part **********************************/
   if (NumCtrsWithMap)
     {
      HTM_DIV_Begin ("class=\"HIE_CFG_RIGHT HIE_CFG_WIDTH\"");

      /***** Country map *****/
      SysCfg_Map ();

      HTM_DIV_End ();
     }

   /***** End box *****/
   Box_BoxEnd ();
  }

/*****************************************************************************/
/************* Put icon to print the configuration of a country **************/
/*****************************************************************************/

static void SysCfg_PutIconToPrint (__attribute__((unused)) void *Args)
  {
   Ico_PutContextualIconToPrint (ActPrnSysInf,
				 NULL,NULL);
  }

/*****************************************************************************/
/********* Get average coordinates of centres in current institution *********/
/*****************************************************************************/

static void SysCfg_GetCoordAndZoom (struct Coordinates *Coord,unsigned *Zoom)
  {
   char *Query;

   /***** Get average coordinates of centres with both coordinates set
          (coordinates 0, 0 means not set ==> don't show map) *****/
   if (asprintf (&Query,
		 "SELECT AVG(Latitude),"				// row[0]
			"AVG(Longitude),"				// row[1]
			"GREATEST(MAX(Latitude)-MIN(Latitude),"
				 "MAX(Longitude)-MIN(Longitude))"	// row[2]
		 " FROM centres"
		 " WHERE Latitude<>0"
		 " AND Longitude<>0") < 0)
      Lay_NotEnoughMemoryExit ();
   Map_GetCoordAndZoom (Coord,Zoom,Query);
   free (Query);
  }

/*****************************************************************************/
/****************************** Draw country map *****************************/
/*****************************************************************************/

#define SysCfg_MAP_CONTAINER_ID "sys_mapid"

static void SysCfg_Map (void)
  {
   MYSQL_RES *mysql_res;
   MYSQL_ROW row;
   struct Coordinates CtyAvgCoord;
   unsigned Zoom;
   unsigned NumCtrs;
   unsigned NumCtr;
   struct Centre Ctr;
   struct Instit Ins;

   /***** Leaflet CSS *****/
   Map_LeafletCSS ();

   /***** Leaflet script *****/
   Map_LeafletScript ();

   /***** Container for the map *****/
   HTM_DIV_Begin ("id=\"%s\"",SysCfg_MAP_CONTAINER_ID);
   HTM_DIV_End ();

   /***** Script to draw the map *****/
   HTM_SCRIPT_Begin (NULL,NULL);

   /* Let's create a map with pretty Mapbox Streets tiles */
   SysCfg_GetCoordAndZoom (&CtyAvgCoord,&Zoom);
   Map_CreateMap (SysCfg_MAP_CONTAINER_ID,&CtyAvgCoord,Zoom);

   /* Add Mapbox Streets tile layer to our map */
   Map_AddTileLayer ();

   /* Get centres with coordinates */
   NumCtrs = (unsigned) DB_QuerySELECT (&mysql_res,"can not get centres"
						   " with coordinates",
					"SELECT CtrCod"	// row[0]
					" FROM centres"
					" WHERE centres.Latitude<>0"
					" AND centres.Longitude<>0");

   /* Add a marker and a popup for each centre */
   for (NumCtr = 0;
	NumCtr < NumCtrs;
	NumCtr++)
     {
      /* Get next centre */
      row = mysql_fetch_row (mysql_res);

      /* Get centre code (row[0]) */
      Ctr.CtrCod = Str_ConvertStrCodToLongCod (row[0]);

      /* Get data of centre */
      Ctr_GetDataOfCentreByCod (&Ctr);

      /* Get data of institution */
      Ins.InsCod = Ctr.InsCod;
      Ins_GetDataOfInstitutionByCod (&Ins);

      /* Add marker */
      Map_AddMarker (&Ctr.Coord);

      /* Add popup */
      Map_AddPopup (Ctr.ShrtName,Ins.ShrtName,
		    false);	// Closed
     }

   /* Free structure that stores the query result */
   DB_FreeMySQLResult (&mysql_res);

   HTM_SCRIPT_End ();
  }

/*****************************************************************************/
/****************** Show platform in country configuration *******************/
/*****************************************************************************/

static void SysCfg_Platform (void)
  {
   extern const char *Txt_Platform;

   /***** Institution *****/
   HTM_TR_Begin (NULL);

   /* Label */
   Frm_LabelColumn ("RT",NULL,Txt_Platform);

   /* Data */
   HTM_TD_Begin ("class=\"DAT_N LB\"");
   HTM_Txt (Cfg_PLATFORM_SHORT_NAME);
   HTM_TD_End ();

   HTM_TR_End ();
  }

/*****************************************************************************/
/************** Show platform shortcut in system configuration ***************/
/*****************************************************************************/

static void SysCfg_Shortcut (bool PrintView)
  {
   HieCfg_Shortcut (PrintView,NULL,-1L);
  }

/*****************************************************************************/
/***************** Show country QR in country configuration ******************/
/*****************************************************************************/

static void SysCfg_QR (void)
  {
   HieCfg_QR (NULL,-1L);
  }

/*****************************************************************************/
/************ Show number of countries in system configuration ***************/
/*****************************************************************************/

static void SysCfg_NumCtys (void)
  {
   extern const char *Txt_Countries;
   unsigned NumCtys;

   /***** Number of countries ******/
   HTM_TR_Begin (NULL);

   /* Label */
   Frm_LabelColumn ("RT",NULL,Txt_Countries);

   /* Data */
   HTM_TD_Begin ("class=\"LB\"");
   Frm_StartFormGoTo (ActSeeCty);
   HTM_BUTTON_SUBMIT_Begin (Txt_Countries,"BT_LINK DAT",NULL);
   if (!FigCch_GetFigureFromCache (FigCch_NUM_CTYS,Hie_SYS,-1L,
                                   FigCch_Type_UNSIGNED,&NumCtys))
     {
      // Not updated recently in cache ==> compute and update it in cache
      NumCtys = Cty_GetNumCtysTotal ();
      FigCch_UpdateFigureIntoCache (FigCch_NUM_CTYS,Hie_SYS,-1L,
                                    FigCch_Type_UNSIGNED,&NumCtys);
     }
   HTM_Unsigned (NumCtys);
   HTM_BUTTON_End ();
   Frm_EndForm ();
   HTM_TD_End ();

   HTM_TR_End ();
  }

/*****************************************************************************/
/*********** Show number of institutions in system configuration *************/
/*****************************************************************************/

static void SysCfg_NumInss (void)
  {
   extern const char *Txt_Institutions;
   unsigned NumInss;

   /***** Number of institutions ******/
   HTM_TR_Begin (NULL);

   /* Label */
   Frm_LabelColumn ("RT",NULL,Txt_Institutions);

   /* Data */
   HTM_TD_Begin ("class=\"DAT LB\"");
   if (!FigCch_GetFigureFromCache (FigCch_NUM_INSS,Hie_SYS,-1L,
                                   FigCch_Type_UNSIGNED,&NumInss))
     {
      // Not updated recently in cache ==> compute and update it in cache
      NumInss = Ins_GetNumInssTotal ();
      FigCch_UpdateFigureIntoCache (FigCch_NUM_INSS,Hie_SYS,-1L,
                                    FigCch_Type_UNSIGNED,&NumInss);
     }
   HTM_Unsigned (NumInss);
   HTM_TD_End ();

   HTM_TR_End ();
  }

/*****************************************************************************/
/************* Show number of degrees in system configuration ****************/
/*****************************************************************************/

static void SysCfg_NumDegs (void)
  {
   extern const char *Txt_Degrees;
   unsigned NumDegs;

   /***** Number of degrees *****/
   HTM_TR_Begin (NULL);

   /* Label */
   Frm_LabelColumn ("RT",NULL,Txt_Degrees);

   /* Data */
   HTM_TD_Begin ("class=\"DAT LB\"");
   if (!FigCch_GetFigureFromCache (FigCch_NUM_DEGS,Hie_SYS,-1L,
                                   FigCch_Type_UNSIGNED,&NumDegs))
     {
      // Not updated recently in cache ==> compute and update it in cache
      NumDegs = Deg_GetNumDegsTotal ();
      FigCch_UpdateFigureIntoCache (FigCch_NUM_DEGS,Hie_SYS,-1L,
                                    FigCch_Type_UNSIGNED,&NumDegs);
     }
   HTM_Unsigned (NumDegs);
   HTM_TD_End ();

   HTM_TR_End ();
  }

/*****************************************************************************/
/************* Show number of courses in system configuration ****************/
/*****************************************************************************/

static void SysCfg_NumCrss (void)
  {
   extern const char *Txt_Courses;
   unsigned NumCrss;

   /***** Number of courses *****/
   HTM_TR_Begin (NULL);

   /* Label */
   Frm_LabelColumn ("RT",NULL,Txt_Courses);

   /* Data */
   HTM_TD_Begin ("class=\"DAT LB\"");
   if (!FigCch_GetFigureFromCache (FigCch_NUM_CRSS,Hie_SYS,-1L,
                                   FigCch_Type_UNSIGNED,&NumCrss))
     {
      // Not updated recently in cache ==> compute and update it in cache
      NumCrss = Crs_GetNumCrssTotal ();
      FigCch_UpdateFigureIntoCache (FigCch_NUM_CRSS,Hie_SYS,-1L,
                                    FigCch_Type_UNSIGNED,&NumCrss);
     }
   HTM_Unsigned (NumCrss);
   HTM_TD_End ();

   HTM_TR_End ();
  }

/*****************************************************************************/
/***************** Number of users in courses of the system ******************/
/*****************************************************************************/

static void SysCfg_NumUsrsInCrss (Rol_Role_t Role)
  {
   extern const char *Txt_Users_in_courses;
   extern const char *Txt_ROLES_PLURAL_Abc[Rol_NUM_ROLES][Usr_NUM_SEXS];
   unsigned NumUsrsInCrss;
   static FigCch_FigureCached_t Figure[Rol_NUM_ROLES] =
     {
      [Rol_UNK	  ] = FigCch_NUM_USRS_IN_CRSS,	// Any users in courses
      [Rol_GST	  ] = FigCch_UNKNOWN,		// Not applicable
      [Rol_USR	  ] = FigCch_UNKNOWN,		// Not applicable
      [Rol_STD	  ] = FigCch_NUM_STDS_IN_CRSS,	// Students
      [Rol_NET    ] = FigCch_NUM_NETS_IN_CRSS,	// Non-editing teachers
      [Rol_TCH	  ] = FigCch_NUM_TCHS_IN_CRSS,	// Teachers
      [Rol_DEG_ADM] = FigCch_UNKNOWN,		// Not applicable
      [Rol_CTR_ADM] = FigCch_UNKNOWN,		// Not applicable
      [Rol_INS_ADM] = FigCch_UNKNOWN,		// Not applicable
      [Rol_SYS_ADM] = FigCch_UNKNOWN,		// Not applicable
     };

   /***** Number of users in courses *****/
   HTM_TR_Begin (NULL);

   /* Label */
   Frm_LabelColumn ("RT",NULL,
		    Role == Rol_UNK ? Txt_Users_in_courses :
		                      Txt_ROLES_PLURAL_Abc[Role][Usr_SEX_UNKNOWN]);

   /* Data */
   HTM_TD_Begin ("class=\"DAT LB\"");
   if (!FigCch_GetFigureFromCache (Figure[Role],Hie_SYS,-1L,
                                   FigCch_Type_UNSIGNED,&NumUsrsInCrss))
     {
      // Not updated recently in cache ==> compute and update it in cache
      NumUsrsInCrss = Usr_GetNumUsrsInCrss (Hie_SYS,-1L,
					    Role == Rol_UNK ? (1 << Rol_STD) |
							      (1 << Rol_NET) |
							      (1 << Rol_TCH) :	// Any user
							      (1 << Role));
      FigCch_UpdateFigureIntoCache (Figure[Role],Hie_SYS,-1L,
                                    FigCch_Type_UNSIGNED,&NumUsrsInCrss);
     }
   HTM_Unsigned (NumUsrsInCrss);
   HTM_TD_End ();

   HTM_TR_End ();
  }
