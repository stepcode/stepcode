#ifndef LAZYREFS_H
#define LAZYREFS_H

#include <string>
#include <set>
#include <utility>
#include <vector>

#include "lazyTypes.h"
#include "ExpDict.h"
#include "sdaiApplication_instance.h"



class lazyRefs {
    public:
        //typedefs
        typedef std::set<std::string> * entityTypeSet;
        typedef std::pair< EntityDescriptor *, const char * > attrPair; //EntityDescriptor and attr name
        typedef std::vector< STEPattribute * > attrVec;
    protected:
    /* don't need this - the default comparator of std::set, std::less, should work
        ///functor to compare std::pair<> of pointers; in this case, those in attrPair
        struct pairCmp {
            bool operator()( const attrPair & lhs, const attrPair & rhs ) const {
                if( lhs.first == rhs.first && lhs.second == rhs.second ) {
                    return true;
                }
                return false;
            }
        };
        typedef std::set< attrPair, pairCmp > attrVec; //set of attrPair's, and comparison functor
    */

        ///find any inverse attributes, put in `attrs`
        /// attrs not necessarily in order!
        void getInverseAttrs( attrVec & attrs, EntityDescriptor * ed ) {
            attrs.clear();
            supertypesIterator supersIter( ed );
            InverseAItr invAttrIter;
            Inverse_attribute * invAttr;
            for( ; !supersIter.empty(); ++supersIter ) {
                //look at attrs of *si
                invAttrIter( *supersIter );
                while( 0 != ( invAttr = invAttrIter.NextInverse_attribute() ) ) {
                    attrs.push_back( invAttr );
                }
            }
            // look at our own attrs
            invAttrIter( *ed );
            while( 0 != ( invAttr = invAttrIter.NextInverse_attribute() ) ) {
                attrs.push_back( invAttr );
            }
        }

        void loadPossibleInverseRefs( instanceID id ) {
            EntityDescriptor * eDesc = getEntityDescriptor( id );
            //it would be nice to be able to check the type of each potential reference and only load those for which there is an inverse attr
            //however, that would require that we build a list of all types with inverse reference *and* all of their children
            //that requires looking up each type in the registry and iterating over its supertypes
            //slow? faster if results are cached, perhaps in std::set<std::string>

            /*
            * need to look for hits in _revInstanceRefs, then check the type of each
            * however, we must also check their supertypes - use checkIfEntityTypeMatch() (**** faster to compare entities with a subtype list instead? ****)
            * also, we must check which entity type actually declares the attr, and use that as the type name to match
            */



            //for each inverse attr, add that type and its subtypes to a set
            attrVec invAttrs;
            getInverseAttrs( invAttrs, eDesc );
            attrVec::iterator iAiter = invAttrs.begin();
            for( ; iAiter != invAttrs.end(); ++iAiter ) {
                //         const char * _inverted_attr_id;
                //         const char * _inverted_entity_id;

                iAiter->_inverted_attr_id;
                iAiter->_inverted_entity_id;
                iAiter->Owner();
            }

            //find possible referring instances and loop over them, checking each, loading instances of types that may have matching refs
            instanceRefs_t::cvector * possRefs = _revInstanceRefs.find( id );
            const instanceRefs_t::cvector::iterator it = possRefs->cbegin();
            for( ; it != possRefs->cend; ++it ) {
                //1. get inst type
                //2. compare its type to types that may reference this inst
                //3. if there is a match, load it and check whether this instance is reference by the correct attr
                //4. if so, link the two
                //5. repeat
            }
        }
    public:
        void loadInverseRefs( instanceID id ) {}
};

#endif //LAZYREFS_H
