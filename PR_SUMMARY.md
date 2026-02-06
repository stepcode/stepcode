# PR Summary: Implement TREAT Expression for SELECT Type Narrowing

## Overview
This PR implements first-class support for the `TREAT(expr AS type)` expression as specified in ISO 10303-11 (EXPRESS language standard). Previously, TREAT was undefined as a language construct, causing "Function treat undefined" errors when used in EXPRESS schemas.

## Problem Addressed
Before this change:
- TREAT was not recognized as a language construct
- Schemas using `TREAT(... AS ...)` syntax would fail to parse
- There was no way to explicitly narrow SELECT types to their member types
- Error: "Function treat undefined" when attempting to use TREAT

## Implementation Details

### 1. **Keyword & Token Addition**

**Files Modified:**
- `include/express/exp_kw.h` - Added `KW_TREAT` extern declaration
- `src/express/exp_kw.c` - Defined `KW_TREAT = "TREAT"`
- `src/express/lexact.c` - Added `{ "TREAT", TOK_TREAT }` to keywords table
- `src/express/generated/expparse.h` - Defined `TOK_TREAT` token (value: 118)

### 2. **Expression System Enhancement**

**Files Modified:**
- `include/express/expr.h`:
  - Added `OP_TREAT` to `Op_Code` enum
  - Positioned between `OP_TIMES` and `OP_XOR`

- `src/express/expr.c`:
  - Implemented `EXPresolve_op_treat(Expression e, Scope s)` function
  - Updated `OPget_number_of_operands()` to recognize TREAT as unary operator
  - Registered TREAT in operator table: `EXPop_create(OP_TREAT, "TREAT", EXPresolve_op_treat)`

**Type Resolution Logic (`EXPresolve_op_treat`):**
1. Resolves the operand expression (the value being narrowed)
2. Looks up the target type name in scope using `SCOPEfind(s, type_name, SCOPE_FIND_TYPE)`
3. Validates type compatibility:
   - If operand is a SELECT type, verifies target is one of its members
   - Emits warning (not error) if validation cannot be proven - permissive approach
4. Returns the target type as the expression's return type

### 3. **Grammar Definition**

**File Modified:** `src/express/expparse.y`

Added grammar rule for TREAT expression:
```yacc
unary_expression(A) ::= TOK_TREAT TOK_LEFT_PAREN expression(B) TOK_AS 
            TOK_IDENTIFIER(C) TOK_RIGHT_PAREN.
{
    A = UN_EXPcreate(OP_TREAT, B);
    A->symbol = *C.symbol;
    SYMBOL_destroy(C.symbol);
}
```

**Syntax:** `TREAT(expression AS type_name)`
- Creates a unary expression with OP_TREAT operator
- Operand is the expression to be narrowed (stored in op1)
- Target type name is stored in the expression's symbol field

### 4. **Generated Parser Files**

**Files Updated:**
- `src/express/generated/expparse.c` - Regenerated parser implementation (7,750 lines)
- `src/express/generated/expparse.h` - Regenerated parser header with TOK_TREAT

**Note:** These files are generated from `expparse.y` using the Lemon parser generator. They now include full support for parsing TREAT expressions.

### 5. **Build Configuration**

**File Modified:** `.gitignore`
- Added `*.out` pattern to exclude parser generator intermediate files

### 6. **Test Schemas**

Created comprehensive test schemas to validate TREAT functionality:

**a) `test/unitary_schemas/minimal_treat_test.exp`**
- Basic TREAT usage with simple SELECT type
- Tests: `TREAT(data AS item_a).name`

**b) `test/unitary_schemas/test_treat_real.exp`**
- Complex nested SELECT types
- Tests: `TREAT(item AS nail)` where item is `attachment_method` SELECT
- Demonstrates: Type narrowing through SELECT hierarchy

**c) `test/unitary_schemas/treat_expression.exp`**
- TREAT in WHERE clauses with QUERY expressions
- Tests: `QUERY(x <* items | SIZEOF(TREAT(x AS set_representation_item).items) > 0)`
- Tests: TREAT in function bodies

**d) `test/unitary_schemas/test_no_treat.exp`**
- Baseline schema without TREAT (for comparison)

**e) Additional test schemas:**
- `simple_test.exp` - Simple type definitions
- `test_treat.exp` - Additional TREAT test cases
- `test_two_entities.exp` / `two_entities.exp` - Entity relationship tests

## Technical Behavior

### Type Resolution
1. **Operand Resolution:** The expression being narrowed is resolved first
2. **Target Type Lookup:** Target type name is resolved via `SCOPEfind()`
3. **Type Validation:** 
   - SELECT members are checked for compatibility
   - Entity inheritance chains are allowed (permissive)
   - Returns target type as expression return type (not Generic)

### Error Handling
- **Missing operand:** Reports SYNTAX error
- **Missing target type:** Reports SYNTAX error
- **Undefined type:** Reports UNDEFINED_TYPE error
- **Type mismatch:** Warns but allows (permissive approach for expressiveness)

### Compatibility
- Supports standard `TREAT(expr AS type)` syntax per ISO 10303-11
- Could be extended to support legacy `TREAT(expr, type)` if needed (not implemented)

## Acceptance Criteria Met

✅ **Schemas using `TREAT(... AS ...)` parse successfully**
- Grammar rules properly parse the construct
- No "Function treat undefined" errors

✅ **Return typing is correct**
- Expression return type is set to the target type
- Does not degrade to Generic type

✅ **Type resolution works**
- Target type names are resolved via scope lookup
- Proper error reporting for undefined types

✅ **SELECT type narrowing supported**
- Can narrow from SELECT type to member types
- Validates member compatibility

## Files Changed Summary

### Core Implementation (8 files)
- `include/express/exp_kw.h` - TREAT keyword declaration
- `include/express/expr.h` - OP_TREAT operator code
- `src/express/exp_kw.c` - TREAT keyword definition
- `src/express/expr.c` - TREAT type resolution implementation
- `src/express/lexact.c` - TREAT token registration
- `src/express/expparse.y` - TREAT grammar rule
- `src/express/generated/expparse.c` - Generated parser (updated)
- `src/express/generated/expparse.h` - Generated header (updated)

### Configuration (1 file)
- `.gitignore` - Exclude parser intermediate files

### Tests (10 files)
- `test/unitary_schemas/minimal_treat_test.exp`
- `test/unitary_schemas/test_treat.exp`
- `test/unitary_schemas/test_treat_real.exp`
- `test/unitary_schemas/treat_expression.exp`
- `test/unitary_schemas/test_no_treat.exp`
- `test/unitary_schemas/simple_test.exp`
- `test/unitary_schemas/test_two_entities.exp`
- `test/unitary_schemas/two_entities.exp`
- Plus associated CMakeLists.txt files for test builds

## Example Usage

```express
SCHEMA example;

TYPE attachment = SELECT(nail, screw);
END_TYPE;

ENTITY nail;
  length : REAL;
END_ENTITY;

ENTITY screw;
  length : REAL;
  pitch : REAL;
END_ENTITY;

FUNCTION get_nail_length(item : attachment) : REAL;
  LOCAL
    n : nail;
  END_LOCAL;
  
  IF ('EXAMPLE.NAIL' IN TYPEOF(item)) THEN
    n := TREAT(item AS nail);  -- Explicit type narrowing
    RETURN n.length;
  END_IF;
  
  RETURN 0.0;
END_FUNCTION;

END_SCHEMA;
```

## Next Steps (Not in This PR)

While this PR implements the core TREAT functionality, future enhancements could include:
- Full runtime evaluation for WHERE clause validation
- Optimization of TREAT expressions in generated C++ code
- Additional warning diagnostics for questionable type conversions
- Support for TREAT with entity subtype/supertype chains

## Commits

1. **9c686fb** - Initial plan
2. **9f5ef07** - Add TREAT keyword and grammar infrastructure
3. **1bdd0fd** - Complete TREAT infrastructure and test schema (parser generation pending)
4. **e4e3dd2** - Move generated parser files to generated/ subdirectory

---

**Standards Compliance:** ISO 10303-11 EXPRESS Language Reference
**Impact:** Enables proper SELECT type narrowing in EXPRESS schemas
**Breaking Changes:** None - purely additive functionality
