//      main.c
//      
//      Copyright 2009 Sataporn Saijai <nothing2give@gmail.com>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include "resource.h"
#include "avrdude_string.h"
#include "config.h"

#define BUFSIZE 4096

#define AVRDUDE_CMD_FIND_PART 1
#define AVRDUDE_CMD_FUSE_READ 2
#define AVRDUDE_CMD_FUSE_WRITE 3
#define AVRDUDE_CMD_FLASH_READ 4
#define AVRDUDE_CMD_FLASH_WRITE 5
#define AVRDUDE_CMD_FLASH_ERASE 6

HINSTANCE hInstance;
HWND hMainWindow;
HWND hProgress;
PART_INFO *partList = NULL;
PART_INFO currentPart;
char hexFileFlash[260];

BOOL CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM) ;
BOOL CALLBACK StatusDlgProc(HWND, UINT, WPARAM, LPARAM) ;
DWORD WINAPI AvrDudeThread(LPVOID) ;
int refreshPart();

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {

	printf("Welcome to USBasp GUI (beta version)\n");
	printf("Please feel free to use this program.\n\n");
	printf("Initialize common control..");
	InitCommonControls();
	printf("done.\n");
	
	// looking for avrdude.exe
	FILE *f;
	f = fopen("avrdude.exe", "r");
	if (f != NULL) {
		fclose(f);
	} else {
		MessageBox(NULL, "AVRdude executable not found. Program will now quit", "USBasp GUI", MB_OK | MB_ICONERROR);
		return 1;
	}
	
	// read config file
	printf("Reading avrdude.conf..");
	int partCount = read_config(&partList);
	if (partCount<0) {
		MessageBox(NULL, "AVRdude config file not found. Program will now quit", "USBasp GUI", MB_OK | MB_ICONERROR);
		return 1;
	}
	strcpy(currentPart.partID, "m8");
	printf("done.\n");
		
	printf("Launching USBasp_GUI..\n");
	hInstance = hInst;
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, (DLGPROC)MainDlgProc, 0);
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG: {

			ShowWindow(hwnd, SW_NORMAL);
			UpdateWindow(hwnd);
			hMainWindow = hwnd;

			// add part item
			PART_INFO *lpFirstPart = partList;
			while (lpFirstPart!=NULL) {
				printf("Adding device: %s (%s) singature = %0.6X\n", lpFirstPart->partName, lpFirstPart->partID, lpFirstPart->partSig);
				SendDlgItemMessage(hwnd, IDC_COM_PARTLIST, CB_ADDSTRING, (WPARAM)0, (LPARAM)lpFirstPart->partName);
				lpFirstPart = lpFirstPart->next;				
			}
			int selInd = SendDlgItemMessage(hMainWindow, IDC_COM_PARTLIST, CB_FINDSTRING, (WPARAM)0, (LPARAM)"-");
			SendDlgItemMessage(hMainWindow, IDC_COM_PARTLIST, CB_SETCURSEL, (WPARAM)selInd, (LPARAM)0);
			
			// add progress window
			hProgress = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_STATUS_DIALOG), hMainWindow, StatusDlgProc);

			return TRUE;
		}
		break;

		case WM_CLOSE: {
			PostQuitMessage(0);
			EndDialog(hwnd, 0);
		}
		break;
		
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				// show about dialog
				case IDC_SHOW_ABOUT_BOX: {
					DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUT_DIALOG), hwnd, (DLGPROC)AboutDlgProc, 0);
				}
				break;
				// read part
				case IDC_BUT_FINDPART: {
					HANDLE hAvrDude;
					DWORD idAvrDude;
					hAvrDude = CreateThread(
						NULL,
						0,
						AvrDudeThread,
						AVRDUDE_CMD_FIND_PART,
						0,
						&idAvrDude
					);
				}
				break;
				// read fuses
				case IDC_BUT_READFUSE: {
					HANDLE hAvrDude;
					DWORD idAvrDude;
					hAvrDude = CreateThread(
						NULL,
						0,
						AvrDudeThread,
						AVRDUDE_CMD_FUSE_READ,
						0,
						&idAvrDude
					);
				}
				break;
				// write fuses
				case IDC_BUT_WRITEFUSE: {
					HANDLE hAvrDude;
					DWORD idAvrDude;
					hAvrDude = CreateThread(
						NULL,
						0,
						AvrDudeThread,
						AVRDUDE_CMD_FUSE_WRITE,
						0,
						&idAvrDude
					);
				}
				break;
				// read flash
				case IDC_BUT_READFLASH: {
					HANDLE hAvrDude;
					DWORD idAvrDude;
					hAvrDude = CreateThread(
						NULL,
						0,
						AvrDudeThread,
						AVRDUDE_CMD_FLASH_READ,
						0,
						&idAvrDude
					);
				}
				break;
				// erase
				case IDC_BUT_ERASE: {
					HANDLE hAvrDude;
					DWORD idAvrDude;
					hAvrDude = CreateThread(
						NULL,
						0,
						AvrDudeThread,
						AVRDUDE_CMD_FLASH_ERASE,
						0,
						&idAvrDude
					);
				}
				break;
				// choose file
				case IDC_BUT_CHOOSE: {
					char szFile[260];
					memset(szFile, 0, sizeof(char)*260);					
					OPENFILENAME ofn;
					ZeroMemory(&ofn, sizeof(OPENFILENAME));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hMainWindow;
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = "Intel HEX\0*.hex\0";
					ofn.lpstrDefExt = "hex";
					ofn.nFilterIndex = 0;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = ".";
					ofn.lpstrTitle = "Open hex file..";
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
					
					if (GetOpenFileName(&ofn) == TRUE) {
						memset(hexFileFlash, 0, sizeof(char)*260);
						strcpy(hexFileFlash, szFile);
						SetDlgItemText(hwnd, IDC_FILENAME, szFile);
					}
					
				}
				break;
				// write flash
				case IDC_BUT_PROGRAM: {
					HANDLE hAvrDude;
					DWORD idAvrDude;
					hAvrDude = CreateThread(
						NULL,
						0,
						AvrDudeThread,
						AVRDUDE_CMD_FLASH_WRITE,
						0,
						&idAvrDude
					);
				}
				break;
				// exit
				case IDC_EXIT: {
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
				break;
			}
		}
		break;

		default: {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG: {
			ShowWindow(hwnd, SW_NORMAL);
			UpdateWindow(hwnd);
			return TRUE;
		}
		break;

		case WM_CLOSE: {
			EndDialog(hwnd, 0);
		}
		break;
		
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDOK: {
					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
				break;
			}
		}
		break;

		default: {
			return FALSE;
		}
	}
	return TRUE;
}

int refreshPart() {
	char partName[32];
	BOOL bFoundFlag = FALSE;
	int selInd = SendDlgItemMessage(hMainWindow, IDC_COM_PARTLIST, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	SendDlgItemMessage(hMainWindow, IDC_COM_PARTLIST, CB_GETLBTEXT, selInd, partName);
	
	PART_INFO **foundPart;
	foundPart = find_part(&partList, 0, partName, &bFoundFlag);
	
	if (bFoundFlag) {
		strcpy(currentPart.partID, &(*foundPart)->partID);
		strcpy(currentPart.partName, &(*foundPart)->partName);
		currentPart.partSig = &(*foundPart)->partSig;
	}
}

DWORD WINAPI AvrDudeThread(LPVOID lpCommand) {
	
	DWORD dwCommand = lpCommand;
	BOOL bDoNextPart = FALSE;
	CHAR cmdLine[512];
	memset(cmdLine, 0, sizeof(CHAR)*512);
	
	unsigned char hfuse;
	unsigned char lfuse;
	unsigned char efuse;
			
	switch (dwCommand) {
		// find part
		case AVRDUDE_CMD_FIND_PART: {
			sprintf(cmdLine, "avrdude -c usbasp -p m8 -U signature:r:signature:h -s -n -F");
		}
		break;
		// read fuse
		case AVRDUDE_CMD_FUSE_READ: {
			refreshPart();
			sprintf(cmdLine, "avrdude -c usbasp -p %s -U hfuse:r:hfuse:h -U lfuse:r:lfuse:h -U efuse:r:efuse:h", currentPart.partID);
		}
		break;
		// write fuse
		case AVRDUDE_CMD_FUSE_WRITE: {
			unsigned char i;
			hfuse = 0x00;
			lfuse = 0x00;
			efuse = 0x00;

			refreshPart();

			for (i=0;i<8;i++) {
				unsigned char bitvalue = (1<<i);
				if (IsDlgButtonChecked(hMainWindow, IDC_HFUSE_BIT0 + i) == BST_CHECKED)
					hfuse |= bitvalue;
				if (IsDlgButtonChecked(hMainWindow, IDC_LFUSE_BIT0 + i) == BST_CHECKED)
					lfuse |= bitvalue;
				if (IsDlgButtonChecked(hMainWindow, IDC_EFUSE_BIT0 + i) == BST_CHECKED)
					efuse |= bitvalue;
			}
			if (MessageBox(hMainWindow, "Are you sure want to write fuses", "USBasp GUI", MB_YESNO | MB_ICONINFORMATION) == IDYES) {
				sprintf(cmdLine, "avrdude -c usbasp -p %s -U hfuse:w:0x%0.2X:m -U lfuse:w:0x%0.2X:m -U efuse:w:0x%0.2X:m", currentPart.partID, hfuse, lfuse, efuse);				
			} else {
				return 1;
			}
		}
		break;
		// erase flash
		case AVRDUDE_CMD_FLASH_ERASE: {
			refreshPart();
			if (MessageBox(hMainWindow, "Are you sure want to erase device?", "USBasp GUI", MB_YESNO | MB_ICONINFORMATION) == IDYES) {
				sprintf(cmdLine, "avrdude -c usbasp -p %s -e", currentPart.partID);
			} else {
				return 1;
			}
		}
		break;
		// read flash
		case AVRDUDE_CMD_FLASH_READ: {
			refreshPart();
			sprintf(cmdLine, "avrdude -c usbasp -p %s -U flash:r:flash:i", currentPart.partID);
		}
		break;
		// write flash
		case AVRDUDE_CMD_FLASH_WRITE: {
			refreshPart();
			sprintf(cmdLine, "avrdude -c usbasp -p %s -U flash:w:\"%s\":i", currentPart.partID, hexFileFlash);
		}
		break;
		default:
			sprintf(cmdLine, "avrdude");
	}
	
	printf("CMD: %s\n", cmdLine);
	
	SECURITY_ATTRIBUTES saAttr;					
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 
	
	// Pipe output
	HANDLE hStdOut;
	HANDLE hStdErr;
	HANDLE hStdOutRd, hStdOutWr;
	HANDLE hStdErrRd, hStdErrWr;
	
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hStdErr = GetStdHandle(STD_ERROR_HANDLE);
		
	if (!CreatePipe(&hStdOutRd, &hStdOutWr, &saAttr, 0)) {
		MessageBox(hMainWindow, "Can not create Pipe", "USBasp GUI", MB_OK | MB_ICONERROR);
		return 1;
	}
	if (!CreatePipe(&hStdErrRd, &hStdErrWr, &saAttr, 0)) {
		MessageBox(hMainWindow, "Can not create Pipe", "USBasp GUI", MB_OK | MB_ICONERROR);
		return 1;
	}
	
	
	SetHandleInformation(hStdOutRd, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation(hStdErrRd, HANDLE_FLAG_INHERIT, 0);

	// Create Process
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));	
	siStartupInfo.cb = sizeof(STARTUPINFO);
	siStartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	siStartupInfo.hStdOutput = hStdOutWr;
	siStartupInfo.hStdError = hStdErrWr;
	siStartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siStartupInfo.wShowWindow = SW_HIDE;
	
	EnableWindow(hMainWindow, FALSE);
	ShowWindow(hProgress, SW_SHOW);
	SetForegroundWindow(hProgress);
	
	SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	
	if (CreateProcess(
			NULL,
			cmdLine,
			NULL,
			NULL,
			TRUE,
			CREATE_DEFAULT_ERROR_MODE,
			0,
			0,
			&siStartupInfo,
			&piProcessInfo) != FALSE) {														
		SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Starting AVRDude..");
		SetDlgItemText(hProgress, IDC_STATUS_TEXT, "AVRdude Start..");
		CloseHandle(piProcessInfo.hProcess);
		CloseHandle(piProcessInfo.hThread);
	} else {
		MessageBox(hMainWindow, "Process failed to launch", "USBasp GUI", MB_OK | MB_ICONERROR);
		return 1;
	}
	
	DWORD dwRead, dwWritten; 
	CHAR errBuf[10];
	CHAR errLine[BUFSIZE];
	CHAR outBuf[10];
	CHAR outLine[BUFSIZE];
	int iErr = 0;
	int iOut = 0;
	BOOL endErr = FALSE;
	BOOL endOut = FALSE;
	memset(errBuf, 0, 2);
	memset(errLine, 0, BUFSIZE);

	if (!CloseHandle(hStdErrWr)) {
		MessageBox(hMainWindow, "Closing handle failed", "USBasp GUI", MB_OK);
		return 1;
	}
	if (!CloseHandle(hStdOutWr)) {
		MessageBox(hMainWindow, "Closing handle failed", "USBasp GUI", MB_OK);
		return 1;
	}

	iErr = 0;
	iOut = 0;
	bDoNextPart = TRUE;
	for (;;)  {
		// read STDOUT	
		/*	
		if (ReadFile(hStdOutRd, outBuf, 1, &dwRead, NULL)) {
			if (outBuf[0]=='\n') {
				if (iOut>0) {
					outLine[iOut-1] = '\0';
				} else {
					outLine[iOut] = '\0';
				}
				if (iOut>1) {
					printf("STDOUT: >%s<\n", outLine);
				}
			} else {
				outLine[iOut] = outBuf[0];
				iOut++;
			}
		} else {
			endOut = TRUE;
			if (endOut&&endErr)
				break;
		}
		*/
		endOut = TRUE;
		
		// read STDERR
		if (ReadFile(hStdErrRd, errBuf, 1, &dwRead, NULL)) {
			if (errBuf[0]=='\n') {
				if (iErr>0) {
					errLine[iErr-1] = '\0';
				} else {
					errLine[iErr] = '\0';
				}
				if (iErr>1) {
					printf("LINE: >>%s<<\n", errLine);
					
					// identify avrdude message
					if (sscanf(errLine, AVRDUDE_SIGNATURE_READ) > 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Reading SIGNATURE..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Reading SIGNATURE..");
					}
					if (strcmpi(errLine, AVRDUDE_HFUSE_READ) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Reading HFUSE..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Reading HFUSE..");
					}
					if (strcmpi(errLine, AVRDUDE_LFUSE_READ) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Reading LFUSE..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Reading LFUSE..");
					}
					if (strcmpi(errLine, AVRDUDE_EFUSE_READ) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Reading EFUSE..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Reading EFUSE..");
					}
					if (strcmpi(errLine, AVRDUDE_EEPROM_READ) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Reading EEPROM..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Reading EEPROM..");
					}
					if (strcmpi(errLine, AVRDUDE_FLASH_READ) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Reading FLASH..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Reading FLASH..");
					}

					if (strcmpi(errLine, AVRDUDE_HFUSE_WRITE) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Writing HFUSE..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Writing HFUSE..");
					}
					if (strcmpi(errLine, AVRDUDE_LFUSE_WRITE) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Writing LFUSE..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Writing LFUSE..");
					}
					if (strcmpi(errLine, AVRDUDE_EFUSE_WRITE) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Writing EFUSE..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Writing EFUSE..");
					}
					if (strcmpi(errLine, AVRDUDE_EEPROM_WRITE) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Writing EEPROM..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Writing EEPROM..");
					}
					if (sscanf(errLine, AVRDUDE_FLASH_WRITE, &dwWritten) > 0) {
						char bufText[128];
						memset(bufText, 0, sizeof(char)*128);
						sprintf(bufText, "Writing FLASH (%u bytes)..", dwWritten);
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, bufText);
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)bufText);
					}
					if (strcmpi(errLine, AVRDUDE_FLASH_ERASE) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Erasing FLASH..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Erasing FLASH..");
					}
					if (strcmpi(errLine, AVRDUDE_FLASH_VERIFY) == 0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Verifying FLASH..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Verifying FLASH..");
					}
					
					// if process is done
					if (strcmp(errLine, AVRDUDE_DONE)==0) {
						SetDlgItemText(hProgress, IDC_STATUS_TEXT, "Done..");
						SendDlgItemMessage(hMainWindow, IDC_AVRDUDELOG, LB_INSERTSTRING, (WPARAM)-1, "Done!");
					}
					
					// CRITICAL MESSAGE
					
					// if programmer not found
					if ((strcmp(errLine, AVRDUDE_NOTFOUND)==0) &&
						(bDoNextPart == TRUE)) {
						MessageBox(hProgress, "USBasp not found, please check your connection", "USBasp GUI", MB_OK | MB_ICONERROR);
						bDoNextPart = FALSE;
					}
					// if part signature is wrong
					if ((strcmp(errLine, AVRDUDE_WRONGPART_1)==0) ||
						(strcmp(errLine, AVRDUDE_WRONGPART_2)==0) ||
						(strcmp(errLine, AVRDUDE_INVALIDSIG)==0) ||
						(sscanf(errLine, AVRDUDE_UNEXPECTEDPART) > 0)) {
						if (dwCommand != AVRDUDE_CMD_FIND_PART) {
							MessageBox(hProgress, "USBasp reported device signature is incorrect, the cause of problem may be:\n- AVR not connected\n- AVR connected to programmer is malfunction\n- select wrong AVR part", "USBasp GUI", MB_OK | MB_ICONERROR);
							bDoNextPart = FALSE;
						} else {
							// normal operation
						}
					}
					// forgot to select AVR part
					if ((strcmp(errLine, AVRDUDE_SELECTPART)==0) &&
						(bDoNextPart == TRUE)) {
						MessageBox(hProgress, "Please select AVR part", "USBasp GUI", MB_OK | MB_ICONINFORMATION);
						bDoNextPart = FALSE;
					}					
				}
				iErr = 0;
			} else {
				errLine[iErr] = errBuf[0];
				iErr++;
			}
		} else {
			endErr = TRUE;
			if (endOut&&endErr)
				break;
		}
	}
	
	TerminateProcess(piProcessInfo.hProcess, 0);
	
	if (bDoNextPart) {
		// process signature
		if (dwCommand == AVRDUDE_CMD_FIND_PART) {
			BOOL isMCUFound = FALSE;
			FILE *f;
			f = fopen("signature", "r");
			if (f != NULL) {
				unsigned char sx[3];
				unsigned long sig;
				BOOL bFoundFlag = FALSE;
				char bufSig[128];
				memset(bufSig, 0, sizeof(char)*128);
				fgets(bufSig, 128, f);
				fclose(f);
				DeleteFile("signature");				
				sscanf(bufSig, "%x,%x,%x", &sx[0], &sx[1], &sx[2]);
				sig = (sx[0]<<16) + (sx[1]<<8) + sx[2];

				PART_INFO **foundPart;
				foundPart = find_part(&partList, sig, "", &bFoundFlag);
				
				if (bFoundFlag) {
					strcpy(currentPart.partID, &(*foundPart)->partID);
					strcpy(currentPart.partName, &(*foundPart)->partName);
					currentPart.partSig = &(*foundPart)->partSig;
					int selInd = SendDlgItemMessage(hMainWindow, IDC_COM_PARTLIST, CB_FINDSTRING, (WPARAM)0, (LPARAM)&(*foundPart)->partName);
					SendDlgItemMessage(hMainWindow, IDC_COM_PARTLIST, CB_SETCURSEL, (WPARAM)selInd, (LPARAM)0);
					SendMessage(hMainWindow, WM_COMMAND, (WPARAM)IDC_BUT_READFUSE, (LPARAM)0);
					isMCUFound = TRUE;
				} else {
					strcpy(currentPart.partID, "");
					strcpy(currentPart.partName, "");
					currentPart.partSig = 0x000000;
					currentPart.next = NULL;
				}

			} else { // part not found
			}
			if (!isMCUFound) {
				MessageBox(hProgress, "Can not detect AVR part, your connection may be broken or AVR part is not supported by this version of avrdude.", "USBasp GUI", MB_OK | MB_ICONERROR);
			}
			
		}
		
		// process fuses
		hfuse = 0x00;
		lfuse = 0x00;
		efuse = 0x00;
		if (dwCommand == AVRDUDE_CMD_FUSE_READ) {
			unsigned char i;
			FILE *f;
	
			for (i=0;i<8;i++) {
				CheckDlgButton(hMainWindow, IDC_HFUSE_BIT0 + i, BST_UNCHECKED);
				CheckDlgButton(hMainWindow, IDC_LFUSE_BIT0 + i, BST_UNCHECKED);
				CheckDlgButton(hMainWindow, IDC_EFUSE_BIT0 + i, BST_UNCHECKED);
			}
			
			f = fopen("hfuse", "r");
			if (f != NULL) {
				fscanf(f, "0x%X", &hfuse);
				fclose(f);
				for (i=0;i<8;i++) {
					unsigned char bitvalue = (1<<i);
					CheckDlgButton(hMainWindow, IDC_HFUSE_BIT0 + i, ((hfuse&bitvalue)==bitvalue)?BST_CHECKED:BST_UNCHECKED);
				}
				DeleteFile("hfuse");
			}
			
			f = fopen("lfuse", "r");
			if (f != NULL) {
				fscanf(f, "0x%X", &lfuse);
				fclose(f);
				for (i=0;i<8;i++) {
					unsigned char bitvalue = (1<<i);
					CheckDlgButton(hMainWindow, IDC_LFUSE_BIT0 + i, ((lfuse&bitvalue)==bitvalue)?BST_CHECKED:BST_UNCHECKED);
				}
				DeleteFile("lfuse");
			}
			
			f = fopen("efuse", "r");
			if (f != NULL) {
				fscanf(f, "0x%X", &efuse);
				fclose(f);
				for (i=0;i<8;i++) {
					unsigned char bitvalue = (1<<i);
					CheckDlgButton(hMainWindow, IDC_EFUSE_BIT0 + i, ((efuse&bitvalue)==bitvalue)?BST_CHECKED:BST_UNCHECKED);
				}
				DeleteFile("efuse");
			}
		}
		
		// process read flash
		if (dwCommand == AVRDUDE_CMD_FLASH_READ) {
			char szFile[512];
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hMainWindow;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Intel HEX\0*.hex\0";
			ofn.lpstrDefExt = "hex";
			ofn.nFilterIndex = 0;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = ".";
			ofn.lpstrTitle = "Save read flash as..";
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

			if (GetSaveFileName(&ofn) == TRUE) {
				printf("Save file as %s\n", ofn.lpstrFile);
				MoveFileEx("flash", ofn.lpstrFile, MOVEFILE_REPLACE_EXISTING);
			}			
			DeleteFile("flash");
	
		}
	}

	ShowWindow(hProgress, SW_HIDE);
	EnableWindow(hMainWindow, TRUE);
	SetForegroundWindow(hMainWindow);
	
	ExitThread(0);
			
	return 0;
}

BOOL CALLBACK StatusDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG: {
			return TRUE;
		}
		break;
		
		case WM_SHOWWINDOW: {
			HWND hWnd = GetParent(hwnd);
			RECT r1, r2;
			GetClientRect(hWnd, &r1);
			GetWindowRect(hwnd, &r2);
			POINT pt;
			pt.x = (r1.right - r1.left)/2 - (r2.right - r2.left)/2;
			pt.y = (r1.bottom - r1.top)/4 - (r2.bottom - r2.top)/2;
			ClientToScreen(hWnd, &pt);
			SetWindowPos(hwnd, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE);
		}
		break;

		case WM_CLOSE: {
			EndDialog(hwnd, 0);
		}
		break;
		
		default: {
			return FALSE;
		}
	}
	return TRUE;
}
