# AP242 styled_item Test

## Purpose

This test validates that the STEPcode-generated C++ code from both AP242 schemas can correctly handle `styled_item` entities, particularly the WHERE rule WR3 that was problematic in the original schema.

Two tests are provided:
1. **Vanilla AP242** (`ap242_vanilla_styled_item`) - Uses flow-sensitive type narrowing (STEPcode extension)
2. **AP242 TREAT** (`ap242_styled_item`) - Uses explicit TREAT expression (ISO 10303-11 compliant)

## Background

The AP242 schema contains a `styled_item` entity with a complex WHERE rule (WR3) that requires type narrowing from a SELECT type to an aggregate type:

```express
ENTITY styled_item
  SUBTYPE OF (representation_item);
  styles : SET [0 : ?] OF presentation_style_assignment;
  item   : styled_item_target;
WHERE
  WR3: ('AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF.MAPPED_ITEM' IN TYPEOF(item)) OR
       ('AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF.GEOMETRIC_REPRESENTATION_ITEM' IN TYPEOF(item)) OR
       (('AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF.SET_REPRESENTATION_ITEM' IN TYPEOF(item)) AND
        (SIZEOF(QUERY(it
                      <* TREAT(item AS set_representation_item)  -- TREAT in ap242treat
                      <* item                                     -- flow-sensitive in vanilla ap242
                      | NOT (('AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF.MAPPED_ITEM' IN TYPEOF(it)) OR
                             ('AP242_MANAGED_MODEL_BASED_3D_ENGINEERING_MIM_LF.GEOMETRIC_REPRESENTATION_ITEM' IN
                              TYPEOF(it))))) =
         0));
END_ENTITY;
```

### Vanilla AP242 (data/ap242/242_mim_lf.exp)
Uses **flow-sensitive type narrowing** - STEPcode recognizes the TYPEOF guard pattern and automatically narrows the type:
```express
QUERY(it <* item | ...)  -- item is implicitly narrowed to set_representation_item
```

### AP242 TREAT (data/ap242treat/242_mim_lf_treat.exp)
Uses **explicit TREAT expression** - ISO 10303-11 compliant approach:
```express
QUERY(it <* TREAT(item AS set_representation_item) | ...)  -- explicit type narrowing
```

Both approaches produce identical, correct output.

## Test Files

- `test_ap242_styled_item.stp` - Minimal AP242 STEP file containing styled_item instances (used by both tests)
- `test/cpp/schema_specific/ap242_vanilla_styled_item.cc` - C++ test for vanilla AP242 (flow-sensitive narrowing)
- `test/cpp/schema_specific/ap242_styled_item.cc` - C++ test for AP242 TREAT (explicit TREAT expression)

## Test Implementation

Both tests:
1. Read the same AP242 STEP file containing styled_item entities
2. Parse it using their respective generated libraries (libsdai_242_mim_lf.so or libsdai_ap242treat.so)
3. Verify that styled_item instances are correctly parsed and accessible
4. Validate the entity relationships and attributes

The key difference is which schema/library is used, demonstrating that both approaches handle the same data correctly.

## Running the Tests

```bash
# Build both AP242 schemas and tests
cd build
ninja sdai_242_mim_lf      # vanilla AP242 schema
ninja sdai_ap242treat      # TREAT AP242 schema
ninja tst_ap242_vanilla_styled_item
ninja tst_ap242_styled_item

# Run the vanilla AP242 test (flow-sensitive narrowing)
./bin/tst_ap242_vanilla_styled_item ../test/p21/test_ap242_styled_item.stp

# Run the TREAT AP242 test (explicit TREAT)
./bin/tst_ap242_styled_item ../test/p21/test_ap242_styled_item.stp
```

Or via CTest:
```bash
ctest -R ap242.*styled_item -V
```

## Success Criteria

For both tests:
- The STEP file parses without errors
- styled_item entities are found and accessible
- The generated C++ code correctly handles the WR3 WHERE rule (with flow-sensitive narrowing or TREAT)
- No crashes or segmentation faults occur

## Related Documentation

- `data/ap242/242_mim_lf.exp` - Vanilla AP242 schema (flow-sensitive narrowing)
- `data/ap242treat/README.md` - AP242 TREAT schema documentation
- `doc/ap242-comparison.md` - Comparison of flow-sensitive narrowing vs TREAT
- Problem statement: ENTITY styled_item with type narrowing support
