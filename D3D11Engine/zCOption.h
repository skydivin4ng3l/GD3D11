#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include <algorithm>
#include "zSTRING.h"

class zCOption
{
public:
	
	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCOptionReadInt = (zCOptionReadInt)DetourFunction((BYTE *)GothicMemoryLocations::zCOption::ReadInt, (BYTE *)zCOption::hooked_zOptionReadInt);
		HookedFunctions::OriginalFunctions.original_zCOptionReadBool = (zCOptionReadInt)DetourFunction((BYTE *)GothicMemoryLocations::zCOption::ReadBool, (BYTE *)zCOption::hooked_zOptionReadBool);
		HookedFunctions::OriginalFunctions.original_zCOptionReadDWORD = (zCOptionReadDWORD)DetourFunction((BYTE *)GothicMemoryLocations::zCOption::ReadDWORD, (BYTE *)zCOption::hooked_zOptionReadDWORD);
	}

	/** Returns true if the given string is in the commandline of the game */
	bool IsParameter(const std::string & str)
	{
#ifdef BUILD_GOTHIC_1_08k
		return false; // FIXME
#endif

		zSTRING* zCmdline = (zSTRING *)THISPTR_OFFSET(GothicMemoryLocations::zCOption::Offset_CommandLine);
		std::string cmdLine = zCmdline->ToChar();
		std::string cmd = str;

		// Make them uppercase
		std::transform(cmdLine.begin(), cmdLine.end(),cmdLine.begin(), ::toupper);
		std::transform(cmd.begin(), cmd.end(),cmd.begin(), ::toupper);

		return cmdLine.find("-" + cmd) != std::string::npos;
	}

	/** Returns the commandline */
	std::string GetCommandline()
	{
#ifdef BUILD_GOTHIC_2_6_fix
		zSTRING* zCmdline = (zSTRING *)THISPTR_OFFSET(GothicMemoryLocations::zCOption::Offset_CommandLine);
		std::string cmdLine = zCmdline->ToChar();

		return cmdLine;
#endif
		return "";
	}

	/** Returns the value of the given parameter. If the parameter is not in the commandline, it returns "" */
	std::string ParameterValue(const std::string & str)
	{
#ifdef BUILD_GOTHIC_1_08k
		return ""; // TODO
#endif

		zSTRING* zCmdline = (zSTRING *)THISPTR_OFFSET(GothicMemoryLocations::zCOption::Offset_CommandLine);
		std::string cmdLine = zCmdline->ToChar();
		std::string cmd = str;

		// Make them uppercase
		std::transform(cmdLine.begin(), cmdLine.end(),cmdLine.begin(), ::toupper);
		std::transform(cmd.begin(), cmd.end(),cmd.begin(), ::toupper);

		int pos = cmdLine.find("-" + cmd);
		if (pos == std::string::npos)
			return ""; // Not in commandline

		unsigned int paramPos = pos + 1 + cmd.length() + 1; // Skip everything until the -, then 
								     			   // the -, then the param-name and finally the :
		// Safety-check
		if (paramPos >= cmdLine.length())
			return "";

		// *Snip*
		std::string arg = cmdLine.substr(paramPos); 
		
		// Snip the rest of the commandline, if there is any
		if (arg.find_first_of(' ') != std::string::npos)
			arg = arg.substr(0, arg.find_first_of(' '));
		
		return arg;
	}

	/** Reads config stuff */
	static int __fastcall hooked_zOptionReadBool(void * thisptr, void * unknwn,zSTRING const& section, char const* var, int def)
	{
		
		int r = HookedFunctions::OriginalFunctions.original_zCOptionReadBool(thisptr, section, var, def);
		if (_stricmp(var, "zWaterAniEnabled") == 0)
		{
			Engine::GAPI->SetIntParamFromConfig("zWaterAniEnabled", 0);
			return 0; // Disable water animations
		} else if (_stricmp(var, "scaleVideos") == 0) // Force scaleVideos to get them into the upper left corner
		{
			Engine::GAPI->SetIntParamFromConfig("scaleVideos", 0);
			return 0;
		} else if (_stricmp(var, "zStartupWindowed") == 0)
		{
			Engine::GAPI->SetIntParamFromConfig("zStartupWindowed", r);
			return 1;
		} else if (_stricmp(var, "gameAbnormalExit") == 0)
		{
#ifndef PUBLIC_RELEASE
			// No VDFS bullshit when testing
			return 0;
#endif
		}

		

		
		Engine::GAPI->SetIntParamFromConfig(var, r);
		return r;
	}

	/** Reads config stuff */
	static long __fastcall Do_hooked_zOptionReadInt(void * thisptr, zSTRING const& section, char const* var, int def)
	{
		BaseGraphicsEngine* engine = Engine::GraphicsEngine;
		LogInfo() << "Reading Gothic-Config: " << var;

		if (!engine)
		{
			LogWarn() << "ENGINE wasn't initialized yet! WTF!";
		}

		if (_stricmp(var, "zVidResFullscreenX") == 0)
		{
			if (engine)
			{
				LogInfo() << "Forcing zVidResFullscreenX: " << engine->GetResolution().x;
				return engine->GetResolution().x;
			}
		} else if (_stricmp(var, "zVidResFullscreenY") == 0)
		{
			if (engine)
			{
				LogInfo() << "Forcing zVidResFullscreenY: " << engine->GetResolution().y;
				return engine->GetResolution().y;
			}
		} else if (_stricmp(var, "zVidResFullscreenBPP") == 0)
		{
			return 32;
		} else if (_stricmp(var, "zTexMaxSize") == 0)
		{
			return 16384;
		} else if (_stricmp(var, "zTexCacheOutTimeMSec") == 0) // Following values are from Marcellos L'Hiver config
		{
			return 9120000; 
		} else if (_stricmp(var, "zTexCacheSizeMaxBytes") == 0)
		{
			return 1000000000; 
		} else if (_stricmp(var, "zSndCacheOutTimeMSec") == 0) 
		{
			return 10000; 
		} else if (_stricmp(var, "zSndCacheSizeMaxBytes") == 0)
		{
			return 40000000; 
		}

		return HookedFunctions::OriginalFunctions.original_zCOptionReadInt(thisptr, section, var, def);
	}

	/** Reads config stuff */
	static unsigned long __fastcall hooked_zOptionReadDWORD(void * thisptr, void * unknwn, zSTRING const& section, char const* var, unsigned long def)
	{
		BaseGraphicsEngine* engine = Engine::GraphicsEngine;
		LogInfo() << "Reading Gothic-Config: " << var;

		if (_stricmp(var, "zTexCacheOutTimeMSec") == 0) // Following values are from Marcellos L'Hiver config
		{
			return 9120000; 
		} else if (_stricmp(var, "zTexCacheSizeMaxBytes") == 0)
		{
			return 1000000000; 
		} else if (_stricmp(var, "zSndCacheOutTimeMSec") == 0) 
		{
			return 10000; 
		} else if (_stricmp(var, "zSndCacheSizeMaxBytes") == 0)
		{
			return 40000000; 
		}

		return HookedFunctions::OriginalFunctions.original_zCOptionReadDWORD(thisptr, section, var, def);
	}

	

	static long __fastcall hooked_zOptionReadInt(void * thisptr, void * unknwn,zSTRING const& section, char const* var, int def)
	{		
		int i = Do_hooked_zOptionReadInt(thisptr,section, var, def);

		// Save the variable
		Engine::GAPI->SetIntParamFromConfig("var", i);

		return i;
	}

	static zCOption* GetOptions(){return *(zCOption**)GothicMemoryLocations::GlobalObjects::zCOption;}
};



