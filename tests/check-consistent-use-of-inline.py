#!/usr/bin/python3
"""
check-consistent-use-of-inline.py

Scans C++ header files for exported classes/structs and ensures that member
functions defined out-of-line with the 'inline' keyword are also declared with
the 'inline' keyword within the class definition.

This prevents GCC (when compiling for mingw-w64 target) from attributing
`__declspec(dllimport)` to member functions declared without 'inline', avoiding:
- "warning: '...' redeclared without dllimport attribute: previous dllimport ignored"
- "error: '...' redeclared without dllimport attribute after being referenced with dll linkage"

Note that being correct/explicit about marking functions as inline in the class
definition is important as of dad0011bf59f1330f5b89908e01c1eb2840ac6da. This change
fixed the use of `__declspec(dllimport)` which is important for Qt Signals & Slots to
work but means we need to be more explicit.
"""

import argparse
import os
import re
import sys
from typing import List, Optional, Set, Tuple


def split_parameters(params_str: str) -> List[str]:
    """Splits parameter list by top-level commas (respecting <...>, (...), and [...])."""
    if not params_str.strip():
        return []
    parts = []
    p_depth = 0
    t_depth = 0
    b_depth = 0
    cur = []
    for ch in params_str:
        if ch == "(":
            p_depth += 1
        elif ch == ")":
            p_depth -= 1
        elif ch == "<":
            t_depth += 1
        elif ch == ">":
            t_depth -= 1
        elif ch == "[":
            b_depth += 1
        elif ch == "]":
            b_depth -= 1
        elif ch == "," and p_depth == 0 and t_depth == 0 and b_depth == 0:
            parts.append("".join(cur))
            cur = []
            continue
        cur.append(ch)
    if cur:
        parts.append("".join(cur))
    return parts


def normalize_param_type(param: str) -> str:
    """Extracts and normalizes the type part of a parameter, discarding default values and variable names."""
    # Strip default value (= ...)
    p_depth = 0
    t_depth = 0
    res = []
    for ch in param:
        if ch == "(":
            p_depth += 1
        elif ch == ")":
            p_depth -= 1
        elif ch == "<":
            t_depth += 1
        elif ch == ">":
            t_depth -= 1
        elif ch == "=" and p_depth == 0 and t_depth == 0:
            break
        res.append(ch)
    p = "".join(res).strip()

    # Strip parameter variable name if present at end of parameter
    p = re.sub(r"([*&])\s*[a-zA-Z_]\w*$", r"\1", p)
    words = p.split()
    type_keywords = {
        "const",
        "volatile",
        "unsigned",
        "signed",
        "long",
        "short",
        "int",
        "char",
        "wchar_t",
        "char8_t",
        "char16_t",
        "char32_t",
        "void",
        "bool",
        "float",
        "double",
        "auto",
        "struct",
        "class",
        "typename",
    }
    if len(words) >= 2 and words[-1] not in type_keywords and re.match(r"^[a-zA-Z_]\w*$", words[-1]):
        p = " ".join(words[:-1])

    # Normalize pointer and reference tokens
    p = re.sub(r"\s*([*&]+)\s*", r" \1 ", p)
    return re.sub(r"\s+", " ", p).strip()


def extract_decl_info(stmt: str) -> Optional[Tuple[str, Tuple[str, ...], bool]]:
    """Extracts (method_name, normalized_param_types, is_const) from a class member declaration."""
    name_pattern = r"(operator\s*\(\)|operator\s*(?:[^\w\s()]+|\[\]|\w[\w\s*&]*?)|~?\w+)"
    op_call_m = re.search(r"\boperator\s*\(\)\s*\(", stmt)
    if op_call_m:
        name = "operator()"
        open_idx = op_call_m.end() - 1
    else:
        open_idx = stmt.find("(")
        if open_idx == -1:
            return None
        prefix = stmt[:open_idx].rstrip()
        name_m = re.search(name_pattern + r"\s*$", prefix)
        if not name_m:
            return None
        name = name_m.group(1)

    # Find matching closing parenthesis
    depth = 1
    i = open_idx + 1
    while i < len(stmt) and depth > 0:
        if stmt[i] == "(":
            depth += 1
        elif stmt[i] == ")":
            depth -= 1
        i += 1
    if depth != 0:
        return None

    params_str = stmt[open_idx + 1 : i - 1]
    trailer = stmt[i:].strip()
    is_const = bool(re.search(r"\bconst\b", trailer))
    cleaned_params = tuple(normalize_param_type(p) for p in split_parameters(params_str))
    return name, cleaned_params, is_const


def extract_inline_defs(content: str, cls_name: str) -> List[Tuple[str, Tuple[str, ...], bool]]:
    """Finds all out-of-line 'inline' definitions for cls_name in content."""
    name_pattern = r"(operator\s*\(\)|operator\s*(?:[^\w\s()]+|\[\]|\w[\w\s*&]*?)|~?\w+)"
    pattern = r"^inline\s+(?:[^\n{;]+?\s+)?(?:[a-zA-Z_]\w*::)*" + cls_name + r"::" + name_pattern + r"\s*\("
    defs = []
    for m in re.finditer(pattern, content, re.MULTILINE):
        name = m.group(1)
        open_idx = m.end() - 1
        depth = 1
        i = open_idx + 1
        while i < len(content) and depth > 0:
            if content[i] == "(":
                depth += 1
            elif content[i] == ")":
                depth -= 1
            i += 1
        if depth != 0:
            continue
        params_str = content[open_idx + 1 : i - 1]
        brace_idx = content.find("{", i)
        if brace_idx == -1:
            continue
        between = content[i:brace_idx].strip()
        is_const = bool(re.search(r"\bconst\b", between))
        cleaned = tuple(normalize_param_type(p) for p in split_parameters(params_str))
        defs.append((name, cleaned, is_const))
    return defs


def find_matching_class_body(content: str, macro_regex: str, cls_name: str) -> Optional[Tuple[int, int, str]]:
    """Locates the starting and ending indices of the class body for cls_name."""
    cls_pattern = r"(?:class|struct)\s+" + macro_regex + r"\s+" + cls_name + r"\b[\s\S]*?\{"
    cls_m = re.search(cls_pattern, content)
    if not cls_m:
        return None
    start = cls_m.end() - 1
    depth = 1
    i = start + 1
    while i < len(content) and depth > 0:
        if content[i] == "{":
            depth += 1
        elif content[i] == "}":
            depth -= 1
        i += 1
    if depth != 0:
        return None
    return start + 1, i - 1, content[start + 1 : i - 1]


def process_header(file_path: str, macro_regex: str, apply_fixes: bool) -> List[str]:
    """Inspects (and optionally modifies) a single header file for missing in-class inline declarations."""
    with open(file_path, "r", encoding="utf-8", newline="") as fp:
        content = fp.read()

    if not re.search(macro_regex, content):
        return []

    classes: Set[str] = set(re.findall(r"(?:class|struct)\s+" + macro_regex + r"\s+(\w+)\b", content))
    if not classes:
        return []

    issues = []
    new_content = content

    for cls in sorted(classes):
        defs = extract_inline_defs(new_content, cls)
        if not defs:
            continue

        body_info = find_matching_class_body(new_content, macro_regex, cls)
        if not body_info:
            continue
        body_start, body_end, body = body_info

        prev = 0
        replacements = []
        for idx, ch in enumerate(body):
            if ch == ";":
                raw_stmt = body[prev:idx]
                prev = idx + 1

                # Skip statements that are not regular member declarations
                if any(k in raw_stmt for k in ("using ", "typedef ", "friend ", "template")):
                    continue
                decl = extract_decl_info(raw_stmt)
                if not decl:
                    continue

                # Match against out-of-line inline definitions
                if any(d == decl for d in defs):
                    if re.search(r"\binline\b", raw_stmt):
                        continue

                    # Compute approximate line number in file
                    stmt_offset = body_start + (idx - len(raw_stmt))
                    line_num = new_content[:stmt_offset].count("\n") + 1
                    msg = f"{file_path}:{line_num}: {cls}::{decl[0]} missing 'inline' in class declaration"
                    issues.append(msg)

                    if apply_fixes:
                        m = re.search(r"^(.*?(?:public|protected|private)\s*:\s*)?(\s*)(.*)$", raw_stmt, re.DOTALL)
                        if m:
                            prefix_access = m.group(1) or ""
                            indent = m.group(2)
                            code = m.group(3)
                            # Do not prepend inline before preprocessor lines
                            if code.lstrip().startswith("#"):
                                lines = code.split("\n")
                                new_lines = []
                                applied = False
                                for line in lines:
                                    if not applied and line.strip() and not line.strip().startswith("#"):
                                        ind = len(line) - len(line.lstrip())
                                        new_lines.append(line[:ind] + "inline " + line[ind:])
                                        applied = True
                                    else:
                                        new_lines.append(line)
                                new_stmt = prefix_access + indent + "\n".join(new_lines)
                            else:
                                new_stmt = prefix_access + indent + "inline " + code
                            replacements.append((idx - len(raw_stmt), idx, new_stmt))

        if apply_fixes and replacements:
            new_body = body
            for s_idx, e_idx, new_stmt in reversed(replacements):
                new_body = new_body[:s_idx] + new_stmt + new_body[e_idx:]
            new_content = new_content[:body_start] + new_body + new_content[body_end:]

    if apply_fixes and new_content != content:
        with open(file_path, "w", encoding="utf-8", newline="") as fp:
            fp.write(new_content)

    return issues


def main():
    parser = argparse.ArgumentParser(
        description="Check and fix missing 'inline' keywords on in-class declarations of exported classes."
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=["."],
        help="Directories or files to process (default: current directory).",
    )
    parser.add_argument(
        "--apply",
        "-a",
        action="store_true",
        help="Apply replacements in-place instead of only reporting missing 'inline' keywords.",
    )
    parser.add_argument(
        "--macro",
        "-m",
        default=r"(?:[A-Z0-9_]+_EXPORT)",
        help="Regex pattern for export macro (default: '(?:[A-Z0-9_]+_EXPORT)').",
    )
    args = parser.parse_args()

    header_files = []
    for path in args.paths:
        if os.path.isfile(path):
            if path.endswith((".h", ".hpp", ".hxx")):
                header_files.append(os.path.abspath(path))
        elif os.path.isdir(path):
            for root, _, filenames in os.walk(path):
                for f in filenames:
                    if f.endswith((".h", ".hpp", ".hxx")):
                        header_files.append(os.path.abspath(os.path.join(root, f)))

    header_files.sort()
    all_issues = []
    for h in header_files:
        issues = process_header(h, args.macro, args.apply)
        all_issues.extend(issues)

    if all_issues:
        action = "Fixed" if args.apply else "Found"
        print(f"{action} {len(all_issues)} issue(s):")
        for issue in all_issues:
            print(" ", issue)
        if not args.apply:
            sys.exit(1)
    else:
        print("All in-class declarations match out-of-line inline definitions.")


if __name__ == "__main__":
    main()
