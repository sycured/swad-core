// swad_scope.c: scope (platform, center, degree, course...)

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

#include <string.h>		// For string functions

#include "swad_config.h"
#include "swad_error.h"
#include "swad_global.h"
#include "swad_HTML.h"
#include "swad_parameter.h"
#include "swad_scope.h"

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private constants *****************************/
/*****************************************************************************/

/*****************************************************************************/
/******************************* Private types *******************************/
/*****************************************************************************/

/*****************************************************************************/
/************** External global variables from others modules ****************/
/*****************************************************************************/

extern struct Globals Gbl;

/*****************************************************************************/
/************************* Private global variables **************************/
/*****************************************************************************/

/*****************************************************************************/
/***************************** Private prototypes ****************************/
/*****************************************************************************/

/*****************************************************************************/
/** Put a selector to choice between ranges when getting users for listing ***/
/*****************************************************************************/

void Sco_PutSelectorScope (const char *ParamName,HTM_SubmitOnChange_t SubmitOnChange)
  {
   extern const char *Txt_System;
   extern const char *Txt_Country;
   extern const char *Txt_Institution;
   extern const char *Txt_Center;
   extern const char *Txt_Degree;
   extern const char *Txt_Course;
   Hie_Lvl_Level_t Scope;
   unsigned ScopeUnsigned;
   bool WriteScope;

   HTM_SELECT_Begin (SubmitOnChange,
		     "id=\"%s\" name=\"%s\"",ParamName,ParamName);

   for (Scope  = (Hie_Lvl_Level_t) 0;
	Scope <= (Hie_Lvl_Level_t) (Hie_Lvl_NUM_LEVELS - 1);
	Scope++)
      if ((Gbl.Scope.Allowed & (1 << Scope)))
	{
	 /* Don't put forbidden options in selectable list */
	 WriteScope = false;
	 switch (Scope)
	   {
	    case Hie_Lvl_SYS:
	       WriteScope = true;
	       break;
	    case Hie_Lvl_CTY:
	       if (Gbl.Hierarchy.Cty.CtyCod > 0)
		  WriteScope = true;
	       break;
	    case Hie_Lvl_INS:
	       if (Gbl.Hierarchy.Ins.InsCod > 0)
		  WriteScope = true;
	       break;
	    case Hie_Lvl_CTR:
	       if (Gbl.Hierarchy.Ctr.CtrCod > 0)
		  WriteScope = true;
	       break;
	    case Hie_Lvl_DEG:
	       if (Gbl.Hierarchy.Deg.DegCod > 0)
		  WriteScope = true;
	       break;
	    case Hie_Lvl_CRS:
	       if (Gbl.Hierarchy.Level == Hie_Lvl_CRS)
		  WriteScope = true;
	       break;
	    default:
	       Err_WrongScopeExit ();
	       break;
	   }

	 if (WriteScope)
	   {
	    /***** Write allowed option *****/
	    ScopeUnsigned = (unsigned) Scope;
	    switch (Scope)
	      {
	       case Hie_Lvl_SYS:
		  HTM_OPTION (HTM_Type_UNSIGNED,&ScopeUnsigned,
			      Gbl.Scope.Current == Scope,false,
			      "%s: %s",
			      Txt_System,
			      Cfg_PLATFORM_SHORT_NAME);
		  break;
	       case Hie_Lvl_CTY:
		  HTM_OPTION (HTM_Type_UNSIGNED,&ScopeUnsigned,
			      Gbl.Scope.Current == Scope,false,
			      "%s: %s",
			      Txt_Country,
			      Gbl.Hierarchy.Cty.Name[Gbl.Prefs.Language]);
		  break;
	       case Hie_Lvl_INS:
		  HTM_OPTION (HTM_Type_UNSIGNED,&ScopeUnsigned,
			      Gbl.Scope.Current == Scope,false,
			      "%s: %s",
			      Txt_Institution,
			      Gbl.Hierarchy.Ins.ShrtName);
		  break;
	       case Hie_Lvl_CTR:
		  HTM_OPTION (HTM_Type_UNSIGNED,&ScopeUnsigned,
			      Gbl.Scope.Current == Scope,false,
			      "%s: %s",
			      Txt_Center,
			      Gbl.Hierarchy.Ctr.ShrtName);
		  break;
	       case Hie_Lvl_DEG:
		  HTM_OPTION (HTM_Type_UNSIGNED,&ScopeUnsigned,
			      Gbl.Scope.Current == Scope,false,
			      "%s: %s",
			      Txt_Degree,
			      Gbl.Hierarchy.Deg.ShrtName);
		  break;
	       case Hie_Lvl_CRS:
		  HTM_OPTION (HTM_Type_UNSIGNED,&ScopeUnsigned,
			      Gbl.Scope.Current == Scope,false,
			      "%s: %s",
			      Txt_Course,
			      Gbl.Hierarchy.Crs.ShrtName);
		  break;
	       default:
		  Err_WrongScopeExit ();
		  break;
	      }
	   }
	}

   HTM_SELECT_End ();
  }

/*****************************************************************************/
/********************** Put hidden parameter scope ***************************/
/*****************************************************************************/

void Sco_PutParamCurrentScope (void *Scope)
  {
   if (Scope)
      Sco_PutParamScope ("ScopeUsr",*((Hie_Lvl_Level_t *) Scope));
  }

void Sco_PutParamScope (const char *ParamName,Hie_Lvl_Level_t Scope)
  {
   Par_PutHiddenParamUnsigned (NULL,ParamName,(unsigned) Scope);
  }

/*****************************************************************************/
/*************************** Get parameter scope *****************************/
/*****************************************************************************/

void Sco_GetScope (const char *ParamName)
  {
   /***** Get parameter with scope *****/
   Gbl.Scope.Current = (Hie_Lvl_Level_t)
	               Par_GetParToUnsignedLong (ParamName,
                                                 0,
                                                 Hie_Lvl_NUM_LEVELS - 1,
                                                 (unsigned long) Hie_Lvl_UNK);

   /***** Adjust scope avoiding impossible or forbidden scopes *****/
   Sco_AdjustScope ();
  }

/*****************************************************************************/
/*********** Adjust scope avoiding impossible or forbidden scopes ************/
/*****************************************************************************/

void Sco_AdjustScope (void)
  {
   /***** Is scope is unknow, use default scope *****/
   if (Gbl.Scope.Current == Hie_Lvl_UNK)
      Gbl.Scope.Current = Gbl.Scope.Default;

   /***** Avoid impossible scopes *****/
   if (Gbl.Scope.Current == Hie_Lvl_CRS && Gbl.Hierarchy.Crs.CrsCod <= 0)
      Gbl.Scope.Current = Hie_Lvl_DEG;

   if (Gbl.Scope.Current == Hie_Lvl_DEG && Gbl.Hierarchy.Deg.DegCod <= 0)
      Gbl.Scope.Current = Hie_Lvl_CTR;

   if (Gbl.Scope.Current == Hie_Lvl_CTR && Gbl.Hierarchy.Ctr.CtrCod <= 0)
      Gbl.Scope.Current = Hie_Lvl_INS;

   if (Gbl.Scope.Current == Hie_Lvl_INS && Gbl.Hierarchy.Ins.InsCod <= 0)
      Gbl.Scope.Current = Hie_Lvl_CTY;

   if (Gbl.Scope.Current == Hie_Lvl_CTY && Gbl.Hierarchy.Cty.CtyCod <= 0)
      Gbl.Scope.Current = Hie_Lvl_SYS;

   /***** Avoid forbidden scopes *****/
   if ((Gbl.Scope.Allowed & (1 << Gbl.Scope.Current)) == 0)
      Gbl.Scope.Current = Hie_Lvl_UNK;
  }

/*****************************************************************************/
/****************** Set allowed scopes when listing guests *******************/
/*****************************************************************************/

void Sco_SetScopesForListingGuests (void)
  {
   switch (Gbl.Usrs.Me.Role.Logged)
     {
      case Rol_CTR_ADM:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_CTR;
	 Gbl.Scope.Default = Hie_Lvl_CTR;
	 break;
      case Rol_INS_ADM:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_INS |
		             1 << Hie_Lvl_CTR;
	 Gbl.Scope.Default = Hie_Lvl_INS;
	 break;
      case Rol_SYS_ADM:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_SYS |
	                     1 << Hie_Lvl_CTY |
		             1 << Hie_Lvl_INS |
		             1 << Hie_Lvl_CTR;
	 Gbl.Scope.Default = Hie_Lvl_SYS;
	 break;
      default:
      	 Gbl.Scope.Allowed = 0;
      	 Gbl.Scope.Default = Hie_Lvl_UNK;
	 break;
     }
  }

/*****************************************************************************/
/**************** Set allowed scopes when listing students *******************/
/*****************************************************************************/

void Sco_SetScopesForListingStudents (void)
  {
   Gbl.Scope.Default = Hie_Lvl_CRS;
   switch (Gbl.Usrs.Me.Role.Logged)
     {
      case Rol_STD:
      case Rol_NET:
      case Rol_TCH:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_CRS;
	 break;
      case Rol_DEG_ADM:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_DEG |
		             1 << Hie_Lvl_CRS;
	 break;
      case Rol_CTR_ADM:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_CTR |
		             1 << Hie_Lvl_DEG |
		             1 << Hie_Lvl_CRS;
	 break;
      case Rol_INS_ADM:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_INS |
		             1 << Hie_Lvl_CTR |
		             1 << Hie_Lvl_DEG |
		             1 << Hie_Lvl_CRS;
	 break;
      case Rol_SYS_ADM:
	 Gbl.Scope.Allowed = 1 << Hie_Lvl_SYS |
	                     1 << Hie_Lvl_CTY |
		             1 << Hie_Lvl_INS |
		             1 << Hie_Lvl_CTR |
		             1 << Hie_Lvl_DEG |
		             1 << Hie_Lvl_CRS;
	 break;
      default:
      	 Gbl.Scope.Allowed = 0;
      	 Gbl.Scope.Default = Hie_Lvl_UNK;
	 break;
     }
  }

/*****************************************************************************/
/*********************** Get scope from unsigned string **********************/
/*****************************************************************************/

Hie_Lvl_Level_t Sco_GetScopeFromUnsignedStr (const char *UnsignedStr)
  {
   unsigned UnsignedNum;

   if (sscanf (UnsignedStr,"%u",&UnsignedNum) == 1)
      if (UnsignedNum < Hie_Lvl_NUM_LEVELS)
         return (Hie_Lvl_Level_t) UnsignedNum;

   return Hie_Lvl_UNK;
  }

/*****************************************************************************/
/*********************** Get scope from database string **********************/
/*****************************************************************************/

Hie_Lvl_Level_t Sco_GetScopeFromDBStr (const char *ScopeDBStr)
  {
   Hie_Lvl_Level_t Scope;

   for (Scope  = (Hie_Lvl_Level_t) 0;
	Scope <= (Hie_Lvl_Level_t) (Hie_Lvl_NUM_LEVELS - 1);
	Scope++)
      if (!strcmp (Sco_GetDBStrFromScope (Scope),ScopeDBStr))
	 return Scope;

   return Hie_Lvl_UNK;
  }

/*****************************************************************************/
/*********************** Get scope from database string **********************/
/*****************************************************************************/

const char *Sco_GetDBStrFromScope (Hie_Lvl_Level_t Scope)
  {
   static const char *Sco_ScopeDB[Hie_Lvl_NUM_LEVELS] =
     {
      [Hie_Lvl_UNK] = "Unk",
      [Hie_Lvl_SYS] = "Sys",
      [Hie_Lvl_CTY] = "Cty",
      [Hie_Lvl_INS] = "Ins",
      [Hie_Lvl_CTR] = "Ctr",
      [Hie_Lvl_DEG] = "Deg",
      [Hie_Lvl_CRS] = "Crs",
     };

   if (Scope >= Hie_Lvl_NUM_LEVELS)
      Scope = Hie_Lvl_UNK;

   return Sco_ScopeDB[Scope];
  }
