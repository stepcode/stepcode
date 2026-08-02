#ifndef LAZYSUPPORT_H
#define LAZYSUPPORT_H

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "cllazyfile/lazyTypes.h"
#include "sc_export.h"

class lazyInstMgr;
class SDAI_Application_instance;

enum LazyDiagnosticSeverity {
    LAZY_DIAGNOSTIC_INFO,
    LAZY_DIAGNOSTIC_WARNING,
    LAZY_DIAGNOSTIC_ERROR,
    LAZY_DIAGNOSTIC_FATAL
};

struct SC_LAZYFILE_EXPORT LazyDiagnostic {
    LazyDiagnosticSeverity severity;
    instanceID entity;
    std::string type;
    lazyFileOffset offset;
    uint64_t line;
    std::string attribute;
    std::string message;
    uint64_t occurrences;

    LazyDiagnostic(): severity( LAZY_DIAGNOSTIC_INFO ), entity( 0 ), offset( 0 ),
        line( 0 ), occurrences( 1 ) {}
};

struct SC_LAZYFILE_EXPORT LazyScanProgress {
    fileID file;
    lazyFileOffset offset;
    lazyFileOffset fileSize;
    uint64_t instancesScanned;

    LazyScanProgress(): file( 0 ), offset( 0 ), fileSize( 0 ), instancesScanned( 0 ) {}
};

struct SC_LAZYFILE_EXPORT LazyCacheStatistics {
    uint64_t instancesScanned;
    uint64_t instancesLoaded;
    uint64_t instancesPinned;
    uint64_t cacheHighWater;
    uint64_t cacheHits;
    uint64_t cacheMisses;
    uint64_t materializations;
    uint64_t evictions;
    uint64_t activeBatches;
    uint64_t dataSections;
    uint64_t residentSourceBytes;
    uint64_t sourceBytesHighWater;
    bool cancelled;

    LazyCacheStatistics(): instancesScanned( 0 ), instancesLoaded( 0 ), instancesPinned( 0 ),
        cacheHighWater( 0 ), cacheHits( 0 ), cacheMisses( 0 ), materializations( 0 ),
        evictions( 0 ), activeBatches( 0 ), dataSections( 0 ), residentSourceBytes( 0 ),
        sourceBytesHighWater( 0 ), cancelled( false ) {}
};

typedef std::function<void ( const LazyScanProgress & )> LazyProgressCallback;
typedef std::function<bool ()> LazyCancellationCallback;
typedef std::function<void ( const LazyDiagnostic & )> LazyDiagnosticCallback;

/** A non-owning, zero-copy view of an indexed instance-ID vector.
 * The view remains valid until another file is scanned by the manager.
 */
class SC_LAZYFILE_EXPORT LazyInstanceIdView {
    public:
        typedef instanceRefs::const_iterator const_iterator;

        LazyInstanceIdView(): _ids( 0 ) {}
        explicit LazyInstanceIdView( const instanceRefs * ids ): _ids( ids ) {}

        const_iterator begin() const;
        const_iterator end() const;
        size_t size() const;
        bool empty() const;
        instanceID operator[]( size_t index ) const;

    private:
        const instanceRefs * _ids;
};

/** A dependency closure materialized and pinned for a bounded operation.
 * STEPcode parsing remains single-threaded.  Detach immutable conversion data
 * before releasing the batch or passing work to another thread.
 */
class SC_LAZYFILE_EXPORT LazyInstanceBatch {
    public:
        LazyInstanceBatch();
        ~LazyInstanceBatch();
        LazyInstanceBatch( LazyInstanceBatch && other );
        LazyInstanceBatch & operator=( LazyInstanceBatch && other );

        LazyInstanceBatch( const LazyInstanceBatch & ) = delete;
        LazyInstanceBatch & operator=( const LazyInstanceBatch & ) = delete;

        bool valid() const;
        void release();
        const std::vector<instanceID> & roots() const;
        const std::vector<instanceID> & instances() const;
        SDAI_Application_instance * get( instanceID id ) const;

    private:
        friend class lazyInstMgr;
        LazyInstanceBatch( lazyInstMgr * manager, const std::vector<instanceID> & roots,
            const std::vector<instanceID> & instances );

        lazyInstMgr * _manager;
        std::vector<instanceID> _roots;
        std::vector<instanceID> _instances;
};

/** Adapter preserving a stream-oriented diagnostic presentation. */
class SC_LAZYFILE_EXPORT LazyTextDiagnosticAdapter {
    public:
        explicit LazyTextDiagnosticAdapter( std::ostream & stream );
        void operator()( const LazyDiagnostic & diagnostic ) const;

    private:
        std::ostream * _stream;
};

#endif // LAZYSUPPORT_H
