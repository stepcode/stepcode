#ifndef LAZYREFS_H
#define LAZYREFS_H

#include <string>
#include <set>
#include <utility>
#include <vector>

#include "lazyTypes.h"
#include "lazyInstMgr.h"
#include "ExpDict.h"
#include "sdaiApplication_instance.h"
#include "SubSuperIterators.h"
#include <STEPattribute.h>
#include <STEPaggregate.h>

/*
 * given inverted attr ia:
 * attr method                     value
 * -----------                     -----
 * ia->Name()                    isdefinedby
 * ia->inverted_attr_id_()       relatedobjects
 * ia->inverted_entity_id_()     reldefinesbytype
 *
 * 1. for the instance in question, find inverse attrs with recursion
 * 2. look up references to the current instance (_r)
 *  a. for each item in _r, look up its type and add mapping (instanceID -> char* typestring) to _refMap
 * 3. for each ia,
 *  a. entity name is returned by ia->inverted_entity_id_()
 *  b. add this entity and its children to a list ( edL )
 *  c. compare the type of each item in _refMap with types in edL; for types in both, remember the instance number ( _referentInstances )
 *  d. load each instance in _referentInstances
 *  e. check if the relevant inverted attr of instances loaded from _referentInstances reference the instance in step 1; if not, unload **IF** the instance hadn't been loaded
 *     --> if it was loaded, this implies that it is used elsewhere
 *  f. (optional / TODO ) for performance, cache list edL - it may be useful in the future
 *     -- best to store such that the most recently (frequently?) used lists are retained and others are discarded
 */
 /* ****
 * ALTERNATE for 2a, 3c:
 * for each type t in edL, use lim->getInstances( t ) to get a list of instances of that type, Lt
 * for each instance i in Lt, check if it is in _r
 * if so, load it
 * BENEFIT: no need to write lazyInstMgr::getTypeStr() (however, it might be necessary in the future regardless)
 */

//TODO screen out intances that appear to be possible inverse refs but aren't actually
//      note - doing this well will require major changes, since each inst automatically loads every instance that it references
//TODO what about complex instances? scanning each on disk could be a bitch; should the compositional types be scanned during lazy loading?

class lazyRefs {
    public:
        typedef std::set< instanceID > referentInstances_t;
    protected:
        typedef std::set< const Inverse_attribute * > iaList_t;
        typedef judyLArray< instanceID, std::string * > refMap_t;
        typedef std::set< const EntityDescriptor * > edList_t;
        iaList_t _iaList;
        lazyInstMgr * _lim;
        instanceID _id;
        refMap_t _refMap;
        referentInstances_t _referentInstances;
        SDAI_Application_instance * _inst;

        void checkAnInvAttr( const Inverse_attribute * ia ) {
            const EntityDescriptor * ed;
            const Registry * reg = _lim->getMainRegistry();
            ed = reg->FindEntity( ia->_inverted_entity_id );
            subtypesIterator subtypeIter( ed );
            edList_t edL;
            edL.insert( ed );
            // 3b - use subtypeIter to add to edL
            for( ; !subtypeIter.empty(); ++subtypeIter ) {
                edL.insert( *subtypeIter );
            }
            //3c - for each item in both _refMap and edL, add it to _referentInstances
            potentialReferentInsts( edL );
            //3d - load each inst
            invAttrListNode * invNode = invAttr( _inst, ia /*, iaList*/ );
            referentInstances_t::iterator insts = _referentInstances.begin();
            for( ; insts != _referentInstances.end(); ++insts ) {
                loadInstIFFreferent( *insts, invNode );
            }
            //3f - cache edL - TODO
        }

        void loadInstIFFreferent( instanceID inst, invAttrListNode * invNode ) {
            bool prevLoaded = _lim->isLoaded( inst );
            SDAI_Application_instance * rinst = _lim->loadInstance( inst );
            bool ref = refersToCurrentInst( invNode->inverseADesc(), rinst );
            if( ref ) {
                EntityAggregate * ea = invNode->getter()( _inst );
                ea->AddNode( new EntityNode( rinst ) );
            } else {
                if( !prevLoaded ) {
                    //TODO _lim->unload( inst ); //this should keep the inst loaded for now, but put it in a list of ones that can be unloaded if not accessed
                }
            }
        }

        ///3e - check if actually inverse ref
        bool refersToCurrentInst( Inverse_attribute * ia, SDAI_Application_instance * referrer ) {
            //find the attr
            int rindex = attrIndex( referrer, ia->_inverted_attr_id, ia->_inverted_entity_id );
            STEPattribute sa = referrer->attributes[ rindex ];
            assert( ( sa.getADesc()->BaseType() == ENTITY_TYPE ) &&
                    ( sa.getADesc()->IsAggrType() ) );

            //search for current inst id
            EntityAggregate * aggr = dynamic_cast< EntityAggregate * >( sa.Aggregate());
            assert( aggr );
            EntityNode * en = ( EntityNode * ) aggr->GetHead();
            bool found = false;
            while( en ) {
                if( en->node == _inst ) {
                    found = true;
                    break;
                }
                en = ( EntityNode * ) en->NextNode();
            }
            if( !found ) {
                std::cerr << "inst #" << _inst->FileId() << " not found in #" << referrer->FileId();
                std::cerr << ", attr #" << rindex << " [contents: ";
                referrer->STEPwrite( std::cerr );
                std::cerr << "]" << std::endl;
            }
            return found;
        }

        int attrIndex( SDAI_Application_instance * inst, const char * name, const char * entity ) {
            for( int i = 0; i < inst->attributes.list_length(); i++ ) {
                std::cout << "attr " << i << " name " << inst->attributes[i].Name() << ", entity " << inst->EntityName() << std::endl;
                if( ( strcasecmp( name, inst->attributes[i].Name() ) == 0 ) &&
                    ( strcasecmp( entity, inst->attributes[i].getADesc()->Owner().Name() ) == 0 ) ) {
                    return i;
                } else {
                }
            }
            return -1;
        }

        invAttrListNode * invAttr( SDAI_Application_instance * inst, const Inverse_attribute * ia /*, iaList_t & iaList */ ) {
            invAttrListNode * n = ( invAttrListNode * ) inst->iAttrs.GetHead();
            while( n ) {
                if( n->inverseADesc() == ia ) {
                    return n;
                }
                n = ( invAttrListNode * ) n->NextNode();
            }
            std::cerr << "Error! inverse attr " << ia->Name() << " (" << ia << ") not found in iAttrs (" << ( void * )( & ( inst->iAttrs ) ) << ")." << std::endl;
            return 0;
        }

        /**  3c. compare the type of each item in R with types in A
         * for items that match, remember the instance number (list C)
         */
        void potentialReferentInsts( edList_t & edL ) {
            refMap_t::pair kv = _refMap.begin();
            while( kv.value != 0 ) {
                std::set< const EntityDescriptor * >::iterator edi = edL.begin();
                for( ; edi != edL.end(); ++edi ) {
                    if( 0 == strcasecmp( kv.value->c_str(), ( *edi )->Name() ) ) {
                        _referentInstances.insert( kv.key );
                        break;
                    }
                }
                kv = _refMap.next();
            }
        }

        ///find any inverse attributes, put in `iaList`
        /// attrs not necessarily in order!
        void getInverseAttrs( const EntityDescriptor * ed, iaList_t & iaList ) {
            iaList.clear();
            supertypesIterator supersIter( ed );
            const Inverse_attribute * iAttr;
            for( ; !supersIter.empty(); ++supersIter ) {
                //look at attrs of *si
                InverseAItr iai( ( *supersIter )->InverseAttr() );
                while( 0 != ( iAttr = iai.NextInverse_attribute() ) ) {
                    iaList.insert( iAttr );
                }
            }
            // look at our own attrs
            InverseAItr invAttrIter( ed->InverseAttr() );
            while( 0 != ( iAttr = invAttrIter.NextInverse_attribute() ) ) {
                iaList.insert( iAttr );
            }
        }

        // 2. find reverse refs
        //2a. convert to map where K=instanceID and V=char*
        // rather than keeping each V in memory or trying to free non-unique ones, look up each type in the Registry and use that pointer
        bool mapRefsToTypes() {
            _refMap.clear( true ); // true -> use delete on pointers
            instanceRefs_t::cvector * refs = _lim->getRevRefs()->find( _id );
            if( !refs || refs->empty() ) {
                return false;
            }
            instanceRefs_t::cvector::const_iterator it;
            for( it = refs->begin(); it != refs->end(); ++it ) {
                const char * type = _lim->typeFromFile( *it );
                _refMap.insert( *it, new std::string( type ) );
            }
            return true;
        }
    public:
        lazyRefs( lazyInstMgr * lmgr ): _lim( lmgr ), _id( 0 ) {
            _iaList.clear();
        }
        lazyRefs( lazyInstMgr * lmgr, SDAI_Application_instance * ai ): _lim( lmgr ), _id( 0 ) {
            _iaList.clear();
            init( 0, ai );
        }
        lazyRefs( lazyInstMgr * lmgr, instanceID iid ): _lim( lmgr ) {
            _iaList.clear();
            init( iid, 0 );
        }

        ~lazyRefs() {
            // delete strings in refMap
            _refMap.clear( true );
        }

        /// initialize with the given instance; will use ai if given, else loads instance iid
        void init( instanceID iid, SDAI_Application_instance * ai = 0 ) {
            if( iid == 0 && ai == 0 ) {
                std::cerr << "Error at " << __FILE__ << ":" << __LINE__ << " - both args are null" << std::endl;
                return;
            }

            if( !ai ) {
                _inst = _lim->loadInstance( iid );
                _id = iid;
            } else {
                _inst = ai;
                _id = _inst->GetFileId();
            }
            _refMap.clear( true );


            // 1. find inverse attrs with recursion
            getInverseAttrs( ai->eDesc, _iaList );

            //2. find reverse refs, map id to type (stop if there are no inverse attrs or no refs)
            if( _iaList.size() == 0 || !mapRefsToTypes() ) {
                return;
            }

            iaList_t::iterator iai = _iaList.begin();
            for( ; iai != _iaList.end(); ++iai ) {
                // 3. for each IA, ...
                checkAnInvAttr( *iai );
            }
        }

        referentInstances_t result() {
            return _referentInstances;
        }


};
#endif //LAZYREFS_H
