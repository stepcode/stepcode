#ifndef INSTMGRHELPER_H
#define INSTMGRHELPER_H

#include <lazyInstMgr.h>
#include <instmgr.h>
#include <lazyTypes.h>

#include <sc_mutex.h>
#include <sc_thread.h>

/**
 * \file instMgrHelper.h helper classes for the lazyInstMgr. Allows use of SDAI_Application_instance class
 * without modification.
 */


/**
 * This class is used when creating SDAI_Application_instance's and using a lazyInstMgr. It is returned
 * by instMgrAdapter. SDAI_Application_instance only uses the GetSTEPentity function.
 */
class mgrNodeHelper: protected MgrNode {
    protected:
        lazyInstMgr * _lim;
        instanceID _id;
    public:
        mgrNodeHelper( lazyInstMgr * lim ) {
            _lim = lim;
            _id = 0;
        }
        inline void setInstance( instanceID id ) {
            _id = id;
        }
        inline SDAI_Application_instance * GetSTEPentity() {
//         unsigned int c = _lim->countDataSections();
            return _lim->loadInstance( _id );
        }

		///Thread safe counterpart of GetSTEPentity()
        inline SDAI_Application_instance * GetSTEPentitySafely() {
            return _lim->loadInstanceSafely( _id );
        }

};


/**
 * This class is used when creating SDAI_Application_instance's and using a lazyInstMgr.
 *
 * Instances need an InstMgr to look up the instances they refer to. This class pretends to be a normal InstMgr;
 * when an instance is looked up, this uses lazyInstMgr to load it, and then returns a pointer to it.
 */

class instMgrAdapter: public InstMgr {
    protected:
        mgrNodeHelper _mn; //Used in single threaded operations

        lazyInstMgr * _lim; //Used in multi threaded operations
		//map between threadID and the thread's local copy of mgrNodeHelper. Each thread has zero or one copy
		//of mgrNodeHelper assigned to it. This _map holds the pointer to that mgrNodeHelper. 
        idNodeMap_t _map;
        sc_mutex _mapMtx;

    public:
        instMgrAdapter( lazyInstMgr * lim ): InstMgr( 0 ), _mn( lim ) {
            _lim = lim;
            _map.clear();
        }

		//In case of multiple threads an explicit destructor is needed to free each threads mgrNodeHelper copy
        ~instMgrAdapter() {
            if( _map.empty() ) {
                return;
            }

            for( idNodeMap_t::iterator it = _map.begin(); it != _map.end(); it++ ) {
                delete it->second;
            }
        }

        inline mgrNodeHelper * FindFileId( int fileId ) {
            _mn.setInstance( fileId );
            return &_mn;
        }

        ///Thread-safe counterpart of FindFileId( fileId ). It protects the state of mgrNodeHelper.
        inline mgrNodeHelper * FindFileIdSafely( int fileId ) {
            mgrNodeHelper * _myMN;
            thread_id_t tid = sc_thread::getthread_id();

            _mapMtx.lock();
            idNodeMap_t::iterator it = _map.find( tid );

            if( it == _map.end() ) {
				//thread local copy yet not made. Hence create its copy.
                _myMN = new mgrNodeHelper( _lim );
                _map.insert( idNodePair_t( tid, _myMN ) );
            } else {
				//reuse the already existing copy.
                _myMN = it->second;
            }

            _mapMtx.unlock();

            _myMN->setInstance( fileId );
            return _myMN;
        }
};


#endif //INSTMGRHELPER_H

