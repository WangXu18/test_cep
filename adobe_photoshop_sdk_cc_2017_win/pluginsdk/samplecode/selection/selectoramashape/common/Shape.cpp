// ADOBE SYSTEMS INCORPORATED
// Copyright  1993 - 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this 
// file in accordance with the terms of the Adobe license agreement
// accompanying it.  If you have received this file from a source
// other than Adobe, then your use, modification, or distribution
// of it requires the prior written permission of Adobe.
//-------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		Shape.cpp
//
//	Description:
//		This file contains the source and functions for the
//		Import module Shape, which returns a path, layer,
//		or selection with an interesting shape.
//
//	Use:
//		This module specifically works the path return
//		functionality of Photoshop.  The Paths are stored
//		by derezing a work path saved in Photoshop then
//		storing them in a "Path" resource, which is a
//		binary data resource of a "Path" as defined in the
//		"Path layout" section of the Photosop File Format.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Includes
//-------------------------------------------------------------------------------

#include "Shape.h"

//-------------------------------------------------------------------------------
//	Globals -- Define global variables for plug-in scope.
//-------------------------------------------------------------------------------

DLLExport MACPASCAL void PluginMain1 (const int16 selector,
						             PISelectionParams *selectionParamBlock,
						             intptr_t *data,
						             int16 *result);
//-------------------------------------------------------------------------------
//
//	PluginMain / main
//
//	All calls to the plug-in module come through this routine.
//	It must be placed first in the resource.  To achieve this,
//	most development systems require this be the first routine
//	in the source.
//
//	The entrypoint will be "pascal void" for Macintosh,
//	"void" for Windows.
//
//	Inputs:
//		const short selector						Host provides selector indicating
//													what command to do.
//
//		PISelectionParams *selectionParamBlock		Host provides pointer to parameter
//													block containing pertinent data
//													and callbacks from the host.
//													See PISelection.h.
//
//	Outputs:
//		PISelectionParams *selectionParamBlock		Host provides pointer to parameter
//													block containing pertinent data
//													and callbacks from the host.
//													See PISelection.h.
//
//		void *data									Use this to store a handle or pointer to our
//													global parameters structure, which
//													is maintained by the host between
//													calls to the plug-in.
//
//		short *result								Return error result or noErr.  Some
//													errors are handled by the host, some
//													are silent, and some you must handle.
//													See PIGeneral.h.
//
//-------------------------------------------------------------------------------

DLLExport MACPASCAL void PluginMain1 (const short selector,
						             PISelectionParams *selectionParamBlock,
						             intptr_t *data,
						             int16 *result)
{

  try {

	//---------------------------------------------------------------------------
	//	(1) Check for about box request.
	//
	// 	The about box is a special request; the parameter block is not filled
	// 	out, none of the callbacks or standard data is available.  Instead,
	// 	the parameter block points to an AboutRecord, which is used
	// 	on Windows.
	//---------------------------------------------------------------------------

	if (selector == selectionSelectorAbout)
	{
		// we dont have to because Selectorama did it already or will do it soon
		// sSPBasic = ((AboutRecordPtr)selectionParamBlock)->sSPBasic;
		// DoAbout((AboutRecordPtr)selectionParamBlock);
	}
	else
	{ // do the rest of the process as normal:

		sSPBasic = ((PISelectionParams*)selectionParamBlock)->sSPBasic;

		Ptr globalPtr = NULL;		// Pointer for global structure
		GPtr globals = NULL; 		// actual globals

		//-----------------------------------------------------------------------
		//	(2) Allocate and initalize globals.
		//
		// 	AllocateGlobals requires the pointer to result, the pointer to the
		// 	parameter block, a pointer to the handle procs, the size of our local
		// 	"Globals" structure, a pointer to the long *data, a Function
		// 	Proc (FProcP) to the InitGlobalsShape routine.  It automatically sets-up,
		// 	initializes the globals (if necessary), results result to 0, and
		// 	returns with a valid pointer to the locked globals handle or NULL.
		//-----------------------------------------------------------------------
		
		globalPtr = AllocateGlobals (result,
									 selectionParamBlock,
									 selectionParamBlock->handleProcs,
									 sizeof(Globals),
						 			 data,
						 			 InitGlobalsShape);
		
		if (globalPtr == NULL)
		{ // Something bad happened if we couldn't allocate our pointer.
		  // Fortunately, everything's already been cleaned up,
		  // so all we have to do is report an error.
		  
		  *result = memFullErr;
		  return;
		}
		
		// Get our "globals" variable assigned as a Global Pointer struct with the
		// data we've returned:
		globals = (GPtr)globalPtr;

		//-----------------------------------------------------------------------
		//	(3) Dispatch selector.
		//-----------------------------------------------------------------------

		DoExecuteShape(globals);
			
		//-----------------------------------------------------------------------
		//	(4) Unlock data, and exit resource.
		//
		//	Result is automatically returned in *result, which is
		//	pointed to by gResult.
		//-----------------------------------------------------------------------	
		
		// unlock handle pointing to parameter block and data so it can move
		// if memory gets shuffled:
		if ((Handle)*data != NULL)
			PIUnlockHandle((Handle)*data);
	
	} // about selector special		

  } // end try

  catch (...)
  {
	if (NULL != result)
	{
	  *result = -1;
	}
  }

} // end PluginMain

//-------------------------------------------------------------------------------
//
//	InitGlobalsShape
//	
//	Initalize any global values here.  Called only once when global
//	space is reserved for the first time.
//
//	Inputs:
//		Ptr globalPtr		Standard pointer to a global structure.
//
//	Outputs:
//		Initializes any global values with their defaults.
//
//-------------------------------------------------------------------------------

void InitGlobalsShape (Ptr globalPtr)
{	
	// create "globals" as a our struct global pointer so that any
	// macros work:
	GPtr globals = (GPtr)globalPtr;
	
	// Initialize global variables:
	gQueryForParameters = true;
	ValidateParameters (globals);
	
} // end InitGlobalsShape

//-------------------------------------------------------------------------------
//
//	ValidateParameters
//
//	Initialize parameters to default values.
//
//	Inputs:
//		GPtr globals		Pointer to global structure.
//
//	Outputs:
//		gWhatShape			Default: iShapeTriangle.
//
//		gCreate				Default: iCreateSelection.
//
//		gIdleAmount			Default: 2.0 seconds.
//
//-------------------------------------------------------------------------------

void ValidateParameters (GPtr globals)
{
	gWhatShape = iShapeTriangle;
	gCreate = iCreateSelection;

} // end ValidateParameters

//-------------------------------------------------------------------------------
//
//	DoExecuteShape
//
//	Main routine.  In this case, pop the UI then return the path.
//
//	Inputs:
//		GPtr globals		Pointer to global structure.
//
//	Outputs:
//		gResult				Returns noErr if completed without error.
//							Most probable error return, if any, is
//							not enough memory err.
//
//-------------------------------------------------------------------------------

void DoExecuteShape (GPtr globals)
{
	Boolean			doThis = true;

	gQueryForParameters = ReadScriptParamsShape (globals);


	if ( gQueryForParameters ) doThis = DoUIShape (globals);

	if ( doThis )
	{
        gStuff->newPath = GetPathHandle(gStuff->handleProcs, gWhatShape);
		if (gStuff->newPath == NULL)
		{
			gResult = memFullErr;
			return;
		}

		/* look in gStuff->supportedTreatments for support for this next thang */
		
		gStuff->treatment = KeyToEnumShape(EnumToKeyShape(gCreate,typeMyCreate),typeMyPISel);

		WriteScriptParamsShape (globals);
	} // user cancelled or dialog err or silent
}

//-------------------------------------------------------------------------------

Handle GetPathHandle(HandleProcs * procs, short inTreatment)
{
    Handle h = NULL;
    uint8_t * pathData = NULL;
    size_t pathSize = 0;
    switch (inTreatment)
    {
        case iShapeTriangle:
        {
        uint8_t trianglePath[] =
    { 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x86, 0x80, 0x00, 0x00, 0x16, 0x80, 0x00, 0x00, 0x86,
      0x80, 0x00, 0x00, 0x16, 0x80, 0x00, 0x00, 0x86, 0x80, 0x00, 0x00, 0x16, 0x80, 0x00, 0x00, 0x01,
      0x00, 0x15, 0x80, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x15, 0x80, 0x00, 0x00, 0x88, 0x00, 0x00,
      0x00, 0x15, 0x80, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x01, 0x00, 0xC2, 0x80, 0x00, 0x00, 0xD0,
      0x00, 0x00, 0x00, 0xC2, 0x80, 0x00, 0x00, 0xD0, 0x00, 0x00, 0x00, 0xC2, 0x80, 0x00, 0x00, 0xD0,
      0x00, 0x00
        };

        pathData = trianglePath;
        pathSize = sizeof(trianglePath);
        }
        break;
    
        case iShapeSquare:
        {
        uint8_t squarePath[] =
    { 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x2A, 0xAA, 0xAB, 0x00, 0x2A, 0x68, 0x4C, 0x00, 0x2A,
      0xAA, 0xAB, 0x00, 0x2A, 0x68, 0x4C, 0x00, 0x2A, 0xAA, 0xAB, 0x00, 0x2A, 0x68, 0x4C, 0x00, 0x01,
      0x00, 0xD6, 0x2A, 0xAB, 0x00, 0x2A, 0x68, 0x4C, 0x00, 0xD6, 0x2A, 0xAB, 0x00, 0x2A, 0x68, 0x4C,
      0x00, 0xD6, 0x2A, 0xAB, 0x00, 0x2A, 0x68, 0x4C, 0x00, 0x01, 0x00, 0xD6, 0x2A, 0xAB, 0x00, 0xD5,
      0x2A, 0xAB, 0x00, 0xD6, 0x2A, 0xAB, 0x00, 0xD5, 0x2A, 0xAB, 0x00, 0xD6, 0x2A, 0xAB, 0x00, 0xD5,
      0x2A, 0xAB, 0x00, 0x01, 0x00, 0x2A, 0xAA, 0xAB, 0x00, 0xD5, 0x2A, 0xAB, 0x00, 0x2A, 0xAA, 0xAB,
      0x00, 0xD5, 0x2A, 0xAB, 0x00, 0x2A, 0xAA, 0xAB, 0x00, 0xD5, 0x2A, 0xAB
        };
            
        pathData = squarePath;
        pathSize = sizeof(squarePath);

        }
        break;
        case iShapeCircle:
        {
    uint8_t circlePath[] =
    {   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x1F, 0xA5, 0xED, 0x00, 0xFB, 0x2A, 0xAB, 0x00, 0x1F,
        0xA5, 0xED, 0x00, 0x80, 0x42, 0x5F, 0x00, 0x1F, 0xA5, 0xED, 0x00, 0x05, 0x80, 0x00, 0x00, 0x01,
        0x00, 0xE0, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x80, 0x42, 0x5F,
        0x00, 0xE0, 0x00, 0x00, 0x00, 0xFA, 0x12, 0xF7
    };
        pathData = circlePath;
        pathSize = sizeof(circlePath);
        }
    
        break;
        case iShapeStar:
        {
        uint8_t starPath[] =
    {   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x2C, 0xF9, 0x15, 0x00, 0x77, 0xA6, 0xF5, 0x00, 0x2C,
        0xF9, 0x15, 0x00, 0x77, 0xA6, 0xF5, 0x00, 0x2C, 0xF9, 0x15, 0x00, 0x77, 0xA6, 0xF5, 0x00, 0x01,
        0x00, 0x9B, 0xAC, 0xF9, 0x00, 0x32, 0x16, 0x43, 0x00, 0x9B, 0xAC, 0xF9, 0x00, 0x32, 0x16, 0x43,
        0x00, 0x9B, 0xAC, 0xF9, 0x00, 0x32, 0x16, 0x43, 0x00, 0x01, 0x00, 0x9F, 0x22, 0x98, 0x00, 0xB6,
        0x42, 0xC8, 0x00, 0x9F, 0x22, 0x98, 0x00, 0xB6, 0x42, 0xC8, 0x00, 0x9F, 0x22, 0x98, 0x00, 0xB6,
        0x42, 0xC8, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x46,
        0xEB, 0x3E, 0x00, 0x2F, 0x4D, 0xEA, 0x00, 0x46, 0xEB, 0x3E, 0x00, 0x2F, 0x4D, 0xEA, 0x00, 0x46,
        0xEB, 0x3E, 0x00, 0x2F, 0x4D, 0xEA, 0x00, 0x01, 0x00, 0x48, 0xA6, 0x0E, 0x00, 0xB9, 0x0B, 0x21,
        0x00, 0x48, 0xA6, 0x0E, 0x00, 0xB9, 0x0B, 0x21, 0x00, 0x48, 0xA6, 0x0E, 0x00, 0xB9, 0x0B, 0x21,
        0x00, 0x01, 0x00, 0xC5, 0x30, 0x6F, 0x00, 0x73, 0x7A, 0x6F, 0x00, 0xC5, 0x30, 0x6F, 0x00, 0x73,
        0x7A, 0x6F, 0x00, 0xC5, 0x30, 0x6F, 0x00, 0x73, 0x7A, 0x6F
    };
        pathData = starPath;
        pathSize = sizeof(starPath);
        }
        
        break;
        case iShapeTreble:
        {
        uint8_t treblePath[] =
        {   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x59, 0xD2, 0x7D, 0x00, 0x90, 0x6C, 0x17, 0x00, 0x57,
            0x77, 0x77, 0x00, 0x8B, 0x60, 0xB6, 0x00, 0x5B, 0x5B, 0x06, 0x00, 0x8C, 0x22, 0x22, 0x00, 0x01,
            0x00, 0x6A, 0x05, 0xB0, 0x00, 0x8C, 0xEE, 0xEF, 0x00, 0x62, 0x22, 0x22, 0x00, 0x91, 0x11, 0x11,
            0x00, 0x5B, 0x49, 0xF5, 0x00, 0x94, 0x9F, 0x4A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x02, 0x00, 0x4E, 0x3E, 0x94, 0x00, 0x87, 0x66, 0x66, 0x00, 0x4C, 0xCC, 0xCD,
            0x00, 0x87, 0x1C, 0x72, 0x00, 0x4E, 0x88, 0x89, 0x00, 0x85, 0x44, 0x44, 0x00, 0x01, 0x00, 0x51,
            0x22, 0x22, 0x00, 0x82, 0x16, 0xC1, 0x00, 0x53, 0x33, 0x33, 0x00, 0x80, 0xB6, 0x0B, 0x00, 0x59,
            0xF4, 0x9F, 0x00, 0x7C, 0x1C, 0x72, 0x00, 0x01, 0x00, 0x64, 0xEE, 0xEF, 0x00, 0x7E, 0x3E, 0x94,
            0x00, 0x66, 0x66, 0x66, 0x00, 0x86, 0x66, 0x66, 0x00, 0x66, 0xCC, 0xCD, 0x00, 0x88, 0xBB, 0xBC,
            0x00, 0x02, 0x00, 0x65, 0xC7, 0x1C, 0x00, 0x8B, 0x7D, 0x28, 0x00, 0x65, 0xB0, 0x5B, 0x00, 0x8C,
            0xCC, 0xCD, 0x00, 0x63, 0x49, 0xF5, 0x00, 0x8C, 0x44, 0x44, 0x00, 0x01, 0x00, 0x60, 0xDD, 0xDE,
            0x00, 0x8B, 0xEE, 0xEF, 0x00, 0x5D, 0xDD, 0xDE, 0x00, 0x8B, 0x60, 0xB6, 0x00, 0x54, 0x11, 0x11,
            0x00, 0x89, 0x8E, 0x39, 0x00, 0x02, 0x00, 0x56, 0xEE, 0xEF, 0x00, 0x87, 0x38, 0xE4, 0x00, 0x5F,
            0x49, 0xF5, 0x00, 0x85, 0xB0, 0x5B, 0x00, 0x60, 0x49, 0xF5, 0x00, 0x86, 0xDD, 0xDE, 0x00, 0x02,
            0x00, 0x61, 0xD2, 0x7D, 0x00, 0x88, 0x0B, 0x61, 0x00, 0x62, 0xD8, 0x2E, 0x00, 0x89, 0x3E, 0x94,
            0x00, 0x62, 0x60, 0xB6, 0x00, 0x88, 0x88, 0x89, 0x00, 0x01, 0x00, 0x63, 0x22, 0x22, 0x00, 0x87,
            0xDD, 0xDE, 0x00, 0x62, 0xD8, 0x2E, 0x00, 0x87, 0x1C, 0x72, 0x00, 0x61, 0x1C, 0x72, 0x00, 0x82,
            0xAA, 0xAB, 0x00, 0x01, 0x00, 0x58, 0xFA, 0x50, 0x00, 0x82, 0x2D, 0x83, 0x00, 0x55, 0x55, 0x55,
            0x00, 0x84, 0xFA, 0x50, 0x00, 0x54, 0xAA, 0xAB, 0x00, 0x85, 0x77, 0x77, 0x00, 0x01, 0x00, 0x53,
            0xDD, 0xDE, 0x00, 0x88, 0x4F, 0xA5, 0x00, 0x53, 0x33, 0x33, 0x00, 0x88, 0x88, 0x89, 0x00, 0x50,
            0x44, 0x44, 0x00, 0x89, 0x77, 0x77, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x02, 0x00, 0x35, 0xD8, 0x2E, 0x00, 0x8B, 0x3E, 0x94, 0x00, 0x35, 0x55, 0x55, 0x00, 0x8A,
            0xAA, 0xAB, 0x00, 0x36, 0xC7, 0x1C, 0x00, 0x89, 0x9F, 0x4A, 0x00, 0x01, 0x00, 0x39, 0x05, 0xB0,
            0x00, 0x87, 0x7D, 0x28, 0x00, 0x3B, 0x05, 0xB0, 0x00, 0x87, 0x1C, 0x72, 0x00, 0x3C, 0xC1, 0x6C,
            0x00, 0x86, 0xC1, 0x6C, 0x00, 0x02, 0x00, 0x3E, 0xE9, 0x3F, 0x00, 0x87, 0x1C, 0x72, 0x00, 0x40,
            0xB6, 0x0B, 0x00, 0x87, 0x1C, 0x72, 0x00, 0x40, 0xD2, 0x7D, 0x00, 0x87, 0x5B, 0x06, 0x00, 0x02,
            0x00, 0x43, 0xE9, 0x3F, 0x00, 0x87, 0x1C, 0x72, 0x00, 0x44, 0x44, 0x44, 0x00, 0x87, 0x1C, 0x72,
            0x00, 0x43, 0xF4, 0x9F, 0x00, 0x87, 0x55, 0x55, 0x00, 0x01, 0x00, 0x43, 0xD8, 0x2E, 0x00, 0x88,
            0xEE, 0xEF, 0x00, 0x43, 0x8E, 0x39, 0x00, 0x89, 0x3E, 0x94, 0x00, 0x42, 0x60, 0xB6, 0x00, 0x8A,
            0x71, 0xC7, 0x00, 0x01, 0x00, 0x38, 0x4F, 0xA5, 0x00, 0x90, 0x16, 0xC1, 0x00, 0x36, 0x0B, 0x61,
            0x00, 0x8D, 0x82, 0xD8, 0x00, 0x35, 0x93, 0xE9, 0x00, 0x8C, 0xFA, 0x50, 0x00, 0x00, 0x00, 0x1A,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x33, 0x49, 0xF5, 0x00, 0x87, 0x16, 0xC1,
            0x00, 0x2E, 0xEE, 0xEF, 0x00, 0x8A, 0xAA, 0xAB, 0x00, 0x2E, 0xEE, 0xEF, 0x00, 0x8A, 0xAA, 0xAB,
            0x00, 0x01, 0x00, 0x30, 0x9F, 0x4A, 0x00, 0x8D, 0x44, 0x44, 0x00, 0x31, 0x11, 0x11, 0x00, 0x8D,
            0x82, 0xD8, 0x00, 0x37, 0x11, 0x11, 0x00, 0x90, 0xE9, 0x3F, 0x00, 0x01, 0x00, 0x41, 0xC1, 0x6C,
            0x00, 0x8F, 0xDD, 0xDE, 0x00, 0x47, 0x1C, 0x72, 0x00, 0x8C, 0xCC, 0xCD, 0x00, 0x48, 0x16, 0xC1,
            0x00, 0x8C, 0x38, 0xE4, 0x00, 0x01, 0x00, 0x4A, 0x77, 0x77, 0x00, 0x88, 0xB0, 0x5B, 0x00, 0x4B,
            0x60, 0xB6, 0x00, 0x88, 0x88, 0x89, 0x00, 0x4C, 0x93, 0xE9, 0x00, 0x88, 0x49, 0xF5, 0x00, 0x02,
            0x00, 0x50, 0x44, 0x44, 0x00, 0x89, 0x9F, 0x4A, 0x00, 0x52, 0x7D, 0x28, 0x00, 0x89, 0xF4, 0x9F,
            0x00, 0x52, 0xD8, 0x2E, 0x00, 0x8C, 0xA4, 0xFA, 0x00, 0x01, 0x00, 0x52, 0x38, 0xE4, 0x00, 0x8E,
            0x38, 0xE4, 0x00, 0x53, 0x33, 0x33, 0x00, 0x90, 0x5B, 0x06, 0x00, 0x55, 0xC7, 0x1C, 0x00, 0x96,
            0x00, 0x00, 0x00, 0x01, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x96, 0x05, 0xB0, 0x00, 0x63, 0x8E, 0x39,
            0x00, 0x92, 0x7D, 0x28, 0x00, 0x65, 0x1C, 0x72, 0x00, 0x91, 0x44, 0x44, 0x00, 0x02, 0x00, 0x65,
            0x49, 0xF5, 0x00, 0x8F, 0xF4, 0x9F, 0x00, 0x66, 0x66, 0x66, 0x00, 0x8E, 0x38, 0xE4, 0x00, 0x6B,
            0x77, 0x77, 0x00, 0x8F, 0x55, 0x55, 0x00, 0x01, 0x00, 0x75, 0xE9, 0x3F, 0x00, 0x92, 0x9F, 0x4A,
            0x00, 0x78, 0x2D, 0x83, 0x00, 0x8C, 0xCC, 0xCD, 0x00, 0x78, 0x77, 0x77, 0x00, 0x8C, 0x05, 0xB0,
            0x00, 0x02, 0x00, 0x77, 0xB6, 0x0B, 0x00, 0x8B, 0x5B, 0x06, 0x00, 0x78, 0x2D, 0x83, 0x00, 0x8A,
            0xAA, 0xAB, 0x00, 0x78, 0x2D, 0x83, 0x00, 0x8A, 0xAA, 0xAB, 0x00, 0x01, 0x00, 0x79, 0x1C, 0x72,
            0x00, 0x86, 0x49, 0xF5, 0x00, 0x78, 0x2D, 0x83, 0x00, 0x84, 0x44, 0x44, 0x00, 0x75, 0xA4, 0xFA,
            0x00, 0x7E, 0xD2, 0x7D, 0x00, 0x02, 0x00, 0x70, 0xC1, 0x6C, 0x00, 0x81, 0x3E, 0x94, 0x00, 0x6D,
            0x82, 0xD8, 0x00, 0x83, 0x8E, 0x39, 0x00, 0x6D, 0xC1, 0x6C, 0x00, 0x84, 0x9F, 0x4A, 0x00, 0x02,
            0x00, 0x6D, 0x82, 0xD8, 0x00, 0x86, 0x05, 0xB0, 0x00, 0x6D, 0x82, 0xD8, 0x00, 0x87, 0x1C, 0x72,
            0x00, 0x6D, 0xA4, 0xFA, 0x00, 0x87, 0x2D, 0x83, 0x00, 0x02, 0x00, 0x6E, 0xEE, 0xEF, 0x00, 0x87,
            0xD2, 0x7D, 0x00, 0x6E, 0xEE, 0xEF, 0x00, 0x87, 0xD2, 0x7D, 0x00, 0x6E, 0xEE, 0xEF, 0x00, 0x87,
            0xD2, 0x7D, 0x00, 0x02, 0x00, 0x6F, 0x8E, 0x39, 0x00, 0x89, 0x16, 0xC1, 0x00, 0x6F, 0xA4, 0xFA,
            0x00, 0x89, 0x3E, 0x94, 0x00, 0x70, 0x66, 0x66, 0x00, 0x89, 0x27, 0xD2, 0x00, 0x02, 0x00, 0x73,
            0x9F, 0x4A, 0x00, 0x88, 0x93, 0xE9, 0x00, 0x73, 0x33, 0x33, 0x00, 0x88, 0x88, 0x89, 0x00, 0x73,
            0x33, 0x33, 0x00, 0x88, 0x88, 0x89, 0x00, 0x02, 0x00, 0x75, 0x55, 0x55, 0x00, 0x87, 0xD2, 0x7D,
            0x00, 0x75, 0x55, 0x55, 0x00, 0x87, 0xD2, 0x7D, 0x00, 0x75, 0x55, 0x55, 0x00, 0x87, 0x93, 0xE9,
            0x00, 0x02, 0x00, 0x75, 0x2D, 0x83, 0x00, 0x85, 0xD8, 0x2E, 0x00, 0x75, 0x55, 0x55, 0x00, 0x85,
            0xB0, 0x5B, 0x00, 0x75, 0xC7, 0x1C, 0x00, 0x85, 0xD8, 0x2E, 0x00, 0x02, 0x00, 0x77, 0x77, 0x77,
            0x00, 0x86, 0x66, 0x66, 0x00, 0x77, 0x77, 0x77, 0x00, 0x86, 0x66, 0x66, 0x00, 0x77, 0x33, 0x33,
            0x00, 0x87, 0xC1, 0x6C, 0x00, 0x01, 0x00, 0x77, 0xD2, 0x7D, 0x00, 0x89, 0x4F, 0xA5, 0x00, 0x77,
            0x77, 0x77, 0x00, 0x8A, 0xAA, 0xAB, 0x00, 0x75, 0xDD, 0xDE, 0x00, 0x90, 0x82, 0xD8, 0x00, 0x02,
            0x00, 0x6D, 0xF4, 0x9F, 0x00, 0x8E, 0x4F, 0xA5, 0x00, 0x68, 0x88, 0x89, 0x00, 0x8D, 0x82, 0xD8,
            0x00, 0x68, 0x77, 0x77, 0x00, 0x8C, 0x11, 0x11, 0x00, 0x02, 0x00, 0x68, 0x22, 0x22, 0x00, 0x8D,
            0x11, 0x11, 0x00, 0x67, 0x1C, 0x72, 0x00, 0x8C, 0xCC, 0xCD, 0x00, 0x67, 0x4F, 0xA5, 0x00, 0x8A,
            0x11, 0x11, 0x00, 0x01, 0x00, 0x68, 0x4F, 0xA5, 0x00, 0x86, 0xA4, 0xFA, 0x00, 0x67, 0x1C, 0x72,
            0x00, 0x83, 0x8E, 0x39, 0x00, 0x64, 0x9F, 0x4A, 0x00, 0x7D, 0x38, 0xE4, 0x00, 0x01, 0x00, 0x59,
            0x88, 0x89, 0x00, 0x78, 0x71, 0xC7, 0x00, 0x51, 0x11, 0x11, 0x00, 0x7C, 0x71, 0xC7, 0x00, 0x4E,
            0x8E, 0x39, 0x00, 0x7D, 0x99, 0x9A, 0x00, 0x01, 0x00, 0x46, 0xF4, 0x9F, 0x00, 0x86, 0x27, 0xD2,
            0x00, 0x45, 0xB0, 0x5B, 0x00, 0x86, 0x66, 0x66, 0x00, 0x45, 0x27, 0xD2, 0x00, 0x86, 0x7D, 0x28,
            0x00, 0x01, 0x00, 0x41, 0xC7, 0x1C, 0x00, 0x85, 0x05, 0xB0, 0x00, 0x41, 0x6C, 0x17, 0x00, 0x84,
            0xFA, 0x50, 0x00, 0x38, 0x22, 0x22, 0x00, 0x83, 0xB0, 0x5B
        };

        pathData = treblePath;
        pathSize = sizeof(treblePath);
        }
        
        break;
        
        case iShapeRibbon:
        {
        uint8_t ribbonPath[] =
        {   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x5C, 0x33, 0x33, 0x00, 0x66, 0xB0, 0x5B, 0x00, 0x5C,
            0x71, 0xC7, 0x00, 0x67, 0x1C, 0x72, 0x00, 0x60, 0x27, 0xD2, 0x00, 0x6D, 0x88, 0x89, 0x00, 0x02,
            0x00, 0x65, 0xC1, 0x6C, 0x00, 0x68, 0x49, 0xF5, 0x00, 0x69, 0xF4, 0x9F, 0x00, 0x65, 0xB0, 0x5B,
            0x00, 0x6B, 0x88, 0x89, 0x00, 0x66, 0xE3, 0x8E, 0x00, 0x01, 0x00, 0x6D, 0xAA, 0xAB, 0x00, 0x6A,
            0x22, 0x22, 0x00, 0x6F, 0xA4, 0xFA, 0x00, 0x6A, 0xAA, 0xAB, 0x00, 0x70, 0x71, 0xC7, 0x00, 0x6A,
            0xDD, 0xDE, 0x00, 0x02, 0x00, 0x71, 0xB0, 0x5B, 0x00, 0x6A, 0x5B, 0x06, 0x00, 0x72, 0x7D, 0x28,
            0x00, 0x6A, 0xAA, 0xAB, 0x00, 0x73, 0xB0, 0x5B, 0x00, 0x69, 0x60, 0xB6, 0x00, 0x01, 0x00, 0x75,
            0xA4, 0xFA, 0x00, 0x66, 0x1C, 0x72, 0x00, 0x77, 0x77, 0x77, 0x00, 0x65, 0xB0, 0x5B, 0x00, 0x78,
            0x0B, 0x61, 0x00, 0x65, 0x88, 0x89, 0x00, 0x01, 0x00, 0x7C, 0x9F, 0x4A, 0x00, 0x68, 0x00, 0x00,
            0x00, 0x7B, 0x05, 0xB0, 0x00, 0x65, 0xB0, 0x5B, 0x00, 0x77, 0x2D, 0x83, 0x00, 0x60, 0x27, 0xD2,
            0x00, 0x02, 0x00, 0x72, 0x6C, 0x17, 0x00, 0x64, 0xDD, 0xDE, 0x00, 0x6F, 0xA4, 0xFA, 0x00, 0x67,
            0xD2, 0x7D, 0x00, 0x6E, 0x11, 0x11, 0x00, 0x67, 0x16, 0xC1, 0x00, 0x01, 0x00, 0x6C, 0xCC, 0xCD,
            0x00, 0x67, 0x4F, 0xA5, 0x00, 0x6B, 0x60, 0xB6, 0x00, 0x66, 0x66, 0x66, 0x00, 0x69, 0x00, 0x00,
            0x00, 0x64, 0xD8, 0x2E, 0x00, 0x01, 0x00, 0x68, 0x60, 0xB6, 0x00, 0x61, 0xA4, 0xFA, 0x00, 0x64,
            0x44, 0x44, 0x00, 0x63, 0x8E, 0x39, 0x00, 0x63, 0x60, 0xB6, 0x00, 0x63, 0xF4, 0x9F, 0x00, 0x01,
            0x00, 0x62, 0x4F, 0xA5, 0x00, 0x66, 0xC7, 0x1C, 0x00, 0x61, 0x6C, 0x17, 0x00, 0x67, 0x1C, 0x72,
            0x00, 0x61, 0x6C, 0x17, 0x00, 0x67, 0x1C, 0x72, 0x00, 0x02, 0x00, 0x60, 0x00, 0x00, 0x00, 0x67,
            0x1C, 0x72, 0x00, 0x60, 0x00, 0x00, 0x00, 0x67, 0x1C, 0x72, 0x00, 0x60, 0x0B, 0x61, 0x00, 0x67,
            0x1C, 0x72
        };
        
        pathData = ribbonPath;
        pathSize = sizeof(ribbonPath);
        }
        
        break;
        
        case iShapeNote:
        {
        uint8_t notePath[] =
        {   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x83, 0x5B, 0x06, 0x00, 0x6A, 0xB0, 0x5B, 0x00, 0x7E,
            0x93, 0xE9, 0x00, 0x62, 0xD8, 0x2E, 0x00, 0x88, 0x71, 0xC7, 0x00, 0x65, 0xBB, 0xBC, 0x00, 0x02,
            0x00, 0x89, 0x66, 0x66, 0x00, 0x6A, 0x60, 0xB6, 0x00, 0x90, 0x5B, 0x06, 0x00, 0x6F, 0xA4, 0xFA,
            0x00, 0x84, 0xBB, 0xBC, 0x00, 0x6E, 0x11, 0x11, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x02, 0x00, 0x80, 0xE9, 0x3F, 0x00, 0x61, 0x99, 0x9A, 0x00, 0x73, 0xE9, 0x3F,
            0x00, 0x62, 0x22, 0x22, 0x00, 0x7B, 0x9F, 0x4A, 0x00, 0x64, 0xBB, 0xBC, 0x00, 0x01, 0x00, 0x7D,
            0xFA, 0x50, 0x00, 0x6B, 0xA4, 0xFA, 0x00, 0x84, 0x44, 0x44, 0x00, 0x6E, 0xEE, 0xEF, 0x00, 0x89,
            0x8E, 0x39, 0x00, 0x71, 0xB0, 0x5B, 0x00, 0x01, 0x00, 0x8D, 0x11, 0x11, 0x00, 0x6F, 0x82, 0xD8,
            0x00, 0x93, 0x33, 0x33, 0x00, 0x70, 0x5B, 0x06, 0x00, 0x93, 0x33, 0x33, 0x00, 0x70, 0x5B, 0x06,
            0x00, 0x01, 0x00, 0x96, 0x0B, 0x61, 0x00, 0x71, 0xC7, 0x1C, 0x00, 0x96, 0x0B, 0x61, 0x00, 0x71,
            0xC7, 0x1C, 0x00, 0x99, 0xFA, 0x50, 0x00, 0x72, 0x2D, 0x83, 0x00, 0x01, 0x00, 0x9E, 0xEE, 0xEF,
            0x00, 0x71, 0x38, 0xE4, 0x00, 0xA1, 0x6C, 0x17, 0x00, 0x6F, 0xA4, 0xFA, 0x00, 0xA1, 0x9F, 0x4A,
            0x00, 0x6F, 0x82, 0xD8, 0x00, 0x02, 0x00, 0xA1, 0xF4, 0x9F, 0x00, 0x6E, 0x49, 0xF5, 0x00, 0xA2,
            0x22, 0x22, 0x00, 0x6E, 0x38, 0xE4, 0x00, 0x92, 0x7D, 0x28, 0x00, 0x72, 0xFA, 0x50, 0x00, 0x02,
            0x00, 0x90, 0xEE, 0xEF, 0x00, 0x6D, 0x22, 0x22, 0x00, 0x8A, 0xAA, 0xAB, 0x00, 0x62, 0xD8, 0x2E,
            0x00, 0x91, 0xF4, 0x9F, 0x00, 0x62, 0xD8, 0x2E, 0x00, 0x01, 0x00, 0x9E, 0xD2, 0x7D, 0x00, 0x64,
            0x9F, 0x4A, 0x00, 0xA5, 0xB0, 0x5B, 0x00, 0x62, 0xD8, 0x2E, 0x00, 0xAC, 0x0B, 0x61, 0x00, 0x61,
            0x27, 0xD2, 0x00, 0x01, 0x00, 0xB1, 0x60, 0xB6, 0x00, 0x4F, 0x55, 0x55, 0x00, 0xA5, 0xB0, 0x5B,
            0x00, 0x51, 0xC7, 0x1C, 0x00, 0xA4, 0xF4, 0x9F, 0x00, 0x51, 0xE9, 0x3F, 0x00, 0x01, 0x00, 0xA4,
            0x0B, 0x61, 0x00, 0x53, 0x77, 0x77, 0x00, 0xA3, 0x8E, 0x39, 0x00, 0x53, 0xE9, 0x3F, 0x00, 0x9E,
            0xC1, 0x6C, 0x00, 0x58, 0x05, 0xB0, 0x00, 0x02, 0x00, 0x9E, 0x6C, 0x17, 0x00, 0x59, 0xAA, 0xAB,
            0x00, 0x9F, 0x49, 0xF5, 0x00, 0x61, 0x6C, 0x17, 0x00, 0x91, 0x11, 0x11, 0x00, 0x61, 0x6C, 0x17
        };
        
        pathData = notePath;
        pathSize = sizeof(notePath);
        }
        
        break;
        
        
        default:
        break;

    }
    
    h = HostNewHandle(procs, pathSize);
    if (h)
    {
        Ptr p = HostLockHandle(procs, h, TRUE);
        HostCopy(p, pathData, pathSize);
        HostUnlockHandle(procs, h);
    }
    
    return h;
}

// end Shape.cpp
