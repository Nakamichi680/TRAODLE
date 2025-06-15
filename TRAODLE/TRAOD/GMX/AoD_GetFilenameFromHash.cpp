#include "stdafx.h"
#include "Classes.h"
#include "hash_Functions.h"


string AoD_GetFilenameFromHash (int hash)
{
	unsigned int num_IDs = 732;
	string ID[732], cal, chr, css, txt, cam, cbh, cln, evx, pos, rmx, scx, tms, tmt, xxx;

	// Questo database è utilizzato per analizzare gli hash di tutti i files la cui radice del nome non è presente nel nome del livello
	ID[0]	=	"ANTON";				ID[1]	=	"AQUATIC";				ID[2]	=	"LVARMED";				ID[3]	=	"ASSASSIN";				ID[4]	=	"BAT";
	ID[5]	=	"X_JANITO";				ID[6]	=	"BIO_NEF";				ID[7]	=	"BIOSUIT";				ID[8]	=	"BOAZ_BIG";				ID[9]	=	"BOAZ_FLY";
	ID[10]	=	"BOAZ_POD";				ID[11]	=	"SPLINTER";				ID[12]	=	"BOUCHARD";				ID[13]	=	"BOXER1";				ID[14]	=	"OBSCURA";
	ID[15]	=	"CABAL_A";				ID[16]	=	"CABAL_B";				ID[17]	=	"BOXER3";				ID[18]	=	"CHAUFFER";				ID[19]	=	"CHINAMAN";
	ID[20]	=	"CS_GUARD";				ID[21]	=	"CS_COP_A";				ID[22]	=	"CS_COP_B";				ID[23]	=	"CS_COP_C";				ID[24]	=	"CS_COP_D";
	ID[25]	=	"CS_STRAHOVA";			ID[26]	=	"RENNES";				ID[27]	=	"DOORMAN";				ID[28]	=	"DOORVINE";				ID[29]	=	"DRUGDEAL";
	ID[30]	=	"ECK";					ID[31]	=	"ECK_FIRE";				ID[32]	=	"EEL";					ID[33]	=	"F_KNIGHT";				ID[34]	=	"FISH";
	ID[35]	=	"FRANCINE";				ID[36]	=	"GAS_BAG";				ID[37]	=	"MULLER";				ID[38]	=	"DOG";					ID[39]	=	"GUNDERSO";
	ID[40]	=	"JANICE";				ID[41]	=	"KAREL";				ID[42]	=	"KARELNEF";				ID[43]	=	"KARELNC";				ID[44]	=	"BOAZ";
	ID[45]	=	"KURTIS";				ID[46]	=	"LAB_1";				ID[47]	=	"LAB_2";				ID[48]	=	"LARA";					ID[49]	=	"LARAC2";
	ID[50]	=	"LARAC1";				ID[51]	=	"LARAD";				ID[52]	=	"LARAS";				ID[53]	=	"LICKER";				ID[54]	=	"BBC";
	ID[55]	=	"GUARD";				ID[56]	=	"GUARDB";				ID[57]	=	"GUARDC";				ID[58]	=	"LUDDICK";				ID[59]	=	"ROUZIC";
	ID[60]	=	"MELTED";				ID[61]	=	"MENTAL_N";				ID[62]	=	"MENTAL_C";				ID[63]	=	"MENTAL_P";				ID[64]	=	"MENTALPB";
	ID[65]	=	"MENTALPC";				ID[66]	=	"MENTALPD";				ID[67]	=	"CARVIER";				ID[68]	=	"PIERRE";				ID[69]	=	"PODCREEP";
	ID[70]	=	"PODCREPB";				ID[71]	=	"PODCREPC";				ID[72]	=	"COP_A";				ID[73]	=	"COP_B";				ID[74]	=	"COP_C";
	ID[75]	=	"COP_D";				ID[76]	=	"COP_GAS";				ID[77]	=	"COP_DGAS";				ID[78]	=	"P_KINGHT";				ID[79]	=	"PROTO";
	ID[80]	=	"RAT";					ID[81]	=	"RAVEN";				ID[82]	=	"ROPE";					ID[83]	=	"SCUBA";				ID[84]	=	"SCUBA_ST";
	ID[85]	=	"AGENT_A";				ID[86]	=	"AGENT_B";				ID[87]	=	"SHAMAN";				ID[88]	=	"SLEEPER";				ID[89]	=	"SLROPE";
	ID[90]	=	"SLINKY";				ID[91]	=	"SMALLEEL";				ID[92]	=	"STRAHOVA";				ID[93]	=	"STRNMSKA";				ID[94]	=	"STRNMSKB";
	ID[95]	=	"STREET";				ID[96]	=	"SWARMRAT";				ID[97]	=	"TRAMP_A";				ID[98]	=	"TRAMP_B";				ID[99]	=	"TRAMPDOG";
	ID[100]	=	"TRIFFID";				ID[101]	=	"KNIGHT";				ID[102]	=	"VEG_EGG";				ID[103]	=	"VCNEIGH";				ID[104]	=	"VONCROY";
	ID[105]	=	"BOXER2";				ID[106]	=	"LARA_IG_2_52";			ID[107]	=	"DOORMAN_IG_2_52";		ID[108]	=	"OBJECTS_1_IG_2_52";	ID[109]	=	"OBJECTS_2_IG_2_52";
	ID[110]	=	"OBJECTS_3_IG_2_52";	ID[111]	=	"TRAPDOOR";				ID[112]	=	"IG_2_52";				ID[113]	=	"TREE_SHADOW";			ID[114]	=	"S_TREE";
	ID[115]	=	"DOOR_SLIDE_LEFT";		ID[116]	=	"DOOR_RIGHT";			ID[117]	=	"DOOR_LEFT";			ID[118]	=	"STAIR_BLADES";			ID[119]	=	"LIONDOOR";
	ID[120]	=	"LEVER";				ID[121]	=	"FLAME_WALL";			ID[122]	=	"ALC_P_CULLIS";			ID[123]	=	"HALL_P_CULLIS";		ID[124]	=	"AIRHEAD";
	ID[125]	=	"AIR_ROOM_MIST";		ID[126]	=	"AIR_POLE";				ID[127]	=	"ELEMENT_CRYSTALS";		ID[128]	=	"LARA_IG_15_15";		ID[129]	=	"LARA_IG_15_16";
	ID[130]	=	"LARA_IG_15_13";		ID[131]	=	"LARA_IG_15_12";		ID[132]	=	"ECK_IG_15_13";			ID[133]	=	"ECK_IG_15_12";			ID[134]	=	"IG_15_15";
	ID[135]	=	"IG_15_16";				ID[136]	=	"IG_15_13";				ID[137]	=	"IG_15_12";				ID[138]	=	"FORCE_FIELD";			ID[139]	=	"CAGE_TRAPDOOR";
	ID[140]	=	"FL_LADDER";			ID[141]	=	"KAREL_IG_15_16";		ID[142]	=	"ALCHEMY_CAGE";			ID[143]	=	"PIT_DOOR";				ID[144]	=	"LIFT_DIG";
	ID[145]	=	"DIG_SMALL_PC";			ID[146]	=	"LOUVREBARRIER";		ID[147]	=	"XRAYDOORS";			ID[148]	=	"BUTT_TOP_DOOR";		ID[149]	=	"DESKLEVER";
	ID[150]	=	"MONITOR";				ID[151]	=	"DIG_BIG_PC";			ID[152]	=	"XRAYKEYBOARD";			ID[153]	=	"SCAN_MONITOR";			ID[154]	=	"X_RAYDESK";
	ID[155]	=	"WARDROBE";				ID[156]	=	"DOUBLE_DOOR";			ID[157]	=	"LOUVREFAN";			ID[158]	=	"FERGYSMASHGLASS";		ID[159]	=	"SHUTTER_LEFT";
	ID[160]	=	"COP_A_IG_1_16";		ID[161]	=	"LARA_IG_1_16";			ID[162]	=	"IG_1_16";				ID[163]	=	"SHUTTER_RIGHT";		ID[164]	=	"EXPLOSIVE";
	ID[165]	=	"FAN";					ID[166]	=	"BIGFAN";				ID[167]	=	"KURTIS_CS_6_21A";		ID[168]	=	"GUNDERSO_CS_6_21A";	ID[169]	=	"STRAHOVA_CS_6_21A";
	ID[170]	=	"LARA_CS_6_21A";		ID[171]	=	"CS_6_21A";				ID[172]	=	"DOORMAN_CS_6_21A";		ID[173]	=	"CS_1_22";				ID[174]	=	"CS_1_22D";
	ID[175]	=	"CS_2_51";				ID[176]	=	"CS_2_51A";				ID[177]	=	"CS_6_2";				ID[178]	=	"CS_6_16";				ID[179]	=	"CS_6_21B";
	ID[180]	=	"CS_7_19";				ID[181]	=	"CS_9_1";				ID[182]	=	"CS_9_12";				ID[183]	=	"CS_9_15B";				ID[184]	=	"CS_10_8";
	ID[185]	=	"CS_10_14";				ID[186]	=	"CS_12_1";				ID[187]	=	"CS_13_9";				ID[188]	=	"CS_14_4A";				ID[189]	=	"CS_14_6";
	ID[190]	=	"CS_15_10";				ID[191]	=	"CS_15_14";				ID[192]	=	"CS_15_24";				ID[193]	=	"BIN";					ID[194]	=	"LARA_CS_9_1";
	ID[195]	=	"TUNNELEXPLOSION";		ID[196]	=	"KURTIS_CS_10_14";		ID[197]	=	"AIRLOCKDOOR";			ID[198]	=	"AUTOGUN";				ID[199]	=	"LARA_CS_10_14";
	ID[200]	=	"BIOFAN";				ID[201]	=	"LARA_CS_1_22";			ID[202]	=	"LOCKER";				ID[203]	=	"BIO_DOOR1";			ID[204]	=	"WHEEL_SWITCH";
	ID[205]	=	"MACHINE_2";			ID[206]	=	"SALT_HOLDER";			ID[207]	=	"SUMMONWATER";			ID[208]	=	"BOTTLEHBARR";			ID[209]	=	"S_TREE_2";
	ID[210]	=	"S_TREE_3";				ID[211]	=	"SEWERBLAST";			ID[212]	=	"ROOFTOPWINDOWSLEFT";	ID[213]	=	"OBJECT01_CS_6_21A";	ID[214]	=	"OBJECT02_CS_9_1";
	ID[215]	=	"OBJECT03_IG_1_16";		ID[216]	=	"OBJECT01_IG_1_16";		ID[217]	=	"PADLOCK";				ID[218]	=	"OBJECT02_IG_1_16";		ID[219]	=	"DOOR_SLIDE_RIGHT";
	ID[220]	=	"DRAWER";				ID[221]	=	"OBJECT04_CS_1_22";		ID[222]	=	"OBJECT02_CS_1_22";		ID[223]	=	"OBJECT03_CS_1_22";		ID[224]	=	"DOORVINE_IG_7_77";
	ID[225]	=	"OBJECT01_IG_7_77";		ID[226]	=	"OBJECT02_IG_8_88";		ID[227]	=	"OBJECT02_IG_7_77";		ID[228]	=	"IG_7_77";				ID[229]	=	"IG_8_88";
	ID[230]	=	"CHEMDIAL";				ID[231]	=	"DOORVINE_IG_8_88";		ID[232]	=	"GREEN";				ID[233]	=	"LARA_IG_10_4";			ID[234]	=	"POD_GLASS";
	ID[235]	=	"IG_10_4";				ID[236]	=	"ROLLER";				ID[237]	=	"DEATH_CAGE";			ID[238]	=	"IG_5_16";				ID[239]	=	"LARA_IG_5_16";
	ID[240]	=	"HALL_TRAPDOOR";		ID[241]	=	"BIGBLOCK";				ID[242]	=	"ELEMENT_LOCK";			ID[243]	=	"HALL_PAD";				ID[244]	=	"CHAIN";
	ID[245]	=	"LIFT_MACHINE";			ID[246]	=	"MED_CABINET";			ID[247]	=	"RED_FISH";				ID[248]	=	"OBJECT01_IG_1_26";		ID[249]	=	"OBJECT01_IG_1_25";
	ID[250]	=	"OBJECT01_IG_1_24";		ID[251]	=	"OBJECT01_IG_11_40";	ID[252]	=	"IG_11_40";				ID[253]	=	"SWITCH_MATT";			ID[254]	=	"UNIT_LOW";
	ID[255]	=	"OBJECT05_CS_1_22";		ID[256]	=	"COPTER";				ID[257]	=	"OBJECT01_CS_1_22";		ID[258]	=	"ROOFTOPWINDOWSRIGHT";	ID[259]	=	"X_JANITO_DLG";
	ID[260]	=	"DOORMAN_DLG";			ID[261]	=	"LARA_DLG";				ID[262]	=	"MOSAIC_PLATFORMS";		ID[263]	=	"BIGSPIKE";				ID[264]	=	"FURNACE_DOOR";
	ID[265]	=	"HOOK";					ID[266]	=	"OBJECT01_IG_2_6";		ID[267]	=	"LARA_IG_2_6";			ID[268]	=	"LARA_IG_2_1";			ID[269]	=	"DRUGDEAL_DLG";
	ID[270]	=	"KURTIS_IG_2_6";		ID[271]	=	"JANICE_DLG";			ID[272]	=	"TRAMP_A_DLG";			ID[273]	=	"IG_2_1";				ID[274]	=	"IG_2_6";
	ID[275]	=	"CHINAMAN_DLG";			ID[276]	=	"SEWERBLASTTOP";		ID[277]	=	"E_SWITCH";				ID[278]	=	"4_BLADE";				ID[279]	=	"HALL_AUTO_DOOR";
	ID[280]	=	"GEARS";				ID[281]	=	"MULTI_SAW";			ID[282]	=	"KNIGHT_WALL";			ID[283]	=	"TRACK";				ID[284]	=	"LOCK_BLOCKER";
	ID[285]	=	"BUTT_TOP_COLLIDE";		ID[286]	=	"PUZZLE_LOCK";			ID[287]	=	"HEAVY_D_DOORS";		ID[288]	=	"BFLY_SMALL_PLANT";		ID[289]	=	"OBJECT01_IG_10_4";
	ID[290]	=	"SWIPE_SWITCH";			ID[291]	=	"MULLER_DLG";			ID[292]	=	"AIRLOCKDOORGLASS";		ID[293]	=	"OBJECT01_IG_8_88";		ID[294]	=	"OBJECT03_IG_8_88";
	ID[295]	=	"CHEMWATER";			ID[296]	=	"BIOSLIDINGDOORDOOR";	ID[297]	=	"BLACK_ORANGE";			ID[298]	=	"OBJECT05_IG_8_88";		ID[299]	=	"CORRIDORLIFT";
	ID[300]	=	"MULLER_IG_10_4";		ID[301]	=	"PODHATCH";				ID[302]	=	"OBJECT04_IG_8_88";		ID[303]	=	"EMPTYCAGE";			ID[304]	=	"SHARK_SCREEN";
	ID[305]	=	"U_LEVER";				ID[306]	=	"WATERLIFT";			ID[307]	=	"LIFTDOOR";				ID[308]	=	"POWER_MONITOR";		ID[309]	=	"UW_DOOR";
	ID[310]	=	"SCUBADOOR";			ID[311]	=	"FULLCAGE";				ID[312]	=	"PUZZLELIFT";			ID[313]	=	"SHARKDOOR";			ID[314]	=	"LADDERBARRIER";
	ID[315]	=	"FOOD";					ID[316]	=	"FEEDER";				ID[317]	=	"FEEDERPLATE";			ID[318]	=	"FEEDEREMPTY";			ID[319]	=	"FEEDERFULL";
	ID[320]	=	"OBJECT01_IG_12_6";		ID[321]	=	"IG_12_6";				ID[322]	=	"OBJECT03_IG_12_6";		ID[323]	=	"OBJECT02_IG_12_6";		ID[324]	=	"OBJECT04_IG_12_6";
	ID[325]	=	"WATERHORIZLIFT";		ID[326]	=	"LARA_IG_12_6";			ID[327]	=	"OBJECTS02_IG_14_6A";	ID[328]	=	"SLADDERS";				ID[329]	=	"CONSOLE";
	ID[330]	=	"KURTIS_DLG";			ID[331]	=	"VENT_D";				ID[332]	=	"OBJECT01_IG_11_11";	ID[333]	=	"IG_11_11";				ID[334]	=	"STRAHOVA_IG_11_11";
	ID[335]	=	"EXPLODE_FAN";			ID[336]	=	"OBJECT01_IG_11_22";	ID[337]	=	"IG_11_22";				ID[338]	=	"KURTIS_IG_11_11";		ID[339]	=	"OBJECT02_IG_11_22";
	ID[340]	=	"KURTIS_IG_14_6A";		ID[341]	=	"TRAPDOOR_LIFT";		ID[342]	=	"PROTO_IG_9_99";		ID[343]	=	"LAB_1_IG_11_22";		ID[344]	=	"PROTO_IG_11_22";
	ID[345]	=	"OBJECTS_1_IG_9_99";	ID[346]	=	"OBJECT01_IG_14_6A";	ID[347]	=	"IG_14_6A";				ID[348]	=	"PROTO_IG_11_11";		ID[349]	=	"MENTAL_C_DLG";
	ID[350]	=	"MENTAL_P_IG_14_6A";	ID[351]	=	"IG_9_99";				ID[352]	=	"BLASTWALL";			ID[353]	=	"OBJECT09_IG_9_15";		ID[354]	=	"OBJECT01_IG_5_55";
	ID[355]	=	"OBJECT05_IG_5_55";		ID[356]	=	"BIG_SAW";				ID[357]	=	"REEL";					ID[358]	=	"OBJECT02_IG_9_15";		ID[359]	=	"OBJECT012_IG_9_15";
	ID[360]	=	"OBJECT04_IG_5_55";		ID[361]	=	"OBJECT05_IG_9_15";		ID[362]	=	"SLIDE_LOCK";			ID[363]	=	"BOX_LIFT";				ID[364]	=	"SAWGLASS";
	ID[365]	=	"HEAVY_BAR_DOOR";		ID[366]	=	"OBJECT011_IG_9_15";	ID[367]	=	"OBJECT07_IG_5_55";		ID[368]	=	"OBJECT01_CS_9_1";		ID[369]	=	"OBJECT01_CS_10_14";
	ID[370]	=	"GASDOOR";				ID[371]	=	"DOOR_CON_LEFT";		ID[372]	=	"PLANKS";				ID[373]	=	"IG_1_24";				ID[374]	=	"OBJECT02_IG_1_24";
	ID[375]	=	"LARA_IG_1_24";			ID[376]	=	"CARVIER_IG_1_25";		ID[377]	=	"YEL_FISH";				ID[378]	=	"LARA_CS_1_22D";		ID[379]	=	"IG_1_25";
	ID[380]	=	"COPCAR";				ID[381]	=	"WEED";					ID[382]	=	"IG_1_26";				ID[383]	=	"HYD_HOLDER";			ID[384]	=	"OBJECT02_CS_14_6";
	ID[385]	=	"OBJECT03_IG_5_55";		ID[386]	=	"OBJECT07_IG_9_15";		ID[387]	=	"OBJECT010_IG_9_15";	ID[388]	=	"OBJECT08_IG_9_15";		ID[389]	=	"OBJECT01_IG_9_15";
	ID[390]	=	"OBJECT04_IG_9_15";		ID[391]	=	"OBJECT06_IG_9_15";		ID[392]	=	"OBJECT06_IG_5_55";		ID[393]	=	"OBJECT02_IG_5_55";		ID[394]	=	"OBJECT03_IG_9_15";
	ID[395]	=	"PROPTANK";				ID[396]	=	"CRANE";				ID[397]	=	"OBJECT013_IG_9_15";	ID[398]	=	"CARVIER_DLG";			ID[399]	=	"LARA_IG_1_26";
	ID[400]	=	"COP_A_IG_1_26";		ID[401]	=	"CRYPTGATE";			ID[402]	=	"TUNNELSA";				ID[403]	=	"T3_PLATFORM_SLIDE";	ID[404]	=	"CRYPT_COLLAPSE";
	ID[405]	=	"PIERRE_DLG";			ID[406]	=	"T3_END";				ID[407]	=	"CRYPTGATES";			ID[408]	=	"T3_PLATFORM_SLIDE";	ID[409]	=	"BOUCHARD_DLG";
	ID[410]	=	"PUNCHBAG";				ID[411]	=	"CHURCHPLANKS";			ID[412]	=	"ANTON_DLG";			ID[413]	=	"TOMBLID";				ID[414]	=	"STATUE";
	ID[415]	=	"LEDGEA";				ID[416]	=	"FRANCINE_DLG";			ID[417]	=	"BOUCHARD_IG_2_42";		ID[418]	=	"IG_2_42";				ID[419]	=	"LARA_IG_2_42";
	ID[420]	=	"T3_PLATFORM_2";		ID[421]	=	"LARA_IG_1_25";			ID[422]	=	"CARVIER_IG_1_24";		ID[423]	=	"LARA_IG_1_27";			ID[424]	=	"IG_1_27";
	ID[425]	=	"D_W_DOORS";			ID[426]	=	"BRIDGE";				ID[427]	=	"RIG";					ID[428]	=	"DISCO_RING";			ID[429]	=	"BUTTONSWITCH_LARA";
	ID[430]	=	"LIGHT_BOXES";			ID[431]	=	"CLUBLIGHTS";			ID[432]	=	"D_W_LIFT";				ID[433]	=	"GAR_TDOOR";			ID[434]	=	"JEEPLIFT";
	ID[435]	=	"CELLAR_S";				ID[436]	=	"CELLAR_SWITCH";		ID[437]	=	"LARA_IG_2_27";			ID[438]	=	"IG_2_27";				ID[439]	=	"OBJECT01_IG_2_27";
	ID[440]	=	"PENDULUM";				ID[441]	=	"BOXES";				ID[442]	=	"PAWN_BOMB";			ID[443]	=	"RENNEBOOK";			ID[444]	=	"RENNES_DLG";
	ID[445]	=	"OBJECT01_CS_2_51";		ID[446]	=	"ECK_IG_2_44";			ID[447]	=	"IG_2_44";				ID[448]	=	"LARA_IG_2_44";			ID[449]	=	"OBJECT02_CS_2_51";
	ID[450]	=	"OBJECT01_IG_2_44";		ID[451]	=	"OBJECT03_CS_2_51";		ID[452]	=	"LARA_CS_2_51";			ID[453]	=	"KURTIS_CS_2_51";		ID[454]	=	"LOGO_MONITOR";
	ID[455]	=	"KICKABLEGRATE";		ID[456]	=	"XRAYPAINTING";			ID[457]	=	"FLIP_BAT";				ID[458]	=	"B_PLAT_BIG";			ID[459]	=	"B_PLAT_SMALL";
	ID[460]	=	"BUTT_TRAPDOOR";		ID[461]	=	"AQUA_DOOR";			ID[462]	=	"AQUA_FLOOR_GRATE";		ID[463]	=	"UW_BLADE";				ID[464]	=	"UW_SPIKE";
	ID[465]	=	"UW_GRASS";				ID[466]	=	"Q1";					ID[467]	=	"Q2";					ID[468]	=	"Q3";					ID[469]	=	"Q4";
	ID[470]	=	"Q5";					ID[471]	=	"Q6";					ID[472]	=	"Q7";					ID[473]	=	"Q8";					ID[474]	=	"Q9";
	ID[475]	=	"Q10";					ID[476]	=	"Q11";					ID[477]	=	"Q13";					ID[478]	=	"Q14";					ID[479]	=	"Q15";
	ID[480]	=	"Q16";					ID[481]	=	"Q17";					ID[482]	=	"Q18";					ID[483]	=	"Q19";					ID[484]	=	"Q20";
	ID[485]	=	"Q21";					ID[486]	=	"Q22";					ID[487]	=	"Q23";					ID[488]	=	"QSAFE";				ID[489]	=	"PLATFORM";
	ID[490]	=	"OBJECT01_IG_6_66";		ID[491]	=	"IG_6_66";				ID[492]	=	"OBJECT02_IG_6_66";		ID[493]	=	"LAVA_BLOCK_DROP";		ID[494]	=	"FIRE_FLOOR";
	ID[495]	=	"LAV_BLOCK_EXP";		ID[496]	=	"SECRET_BOOKCASE";		ID[497]	=	"CLOCK_WINDOW";			ID[498]	=	"CLOCK_DOOR";			ID[499]	=	"FLASSCABINETS";
	ID[500]	=	"ANTIQUE_STAIRS";		ID[501]	=	"SAFE_PAINTING";		ID[502]	=	"IG_8_1";				ID[503]	=	"IG_8_21";				ID[504]	=	"LARA_IG_8_21";
	ID[505]	=	"LARA_IG_8_11";			ID[506]	=	"IG_8_11";				ID[507]	=	"LARA_IG_8_1";			ID[508]	=	"OBJECT01_IG_8_18";		ID[509]	=	"IG_8_18";
	ID[510]	=	"OBJECT02_IG_8_1";		ID[511]	=	"TRAP_LOCK";			ID[512]	=	"OBJECT01_IG_8_21";		ID[513]	=	"BOUCHARD_IG_8_11";		ID[514]	=	"BOUCHARD_IG_8_21";
	ID[515]	=	"LUDDICK_DLG";			ID[516]	=	"OBJECT01_IG_8_1";		ID[517]	=	"AUTOGUN_C";			ID[518]	=	"LARA_IG_9_15";			ID[519]	=	"IG_9_15";
	ID[520]	=	"IG_5_55";				ID[521]	=	"SWITCH_MATT";			ID[522]	=	"HEAVY_SLIDE_DOOR";		ID[523]	=	"KURTIS_IG_11_40";		ID[524]	=	"KURTIS_IG_14_6B";
	ID[525]	=	"IG_14_6B";				ID[526]	=	"OBJECT01_IG_8_2";		ID[527]	=	"IG_8_2";				ID[528]	=	"PROTO_IG_11_40";		ID[529]	=	"KURTIS_IG_8_2";
	ID[530]	=	"OBJECT02_IG_8_2";		ID[531]	=	"PROTO_IG_8_2";			ID[532]	=	"8_KNIGHTS";			ID[533]	=	"UW_WALL";				ID[534]	=	"UW_KNIGHT";
	ID[535]	=	"U_CHAIN";				ID[536]	=	"8KNIGHTS_FX";			ID[537]	=	"TAPESTRY";				ID[538]	=	"SCOOBY_BOOKCASE";		ID[539]	=	"SCUBA_ST_IG_12_14";
	ID[540]	=	"LARA_IG_12_14";		ID[541]	=	"IG_12_14";				ID[542]	=	"OBJECT01_IG_12_14";	ID[543]	=	"OBJECT03_IG_12_14";	ID[544]	=	"OBJECT02_IG_12_14";
	ID[545]	=	"ARMOUR";				ID[546]	=	"P_KNIGHT";				ID[547]	=	"OXY_HOLDER";			ID[548]	=	"SUMMONELECTRICITY";	ID[549]	=	"SUMMONFIRE";
	ID[550]	=	"KAREL_IG_15_15";		ID[551]	=	"VC_DIARY";				ID[552]	=	"KAREL_CS_2_51A";		ID[553]	=	"BOAZ_CS_2_51A";		ID[554]	=	"MULLER_CS_2_51A";
	ID[555]	=	"GUNDERSO_CS_2_51A";	ID[556]	=	"SLEEPER_CS_2_51A";		ID[557]	=	"ECK_CS_2_51A";			ID[558]	=	"STRAHOVA_CS_6_2";		ID[559]	=	"OBJECT08_CS_6_2";
	ID[560]	=	"OBJECT05_CS_6_2";		ID[561]	=	"OBJECT01_CS_6_2";		ID[562]	=	"ROPE_CS_6_2";			ID[563]	=	"OBJECT06_CS_6_2";		ID[564]	=	"GUARD_CS_6_2";
	ID[565]	=	"OBJECT07_CS_6_2";		ID[566]	=	"OBJECT04_CS_6_2";		ID[567]	=	"OBJECT03_CS_6_2";		ID[568]	=	"GUNDERSO_CS_6_2";		ID[569]	=	"OBJECT02_CS_6_2";
	ID[570]	=	"OBJECT10_CS_6_16";		ID[571]	=	"OBJECT57_CS_6_16";		ID[572]	=	"OBJECT13_CS_6_16";		ID[573]	=	"OBJECT11_CS_6_16";		ID[574]	=	"OBJECT05_CS_6_16";
	ID[575]	=	"OBJECT64_CS_6_16";		ID[576]	=	"OBJECT17_CS_6_16";		ID[577]	=	"OBJECT22_CS_6_16";		ID[578]	=	"OBJECT61_CS_6_16";		ID[579]	=	"OBJECT27_CS_6_16";
	ID[580]	=	"OBJECT03_CS_6_16";		ID[581]	=	"OBJECT04_CS_6_16";		ID[582]	=	"OBJECT40_CS_6_16";		ID[583]	=	"OBJECT44_CS_6_16";		ID[584]	=	"OBJECT49_CS_6_16";
	ID[585]	=	"OBJECT14_CS_6_16";		ID[586]	=	"OBJECT06_CS_6_16";		ID[587]	=	"OBJECT26_CS_6_16";		ID[588]	=	"OBJECT07_CS_6_16";		ID[589]	=	"OBJECT02_CS_6_16";
	ID[590]	=	"OBJECT51_CS_6_16";		ID[591]	=	"KURTIS_CS_6_16";		ID[592]	=	"OBJECT56_CS_6_16";		ID[593]	=	"OBJECT24_CS_6_16";		ID[594]	=	"OBJECT16_CS_6_16";
	ID[595]	=	"OBJECT62_CS_6_16";		ID[596]	=	"OBJECT18_CS_6_16";		ID[597]	=	"LARA_CS_6_16";			ID[598]	=	"OBJECT45_CS_6_16";		ID[599]	=	"OBJECT47_CS_6_16";
	ID[600]	=	"OBJECT58_CS_6_16";		ID[601]	=	"OBJECT52_CS_6_16";		ID[602]	=	"OBJECT15_CS_6_16";		ID[603]	=	"OBJECT01_CS_6_16";		ID[604]	=	"OBJECT60_CS_6_16";
	ID[605]	=	"OBJECT48_CS_6_16";		ID[606]	=	"OBJECT46_CS_6_16";		ID[607]	=	"OBJECT55_CS_6_16";		ID[608]	=	"GUNDERSO_CS_6_16";		ID[609]	=	"OBJECT54_CS_6_16";
	ID[610]	=	"OBJECT12_CS_6_16";		ID[611]	=	"OBJECT50_CS_6_16";		ID[612]	=	"OBJECT23_CS_6_16";		ID[613]	=	"OBJECT43_CS_6_16";		ID[614]	=	"OBJECT19_CS_6_16";
	ID[615]	=	"OBJECT28_CS_6_16";		ID[616]	=	"OBJECT63_CS_6_16";		ID[617]	=	"OBJECT53_CS_6_16";		ID[618]	=	"OBJECT25_CS_6_16";		ID[619]	=	"OBJECT59_CS_6_16";
	ID[620]	=	"OBJECT42_CS_6_16";		ID[621]	=	"STRAHOVA_CS_6_16";		ID[622]	=	"OBJECT20_CS_6_16";		ID[623]	=	"OBJECT41_CS_6_16";		ID[624]	=	"OBJECT02_CS_6_21A";
	ID[625]	=	"OBJECT03_CS_6_21A";	ID[626]	=	"OBJECT03_CS_6_21B";	ID[627]	=	"BOUCHARD_CS_6_21B";	ID[628]	=	"OBJECT01_CS_6_21B";	ID[629]	=	"OBJECT02_CS_6_21B";
	ID[630]	=	"CHAUFFER_CS_6_21B";	ID[631]	=	"LARA_CS_6_21B";		ID[632]	=	"OBJECT02_CS_7_19";		ID[633]	=	"BOUCHARD_CS_7_19";		ID[634]	=	"OBJECT01_CS_7_19";
	ID[635]	=	"OBJECT05_CS_7_19";		ID[636]	=	"LARA_CS_7_19";			ID[637]	=	"OBJECT06_CS_7_19";		ID[638]	=	"OBJECT04_CS_7_19";		ID[639]	=	"ASSASSIN_CS_7_19";
	ID[640]	=	"OBJECT07_CS_7_19";		ID[641]	=	"OBJECT03_CS_7_19";		ID[642]	=	"OBJECT02_CS_9_12";		ID[643]	=	"OBJECT04_CS_9_12";		ID[644]	=	"LARA_CS_9_12";
	ID[645]	=	"GUNDERSO_CS_9_12";		ID[646]	=	"OBJECT01_CS_9_12";		ID[647]	=	"OBJECT03_CS_9_12";		ID[648]	=	"LUDDICK_CS_9_12";		ID[649]	=	"ECK_CS_9_12";
	ID[650]	=	"OBJECT01_CS_9_15B";	ID[651]	=	"OBJECT02_CS_9_15B";	ID[652]	=	"OBJECT03_CS_9_15B";	ID[653]	=	"STRAHOVA_CS_9_15B";	ID[654]	=	"PROTO_CS_9_15B";
	ID[655]	=	"OBJECT04_CS_9_15B";	ID[656]	=	"GUNDERSO_CS_10_8";		ID[657]	=	"BOAZ_CS_10_8";			ID[658]	=	"OBJECT01_CS_10_8";		ID[659]	=	"KAREL_CS_10_8";
	ID[660]	=	"ECK_CS_10_8";			ID[661]	=	"LARA_CS_10_8";			ID[662]	=	"MULLER_CS_10_8";		ID[663]	=	"BOAZ_POD_CS_10_8";		ID[664]	=	"KURTIS_CS_12_1";
	ID[665]	=	"ECK_CS_12_1";			ID[666]	=	"MENTAL_N_CS_12_1";		ID[667]	=	"LARA_CS_12_1";			ID[668]	=	"OBJECT10_CS_12_1";		ID[669]	=	"OBJECT09_CS_12_1";
	ID[670]	=	"OBJECT11_CS_12_1";		ID[671]	=	"KAREL_CS_12_1";		ID[672]	=	"OBJECT06_CS_12_1";		ID[673]	=	"OBJECT08_CS_12_1";		ID[674]	=	"OBJECT01_CS_12_1";
	ID[675]	=	"OBJECT13_CS_12_1";		ID[676]	=	"OBJECT05_CS_12_1";		ID[677]	=	"OBJECT07_CS_12_1";		ID[678]	=	"OBJECT03_CS_12_1";		ID[679]	=	"OBJECT02_CS_12_1";
	ID[680]	=	"OBJECT04_CS_12_1";		ID[681]	=	"OBJECT12_CS_12_1";		ID[682]	=	"ECK_CS_13_9";			ID[683]	=	"BOAZ_BIG_CS_13_9";		ID[684]	=	"OBJECT04_CS_13_9";
	ID[685]	=	"KURTIS_CS_13_9";		ID[686]	=	"GUNDERSO_CS_13_9";		ID[687]	=	"LARA_CS_13_9";			ID[688]	=	"OBJECT02_CS_13_9";		ID[689]	=	"MULLER_CS_13_9";
	ID[690]	=	"OBJECT01_CS_13_9";		ID[691]	=	"OBJECT05_CS_13_9";		ID[692]	=	"OBJECT03_CS_13_9";		ID[693]	=	"KURTIS_CS_14_4A";		ID[694]	=	"OBJECT01_CS_14_4A";
	ID[695]	=	"BOAZ_FLY_CS_14_4A";	ID[696]	=	"BOAZ_FLY_CS_14_6";		ID[697]	=	"BOAZ_F2_CS_14_6";		ID[698]	=	"OBJECT04_CS_14_6";		ID[699]	=	"OBJECT01_CS_14_6";
	ID[700]	=	"OBJECT03_CS_14_6";		ID[701]	=	"KURTIS_CS_14_6";		ID[702]	=	"OBJECT10_CS_15_10";	ID[703]	=	"OBJECT11_CS_15_10";	ID[704]	=	"LARA_CS_15_10";
	ID[705]	=	"ECK_CS_15_10";			ID[706]	=	"OBJECT08_CS_15_10";	ID[707]	=	"OBJECT09_CS_15_10";	ID[708]	=	"OBJECT07_CS_15_10";	ID[709]	=	"OBJECT02_CS_15_10";
	ID[710]	=	"SLROPE_CS_15_10";		ID[711]	=	"OBJECT01_CS_15_10";	ID[712]	=	"OBJECT05_CS_15_10";	ID[713]	=	"SLEEPER_CS_15_10";		ID[714]	=	"OBJECT03_CS_15_10";
	ID[715]	=	"OBJECT06_CS_15_10";	ID[716]	=	"OBJECT12_CS_15_10";	ID[717]	=	"OBJECT04_CS_15_10";	ID[718]	=	"OBJECT04_CS_15_14";	ID[719]	=	"OBJECT01_CS_15_14";
	ID[720]	=	"LARA_CS_15_14";		ID[721]	=	"KARELNC_CS_15_14";		ID[722]	=	"OBJECT03_CS_15_14";	ID[723]	=	"OBJECT02_CS_15_14";	ID[724]	=	"LUDDICK_CS_15_14";
	ID[725]	=	"KURTIS_CS_15_14";		ID[726]	=	"KAREL_CS_15_14";		ID[727]	=	"BOUCHARD_CS_15_14";	ID[728]	=	"ECK_CS_15_14";			ID[729]	=	"LARA_CS_15_24";
	ID[730]	=	"KAREL_CS_15_24";		ID[731]	=	"LAB_IG_11_22";

	// Aggiunge il file alla lista contenuta in IO e restituisce la stringa con il nome
	for (unsigned int i = 0; i < num_IDs; i++)
	{
		cal = chr = css = txt = cam = pos = tms = tmt = xxx = ID[i];
		cal.append(".CAL");
		chr.append(".CHR");
		css.append(".CSS");
		txt.append(".TXT");
		cam.append(".CAM");
		pos.append(".POS");
		tms.append(".TMS");
		tmt.append(".TMT");
		xxx.append(".XXX");
		if (GetHashValue(cal.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(cal, AoDFileType::CAL);			// File animazioni
		if (GetHashValue(chr.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(chr, AoDFileType::CHR);			// Scheletro, mesh e texture personaggi (tipo 1), nodi oggetti (tipo 2)
		if (GetHashValue(css.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(css, AoDFileType::CSS);
		if (GetHashValue(txt.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(txt, AoDFileType::TXT);
		if (GetHashValue(cam.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(cam, AoDFileType::CAM);			// Animazione telecamera per cutscenes
		if (GetHashValue(pos.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(pos, AoDFileType::POS);			// Root motion personaggi per cutscenes
		if (GetHashValue(tms.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(tms, AoDFileType::TMS);			// Curve di animazione dei blendshapes (tipo 1, contiene una sola animazione)
		if (GetHashValue(tmt.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(tmt, AoDFileType::TMT);			// Blendshapes
		if (GetHashValue(xxx.c_str()) == hash)
			return AOD_IO.AddFileToGMXList(xxx, AoDFileType::XXX);			// Root motion oggetti per cutscenes
	}

	cbh = cln = evx = rmx = scx = AOD_IO.levelname;
	cbh.append(".CBH");
	cln.append(".CLN");
	evx.append(".EVX");
	rmx.append(".RMX");
	scx.append(".SCX");
	if (GetHashValue(cbh.c_str()) == hash)
		return AOD_IO.AddFileToGMXList(cbh, AoDFileType::CBH);				// Camere fisse nei livelli
	if (GetHashValue(cln.c_str()) == hash)
		return AOD_IO.AddFileToGMXList(cln, AoDFileType::CLN);				// Collisioni
	if (GetHashValue(evx.c_str()) == hash)
		return AOD_IO.AddFileToGMXList(evx, AoDFileType::EVX);
	if (GetHashValue(rmx.c_str()) == hash)
		return AOD_IO.AddFileToGMXList(rmx, AoDFileType::RMX);				// File di progetto livello
	if (GetHashValue(scx.c_str()) == hash)
		return AOD_IO.AddFileToGMXList(scx, AoDFileType::SCX);


	for (unsigned int i = 0; i < 10; i++)			// Il level exporter supporta fino a 10 zone (il massimo usato nel gioco è 7)
	{
		stringstream zone;
		zone << AOD_IO.levelname << ".Z0" << i;
		if (GetHashValue(zone.str().c_str()) == hash)
			return AOD_IO.AddFileToGMXList(zone.str(), AoDFileType::ZONE);		// Mesh stanze, oggetti e collisioni oggetti
	}

	for (unsigned int i = 0; i < num_IDs; i++)		// Questo ciclo restituisce i nomi con estensione numerica (es. LARA.13)
	{
		for (unsigned int j = 0; j < 200; j++)		// Il massimo dovrebbe essere 176
		{
			stringstream tms2;
			tms2 << ID[i] << "." << j;
			if (GetHashValue(tms2.str().c_str()) == hash)
				return AOD_IO.AddFileToGMXList(tms2.str(), AoDFileType::TMS);	// Curve di animazione dei blendshapes (tipo 2, contiene molteplici animazioni)
		}
	}

	stringstream searchfailed;							// Se non trova alcuna corrispondenza restituisce l'hash originale
	searchfailed << std::hex << hash;
	msg(msg::TGT::FILE, msg::TYP::WARN) << "Unable to find original string for hash " << searchfailed.str();
	return AOD_IO.AddFileToGMXList(searchfailed.str(), AoDFileType::UNKNOWN);
}