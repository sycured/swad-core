// swad_centre_config.h: configuration of current centre

#ifndef _SWAD_CTR_CFG
#define _SWAD_CTR_CFG
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

/*****************************************************************************/
/************************** Public types and constants ***********************/
/*****************************************************************************/

/*****************************************************************************/
/****************************** Public prototypes ****************************/
/*****************************************************************************/

void CtrCfg_ShowConfiguration (void);
void CtrCfg_PrintConfiguration (void);

void CtrCfg_RequestLogo (void);
void CtrCfg_ReceiveLogo (void);
void CtrCfg_RemoveLogo (void);
void CtrCfg_RequestPhoto (void);
void CtrCfg_ReceivePhoto (void);
void CtrCfg_ChangeCtrPhotoAttribution (void);
void CtrCfg_ChangeCtrIns (void);
void CtrCfg_RenameCentreShort (void);
void CtrCfg_RenameCentreFull (void);
void CtrCfg_ChangeCtrPlc (void);
void CtrCfg_ChangeCtrLatitude (void);
void CtrCfg_ChangeCtrLongitude (void);
void CtrCfg_ChangeCtrAltitude (void);
void CtrCfg_ChangeCtrWWW (void);
void CtrCfg_ContEditAfterChgCtr (void);

#endif
