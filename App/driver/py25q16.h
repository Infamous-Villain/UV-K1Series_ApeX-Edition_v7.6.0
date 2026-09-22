/* Copyright 2025 muzkr
 * https://github.com/muzkr
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#ifndef DRIVER_PY25Q16_H
#define DRIVER_PY25Q16_H

#include <stdint.h>
#include <stdbool.h>

void PY25Q16_Init();
void PY25Q16_ReadBuffer(uint32_t Address, void *pBuffer, uint32_t Size);
void PY25Q16_WriteBuffer(uint32_t Address, const void *pBuffer, uint32_t Size, bool Append);
void PY25Q16_SectorErase(uint32_t Address);

// --- Deferred-write support (ENABLE_DEFERRED_FLASH_WRITES) -----------------
// With the feature enabled, PY25Q16_WriteBuffer() does NOT erase+program the
// flash synchronously when a sector erase would be required (~300ms stall).
// Instead the new sector image is kept in the driver's sector cache (write-
// back) and PY25Q16_FlushPendingWrite() performs the erase+program+verify
// later, from the 10ms time slice. All writes to the same sector between two
// flushes are coalesced into a single erase cycle (less wear, fewer stalls).
// Reads hitting the dirty cached sector are served coherently from the cache.
//
// When the feature is disabled these are no-ops / always-true.
void PY25Q16_FlushPendingWrite(void);   // write back the dirty sector, if any
bool PY25Q16_HasPendingWrite(void);     // true if a flush is pending
bool PY25Q16_LastFlushVerified(void);   // read-back verify result of last flush

// Like PY25Q16_ReadBuffer, but waits for the flash to be idle (WIP=0) first.
void PY25Q16_ReadBufferSafe(uint32_t Address, void *pBuffer, uint32_t Size);

// Drop the internal sector cache (after raw flash ops behind the driver's back,
// e.g. multiboot slot/bank operations). Does NOT flush a pending deferred
// write - it may run with IRQs masked - so call PY25Q16_FlushPendingWrite()
// first while IRQs/DMA are still available.
void PY25Q16_InvalidateCache(void);

#ifdef ENABLE_FEAT_F4HWN_MULTIBOOT
// Multiboot per-bank config banking, ported from F4HWN (armel, 3a50e212).
// Accesses BELOW PY25Q16_BANK_SHARED_FROM are shifted into the active bank;
// calibration, boot logo, firmware slots and the multiboot marker live at or
// above that boundary and stay shared by every bank. Set once at boot from
//   PY25Q16_SetBankBase(MB_BankBase(MB_BootResolveState()));
#define PY25Q16_BANK_SHARED_FROM  0x00010000u   /* calibration boundary */
void PY25Q16_SetBankBase(uint32_t Base);
#endif

#endif
