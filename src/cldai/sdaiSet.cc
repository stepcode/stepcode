#include <memory.h>
#include <math.h>

#include <sdai.h>
#include "sc_memmgr.h"

#ifndef HAVE_MEMMOVE
extern "C"
{
    void * memmove( void * __s1, const void * __s2, size_t __n );
}
#endif

/*****************************************************************************/

SDAI__set::SDAI__set( int defaultSize ) {
    _bufsize = defaultSize;
    _buf = new SDAI_ptr[_bufsize];
    _count = 0;
}

SDAI__set::~SDAI__set() {
    delete _buf;
}

void SDAI__set::Check( int index ) {
    SDAI_ptr * newbuf;

    if( index >= _bufsize ) {
        _bufsize = ( index + 1 ) * 2;
        newbuf = new SDAI_ptr[_bufsize];
        memmove( newbuf, _buf, _count * sizeof( SDAI_ptr ) );
        delete _buf;
        _buf = newbuf;
    }
}

void
SDAI__set::Insert( SDAI_ptr v, int index ) {

    SDAI_ptr * spot;
    index = ( index < 0 ) ? _count : index;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( SDAI_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void SDAI__set::Append( SDAI_ptr v ) {

    int index = _count;
    SDAI_ptr * spot;

    if( index < _count ) {
        Check( _count + 1 );
        spot = &_buf[index];
        memmove( spot + 1, spot, ( _count - index )*sizeof( SDAI_ptr ) );

    } else {
        Check( index );
        spot = &_buf[index];
    }
    *spot = v;
    ++_count;
}

void SDAI__set::Remove( int index ) {

    if( 0 <= index && index < _count ) {
        --_count;
        SDAI_ptr * spot = &_buf[index];
        memmove( spot, spot + 1, ( _count - index )*sizeof( SDAI_ptr ) );
    }
}

int SDAI__set::Index( SDAI_ptr v ) {

    for( int i = 0; i < _count; ++i ) {
        if( _buf[i] == v ) {
            return i;
        }
    }
    return -1;
}

SDAI_ptr
SDAI__set::retrieve( int index ) {
    return operator[]( index );
}

SDAI_ptr & SDAI__set::operator[]( int index ) {

    Check( index );
    _count = ( ( _count > index + 1 ) ? _count : ( index + 1 ) );
    return _buf[index];
}

int
SDAI__set::Count() {
    return _count;
}

int
SDAI__set::is_empty() {
    return _count;
}

void
SDAI__set::Clear() {
    _count = 0;
}
