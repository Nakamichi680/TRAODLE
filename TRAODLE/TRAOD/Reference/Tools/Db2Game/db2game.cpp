/*  db2game.c
 *
 *  Database to game converter tool
 *
 *  Copyright (c) Core Design Ltd 2002
 *  Version: $Id: db2game.h, v 1.00 2002-23-02 12:08:43+00 $

	23/07/02 - Actually load full database (75 Kb)
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
	  format:

	  Number of entries
	  < array of index - short >
	  < array of text length - int > will be converted to POINTER on text after loading 
	  < text data >

	  Exports all the database (no optimisation, but only < 75 Kb for all the game)
 */

#define SCRIPTS_SERVER_DIR "Z:\\NEXTGEN"
#define SCRIPTS_INPUT_PATH "Z:\\NEXTGEN\\GDBEXPORT"
#define SCRIPTS_OUTPUT_PATH "C:\\Projects\\Tomb Raider\\SCRIPTS"
#define SCRIPTS_OUTPUT_DIR "C:\\Projects\\Tomb Raider\\GAME"

void wait_key()
{
	
}



FILE *open_for_output(const char *sz, const char *mode, bool warn = false)
{
	FILE *f = fopen(sz, mode);
	if (!f)
	{
		if (warn)
			printf("Warning: %s is READ ONLY.\n", sz);
	}
	return f;


}

void BuildGlobalTextDatabase(const char *langtxt, const char *langpack)
{
	FILE *fd = open_for_output(langpack, "wb");
	if (!fd)
		return;

	int misa = 0;
	for (int phase = 0; phase < 4; phase ++)
	{
		FILE *fs = fopen(langtxt, "rt");	
		if (fs)
		{
			unsigned int count = 0;
			while (!feof(fs))
			{
				char tmp[4096];
				
				if (!fgets(tmp, 4096, fs))
					break;
				
				char seps[] = "\"";
				char *token;
				token = strtok( tmp, seps );
				if (!token)
					break;
				unsigned short index = atoi(token);				
				token = strtok( NULL, seps );					
				unsigned int length = token ? strlen(token) : 0;
				if (!length)
					printf("Warning: Null string item in %s key %d - %s\n", langtxt, index, tmp);
				count++;			
				if (phase == 1)
				{						
					fwrite(&index, 1, sizeof(short), fd);					
				}
				else
				if (phase == 2)
				{
					fwrite(&length, 1, sizeof(int), fd);
				}
				else
				if (phase == 3)
				{
					if (token)						
						fwrite(token, 1, length+1, fd);
				
				}

			}
			if (phase == 0)
			{
				if (count&1)
				{
					count++;
					misa = 1;
				}
				
				fwrite(&count, 1, sizeof(int), fd);	
			}

			if (phase == 1)
			{			
				short index;
				if (misa)
					fwrite(&index, 1, sizeof(short), fd);					
			}
			else
			if (phase == 2)
			{
				int length;
				if (misa)
					fwrite(&length, 1, sizeof(int), fd);
			}

			fclose(fs);
		}		
	}

	fclose(fd);
}

// Convert Dialogues To TextID.txt information into script headers file
int file_length(FILE *f)
{
	int l;
	fseek(f,0,SEEK_END);
	l = ftell(f);
	fseek(f,0,SEEK_SET);
	return l;
}
int file_exists(const char *s)
{
	FILE *f = fopen(s, "rb");
	if (!f)
		return 0;
	fclose(f);
	return 1;
}

char	g_szCache[256][260];
int		g_numCache;

int auto_detect_cache(const char *name)
{
	int i;
	for (i=0;i<g_numCache;i++)
	{
		if (!strcmp(name, g_szCache[i]))
			return 0;
	}	
	if (g_numCache>=256)
		printf("Error : Internal overflow\n");

	strcpy(g_szCache[g_numCache], name);
	g_numCache++;
	return 1;
}

void append_file(const char *dstName, const char *srcName, int bFirst)
{		
	if (file_exists(srcName))
	{
		bFirst = auto_detect_cache(dstName);
		FILE *fd = open_for_output(dstName, bFirst ? "wb" : "a+b");
		if (!fd)
			return;
		FILE *fs = fopen(srcName,"rb");
		if (!fs)
			return;
		int l = file_length(fs);
		void *tmp = new char[l];
		fseek(fd,0,SEEK_END);
		fread(tmp, 1, l, fs);
		fwrite(tmp, 1, l, fd);
		fclose(fd);
		fclose(fs);
		delete[] tmp;
	}
}

void BuildScriptHeadersFiles()
{
	FILE *fs = fopen(SCRIPTS_INPUT_PATH"\\Dialogues To TextID.txt", "rt");
	char previous_dialog_name[32];
	FILE *fd = 0;
	if (fs)
	{
		while (!feof(fs))
		{
			char tmp[256];

			char dialog_name[32];
			char sentence_name[32];
			char character_name[32];

			int sentence_key, text_key, dialogue_key, character_key;
			int type;

			memset(character_name,0, 32);
			memset(dialog_name, 0, 32);
			memset(sentence_name, 0, 32);
			
			if (!fgets(tmp, 256, fs))
				break;

			
			char seps[] = ",";
			char *token;

			// Dialogue Name
			token = strtok( tmp, seps );
			strncpy(dialog_name, token + 1 + 3, strlen(token + 3) - 2);

			// Dialogue Key
			token = strtok( NULL, seps );
			dialogue_key = atoi(token);

			// Dialogue Type
			token = strtok( NULL, seps );
			type = atoi(token);

			// Sentence IDName
			token = strtok( NULL, seps );
			strncpy(sentence_name, token + 1, strlen(token) - 2);

			// Sentence Key
			token = strtok( NULL, seps );
			sentence_key = atoi(token);

			// Character resource Name
			token = strtok( NULL, seps );
			strncpy(character_name, token + 1, strlen(token) - 2);

			token = strtok( NULL, seps );
			character_key = atoi(token);

			// Text Key
			token = strtok( NULL, seps );
			text_key = atoi(token);

			int bFirst = 0;
			if (strcmp(dialog_name, previous_dialog_name))
			{
				char dst_filename[128];
				if (fd)
					fclose(fd);

				const char *subfolder[] = 
				{
					NULL,
					"cutscene\\cut-",
					"level\\gp-",
					"fmv\\fmv-",
					"cutscene\\ing-",
					"dialogue\\dlg-",
					"level\\vp-",
					"enemy\\",
					NULL
				};
				
				sprintf(dst_filename, SCRIPTS_OUTPUT_PATH"\\%s%s.h", subfolder[type], dialog_name);
				fd = open_for_output(dst_filename, "wt", false);
				if (fd)
				{
					fprintf(fd, "// AUTO-GENERATED by db2game - DO NOT MODIFY -\n");
					fprintf(fd, "#define DIALOGUE_ID %d\n", dialogue_key);
					strcpy(previous_dialog_name, dialog_name);
					bFirst = 1;
				}
			}
			if (fd)
			{
				fprintf(fd, "#define %s 0x%x // %s\n", sentence_name, text_key|(sentence_key<<16), character_name); // TODO: could be fine to compress string ?
			}

//			Now Done In Batch Exporter RF 27/02/03			
//			char sms[_MAX_PATH];
//			char tms[_MAX_PATH];
//			sprintf(sms,SCRIPTS_SERVER_DIR"\\gamechars\\export.dlg\\%s.%d", character_name, dialogue_key);
//			sprintf(tms,SCRIPTS_SERVER_DIR"\\gamechars\\export.dlg\\%d.tms", sentence_key);
//			append_file(sms, tms, bFirst);
		}
		fclose(fs);
		if (fd)
			fclose(fd);
	}
}


struct InventoryDbEntry
{	
	int		nKeyInventory;		// Key Inventory
	int		nKeyObjectClass;	// Key object class
	int		nKeyText;			// Key text
	float	x, y, z, w;
	int		nFlags;
};

void BuildInventoryDb()
{
	FILE *fs = fopen(SCRIPTS_INPUT_PATH"\\Inventory List.txt", "rt");
	FILE *fo = open_for_output(SCRIPTS_OUTPUT_DIR"\\data\\INVENT.DB", "wb");
	int nInventoryItem = 0;

	if (fs && fo)
	{		
		
		fwrite(&nInventoryItem, 1, sizeof(int), fo);
		while (!feof(fs))
		{
			char tmp[256];
			if (!fgets(tmp, 256, fs))
				break;			

			struct InventoryDbEntry entry;
			
			char seps[] = ",";
			char *token;

			token = strtok( tmp, seps );
			entry.nKeyInventory = atoi(token);

			token = strtok( NULL, seps );
			entry.nKeyObjectClass = atoi(token);

			token = strtok( NULL, seps );
			entry.nKeyText = atoi(token);

			token = strtok( NULL, seps );
			entry.nKeyText |= atoi(token)<<16;

			// 4 float
			token = strtok( NULL, seps );		
			entry.x = token ? atof(token) : 0.f;

			token = strtok( NULL, seps );
			entry.y = token ? atof(token) : 0.f;

			token = strtok( NULL, seps );
			entry.z = token ? atof(token) : 0.f;

			token = strtok( NULL, seps );
			entry.w = token ? atof(token) : 1.f;

			token = strtok( NULL, seps);
			entry.nFlags = token ? atoi(token) : 1;

			fwrite(&entry, 1, sizeof(entry), fo);
			nInventoryItem++;
		}
		fseek(fo, 0, SEEK_SET);
		fwrite(&nInventoryItem, 1, sizeof(int), fo);

		fclose(fs);
	}
	if (!fo)
	{
		printf("\bCan't export invent.db\n");
	}
}

void BuildInventoryGameDb()
{
	FILE *fs = fopen(SCRIPTS_INPUT_PATH"\\User Interface Texts.txt", "rt");
	if (fs)
	{
		FILE *fd = open_for_output(SCRIPTS_OUTPUT_PATH"\\uitexts.h","wt");
		if (fd)
		{
			fprintf(fd, "// AUTO-GENERATED by db2game - DO NOT MODIFY -\n");					
			fprintf(fd,"#if !defined _UI_TEXT_ID_HH\n");
			fprintf(fd,"#define _UI_TEXT_ID_HH\n\n");

			while (!feof(fs))
			{
				char tmp[256];
				char name[32];
				memset(name, 0, 32);			

				if (!fgets(tmp, 256, fs))
					break;

				char seps[] = ",";
				char *token;
				token = strtok( tmp, seps );
				int ID = atoi(token);		// ID
				token = strtok( NULL, seps );
				int Cat = atoi(token);		// Cat
				token = strtok( NULL, seps );				
				int Key = atoi(token);		// Key
				token = strtok( NULL, seps );
				if (token[strlen(token)-2] == '\"')
					token[strlen(token)-2]=0;
				
				fprintf(fd, "#define UITEXT_%d %d // %s\n", ID, Key, token+1);			

			}
			fprintf(fd,"#endif // _UI_TEXT_ID_HH\n\n");
			fclose(fd);
		}
	}

	fclose(fs);

	// todo: inventory.db binary file
	//		 resourcelo, resourcehi, index_text, flags
	

}

void BuildNotebookGameDb()
{
	FILE *fs = fopen(SCRIPTS_INPUT_PATH"\\Notebook List.txt", "rt");
	if (fs)
	{
		FILE *fd = open_for_output(SCRIPTS_OUTPUT_PATH"\\notebook.h","wt");
		if (fd)
		{
			fprintf(fd, "// AUTO-GENERATED by db2game - DO NOT MODIFY -\n");					
			fprintf(fd,"#if !defined _NOTEBOOK_ID_HH\n");
			fprintf(fd,"#define _NOTEBOOK_ID_HH\n\n");

			while (!feof(fs))
			{
				char tmp[256];
				char name[32];
				memset(name, 0, 32);			

				if (!fgets(tmp, 256, fs))
					break;

				char seps[] = ",";
				char *token;
				token = strtok( tmp, seps );
				int ID = atoi(token);		// ID
				token = strtok( NULL, seps );
				int Key = atoi(token);		// Key
				token = strtok( NULL, seps );
				if (token[strlen(token)-2] == '\"')
					token[strlen(token)-2]=0;
				
				fprintf(fd, "#define NOTEBOOK_%d %d // %s\n", ID, Key, token+1);			

			}
			fprintf(fd,"#endif // _NOTEBOOK_ID_HH\n\n");
			fclose(fd);
		}
	}

	fclose(fs);

	// todo: Notebook.db binary file
	//		 resourcelo, resourcehi, index_text, flags
	

}

FILE * fopen_if_notexists(const char *filename)
{
	FILE *f = fopen(filename,"rt");
	if (f)
	{
		fclose(f);
		return 0;
	}
	printf("create %s\n", filename);
	return fopen(filename, "wt");
}

struct ActorDb
{
	int						nKey;					// database key
	int						iLifeMax;				// max life points
	int						iClassID;				// player class ID
	int						iCloneOf;				// clone index
	char					resource[16];			// Skin resource
	int						iFlags;
};

#define RAGDOLL_DEATH	1
#define RAGDOLL_HIT		2


void BuildScriptTemplate()
{
	FILE *fs = fopen(SCRIPTS_INPUT_PATH"\\Character List.txt", "rt");
	FILE *fo = open_for_output(SCRIPTS_OUTPUT_DIR"\\data\\ACTOR.DB", "wb");
	int numActors = 0;	
	if (fs && fo)
	{
		fwrite(&numActors, 1, sizeof(int), fo);
		while(!feof(fs))
		{
			struct ActorDb	db;

			char resourceName[32];
			char realName[64];
			char tmp[256];
			if (!fgets(tmp, 256, fs))
				break;
			
			memset(&db,0,sizeof(db));
			memset(realName, 0, 32);
			memset(resourceName, 0, 32);
			
			char seps[] = ",";
			char *token;
			token = strtok( tmp, seps );
			int ID = atoi(token);
			token = strtok( NULL, seps );
			strncpy(resourceName, token + 1, strlen(token) - 2);
			token = strtok( NULL, seps );
			strncpy(realName, token + 1, strlen(token) - 2);
			token = strtok( NULL, seps );
			int classID = atoi(token);

			token = strtok( NULL, seps );
			int iLifeMax = atoi(token);

			token = strtok( NULL, seps );
			db.iCloneOf = atoi(token);

			token = strtok(NULL, seps);
			int iRagDollD = atoi(token);

			token = strtok(NULL, seps);
			int iRagDollH = atoi(token);

			strcpy(db.resource, resourceName);

			const char *className[] = 
			{
				NULL,
				"player",
				"neutral",
				"enemy",
				"obj"
			};
			db.nKey = ID;
			db.iClassID = classID;
			db.iLifeMax = iLifeMax;
			db.iFlags = iRagDollD | (iRagDollH <<1);
			fwrite(&db, 1, sizeof(db), fo);
			numActors++;


			char filename[256];
			sprintf(filename, SCRIPTS_OUTPUT_PATH"\\%s\\%s.inc", className[classID], resourceName);

			FILE *fd = fopen_if_notexists(filename);
			if (fd)
			{
				fprintf(fd, " // %s (%s) Tombraider NG\n", resourceName, realName);
				fprintf(fd, "#include <%s/%s.hsh>\n", className[classID], resourceName);
				fprintf(fd, "#define ANIM_RUN			-1\n");
				fprintf(fd, "#define ANIM_STAND			-1\n");
				fprintf(fd, "#define ANIM_WALK			ANIM_RUN\n");
				fprintf(fd, "#define ANIM_STRAFFLEFT	-1\n");
				fprintf(fd, "#define ANIM_STRAFFRIGHT	-1\n");
				fprintf(fd, "#define ANIM_BACK			ANIM_RUN\n");
				fprintf(fd, "#define ANIM_TURN_180		-1\n");
				fprintf(fd, "#define ANIM_TURN_90R		-1\n");
				fprintf(fd, "#define ANIM_TURN_90L		-1\n");
				fprintf(fd, "#define ANIM_SHUFFLE		-1\n");
				fprintf(fd, "#define ANIM_HIT			ANIM_RUN\n");
			}
			sprintf(filename, SCRIPTS_OUTPUT_PATH"\\%s\\%s.sma", className[classID], resourceName);

			fd = fopen_if_notexists(filename);
			if (fd)
			{
				fprintf(fd, " // %s (%s) Tombraider 6 \n", resourceName, realName);
				fprintf(fd, "#include <common>\n");
				fprintf(fd, "#include <%s/%s.inc>\n", className[classID], resourceName);
				fprintf(fd, "#include <common.sma>\n");
				fprintf(fd, "\n");
				fprintf(fd, "// Active state ////////////////////////////////////\n");
				fprintf(fd, "\n");
				fprintf(fd, "OnThinkActive(objref)\n");
				fprintf(fd, "{\n");
				fprintf(fd, "	// TODO: Add extra initialization here\n");
				fprintf(fd, "}\n");
				fprintf(fd, "\n");
				fprintf(fd, "// Idle state /////////////////////////////////////\n");
				fprintf(fd, "\n");
				fprintf(fd, "OnThinkIdle(objref)\n");
				fprintf(fd, "{\n");
				fprintf(fd, "	// TODO: Add extra initialization here\n");
				fprintf(fd, "}\n");
				fprintf(fd, "\n");
				fprintf(fd, "// Dead state /////////////////////////////////////\n");
				fprintf(fd, "\n");
				fprintf(fd, "OnThinkDead(objref)\n");
				fprintf(fd, "{\n");
				fprintf(fd, "	// TODO: Add extra initialization here\n");
				fprintf(fd, "}\n");
				fprintf(fd, "\n");
				fprintf(fd, "// Take damage ////////////////////////////////////\n");
				fprintf(fd, "\n");
				fprintf(fd, "public OnDamage(objref, objref2)\n");
				fprintf(fd, "{\n");
				fprintf(fd, "	// TODO: Add extra initialization here\n");
				fprintf(fd, "	return 0;\n");
				fprintf(fd, "}\n");
				fprintf(fd, "\n");
				fprintf(fd, "// Update /////////////////////////////////\n");
				fprintf(fd, "\n");
				fprintf(fd, "public OnUpdate(objref)\n");
				fprintf(fd, "{\n");
				fprintf(fd, "	new state = GetSuperState(objref);\n");
				fprintf(fd, "	if (objref_ACTIVE == state)\n");
				fprintf(fd, "	{\n");
				fprintf(fd, "		OnThinkActive(objref);\n");
				fprintf(fd, "	}\n");
				fprintf(fd, "	else\n");
				fprintf(fd, "	if (objref_DEAD == state)\n");
				fprintf(fd, "	{\n");
				fprintf(fd, "		OnThinkDead(objref);\n");
				fprintf(fd, "	}\n");
				fprintf(fd, "	else\n");
				fprintf(fd, "	{\n");
				fprintf(fd, "		OnThinkIdle(objref);\n");
				fprintf(fd, "	}\n");
				fprintf(fd, "\n");
				fprintf(fd, "	return 0;\n");
				fprintf(fd, "}\n");
				fprintf(fd, "\n");
				fprintf(fd, "// Init resources /////////////////////////////////\n");
				fprintf(fd, "\n");
				fprintf(fd, "public OnInit(objref)\n");
				fprintf(fd, "{\n");
				fprintf(fd, "	new actor = GetActor(objref);\n");
				fprintf(fd, "	SetSuperState(actor, ACTOR_ACTIVE); \n");
				fprintf(fd, "}\n");
				fprintf(fd, "\n");				
				fclose(fd);
			}
		}
		
		fseek(fo, 0, SEEK_SET);
		fwrite(&numActors, 1, sizeof(int), fo);
		fclose(fo);
		fclose(fs);
	}
	if (!fo)
	{
		printf("\bCan't export invent.db\n");
	}
}

void strcpy_noslash(char *d, const char *s)
{
	while (*s)
	{
		if (*s=='\\') 
			*d = '/';
		else
			*d = *s;
		s++;
		d++;
	}
	*d = 0;
}


struct WeaponDbEntry 
{
	int		nKeyWeapon;			// Key Weapon (used by ActorMgr_ShootAt)
	int		nKeyInventory;		// Key Inventory (0 if not applicable)
	int		nMaxAmmo;			// Max Ammo 
	short	nClip;
	short	nOption;
};

struct DamageDbEntry 
{
	int		nKeyCharacter;		// Key character (MAP_CHARNODE::nKeyCharacter)
	int		nKeyWeapon;			// key weapon (from WeaponDbEntry)
	int		nValue;				// Damage value
	int		nPoisonRate;
};


void BuildWeaponDb()
{
	FILE *fs = fopen(SCRIPTS_INPUT_PATH"\\Weapons List.txt", "rt");
	FILE *fo = open_for_output(SCRIPTS_OUTPUT_DIR"\\data\\WEAPON.DB", "wb");	
	int nWeapon = 0;
	int nDamage = 0;

	if (fs && fo)
	{		
		
		fwrite(&nWeapon, 1, sizeof(int), fo);
		fwrite(&nDamage, 1, sizeof(int), fo);

		FILE *fh = open_for_output(SCRIPTS_OUTPUT_PATH"\\weapons.inc","wt");
		
		if (fh)
		{
			fprintf(fh,"// AUTO-GENERATED BY DB2GAME - DO NOT EDIT\n");
			fprintf(fh,"#if !defined _WEAPON_DEF_HH\n");
			fprintf(fh,"#define _WEAPON_DEF_HH\n\n");
			fprintf(fh,"enum WEAPON_TYPE {\n");
		}		
		
		while (!feof(fs))
		{
			char tmp[256];
			if (!fgets(tmp, 256, fs))
				break;			

			struct WeaponDbEntry entry;

			char	tag[128];
			memset(tag,0,128);
			
			char seps[] = ",";
			char *token;


			token = strtok( tmp, seps );
			entry.nKeyWeapon = atoi(token);

			token = strtok( NULL, seps );
			entry.nMaxAmmo = atoi(token);

			token = strtok( NULL, seps );
			entry.nClip = atoi(token);

			token = strtok( NULL, seps );	
			entry.nOption = atoi(token);

			token = strtok( NULL, seps );
			entry.nKeyInventory = atoi(token);

			token = strtok( NULL, seps );
			strncpy(tag, token+1, strlen(token)-3);

			strupr(tag);
			int i = 0;

			while(tag[i])
			{
				if (tag[i] == ' ' || tag[i] == '\'')
					tag[i] ='_';
				i++;
			}

			if (fh)
			{
				fprintf(fh, "\tWEAPON_%s = %d,\n", tag, entry.nKeyWeapon);
			}

			fwrite(&entry, 1, sizeof(entry), fo);
			nWeapon++;
		}
		if (fh)
		{
			fprintf(fh, "\tWEAPON_FORCE_BYTE = 0xff\n");
			fprintf(fh, "};\n");
			fprintf(fh,"\n#endif // _WEAPON_DEF_HH\n");
			fclose(fh);		
		}
	
		fclose(fs);		

		fs = fopen(SCRIPTS_INPUT_PATH"\\Weapon Damage.txt", "rt");
		if (fs)
		{
			while (!feof(fs))
			{
				char tmp[256];
				if (!fgets(tmp, 256, fs))
					break;			

				struct DamageDbEntry entry;

			
				char seps[] = ",";
				char *token;

				token = strtok( tmp, seps );
				entry.nKeyCharacter = atoi(token);

				token = strtok( NULL, seps );
				entry.nKeyWeapon = atoi(token);

				token = strtok( NULL, seps );
				entry.nValue = atoi(token);

				token = strtok( NULL, seps );
				entry.nPoisonRate = atoi(token);

				fwrite(&entry, 1, sizeof(entry), fo);
				nDamage++;		
			}
			fclose(fs);		
		}			

		fseek(fo, 0, SEEK_SET);
		fwrite(&nWeapon, 1, sizeof(int), fo);
		fwrite(&nDamage, 1, sizeof(int), fo);

		fclose(fo);
	}			
	if (!fo)
	{
		printf("Unable to export WEAPON.DB\n");
	}
	return;
}

void BuildScriptConversation()
{
	FILE *fs = fopen(SCRIPTS_INPUT_PATH"\\Dialogues To TextID.txt", "rt");
	char previous_dialog_name[32];
	FILE *fd = 0;
	if (fs)
	{
		while (!feof(fs))
		{
			char tmp[256];

			char dialog_name[32];
			char sentence_name[32];
			char character_name[32];

			int sentence_key, text_key, dialogue_key, character_key;
			int type;
			

			memset(character_name,0, 32);
			memset(dialog_name, 0, 32);
			memset(sentence_name, 0, 32);

						
			if (!fgets(tmp, 256, fs))
				break;			
			
			char seps[] = ",";
			char *token;

			// Dialogue Name
			token = strtok( tmp, seps );
			strncpy(dialog_name, token + 1, strlen(token) - 2);

			// Dialogue Key
			token = strtok( NULL, seps );
			dialogue_key = atoi(token);

			// Dialogue Type
			token = strtok( NULL, seps );
			type = atoi(token);

			// Sentence ID
			token = strtok( NULL, seps );
			strncpy(sentence_name, token + 1, strlen(token) - 2);

			// Sentence Key
			token = strtok( NULL, seps );
			sentence_key = atoi(token);

			// Character resource Name
			token = strtok( NULL, seps );
			strncpy(character_name, token + 1, strlen(token) - 2);

			token = strtok( NULL, seps );
			character_key = atoi(token);

			//  Key
			token = strtok( NULL, seps );
			text_key = atoi(token);

			
			if (strcmp(dialog_name, previous_dialog_name))
			{				
				if (fd)
				{
					fprintf(fd, "}\n");
					fclose(fd);
				}

				if (type == 1) // cutscene
				{
					const char *subfolder[] = 
					{
						NULL,
						"cutscene\\cut",
						"level\\gp",
						"fmv\\fmv",
						"cutscene\\ing",
						"dialogue\\dlg",
						"level\\vp",
						NULL
					};
					char dst_filename[128];
					
					sprintf(dst_filename, SCRIPTS_OUTPUT_PATH"\\%s-%s.sma", subfolder[type], dialog_name);
				
					fd = fopen_if_notexists(dst_filename);
					if (fd)
					{
						fprintf(fd, "// AUTO-GENERATED by db2game (%s)\n", dialog_name);
						fprintf(fd, "#include <character.h>\n\n", subfolder[type], dialog_name);
						char tmp[32];
						strcpy_noslash(tmp, subfolder[type]);
						fprintf(fd, "#include <%s-%s.h>\n\n", tmp, dialog_name);
						fprintf(fd, "OnDialogue()\n");
						fprintf(fd, "{\n");
						
						strcpy(previous_dialog_name, dialog_name);
					}
				}
			}
			if (fd)
			{
				fprintf(fd, "\tSay(ID_%s, %s);\n", character_name, sentence_name); // TODO: could be fine to compress string ?

			}
		}
		fclose(fs);
		if (fd)
			fclose(fd);
	}

	
}

main()
{	
	BuildScriptHeadersFiles();
	BuildGlobalTextDatabase(SCRIPTS_INPUT_PATH"\\TextList.txt", SCRIPTS_OUTPUT_DIR"\\data\\locale\\text_uk.db");
	BuildGlobalTextDatabase(SCRIPTS_INPUT_PATH"\\TextList_FR.txt", SCRIPTS_OUTPUT_DIR"\\data\\locale\\text_fr.db");
	BuildGlobalTextDatabase(SCRIPTS_INPUT_PATH"\\TextList_GR.txt", SCRIPTS_OUTPUT_DIR"\\data\\locale\\text_gr.db");
	BuildGlobalTextDatabase(SCRIPTS_INPUT_PATH"\\TextList_IT.txt", SCRIPTS_OUTPUT_DIR"\\data\\locale\\text_it.db");
	BuildGlobalTextDatabase(SCRIPTS_INPUT_PATH"\\TextList_SP.txt", SCRIPTS_OUTPUT_DIR"\\data\\locale\\text_sp.db");
	BuildGlobalTextDatabase(SCRIPTS_INPUT_PATH"\\TextList_POR.txt", SCRIPTS_OUTPUT_DIR"\\data\\locale\\text_por.db");
	
	BuildInventoryDb();
	BuildInventoryGameDb();
	BuildNotebookGameDb();
	BuildScriptTemplate();
	BuildScriptConversation();
	BuildWeaponDb();

	return 0;
}

