#include <cstdlib>
#include <iostream>
#include <sstream>

#include "cllazyfile/lazyInstMgr.h"

namespace {
void require( bool condition, const char * message ) {
    if( !condition ) {
        std::cerr << "lazy_index_test: " << message << std::endl;
        std::exit( EXIT_FAILURE );
    }
}

void emptyRegistryInit( Registry & ) {
}
}

int main( int argc, char ** argv ) {
    require( argc == 4, "expected ordinary, scoped and broken-section fixture paths" );

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
    require( manager.componentTypes( 3 ).size() == 2 &&
        manager.componentTypes( 3 )[0] == "C" &&
        manager.componentTypes( 3 )[1] == "D",
        "complex component source order was not preserved" );
    require( manager.componentTypes( 1 ).empty() &&
        manager.componentTypes( 99 ).empty(),
        "ordinary or missing instance has complex components" );
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

    Registry emptyRegistry( emptyRegistryInit );
    manager.setRegistry( &emptyRegistry );
    std::ostringstream schemaWarnings;
    std::streambuf * originalCerr = std::cerr.rdbuf( schemaWarnings.rdbuf() );
    LazyInstanceBatch missingBatch = manager.loadBatch( 5 );
    LazyInstanceBatch secondBatch = manager.loadBatch( 1 );
    std::cerr.rdbuf( originalCerr );
    require( missingBatch.instances().size() == 1 &&
        missingBatch.instances()[0] == 5,
        "missing reference leaked into materialization closure" );
    const std::string warningText =
        "Warning - multiple schema names found. Only searching with first one.";
    const size_t firstWarning = schemaWarnings.str().find( warningText );
    require( firstWarning != std::string::npos &&
        schemaWarnings.str().find( warningText, firstWarning + 1 ) ==
            std::string::npos,
        "multiple FILE_SCHEMA warning was not bounded per file" );
    require( schemaWarnings.str().find( "Error loading instance" ) ==
            std::string::npos,
        "structured materialization diagnostic was also written to stderr" );
    secondBatch.release();
    missingBatch.release();

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
    require( scoped.componentTypes( 20 ).size() == 2 &&
        scoped.componentTypes( 20 )[0] == "C" &&
        scoped.componentTypes( 20 )[1] == "D",
        "scoped complex component order was not preserved" );
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
    /* A DATA section indexes its instances from its own constructor, under the
     * id it will be given, and is registered only afterwards and only if the
     * scan succeeded. A section that fails part way through therefore leaves
     * every instance it had already indexed naming a section that does not
     * exist. Asking for one of those must fail like any other unloadable
     * instance rather than index an empty vector. */
    lazyInstMgr broken;
    broken.setRegistry( &emptyRegistry );
    uint64_t brokenDiagnostics = 0;
    broken.setDiagnosticCallback( [&brokenDiagnostics]( const LazyDiagnostic & ) {
        ++brokenDiagnostics;
    } );
    broken.openFile( argv[3] );
    stats = broken.cacheStatistics();
    require( stats.dataSections == 0, "the failed data section was registered anyway" );
    require( broken.totalInstanceCount() == 1,
        "the instance indexed before the failure was dropped from the index" );
    require( broken.typeFromFile( 1 ) == 0,
        "type of an instance in an unregistered section was not refused" );
    require( broken.loadInstance( 1 ) == 0,
        "instance in an unregistered section was not refused" );
    require( brokenDiagnostics > 0,
        "refusing the instance produced no diagnostic" );
    require( broken.sourceRecord( 1 ).empty(),
        "source record of an instance in an unregistered section was not empty" );

    return EXIT_SUCCESS;
}
