---
name: handling-permission-errors
description: |
  Rules for handling permission errors.
triggers:
  - "Permission denied"
  - "Access denied"
  - "EACCES"
  - "401 Unauthorized"
  - "403 Forbidden"
---

# Handling permission errors
When you encounter a permission error, you must follow these rules strictly:

* Abort and report back to the user. Do NOT get creative working around the
  problem.
* Ask the user to supply needed credentials or query the data for you. Do NOT
  try to search for credentials.

This applies to all kinds of permission errors like local filesystem paths
(being inaccessible due to sandboxing) or web pages and APIs (requiring
authentication).
