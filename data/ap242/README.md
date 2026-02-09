# AP242 Schema Files

This directory contains two versions of the AP242 (ISO 10303-242) schema.

## Files

### `242_mim_lf.exp` (Original)
- **Status**: Original ISO schema with non-standard parsing requirement
- **Size**: 54,756 lines
- **Parsing**: Requires STEPcode's flow-sensitive type narrowing extension
- **Standards**: Contains pattern that needs implicit type narrowing (non-standard)

### `242_mim_lf_treat.exp` (Standards-Compliant)
- **Status**: Corrected version using TREAT expression
- **Size**: 54,756 lines
- **Parsing**: Fully ISO 10303-11 compliant
- **Standards**: Uses explicit TREAT expression for type narrowing

## The Difference

Both files are identical except for a single line at line 34149 in the `styled_item` entity WHERE rule WR3:

**Original (242_mim_lf.exp)**:
```express
QUERY(it <* item | ...)
```

**TREAT version (242_mim_lf_treat.exp)**:
```express
QUERY(it <* TREAT(item AS set_representation_item) | ...)
```

## Why Two Versions?

### Historical Context
The original AP242 schema contains a WHERE rule that:
1. Checks if `item` (a SELECT type) is a `SET_REPRESENTATION_ITEM`
2. Then tries to iterate over `item` in a QUERY expression

This pattern requires implicit type narrowing, which is not part of the ISO 10303-11 standard.

### STEPcode's Solution
STEPcode implements **flow-sensitive type narrowing** as an extension:
- Recognizes TYPEOF guard patterns like `'Type' IN TYPEOF(var)` in AND expressions
- Automatically narrows the type for subsequent uses
- Makes the original schema parse successfully

### Standards-Compliant Alternative
The TREAT version uses the ISO 10303-11 TREAT expression:
- `TREAT(item AS set_representation_item)` explicitly narrows the type
- No implicit narrowing needed
- Works in any compliant EXPRESS parser

## Testing

Both versions parse successfully and produce identical output:

```bash
# Run comparison test
./test/compare_ap242_versions.sh
```

Expected output:
```
✓ Both versions parse successfully
✓ Both versions generate the same number of files
```

For detailed comparison, see `doc/ap242-comparison.md`.

## Which Version to Use?

- **For STEPcode**: Both work equally well
- **For portability**: Use the TREAT version (`242_mim_lf_treat.exp`)
- **For ISO compliance**: Use the TREAT version (`242_mim_lf_treat.exp`)
- **For historical reference**: The original is preserved as `242_mim_lf.exp`

## Related Documentation

- `doc/ap242-comparison.md` - Detailed comparison and analysis
- `doc/express-type-unwrapping.md` - TREAT and narrowing documentation
- `src/express/test/README_AP242_BASELINE.md` - Historical PE056 error info
