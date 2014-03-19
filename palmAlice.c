/* Copyright (c) 2000-2002, Jacco Bikker
 * Copyright (c) 2000-2001, Ben Burton
 * Copyright (c) 2002, Robert Bjarnason
 * Copyright (c) 2002, InOrbit Entertainment, Inc.
 *
 * This file is part of PalmAlice.
 * PalmAlice is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the 
 * Free Software Foundation; either version 2, or (at your option) any later 
 * version.
 *
 * PalmAlice is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.

 * You should have received a copy of the GNU General Public License along 
 * with PalmAlice; see the file COPYING. If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <PalmOS.h>
#include <PalmCompatibility.h>

#include "palmAliceRsc.h"	
#include "palmAlice.h"

#define ourMinVersion	sysMakeROMVersion(3,1,0,sysROMStageRelease,0)
#define sysVersion35	sysMakeROMVersion(3,5,0,sysROMStageRelease,0)

CurrentAliceData currentAlice;

FormPtr mainForm;
FieldPtr aliceField;
FieldPtr aliceInputField;
Word aliceFieldIndex;
Word aliceInputFieldIndex;

Boolean HasLoadedVars = false;

UInt16	MinMemory = 1840;
UInt16	MediumMemory = 1840;
UInt16	MaxMemory = 2600;
Boolean RunningInLowMemMode = false;
UInt32 depth;

UInt32	LastImageSwapTime=0;

void showAlice(AliceContents* contents) {
	FieldAttrType attr;

	if (! mainForm)
		return;

	FrmHideObject(mainForm, aliceFieldIndex);

	FldGetAttributes(aliceField, &attr);
	attr.editable = 0;
	FldSetAttributes(aliceField, &attr);

	if (contents && contents->alice) {
		UInt aliceLen;
		CharPtr alice;
		alice = MemHandleLock(contents->alice);
		aliceLen = StrLen(alice);
		FldDelete(aliceField, 0, FldGetTextLength(aliceField));
		FldSetMaxChars(aliceField, aliceLen);
		FldInsert(aliceField, alice, aliceLen);
		FldSetInsertionPoint(aliceField, 0);
		MemHandleUnlock(contents->alice);
	} else {
		FldDelete(aliceField, 0, FldGetTextLength(aliceField));
		FldSetMaxChars(aliceField, 0);
	}
	
	attr.editable = 0;
	FldSetAttributes(aliceField, &attr);

	FrmShowObject(mainForm, aliceFieldIndex);
}

void showCurrentAlice() { 

	// Is there a current alice at all?
	if (currentAlice.exists) {
		AliceContents contents;

		if (currentAlice.exists)
			showAlice(&contents);
		else
			showAlice(0);
	} else
		showAlice(0);
}


/* Save preferences, close forms, close app database */
static void StopApplication(void)
{
	// Save the current alice.
	PrefSetAppPreferences(palmDBCreator, prefsCurrentAlice,
		palmVersion, &currentAlice, sizeof(CurrentAliceData), false);

	saveinitstate();

	// Other shutdown.
	FrmSaveAllForms();
	FrmCloseAllForms();
	CleanInit();
}

static Boolean MainFormHandleEvent (EventPtr e)
{
	Boolean handled = false;
	FormPtr frm;

	MemHandle inputHandle;

	switch (e->eType) {
		case frmOpenEvent:
			frm = FrmGetActiveForm();
			FrmDrawForm(frm);
			handled = true;
			if (!HasLoadedVars) 
			{
				initialize();
				HasLoadedVars=true;
			}

			greeting();

			// Set the form's focus to the field
			FrmSetFocus(frm, aliceInputFieldIndex);	
			break;

		case menuEvent:
			MenuEraseStatus(NULL);

			switch(e->data.menu.itemID) {
			}

			handled = true;
			break;

		case ctlSelectEvent:
			switch(e->data.ctlSelect.controlID) {
				case idAboutBtn:
					FrmAlert(idAboutBox);
					break;
				case idChatBtn:
					if (FldGetTextPtr(aliceInputField)==NULL) break;
					print( "" );
					DrawRandomImage(true);
					
					star[0] = 0;
					star2[0] = 0;
					user_input[0] = 0;
					
					pateval = 0;
					recurs = 0;

					inputHandle = FldGetTextHandle(aliceInputField);
										
					StrCopy(user_input,  FldGetTextPtr( aliceInputField ));

					respond( user_input, "alice" );
					
					printbSwap();

					FldSetTextHandle(aliceInputField, NULL);
					FldDrawField(aliceInputField); 
	
					if (inputHandle) 
						MemHandleFree(inputHandle);
//					if (exitplease) StopApplication(); 
					break;
			}
			break;

/*		case penDownEvent:
			frm = FrmGetActiveForm();
			FrmSetFocus(frm, aliceInputFieldIndex);
			break;
*/
		case keyDownEvent:
			switch(e->data.keyDown.chr) {
				case linefeedChr:
					if (FldGetTextPtr(aliceInputField)==NULL) break;
					
					print( "" );
					DrawRandomImage(true);

					star[0] = 0;
					star2[0] = 0;
					user_input[0] = 0;
					pateval = 0;
					recurs = 0;

					inputHandle = FldGetTextHandle(aliceInputField);
										
					StrCopy(user_input,  FldGetTextPtr( aliceInputField ));

					respond( user_input, "alice" );
					
					printbSwap();
					
					FldSetTextHandle(aliceInputField, NULL);
					FldDrawField(aliceInputField); 
	
					if (inputHandle) 
						MemHandleFree(inputHandle);
					break;				
				case pageUpChr:
				case pageDownChr:
				if (! mainForm)
					break;
				break;
			}
			break;

		default:
			break;
	}

	return handled;
}

/* Provide default actions when forms are loaded. */
static Boolean ApplicationHandleEvent(EventPtr e)
{
	FormPtr frm;
	Boolean handled = false;

	if (e->eType == frmLoadEvent) {
		Word formId;
		FormPtr frm;

		formId = e->data.frmLoad.formID;
		frm = FrmInitForm(formId);
		FrmSetActiveForm(frm);

		switch(formId) {
			case idMainForm:
				FrmSetEventHandler(frm, MainFormHandleEvent);

				// Set global variables.
				mainForm = frm;
				aliceFieldIndex = FrmGetObjectIndex(mainForm, idAliceField);
				aliceInputFieldIndex = FrmGetObjectIndex(mainForm, idAliceInputField);
				
				aliceField = FrmGetObjectPtr(mainForm, aliceFieldIndex);
				aliceInputField = FrmGetObjectPtr(mainForm, aliceInputFieldIndex);

				showCurrentAlice();
				break;
		}
		handled = true;
	} else if (e->eType == penUpEvent)
	{
		frm = FrmGetActiveForm();
		FrmSetFocus(frm, aliceInputFieldIndex);
	}

	return handled;
}

/* Get preferences, open (or create) app database */
static Word StartApplication(void)
{
	Word prefsSize;
	SWord prefsReturn;

	// Get the current alice if any.
	prefsSize = sizeof(CurrentAliceData);
	prefsReturn = PrefGetAppPreferences(palmDBCreator, prefsCurrentAlice,
		&currentAlice, &prefsSize, false);
	if (prefsReturn == noPreferenceFound)
		currentAlice.exists = false;
	
	// Do other initialisation.
	mainForm = 0;
	aliceField = 0;
	InitAlice();
    FrmGotoForm(idMainForm);
    return 0;
}

/* The main event loop */
static void EventLoop(void)
{
	Word err;
	EventType e;

	do {
		EvtGetEvent(&e, evtWaitForever);
		if (! SysHandleEvent (&e))
			if (! MenuHandleEvent (NULL, &e, &err))
				if (! ApplicationHandleEvent (&e))
					FrmDispatchEvent (&e);
	} while (e.eType != appStopEvent);
}

static UInt32 GetOSFreeMem( UInt32* totalMemoryP, UInt32* dynamicMemoryP )
{
	#define		memoryBlockSize		(1024L)
	UInt32		heapFree;
	UInt32		max;
	Int16		i;
	Int16		nCards;
	UInt16		cardNo;
	UInt16		heapID;
	UInt32		freeMemory = 0;
	UInt32		totalMemory = 0;
	UInt32		dynamicMemory = 0;

	// Iterate through each card to support devices with multiple cards.
	nCards = MemNumCards();		
	for (cardNo = 0; cardNo <nCards; cardNo++)
	{
		// Iterate through the RAM heaps on a card (excludes ROM).
		for (i=0; i< MemNumRAMHeaps(cardNo); i++)
		{
			// Obtain the ID of the heap.
			heapID = MemHeapID(cardNo, i);
			
			// If the heap is dynamic, increment the dynamic memory total.
			if (MemHeapDynamic(heapID))
			{
				dynamicMemory += MemHeapSize(heapID);
			}
			
			// The heap is nondynamic.
			else
			{
				// Calculate the total memory and free memory of the heap.
				totalMemory += MemHeapSize(heapID);
				MemHeapFreeBytes(heapID, &heapFree, &max);
				freeMemory += heapFree;
			}
		}
	}
	
	// Reduce the stats to KB.  Round the results.
	freeMemory  = freeMemory / memoryBlockSize;
	totalMemory = totalMemory  / memoryBlockSize;
	dynamicMemory = dynamicMemory / memoryBlockSize;

	if (totalMemoryP)	*totalMemoryP = totalMemory;
	if (dynamicMemoryP)	*dynamicMemoryP = dynamicMemory;

	return (freeMemory);
}	// GetOSFreeMem


void DrawImage(int x, int y, UInt16 RscId)
{
    MemHandle h;
    BitmapPtr bitmap;
    		
//	if (!RunningInLowMemMode)
//	{

	if (depth==1)
	{
		h = DmGetResource(bitmapRsc, RscId+1);
		bitmap = MemHandleLock(h);
		WinDrawBitmap(bitmap, x, y);
		MemPtrUnlock(bitmap);
		DmReleaseResource(h);
	} else if (depth==2 || depth==4)
	{
		h = DmGetResource(bitmapRsc, RscId+1);
		bitmap = MemHandleLock(h);
		WinDrawBitmap(bitmap, x, y);
		MemPtrUnlock(bitmap);
		DmReleaseResource(h);
	}
	else if (depth==8)
	{
		h = DmGetResource(bitmapRsc, RscId);
		bitmap = MemHandleLock(h);
		WinDrawBitmap(bitmap, x, y);
		MemPtrUnlock(bitmap);
		DmReleaseResource(h);
	}
//	}
//	else
//	{
//			h = DmGetResource(bitmapRsc, RscId+1);
//			bitmap = MemHandleLock(h);
//			WinDrawBitmap(bitmap, x, y);
//			MemPtrUnlock(bitmap);
//			DmReleaseResource(h);
//	}
		
}

void DrawSplashScreen()
{
    DrawImage(0,0,729);
}

void DrawRandomImage(Boolean force)
{
	unsigned int x;
	UInt32 theTime;

	theTime = TimGetSeconds();
	
	if (((theTime-LastImageSwapTime)<1) && !force) return;
	
	x = RandomNum(8);
	
	if (x==0)
		DrawImage(48,48,420);
	else if (x==1)
		DrawImage(48,48,430);
	else if (x==2)
		DrawImage(48,48,440);
	else if (x==3)
		DrawImage(48,48,450);
	else if (x==4)
		DrawImage(48,48,460);
	else if (x==5)
		DrawImage(48,48,470);
	else if (x==6)
		DrawImage(48,48,480);
	else if (x==7)
		DrawImage(48,48,490);
	LastImageSwapTime=theTime;
}
	
void print(Char *newStr)
{
	MemHandle     oldTxtHandle;
	MemHandle     txtHandle;

	txtHandle = MemHandleNew(StrLen(newStr) + 1);
	StrCopy(MemHandleLock(txtHandle), newStr);
	MemHandleUnlock(txtHandle);

	oldTxtHandle = FldGetTextHandle(aliceField);

	FldSetTextHandle(aliceField, txtHandle);
	if (oldTxtHandle)
		MemHandleFree(oldTxtHandle);
	FldDrawField(aliceField); 
}

void printb(Char *newString)
{
	StrCopy(outputBuffer, newString);
}

void printbMore(Char *newString)
{
//	StrPrintF( outputBuffer,"%s%s", outputBuffer, newString);
	StrCat( outputBuffer, newString);
}

void printbSwap()
{
	print(outputBuffer);
	StrCopy(outputBuffer, "");
}

void printMore(Char *newStr)
{
	MemHandle     oldTxtHandle;
	MemHandle     txtHandle;
	Char oldTxt[500];
	Int16 result;
	Char combinedStr[500];

	oldTxtHandle = FldGetTextHandle(aliceField);
	StrCopy( oldTxt, FldGetTextPtr( aliceField ) );

	result = StrPrintF( combinedStr,"%s%s", oldTxt, newStr);

	txtHandle = MemHandleNew(StrLen(combinedStr) + 1);
	StrCopy(MemHandleLock(txtHandle), combinedStr);
	MemHandleUnlock(txtHandle);

	FldSetTextHandle(aliceField, txtHandle);
	FldDrawField(aliceField); 
	if (oldTxtHandle) 
		MemHandleFree(oldTxtHandle);
}

Boolean FGetS(FileHand stream, Char* buffer, Int32 objSize)
{
	Int32	result,oldpos,newpos,fileSize;
	char* foundEnd;
	
	buffer[0]=0;
	
	oldpos = FileTell (stream, &fileSize, NULL);
	
	if (oldpos<fileSize)
	{	
		result = FileRead( stream, readBuffer, objSize-1, 1, NULL );
	
//		*StrStr(readBuffer, "\r") = ' ';
		foundEnd = StrStr(readBuffer, "\n");
		
		if (foundEnd) 
			*StrStr(readBuffer, "\n") = 0;
		else
		{
			readBuffer[objSize-23] = ' ';
			readBuffer[objSize-22] = ' ';
			readBuffer[objSize-21] = ' ';
			readBuffer[objSize-20] = ' ';
			readBuffer[objSize-19] = ' ';
			readBuffer[objSize-18] = ' ';
			readBuffer[objSize-17] = '<';
			readBuffer[objSize-16] = '/';
			readBuffer[objSize-15] = 'l';
			readBuffer[objSize-14] = 'i';
			readBuffer[objSize-13] = '>';
			readBuffer[objSize-12] = '<';
			readBuffer[objSize-11] = '/';
			readBuffer[objSize-10] = 'r';
			readBuffer[objSize-9] = 'a';
			readBuffer[objSize-8] = 'n';
			readBuffer[objSize-7] = 'd';
			readBuffer[objSize-6] = 'o';
			readBuffer[objSize-5] = 'm';
			readBuffer[objSize-4] = '>';
			readBuffer[objSize-3] = ' ';
			readBuffer[objSize-2] = 0;
		}
		
		StrCopy(buffer, readBuffer);

		newpos = StrLen(readBuffer)+oldpos+1;
		
		FileSeek(stream, newpos, fileOriginBeginning);
		return true;
	}
	else
	{
		return false;
	}
}	

unsigned long RandomNum(unsigned long n)
{
	static Boolean initialized = false;

	unsigned long x;
	if (initialized == false) {
		initialized = true;
		SysRandom(TimGetTicks());
	}
	if (n == 0)
	n = 1;

	x = SysRandom(0) ;
	x = (double) x / ( (double) (1 + sysRandomMax / n )) ;

	return x;
}

void InitAlice()
{
	int i, initcount;
    DWord requiredDepth;
    Err err;
    FileHand mood, moodcalcfile, substitutesfile;
	char* tag;
	UInt32 romVersion;
	UInt32 totalMemory;
	UInt32 totalDynamicMemory;
	UInt32 totalFreeMemory;
	
	// first, what OS are we on? 
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	
	if (romVersion<sysVersion35)
	{
		MinMemory = 512;
		MediumMemory = 520;
		MaxMemory = 800;
		RunningInLowMemMode = true;
	} else
	{
		totalFreeMemory = GetOSFreeMem(&totalMemory, &totalDynamicMemory);
		if (totalMemory<3000)
		{
				MinMemory = 250;
				MediumMemory = 300;
				MaxMemory = 500;
				RunningInLowMemMode = true;
		}
	}

	// now, what color depth are we in?
	WinScreenMode(winScreenModeGet, 0, 0, &depth, 0);
	
	if (depth<2)
	{
		requiredDepth = 2;
	 	err = ScrDisplayMode(scrDisplayModeSet, NULL, NULL, &requiredDepth, NULL);
	 	if (!err) depth=2;
	}
		
	DrawSplashScreen();
	
	star = (Char *) MemPtrNew(MinMemory);
	star2 = (Char *) MemPtrNew(MinMemory);
	second = (Char *) MemPtrNew(MediumMemory);
	that = (Char *) MemPtrNew(MinMemory);
	justthat = (Char *) MemPtrNew(MinMemory);
	beforethat = (Char *) MemPtrNew(MinMemory);
	bufferMain = (Char *) MemPtrNew(MinMemory);
	pluseoln= (Char *) MemPtrNew(MediumMemory);
	texteoln= (Char *) MemPtrNew(MediumMemory);
	user_input= (Char *) MemPtrNew(MinMemory);
	token= (char**) MemPtrNew((64 + 1)*4);
	
	setvars = (char**) MemPtrNew((NUM_VARS + 1)*4);
	getvars = (char**) MemPtrNew((NUM_VARS + 1)*4);
	defaultvar = (char**) MemPtrNew((NUM_VARS + 1)*4);
	var = (char**) MemPtrNew((NUM_VARS + 1)*4);
	saves = (char**) MemPtrNew(11*4);
	
	moodvar = (char**) MemPtrNew((198 + 1)*4);
	moodcalc = (char**) MemPtrNew((148 + 1)*4);
	substitutes = (char**) MemPtrNew(169*4);
	
	tag = (Char *) MemPtrNew(42);
	readBuffer = (Char *) MemPtrNew(MediumMemory);
	replaceBuffer = (Char *) MemPtrNew(MaxMemory);
	
	part1 = (Char *) MemPtrNew(MediumMemory);
	part2 = (Char *) MemPtrNew(MediumMemory);
	part3 = (Char *) MemPtrNew(MediumMemory);
	reetemp = (Char *) MemPtrNew(MediumMemory);
	reebuffer = (Char *) MemPtrNew(MediumMemory);
	fname = (Char *) MemPtrNew(128);
	tempsaves = (Char *) MemPtrNew(MediumMemory);
	
	spaces = (Char *) MemPtrNew(MediumMemory);

	rstring = (Char *) MemPtrNew(MaxMemory);
	randbuffer = (Char *) MemPtrNew(MaxMemory);

	respondposHandle = MemHandleNew(MediumMemory);

	substbuffer = (Char *) MemPtrNew(MediumMemory);
	substline = (Char *) MemPtrNew(MediumMemory);
	subststring1 = (Char *) MemPtrNew(MediumMemory);
	subststring2 = (Char *) MemPtrNew(MediumMemory);
	
	personBuffer = (Char *) MemPtrNew(MinMemory);
	
	cleanerBuffer = (Char *) MemPtrNew(128);
	outputBuffer = (Char *) MemPtrNew(MediumMemory);

	removeTemp1 = (Char *) MemPtrNew(MaxMemory);

	for ( i = 0; i < 199; i++ )
	{
		char* c = (Char *) MemPtrNew(42);
		moodvar[i] = c;
	}

	for ( i = 0; i < 148; i++ )
	{
		char* c = (Char *) MemPtrNew(21);
		moodcalc[i] = c;
	}

	for ( i = 0; i < 169; i++ )
	{
		char* c = (Char *) MemPtrNew(42);
		substitutes[i] = c;
	}
	
	for ( i = 0; i < 64; i++ )
	{
		char* c = (Char *) MemPtrNew(64+1);
		token[i] = c;
	}

	
	for ( i = 0; i < 10; i++ ) 
	{
		char* c = (Char *) MemPtrNew(128+1);
		saves[i] = c;
		sage[i] = 1000;
	}

	for ( i = 0; i < NUM_VARS + 1; i++ )
	{
		if (i==0)
			var[i] = (Char *) MemPtrNew(256);
		else
			var[i] = (Char *) MemPtrNew(256);
		setvars[i] = (Char *) MemPtrNew(256);
		getvars[i] = (Char *) MemPtrNew(256);
		defaultvar[i] = (Char *) MemPtrNew(256);
	}

	for ( i = 0; i < 32; i++ )
	{
		tx[i] = ty[i] = tc[i] = 0;
		tx2[i] = ty2[i] = 0;
	}
	
	thisrun = TimGetSeconds();
	
	second[0] = 0;
	outputBuffer[0] = 0;

	exitplease = newvarfile = false;
	repetitive = false;
	happyness = helpfullness = humor = affection = trust = interest = 0;
	curt = 0;

	chunk[0] = 0; //A
	chunk[1] = 24747; //B
	chunk[2] = 27149; //C
	chunk[3] = 39059; //D
	chunk[4] = 72269; //E
	chunk[5] = 73726; //F
	chunk[6] = 75501; //G
	chunk[7] = 78866; //H
	chunk[8] = 93410; //I
	chunk[9] = 137664; //J
	chunk[10] = 138588; //K
	chunk[11] = 139154; //L
	chunk[12] = 140868; //M
	chunk[13] = 144745; //N
	chunk[14] = 149007; //O
	chunk[15] = 150512; //P
	chunk[16] = 151926; //Q
	chunk[17] = 151990; //R
	chunk[18] = 153412; //S
	chunk[19] = 157908; //T
	chunk[20] = 170322; //U
	chunk[21] = 170921; //V
	chunk[22] = 171230; //W
	chunk[23] = 231644; //Y
	chunk[24] = 254799; //X
	chunk[25] = 254799; //Z
	chunk[26] = 254814; //_
	// chunk[26] = 254356; //1
	// chunk[27] = 254597; //2
	// chunk[28] = 254800; //3
	// chunk[29] = 254883; //4
	// chunk[30] = 255033; //5
	// chunk[31] = 255043; //6
	// chunk[32] = 255076; //8
	// chunk[33] = 255097; //9
	
	mood = FileOpen(0, "character", palmDBType, palmDBCreator, fileModeReadOnly, NULL);

	initcount=0;
	while (FGetS(mood, tag, 44))
	{
		StrCopy(moodvar[initcount], tag);
		initcount++;
	}
	FileClose(mood);

	moodcalcfile = FileOpen(0, "taglist", palmDBType, palmDBCreator, fileModeReadOnly, NULL);

	initcount=0;
	while (FGetS(moodcalcfile, tag, 21) && initcount<148)
	{
		StrCopy(moodcalc[initcount], tag);
		initcount++;
	}
	FileClose(moodcalcfile);
	
	substitutesfile = FileOpen(0, "substitute", palmDBType, palmDBCreator, fileModeReadOnly, NULL);

	initcount=0;
	while (FGetS(substitutesfile, tag, 41) && initcount<166)
	{
		StrCopy(substitutes[initcount], tag);
		initcount++;
	}
	FileClose(substitutesfile);
	
	MemPtrFree(tag);
	SysRandom(7230);
	
}

void CleanInit()
{
	int i;
	
	for ( i = 0; i < 199; i++ )
	{
		MemPtrFree(moodvar[i]);
	}

	for ( i = 0; i < 148; i++ )
	{
		MemPtrFree(moodcalc[i]);
	}
	
	for ( i = 0; i < 169; i++ )
	{
		MemPtrFree(substitutes[i]);
	}
		
	for ( i = 0; i < 64; i++ )
	{
		MemPtrFree(token[i]);
	}

	for ( i = 0; i < 10; i++ ) 
	{
		MemPtrFree(saves[i]);
		sage[i] = 1000;
	}

	for ( i = 0; i < NUM_VARS + 1; i++ )
	{
		MemPtrFree(var[i]);
		MemPtrFree(setvars[i]);
		MemPtrFree(getvars[i]);
		MemPtrFree(defaultvar[i]);
	}

	MemPtrFree(star);
	MemPtrFree(star2);
	MemPtrFree(second);
	MemPtrFree(that);
	MemPtrFree(justthat);
	MemPtrFree(beforethat);
	MemPtrFree(bufferMain);
	MemPtrFree(pluseoln);
	MemPtrFree(texteoln);
	MemPtrFree(user_input);
	MemPtrFree(token);
	MemPtrFree(setvars);
	MemPtrFree(getvars);
	MemPtrFree(defaultvar);
	MemPtrFree(var);
	MemPtrFree(saves);
	MemPtrFree(moodvar);
	MemPtrFree(moodcalc);
	MemPtrFree(substitutes);
	MemPtrFree(readBuffer);
	MemPtrFree(replaceBuffer);
	
	MemPtrFree(part1);
	MemPtrFree(part2);
	MemPtrFree(part3);
	MemPtrFree(reetemp);
	MemPtrFree(reebuffer);
	MemPtrFree(fname);
	MemPtrFree(tempsaves);
	
	MemPtrFree(spaces);

	MemPtrFree(rstring);
	MemPtrFree(randbuffer);
	
	MemPtrFree(substbuffer);
	MemPtrFree(substline);
	MemPtrFree(subststring1);
	MemPtrFree(subststring2);
	
	MemHandleFree(respondposHandle);
	
	MemPtrFree(personBuffer);
	MemPtrFree(cleanerBuffer);
	MemPtrFree(removeTemp1);
	MemPtrFree(outputBuffer);
}

Boolean fexists( char* file ) // -------------------------------------------- File checker
{
	return false;
}

void expand( char* line ) // --------------------------------------------- Uncompressor
{
	while (replace( line, "~J", "ell me " ));
	while (replace( line, "~I", "<usesaved>" ));
	while (replace( line, "~H", " favorite " ));
	while (replace( line, "~G", " about " ));
	while (replace( line, "~F", "WHAT " ));
	while (replace( line, "~E", "I WAS " ));
	while (replace( line, "~D", "IT IS " ));
	while (replace( line, "~C", "I HAVE " ));
	while (replace( line, "~B", "I DID " ));
	while (replace( line, "~A", "I AM " ));
	while (replace( line, "~z", "<ironic>" ));
	while (replace( line, "~y", "<upset>" ));
	while (replace( line, "~x", "<worried>" ));
	while (replace( line, "~w", "<serious>" ));
	while (replace( line, "~v", "</save>" ));
	while (replace( line, "~u", "<save>" ));
	while (replace( line, "~t", "<suspecting>" ));
	while (replace( line, "~s", "<angry>" ));
	while (replace( line, "~r", "<getname/>" ));
	while (replace( line, "~q", "<set_" ));
	while (replace( line, "~p", "<get_" ));
	while (replace( line, "~o", "</think>" ));
	while (replace( line, "~n", "<think>" ));
	while (replace( line, "~m", "<star/>" ));
	while (replace( line, "~l", "<thinking>" ));
	while (replace( line, "~k", "<interested>" ));
	while (replace( line, "~j", "<happy>" ));
	while (replace( line, "~i", "<glad>" ));
	while (replace( line, "~h", "<funny>" ));
	while (replace( line, "~g", "<puzzled>" ));
	while (replace( line, "~f", "I don't " ));
	while (replace( line, "~e", "I am " ));
	while (replace( line, "~d", "o you " ));
	while (replace( line, "~K", " can " ));
	while (replace( line, "~c", " you" ));
	while (replace( line, "~b", "<sr/>" ));
	while (replace( line, "~a", "<person/>" ));
	while (replace( line, "<)>", "</set_it>" ));
	while (replace( line, "}", "</srai>" ));
	while (replace( line, "{", "<srai>" ));
	while (replace( line, "<(>", "<set_it>" ));
	while (replace( line, "@", "</li></random>" ));
	while (replace( line, "&", "</li><li>" ));
	while (replace( line, "%", "<random><li>" ));
}

void compress( char* buffer ) // ----------------------------------------- Compressor
{
	while (replace( buffer, "<random><li>", "%" ));
	while (replace( buffer, "</li><li>", "&" ));
	while (replace( buffer, "</li></random>", "@" ));
	while (replace( buffer, "<set_it>", "<(>" ));
	while (replace( buffer, "</set_it>", "<)>" ));
	while (replace( buffer, "<srai>", "{" ));
	while (replace( buffer, "</srai>", "}" ));
	while (replace( buffer, "<person/>", "~a" ));
	while (replace( buffer, "<sr/>", "~b" ));
	while (replace( buffer, " you", "~c" ));
	while (replace( buffer, " can ", "~K" ));
	while (replace( buffer, "o you ", "~d" ));
	while (replace( buffer, "I am ", "~e" ));
	while (replace( buffer, "I don't ", "~f" ));
	while (replace( buffer, "<puzzled>", "~g" ));
	while (replace( buffer, "<funny>", "~h" ));
	while (replace( buffer, "<glad>", "~i" ));
	while (replace( buffer, "<happy>", "~j" ));
	while (replace( buffer, "<interested>", "~k" ));
	while (replace( buffer, "<thinking>", "~l" ));
	while (replace( buffer, "<star/>", "~m" ));
	while (replace( buffer, "<think>", "~n" ));
	while (replace( buffer, "</think>", "~o" ));
	while (replace( buffer, "<get_", "~p" ));
	while (replace( buffer, "<set_", "~q" ));
	while (replace( buffer, "<getname/>", "~r" ));
	while (replace( buffer, "<angry>", "~s" ));
	while (replace( buffer, "<suspecting>", "~t" ));
	while (replace( buffer, "<save>", "~u" ));
	while (replace( buffer, "</save>", "~v" ));
	while (replace( buffer, "<serious>", "~w" ));
	while (replace( buffer, "<worried>", "~x" ));
	while (replace( buffer, "<upset>", "~y" ));
	while (replace( buffer, "<ironic>", "~z" ));
	while (replace( buffer, "I AM ", "~A" ));
	while (replace( buffer, "I DID ", "~B" ));
	while (replace( buffer, "I HAVE ", "~C" ));
	while (replace( buffer, "IT IS ", "~D" ));
	while (replace( buffer, "I WAS ", "~E" ));
	while (replace( buffer, "WHAT ", "~F" ));
	while (replace( buffer, " about ", "~G" ));
	while (replace( buffer, " favorite ", "~H" ));
	while (replace( buffer, "<usesaved>", "~I" ));
	while (replace( buffer, "ell me ", "~J" ));
}

int count( char* text, char* subst ) // ---------------------------------- String counter
{
	char* tmp, *pos = text;
	int retval = 0;
	while ((tmp = StrStr( pos, subst )))
	{
		retval++;
		pos = tmp + 1;
	}
	return retval;
}

int strval( char* text ) // ---------------------------------------------- String value
{
	char *pos = text;
	int val = 0;
	while (*pos) 
	{
		if ((*pos >= '0') && (*pos <= '9')) val = 10 * val + (*pos - '0');
		pos++;
	}
	return val;
}

int random( int max ) // ------------------------------------------------- Randomizer
{
//	return (int)((float)rand() * max / RAND_MAX);
	return SysRandom(0) % max;
}

void delay( int ms ) // ----------------------------------------------- Sleeper
{
//	resettimer();
//	while (msecs() < ms) {};
}

void remove( char* text, char* first, char* last ) // -------------------- Remover
{
	char* pos1, *pos2;
	if (!(pos1 = StrStr( text, first )))
	{
		return;
	}
	
	if (!(pos2 = StrStr( text, last )))
	{
		return;
	}
	
	StrCopy( removeTemp1, text );
	removeTemp1[pos1 - text] = 0;
	StrCopy( removeTemp1 + StrLen( removeTemp1 ), pos2 + StrLen( last) );
	StrCopy( text, removeTemp1 );
}

void bzero( char* text, int len ) // ------------------------------------- Clear string
{
	int i;
	for ( i = 0; i < len; i++ ) *(text + i) = 0;
}

void uppercasename( char* text ) // ------------------------------------------ Upper case
{
	Boolean first;
	char* temp = text - 1;
	first = true;
	while (*(++temp))
	{
		if (first)
		{
			if ((*temp >= 'a') && (*temp <= 'z')) *temp += 'A' - 'a';
			first = false;
		}
		else
		{
			if ((*temp >= 'A') && (*temp <= 'Z')) *temp -= 'A' - 'a';
		}
	}
}

void uppercase( char* text ) // ------------------------------------------ Upper case
{
	char* temp = text - 1;
	while (*(++temp)) if ((*temp >= 'a') && (*temp <= 'z')) *temp += 'A' - 'a';
}

void lowercase( char* text ) // ------------------------------------------ Lower case
{
	char* temp = text - 1;
	while (*(++temp)) if ((*temp >= 'A') && (*temp <= 'Z')) *temp -= 'A' - 'a';
}

Boolean replace( char* line, char* string1, char* string2 ) // -------------- Replacer
{
	char *pos = StrStr( line, string1 );
	Boolean retval = false;
	
//	print("Begin replace()");
	if (pos)
	{
		int len1 = StrLen( string1 ), len2 = StrLen( string2 ), loc = pos - line;
		replaceBuffer[0]=0;
		StrCopy( replaceBuffer, line );
		replaceBuffer[loc] = 0;
		StrCopy( replaceBuffer + loc, string2);
		replaceBuffer[loc + len2 + 1] = 0;
		StrCopy( replaceBuffer + loc + len2, line + loc + len1 );
		replaceBuffer[loc + len2 + StrLen( line ) - loc - len1 + 1] = 0;
		StrCopy ( line, replaceBuffer );
		retval = true;
	}
	return retval;
}

void cleaner( char* text ) // -------------------------------------------- Cleaner
{
	int bpos = 0, len = StrLen( text );
	int i;
	
	for ( i = 0; i < len; i++ )
	{
		if ((text[i] == 32) || ((text[i] >= 'A') && (text[i] <= 'Z')) ||
		   ((text[i] >= 'a') && (text[i] <= 'z')) || (text[i] == '@') ||
		   ((text[i] >= '0') && (text[i] <= '9')) || (text[i] == '-') || 
		   (text[i] == ':') ||
		   ((text[i] == '.') && ((text[i + 1] >= 'A') && (text[i + 1] <= 'Z')) 
//		   || ((text[i + 1] >= '0') && (text[i + 1] <= '9')))
		   ))
		   cleanerBuffer[bpos++] = text[i];
	}
	cleanerBuffer[bpos--] = 0;
	while (cleanerBuffer[bpos] == 32 && bpos!=0) cleanerBuffer[bpos--] = 0;
	StrCopy( text, cleanerBuffer );
}

void substitute( char* text, char* file ) // ----------------------------- Substituter
{
	int count=0;
	int pos1, pos2, pos3, pos4;
	char* pos1c, pos2c, pos3c, pos4c;
		
	StrCopy( substbuffer + 1, text );
	substbuffer[0] = 32;
	substbuffer[StrLen( substbuffer ) + 1] = 0;
	substbuffer[StrLen( substbuffer )] = 32;

	count=0;
	while (count<166)
	{
		StrCopy(substline,substitutes[count]);
		count++;

		pos1c = StrStr( substline, "'" );
		pos1 = StrStr( substline, "'" ) - substline;

		pos2c = StrStr( substline + pos1 + 1, "'" );
		pos2 =  StrStr( substline + pos1 + 1, "'" ) - substline; 

		pos3c = StrStr( substline + pos2 + 1, "'" );
		pos3 =  StrStr( substline + pos2 + 1, "'" ) - substline; 

		pos4c = StrStr( substline + pos3 + 1, "'" );
		pos4 =  StrStr( substline + pos3 + 1, "'" ) - substline; 

		if (!pos1c) continue;
		if (!pos2c) continue;
		if (!pos3c) continue;
		if (!pos4c) continue;

		StrCopy( subststring1, substline + pos1 + 1 );
		subststring1[pos2 - pos1] = 0;
		StrCopy( subststring2, substline + pos3 + 1 );
		subststring2[pos4 - pos3] = 0;
		subststring1[pos2 - pos1 - 1] = subststring2[pos4 - pos3 - 1] = 0;
		while (replace( substbuffer, subststring1, subststring2 ));
	}
	StrCopy( text, substbuffer + 1);
}

void tidyup( char* text ) // -------------------------------------- Substituter
{
	int pos;
	UInt16        i;
	UInt16        l;

	for ( i = 0; i < NUM_VARS; i++ )
	{
		if ((pos = StrStr( text, getvars[i] ) - text) >= 0 )
			while (replace( text, getvars[i], var[i] ));
	}
	for ( i = 0; i < StrLen( text ); i++ ) if (text[i] == 39) text[i] = '`';
	StrCopy( spaces + 1, text );
	spaces[0] = 32;
	l = StrLen( spaces );
	spaces[l] = 32;
	spaces[l + 1] = 0;
//  crashes for unknown reason disabled for the moment, not 
//  mission critical
	substitute( spaces, "substitute" );
	StrCopy( text, spaces + 1 );
	cleaner( text );
}

int calcmood( char* text ) // ------------------------------------- Get mood
{
	char* tag = (Char *) MemPtrNew(128);

	char* pos;
	char* nextli; 
	int tagnr = 0, retval = 0;
	int moodcount;
	
	Boolean valid = false;

//	print ("Begin calcmood");

	// temporary hack (I hope)	
	if (*text==85)
	{
		MemPtrFree(tag);
		return 0;
	}

	nextli = StrStr( text, "</li>" );	
	if ((pos = StrStr( text, "<sec>" ))) {};
		if (pos < nextli) nextli = pos;

	moodcount=0;
	while (moodcount<148)
	{
		StrCopy(tag,moodcalc[moodcount]);
		moodcount++;
		*(StrStr( tag, ">" ) + 1) = 0;
		if (!StrCompare( tag, "<eof>" )) break;
		if (!StrCompare( tag, "<warm>" )) 
		{
			tagnr = 0;
			valid = true;
		}
		if ((pos = StrStr( text, tag )))
			if ((pos < nextli) && valid) 
		{
			retval =  tagnr;
			break;
		}
		if ((!StrCompare( tag, "serious" )) && (retval = 0)) retval = tagnr;
		if (!StrCompare( tag, "<sarcastic>" )) break;
		tagnr++;
	}
	MemPtrFree(tag);
//	print ("End calcmood");
	return retval;
}

void updatemood( char* text ) // ---------------------------------- Moodifier
{
	char* tag = (Char *) MemPtrNew(42);
	char* buffer = (Char *) MemPtrNew(256);
	
	UInt16 i;
	int moodcount;

	double plus[6];
	double mul[6];

//	print ("Begin updatemood");

	moodcount=0;
	while (moodcount<198)
	{
		StrCopy(tag,moodvar[moodcount]);
		moodcount++;
		*(StrStr( tag, ">" ) + 1) = 0;
		if (!StrCompare( tag, "<eof>" )) break;
		for ( i = 0; i < 6; i++ )
		{
			StrCopy(buffer, moodvar[moodcount]);
			moodcount++;

// Does not compile with codewarrior but does compile with gcc
			FlpBufferAToF(&plus[i], StrStr( buffer, "+" ) + 2);

			*StrStr( buffer, "+" ) = 0;

// Does not compile with codewarrior but does compile with gcc
			FlpBufferAToF(&mul[i], StrStr( buffer, "*" ) + 1);
		}
		while (StrStr( text, tag ))
		{
			happyness	 = mul[0] * happyness	 + plus[0];
			helpfullness = mul[1] * helpfullness + plus[1];
			humor		 = mul[2] * humor		 + plus[2];
			affection	 = mul[3] * affection	 + plus[3];
			trust		 = mul[4] * trust		 + plus[4];
			interest	 = mul[5] * interest	 + plus[5];
			replace( text, tag, "" );
		}
//		moodcount++;
	}
	
	happyness = (happyness > 0.5f)?0.5f:((happyness < -0.5f)?-0.5f:happyness);
	helpfullness = (helpfullness > 0.5f)?0.5f:((helpfullness < -0.5f)?-0.5f:helpfullness);
	humor = (humor > 0.5f)?0.5f:((humor < -0.5f)?-0.5f:humor);
	affection = (affection > 0.5f)?0.5f:((affection < -0.5f)?-0.5f:affection);
	trust = (trust > 0.5f)?0.5f:((trust < -0.5f)?-0.5f:trust);
	interest = (interest > 0.5f)?0.5f:((interest < -0.5f)?-0.5f:interest);
	MemPtrFree(tag);
	MemPtrFree(buffer);
//	print ("End updatemood");

}

Boolean match( char* text, char* pattern, Boolean final )
{
	int starpos, pos2, len;
	char* pos;
	char* pos2c;
	
//	print("Begin match()");
	pateval++;

	if (!StrCompare( pattern, text )) return true;

	if (((pattern[0] == '*') || (pattern[0] == '_')) && 
		((pattern[StrLen( pattern ) - 1] == '*') || (pattern[StrLen( pattern ) - 1] == '_')) && 
		 (pattern[1]))
	{
		StrCopy( texteoln, pattern + 1 );
		
		texteoln[StrLen( texteoln ) - 1] = 0;

		if ((pos = StrStr( text, texteoln )))
		{
			StrCopy( star, text );
			star[pos - text] = 0;
			uppercasename(star);
			StrCopy( star2, pos + StrLen( texteoln ) );
			lowercase(star2);
			return true;
		}
		else return false;
	}
	if ((pattern[0] == '*') && (!pattern[1])) { if (final) { StrCopy( star, text ); lowercase(star); } return true; }
	if (((starpos = StrStr( pattern, "*" ) - pattern) < 0) &&
		((starpos = StrStr( pattern, "_" ) - pattern) < 0)) return false;
	if (starpos > 0)
	{
		if (StrNCompare( pattern, text, starpos )) return false;
		if (!pattern[starpos + 1]) 
		{
			if (final)
			{
				StrCopy( star, text + starpos );
				uppercasename(star);
			} 
			return true; 
		}
	}

	StrCopy( texteoln, text );

	len = StrLen( text );
	texteoln[len] = 10;
	texteoln[len + 1] = 0;
	StrCopy( pluseoln, pattern + starpos + 1 );
	len = StrLen( pluseoln );
	pluseoln[len] = 10;
	pluseoln[len + 1] = 0;
	pos2 = StrStr( texteoln, pluseoln ) - texteoln;
	pos2c = StrStr( texteoln, pluseoln );

	if (pos2c && pos2!=-1) 
	{
		if (text[pos2 + StrLen( pattern + starpos + 1)] == 0) 
		{
			if (final) 
			{
				StrCopy( star, text + starpos );
				// alert! "A Cat" crashes here
				star[pos2 - starpos] = 0;
				uppercasename(star); 
			}
			return true;
		}
	}
	return false;
}

void person( char* text, char* pstring, char* pfile ) // ---------- 1st to 3rd person
{
	if (StrStr( text, pstring ))
	{
		StrCopy( personBuffer, star );
		// removed for debug doesnt work EOFile not detected problem
		substitute( personBuffer, pfile );
		replace( text, pstring, personBuffer );
	}
}

void reevaluate( char* line ) // ---------------------------------- Re-evaluator
{
	char* posc;

	int pos = -1;
	UInt16 i;

	DrawRandomImage(false);

	if (!line[0]) return;

//	print ("Begin reevaluate");

	while (replace( line, "<sr/>", "<srai><star/></srai>" ));
	while (replace( line, "<setname/>", "<setname><star/></setname>" ));
	while (replace( line, "<set_female/>", "<set_gender>she</set_gender>" ));
	while (replace( line, "<set_male/>", "<set_gender>he</set_gender>" ));
	while (replace( line, "<star/>", star ));
	while (replace( line, "<justthat/>", justthat ));
	while (replace( line, "<beforethat/>", beforethat ));
	while (replace( line, "<star2/>", star2 ));
	while (StrStr( line, "<person/>" )) person( line, "<person/>", "person"  );
	while (StrStr( line, "<person2/>" )) person( line, "<person2/>", "person2" );
	while (replace( line, "<personf/>", star ));
	while (replace( line, "<that/>", that ));

	posc = StrStr( line, "<APPLET" );
	pos = StrStr( line, "<APPLET" ) - line;
	if (posc)
	{
		reebuffer = StrCopy( reebuffer, line + pos );
		*(StrStr( reebuffer, "</APPLET>" ) + 9) = 0;
		replace( line, reebuffer, "" );
	}

	for ( i = 0; i < NUM_VARS; i++ )
	{
		if (setvars[i][0])
		{
			Boolean go_on = false;
			while (!go_on)
			{
				go_on = true;
				StrCopy( reetemp, setvars[i] );
				replace( reetemp, ">", "/>" );
				if (StrStr( line, reetemp )) 
				{
					StrCopy( reebuffer, setvars[i] );
					StrCopy( StrStr( reebuffer, ">" ) + 1, star );
					StrCopy( reebuffer + StrLen( reebuffer ), "</" );
					StrCopy( reebuffer + StrLen( reebuffer ), setvars[i] + 1 );
					replace( line, reetemp, reebuffer );
					go_on = false;
				}
				
				posc = StrStr( line, setvars[i] );
				pos = StrStr( line, setvars[i] ) - line;
				
				if (posc)
				{
					StrCopy( reetemp, "</" );
					StrCopy( reetemp + 2, setvars[i] + 1 );
					StrNCopy( var[i], line + pos + StrLen( setvars[i] ), 256 );
					var[i][StrStr( var[i], reetemp ) - var[i]] = 0;
					replace( line, setvars[i], "" );
					replace( line, reetemp, "" );
					if (!StrCompare( setvars[i], "<set_it>" )) replace( line, var[i], "it" );
					if (!StrCompare( setvars[i], "<set_he>" )) replace( line, var[i], "he" );
					if (!StrCompare( setvars[i], "<set_she>" )) replace( line, var[i], "she" );
					if (!StrCompare( setvars[i], "<set_they>" )) replace( line, var[i], "they" );
					if (!StrCompare( setvars[i], "<set_we>" )) replace( line, var[i], "we" );
					go_on = false;
					if (i == 8)
					{
						uppercasename( var[i] );
						savevars( var[8] );
					}
					else
					{
						lowercase( var[i] );
					}

				}
			}
		}
		if (getvars[i][0])
		{
			posc = StrStr( line, getvars[i] );
			pos = StrStr( line, getvars[i] ) - line;
			if (posc)
				while (replace( line, getvars[i], var[i] ));
		}
	}
	
	posc = StrStr( line, "<sec>" );	
	pos = StrStr( line, "<sec>" ) - line;	
	if (posc)
	{
		StrCopy( second, line + pos + 5 );
		*StrStr( second, "</sec>" ) = 0;
		remove( line, "<sec>", "</sec>" );
	}
	
	posc = StrStr( line, "<save>" );
	pos = StrStr( line, "<save>" ) - line;
	while (posc)
	{
		int best = 0, bestage = 0;
		for ( i = 0; i < 10 ; i++ )
		{
			if (sage[i] > bestage)
			{
				bestage = sage[i];
				best = i;
			}
		}
		sage[best] = 0;	
		StrCopy( tempsaves, line + pos + 6 );
		*StrStr( tempsaves, "</save>" ) = 0;
		StrCopy( saves[best], tempsaves );
		remove( line, "<save>", "</save>" );
		posc = StrStr( line, "<save>" );
		pos = StrStr( line, "<save>" ) - line;
	}
	
	posc = StrStr( line, "<system>" );
	pos = StrStr( line, "<system>" )-line;
	
	if (posc)
	{
		StrCopy( reebuffer, line );
		reebuffer[pos] = 0;
		reevaluate( reebuffer );
		StrCopy( reebuffer, line + pos + 8 );
		*StrStr( reebuffer, "</system>" ) = 0;
		execute( reebuffer );
		StrCopy( reebuffer, StrStr( line, "</system>" ) + 9 );
		reevaluate( reebuffer );
	}
	else
	{
		remove( line, "<think>", "</think>" );
		while( replace( line, "<think>", "" ));
		while( replace( line, "</think>", "" ));
		
		
		posc = StrStr( line, "<srai>" );
		pos = StrStr( line, "<srai>" ) - line;
		
		if (posc)
		{
			UInt32 pos2 = StrStr( line, "</srai" ) - line;
			StrCopy( part1, line );
			part1[pos] = 0;
			StrCopy( part2, line + pos + 6 );
			part2[pos2 - pos - 6] = 0;
			StrCopy( part3, line + pos2 + 7 );
			if (part1[0]) reevaluate( part1 );
			if (part2[0]) respond( part2, "alice" );
			if (part3[0]) reevaluate( part3 );

		}
		else
		{
			StrCopy( beforethat, that );
			StrNCopy( that, line, MinMemory);
			if (second[0])
			{
				secdelay = TimGetSeconds();
				secdelay += 4 + (StrLen( second ) / 8) + random( 3 );
			}
			// don't calculate mood for the longest string in lowmemmode
			if (RunningInLowMemMode)
			{	
				if (StrStr(that,"<p>Experience with Ruby has allowed us broadly"))
				{
					StrCopy(that, "Category A=rude, B=fun, C=nerds");
				}
				else updatemood( that );
			}
			else updatemood( that );
//			tidyup( that );			
			if (recurs<2) printb( that );
			else printbMore( that );
			DrawRandomImage(true);
		}		
	}
//	print ("End reevaluate");
}

void saveinitstate() // --------------------------------- Var saver
{
	UInt32 result;
	char* savetemp = MemPtrNew(256);
	
	FileHand init = FileOpen(0, "init", palmDBType, palmDBCreator, fileModeReadWrite, NULL);

	savevars( var[8] );	

	StrPrintF( savetemp, "%s\r\n", var[8] );
	result = FileWrite( init, savetemp, StrLen(savetemp), 1, NULL );

	FileClose( init );
	MemPtrFree( savetemp );
}

void savevars( char* varfile ) // --------------------------------- Var saver
{
	UInt16 i;
	UInt32 thetime, result;
	char* savetemp = MemPtrNew(256);
	FileHand vars = FileOpen(0, varfile, palmDBType, palmDBCreator, fileModeReadWrite, NULL);

	thetime = TimGetSeconds();

	for ( i = 0; i < NUM_VARS; i++ ) 
	{
		StrPrintF( savetemp, "%s:%s\r\n", getvars[i], var[i] );
		result = FileWrite( vars, savetemp, StrLen(savetemp), 1, NULL );
	}
	if (StrLen( that )>253) that[253] = 0;
	StrPrintF( savetemp, "%s\r\n", that );
	result = FileWrite( vars, savetemp, StrLen(savetemp), 1, NULL );
	StrPrintF( savetemp, "%lu\r\n", thisrun ); // Apllication start time
	result = FileWrite( vars, savetemp, StrLen(savetemp), 1, NULL );
	StrPrintF( savetemp, "%lu\r\n", thetime ); // Application termination time
	result = FileWrite( vars, savetemp, StrLen(savetemp), 1, NULL );
	for ( i = 0; i < 5; i++ )
	{
		if (topic[i]) StrPrintF( savetemp, "true\r\n" );
		else StrPrintF( savetemp, "false\r\n" );
		result = FileWrite( vars, savetemp, StrLen(savetemp), 1, NULL );
	}
	FileClose( vars );
	MemPtrFree( savetemp );
}

void loadvars( char* varfile ) // --------------------------------- Var saver
{
	UInt16 i;	

	FileHand vars = FileOpen(0, varfile, palmDBType, palmDBCreator, fileModeReadOnly, NULL);
	FileHand tags = FileOpen(0, "defvars", palmDBType, palmDBCreator, fileModeReadOnly, NULL);

//	print("starting...");
	for ( i = 0; i < NUM_VARS; i++ )
	{	
		char* buffer = (Char *) MemPtrNew(128+1);
		FGetS(tags, buffer, 128);

		StrCopy( getvars[i], buffer);
		*(StrStr( getvars[i], "/>" ) + 2) = 0;
		StrCopy( defaultvar[i], StrStr( buffer, "/>" ) + 2 );
		StrCopy( setvars[i], getvars[i] );
		setvars[i][1] = 's';
		replace( setvars[i], "/>", ">" );
		MemPtrFree(buffer);
	}

	for ( i = 0; i < NUM_VARS; i++ ) 
	{
		MemPtrFree(var[i]);
		var[i] = (Char *) MemPtrNew(256);
		
		FGetS(vars, bufferMain, 256);

		if (StrStr( bufferMain, ">:" )) 
			StrCopy( var[i], StrStr( bufferMain, ">:" ) + 2 );
		else
			var[i][0] = 0;
		var[i][StrLen( var[i] ) - 1] = 0;
	}

	FGetS(vars, that, MediumMemory);

	that[StrLen( that ) - 1] = 0;

	FGetS(vars, bufferMain, 128);
	lastrun = strval( bufferMain );
	FGetS(vars, bufferMain, 128);
	lastquit = strval( bufferMain );
	
	for ( i = 0; i < 5; i++ )
	{
		FGetS(vars, bufferMain, 128);
		if (!StrNCompare( bufferMain, "true", 4 )) topic[i] = true; else topic[i] = false;
	}
	
	FileClose( vars );
	FileClose( tags );
}

void randomize( char* text ) // ----------------------------------- Randomizer
{
	int pos1;
	char* pos1c;
	char* posc;
	int list[100], mood[100];
	double sqdist[100];
	int items;
	int pos, last;
	int i,j;
	int h;
	float cmood;
	double rnd;
	int item;
	double tdist;
//	print ("Begin randomize");
	pos1c = StrStr( text, "<random>" );
	pos1 = StrStr( text, "<random>" ) - text;

	DrawRandomImage(false);
		
	while (pos1c)
	{
		StrCopy( rstring, text + pos1 + 8 );
		*StrStr( rstring, "</random>" ) = 0;
		items = 0;
		pos = -1;
		last = -1;
		pos = StrStr( rstring + last + 1, "<li>" ) - rstring;
		posc = StrStr( rstring + last + 1, "<li>" );
		while (posc)		
		{
			list[items] = pos + 4;
			mood[items++] = calcmood( pos + 4 + rstring );
			last = pos;
			pos = StrStr( rstring + last + 1, "<li>" ) - rstring;
			posc = StrStr( rstring + last + 1, "<li>" );
		}
		for ( i = 0; i < items; i++ )
		{
			for ( j = 1; j < items; j++ )
			{
				if (mood[j - 1] < mood[j])
				{
					h = mood[j]; mood[j] = mood[j - 1]; mood[j - 1] = h;
					h = list[j]; list[j] = list[j - 1]; list[j - 1] = h;
				}
			}
		}
		cmood = (happyness + 0.5f * helpfullness + humor + affection + 0.5f * trust) / 3 + 0.5f;
		if (cmood < 0) cmood = 0; else if (cmood > 1) cmood = 1;
		tdist = 0;
		for ( i = 0; i < items; i++) 
			tdist += (sqdist[i] = 1.0 / ((cmood - mood[i]) * (cmood - mood[i])));
		for ( i = 0; i < items; i++)
		{
			sqdist[i] *= 1 / tdist;
			if ( i > 0) sqdist[i] += sqdist[i - 1];
		}
		rnd = RandomNum(100)/100.0;
		item = 0;
		for ( i = 0; i < items; i++)
		{
			if (rnd < sqdist[i]) break;
			item++;
		}
		if (item == items) item = items - 1;
		StrCopy( randbuffer, text );
		StrCopy( randbuffer + pos1, rstring + list[item] );
		*StrStr( randbuffer, "</li>" ) = 0;
		StrCopy( randbuffer + StrLen( randbuffer ), " " );
		StrCopy( randbuffer + StrLen( randbuffer ), StrStr( text, "</random>" ) + 9 );
		StrCopy( text, randbuffer );
	
		pos1c = StrStr( text, "<random>" );
		pos1 = StrStr( text, "<random>" ) - text;
	}
//	print ("End randomize");
}

Boolean trysecond() // ----------------------------------------------- Plan B
{
	if (second[0] != 0)
	{
		char* line = (Char *) MemPtrNew(MediumMemory);
		StrCopy( line, second );
		reevaluate( line );
		MemPtrFree(line);
		return true;
	}
	return false;
}

void tokenizer( char* text ) // ----------------------------------- Tokenizer
{
	char* pos = text;
	int word = 0, wlen = 0;
	while (*pos)
	{
		if (((*pos >= 'A') && (*pos <= 'Z')) || (*pos == '\''))
		{
			token[word][wlen++] = *pos;
		}
		if ((*pos == ' ') || (*pos == ',') || (*pos == '.') || (*pos == '?') ||
		    (*pos == '!') || (*pos == '-')) 
		{
			token[word++][wlen] = 0;
			wlen = 0;
		}
		if ((wlen == 63) || (word == 63)) break;
		pos++;
	}
	token[word++][wlen] = 0;
	tokens = word;
}


Boolean condition( char* text ) // ----------------------------------- Conditionizer
{
	Int32 pos, i;
	char* s1;
	char* s2;

	for ( i = 0; i < NUM_VARS; i++ )
	{
		if (getvars[i][0])
		{
			pos = (Int32) StrStr( text, getvars[i] ); 
			if ((pos - (Int32) text) >= 0 )
				while (replace( text, getvars[i], var[i] ));
		}
	}
	s1 = text;
	s2 = StrStr( text, "==" ) + 3;
	*(StrStr( s1, "==" ) - 1) = 0;
	*(StrStr( s2, ">" ) - 1) = 0;
	return !StrCompare( s1, s2 );
}

void respond( char* text, char* bot )
{
	UInt32 tpos;
	UInt32 bestpos=0;
	
	char* line = (Char *) MemPtrNew(MaxMemory);
	char* best = (Char *) MemPtrNew(MediumMemory);
	char* buffer = (Char *) MemPtrNew(MaxMemory);	

	// for debug
	char* lastLine = (Char *) MemPtrNew(MediumMemory);
	char* randtagpos;
	int totalLines;

	Boolean alpha;
	
	UInt16 i;
	UInt8 answered;
	
	FileHand aiml,tmpl;
	
	int tval, bestline, linenr;
	char* tvalc;

	Boolean thatused, perfectthat;
//	print ("Begin respond");

	DrawRandomImage(false);

	recurs=recurs+1;
	if ((RunningInLowMemMode && recurs > 3) || (!RunningInLowMemMode && recurs > 8)) 
	{
		printb( "Oh, my head is spinning in a dangerous recurs." );
		MemPtrFree(line);
		MemPtrFree(best);
		MemPtrFree(buffer);
		MemPtrFree(lastLine); 
		return;
	}
	
	uppercase( text );
	tidyup( text );

	if (recurs == 1)
	{
		tokenizer( text );
	}
	
	if (!StrCompare( text, justthat ))
	{
		if (!repetitive)
		{
			unsigned int r = RandomNum(5);
			if (r == 0) printb( "I heard you." );
			else if (r == 1) printb( "You are repeating yourself." );
			else if (r == 2) printb( "That's what you said." );
			else if (r == 3) printb( "You said that." );
			else printb( "You already said that." );
//			repetitive = true;
		}
		MemPtrFree(line);
		MemPtrFree(best);
		MemPtrFree(buffer);
		MemPtrFree(lastLine);	
		return;
	}
	else
	{
		repetitive = false;
	}

	best[0] = 0;
	
	if (text[0] >= 'A')
		alpha = true;
	else if (text[0] <= 'Z')
		alpha = true;
	else
		alpha = false;

	aiml = FileOpen(0, "patterns", palmDBType, palmDBCreator, fileModeReadOnly, NULL);
	tmpl = FileOpen(0, "templates", palmDBType, palmDBCreator, fileModeReadOnly, NULL);

	bestline = -1;
	linenr = -1;

	// for debug purpose
	StrCopy( lastLine, "hmm");
	if (text[0]=='A') totalLines = 1069;
	else if (text[0]=='B') totalLines = 141;
	else if (text[0]=='C') totalLines = 491;
	else if (text[0]=='D') totalLines = 1193;
	else if (text[0]=='E') totalLines = 84;
	else if (text[0]=='F') totalLines = 98;
	else if (text[0]=='G') totalLines = 165;
	else if (text[0]=='H') totalLines = 587;
	else if (text[0]=='I') totalLines = 1901;
	else if (text[0]=='J') totalLines = 52;
	else if (text[0]=='K') totalLines = 34;
	else if (text[0]=='L') totalLines = 91;
	else if (text[0]=='M') totalLines = 197;
	else if (text[0]=='N') totalLines = 178;
	else if (text[0]=='O') totalLines = 84;
	else if (text[0]=='P') totalLines = 78;
	else if (text[0]=='Q') totalLines = 4;
	else if (text[0]=='R') totalLines = 77;
	else if (text[0]=='S') totalLines = 241;
	else if (text[0]=='T') totalLines = 534;
	else if (text[0]=='U') totalLines = 38;
	else if (text[0]=='V') totalLines = 17;
	else if (text[0]=='W') totalLines = 2209;
	else if (text[0]=='Y') totalLines = 929;
	else if (text[0]=='X') totalLines = 1;
	else if (text[0]=='Z') totalLines = 1;
	else totalLines = 152;
	
	if (alpha) FileSeek( aiml, chunk[text[0] - 'A'], fileOriginBeginning); 
		  else FileSeek(  aiml, chunk[26], fileOriginBeginning);

	// testing MemHandle
	respondpos = (char *) MemHandleLock(respondposHandle);
	
	while (linenr<totalLines && StrCompare(text, best))
	{
		linenr++;
		
		if (linenr==500 || linenr==1000 || linenr==1500 || linenr==2000)
			DrawRandomImage(false);

		FGetS(aiml, line, MediumMemory);		
		
		if ((alpha) && (line[0] != text[0])) 
		{
			alpha = false;
			FileSeek( aiml, chunk[26], fileOriginBeginning);
			totalLines = 152;
			linenr=0;
			continue;
		}

		tvalc = StrStr( line, "@" );
	
		if (!tvalc) continue;

		tval = StrStr( line, "@" ) - line;
		StrCopy( buffer, line + tval + 1 );
		
		// for debug
		StrCopy(lastLine, line);
		
		buffer[StrLen( buffer ) - 1] = 0;
		line[tval] = 0;
		tpos = StrAToI( buffer );
		while (replace( line, "<name/>", var[12] )) uppercase( line );
		thatused = perfectthat = false;
		if ((respondpos = StrStr( line, "<that>" )))
		{
			StrCopy( buffer, respondpos + 6 );
			*StrStr( line, "<that>" ) = 0;
			*StrStr( buffer, "</that>" ) = 0;
			if (!match( that, buffer, false )) continue;
			thatused = true;
			if (StrCompare( that, buffer ) == 0) perfectthat = true;
		}
		if ((respondpos = StrStr( line, "<if" )))
		{
			if (!condition( respondpos + 4 )) continue;
			*StrStr( line, "<if" ) = 0;
		}
		if (!match( text, line, false )) continue;
		if ((StrCompare( text, line ) == 0) && thatused && perfectthat)
		{
			StrCopy( best, line );
			bestline = linenr;
			bestpos = tpos;
			break;
		}
		if ((StrCompare( line, best ) < 0) && (bestline > -1)) continue;
		StrCopy( best, line );
		bestline = linenr;
		bestpos = tpos;
	}

	// testing MemHandle
	MemHandleUnlock(respondposHandle);

	match( text, best, true );	
	FileSeek(tmpl, bestpos, fileOriginBeginning);

	FGetS(tmpl, line, MediumMemory);

	answered = false;
	expand( line );
	if ((best[1] == 0) || (StrStr( line, "<usesaved>" ))) 
	{
		answered = trysecond();
		if (!answered)
		{
			for ( i = 0; i < 10; i++ )
			{
				if (sage[i] < 15)
				{
					char* sline = (Char*) MemPtrNew(MinMemory);
					StrCopy( sline, saves[i] );
					reevaluate( sline );
					MemPtrFree(sline);
					sage[i] = 1000;
					answered = true;
					break;
				}
			}
		}
	}
	second[0] = 0;
	if (!answered)
	{
		buffer[0] = 10;
		buffer[1] = 0;
		while (replace( line, "<usesaved>", "" ));
		while (replace( line, "<gossip>", "" ));
		while (replace( line, "</gossip>", "" ));
		while (replace( line, "<boring>", "" ));
		while (replace( line, "<br>", buffer ));
		while (replace( line, "<BR>", buffer ));
		while (replace( line, "<Br>", buffer ));
		if (StrLen( line ) > 0) line[StrLen( line ) - 1] = 0;

		randtagpos = StrStr( line, "<random");
		if (randtagpos) randomize( line );
		reevaluate( line );
	}
	FileClose( aiml );
	FileClose( tmpl );
	StrCopy( justthat, text );

	MemPtrFree(line);
	MemPtrFree(best);
	MemPtrFree(buffer);
	MemPtrFree(lastLine);
//	print ("End respond");

}


void execute( char* command ) // ---------------------------------- Executer
{
	DateTimeType today;
	UInt32 theSeconds;
	Err result;

	if (!StrCompare( command, "exit" ))
	{
		unsigned int r = RandomNum(4);
		if (r == 0) printb( "Bye bye, " );
		else if (r == 1) printb( "Good luck with the rest of your day, " );
		else if (r == 2) printb( "Bye, hope I see you soo, " );
		else if (r == 3) printb( "Are you leaving me all alone, " );
		else printb( "Goodbye, ");
		printbMore( var[8] );
		exitplease = true;
		return;
	}
	if (!StrCompare( command, "date" ))
	{
		char* tempDate = MemPtrNew(100);
		theSeconds = TimGetSeconds();
		TimSecondsToDateTime(theSeconds, &today);
		result = StrPrintF( tempDate,"The time is %i:%i:%i and the date is %i-%i-%i.", 
		today.hour, today.minute, today.second, today.month, today.day, today.year);
		printb(tempDate);
		MemPtrFree(tempDate);
	}
	if (!StrCompare( command, "clearscreen" ))
	{
//		clearscreen();
		return;
	}
}

void initialize() // ---------------------------------------------- Initializer
{
	char* initbuffer = MemPtrNew(128);

	FileHand init = FileOpen(0, "init", palmDBType, palmDBCreator, fileModeReadOnly, NULL);
	FGetS(init, initbuffer, 128);
	FileClose (init);
	
	initbuffer[StrLen( initbuffer ) - 1] = 0;
	
	loadvars( initbuffer  );
	
	MemPtrFree( initbuffer );
}

void greeting() // ------------------------------------------------ First words
{
	unsigned int r;
	UInt32 midnight;
	DateTimeType today;
	int mhour=4;
	int mmin=20;

	midnight = TimGetSeconds();
	TimSecondsToDateTime(midnight, &today);
	
	print("");
	
//	if (midnight > lastquit)
//	{
		r = RandomNum(2);
		mhour = today.hour;
		mmin = today.minute;
		
		if (mhour < 6) 
		{
			if (r == 0) print( "Good night, " );
			else print( "This is a very good night, isn't it, " );
		}
		else if (mhour < 12) 
		{
			if ((mhour < 9))
			{
				print( "Wow, You're early today. " );
			}
			if (r == 0) printMore( "Good morning, " );
			else printMore( "Let's enjoy a splendid morning, " );
		}
		else if (mhour < 18) 
		{
			if ((mhour == 17) && (mmin > 40))
			{
				print( "Let me see, is it still afternoon? Yes, it is. " );
			}
			if (r == 0) printMore( "Good afternoon, " );
			else printMore( "Let's have a cool afternoon, " ); 
		}
		else 
		{
			if (r == 0) print( "Good evening, " );	
			else print( "This is a good evening, " );
		}
		printMore( var[8] );
//	}
//	else
//	{
//		unsigned int r = RandomNum(4);
//		if (r == 0) print( "Hello Again, " );
//		else if (r == 1) print( "Nice to see you back, " );
//		else if (r == 2) print( "Ah, you're back, " );
//		else print( "Nice to talk to you again, " );
//		printMore( var[8] );
//	}
	if (!newvarfile)
	{
		if ((lastquit - lastrun) < 20) 
		{
			unsigned int r = RandomNum(3);
			if (r == 0)	printMore( ". Is this another testrun?" );
			else if (r == 1) printMore( ". Just testing again?" );
			else printMore( ". This is just a test, isn't it?" );
		}
		else if ((lastquit - lastrun) < 300) 
		{
			unsigned int r = RandomNum(3);
			if (r == 0) printMore( ". Are you going to leave me again within few minutes?" );
			else if (r == 1) printMore (". Last time you left me within five minutes..." );
			else printMore (". How can I talk to you if you leave me so soon?" );
		}
	}
}

DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
	Word err;

	if (cmd == sysAppLaunchCmdNormalLaunch)
	{
			err = StartApplication();
			if (err) return err;
			EventLoop();
			StopApplication();
	}
	else
	{
		return sysErrParamErr;
	}

	return 0;
}
