#include <sdai.h>

SDAI_sdaiObject::SDAI_sdaiObject() {
}

SDAI_sdaiObject::~SDAI_sdaiObject() {
}

#ifdef PART26
//SCLP26(Application_instance_ptr)
IDL_Application_instance_ptr
SDAI_sdaiObject::create_TIE() {
    cout << "ERROR sdaiObject::create_TIE() called." << endl;
    return 0;
}
#endif

