Contributing to Mooncoin Core
=============================

This branch accepts focused contributions that improve Mooncoin node reliability,
build reproducibility, wallet usability, and network safety.

Contribution Scope
------------------

Priority areas:

- Consensus correctness and chain safety
- Node/network reliability and sync behavior
- Build and release reproducibility (Linux/WSL -> Windows)
- Wallet/GUI fixes that do not weaken security assumptions
- Documentation accuracy for operators, miners, and integrators

Workflow
--------

1. Fork and create a topic branch.
2. Keep commits small and reviewable.
3. Include rationale in commit messages.
4. Open a PR against the active Mooncoin branch.
5. Address review feedback and re-test.

PR Naming
---------

Use clear prefixes such as:

- `consensus:`
- `net:` / `p2p:`
- `wallet:`
- `qt:`
- `rpc:` / `zmq:`
- `build:`
- `doc:`
- `test:`

Testing Expectations
--------------------

Before submitting:

- Verify the code builds on your target platform.
- Run relevant unit/functional tests for touched areas.
- For consensus or P2P changes, include explicit test notes.

Review Standard
---------------

Changes with high review cost and low operational value are likely to be deferred.
Consensus changes require stronger justification, wider review, and rollout planning.

Security Disclosure
-------------------

Do not disclose vulnerabilities publicly before coordinated handling.
See `SECURITY.md`.
