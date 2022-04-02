#pragma once

#include "WebConfig.h"

#define WEBCONFIG			"yyf.dat"
#define LOGO				"LOGO"
#define LOGO_MD5			"LOGO_MD5"
#define YYF2				"YYF2"
#define YYF2_MD5			"YYF2_MD5"
#define MODULEINJECT		"MODULEINJECT"
#define MODULEINJECT_MD5	"MODULEINJECT_MD5"
#define YYFPATCH			"YYFPATCH"
#define YYFPATCH_MD5		"YYFPATCH_MD5"
#define CCA					"CCA"
#define CCA_MD5				"CCA_MD5"
#define BIGMAP				"BIGMAP"
#define BIGMAP_MD5			"BIGMAP_MD5"
#define ITEM				"ITEM"
#define ITEM_MD5			"ITEM_MD5"
#define MAP					"MAP"
#define MAP_MD5				"MAP_MD5"
#define SKILL				"SKILL"
#define SKILL_MD5			"SKILL_MD5"

bool RecoverLogo(CWebConfig * pWebcfg,const char* url);

bool RecoverFile(CWebConfig * pWebcfg,const char* url);

CWebConfig* LoadWebConfig(const char* url);