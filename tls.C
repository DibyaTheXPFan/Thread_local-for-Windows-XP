
/* This Library implements thread_local for NT5.1/NT5.2
*  Copyright 2023, Dibyamartanda Samanta <dibya.samanta@neverseen.de>
*  License: GPL 2.1/ Apache 2.0 
*/

include <ntos.h> // It just include all header their in ROS SDK & NDK 

DWORD WINAPI TlsAlloc( void )
{
    
    
    PEB * const peb = NtCurrentTeb()->ProcessEnvironmentBlock;
    RtlAcquirePebLock();
	DWORD index  = RtlFindClearBitsAndSet( peb->TlsBitmap, 1, 1 );
    if (index != ~0U) 
	{
	NtCurrentTeb()->TlsSlots[index] = 0; 
	}
    else
    {
        index = RtlFindClearBitsAndSet( peb->TlsExpansionBitmap, 1, 0 );
        if (index != ~0U)
        {
            if (!NtCurrentTeb()->TlsExpansionSlots &&
                !(NtCurrentTeb()->TlsExpansionSlots = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY,
                                         8 * sizeof(peb->TlsExpansionBitmapBits) * sizeof(void*) )))
            {
                RtlClearBits( peb->TlsExpansionBitmap, index, 1 );
                index = ~0U;
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            }
            else
            {
                NtCurrentTeb()->TlsExpansionSlots[index] = 0; /* clear the value */
                index = index+ TLS_MINIMUM_AVAILABLE;
            }
        }
        else SetLastError( ERROR_NO_MORE_ITEMS );
    }
    RtlReleasePebLock();
    return index;
}


BOOL WINAPI TlsFree( DWORD index )
{
    BOOL result;

    RtlAcquirePebLock();
	if(index < TLS_MINIMUM_AVAILABLE)
    {
        result = RtlAreBitsSet( NtCurrentTeb()->ProcessEnvironmentBlock->TlsBitmap, index, 1 );
        if (result ! =0) 
		RtlClearBits( NtCurrentTeb()->ProcessEnvironmentBlock->TlsBitmap, index, 1 );
    }
	
    if (index >= TLS_MINIMUM_AVAILABLE)
    {
        result = RtlAreBitsSet( NtCurrentTeb()->ProcessEnvironmentBlock->TlsExpansionBitmap, index - TLS_MINIMUM_AVAILABLE, 1 );
        if (result != 0) 
		RtlClearBits( NtCurrentTeb()->ProcessEnvironmentBlock->TlsExpansionBitmap, index - TLS_MINIMUM_AVAILABLE, 1 );
    }
    
    if (result != 0) 
	{
	NtSetInformationThread( GetCurrentThread(), ThreadZeroTlsCell, &index, sizeof(index) );
	}
    else 
	{
	SetLastError( ERROR_INVALID_PARAMETER );
    RtlReleasePebLock();
	}
	
    return result;
}


LPVOID WINAPI TlsGetValue( DWORD index )
{
    LPVOID result;

    if (index < TLS_MINIMUM_AVAILABLE)
    {
        result = NtCurrentTeb()->TlsSlots[index];
    }
    if(index >= TLS_MINIMUM_AVAILABLE
    {
        index = index - TLS_MINIMUM_AVAILABLE;
		
        if (index >= 8 * sizeof(NtCurrentTeb()->ProcessEnvironmentBlock->TlsExpansionBitmapBits))
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            result = NULL;
        }
		if (!NtCurrentTeb()->TlsExpansionSlots) 
		{
			result = NULL;
		}
        
        else 
		result = NtCurrentTeb()->TlsExpansionSlots[index];
    }
    SetLastError( ERROR_SUCCESS );
    return result;
}

// might have bugs 

BOOL WINAPI TlsSetValue( DWORD index, LPVOID value )
{
	bool result;
    if (index < TLS_MINIMUM_AVAILABLE)
    {
        NtCurrentTeb()->TlsSlots[index] = value;
    }
    if(index >= TLS_MINIMUM_AVAILABLE)
    {
        index = index - TLS_MINIMUM_AVAILABLE;
        if (index >= 8 * sizeof(NtCurrentTeb()->ProcessEnvironmentBlock->TlsExpansionBitmapBits))
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            result = FALSE;
        }
        if (!NtCurrentTeb()->TlsExpansionSlots &&
            !(NtCurrentTeb()->TlsExpansionSlots = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY,
                         8 * sizeof(NtCurrentTeb()->ProcessEnvironmentBlock->TlsExpansionBitmapBits) * sizeof(void*) )))
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            result = FALSE;
        }
        NtCurrentTeb()->TlsExpansionSlots[index] = value;
		return result;
    }
    return TRUE;
}
