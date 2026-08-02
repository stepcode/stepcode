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
    require( argc == 3, "expected ordinary and scoped fixture paths" );

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
    const std::string firstSource = manager.sourceRecord( 1 );
    const std::string secondSource = manager.sourceRecord( 2 );
    require( firstSource.find( "#1=A(" ) != std::string::npos &&
        !firstSource.empty() && firstSource[firstSource.size() - 1] == ';',
        "first exact source record failed" );
    require( secondSource.find( "#2=B();" ) != std::string::npos &&
        !secondSource.empty() && secondSource[secondSource.size() - 1] == ';',
        "second exact source record or stream restoration failed" );
    require( manager.sourceRecord( 99 ).empty(),
        "missing source record was not empty" );

    lazyInstMgr cancelled;
    uint64_t cancellationCalls = 0;
    cancelled.setCancellationCallback( [&cancellationCalls]() {
        return ++cancellationCalls >= 2;
    } );
    cancelled.openFile( argv[1] );
    stats = cancelled.cacheStatistics();
    require( stats.cancelled, "cancellation was not recorded" );
    require( stats.instancesScanned == 2, "scan did not stop at cancellation boundary" );

    lazyInstMgr scoped;
    require( scoped.openFile( argv[2] ), "scoped fixture did not open" );
    stats = scoped.cacheStatistics();
    require( stats.instancesScanned == 5, "wrong scoped scan count" );
    require( scoped.instancesByType( "A" ).size() == 3,
        "scoped simple instances were not indexed" );
    require( scoped.instancesByType( "B" ).size() == 1,
        "nested scoped instance was not indexed" );
    require( scoped.instancesByType( "C" ).size() == 1 &&
        scoped.instancesByType( "D" ).size() == 1 &&
        scoped.instancesByType( "" ).size() == 1,
        "scoped complex owner was not indexed" );
    require( scoped.forwardReferences( 11 ).size() == 1 &&
        scoped.forwardReferences( 11 )[0] == 12,
        "nested scoped reference was not indexed" );
    require( scoped.forwardReferences( 20 ).size() == 1 &&
        scoped.forwardReferences( 20 )[0] == 21,
        "complex scope-owner reference was not indexed" );
    const std::string nestedOwner = scoped.sourceRecord( 11 );
    const std::string complexOwner = scoped.sourceRecord( 20 );
    require( nestedOwner.find( "#11=&SCOPE" ) != std::string::npos &&
        nestedOwner.find( "ENDSCOPE /#12/ A(" ) != std::string::npos &&
        !nestedOwner.empty() && nestedOwner[nestedOwner.size() - 1] == ';',
        "nested scope source record was not preserved" );
    require( complexOwner.find( "#20=&SCOPE" ) != std::string::npos &&
        complexOwner.find( "ENDSCOPE (C()D(" ) != std::string::npos &&
        !complexOwner.empty() && complexOwner[complexOwner.size() - 1] == ';',
        "complex scope source record was not preserved" );
    return EXIT_SUCCESS;
}
