// swad_indicator.h: indicators of courses

#ifndef _SWAD_IND
#define _SWAD_IND
/*
    SWAD (Shared Workspace At a Distance in Spanish),
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

/*****************************************************************************/
/************************** Public types and constants ***********************/
/*****************************************************************************/

#define Ind_NUM_INDICATORS 5
#define Ind_MAX_SIZE_INDICATORS_SELECTED ((1 + Ind_NUM_INDICATORS) * (10 + 1))

struct Ind_IndicatorsCrs
  {
   unsigned long NumFilesInDocumentZones;
   unsigned long NumFilesInSharedZones;
   bool ThereIsSyllabus;
   Inf_InfoSrc_t TeachingGuideSrc;
   Inf_InfoSrc_t SyllabusLecSrc;
   Inf_InfoSrc_t SyllabusPraSrc;
   bool ThereAreAssignments;
   unsigned NumAssignments;
   unsigned long NumFilesAssignments;
   unsigned long NumFilesWorks;
   bool ThereIsOnlineTutoring;
   unsigned NumThreads;
   unsigned NumPosts;
   unsigned NumUsrsToBeNotifiedByEMail;
   unsigned NumMsgsSentByTchs;
   bool ThereAreMaterials;
   bool ThereIsAssessment;
   Inf_InfoSrc_t AssessmentSrc;
   unsigned NumIndicators;
   bool CoursePartiallyOK;
   bool CourseAllOK;
  };

/*****************************************************************************/
/***************************** Public prototypes *****************************/
/*****************************************************************************/

void Ind_ReqIndicatorsCourses (void);
void Ind_ShowIndicatorsCourses (void);
int Ind_GetNumIndicatorsCrsFromDB (long CrsCod);
void Ind_ComputeAndStoreIndicatorsCrs (long CrsCod,int NumIndicatorsFromDB,
                                       struct Ind_IndicatorsCrs *Indicators);

#endif
