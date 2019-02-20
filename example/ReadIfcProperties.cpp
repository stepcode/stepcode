//
// ReadStepData.cpp
//
// This examples demonstrate how to read IFC file, enumerate entities of particular class and get dynamic properties of an entity
// using scheme independed libaries
//
// Start looking from 'main' function. 
//

#define _CRT_SECURE_NO_WARNINGS


//MSVC specific
#define SC_SCHEMA_DLL_IMPORTS
#pragma warning (disable:4275)
#pragma warning (disable:4251)


//Link list: stepeditor.lib;stepcore.lib;stepdai.lib;steputils.lib
#include <STEPfile.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <STEPaggregate.h>


// Link with sdai_XXX.lib
extern void SchemaInit (Registry&);

static Registry* LoadScheme ()
    {
    return new Registry (SchemaInit);
    }

//
//
//
static const char* FormatTime (time_t secs)
    {
    long mins = (long)secs / 60;
    secs = secs % 60;

    long hrs = mins / 60;
    hrs = hrs % 60;

    static char buf[256];
    sprintf (buf, "%d:%02d:%02d", hrs, mins, (long)secs);

    return buf;
    }


//
//
//
static SDAI_Application_instance* CountElements (InstMgr& model, const char* className)
    {
    SDAI_Application_instance* entity = NULL;

    time_t timeStart = time (NULL);

    int search_index = 0;
    int i = 0;
    while (SDAI_Application_instance* e = model.GetApplication_instance (className, search_index))
        {
        auto stepId = e->StepFileId ();
        if (!stepId)
            break;
        entity = e;

        i++;

        MgrNode* mnode = model.FindFileId (stepId);
        search_index = model.GetIndex (mnode) + 1;
        }

    time_t timeEnd = time (NULL);
    auto timeSpent = timeEnd - timeStart;
    printf ("***************************************************************\n");
    printf ("Found %d %s %s\n", i, className, FormatTime (timeSpent));

    return entity;
    }

//
//
//
static const char* GetEntityClassName (SDAI_Application_instance* entity)
    {
    const char* className = entity->EntityName ();
    return className;
    }

//
//
//
enum GetAttrResult {GetAttrOK, GetAttrNotExist, GetAttrOtherType, GetAttrNotImpl
    };

//
//
//
static GetAttrResult GetAttrVal (STEPattribute* pAttr, PrimitiveType attrType, void** ppVal)
    {
    PrimitiveType aType = pAttr->Type ();

    switch (aType)
        {
        case sdaiAGGR:
        case ARRAY_TYPE:      // DAS
        case BAG_TYPE:        // DAS
        case SET_TYPE:        // DAS
        case LIST_TYPE:       // DAS
            aType = sdaiAGGR;
            break;

        default:
            aType = pAttr->BaseType ();
        }

    if (attrType != aType)
        {
        return GetAttrOtherType;
        }

    switch (aType)
        {
        case sdaiINTEGER:
            {          
            SDAI_Integer* pInt = pAttr->Integer ();
            *((SDAI_Integer**)ppVal) = pInt;
            return GetAttrOK;
            }

        case sdaiREAL:
            {
            SDAI_Real* pReal = pAttr->Real ();
            *((SDAI_Real**)ppVal) = pReal;
            return GetAttrOK;
            }

        case sdaiBOOLEAN:
            {
            SDAI_BOOLEAN* pBool = pAttr->Boolean ();
            *((SDAI_BOOLEAN**)ppVal) = pBool;
            return GetAttrOK;
            }

        case sdaiLOGICAL:
            {
            SDAI_LOGICAL* pLogical = pAttr->Logical ();
            *((SDAI_LOGICAL**)ppVal) = pLogical;
            return GetAttrOK;
            }

        case sdaiSTRING:
            {
            SDAI_String* pStr = pAttr->String ();
            *((SDAI_String**)ppVal) = pStr;
            return GetAttrOK;
            }

        case sdaiBINARY:
            {
            SDAI_Binary* pBin = pAttr->Binary ();
            *((SDAI_Binary**)ppVal) = pBin;
            return GetAttrOK;
            }

        case sdaiENUMERATION:
            {
            SDAI_Enum* pEnum = pAttr->Enum ();
            *((SDAI_Enum**)ppVal) = pEnum;
            return GetAttrOK;
            }

        case sdaiSELECT:
            {
            SDAI_Select* pSel = pAttr->Select ();
            *((SDAI_Select**)ppVal) = pSel;
            return GetAttrOK;
            }

        case sdaiINSTANCE:
            {
            SDAI_Application_instance* pInst = pAttr->Entity ();
            *((SDAI_Application_instance**)ppVal) = pInst;
            return GetAttrOK;
            }

        case sdaiAGGR:
            {
            STEPaggregate* pAggr = pAttr->Aggregate ();
            *((STEPaggregate**)ppVal) = pAggr;
            return GetAttrOK;
            }

        case sdaiNUMBER:
            {
            SDAI_Real* pNumber = pAttr->Number ();
            *((SDAI_Real**)ppVal) = pNumber;
            return GetAttrOK;
            }

        default:
            printf ("       NOT IMPLEMENTED PRINT ATTRIBUTE FOR TYPE %d\n", attrType);
            return GetAttrNotImpl;
        }
    }

//
//
//
static STEPattribute* GetAttrBN (SDAI_Application_instance* pInst, const char* attrName)
    {
    pInst->ResetAttributes ();
    while (STEPattribute* pAttr = pInst->NextAttribute ())
        {
        const char* aName = pAttr->Name ();
        if (0 == _stricmp (attrName, aName))
            {
            return pAttr;
            }
        }
    return NULL;
    }


//
//
//
static GetAttrResult sdaiGetAttrBN (SDAI_Application_instance* pInst, const char* attrName, PrimitiveType attrType, void** ppVal)
    {
    *ppVal = NULL;

    STEPattribute* pAttr = GetAttrBN (pInst, attrName);

    if (pAttr)
        {
        return GetAttrVal (pAttr, attrType, ppVal);
        }

    return GetAttrNotExist;
    }

//
//
//
static const char* GetEntityIfcName (SDAI_Application_instance* entity)
    {
    SDAI_String* str = NULL;
    if (sdaiGetAttrBN (entity, "name", sdaiSTRING, (void**)&str) == GetAttrOK)
        {
        if (str)
            {
            return str->c_str ();
            }
        }
    return "";
    }

//
//
//
static bool DefinesEntityPropertyes (InstMgr& model, SDAI_Application_instance* rel, SDAI_Application_instance* entity)
    {
    bool found = false;

    STEPaggregate* pRelatedObjects = NULL;
    sdaiGetAttrBN (rel, "RelatedObjects", sdaiAGGR, (void**)&pRelatedObjects);

    if (pRelatedObjects)
        {
        SingleLinkNode* pNode = pRelatedObjects->GetHead ();
        while (!found && pNode)
            {
            if (EntityNode* pEntNode = dynamic_cast<EntityNode*> (pNode))
                {
                if (SDAI_Application_instance* ent = pEntNode->node)
                    {
                    if (ent->StepFileId () == entity->StepFileId ())
                        {
                        //assert (ent == entity);
                        found = true;
                        }
                    }
                }
            pNode = pNode->NextNode ();
            }
        }

    return found;
    }

//
//
//
static void GetPropValType (SDAI_Application_instance* prop, std::string& val, std::string& type)
    {
    STEPattribute* pAttr = GetAttrBN (prop, "NominalValue");
    if (!pAttr)
        {
        return;
        }

    val = pAttr->asStr ();
    type = pAttr->TypeName ();
    }

//
//
//
static void PrintProperty (SDAI_Application_instance* prop)
    {
    auto propClass = GetEntityClassName (prop);
    auto propName = GetEntityIfcName (prop);
    std::string val = "-";
    std::string type = "-";
    GetPropValType (prop, val, type);
    printf ("   %s %s = %s %s\n", propClass, propName, val.c_str (), type.c_str ());
    }

//
//
//
static void PrintProperties (STEPaggregate* pProperties)
    {
    if (pProperties)
        {
        SingleLinkNode* pNode = pProperties->GetHead ();
        while (pNode)
            {
            if (EntityNode* pEntNode = dynamic_cast<EntityNode*> (pNode))
                {
                if (SDAI_Application_instance* ent = pEntNode->node)
                    {
                    PrintProperty (ent);
                    }
                }
            pNode = pNode->NextNode ();
            }
        }
    }

//
//
//
static void PrintPropSet (InstMgr& model, SDAI_Application_instance* pset)
    {
    auto clsName = GetEntityClassName (pset);
    auto psetName = GetEntityIfcName (pset);

    printf ("---\n");
    printf ("%s %s\n", clsName, psetName);

    if (0 == _stricmp ("IfcPropertySet", clsName))
        {
        STEPaggregate* pProperties = NULL;
        sdaiGetAttrBN (pset, "HasProperties", sdaiAGGR, (void**) &pProperties);
        PrintProperties (pProperties);
        }
    }

//
//
//
static void PrintProperties (InstMgr& model, SDAI_Application_instance* entity, const char* clsName)
    {
    printf ("***************************************************************\n");
    int elemId = entity->StepFileId ();
    printf ("properties of %s %s stepId %d\n", clsName, GetEntityIfcName (entity), elemId);

    auto timeStart = time (NULL);

    int search_index = 0;
    while (SDAI_Application_instance* rel = model.GetApplication_instance ("IfcRelDefinesByProperties", search_index))
        {
        auto stepId = rel->StepFileId ();
        if (!stepId)
            break;

        if (DefinesEntityPropertyes (model, rel, entity))
            {
            SDAI_Application_instance* pset = NULL;
            sdaiGetAttrBN (rel, "RelatingPropertyDefinition", sdaiINSTANCE, (void**)&pset);
            PrintPropSet (model, pset);
            }

        MgrNode* mnode = model.FindFileId (stepId);
        search_index = model.GetIndex (mnode) + 1;
        }

    auto timeEnd = time (NULL);
    auto timeSpent = timeEnd - timeStart;
    printf ("---\n");
    printf ("Process properties time %s\n", FormatTime (timeSpent));
    }

//
//
//
static void PropTest (InstMgr& model, const char* className)
    {
    auto entity = CountElements (model, className);
    
    if (entity && entity->StepFileId ())
        {
        PrintProperties (model, entity, className);
        }
        
    }

 
//
//
//
int main (int argc, char *argv[])
    {
    if (argc != 2)
        {
        printf ("specify ifc2x3 file path\n");
        exit (1);
        }
    printf ("Input file %s\n", argv[1]);

    //
    //load cheme and read file
    //
    Registry* registry = LoadScheme (); //caller have to delete it
    if (!registry)
        exit (1);

    InstMgr   instance_list;
    STEPfile  sfile (*registry, instance_list, "", false);

    time_t startTime = time (NULL);

    sfile.ReadExchangeFile (argv[1]);

    time_t endTime = time (NULL);
    printf ("***************************************************************\n");
    printf ("READ TIME: %s\n", FormatTime (endTime - startTime));

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
    // Traverse and get properties for some entities
    //
    startTime = time (NULL);

    PropTest (instance_list, "IfcSite");
    PropTest (instance_list, "IfcBuildingStorey");
    PropTest (instance_list, "IfcBuildingElementProxy");
    PropTest (instance_list, "IfcWall");
    PropTest (instance_list, "IfcSpace");

    endTime = time (NULL);
    printf ("***************************************************************\n");
    printf ("PROCESS TIME: %s\n", FormatTime (endTime - startTime));
    }

