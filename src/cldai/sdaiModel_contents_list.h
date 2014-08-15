#ifndef SDAIMODEL_CONTENTS_LIST_H
#define SDAIMODEL_CONTENTS_LIST_H 1

#include <sc_export.h>

class SC_DAI_EXPORT SDAI_Model_contents__list : public SDAI__set {
    public:
        SDAI_Model_contents__list( int = 16 );
        ~SDAI_Model_contents__list();

        SDAI_Model_contents_ptr retrieve( int index );
        SDAI_Model_contents_ptr & operator[]( int index );
};

typedef SDAI_Model_contents__list *
SDAI_Model_contents__list_ptr;
typedef SDAI_Model_contents__list_ptr
SDAI_Model_contents__list_var;

#endif
