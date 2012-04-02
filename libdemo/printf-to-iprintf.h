/**
 * @file
 *
 * @brief Maps printf() like functions to iprintf() equivalent.
 */

/*
 * Copyright (c) 2010-2011 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RTEMS_PRINTF_TO_IPRINTF_H
#define RTEMS_PRINTF_TO_IPRINTF_H

#define asprintf asiprintf
#define _asprintf_r _asiprintf_r
#define asnprintf asniprintf
#define _asnprintf_r _asniprintf_r
#define dprintf diprintf
#define _dprintf_r _diprintf_r
#define fprintf fiprintf
#define _fprintf_r _fiprintf_r
#define printf iprintf
#define _printf_r _iprintf_r
#define sprintf siprintf
#define _sprintf_r _siprintf_r
#define snprintf sniprintf
#define _snprintf_r _sniprintf_r
#define vasprintf vasiprintf
#define _vasprintf_r _vasiprintf_r
#define vasnprintf vasniprintf
#define _vasnprintf_r _vasniprintf_r
#define vdprintf vdiprintf
#define _vdprintf_r _vdiprintf_r
#define vfprintf vfiprintf
#define _vfprintf_r _vfiprintf_r
#define vprintf viprintf
#define _vprintf_r _viprintf_r
#define vsprintf vsiprintf
#define _vsprintf_r _vsiprintf_r
#define vsnprintf vsniprintf
#define _vsnprintf_r _vsniprintf_r

#endif /* RTEMS_PRINTF_TO_IPRINTF_H */
