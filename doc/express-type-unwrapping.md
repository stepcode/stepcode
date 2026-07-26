# EXPRESS Type Unwrapping and Narrowing

## Overview

This document describes the type unwrapping and narrowing features implemented in STEPcode's EXPRESS compiler. These features enable proper type checking and semantic analysis for aggregate types and SELECT type narrowing.

**Note:** Some features described here are extensions beyond the ISO 10303-11 standard and are explicitly marked as such.

## Deep Aggregate Unwrapping

### Background

In EXPRESS, aggregate types (ARRAY, LIST, SET, BAG) can be nested and wrapped in type definitions. For semantic analysis and type checking, the compiler often needs to "unwrap" these layers to access the underlying element type.

### Shallow vs Deep Type Checking

#### Shallow Aggregate Check: `TYPEis_aggregate`

The macro `TYPEis_aggregate(t)` performs a **shallow** check by testing if `t->u.type->body->base` is non-NULL. This indicates the type has a base type, which is characteristic of aggregate types.

```c
#define TYPEis_aggregate(t) ((t)->u.type->body->base)
```

**Important:** This is a shallow check and does not follow type definition chains. It returns true if the immediate type structure has a base type.

**Use when:**
- You need a quick check if a type is directly an aggregate
- You're working with the immediate type structure
- Performance is critical and deep unwrapping isn't needed

#### Deep Aggregate Unwrapping: `TYPEget_nonaggregate_base_type`

The function `TYPEget_nonaggregate_base_type(Type t)` performs **deep unwrapping** by iteratively following the chain of aggregate base types until reaching a non-aggregate type.

```express
TYPE int_list = LIST OF INTEGER;
TYPE named_int_list = int_list;  -- typedef chain
TYPE set_of_lists = SET OF named_int_list;
```

For `set_of_lists`:
- `TYPEis_aggregate()` → true (immediate check)
- `TYPEget_nonaggregate_base_type()` → INTEGER (unwraps through SET → LIST → INTEGER)

**Use when:**
- Comparing element types across different aggregate wrappers
- Type resolution requires the ultimate element type
- Semantic checking needs to validate deep type compatibility

### Deep Unwrapping Implementation

```c
Type TYPEget_nonaggregate_base_type( Type t ) {
    while( TYPEis_aggregate( t ) ) {
        t = t->u.type->body->base;
    }
    return t;
}
```

This function safely handles:
- Nested aggregates (SET OF LIST OF ...)
- Type definition chains
- Mixed aggregate types

### Aggregate Type Comparison: `TYPE_retrieve_aggregate`

For SELECT types containing multiple aggregate members, `TYPE_retrieve_aggregate()` validates that all aggregate members share the same element type.

```express
TYPE item_a = SET OF INTEGER;
TYPE item_b = LIST OF INTEGER;
TYPE my_select = SELECT(item_a, item_b);  -- Valid: both have INTEGER elements
```

The function:
1. Iterates through SELECT members
2. Identifies which are aggregates
3. Unwraps typedef chains to find element types
4. Returns common element type or NULL if incompatible

## TREAT Expression Semantics

### ISO 10303-11 Standard Syntax

The TREAT expression is defined in ISO 10303-11 as:

```express
TREAT(expression AS type_identifier)
```

**Purpose:** Explicitly narrow a SELECT type to one of its member types.

### STEPcode Implementation

STEPcode implements TREAT as specified in ISO 10303-11 with the following semantics:

#### Type Resolution

1. **Operand Resolution:** The expression being narrowed is resolved first
   ```c
   EXPresolve( e->e.op1, s, Type_Dont_Care );
   operand_type = e->e.op1->return_type;
   ```

2. **Target Type Lookup:** The target type identifier is resolved via scope lookup
   ```c
   target_type = (Type)SCOPEfind( s, e->symbol.name, SCOPE_FIND_TYPE );
   ```

3. **Type Compatibility:** The implementation validates that narrowing is semantically valid
   - For SELECT operands: Verifies target is a member type
   - For other types: Allows permissively (may emit warnings)

4. **Return Type:** The expression's return type becomes the target type
   ```c
   return target_type;  /* Not Type_Generic */
   ```

#### Error Handling

**Clear errors with location information:**

- **Missing operand:** `SYNTAX` error - "TREAT expression missing operand"
- **Missing target type:** `SYNTAX` error - "TREAT expression missing target type"  
- **Undefined type:** `UNDEFINED_TYPE` error with the type name
- **Type mismatch:** Currently permissive (no error, future: may warn)

All errors use `ERRORreport_with_symbol()` which provides:
- File name and line number
- Symbol context
- Descriptive error message

#### Example Usage

```express
TYPE attachment = SELECT(nail, screw);
END_TYPE;

FUNCTION get_length(item : attachment) : REAL;
  LOCAL
    n : nail;
  END_LOCAL;
  
  IF ('EXAMPLE.NAIL' IN TYPEOF(item)) THEN
    n := TREAT(item AS nail);  -- Narrows SELECT to nail
    RETURN n.length;
  END_IF;
  
  RETURN 0.0;
END_FUNCTION;
```

### Grammar Integration

The TREAT syntax is integrated into the EXPRESS parser grammar as a unary expression:

```yacc
unary_expression ::= TOK_TREAT TOK_LEFT_PAREN expression TOK_AS 
                     TOK_IDENTIFIER TOK_RIGHT_PAREN
```

Key points:
- `TOK_TREAT`: Recognized as a keyword
- Creates unary expression with `OP_TREAT` operator
- Operand stored in `op1`
- Target type name stored in `symbol` field

## Flow-Sensitive SELECT Narrowing

**Extension Status:** This is a **non-standard extension** that enhances type safety for SELECT types.

### Background

EXPRESS WHERE clauses often use TYPEOF guards to check SELECT types before accessing member-specific attributes. Flow-sensitive narrowing automatically narrows SELECT types within the scope of such guards.

### Pattern Recognition

The narrowing system recognizes this pattern:

```express
'TypeName' IN TYPEOF(variable)
```

When this pattern appears in an AND conjunction, the variable is narrowed to `TypeName` for subsequent expressions in the conjunction.

### Implementation Architecture

#### Refinement Context

A **refinement context** tracks active type narrowings during expression resolution:

```c
typedef struct RefinementContext_ {
    Refinement refinements;        /* Linked list of variable → type mappings */
    struct RefinementContext_ *previous;  /* Stack of contexts */
} *RefinementContext;

RefinementContext active_refinements = NULL;  /* Thread-local global */
```

#### Pattern Matching

The function `match_typeof_guard()` identifies guard patterns:

1. Expression is `IN` operator
2. Left operand is string literal (type name)
3. Right operand is `TYPEOF(identifier)`
4. Identifier resolves to a variable

#### Refinement Collection

The function `collect_refinements_from_conjunction()` recursively walks AND expressions:

1. Recursively descends into AND nodes
2. Matches TYPEOF guard patterns at leaves
3. Validates narrowing is semantically correct:
   - Variable must have SELECT type
   - Target type must be SELECT member (validated via `is_select_member()`)
4. Returns linked list of valid refinements

#### Application

In `EXPresolve_op_and()`:

```c
Type EXPresolve_op_and(Expression e, Scope s) {
    EXPresolve(e->e.op1, s, Type_Dont_Care);  /* Resolve LHS normally */
    
    Refinement refs = collect_refinements_from_conjunction(e->e.op1, s);
    
    if (refs) {
        /* Create temporary context */
        struct RefinementContext_ ctx;
        ctx.refinements = refs;
        ctx.previous = active_refinements;
        active_refinements = &ctx;
        
        EXPresolve(e->e.op2, s, Type_Dont_Care);  /* Resolve RHS with narrowing */
        
        /* Restore previous context */
        active_refinements = ctx.previous;
        free_refinements(refs);
    } else {
        EXPresolve(e->e.op2, s, Type_Dont_Care);
    }
    
    return Type_Logical;
}
```

### Behavior Scope

**What is narrowed:**
- Variables with SELECT type
- Within RHS of AND expressions
- Where LHS contains TYPEOF guard

**What is NOT narrowed:**
- OR expressions (conservative - doesn't extract from disjunction)
- NOT expressions (conservative - doesn't extract from negation)
- Other boolean operators
- Variables not guarded by TYPEOF

**Rationale:** Conservative approach prevents false narrowing in cases where the guard may not hold.

### Limitations

1. **Single-level context:** Only immediate AND RHS is narrowed, not deeply nested scopes
2. **No cross-statement narrowing:** Limited to expression-level scope
3. **AND-only:** Does not propagate through OR/NOT (conservative)
4. **Pattern-based:** Only recognizes exact `'Type' IN TYPEOF(var)` pattern

### Example

```express
ENTITY test_entity;
  item : target;  -- target is SELECT(set_representation_item, simple_int)
WHERE
  wr1: ('SCHEMA.SET_REPRESENTATION_ITEM' IN TYPEOF(item)) AND 
       (SIZEOF(item) > 0);  -- item narrowed to set_representation_item
END_ENTITY;
```

Without narrowing, `SIZEOF(item)` would fail (can't take SIZEOF of SELECT). With narrowing, `item` is known to be an aggregate, so `SIZEOF` is valid.

## Memory Management

### Refinement Lifecycle

1. **Collection:** `collect_refinements_from_conjunction()` allocates refinement nodes
2. **Usage:** Active during RHS expression resolution only
3. **Cleanup:** `free_refinements()` called after RHS resolution completes

### Context Stack Model

The implementation uses a **stack-based context model** rather than deep copying:

- `RefinementContext` has `previous` pointer forming a stack
- New contexts pushed onto stack with `active_refinements = &ctx`
- Restored with `active_refinements = ctx.previous`
- No deep copying of refinement data

**Memory safety:**
- Refinement nodes allocated individually
- Freed immediately after use
- No dangling pointers (stack-local contexts)
- No memory leaks (verified by code inspection)

## Testing

### Test Coverage

**TREAT expression tests:**
- `test/unitary_schemas/minimal_treat_test.exp` - Basic TREAT usage
- `test/unitary_schemas/test_treat_real.exp` - Nested SELECT types
- `test/unitary_schemas/treat_expression.exp` - TREAT in WHERE clauses

**Flow-sensitive narrowing tests:**
- `test/unitary_schemas/flow_narrowing_test.exp` - AND-based narrowing
- `test/unitary_schemas/select_aggregate_test.exp` - SELECT of aggregates

**Production schema comparison:**
- `data/ap242/242_mim_lf.exp` - Original AP242 with flow-sensitive narrowing
- `data/ap242/242_mim_lf_treat.exp` - TREAT-based standards-compliant version
- `test/compare_ap242_versions.sh` - Automated comparison script

See `doc/ap242-comparison.md` for detailed analysis of the AP242 schema comparison.

### AP242 Compatibility

The implementation is validated against the AP242 schema, ensuring:
- Large-scale schema parsing succeeds
- Complex type hierarchies handled correctly
- Production schema compatibility

**AP242 Schema Versions:**
- `data/ap242/242_mim_lf.exp` - Original schema using flow-sensitive narrowing extension
- `data/ap242/242_mim_lf_treat.exp` - Standards-compliant version using TREAT expression

Both versions parse successfully and produce identical output. See `doc/ap242-comparison.md` for detailed analysis.

## Standards Compliance Notes

### ISO 10303-11 Compliance

**Fully compliant:**
- TREAT expression syntax and semantics
- Aggregate type definitions
- SELECT type definitions
- Standard type resolution rules

**Extensions (non-standard):**
- Flow-sensitive type narrowing in AND expressions
- Permissive TREAT type checking (warns vs errors)

### Extension Philosophy

STEPcode's extensions follow these principles:

1. **Conservative:** Only apply when semantically safe
2. **Permissive:** Prefer warnings over errors where reasonable
3. **Explicit:** Document extensions clearly
4. **Compatible:** Don't break standard-compliant schemas

Extensions are designed to enhance usability while maintaining backward compatibility with ISO 10303-11 compliant EXPRESS schemas.
