/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */
<!--  to hide script contents from old browsers

//the file that includes this must also include settings.js and nciglob.js
var loaded = false;

function loadScript(fileName)
{

	var FormName = "NCISCRIPTEDLOGIN";
	
	dirty(false);
	
	if ((fileName != null) && (fileName != ""))
	{
		parent.parent.nciglobals.setText(FormName, "ScriptFileName", fileName);
		var theScript = parent.parent.nciglobals.GetNameValuePair(fileName, null, null);
		if ((theScript != null))
		{
			parent.parent.nciglobals.setText(FormName, "LOGINSCRIPT", theScript);
			parent.parent.nciglobals.setGlobal("ScriptEnabled", "yes");

		}
		else
			parent.parent.nciglobals.setText(FormName, "LOGINSCRIPT", "");
	}
	else
	{
		fileName = "";
		parent.parent.nciglobals.setText(FormName, "ScriptFileName", "");
		parent.parent.nciglobals.setText(FormName, "LOGINSCRIPT", "");
		parent.parent.nciglobals.setGlobal("ScriptEnabled", "no");
		parent.parent.nciglobals.setGlobal("ScriptFileName", "");

	}	
	parent.parent.nciglobals.setSelectList(FormName, "SCPList", fileName);
	this.focus();
	document.forms[0]["SCPList"].focus();
}

function loadData()
{
	if (parent.parent.nciglobals && parent.parent.nciglobals.getText)
	{
		var FormName = "NCISCRIPTEDLOGIN";
		var selectedScript = "";
		
		parent.parent.nciglobals.fillSelectListWithConfigFiles(FormName, "SCPList", ".SCP", false);
		selectedScript = parent.parent.nciglobals.getGlobal("ScriptFileName");
		
		loadScript(selectedScript);
		loaded = true;
	}
	else
		setTimeout("loadData()",500);
}

function checkData()
{
	return true;
}

function saveData()
{
	if (loaded && loaded == true)
	{
		var FormName 	= "NCISCRIPTEDLOGIN";
		var save = null;
	
		if (dirty(null))
		{
			if( confirm("You have made changes to the login script without saving.  Would you like to save them?"))
				save = saveNewOrOldScript();
		}
	
		//now check if we have a valid script file selected & set the globals accordingly
		if (save == null)
		{
			save  = parent.parent.nciglobals.getSelectListSelection(FormName, "SCPList");
			if ((save == "_new_") || (save == ""))
				save = null;	
		}
	
		if (save != null)
		{
			parent.parent.nciglobals.setGlobal("ScriptFileName", save);
			parent.parent.nciglobals.setGlobal("ScriptEnabled", "yes");
		}
		else
		{
			parent.parent.nciglobals.setGlobal("ScriptFileName", "");
			parent.parent.nciglobals.setGlobal("ScriptEnabled", "no");
		}
	}
}


function forceSCPSuffix()
{
	var FormName 	= "NCISCRIPTEDLOGIN";

	var fName = "";
	
	fName = suggestSCPFileName(null);

	parent.parent.nciglobals.setText(FormName, "ScriptFileName", fName);
	
	return fName;
}


function suggestSCPFileName(inFName)
{
	
	parent.parent.nciglobals.debug("I'm in forceSCPsuffix");

	var FormName 	= "NCISCRIPTEDLOGIN";
	var fname		= "";
	
	if ((inFName == null) || (inFName == ""))
		fname = parent.parent.nciglobals.getText(FormName, "ScriptFileName");
	else
		fname = inFName;
		
	var outName = "";
	
	if ((fname != null) && (fname != ""))
	{
	
		var len 	= 	fname.length;
		var finLen	=	0;

		if (fname.substring(len-4, len) == ".SCP")
			len -= 4;
		
		for (var i = 0; i < len; i++)
		{
			if (isAlphaNumeric(fname.charAt(i)))
			{
				outName = outName + fname.charAt(i);
				finLen++;
			}
			if (finLen >= 8)
				break;
		}
		fname = (outName + ".SCP"); 			
	}
	else 
		fname = "";
	return fname;
}

function dirty(inDirt)
{

	//parent.parent.nciglobals.debug("I'm in dirty");

	var isDirty = parent.parent.nciglobals.getGlobal("ScriptDirty");
	
	if ((inDirt != null) && (inDirt == true))
		parent.parent.nciglobals.setGlobal("ScriptDirty", "yes");
	else if ((inDirt != null) && (inDirt == false))
		parent.parent.nciglobals.setGlobal("ScriptDirty", "no");
		
	if ((isDirty != null) && (isDirty == "yes"))
		return true;
	else 
		return false;
}

function saveScript()
{
	//parent.parent.nciglobals.debug("I'm in savescript");
	var FormName = "NCISCRIPTEDLOGIN";

	var saveScriptName = null;
	
	saveScriptName = saveNewOrOldScript();
	//alert("SaveScriptName: " + saveScriptName);
	
	if ((saveScriptName != null) && (saveScriptName != ""))		
	{		
		parent.parent.nciglobals.fillSelectListWithConfigFiles(FormName, "SCPList", ".SCP", false);
		parent.parent.nciglobals.setSelectList(FormName, "SCPList", saveScriptName);
		loadScript(saveScriptName);
	}
}



function saveNewOrOldScript()
{
	var FormName = "NCISCRIPTEDLOGIN";
	var theScriptFileName = parent.parent.nciglobals.getText(FormName, "ScriptFileName");
	var selScriptFileName = parent.parent.nciglobals.getSelectListSelection(FormName, "SCPList");
	var theScript = parent.parent.nciglobals.getText(FormName, "LOGINSCRIPT");
	var save = null;
	var endFileName = "";
	
	
	//case 1: if the script is an existing script, save without asking
	if ((selScriptFileName != null) && (selScriptFileName != "") && (selScriptFileName == theScriptFileName))
		save = true;
	//case 2: if a new script is there and has not been selected, ask for a fileName
	// OR if a filename has been entered, check against existing fileNames
	else if ((theScript != null) && (theScript != ""))
	{
		theScriptFileName = askSCPFileName(theScriptFileName);
		if ((theScriptFileName != null) && (theScriptFileName != ""))
			save = true;
	}

	if ((save != null) && (save == true))		
	{		
		dirty(false);
		parent.parent.nciglobals.saveNewConfigFile(theScriptFileName, theScript, false);
	}

	if (save == true)
		return	theScriptFileName;
	else
		return null;
}

function checkIfSCPFileExists(inFileName)
{
	var outValue = false;
	
	if ((inFileName != null) && (inFileName != ""))
	{	
		fileList = parent.parent.nciglobals.getFileListFromConfigFolder(".SCP");
		
		if (fileList != null)	
		{
			for (var i=0; i<fileList.length; i++)	
			{
				if (fileList[i] == inFileName)
					outValue = true;
			}
		}
	}
	return outValue;
}


function askSCPFileName(inFileName)
{
	var save 	= 	null;
	var sName 	= 	inFileName;
	var fName	= 	null;
	
	//now, if there no fileName, suggest an SCP fileName, prompt for confirmation
	//parent.parent.nciglobals.debug( "about to suggest..." );
	while (save == null)
	{
		parent.parent.nciglobals.debug( "in that while loop..." );
		
		if ((!sName) || (sName == null) || (sName == ""))
		{
			var sgName = suggestSCPFileName(fName);
			fName = prompt("Enter the file name for this script (must end with .SCP)", sgName);
			
			//if they entered an improper suffix, prompt again, and again
			while ((fName != null) && (fName.substring(fName.length-4, fName.length)  != ".SCP"))
			{
				sgName = suggestSCPFileName(fName);
				fName = prompt("Enter the file name for this script (must end with .SCP)", sgName);
			}
		}
		else
			fName = sName;
		
		// if the name exists, prompt to replace
		
		if ((fName != null) && (checkIfSCPFileExists(fName)))
		{
			var conf = confirm("The file: " + fName + " already exists.  Replace?");
			if (conf)
			{ 
				save = true
			}
			else
			{
				sName = null;
				save = null;
			}
		}
		else if(fName != null)
		{
			//fileName doesn't exist - we can save
			save = true;
		}
		else
		{
			//user canceled - name = null
			save = false;		
		}
	}
	
	//now save the file if the user didn't cancel
	if (save == true)
	{
		//return fileName
		return fName;
	}
	else
	{
		//alert("Could not save this configuration because you did not provide a Name");
		return null;
	}
}

function isAlphaNumeric(inLetter)
{

	var outValue = false;
	
	if ((inLetter != null) && (inLetter != ""))
	{

		if (!isNaN(parseInt(inLetter)))
			outValue = true;
		else
		{
			var let = inLetter.toLowerCase();
			if ((let <= "z") && ( let >= "a"))
				outValue = true;
			else if (let == "_")
				outValue = true;
		}
	}
	return outValue;
}


// end hiding contents from old browsers  -->
