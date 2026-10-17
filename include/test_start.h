#ifndef GUARD_TEST_START_H
#define GUARD_TEST_START_H

// Emerald Vault: test-only ROM (make TEST_START=1). See docs/MAC_WORKFLOW.md.
void TestStart_WaitForHarness(void);
void TestStart_SetWarp(void);
void TestStart_ApplyPreset(void);

#endif // GUARD_TEST_START_H
