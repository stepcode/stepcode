
#include <memory.h>
#include <math.h>

#include <sdai.h>
#include "sc_memmgr.h"

// to help ObjectCenter
#ifndef HAVE_MEMMOVE
extern "C"
{
    void * memmove( void * __s1, const void * __s2, size_t __n );
}
#endif


SDAI_PID::SDAI_PID() {
}

SDAI_PID::~SDAI_PID() {
}

SDAI_PID_DA::SDAI_PID_DA() {
}

SDAI_PID_DA::~SDAI_PID_DA() {
}

SDAI_PID_SDAI::SDAI_PID_SDAI() {
}

SDAI_PID_SDAI::~SDAI_PID_SDAI() {
}

SDAI_DAObject::SDAI_DAObject() {
}

SDAI_DAObject::~SDAI_DAObject() {
}

SDAI_DAObject_SDAI::SDAI_DAObject_SDAI() {
}

/*
SDAI_DAObject_SDAI::SDAI_DAObject_SDAI(const DAObject_SDAI&)
{
}
*/

SDAI_DAObject_SDAI::~SDAI_DAObject_SDAI() {
}

/*
 * Copyright (c) 1990, 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * UArray implementation.
 */

/*****************************************************************************/

SDAI_DAObject__set::SDAI_DAObject__set( int defaultSize ) : SDAI__set( defaultSize ){
}

SDAI_DAObject__set::~SDAI_DAObject__set() {
}

SDAI_DAObject_ptr
SDAI_DAObject__set::retrieve( int index ) {
    return operator[]( index );
}

SDAI_DAObject_ptr & SDAI_DAObject__set::operator[]( int index ) {
    return ( SDAI_DAObject_ptr & )SDAI__set::operator[]( index );
}
