// swad_user.h: users

#ifndef _SWAD_USR
#define _SWAD_USR
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

#include <mysql/mysql.h>	// To access MySQL databases

#include "swad_action.h"
#include "swad_constant.h"
#include "swad_country.h"
#include "swad_cryptography.h"
#include "swad_date.h"
#include "swad_degree.h"
#include "swad_icon.h"
#include "swad_layout.h"
#include "swad_menu.h"
#include "swad_nickname.h"
#include "swad_password.h"
#include "swad_privacy_visibility_type.h"
#include "swad_role_type.h"
#include "swad_scope.h"
#include "swad_search.h"
#include "swad_string.h"
#include "swad_theme.h"

/*****************************************************************************/
/****************************** Public constants *****************************/
/*****************************************************************************/

#define Usr_MIN_MONTHS_WITHOUT_ACCESS_TO_REMOVE_OLD_USRS  6
#define Usr_DEF_MONTHS_WITHOUT_ACCESS_TO_REMOVE_OLD_USRS 12
#define Usr_MAX_MONTHS_WITHOUT_ACCESS_TO_REMOVE_OLD_USRS 60

#define Usr_MAX_CHARS_FIRSTNAME_OR_SURNAME	(32 - 1)	// 31
#define Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME	((Usr_MAX_CHARS_FIRSTNAME_OR_SURNAME + 1) * Str_MAX_BYTES_PER_CHAR - 1)	// 511

#define Usr_MAX_BYTES_SURNAMES			(Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1 + Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME)
						// Surname1                         +' '+ Surname2
#define Usr_MAX_BYTES_FULL_NAME			(Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1 + Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME +     6    + Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME)
						// Surname1                         +' '+ Surname2                           +  ','+' ' + FirstName
						// Surname1                         +' '+ Surname2                           + '<br />' + FirstName

#define Usr_BIRTHDAY_STR_DB_LENGTH (1 + 4 + 1 + 2 + 1 + 2 + 1)	// "'%04u-%02u-%02u'"

#define Usr_MAX_CHARS_ADDRESS	(128 - 1)	// 127
#define Usr_MAX_BYTES_ADDRESS	((Usr_MAX_CHARS_ADDRESS + 1) * Str_MAX_BYTES_PER_CHAR - 1)	// 2047

#define Usr_MAX_CHARS_PHONE	16
#define Usr_MAX_BYTES_PHONE	Usr_MAX_CHARS_PHONE

#define Usr_CLASS_PHOTO_COLS_DEF	 10	// Default number of columns in a class photo
#define Usr_CLASS_PHOTO_COLS_MAX	100	// Maximum number of columns in a class photo

#define Usr_LIST_WITH_PHOTOS_DEF	true

#define Usr_MAX_BYTES_LIST_ENCRYPTED_USR_CODS	(Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64 * Cfg_MAX_USRS_IN_LIST)

#define Usr_NUM_MAIN_FIELDS_DATA_USR	 8

#define Usr_USER_LIST_SECTION_ID	"user_list"

/*****************************************************************************/
/******************************** Public types *******************************/
/*****************************************************************************/

// Get user's data with or without personal settings
typedef enum
  {
   Usr_DONT_GET_PREFS = 0,
   Usr_GET_PREFS      = 1,
  } Usr_GetPrefs_t;

typedef enum
  {
   Usr_DONT_GET_ROLE_IN_CURRENT_CRS = 0,
   Usr_GET_ROLE_IN_CURRENT_CRS      = 1,
  } Usr_GetRoleInCurrentCrs_t;


// Related with user's sexs
#define Usr_NUM_SEXS 4	// Unknown, female, male, all
typedef enum
  {
   Usr_SEX_UNKNOWN = 0,
   Usr_SEX_FEMALE  = 1,
   Usr_SEX_MALE    = 2,
   Usr_SEX_ALL     = 3,	// Usr_SEX_ALL is intended for "all sexs"
  } Usr_Sex_t;
// Don't change these numbers! They are used for user's sex in database

// Related with class photograph
typedef enum
  {
   Usr_CLASS_PHOTO_SEL,		// Only for selection of users
   Usr_CLASS_PHOTO_SEL_SEE,	// Selecting and seeing users
   Usr_CLASS_PHOTO_PRN,		// Only print users
  } Usr_ClassPhotoType_t;

// Related with type of list of users
#define Usr_NUM_USR_LIST_TYPES 3
typedef enum
  {
   Usr_LIST_UNKNOWN        = 0,
   Usr_LIST_AS_CLASS_PHOTO = 1,
   Usr_LIST_AS_LISTING     = 2,
  } Usr_ShowUsrsType_t;
#define Usr_SHOW_USRS_TYPE_DEFAULT Usr_LIST_AS_CLASS_PHOTO

#define Usr_LIST_USRS_NUM_OPTIONS 8
typedef enum
  {
   Usr_OPTION_UNKNOWN		= 0,
   Usr_OPTION_RECORDS		= 1,
   Usr_OPTION_HOMEWORK		= 2,
   Usr_OPTION_ATTENDANCE	= 3,
   Usr_OPTION_MESSAGE		= 4,
   Usr_OPTION_EMAIL		= 5,
   Usr_OPTION_FOLLOW		= 6,
   Usr_OPTION_UNFOLLOW		= 7,
  } Usr_ListUsrsOption_t;
#define Usr_LIST_USRS_DEFAULT_OPTION Usr_OPTION_RECORDS

typedef enum
  {
   Usr_ME,
   Usr_OTHER,
  } Usr_MeOrOther_t;

#define Usr_NUM_WHO 5
typedef enum
  {
   Usr_WHO_UNKNOWN,
   Usr_WHO_ME,
   Usr_WHO_SELECTED,
   Usr_WHO_FOLLOWED,
   Usr_WHO_ALL,
  } Usr_Who_t;
#define Usr_WHO_DEFAULT Usr_WHO_ALL

// Related with user's data
struct UsrData
  {
   long UsrCod;
   char EnUsrCod [Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64 + 1];
   char UsrIDNickOrEmail[Cns_MAX_BYTES_EMAIL_ADDRESS + 1];	// String to store the ID, nickname or email
   struct
     {
      struct ListIDs *List;
      unsigned Num;
     } IDs;
   char Nickname        [Nck_MAX_BYTES_NICK_WITHOUT_ARROBA + 1];
   char Password        [Pwd_BYTES_ENCRYPTED_PASSWORD + 1];
   struct
     {
      Rol_Role_t InCurrentCrs;	// Role in current course (Rol_UNK is not filled/calculated or no course selected)
      int InCrss;	// Roles in all his/her courses
			// Check always if filled/calculated
			// >=0 ==> filled/calculated
			//  <0 ==> not yet filled/calculated
     } Roles;
   bool Accepted;	// User has accepted joining to current course?
   char Surname1	[Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];
   char Surname2	[Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];
   char FrstName	[Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];
   char FullName	[Usr_MAX_BYTES_FULL_NAME + 1];
   Usr_Sex_t Sex;
   char Email		[Cns_MAX_BYTES_EMAIL_ADDRESS + 1];
   bool EmailConfirmed;
   char Photo		[Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64 + 1];	// Name of public link to photo
   Pri_Visibility_t PhotoVisibility;	// Who can see user's photo
   Pri_Visibility_t BaPrfVisibility;	// Who can see user's basic public profile (minimal record card)
   Pri_Visibility_t ExPrfVisibility;	// Who can see user's extended public profile (figures, follow)
   long CtyCod;		// Country
   struct Date Birthday;
   char StrBirthday	[Cns_MAX_BYTES_DATE + 1];
   char Phone           [2][Usr_MAX_BYTES_PHONE + 1];
   char *Comments;
   long InsCtyCod;	// Country of the institution
   long InsCod;		// Institution
   struct
     {
      long CtrCod;	// Center
      long DptCod;	// Department
      char Office	[Usr_MAX_BYTES_ADDRESS + 1];
      char OfficePhone	[Usr_MAX_BYTES_PHONE  + 1];
     } Tch;
   struct
     {
      unsigned CreateNotif;	// One bit activated for each type of event
      unsigned SendEmail;	// One bit activated for each type of event
     } NtfEvents;
   struct
     {
      Lan_Language_t Language;
      unsigned FirstDayOfWeek;
      Dat_Format_t DateFormat;
      The_Theme_t Theme;
      Ico_IconSet_t IconSet;
      Mnu_Menu_t Menu;
      unsigned SideCols;
      bool AcceptThirdPartyCookies;	// User has accepted third party cookies
     } Prefs;
  };

struct UsrLast
  {
   Sch_WhatToSearch_t WhatToSearch;	// Search courses, teachers, documents...?
   struct
     {
      Hie_Lvl_Level_t Scope;	// Course, degree, center, etc.
      long Cod;			// Course code, degree code, center code, etc.
     } LastHie;
   Act_Action_t LastAct;
   Rol_Role_t LastRole;
   long LastTime;
   long LastAccNotif;
  };

struct UsrInList
  {
   long UsrCod;
   char EnUsrCod[Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64 + 1];
   char Password[Pwd_BYTES_ENCRYPTED_PASSWORD + 1];
   char Surname1 [Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];
   char Surname2 [Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];
   char FrstName[Usr_MAX_BYTES_FIRSTNAME_OR_SURNAME + 1];
   Usr_Sex_t Sex;
   char Photo[Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64 + 1];	// Name of public link to photo
   Pri_Visibility_t PhotoVisibility;				// Who can see user's photo
   long CtyCod;		// Country
   long InsCod;		// Institution
   Rol_Role_t RoleInCurrentCrsDB;	// Role in current course in database
   bool Accepted;	// User has accepted joining to one/all courses?
   bool Remove;		// A boolean associated with each user that indicates if it must be removed
  };

struct ListUsrs
  {
   struct UsrInList *Lst;	// List of users
   unsigned NumUsrs;		// Number of users in the list
  };

struct SelectedUsrs
  {
   char *List[Rol_NUM_ROLES];	// Lists of encrypted codes of users selected from a form
   bool Filled;			// If lists are already filled/readed
   char *ParamSuffix;
   Usr_ListUsrsOption_t Option;	// What option I have selected to do with these selected users
  };

struct ListUsrCods
  {
   long *Lst;		// List of users' codes
   unsigned NumUsrs;	// Number of users in the list
  };

/*****************************************************************************/
/****************************** Public prototypes ****************************/
/*****************************************************************************/

void Usr_InformAboutNumClicksBeforePhoto (void);

void Usr_UsrDataConstructor (struct UsrData *UsrDat);
void Usr_ResetUsrDataExceptUsrCodAndIDs (struct UsrData *UsrDat);
void Usr_ResetMyLastData (void);
void Usr_UsrDataDestructor (struct UsrData *UsrDat);
void Usr_GetAllUsrDataFromUsrCod (struct UsrData *UsrDat,
                                  Usr_GetPrefs_t GetPrefs,
                                  Usr_GetRoleInCurrentCrs_t GetRoleInCurrentCrs);
void Usr_AllocateListUsrCods (struct ListUsrCods *ListUsrCods);
void Usr_FreeListUsrCods (struct ListUsrCods *ListUsrCods);
bool Usr_ItsMe (long UsrCod);
void Usr_GetUsrCodFromEncryptedUsrCod (struct UsrData *UsrDat);
void Usr_GetUsrDataFromUsrCod (struct UsrData *UsrDat,
                               Usr_GetPrefs_t GetPrefs,
                               Usr_GetRoleInCurrentCrs_t GetRoleInCurrentCrs);

void Usr_BuildFullName (struct UsrData *UsrDat);

void Usr_WriteFirstNameBRSurnames (const struct UsrData *UsrDat);

void Usr_FlushCachesUsr (void);

bool Usr_CheckIfUsrIsAdm (long UsrCod,Hie_Lvl_Level_t Scope,long Cod);
void Usr_FlushCacheUsrIsSuperuser (void);
bool Usr_CheckIfUsrIsSuperuser (long UsrCod);

bool Usr_ICanChangeOtherUsrData (const struct UsrData *UsrDat);
bool Usr_ICanEditOtherUsr (const struct UsrData *UsrDat);

unsigned Usr_GetNumCrssOfUsr (long UsrCod);
unsigned Usr_GetNumCrssOfUsrNotAccepted (long UsrCod);
unsigned Usr_GetNumCrssOfUsrWithARole (long UsrCod,Rol_Role_t Role);
unsigned Usr_GetNumCrssOfUsrWithARoleNotAccepted (long UsrCod,Rol_Role_t Role);

unsigned Usr_GetNumUsrsInCrssOfAUsr (long UsrCod,Rol_Role_t UsrRole,
                                     unsigned OthersRoles);

void Usr_FlushCacheUsrBelongsToCurrentCrs (void);
bool Usr_CheckIfUsrBelongsToCurrentCrs (const struct UsrData *UsrDat);
void Usr_FlushCacheUsrHasAcceptedInCurrentCrs (void);
bool Usr_CheckIfUsrHasAcceptedInCurrentCrs (const struct UsrData *UsrDat);

bool Usr_CheckIfICanViewRecordStd (const struct UsrData *UsrDat);
bool Usr_CheckIfICanViewRecordTch (struct UsrData *UsrDat);
bool Usr_CheckIfICanViewTstExaMchResult (const struct UsrData *UsrDat);
bool Usr_CheckIfICanViewAsgWrk (const struct UsrData *UsrDat);
bool Usr_CheckIfICanViewAtt (const struct UsrData *UsrDat);
bool Usr_CheckIfICanViewUsrAgenda (struct UsrData *UsrDat);

void Usr_FlushCacheUsrSharesAnyOfMyCrs (void);
bool Usr_CheckIfUsrSharesAnyOfMyCrs (struct UsrData *UsrDat);
bool Usr_CheckIfUsrSharesAnyOfMyCrsWithDifferentRole (long UsrCod);

void Usr_GetMyCountrs (void);
void Usr_GetMyInstits (void);
void Usr_GetMyCenters (void);
void Usr_GetMyDegrees (void);
void Usr_GetMyCourses (void);

void Usr_FreeMyCountrs (void);
void Usr_FreeMyInstits (void);
void Usr_FreeMyCenters (void);
void Usr_FreeMyDegrees (void);
void Usr_FreeMyCourses (void);

void Usr_FlushCacheUsrBelongsToIns (void);
bool Usr_CheckIfUsrBelongsToIns (long UsrCod,long InsCod);
void Usr_FlushCacheUsrBelongsToCtr (void);
bool Usr_CheckIfUsrBelongsToCtr (long UsrCod,long CtrCod);
void Usr_FlushCacheUsrBelongsToDeg (void);
bool Usr_CheckIfUsrBelongsToDeg (long UsrCod,long DegCod);
void Usr_FlushCacheUsrBelongsToCrs (void);
bool Usr_CheckIfUsrBelongsToCrs (long UsrCod,long CrsCod,
                                 bool CountOnlyAcceptedCourses);

bool Usr_CheckIfIBelongToCty (long CtyCod);
bool Usr_CheckIfIBelongToIns (long InsCod);
bool Usr_CheckIfIBelongToCtr (long CtrCod);
bool Usr_CheckIfIBelongToDeg (long DegCod);
bool Usr_CheckIfIBelongToCrs (long CrsCod);

unsigned Usr_GetCtysFromUsr (long UsrCod,MYSQL_RES **mysql_res);
unsigned Usr_GetInssFromUsr (long UsrCod,long CtyCod,MYSQL_RES **mysql_res);
unsigned Usr_GetCtrsFromUsr (long UsrCod,long InsCod,MYSQL_RES **mysql_res);
unsigned Usr_GetDegsFromUsr (long UsrCod,long CtrCod,MYSQL_RES **mysql_res);
unsigned Usr_GetCrssFromUsr (long UsrCod,long DegCod,MYSQL_RES **mysql_res);
void Usr_GetMainDeg (long UsrCod,
		     char ShrtName[Cns_HIERARCHY_MAX_BYTES_SHRT_NAME + 1],
		     Rol_Role_t *MaxRole);

bool Usr_ChkIfEncryptedUsrCodExists (const char EncryptedUsrCod[Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64]);

void Usr_WriteLandingPage (void);
void Usr_WriteFormLogout (void);
void Usr_Logout (void);
void Usr_PutLinkToLogin (void);
void Usr_WriteFormLogin (Act_Action_t NextAction,void (*FuncParams) (void));
void Usr_WelcomeUsr (void);

void Usr_CreateBirthdayStrDB (const struct UsrData *UsrDat,
                              char BirthdayStrDB[Usr_BIRTHDAY_STR_DB_LENGTH + 1]);

void Usr_PutFormLogIn (void);
void Usr_WriteLoggedUsrHead (void);
void Usr_PutFormLogOut (void);
void Usr_GetParamUsrIdLogin (void);
unsigned Usr_GetParamOtherUsrIDNickOrEMailAndGetUsrCods (struct ListUsrCods *ListUsrCods);

void Usr_PutParamMyUsrCodEncrypted (void *EncryptedUsrCod);
void Usr_PutParamOtherUsrCodEncrypted (void *EncryptedUsrCod);
void Usr_PutParamUsrCodEncrypted (const char EncryptedUsrCod[Cry_BYTES_ENCRYPTED_STR_SHA256_BASE64 + 1]);
void Usr_GetParamOtherUsrCodEncrypted (struct UsrData *UsrDat);
void Usr_GetParamOtherUsrCodEncryptedAndGetListIDs (void);
bool Usr_GetParamOtherUsrCodEncryptedAndGetUsrData (void);

void Usr_ChkUsrAndGetUsrData (void);

void Usr_ShowFormsLogoutAndRole (void);

bool Usr_ChkUsrCodAndGetAllUsrDataFromUsrCod (struct UsrData *UsrDat,
                                              Usr_GetPrefs_t GetPrefs,
                                              Usr_GetRoleInCurrentCrs_t GetRoleInCurrentCrs);
void Usr_UpdateMyLastData (void);
void Usr_InsertMyLastCrsTabAndTime (void);

void Usr_DB_RemoveUsrLastData (long UsrCod);
void Usr_DB_RemoveUsrData (long UsrCod);

void Usr_WriteRowUsrMainData (unsigned NumUsr,struct UsrData *UsrDat,
                              bool PutCheckBoxToSelectUsr,Rol_Role_t Role,
			      struct SelectedUsrs *SelectedUsrs);

long Usr_GetRamdomStdFromCrs (long CrsCod);
long Usr_GetRamdomStdFromGrp (long GrpCod);

unsigned Usr_GetNumTchsCurrentInsInDepartment (long DptCod);

void Usr_FlushCacheNumUsrsWhoDontClaimToBelongToAnyCty (void);
unsigned Usr_GetNumUsrsWhoDontClaimToBelongToAnyCty (void);
unsigned Usr_GetCachedNumUsrsWhoDontClaimToBelongToAnyCty (void);

void Usr_FlushCacheNumUsrsWhoClaimToBelongToAnotherCty (void);
unsigned Usr_GetNumUsrsWhoClaimToBelongToAnotherCty (void);
unsigned Usr_GetCachedNumUsrsWhoClaimToBelongToAnotherCty (void);

void Usr_FlushCacheNumUsrsWhoClaimToBelongToCty (void);
unsigned Usr_GetNumUsrsWhoClaimToBelongToCty (struct Cty_Countr *Cty);
unsigned Usr_GetCachedNumUsrsWhoClaimToBelongToCty (struct Cty_Countr *Cty);

void Usr_FlushCacheNumUsrsWhoClaimToBelongToIns (void);
unsigned Usr_GetNumUsrsWhoClaimToBelongToIns (struct Ins_Instit *Ins);
unsigned Usr_GetCachedNumUsrsWhoClaimToBelongToIns (struct Ins_Instit *Ins);

void Usr_FlushCacheNumUsrsWhoClaimToBelongToCtr (void);
unsigned Usr_GetNumUsrsWhoClaimToBelongToCtr (struct Ctr_Center *Ctr);
unsigned Usr_GetCachedNumUsrsWhoClaimToBelongToCtr (struct Ctr_Center *Ctr);

void Usr_GetListUsrs (Hie_Lvl_Level_t Scope,Rol_Role_t Role);

void Usr_SearchListUsrs (Rol_Role_t Role);
void Usr_CreateTmpTableAndSearchCandidateUsrs (const char SearchQuery[Sch_MAX_BYTES_SEARCH_QUERY + 1]);
void Usr_DropTmpTableWithCandidateUsrs (void);

void Usr_GetUnorderedStdsCodesInDeg (long DegCod);

void Usr_CopyBasicUsrDataFromList (struct UsrData *UsrDat,
                                   const struct UsrInList *UsrInList);
void Usr_FreeUsrsList (Rol_Role_t Role);

bool Usr_GetIfShowBigList (unsigned NumUsrs,
                           void (*FuncParams) (void *Args),void *Args,
                           const char *OnSubmit);

void Usr_CreateListSelectedUsrsCodsAndFillWithOtherUsr (struct SelectedUsrs *SelectedUsrs);
void Usr_PutHiddenParSelectedUsrsCods (struct SelectedUsrs *SelectedUsrs);
void Usr_GetListsSelectedEncryptedUsrsCods (struct SelectedUsrs *SelectedUsrs);

bool Usr_GetListMsgRecipientsWrittenExplicitelyBySender (bool WriteErrorMsgs);

bool Usr_FindEncryptedUsrCodsInListOfSelectedEncryptedUsrCods (const char *EncryptedUsrCodToFind,
							       struct SelectedUsrs *SelectedUsrs);
bool Usr_CheckIfThereAreUsrsInListOfSelectedEncryptedUsrCods (struct SelectedUsrs *SelectedUsrs);
unsigned Usr_CountNumUsrsInListOfSelectedEncryptedUsrCods (struct SelectedUsrs *SelectedUsrs);
void Usr_FreeListsSelectedEncryptedUsrsCods (struct SelectedUsrs *SelectedUsrs);

void Usr_GetListSelectedUsrCods (struct SelectedUsrs *SelectedUsrs,
				 unsigned NumUsrsInList,
				 long **LstSelectedUsrCods);
void Usr_FreeListSelectedUsrCods (long *LstSelectedUsrCods);

void Usr_CreateSubqueryUsrCods (long LstSelectedUsrCods[],
				unsigned NumUsrsInList,
				char **SubQueryUsrs);
void Usr_FreeSubqueryUsrCods (char *SubQueryUsrs);

void Usr_FreeListOtherRecipients (void);

void Usr_ShowFormsToSelectUsrListType (void (*FuncParams) (void *Args),void *Args);
unsigned Usr_GetColumnsForSelectUsrs (void);
void Usr_SetUsrDatMainFieldNames (void);
void Usr_WriteHeaderFieldsUsrDat (bool PutCheckBoxToSelectUsr);

void Usr_PutFormToSelectUsrsToGoToAct (struct SelectedUsrs *SelectedUsrs,
				       Act_Action_t NextAction,
				       void (*FuncParams) (void *Args),void *Args,
				       const char *Title,
                                       const char *HelpLink,
                                       const char *TxtButton,
				       bool PutFormDateRange);
void Usr_GetSelectedUsrsAndGoToAct (struct SelectedUsrs *SelectedUsrs,
				    void (*FuncWhenUsrsSelected) (void *ArgsSelected),void *ArgsSelected,
                                    void (*FuncWhenNoUsrsSelected) (void *ArgsNoSelected),void *ArgsNoSelected);
void Usr_ListUsersToSelect (Rol_Role_t Role,struct SelectedUsrs *SelectedUsrs);

void Usr_ListAllDataGsts (void);
void Usr_ListAllDataStds (void);
void Usr_ListAllDataTchs (void);
unsigned Usr_ListUsrsFound (Rol_Role_t Role,
                            const char SearchQuery[Sch_MAX_BYTES_SEARCH_QUERY]);
void Usr_ListDataAdms (void);

void Usr_PutParamsPrefsAboutUsrList (void);
void Usr_GetAndUpdatePrefsAboutUsrList (void);
void Usr_PutParamUsrListType (Usr_ShowUsrsType_t ListType);
void Usr_GetAndUpdateColsClassPhoto (void);
void Usr_PutParamColsClassPhoto (void);
void Usr_PutParamListWithPhotos (void);
void Usr_GetMyPrefAboutListWithPhotosFromDB (void);

void Usr_SeeGuests (void);
void Usr_SeeStudents (void);
void Usr_SeeTeachers (void);

void Usr_DoActionOnSeveralUsrs1 (void);
void Usr_DoActionOnSeveralUsrs2 (void);

void Usr_SeeGstClassPhotoPrn (void);
void Usr_SeeStdClassPhotoPrn (void);
void Usr_SeeTchClassPhotoPrn (void);
void Usr_PutSelectorNumColsClassPhoto (void);

void Usr_ConstructPathUsr (long UsrCod,char PathUsr[PATH_MAX + 1 + Cns_MAX_DECIMAL_DIGITS_LONG + 1]);
bool Usr_ChkIfUsrCodExists (long UsrCod);

void Usr_ShowWarningNoUsersFound (Rol_Role_t Role);

unsigned Usr_GetTotalNumberOfUsers (void);
unsigned Usr_GetNumUsrsInCrss (Hie_Lvl_Level_t Scope,long Cod,unsigned Roles);
unsigned Usr_GetCachedNumUsrsInCrss (Hie_Lvl_Level_t Scope,long Cod,unsigned Roles);

unsigned Usr_GetCachedNumUsrsNotBelongingToAnyCrs (void);

double Usr_GetCachedNumCrssPerUsr (Hie_Lvl_Level_t Scope,long Cod,Rol_Role_t Role);
double Usr_GetCachedNumUsrsPerCrs (Hie_Lvl_Level_t Scope,long Cod,Rol_Role_t Role);

bool Usr_DB_CheckIfUsrBanned (long UsrCod);
void Usr_DB_RemoveUsrFromBanned (long UsrCod);

void Usr_PrintUsrQRCode (void);

void Usr_WriteAuthor1Line (long UsrCod,bool Hidden);

void Usr_ShowTableCellWithUsrData (struct UsrData *UsrDat,unsigned NumRows);

void Usr_PutWhoIcon (Usr_Who_t Who);
void Usr_PutHiddenParamWho (Usr_Who_t Who);
Usr_Who_t Usr_GetHiddenParamWho (void);

#endif
