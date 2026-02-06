# TREAT Expression Implementation Guide

## Quick Reference for Code Locations

### 1. Keyword Definition
**File:** `src/express/exp_kw.c` (line 108)
```c
char * KW_TREAT = "TREAT";
```

**File:** `include/express/exp_kw.h` (line 111)
```c
extern SC_EXPRESS_EXPORT char * KW_TREAT;
```

### 2. Token Registration
**File:** `src/express/lexact.c` (line 190)
```c
{ "TREAT", TOK_TREAT },
```

**File:** `src/express/generated/expparse.h` (line 118)
```c
#define TOK_TREAT 118
```

### 3. Operator Code
**File:** `include/express/expr.h` (line 86)
```c
typedef enum {
    // ... other operators ...
    OP_TIMES, OP_TREAT, OP_XOR, OP_UNKNOWN,
    OP_LAST
} Op_Code;
```

### 4. Grammar Rule
**File:** `src/express/expparse.y` (around line 2266)
```yacc
unary_expression(A) ::= TOK_TREAT TOK_LEFT_PAREN expression(B) TOK_AS 
            TOK_IDENTIFIER(C) TOK_RIGHT_PAREN.
{
    A = UN_EXPcreate(OP_TREAT, B);
    A->symbol = *C.symbol;
    SYMBOL_destroy(C.symbol);
}
```

### 5. Type Resolution Function
**File:** `src/express/expr.c` (function EXPresolve_op_treat)

**Key Logic:**
```c
Type EXPresolve_op_treat(Expression e, Scope s) {
    // 1. Resolve operand expression
    EXPresolve(e->e.op1, s, Type_Dont_Care);
    operand_type = e->e.op1->return_type;
    
    // 2. Look up target type by name
    target_type = (Type)SCOPEfind(s, e->symbol.name, SCOPE_FIND_TYPE);
    
    // 3. Validate SELECT member compatibility
    if (TYPEis_select(operand_type)) {
        // Check if target is a member of SELECT
    }
    
    // 4. Return target type as expression's return type
    e->return_type = target_type;
    return target_type;
}
```

### 6. Operator Table Registration
**File:** `src/express/expr.c` (in EXPop_init function)
```c
EXPop_create(OP_TREAT, "TREAT", EXPresolve_op_treat);
```

### 7. Operator Operand Count
**File:** `src/express/expr.c` (function OPget_number_of_operands)
```c
static inline int OPget_number_of_operands(Op_Code op) {
    if ((op == OP_NEGATE) || (op == OP_NOT) || (op == OP_TREAT)) {
        return 1;  // TREAT is a unary operator
    }
    // ...
}
```

## Data Flow

```
┌─────────────────────────────────────────────────────────────────┐
│ INPUT: TREAT(item AS nail)                                      │
└─────────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────────┐
│ LEXER (expscan.l / lexact.c)                                    │
│ - Recognizes "TREAT" keyword → TOK_TREAT                        │
│ - Recognizes "AS" keyword → TOK_AS                              │
│ - Recognizes "nail" identifier → TOK_IDENTIFIER                 │
└─────────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────────┐
│ PARSER (expparse.y)                                              │
│ - Matches grammar rule:                                          │
│   TOK_TREAT TOK_LEFT_PAREN expression TOK_AS TOK_IDENTIFIER      │
│   TOK_RIGHT_PAREN                                                │
│ - Creates: UN_EXPcreate(OP_TREAT, operand_expr)                 │
│ - Stores target type name in expression->symbol.name            │
└─────────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────────┐
│ RESOLVER (expr.c - EXPresolve_op_treat)                         │
│ 1. Resolve operand: EXPresolve(e->e.op1, scope, ...)           │
│ 2. Find target type: SCOPEfind(scope, "nail", SCOPE_FIND_TYPE) │
│ 3. Validate: Check if SELECT members compatible                 │
│ 4. Set return type: e->return_type = target_type               │
└─────────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────────┐
│ OUTPUT: Expression with return_type = nail entity type          │
└─────────────────────────────────────────────────────────────────┘
```

## Testing

### Test Schema Locations
All in `test/unitary_schemas/`:

1. **minimal_treat_test.exp** - Simplest case
   ```express
   TREAT(data AS item_a).name
   ```

2. **test_treat_real.exp** - Nested SELECT
   ```express
   TREAT(item AS nail)  -- where item is attachment_method SELECT
   ```

3. **treat_expression.exp** - Complex usage
   ```express
   QUERY(x <* items | SIZEOF(TREAT(x AS set_representation_item).items) > 0)
   ```

## Building

The parser is generated from `expparse.y` using Lemon parser generator:
```bash
# Grammar source
src/express/expparse.y

# Generated files (committed to repo)
src/express/generated/expparse.c
src/express/generated/expparse.h
```

## Error Messages

| Error | Condition | Location |
|-------|-----------|----------|
| "TREAT expression missing operand" | No operand provided | EXPresolve_op_treat |
| "TREAT expression missing target type" | No type name | EXPresolve_op_treat |
| "Undefined type: [name]" | Target type not found | EXPresolve_op_treat |

## Standards Compliance

Implements ISO 10303-11 Section 12.10 (TREAT expression):
- Syntax: `TREAT ( expression AS type_name )`
- Purpose: Explicit type narrowing/casting
- Usage: Primarily with SELECT types

