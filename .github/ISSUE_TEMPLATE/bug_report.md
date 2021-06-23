---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug (unverified)
assignees: ''

---

Before submitting an issue, please ensure that you have read the documentation:

* Basics is an overview of how to use simdjson and its APIs: https://github.com/simdjson/simdjson/blob/master/doc/basics.md
* Performance shows some more advanced scenarios and how to tune for them: https://github.com/simdjson/simdjson/blob/master/doc/performance.md
* Contributing: https://github.com/simdjson/simdjson/blob/master/CONTRIBUTING.md
* We follow the [JSON specification as described by RFC 8259](https://www.rfc-editor.org/rfc/rfc8259.txt) (T. Bray, 2017).


**Describe the bug**
A clear and concise description of what the bug is.

Note that a compiler warning is not a bug.

**To Reproduce**
Steps to reproduce the behaviour: provide a code sample if possible.

If we cannot reproduce the issue, then we cannot address it.

Note that a stack trace from your own program is not enough.

**simjson release:**

Unless you plan to contribute to simdjson, you should only work from releases. Please be mindful that our main branch may have additional features, bugs and documentation items.

It is fine to report bugs against our main branch, but if that is what you are doing, please be explicit.

**Configuration (please complete the following information if relevant):**
 - OS: [e.g. Ubuntu 16.04.6 LTS]
 - Compiler [e.g. Apple clang version 11.0.3 (clang-1103.0.32.59) x86_64-apple-darwin19.4.0]
 - Version [e.g. 22]

We support up-to-date 64-bit ARM and x64 FreeBSD, macOS, Windows and Linux systems. Please ensure that your configuration is supported before labelling the issue as a bug. In particular, we do not support legacy 32-bit systems.

**Indicate whether you are willing or able to provide a bug fix as a pull request**

If you plan to contribute to simdjson, please read our
* CONTRIBUTING guide: https://github.com/simdjson/simdjson/blob/master/CONTRIBUTING.md and our
* HACKING guide: https://github.com/simdjson/simdjson/blob/master/HACKING.md
