
#ifndef SESSION_INSTANCE_H
#define SESSION_INSTANCE_H 1

//#include <sdai.h>

class SDAI_Session_instance : public SDAI_sdaiObject 
{

  public:
    int x;

    SDAI_Session_instance();
    virtual ~SDAI_Session_instance();
//    SDAI_Session_instance(const SDAI_Session_instance& si) {}

//    static SDAI_Session_instance_ptr 
//	_duplicate(SDAI_Session_instance_ptr sip);
//    static SDAI_Session_instance_ptr 
//	_narrow(SDAI_Object_ptr o);
//    static SDAI_Session_instance_ptr _nil();

};

typedef SDAI_Session_instance* SDAI_Session_instance_ptr;
typedef SDAI_Session_instance_ptr SDAI_Session_instance_var;

// the old names
//typedef SDAI_Session_instance SDAI_SessionInstance;
//typedef SDAI_Session_instance_ptr SDAI_SessionInstanceH;

#endif
