#include <stdexcept>
#include <utility>
#include <ostream>

#include "cllazyfile/lazySupport.h"
#include "cllazyfile/lazyInstMgr.h"

namespace {
const instanceRefs & emptyIds() {
    static const instanceRefs ids;
    return ids;
}
}

LazyInstanceIdView::const_iterator LazyInstanceIdView::begin() const {
    return _ids ? _ids->begin() : emptyIds().begin();
}

LazyInstanceIdView::const_iterator LazyInstanceIdView::end() const {
    return _ids ? _ids->end() : emptyIds().end();
}

size_t LazyInstanceIdView::size() const {
    return _ids ? _ids->size() : 0;
}

bool LazyInstanceIdView::empty() const {
    return size() == 0;
}

instanceID LazyInstanceIdView::operator[]( size_t index ) const {
    if( !_ids || index >= _ids->size() ) {
        throw std::out_of_range( "LazyInstanceIdView index" );
    }
    return ( *_ids )[index];
}

LazyInstanceBatch::LazyInstanceBatch(): _manager( 0 ) {}

LazyInstanceBatch::LazyInstanceBatch( lazyInstMgr * manager,
        const std::vector<instanceID> & roots, const std::vector<instanceID> & instances ):
    _manager( manager ), _roots( roots ), _instances( instances ) {}

LazyInstanceBatch::~LazyInstanceBatch() {
    release();
}

LazyInstanceBatch::LazyInstanceBatch( LazyInstanceBatch && other ):
    _manager( other._manager ), _roots( std::move( other._roots ) ),
    _instances( std::move( other._instances ) ) {
    other._manager = 0;
}

LazyInstanceBatch & LazyInstanceBatch::operator=( LazyInstanceBatch && other ) {
    if( this != &other ) {
        release();
        _manager = other._manager;
        _roots = std::move( other._roots );
        _instances = std::move( other._instances );
        other._manager = 0;
    }
    return *this;
}

bool LazyInstanceBatch::valid() const {
    return _manager != 0;
}

void LazyInstanceBatch::release() {
    if( _manager ) {
        _manager->releaseBatch( _instances );
        _manager = 0;
    }
    _roots.clear();
    _instances.clear();
}

const std::vector<instanceID> & LazyInstanceBatch::roots() const {
    return _roots;
}

const std::vector<instanceID> & LazyInstanceBatch::instances() const {
    return _instances;
}

SDAI_Application_instance * LazyInstanceBatch::get( instanceID id ) const {
    return _manager ? _manager->cachedInstance( id ) : 0;
}

LazyTextDiagnosticAdapter::LazyTextDiagnosticAdapter( std::ostream & stream ): _stream( &stream ) {}

void LazyTextDiagnosticAdapter::operator()( const LazyDiagnostic & diagnostic ) const {
    const char * label = "info";
    if( diagnostic.severity == LAZY_DIAGNOSTIC_WARNING ) label = "warning";
    if( diagnostic.severity == LAZY_DIAGNOSTIC_ERROR ) label = "error";
    if( diagnostic.severity == LAZY_DIAGNOSTIC_FATAL ) label = "fatal";
    ( *_stream ) << label;
    if( diagnostic.entity ) ( *_stream ) << " #" << diagnostic.entity;
    if( !diagnostic.type.empty() ) ( *_stream ) << " " << diagnostic.type;
    if( diagnostic.offset ) ( *_stream ) << " at offset " << diagnostic.offset;
    if( diagnostic.line ) ( *_stream ) << " line " << diagnostic.line;
    if( !diagnostic.attribute.empty() ) ( *_stream ) << " attribute " << diagnostic.attribute;
    ( *_stream ) << ": " << diagnostic.message;
    if( diagnostic.occurrences > 1 ) ( *_stream ) << " (" << diagnostic.occurrences << " occurrences)";
    ( *_stream ) << std::endl;
}
