//test SubtypesIterator, SupertypesIterator

//what about an iterator for inverse attrs?

//subtypesiterator shouldn't need tested separately from supertypesiterator since there is very little difference

#include "SubSuperIterators.h"
#include "ExpDict.h"

int main(int /*argc*/, char** /*argv*/) {
    Schema * a = 0;
    Logical b(LFalse);
    char buf[20][2]  = { { '\0' } };
    int i;
    EntityDescriptor * descriptors[20], ed( "ed", a, b, b );

    //create 20 more ed's
    for( i = 0; i < 20; i++ ) {
        buf[i][0] = 'a' + i; //ed names are 1st 20 lowercase chars
        descriptors[i] = new EntityDescriptor(buf[i], a, b, b );
    }
    //link the ed's together
    ed.AddSupertype(descriptors[0]);
    for( i = 0; i < 10; i++ ) {
        descriptors[i]->AddSupertype(descriptors[i+1]);
    }
    for( i = 10; i < 20; i++ ) {
        descriptors[5]->AddSupertype(descriptors[i]);
    }

    //print the ed's
    cout << "first, name " << ed.Name() << endl;
    supertypesIterator iter(&ed);
    for( ; !iter.empty(); iter++ ) {
        cout << "position " << iter.pos() << ", name " << iter->Name() << endl;
    }
    if( iter.pos() == 21 ) { //1 beyond the end
        cout << "success" << endl;
        exit( EXIT_SUCCESS );
    } else {
        cout << "expected 21, got " << iter.pos() << endl;
        exit( EXIT_FAILURE );
    }
}
/* output:
 *
 * first, name ed
 * position 0, name a
 * position 1, name b
 * position 2, name c
 * position 3, name d
 * position 4, name e
 * position 5, name f
 * position 6, name g
 * position 7, name k
 * position 8, name l
 * position 9, name m
 * position 10, name n
 * position 11, name o
 * position 12, name p
 * position 13, name q
 * position 14, name r
 * position 15, name s
 * position 16, name t
 * position 17, name h
 * position 18, name i
 * position 19, name j
 * position 20, name k
 * success
 */