# Simple/stupid benchmarking

Compares the string builder and numberToString() function as provided by c++utilities
with std::stringstream (without reserving stringstream buffer).

The string builder is actually just a fancy syntax for std::string::reserve()
making its use more convenient.

## Compile and run

eg.
```
g++ -O3 stringbuilder-bench.cpp -o stringbuilder-bench-O3 -Wl,-rpath /lib/path -L /lib/path -lc++utilities
./stringbuilder-bench-O3
```

## Results on my machine

Results with -03:

```
plus operator: 00:00:17
stringstream: 00:00:16
string builder: 00:00:07
diff (stringstream minus string builder): 00:00:09
factor (stringstream / string builder): 2.28571
```

However, with -O0 4 times *slower*:

```
plus operator: 00:00:29
stringstream: 00:00:16
string builder: 00:01:04
diff (stringstream minus string builder): - 00:35:22
factor (stringstream / string builder): 0.25
```

Still 1.45 times faster than stringstream with -O2:

```
plus operator: 00:00:21
stringstream: 00:00:16
string builder: 00:00:11
diff (stringstream minus string builder): 00:00:05
factor (stringstream / string builder): 1.45455
```

So this basic tests show that string builder is up to 2 times faster when using full optimization
and still 1.4 times faster when using -O2 (default under Arch Linux). However, this templating
stuff completely relies on optimization (as expected). Results with clang++ where similar.
