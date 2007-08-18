/*******************************************************************************
* This program and the accompanying materials
* are made available under the terms of the Common Public License v1.0
* which accompanies this distribution, and is available at 
* http://www.eclipse.org/legal/cpl-v10.html
* 
* Contributors:
*     Peter Smith
*******************************************************************************/

#include "XLUtil.h"
#include "XLObject.h"
#include "../common/Log.h"
#include "../java/JNI.h"

// Make an excel string
LPSTR XLUtil::MakeExcelString(const char* string)
{
	if(string == NULL) return NULL;
	size_t len = strlen(string);
	char* temp = (char *) malloc(len + 2);
	strcpy_s(temp + 1, len + 1, string);
	temp[0] = (BYTE) len;
	return temp;
}

// A helper function used to register a function
int XLUtil::RegisterFunction(LPXLOPER xllName, 
					  const char* procedure, const char* typeText, const char* functionText,
					  const char* argumentText, const char* macroType, const char* category,
					  const char* shortcutText, const char* helpTopic, 
					  const char* functionHelp, const char* argumentHelp)
{
	XLOPER args[10];
	for(int i = 0; i < 10; i++) {
		args[i].xltype = xltypeStr | xlbitXLFree;
	}
	args[0].val.str = MakeExcelString(procedure);
	args[1].val.str = MakeExcelString(typeText);
	args[2].val.str = MakeExcelString(functionText);
	args[3].val.str = MakeExcelString(argumentText);
	args[4].val.str = MakeExcelString(macroType);
	args[5].val.str = MakeExcelString(category);
	args[6].val.str = MakeExcelString(shortcutText);
	args[7].val.str = MakeExcelString(helpTopic);
	args[8].val.str = MakeExcelString(functionHelp);
	args[9].val.str = MakeExcelString(argumentHelp);

	// Check types for NULL
	for(int i = 0; i < 10; i++) {
		if(args[i].val.str == NULL)
			args[i].xltype = xltypeMissing;
	}

	int res = Excel4(xlfRegister, 0, 11, (LPXLOPER) xllName, (LPXLOPER) &args[0], 
		(LPXLOPER) &args[1], (LPXLOPER) &args[2], (LPXLOPER) &args[3],
		(LPXLOPER) &args[4], (LPXLOPER) &args[5], (LPXLOPER) &args[6], 
		(LPXLOPER) &args[7], (LPXLOPER) &args[8], (LPXLOPER) &args[9]);

	if(res != 0) {
		Log::SetLastError("Could not register function: %s", functionText);
	}

	// Free strings
	for(int i = 0; i < 10; i++) {
		if(!args[i].val.str == NULL)
			free(args[i].val.str);
	}

	return res;
}

void XLUtil::CopyValue(LPXLOPER xloperSrc, LPXLOPER xloperDst)
{
	memcpy(xloperDst, xloperSrc, sizeof(XLOPER));
	xloperDst->xltype = (xloperSrc->xltype & ~(xlbitXLFree | xlbitDLLFree));
}


jobject JNICALL XLUtil::Excel4J(JNIEnv* env, jobject self, int xlfn, jobjectArray args)
{
	LPXLOPER res = new XLOPER;
	res->xltype = xltypeNum;
	int numArgs = env->GetArrayLength(args);
	XLOPER* operArgs = new XLOPER[numArgs];
	for(int i = 0; i < numArgs; i++) {
		jobject arg = env->GetObjectArrayElement(args, i);
		CopyValue(XLObject::GetXLoper(env, arg), &operArgs[i]);
	}
	int fRes = Excel4v(xlfn, (LPXLOPER) res, numArgs, (LPXLOPER*) operArgs);
	delete [] operArgs;
	if(fRes) {
		delete res;
		// TODO return error code
		return NULL;
	}
	return XLObject::CreateXLObject(env, res);
}

int JNICALL XLUtil::XLCallVerJ(JNIEnv* env, jobject self)
{
	return XLCallVer();
}

void JNICALL XLUtil::SetLastError(JNIEnv* env, jobject self, jstring str)
{
	if(str == NULL)
		return;

	jboolean iscopy = false;
	if(env->ExceptionOccurred())
		env->ExceptionClear();
	const char* chars = env->GetStringUTFChars(str, &iscopy);
	Log::SetLastError(chars);
	env->ReleaseStringUTFChars(str, chars); 
}

bool XLUtil::RegisterNatives(JNIEnv *env)
{
	// Register excel native function
	jclass excelClass = env->FindClass("org/excel4j/Excel");
	if(excelClass == NULL) {
		Log::SetLastError("Could not find Excel class");
		return false;
	}
	
	JNINativeMethod nm[3];
	nm[0].name = "Excel4";
	nm[0].signature = "(I[Lorg/excel4j/XLObject;)Lorg/excel4j/XLObject;";
	nm[0].fnPtr = XLUtil::Excel4J;
	nm[1].name = "XLCallVer";
	nm[1].signature = "()I";
	nm[1].fnPtr = XLUtil::XLCallVerJ;
	nm[2].name = "SetLastError";
	nm[2].signature = "(Ljava/lang/String;)V";
	nm[2].fnPtr = XLUtil::SetLastError;
	env->RegisterNatives(excelClass, nm, 3);

	if(env->ExceptionCheck()) {
		Log::SetLastError(JNI::GetExceptionMessage(env));
		return false;
	}
	
	return true;
}
