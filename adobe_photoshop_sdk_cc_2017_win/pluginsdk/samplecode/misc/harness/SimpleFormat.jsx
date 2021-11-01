﻿// call a function so we can hide variables from other scripts
main();

function main() {

LogMessage( "Starting test: SimpleFormat" );

// Save the current preferences
var startRulerUnits = preferences.rulerUnits;
var startTypeUnits = preferences.typeUnits;
var startDisplayDialogs = displayDialogs;

// Set Photoshop to use pixels and display no dialogs
preferences.rulerUnits = Units.PIXELS;
preferences.typeUnits = TypeUnits.PIXELS;
displayDialogs = DialogModes.NO;

var maxTime = 5 * 60;
var iterations = 5;

var timeIt = new Timer();

var tests = 0;
var errors = 0;

var eArray = new Array();

var simpleFormatLog = new File( Folder.desktop + "/SimpleFormat.log" );
simpleFormatLog.open( "w", "TEXT", "????" );
simpleFormatLog.writeln( "width,\theight,\tdepth,\tsave time,\topen time,\tmax est size" );

var maxWidth = 30000;
var minWidth = 256;
var incWidth = parseInt( (maxWidth - minWidth) / iterations );
if ( incWidth == 0 )
	incWidth = 1;

var maxHeight = 30000;
var minHeight = 256;
var incHeight = parseInt( (maxHeight - minHeight) / iterations );
if ( incHeight == 0 )
	incHeight = 1;

var modes = [ "indX", "rgbC", "bitM", "gryS" ];
var modesIndex = 0;

var invert = false;
var transparent = false;
var itIndex = 0;

var maxEstSize = 0;


// start clean
while (documents.length) {
    activeDocument.close(SaveOptions.DONOTSAVECHANGES);
}

var f = new File( Folder.desktop + "/SimpleFormatTest.sme" );

var d = BitsPerChannelType.EIGHT;

for ( var w = minWidth; w < maxWidth; w += incWidth ) {
	for ( var h = minHeight; h < maxHeight; h += incHeight ) {
			
		try {
			
			var timeSave = new Timer();
				
			if ( d == BitsPerChannelType.EIGHT )
				d = BitsPerChannelType.SIXTEEN;
			else
				d = BitsPerChannelType.EIGHT;

			// i can only go to 2 gig on files, and let's round down to 80% for overhead
			var estFileSize = w * h * 3 * ( d == BitsPerChannelType.EIGHT ? 1 : 2 );
			if ( estFileSize >= ( 2147483648 * 0.8 ) ) {
				LogMessage( "skipping" );
				continue;
			}

			if ( estFileSize > maxEstSize )
				maxEstSize = estFileSize;

			LogMessage( "Est: " + estFileSize );

			app.documents.add(UnitValue(w, "px"), UnitValue(h,"px"), undefined, "SimpleFormat Test", undefined, undefined, undefined, d);

            SimpleFormatSave( f );
             
            tests++;
             
            simpleFormatLog.write( w + ",\t" + h + ",\t" + d + ",\t" + timeSave.getElapsed() + ",\t" );
             
            // document should not be dirty
            activeDocument.close();
             
            var timeOpen = new Timer();
             
            SimpleFormatOpen( f );
             
            simpleFormatLog.writeln( timeOpen.getElapsed() + ",\t" + estFileSize );

            if ( activeDocument.width != w ) 
                errors++;
			if ( activeDocument.height != h ) 
                errors++;
             
             // we open as smart object so the document is dirty
             activeDocument.close( SaveOptions.DONOTSAVECHANGES );

             if ( timeIt.getElapsed() > maxTime ) {
                w = maxWidth + 1;
                h = maxHeight + 1;
            }
         
            tests++;
	
		}
		catch(e) {
			if ( e.message.search(/cancel/i) != -1 ) {
				w = maxWidth + 1;
				h = maxHeight + 1;
			}
			eArray[eArray.length] = e;
			errors++;
			// debugger;
		} // end catch
	} // end for height
} // end for width

simpleFormatLog.writeln( errors + " errors. " + tests + " tests in " + timeIt.getElapsed() + " seconds. " + tests/timeIt.getElapsed() + " tests/sec.");
simpleFormatLog.writeln( maxEstSize + " was the largest estimated size." )
simpleFormatLog.close();
simpleFormatLog.execute();

if ( typeof gHarness == "undefined" || gHarness.showAlerts ) {
	if ( errors == 0 )
		alert( " PASS" );
	else
		alert( " FAIL" );
}

// end clean
while (documents.length) {
    activeDocument.close(SaveOptions.DONOTSAVECHANGES);
}

f.remove();

// Reset the application preferences
preferences.rulerUnits = startRulerUnits;
preferences.typeUnits = startTypeUnits;
displayDialogs = startDisplayDialogs;

//     1) " FAIL" for failures
//     2) " PASS" for test results OK
//     3) "  BUG" for known bugs, have the file name give the bug number
//     4) "ERROR" this comes from the harness if the script barfed/exception,
return errors == 0 ? ' PASS' : ' FAIL';

} // end function main

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// SimpleFormatSave( f )
//////////////////////////////////////////////////////////////////////////////
function SimpleFormatSave( f ) {
    var idsave = charIDToTypeID( "save" );
    var desc5 = new ActionDescriptor();
    var idAs = charIDToTypeID( "As  " );
    var desc6 = new ActionDescriptor();
    var idAdobeSDKSimpleFormat = stringIDToTypeID( "AdobeSDK SimpleFormat" );
    desc5.putObject( idAs, idAdobeSDKSimpleFormat, desc6 );
    var idIn = charIDToTypeID( "In  " );
    desc5.putPath( idIn, f );
    executeAction( idsave, desc5, DialogModes.NO );
}

//////////////////////////////////////////////////////////////////////////////
// SimpleFormatOpen( f )
//////////////////////////////////////////////////////////////////////////////
function SimpleFormatOpen( f ) {
    var idOpn = charIDToTypeID( "Opn " );
    var desc7 = new ActionDescriptor();
    var idnull = charIDToTypeID( "null" );
    desc7.putPath( idnull, f );
    var idAs = charIDToTypeID( "As  " );
    var desc8 = new ActionDescriptor();
    var idAdobeSDKSimpleFormat = stringIDToTypeID( "AdobeSDK SimpleFormat" );
    desc7.putObject( idAs, idAdobeSDKSimpleFormat, desc8 );
    executeAction( idOpn, desc7, DialogModes.NO );
}

/****************************
  Function WaitForRedraw
  Usage: Use it to force Photoshop to redraw the screen before continuing
  Example: 
       WaitForRedraw();
****************************/
function WaitForRedraw() {
	var keyID = charIDToTypeID( "Stte" );
	var desc = new ActionDescriptor();
	desc.putEnumerated( keyID, keyID, charIDToTypeID( "RdCm" ) );
	executeAction( charIDToTypeID( "Wait" ), desc, DialogModes.NO );
}

//////////////////////////////////////////////////////////////////////////////
// WaitNSeconds, slow the script down so you can watch and figure out issues
//////////////////////////////////////////////////////////////////////////////
function WaitNSeconds(seconds) {
   startDate = new Date();
   endDate = new Date();
   while ((endDate.getTime() - startDate.getTime()) < (1000 * seconds)) 
		endDate = new Date();
}

//////////////////////////////////////////////////////////////////////////////
// FitOnScreen, fits the document and redraws the screen
//////////////////////////////////////////////////////////////////////////////
function FitOnScreen() {
	var id45 = charIDToTypeID( "slct" );
    var desc7 = new ActionDescriptor();
    var id46 = charIDToTypeID( "null" );
	var ref1 = new ActionReference();
	var id47 = charIDToTypeID( "Mn  " );
	var id48 = charIDToTypeID( "MnIt" );
	var id49 = charIDToTypeID( "FtOn" );
	ref1.putEnumerated( id47, id48, id49 );
	desc7.putReference( id46, ref1 );
	executeAction( id45, desc7, DialogModes.NO );
}

//////////////////////////////////////////////////////////////////////////////
// Library for timing things in JavaScript
//////////////////////////////////////////////////////////////////////////////
function Timer() {
	// member variables
	this.startTime = new Date();
	this.endTime = new Date();
	
	// member functions
	
	// reset the start time to now
	this.start = function () { this.startTime = new Date(); }
	
	// reset the end time to now
	this.stop = function () { this.endTime = new Date(); }
	
	// get the difference in milliseconds between start and stop
	this.getTime = function () { return (this.endTime.getTime() - this.startTime.getTime()) / 1000; }
	
	// get the current elapsed time from start to now, this sets the endTime
	this.getElapsed = function () { this.endTime = new Date(); return this.getTime(); }
}

function LogMessage( inMessage ) {
	if ( BridgeTalk.isRunning( "estoolkit" ) )
		$.writeln( inMessage );
}

// end SimpleFormat.jsx
