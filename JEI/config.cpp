// JEI - Just Enough Items for DayZ
// Author: vyrriox

class CfgPatches
{
	class JEI
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class JEI
	{
		dir = "JEI";
		name = "JEI - Just Enough Items";
		author = "vyrriox";
		type = "mod";
		inputs = "JEI/data/inputs.xml";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"JEI/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"JEI/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"JEI/scripts/5_Mission"};
			};
		};
	};
};
