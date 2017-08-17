# Calculates total statistics for source-based code coverage report created
# with `llvm-cov report` when at least one source file has been specified.

# NOTE: When at least one source file is passed to `llvm-cov`, the summaries
# are shown for each function in the listed files (and not for each file in the
# coverage data).

{
    if($1 == "TOTAL") {
        total_regions += $2;
        missed_regions += $3;
        total_lines += $5;
        missed_lines += $6;
    }
}

END {
    covered_regions = total_regions - missed_regions;
    covered_lines = total_lines - missed_lines;

    print "Covered regions: " covered_regions;
    print "Missed regions: " missed_regions;
    print "Region cover: " covered_regions/total_regions*100 "%\n";
    print "Covered lines: " covered_lines;
    print "Missed lines: " missed_lines;
    print "Line cover: " covered_lines/total_lines*100 "%";
}
