//
// ReadStepData.cpp
//
// This examples demonstrate how to read STEP file and access all data using common SDAI API (without scheme specific libraries),
// known as late bingding.
// The code is suitable for any STEP scheme.
// Also it can load scheme dynamically (example for Winodws is provided)
//
// Start looking from 'main' function. 
//

#define _CRT_SECURE_NO_WARNINGS

#include "windows.h" //we need it for dynamic scheme loading only

//MSVC specific
#define SC_SCHEMA_DLL_IMPORTS
#pragma warning (disable:4275)
#pragma warning (disable:4251)


//Link list: stepeditor.lib;stepcore.lib;stepdai.lib;steputils.lib
#include <STEPfile.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <STEPaggregate.h>


#if 0
//
// static scheme
//
// Link with sdai_xxx.dll
#include <SdaiXXX.h>
static Registry* LoadScheme (const char* /*schemeName*/)
    {
    return new Registry (SchemaInit);
    }

#else
//
// dynamic scheme loading 
//
static Registry* LoadScheme (const char* schemeName)
    {
    std::string schemeDll = "sdai_";
    schemeDll.append (schemeName);
    schemeDll.append (".dll");

    HMODULE hLib = LoadLibraryA (schemeDll.c_str ());
    if (hLib <= (HMODULE) HINSTANCE_ERROR)
        {
        printf ("Can not load library %s\n", schemeDll.c_str ());
        return NULL;
        }

    CF_init fnInit = (CF_init) GetProcAddress (hLib, "?SchemaInit@@YAXAEAVRegistry@@@Z");
    if (!fnInit)
        {
        printf ("Can not find SchemeInit function in %s", schemeDll.c_str ());
        return NULL;
        }

    return new Registry (fnInit);
    }
#endif

//
// Access select data
//
static void PrintSelect (SDAI_Select* pSel)
    {
    if (pSel)
        {
        SDAI_String underlType = pSel->UnderlyingTypeName ();        
        printf ("object is %s, value is %s", typeid (*pSel).name (), underlType.c_str ());

        //more detailed, parsed info can be obtained by scheme specific class
        //on common level I only found
        std::string sval;
        pSel->STEPwrite (sval);
        }
    else
        {
        printf ("$");
        }
    }

//
// Access aggregate data
//
static void PrintAggregate (STEPaggregate* pAggr)
    {
    SingleLinkNode* pNode = pAggr->GetHead ();
    while (pNode)
        {
        if (IntNode* pIntNode = dynamic_cast<IntNode*> (pNode))
            {
            printf ("%d ", pIntNode->value);
            }
        else if (EntityNode* pEntNode = dynamic_cast<EntityNode*> (pNode))
            {
            printf ("%d ", (pEntNode && pEntNode->node) ? pEntNode->node->StepFileId () : -1);
            }
        else if (SelectNode* pSel = dynamic_cast<SelectNode*> (pNode))
            {
            SDAI_Select* pSelect = pSel->node;
            PrintSelect (pSelect);
            }
        else if (GenericAggrNode* pGenNode = dynamic_cast<GenericAggrNode*> (pNode))
            {
            SCLundefined val = pGenNode->value;
            printf ("TODO in SC. Implement typed aggregate for this entify");
            }
        else
            {
            printf ("PrintAggregate is not implemented for %s", typeid (*pNode).name ());
            }

        pNode = pNode->NextNode ();
        }
    }
    

//
// Access general attribute typed data
//
static void PrintAttributeByType (STEPattribute* pAttr)
    {
    PrimitiveType attrType = pAttr->Type ();

    switch (attrType)
        {
        case sdaiAGGR:
        case ARRAY_TYPE:      // DAS
        case BAG_TYPE:        // DAS
        case SET_TYPE:        // DAS
        case LIST_TYPE:       // DAS
            break;

        default:
            attrType = pAttr->BaseType ();
        }

    switch (attrType)
        {
        case sdaiINTEGER:
            {
            SDAI_Integer* pInt = pAttr->Integer ();
            printf (" %d ", pInt ? *pInt : 0);
            break;
            }

        case sdaiREAL:
            {
            SDAI_Real* pReal = pAttr->Real ();
            printf (" %g ", pReal ? *pReal : 0);
            break;
            }

        case sdaiBOOLEAN:
            {
            SDAI_BOOLEAN* pBool = pAttr->Boolean ();
            switch (pBool ? Boolean (*pBool) : Boolean::BUnset)
                {
                case BTrue:  printf (" true "); break;
                case BFalse: printf (" false "); break;
                case BUnset: printf (" $ "); break;
                }
            break;
            }

        case sdaiLOGICAL:
            {
            SDAI_LOGICAL* pLogical = pAttr->Logical ();
            switch (pLogical ? Logical (*pLogical) : Logical::LUnset)
                {
                case LTrue:  printf (" true "); break;
                case LFalse: printf (" false "); break;
                case LUnknown: printf (" unknown "); break;
                case LUnset: printf (" $ "); break;
                }
            break;
            }

        case sdaiSTRING:
            {
            SDAI_String* pStr = pAttr->String ();
            printf (" %s ", pStr ? pStr->c_str () : "$");
            break;
            }

        case sdaiBINARY:
            {
            SDAI_Binary* pBin = pAttr->Binary ();
            printf (" content: %s ", pBin ? pBin->c_str () : "$");
            break;
            }

        case sdaiENUMERATION:
            {
            SDAI_Enum* pEnum = pAttr->Enum ();
            if (pEnum)
                {
                int val = pEnum->asInt ();
                printf (" enum %s; value %d (%s) ", pEnum->Name (), val, pEnum->get_value_at (val));
                }
            else
                {
                printf (" $ ");
                }
            break;
            }

        case sdaiSELECT:
            {
            SDAI_Select* pSel = pAttr->Select ();
            PrintSelect (pSel);
            break;
            }

        case sdaiINSTANCE:
            {
            SDAI_Application_instance* pInst = pAttr->Entity ();
            printf (" instance #%d ", pInst ? pInst->StepFileId () : -1);
            break;
            }

        case sdaiAGGR:
        case ARRAY_TYPE:      // DAS
        case BAG_TYPE:        // DAS
        case SET_TYPE:        // DAS
        case LIST_TYPE:       // DAS
            {
            STEPaggregate* pAggr = pAttr->Aggregate ();
            PrintAggregate (pAggr);
            break;
            }

        case sdaiNUMBER:
            {
            SDAI_Real* pNumber = pAttr->Number ();
            printf (" %g ", pNumber ? *pNumber : 0);
            break;
            }

        default:
            printf ("       NOT IMPLEMENTED PRINT ATTRIBUTE FOR TYPE %d\n", attrType);
        }
    }

//
// Access general attribute data
//
static void PrintAttribute (STEPattribute* pAttr)
    {
    const char* attrName = pAttr->Name ();
    std::string attrTypeName = pAttr->TypeName ();

    //value as string
    std::string strVal = pAttr->asStr ();
    printf ("    %s type %s value asStr %s\n", attrName, attrTypeName.c_str (), strVal.c_str ());

    //access typed value
    printf ("        typed value: ");
    PrintAttributeByType (pAttr);
    printf ("\n");
    }


//
// Access instance data
//
static void PrintInstance (SDAI_Application_instance* pInst)
    {
    int         stepId    = pInst->StepFileId ();
    const char* className = pInst->EntityName ();

    printf ("#%d=%s\n", stepId, className);

    pInst->ResetAttributes ();
    while (STEPattribute* pAttr = pInst->NextAttribute ())
        {
        PrintAttribute (pAttr);
        }
    }


//
//
//
int main (int argc, char *argv[])
    {
    if (argc != 3)
        {
        printf ("specify file path and sheme name\n");
        exit (1);
        }

    //
    //load cheme and read file
    Registry* registry = LoadScheme (argv[2]); //caller have to delete it
    if (!registry)
        exit (1);

    InstMgr   instance_list;
    STEPfile  sfile (*registry, instance_list, "", false);

    sfile.ReadExchangeFile (argv[1]);

    // check for errors; exit if they are particularly severe
    if (sfile.Error ().severity () < SEVERITY_USERMSG)
        {
        sfile.Error ().PrintContents (cout);
        }
    if (sfile.Error ().severity () <= SEVERITY_INCOMPLETE)
        {
        exit (1);
        }



    //
    // Traverse entities
    //
    printf ("Dump application instances:\n");
    int i = 0;
    while (SDAI_Application_instance* pInst = instance_list.GetApplication_instance (i++))
    //alternative: while (SDAI_Application_instance* pInst = instance_list.GetSTEPentity (i++))
        {
        PrintInstance (pInst);
        }

    }

