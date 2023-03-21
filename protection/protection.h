#pragma once
#include <windows.h>
#include <thread>
#include "lazy.h"
#include "skStr.h"

inline bool hide_thread(HANDLE thread)
{
	typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);
	NTSTATUS Status;

	pNtSetInformationThread NtSIT = (pNtSetInformationThread)LI_FN(GetProcAddress).forwarded_safe_cached()((LI_FN(GetModuleHandleA).forwarded_safe_cached())(skCrypt("ntdll.dll")), skCrypt("NtSetInformationThread"));

	if (NtSIT == NULL) return false;
	if (thread == NULL)
		Status = NtSIT(LI_FN(GetCurrentThread).forwarded_safe_cached(), 0x11, 0, 0);
	else
		Status = NtSIT(thread, 0x11, 0, 0);

	if (Status != 0x00000000)
		return false;
	else
		return true;
}

inline int thread_context() // we look for debuggers hiding in the thread context :sus:
{
	int found = false;
	CONTEXT ctx = { 0 };
	void* h_thread = LI_FN(GetCurrentThread).forwarded_safe_cached();

	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	if (LI_FN(GetThreadContext).forwarded_safe_cached()(h_thread, &ctx))
	{
		if ((ctx.Dr0 != 0x00) || (ctx.Dr1 != 0x00) || (ctx.Dr2 != 0x00) || (ctx.Dr3 != 0x00) || (ctx.Dr6 != 0x00) || (ctx.Dr7 != 0x00))
		{
			found = true;
		}
	}

	return found;
}

inline int remote_is_present()
{
	int debugger_present = false;

	LI_FN(CheckRemoteDebuggerPresent).forwarded_safe_cached()(LI_FN(GetCurrentProcess).forwarded_safe_cached()(), &debugger_present); // very interesting method of doing this? possible

	return debugger_present;
}

int is_debugger_present()
{
	return LI_FN(IsDebuggerPresent).forwarded_safe_cached()(); // i am very well aware i couldve just called this in the thread, but this looks better imo & has the same performance
}

inline bool debug_perms_check() // check if the program has debug permissions, if it does then it returns true
{
	PCONTEXT ctx = PCONTEXT(LI_FN(VirtualAlloc).forwarded_safe_cached()(NULL, sizeof(ctx), MEM_COMMIT, PAGE_READWRITE));
	RtlSecureZeroMemory(ctx, sizeof(CONTEXT));

	ctx->ContextFlags = CONTEXT_DEBUG_REGISTERS;

	if (LI_FN(GetThreadContext).forwarded_safe_cached()(LI_FN(GetCurrentThread).forwarded_safe_cached()(), ctx) == 0)
		return -1;


	if (ctx->Dr0 != 0 || ctx->Dr1 != 0 || ctx->Dr2 != 0 || ctx->Dr3 != 0)
		return TRUE;
	else
		return FALSE;
}

void Protection_Loop()
{
	hide_thread(LI_FN(GetCurrentThread).forwarded_safe_cached());
	if (thread_context()) *(uintptr_t*)(0) = 1;
	if (remote_is_present()) *(uintptr_t*)(0) = 1;
	if (is_debugger_present()) *(uintptr_t*)(0) = 1;
	if (debug_perms_check()) *(uintptr_t*)(0) = 1;
	std::this_thread::sleep_for(std::chrono::milliseconds(10)); // increase / decrease depending on how your computer handles it
}
