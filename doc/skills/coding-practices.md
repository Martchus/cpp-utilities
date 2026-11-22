---
name: coding-practices
description: |
  Coding practices for my projects.
triggers:
  - c++utilities
  - qtutilities
  - qtforkawesome
  - syncthingtray
  - passwordfile
  - passwordmanager
  - tagparser
  - tageditor
  - auto-makepkg
  - pianobooster
---

# C++ coding practices
Follow these coding practices when writing code for my projects:

* Use modern C++. All features of C++ 23 are ok to use by default. Features of
  newer standards might also be ok if well-supported or already in use anyway.
  Otherwise, ask whether a certain feature should be used or avoided.
    * Use `auto` which should be possible on most variable declarations.
        * Use e.g. `auto i = int();` or `auto h = HWND();` (and NOT e.g.
          `int i = 0;`).
    * Use `std::string_view` where it makes sense.
    * Use `constexpr` where it makes sense.
* Avoid repetitions.
    * Avoid repeating the same preprocessor conditions. Instead, add a `#define`
      and test for that.
* Error handling:
    * Handle all errors by default.
    * Be explicit (e.g. in a code comment) if an error can really be ignored
      (which is usually the exception).
    * Document if a function throws exceptions (directly or by letting an
      exception of a nested function call unhandled) via a Doxygen `\throws`
      section.
    * Prefer use of exceptions to return errors (unless it does not make sense
      in the current context).
* Prefer using constructors/destructors over manual resource management
  (especially when using C APIs).
* Comments:
    * Write comments like `// do this` and `// do that`.
    * Do NOT include numbers (so do NOT write comments like `// 1. do this`).
    * Avoid overuse of comments; do not describe the obvious.
    * Focus on the *why* (and NOT on the *what*). (You may describe the *what*
      if it is not obvious or counterintuitive, though.)
* Add Doxygen documentation for new functions/classes/…; extend/update
  documentation of existing functions/classes/….
  * Add at least a short `\brief`.
  * Document exceptions via `\throws`.
  * For internal, small and self-explanatory functions: You may omit
    documentation for those. Wrap them inside `/// \cond` and `/// \endcond` to
    exclude them from the generated documentation completely.
* Wrap compiler/vendor/platform-specific code in preprocessor conditions.
