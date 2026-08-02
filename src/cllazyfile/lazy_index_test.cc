#include <cstdlib>
#include <iostream>

#include "cllazyfile/lazyInstMgr.h"

namespace {
void require( bool condition, const char * message ) {
    if( !condition ) {
        std::cerr << "lazy_index_test: " << message << std::endl;
        std::exit( EXIT_FAILURE );
    }
}
}

int main( int argc, char ** argv ) {
    require( argc == 2, "expected fixture path" );

    lazyInstMgr manager;
    uint64_t progressCalls = 0;
    uint64_t diagnosticCalls = 0;
    LazyDiagnostic missingReference;
    manager.setProgressCallback( [&progressCalls]( const LazyScanProgress & progress ) {
        require( progress.fileSize >= progress.offset, "progress offset exceeds file size" );
        ++progressCalls;
    } );
    manager.setDiagnosticCallback( [&diagnosticCalls, &missingReference]( const LazyDiagnostic & diagnostic ) {
        ++diagnosticCalls;
        if( diagnostic.entity == 5 ) missingReference = diagnostic;
    } );
    manager.openFile( argv[1] );

    LazyCacheStatistics stats = manager.cacheStatistics();
    require( stats.instancesScanned == 5, "wrong scan count" );
    require( stats.dataSections == 2, "second DATA section was not indexed" );
    require( progressCalls > 0, "progress callback was not called" );
    require( manager.instancesByType( "a" ).size() == 1, "case-insensitive type index failed" );
    require( manager.allInstances().size() == manager.totalInstanceCount(), "all-instance index count failed" );
    require( manager.instancesByType( "C" ).size() == 1, "first complex component not indexed" );
    require( manager.instancesByType( "D" ).size() == 1, "second complex component not indexed" );
    require( manager.instancesByType( "" ).size() == 1, "complex instance index failed" );
    require( diagnosticCalls == 1, "structured diagnostic callback was not bounded" );
    require( missingReference.severity == LAZY_DIAGNOSTIC_ERROR && missingReference.offset > 0,
        "missing-reference diagnostic lacks structured context" );
    require( missingReference.message == "reference to missing instance #99", "wrong missing-reference diagnostic" );
    require( manager.forwardReferences( 1 ).size() == 1 && manager.forwardReferences( 1 )[0] == 2,
        "forward reference index failed" );
    require( manager.reverseReferences( 1 ).size() == 1 && manager.reverseReferences( 1 )[0] == 3,
        "reverse reference index failed" );

    lazyInstMgr cancelled;
    uint64_t cancellationCalls = 0;
    cancelled.setCancellationCallback( [&cancellationCalls]() {
        return ++cancellationCalls >= 2;
    } );
    cancelled.openFile( argv[1] );
    stats = cancelled.cacheStatistics();
    require( stats.cancelled, "cancellation was not recorded" );
    require( stats.instancesScanned == 2, "scan did not stop at cancellation boundary" );
    return EXIT_SUCCESS;
}
