/*
 * File: list-slays.h
 * Purpose: List of slay/brand types
 *
 * Entries in this table should be in ascending order of multiplier, to
 * ensure that the highest one takes precedence.
 * name
 * object flag
 * mon flag - which type of monster is vulnerable?
 * resist flag - monsters with this ignore the brand
 * vuln flag - monsters with this take extra damage (mult += 1)
 * multiplier
 * ranged verb - when missiles have this slay
 * melee verb - when melee weapons have this slay
 * verb describing what the thing does when it is active
 * description of affected creatures
 * brand name
 */
/*   name		object flag		mon flag	res flag 	vuln flag		mult ranged verb		melee verb			active verb			affected creatures							brand name */
SLAY(XXX, 		FLAG_END,       FLAG_END,  	FLAG_END,	FLAG_END,		0,	NULL,	    		NULL,      			NULL,         		NULL, 										NULL)
SLAY(IMPACT,	OF_IMPACT,		FLAG_END,	FLAG_END,	RF_HURT_ROCK,	1,	"bores into",		"hack at",			"hums",				"creatures made of stone",					NULL)
SLAY(TUNNEL,	OF_TUNNEL,		FLAG_END,	FLAG_END,	RF_HURT_ROCK,	1,	"bores into",		"hack at",			"hums",				"creatures made of stone",					NULL)
SLAY(LIGHT,		OF_LIGHT,		FLAG_END,	FLAG_END,	RF_HURT_LIGHT,	1,	"dazzles",			"dazzle",			"glows",			"creatures hurt by light",					NULL)
SLAY(ANIMAL2,	OF_SLAY_ANIMAL, RF_ANIMAL, 	FLAG_END,	FLAG_END,		2,	"pierces",  		"smite",   			"glows",      		"animals", 									NULL)
SLAY(EVIL2,		OF_SLAY_EVIL,   RF_EVIL,   	FLAG_END,	FLAG_END,		2,	"pierces",  		"smite",   			"glows",      		"evil creatures", 							NULL)
SLAY(ACID2,		OF_BRAND_FIZZ,  FLAG_END,  	RF_IM_ACID,	FLAG_END,		2,	"corrodes",			"corrode", 			"fizzes",   		"creatures not resistant to acid", 			"weak acid")
SLAY(ELEC2,		OF_BRAND_BUZZ,  FLAG_END,  	RF_IM_ELEC,	FLAG_END,		2,	"zaps",     		"zap",     			"buzzes",			"creatures not resistant to electricity", 	"weak lightning")
SLAY(FIRE2,		OF_BRAND_WARM,  FLAG_END,  	RF_IM_FIRE,	RF_HURT_FIRE,	2,	"singes",			"singe",			"grows warm", 		"creatures not resistant to fire", 			"weak flames")
SLAY(COLD2,		OF_BRAND_COOL,  FLAG_END,  	RF_IM_COLD,	RF_HURT_COLD,	2,	"chills" , 			"chill",  			"grows cool", 		"creatures not resistant to cold", 			"weak frost")
SLAY(POISON2,	OF_BRAND_ICKY,  FLAG_END,  	RF_IM_POIS,	FLAG_END,		2,	"sickens", 			"sicken",  			"glows green",  	"creatures not resistant to poison", 		"weak venom")
SLAY(UNDEAD2,	OF_HURT_UNDEAD, RF_UNDEAD, 	FLAG_END,	FLAG_END,		2,	"pierces",  		"smite",   			"glows",      		"undead", 									NULL)
SLAY(DEMON2,	OF_HURT_DEMON,	RF_DEMON,  	FLAG_END,	FLAG_END,		2,	"pierces",  		"smite",   			"glows",      		"demon", 									NULL)
SLAY(UNDEAD3,	OF_SLAY_UNDEAD, RF_UNDEAD, 	FLAG_END,	FLAG_END,		3,	"pierces",  		"smite",   			"glows",      		"undead", 									NULL)
SLAY(DEMON3,	OF_SLAY_DEMON,  RF_DEMON,  	FLAG_END,	FLAG_END,		3,	"pierces",  		"smite",   			"glows",      		"demons", 									NULL)
SLAY(ORC3,		OF_SLAY_ORC,    RF_ORC,    	FLAG_END,	FLAG_END,		3,	"pierces",  		"smite",   			"glows",      		"orcs", 									NULL)
SLAY(TROLL3,	OF_SLAY_TROLL,  RF_TROLL,  	FLAG_END,	FLAG_END,		3,	"pierces",  		"smite",   			"glows",      		"trolls", 									NULL)
SLAY(GIANT3,	OF_SLAY_GIANT,  RF_GIANT,  	FLAG_END,	FLAG_END,		3,	"pierces",  		"smite",   			"glows",      		"giants", 									NULL)
SLAY(DRAGON3,	OF_SLAY_DRAGON, RF_DRAGON, 	FLAG_END,	FLAG_END,		3,	"pierces",  		"smite",   			"glows",      		"dragons", 									NULL)
SLAY(ACID3,		OF_BRAND_ACID,  FLAG_END,  	RF_IM_ACID,	FLAG_END,		3,	"dissolves", 		"dissolve", 		"spits",      		"creatures not resistant to acid", 			"acid")
SLAY(ELEC3,		OF_BRAND_ELEC,  FLAG_END,  	RF_IM_ELEC,	FLAG_END,		3,	"shocks",     		"shock",     		"crackles",   		"creatures not resistant to electricity", 	"lightning")
SLAY(FIRE3,		OF_BRAND_FIRE,  FLAG_END,  	RF_IM_FIRE,	FLAG_END,		3,	"burns",    		"burn",    			"flares",     		"creatures not resistant to fire", 			"flames")
SLAY(COLD3,		OF_BRAND_COLD,  FLAG_END,  	RF_IM_COLD,	RF_HURT_FIRE,	3,	"freezes" , 		"freeze",  			"grows cold", 		"creatures not resistant to cold", 			"frost")
SLAY(POISON3,	OF_BRAND_POIS,  FLAG_END,  	RF_IM_POIS,	RF_HURT_COLD,	3,	"poisons",  		"poison",  			"seethes",    		"creatures not resistant to poison", 		"venom")
SLAY(DRAGON5, 	OF_KILL_DRAGON, RF_DRAGON, 	FLAG_END,	FLAG_END,		5,	"deeply pierces", 	"fiercely smite", 	"glows brightly", 	"dragons", 									NULL)
SLAY(DEMON5, 	OF_KILL_DEMON,  RF_DEMON,  	FLAG_END,	FLAG_END,		5,	"deeply pierces", 	"fiercely smite", 	"glows brightly", 	"demons", 									NULL)
SLAY(UNDEAD5, 	OF_KILL_UNDEAD, RF_UNDEAD, 	FLAG_END,	FLAG_END,		5,	"deeply pierces", 	"fiercely smite", 	"glows brightly", 	"undead", 									NULL)
