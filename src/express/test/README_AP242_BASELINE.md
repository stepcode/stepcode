# AP242 PE056 Baseline Test Documentation

## Purpose

This test captures the PE056 error that occurs when parsing the AP242 schema file (`data/ap242/242_mim_lf.exp`). It serves as a baseline regression test to ensure we can pinpoint exactly where the error is triggered.

## Error Details

- **Error Code**: PE056 (QUERY_REQUIRES_AGGREGATE)
- **Error Message**: "Query expression source must be an aggregate."
- **Location**: Line 34148 in `data/ap242/242_mim_lf.exp`
- **Context**: `styled_item` entity, WHERE rule WR3
- **Expression**: `QUERY(it <* item | ...)` where `item` is not being recognized as an aggregate

## Running the Test

### Quick Local Run (Recommended)

After building the project, you can run the test directly:

```bash
cd build
./bin/check-express ../data/ap242/242_mim_lf.exp
```

Expected output should include:
```
../data/ap242/242_mim_lf.exp:34148: --ERROR PE056: Query expression source must be an aggregate.
Errors in input
```

### Via CTest

```bash
cd build
ctest -R test_ap242_pe056_baseline -V
```

Or to run with labels:
```bash
ctest -L ap242 -V
ctest -L baseline -V
```

## Test Behavior

### Current State (Before Fix)
- The test **expects** check-express to fail with PE056 error
- CTest reports: `PASSED` (because the expected failure occurred)
- This is controlled by the `WILL_FAIL true` property in CMakeLists.txt

### After Fix (Future)
- Once the PE056 issue is resolved, the test property should be updated:
  - Change `WILL_FAIL true` to `WILL_FAIL false` (or remove the property)
- The test will then verify that AP242 parses successfully without PE056 errors

## Performance

- Test execution time: ~0.17 seconds (very fast)
- This is a focused parse/resolve test - no full schema build required
- No code generation or compilation needed

## Integration with CI

The test is labeled with:
- `parser` - Groups it with other parser tests
- `ap242` - Identifies it as AP242-specific
- `baseline` - Marks it as a baseline regression test

## Configuration Notes

To build and run this test:
1. Configure with testing enabled: `cmake .. -DSC_ENABLE_TESTING=ON`
2. Build the check-express tool: `cmake --build . --target check-express`
3. Run the test as shown above

No schema building is required for this test, so you can use:
```bash
cmake .. -DSC_ENABLE_TESTING=ON -DSC_BUILD_SCHEMAS=""
```

This keeps the configuration fast and focused on parser testing.
