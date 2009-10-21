/* Copyright (c) 2009 Open Information Security Foundation */

/** \file
 *  \author Breno Silva <breno.silva@gmail.com>
 */

#include "eidps-common.h"
#include "eidps.h"
#include "decode.h"
#include "detect.h"
#include "flow-var.h"
#include "decode-events.h"

#include "detect-gid.h"
#include "util-unittest.h"

#define PARSE_REGEX "[0-9]+"

static pcre *parse_regex;
static pcre_extra *parse_regex_study;

static int DetectGidSetup (DetectEngineCtx *, Signature *s, SigMatch *m, char *str);

/**
 * \brief Registration function for gid: keyword
 */

void DetectGidRegister (void) {
    sigmatch_table[DETECT_GID].name = "gid";
    sigmatch_table[DETECT_GID].Match = NULL;
    sigmatch_table[DETECT_GID].Setup = DetectGidSetup;
    sigmatch_table[DETECT_GID].Free  = NULL;
    sigmatch_table[DETECT_GID].RegisterTests = GidRegisterTests;

    const char *eb;
    int opts = 0;
    int eo;

    parse_regex = pcre_compile(PARSE_REGEX, opts, &eb, &eo, NULL);
    if(parse_regex == NULL)
    {
        printf("pcre compile of \"%s\" failed at offset %" PRId32 ": %s\n", PARSE_REGEX, eo, eb);
        goto error;
    }

    parse_regex_study = pcre_study(parse_regex, 0, &eb);
    if(eb != NULL)
    {
        printf("pcre study failed: %s\n", eb);
        goto error;
    }

error:
    return;

}

/**
 * \internal
 * \brief This function is used to parse gid options passed via gid: keyword
 *
 * \param rawstr Pointer to the user provided gid options
 *
 * \retval  gid number on success
 * \retval -1 on failure
 */
static uint32_t DetectGidParse (char *rawstr)
{
    int ret = 0, res = 0;
#define MAX_SUBSTRINGS 30
    int ov[MAX_SUBSTRINGS];
    const char *str_ptr = NULL;
    char *ptr = NULL;
    uint32_t rc;

    ret = pcre_exec(parse_regex, parse_regex_study, rawstr, strlen(rawstr), 0, 0, ov, MAX_SUBSTRINGS);

    if (ret < 1) {
        return -1;
    }

    res = pcre_get_substring((char *)rawstr, ov, MAX_SUBSTRINGS, 0, &str_ptr);

    if (res < 0) {
        return -1;
    }

    ptr = (char *)str_ptr;

    if(ptr == NULL)
        return -1;

    rc = (uint32_t )atol(ptr);

    free(ptr);
    return rc;
}

/**
 * \internal
 * \brief this function is used to add the parsed gid into the current signature
 *
 * \param de_ctx pointer to the Detection Engine Context
 * \param s pointer to the Current Signature
 * \param m pointer to the Current SigMatch
 * \param rawstr pointer to the user provided gid options
 *
 * \retval 0 on Success
 * \retval -1 on Failure
 */
static int DetectGidSetup (DetectEngineCtx *de_ctx, Signature *s, SigMatch *m, char *rawstr)
{
    s->gid = DetectGidParse(rawstr);

    if(s->gid > 0)
    return 0;

    return -1;
}

/*
 * ONLY TESTS BELOW THIS COMMENT
 */

#ifdef UNITTESTS
/**
 * \test GidTestParse01 is a test for a  valid gid value
 *
 *  \retval 1 on succces
 *  \retval 0 on failure
 */
static int GidTestParse01 (void) {

    int gid = 0;

    gid = DetectGidParse("1");

    if (gid == 1) {
        return 1;
    }

    return 0;
}

/**
 * \test GidTestParse02 is a test for an invalid gid value
 *
 *  \retval 1 on succces
 *  \retval 0 on failure
 */
static int GidTestParse02 (void) {

    int gid = 0;

    gid = DetectGidParse("a");

    if (gid > 1) {
        return 1;
    }

    return 0;
}
#endif /* UNITTESTS */

/**
 * \brief this function registers unit tests for Gid
 */
void GidRegisterTests(void) {
#ifdef UNITTESTS
    UtRegisterTest("GidTestParse01", GidTestParse01, 1);
    UtRegisterTest("GidTestParse02", GidTestParse02, 0);
#endif /* UNITTESTS */
}
