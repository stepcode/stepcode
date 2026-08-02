/*****************************************************************************
 * write.cc                                                                  *
 *                                                                           *
 * Description: Contains the write() functions through which the complex     *
 *              structures can write themselves to file.  More specifically, *
 *              the structures generate code in an output file which can be  *
 *              used to recreate the same structures.                        *
 *                                                                           *
 * Created by:  David Rosenfeld                                              *
 * Date:        01/09/97                                                     *
 *****************************************************************************/

#include <string>
#include <vector>

#include "complexSupport.h"

// Local function prototypes:
static void writeheader( ostream &, int );

struct CompactComplexNode {
    JoinType kind;
    std::string name;
    size_t firstChild;
    size_t nextSibling;
};

static size_t appendCompactNode( EntList * node,
                                 std::vector<CompactComplexNode> & nodes ) {
    const size_t noNode = static_cast<size_t>( -1 );
    const size_t index = nodes.size();
    CompactComplexNode record = { node->join, "", noNode, noNode };
    if( node->join == SIMPLE ) {
        record.name = static_cast<SimpleList *>( node )->Name();
    }
    nodes.push_back( record );

    if( node->multiple() ) {
        MultList * parent = static_cast<MultList *>( node );
        size_t previous = noNode;
        for( int i = 0; i < parent->childCount(); ++i ) {
            size_t child = appendCompactNode( parent->getChild( i ), nodes );
            if( previous == noNode ) {
                nodes[index].firstChild = child;
            } else {
                nodes[previous].nextSibling = child;
            }
            previous = child;
        }
    }
    return index;
}

static const char * compactNodeKind( JoinType kind ) {
    switch( kind ) {
        case AND:
            return "ComplexNodeInit_And";
        case OR:
            return "ComplexNodeInit_Or";
        case ANDOR:
            return "ComplexNodeInit_AndOr";
        case SIMPLE:
        default:
            return "ComplexNodeInit_Simple";
    }
}

static void writeCompactComplex( ostream & output, ComplexCollect & collect ) {
    const size_t noNode = static_cast<size_t>( -1 );
    std::vector<CompactComplexNode> nodes;
    std::vector<size_t> roots;
    for( ComplexList * list = collect.clists; list; list = list->next ) {
        roots.push_back( appendCompactNode( list->head, nodes ) );
    }

    output << "// compact, table-driven API v2 complex metadata\n"
           << "#include \"clstepcore/schemaInit.h\"\n\n"
           << "namespace {\n"
           << "const size_t noNode = static_cast<size_t>( -1 );\n"
           << "const ComplexNodeInitRecord complexNodes[] = {\n";
    if( nodes.empty() ) {
        output << "    { ComplexNodeInit_Simple, 0, noNode, noNode },\n";
    }
    for( size_t i = 0; i < nodes.size(); ++i ) {
        output << "    { " << compactNodeKind( nodes[i].kind ) << ", ";
        if( nodes[i].kind == SIMPLE ) {
            output << '"' << nodes[i].name << '"';
        } else {
            output << '0';
        }
        output << ", ";
        if( nodes[i].firstChild == noNode ) {
            output << "noNode";
        } else {
            output << nodes[i].firstChild;
        }
        output << ", ";
        if( nodes[i].nextSibling == noNode ) {
            output << "noNode";
        } else {
            output << nodes[i].nextSibling;
        }
        output << " },\n";
    }
    output << "};\n\nconst ComplexListInitRecord complexLists[] = {\n";
    if( roots.empty() ) {
        output << "    { 0 },\n";
    }
    for( size_t i = 0; i < roots.size(); ++i ) {
        output << "    { " << roots[i] << " },\n";
    }
    output << "};\n}\n\nComplexCollect * gencomplex() {\n"
           << "    return InitializeComplexSupport( complexNodes, "
           << nodes.size() << ", complexLists, " << roots.size() << " );\n"
           << "}\n";
}

void print_complex( ComplexCollect & collect, const char * filename,
                    bool compact )
/*
 * Standalone function called from exp2cxx.  Takes a ComplexCollect
 * and writes its contents to a file (filename) which can be used to
 * recreate this CCollect.
 */
{
#ifdef COMPLEX_INFO
    ComplexList * cl;
    if( collect.clists ) {
        // If there's something in this collect, print it out:
        cout << "\nHere's everything:\n";
        for( cl = collect.clists; cl != NULL; cl = cl->next ) {
            cout << *cl << endl;
        }
    }
#endif
    collect.write( filename, compact );
}

void ComplexCollect::write( const char * fname, bool compactOutput )
/*
 * Generates C++ code in os which may be compiled and run to create a
 * ComplexCollect structure.  Functions are called to write out the
 * ComplexList structures contained in this.
 */
{
    ofstream complex;
    ComplexList * clist;
    int maxlevel, listmax;

    // Open the stream:
    complex.open( fname );
    if( !complex ) {
        cerr << "ERROR: Could not create output file " << fname << endl;
        // yikes this is pretty drastic, Sun C++ doesn't like this anyway DAS
//  exit(-1);
        return;
    }
    if( compactOutput ) {
        writeCompactComplex( complex, *this );
        complex.close();
        return;
    }
    writeheader( complex, clists == NULL );

    // If there's nothing in this, make function a stub (very little was
    // printed in writeheader() also):
    if( clists == NULL ) {
        complex << "    return 0;" << endl;
        complex << "}" << endl;
        complex.close();
        return;
    }

    // First write to os the variables it will need:
    complex << "    ComplexCollect *cc;\n";
    complex << "    ComplexList *cl;\n";
    complex << "    EntList *node, *child;\n";

    // Determine maximum EntList level among all lists so we know how large
    // of an array to create.
    maxlevel = 0;
    clist = clists;
    while( clist ) {
        listmax = clist->getEntListMaxLevel();
        if( listmax > maxlevel ) {
            maxlevel = listmax;
        }
        clist = clist->next;
    }

    complex << "    EntList *next[" << maxlevel + 1 << "];\n\n";

    // Next create the CCollect and CLists:
    complex << "    cc = new ComplexCollect;\n";
    clist = clists;
    while( clist ) {
        complex << endl;
        complex << "    // ComplexList with supertype \"" << clist->supertype()
                << "\":\n";
        clist->write( complex );
        complex << "    cc->insert( cl );\n";
        clist = clist->next;
    }

    // Close up:
    complex << "\n    return cc;\n";
    complex << "}" << endl;
    complex.close();
}

static void writeheader( ostream & os, int noLists )
/*
 * Writes the header for the complex file.
 */
{
    // If there are no ComplexLists in the ComplexCollect, make this function
    // a stub:
    if( noLists ) {
        os << "/*" << endl
           << " * This file normally contains instantiation statements to\n"
           << " * create complex support structures.  For the current EXPRESS\n"
           << " * file, however, there are no complex entities, so this\n"
           << " * function is a stub.\n"
           << " */" << endl << endl;
        os << "#include \"clstepcore/complexSupport.h\"\n\n";
        os << "ComplexCollect *gencomplex()" << endl;
        os << "{" << endl;
        return;
    }

    // Otherwise, write file and function comments:
    os << "/*" << endl
       << " * This file contains instantiation statements to create complex\n"
       << " * support structures.  The structures will be used in the SCL to\n"
       << " * validate user requests to instantiate complex entities.\n"
       << " */" << endl << endl;
    os << "#include \"clstepcore/complexSupport.h\"\n\n";
    os << "ComplexCollect *gencomplex()" << endl;
    os << "    /*" << endl
       << "     * This function contains instantiation statements for all the\n"
       << "     * ComplexLists and EntLists in a ComplexCollect.  The instan-\n"
       << "     * stiation statements were generated in order of lower to\n"
       << "     * higher, and last to first to simplify creating some of the\n"
       << "     * links between structures.  Because of this, the code is not\n"
       << "     * very readable, but does the trick.\n"
       << "     */" << endl;
    os << "{" << endl;
}

void ComplexList::write( ostream & os )
/*
 * Generates C++ code in os which will create an instantiation of a CList
 * which will recreate this.
 */
{
    head->write( os );
    os << "    cl = new ComplexList((AndList *)node);\n";
    os << "    cl->buildList();\n";
    os << "    cl->head->setLevel( 0 );\n";
}

void MultList::write( ostream & os )
/*
 * Writes to os code to instantiate a replica of this.  Does so by first
 * recursing to replicate this's children, and then instantiating this.
 * When write() is finished, the "node" variable in os will point to this.
 */
{
    EntList * child = getLast();

    // First write our children, from last to first.  (We go in backwards order
    // so that "node" (a variable name in the os) will = our first child when
    // this loop is done.  See below.)
    child->write( os );
    while( child->prev ) {
        // Whenever an EntList::write() function is called, it writes to os
        // an instantiation statement basically of the form "node = new XXX-
        // List;".  So we know that in the output file (os) the newly-created
        // EntList is pointed to by variable node.
        os << "    next[" << level + 1 << "] = node;\n";
        child = child->prev;
        child->write( os );
        os << "    next[" << level + 1 << "]->prev = node;\n";
        os << "    node->next = next[" << level + 1 << "];\n";
    }

    // Now write out this:
    os << "    child = node;\n";
    // "node" was set to the last child list we just added (which is the first
    // of our children).  Now we set the variable "child" to it so we can reset
    // node.  We do this so that node will = this when we're done and return
    // to the calling function (so the calling fn can make the same assumption
    // we just did).
    if( join == AND ) {
        os << "    node = new AndList;\n";
    } else if( join == ANDOR ) {
        os << "    node = new AndOrList;\n";
    } else {
        os << "    node = new OrList;\n";
    }
    os << "    ((MultList *)node)->appendList( child );\n";
    // The above line will set node's childList and numchidren count.
}

void SimpleList::write( ostream & os )
/*
 * Writes to os a statement to instantiate this.
 */
{
    os << "    node = new SimpleList( \"" << name << "\" );\n";
}
