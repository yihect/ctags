/*
 *   $Id$
 *
 *   Copyright (c) 2000-2022, Tong Chen <yihect@gmail.com>.
 *
 *   This source code is released for free distribution under the terms of the
 *   GNU General Public License.
 *
 *   This module contains functions for generating tags for Ltd data descriptive
 *   language.
 */

/*
 *   INCLUDE FILES
 */
#include "general.h"  /* must always come first */

#include <string.h>

#include "options.h"
#include "parse.h"
#include "read.h"
#include "vstring.h"

/*
 *   DATA DEFINITIONS
 */
typedef enum {
  K_KEYWORD,
  K_RFX,
  K_VOC, K_VGRP,
  K_GRAM, K_VS
} ltaKind;

static kindOption LtdKinds [] = {
  { TRUE, 'r', "rfx", "root_fix" },
  { TRUE, 'v', "voc", "vocabulary" },
  { TRUE, 'p', "voc_group", "vocabulary_group" },
  { TRUE, 'g', "gram", "grammar" },
  { FALSE, 's', "sentense", "vocabulary_sentense" },
};

typedef enum {
  KW_ENUM, KW_RFX, KW_VOC, KW_VGRP, KW_GRAM, KW_VS,
  KW_ITEM, KW_RFREF, KW_VREF, KW_VGREF, KW_GREF,
} kword_idx;

struct kword{
  char *name;
  boolean valid;
};

static struct kword ltd_kwords [] = {
  {"ENUM", 1},
  {"RFX", 1},
  {"VOC", 1},
  {"VGRP", 1},
  {"GRAM", 1},
  {"VS", 0},
  {"ITEM", 1},
  {"RFREF", 0},
  {"VREF", 0},
  {"VGREF", 0},
  {"GREF", 0},
};

/*
 *   FUNCTION DEFINITIONS
 */

/* for debugging purposes */
static void /*__unused__ */print_string (char *p, char *q)
{
  for ( ; p != q; p++)
    fprintf (errout, "%c", *p);
  fprintf (errout, "\n");
}

/*
 * Helper function.
 * Returns 1 if line looks like a line of Ltd code.
 *
 * TODO: Recognize UNIX bang notation.
 * (Ltd treat first line as a comment if it starts with #!)
 *
 */
static boolean is_a_code_line (const unsigned char *line)
{
  boolean result;
  const unsigned char *p = line;
  while (isspace ((int) *p))
    p++;
  if (p [0] == '\0')
    result = FALSE;
  else if (p [0] == '#')
    result = FALSE;
  else
    result = TRUE;
  return result;
}

static void extract_name (const char *begin, const char *end, vString *name, const int kind)
{
  //print_string(begin, end);
  if (begin != NULL  &&  end != NULL  &&  begin < end)
  {
    const char *cp;

    while (isspace ((int) *begin))
      begin++;
    while (isspace ((int) *end))
      end--;
    if (begin < end)
    {
      for (cp = begin ; cp != end; cp++)
	vStringPut (name, (int) *cp);
      vStringTerminate (name);

      makeSimpleTag (name, LtdKinds, kind);
      vStringClear (name);
    }
  }
}


static int have_keyword(const unsigned char *line)
{
  int i;
  for(i=0; i<(sizeof(ltd_kwords)/sizeof(struct kword)); i++)
  {
    if((ltd_kwords[i].valid==1) && strstr(line, ltd_kwords[i].name)!=0)
      return i;
    else
      continue;
  }
  return -1;
}

static void process_keyword(int kw_type, const char *line)
{
  char *p, *q, *pt;
  vString *name = vStringNew ();

  if (kw_type == KW_ENUM) {
    p =pt= strchr(line, '{')+1;
    q = strchr(line, '}');
    while( pt<q)
    {
      if (*pt == ',') {
	extract_name(p, pt, name, K_KEYWORD);
	p = pt+1;
      }
      pt++;
    }
  }else if (kw_type>=KW_RFX && kw_type<=KW_VS) {
    p = strstr(line, ltd_kwords[kw_type].name)+strlen(ltd_kwords[kw_type].name);
    q = ((pt=strchr(line, '.'))==NULL) ? strchr(line, '{') : pt;
    extract_name(p, q, name, kw_type);
  }else if  (kw_type == KW_ITEM) {
    p = strstr(line, ltd_kwords[kw_type].name)+strlen(ltd_kwords[kw_type].name);
    if ((pt=strchr(line, '.')) != NULL)
    {
      if (NULL == strstr(line, "REF"))
	extract_name(p, pt, name, kw_type);
    } else if ((pt=strchr(line, ':')) != NULL)
      extract_name(p, pt, name, kw_type);
  }else if  (kw_type>=KW_RFREF && kw_type<=KW_GREF) {
    //nothing
  }


  vStringDelete (name);
}

static void findLtdTags (void)
{
  int res=0;
  vString *name = vStringNew ();
  const unsigned char *line;

  verbose("[TONG] find every ltd tags.\n");
  while ((line = fileReadLine ()) != NULL)
  {
    const char *p, *q;

    verbose("[TONG] have line %s\n", line);
    if (! is_a_code_line (line))
      continue;

    if ((res = have_keyword(line)) >= 0) 
      process_keyword(res, line);
  }
  vStringDelete (name);
}

extern parserDefinition* LtdParser (void)
{
  verbose("this is LtdParser");
  static const char* const extensions [] = { "ltd", NULL };
  parserDefinition* def = parserNew ("Ltd");
  def->kinds      = LtdKinds;
  def->kindCount  = KIND_COUNT (LtdKinds);
  def->extensions = extensions;
  def->parser     = findLtdTags;
  return def;
}

/* vi:set tabstop=4 shiftwidth=4: */
