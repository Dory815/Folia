'use strict';

// The native generator materializes Source's visible `for..in` SMP rows into
// each requested variant's remap map. This is intentionally based on the
// serialized visible table, not the JSON-own-property `ownChars` subset.
function expectedEffectiveRemaps(variantName, visibleChars) {
  const remaps = {};
  for (const [codepoint, tuple] of Object.entries(visibleChars || {})) {
    const target = tuple?.[3]?.smp;
    if (Number.isInteger(target))
      remaps[codepoint] = {variant: variantName, codepoint: target};
  }
  return remaps;
}

module.exports = {expectedEffectiveRemaps};
