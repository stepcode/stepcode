///test constructors, destructor, shallow copy, assignment operators for STEPattribute

#include "clstepcore/STEPattribute.h"
#include "clstepcore/ExpDict.h"
#include <stdint.h>


/// test copying a STEPattribute; returns true on success
bool testCopy( STEPattribute & attr, const char * desc ) {
    bool pass = true;
    STEPattribute    b = attr;
    STEPattribute    c;


    if( !( attr == attr ) ) {
        std::cerr << "test class initialization failed " << desc << std::endl;
        pass = false;
    }

    if( !( attr == b ) ) {
        std::cerr << "assignment operator failed " << desc << std::endl;
        pass = false;
    }

    c.ShallowCopy( & attr );
    if( !( attr == c ) ) {
        std::cerr << "ShallowCopy() failed " << desc << std::endl;
        pass = false;
    }

    return pass;
}

bool testEqu( const STEPattribute & a1, const STEPattribute & a2, bool invert, const char * desc ) {
    bool pass = ( invert ? ( !( a1 == a2 ) ) : ( a1 == a2 ) );
    if( !pass ) {
        std::cerr << "Comparison test " << desc << " failed." << std::endl;
    }
    return pass;
}

bool testRedefinedDerivedWrite( const AttrDescriptor & ad ) {
    SDAI_Integer inherited_value = 123L;
    SDAI_Integer narrowed_value = 456L;
    STEPattribute inherited( ad, & inherited_value );
    STEPattribute narrowed( ad, & narrowed_value );

    // A redeclared explicit attribute occupies its inherited physical-file
    // slot, which is also marked derived in the generated supertype class.
    inherited.Derive();
    inherited.RedefiningAttr( & narrowed );

    bool pass = true;
    std::string value;
    inherited.asStr( value );
    if( inherited.asStr() != "456" || value != "456" ) {
        std::cerr << "redefined derived attribute asStr() failed" << std::endl;
        pass = false;
    }

    std::ostringstream output;
    inherited.STEPwrite( output );
    if( output.str() != "456" ) {
        std::cerr << "redefined derived attribute STEPwrite() failed" << std::endl;
        pass = false;
    }

    return pass;
}

int main( int /*argc*/, char ** /*argv*/ ) {
    bool pass = true;
    EntityDescriptor ed( "ename", 0, LFalse, LFalse );

    // used to test copying without operator new
    TypeDescriptor   tdi( "tint", sdaiINTEGER, 0, "int" );
    AttrDescriptor   adi( "aint", & tdi, LFalse, LFalse, AttrType_Explicit, ed );
    SDAI_Integer     sint = 1234L, s2int = 123L;
    STEPattribute    ai( adi, & sint );

    //test copying with operator new (SDAI_Logical requires new)
    TypeDescriptor   tdl( "tlog", sdaiLOGICAL, 0, "str" );
    AttrDescriptor   adl( "alog", & tdl, LFalse, LFalse, AttrType_Explicit, ed );
    SDAI_LOGICAL     slog( LTrue );
    STEPattribute    al( adl, & slog );

    pass &= testCopy( ai, "without operator new" );
    pass &= testCopy( al, "with operator new" );

    pass &= testEqu( al, al, false, "LOGICAL ==" );
    pass &= testEqu( ai, ai, false, "int ==" );
    pass &= testEqu( ai, al, true, "int != LOGICAL" );

    STEPattribute aii( adi, & s2int );
    pass &= testEqu( ai, aii, true, "ints !=" );
    pass &= testRedefinedDerivedWrite( adi );

    if( pass ) {
        exit( EXIT_SUCCESS );
    }
    exit( EXIT_FAILURE );
}

