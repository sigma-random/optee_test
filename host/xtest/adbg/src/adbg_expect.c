/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*************************************************************************
 * 1. Includes
 ************************************************************************/
#include "adbg_int.h"

/*************************************************************************
 * 2. Definition of external constants and variables
 ************************************************************************/

/*************************************************************************
 * 3. File scope types, constants and variables
 ************************************************************************/

/*************************************************************************
 * 4. Declaration of file local functions
 ************************************************************************/

static bool ADBG_AssertHelper(ADBG_Case_t *const Case_p,
			      const char *const FileName_p,
			      const int LineNumber, const bool ExpressionOK);

static const char *ADBG_GetFileBase(const char *const FileName_p);

/*************************************************************************
 * 5. Definition of external functions
 ************************************************************************/
void Do_ADBG_Assert(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const bool ExpressionOK,
	const char *const Format_p, ...
	)
{
	char Buffer[ADBG_STRING_LENGTH_MAX];
	va_list List;

	if (ADBG_AssertHelper(Case_p, FileName_p, LineNumber, ExpressionOK))
		return;

	/*lint -save -e718 -e746 -e530 lint doesn't seem to know of va_start */
	va_start(List, Format_p);
	/*lint -restore */
	(void)vsnprintf(Buffer, sizeof(Buffer), Format_p, List);
	va_end(List);

	Do_ADBG_Log("%s:%d: %s",
		    ADBG_GetFileBase(FileName_p), LineNumber, Buffer);
}

bool Do_ADBG_Expect(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const int Expected,
	const int Got,
	const char *const GotVarName_p,
	const ADBG_EnumTable_t *const EnumTable_p
	)
{
	if (ADBG_AssertHelper(Case_p, FileName_p, LineNumber, Expected == Got))
		return true;

	if (EnumTable_p != NULL) {
		Do_ADBG_Log("%s:%d: %s has an unexpected value: 0x%x = %s, "
			    "expected 0x%x = %s",
			    ADBG_GetFileBase(FileName_p), LineNumber,
			    GotVarName_p,
			    Got, Do_ADBG_GetEnumName(Got, EnumTable_p),
			    Expected,
			    Do_ADBG_GetEnumName(Expected, EnumTable_p));
	} else {
		Do_ADBG_Log(
			"%s:%d: %s has an unexpected value: 0x%x, expected 0x%x",
			ADBG_GetFileBase(FileName_p), LineNumber,
			GotVarName_p, Got, Expected);
	}

	return false;
}

bool Do_ADBG_ExpectNot(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const int NotExpected,
	const int Got,
	const char *const GotVarName_p,
	const ADBG_EnumTable_t *const EnumTable_p
	)
{
	if (ADBG_AssertHelper(Case_p, FileName_p, LineNumber, NotExpected !=
			      Got))
		return true;

	if (EnumTable_p != NULL) {
		Do_ADBG_Log("%s:%d: %s has an unexpected value: 0x%x = %s, "
			    "expected 0x%x = %s",
			    ADBG_GetFileBase(FileName_p), LineNumber,
			    GotVarName_p,
			    Got, Do_ADBG_GetEnumName(Got, EnumTable_p),
			    NotExpected,
			    Do_ADBG_GetEnumName(NotExpected, EnumTable_p));
	} else {
		Do_ADBG_Log(
			"%s:%d: %s has an unexpected value: 0x%zu, expected 0x%zu",
			ADBG_GetFileBase(FileName_p), LineNumber,
			GotVarName_p, (size_t)Got, (size_t)NotExpected);
	}

	return false;
}

bool Do_ADBG_ExpectBuffer(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const void *const ExpectedBuffer_p,
	const size_t ExpectedBufferLength,
	const char *const GotBufferName_p,
	const void *const GotBuffer_p,
	const char *const GotBufferLengthName_p,
	const size_t GotBufferLength
	)
{
	if (!ADBG_AssertHelper(Case_p, FileName_p, LineNumber,
			       ExpectedBufferLength == GotBufferLength)) {
		Do_ADBG_Log(
			"%s:%d: %s has an unexpected value: %zu, expected %zu",
			ADBG_GetFileBase(
				FileName_p), LineNumber,
			GotBufferLengthName_p, GotBufferLength,
			ExpectedBufferLength);
		return false;
	}

	if (!ADBG_AssertHelper(Case_p, FileName_p, LineNumber,
			       memcmp(ExpectedBuffer_p, GotBuffer_p,
				      ExpectedBufferLength) == 0)) {
		Do_ADBG_Log("%s:%d: %s has an unexpected content:",
			    ADBG_GetFileBase(
				    FileName_p), LineNumber, GotBufferName_p);
		Do_ADBG_Log("Got");
		Do_ADBG_HexLog(GotBuffer_p, GotBufferLength, 16);
		Do_ADBG_Log("Expected");
		Do_ADBG_HexLog(ExpectedBuffer_p, ExpectedBufferLength, 16);
		return false;
	}

	return true;
}

bool Do_ADBG_ExpectPointer(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const void *Expected_p,
	const void *Got_p,
	const char *const GotVarName_p
	)
{
	if (ADBG_AssertHelper(Case_p, FileName_p, LineNumber, Expected_p ==
			      Got_p))
		return true;

	Do_ADBG_Log("%s:%d: %s has an unexpected value: %p, expected %p",
		    ADBG_GetFileBase(FileName_p), LineNumber,
		    GotVarName_p, Got_p, Expected_p);

	return false;
}



bool Do_ADBG_ExpectPointerNotNULL(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const void *Got_p,
	const char *const GotVarName_p
	)
{
	if (ADBG_AssertHelper(Case_p, FileName_p, LineNumber, Got_p != NULL))
		return true;

	Do_ADBG_Log("%s:%d: %s has an unexpected value: %p, expected not NULL",
		    ADBG_GetFileBase(FileName_p), LineNumber,
		    GotVarName_p, Got_p);

	return false;
}

bool Do_ADBG_ExpectCompareSigned(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const long Value1,
	const long Value2,
	const bool Result,
	const char *const Value1Str_p,
	const char *const ComparStr_p,
	const char *const Value2Str_p
	)
{
	if (!ADBG_AssertHelper(Case_p, FileName_p, LineNumber, Result)) {
		Do_ADBG_Log(
			"%s:%d: Expression \"%s %s %s\" (%ld %s %ld) is false",
			ADBG_GetFileBase(FileName_p), LineNumber,
			Value1Str_p, ComparStr_p, Value2Str_p,
			Value1, ComparStr_p, Value2);
	}
	return Result;
}

bool Do_ADBG_ExpectCompareUnsigned(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const unsigned long Value1,
	const unsigned long Value2,
	const bool Result,
	const char *const Value1Str_p,
	const char *const ComparStr_p,
	const char *const Value2Str_p
	)
{
	if (!ADBG_AssertHelper(Case_p, FileName_p, LineNumber, Result)) {
		Do_ADBG_Log(
			"%s:%d: Expression \"%s %s %s\" (%lu %s %lu) is false",
			ADBG_GetFileBase(FileName_p), LineNumber,
			Value1Str_p, ComparStr_p, Value2Str_p,
			Value1, ComparStr_p, Value2);
	}
	return Result;
}


bool Do_ADBG_ExpectComparePointer(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const void *const Value1_p,
	const void *const Value2_p,
	const bool Result,
	const char *const Value1Str_p,
	const char *const ComparStr_p,
	const char *const Value2Str_p
	)
{
	if (!ADBG_AssertHelper(Case_p, FileName_p, LineNumber, Result)) {
		Do_ADBG_Log(
			"%s:%d: Expression \"%s %s %s\" (%p %s %p) is false",
			ADBG_GetFileBase(FileName_p), LineNumber,
			Value1Str_p, ComparStr_p, Value2Str_p,
			Value1_p, ComparStr_p, Value2_p);
	}
	return Result;
}



size_t Do_ADBG_GetNumberOfErrors(ADBG_Case_t *const Case_p)
{
	return Case_p->CurrentSubCase_p->Result.NumFailedTests;
}


/*************************************************************************
 * 6. Definitions of internal functions
 ************************************************************************/
static bool ADBG_AssertHelper(
	ADBG_Case_t *const Case_p,
	const char *const FileName_p,
	const int LineNumber,
	const bool ExpressionOK
	)
{
	ADBG_SubCase_t *SubCase_p = Case_p->CurrentSubCase_p;

	IDENTIFIER_NOT_USED(Case_p)

	SubCase_p->Result.NumTests++;

	if (!ExpressionOK) {
		SubCase_p->Result.NumFailedTests++;
		if (SubCase_p->Result.FirstFailedRow == 0) {
			SubCase_p->Result.FirstFailedRow = LineNumber;
			SubCase_p->Result.FirstFailedFile_p = ADBG_GetFileBase(
				FileName_p);
		}
	}

	return ExpressionOK;
}

static const char *ADBG_GetFileBase(const char *const FileName_p)
{
	const char *Ch_p = FileName_p;
	const char *Base_p = FileName_p;

	while (*Ch_p != '\0') {
		if (*Ch_p == '\\')
			Base_p = Ch_p + 1;

		Ch_p++;
	}

	return Base_p;
}
