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
plus operator: 00:00:19
stringstream: 00:00:16
string builder: 00:00:05
diff (stringstream minus string builder): 00:00:11
factor (stringstream / string builder): 3.2
```

However, with -O0 4 times *slower*:

```
plus operator: 00:00:34
stringstream: 00:00:23
string builder: 00:01:02
diff (stringstream minus string builder): - 00:35:31
factor (stringstream / string builder): 0.370968
```

Still 2.42 times faster than stringstream with -O2:

```
plus operator: 00:00:21
stringstream: 00:00:17
string builder: 00:00:07
diff (stringstream minus string builder): 00:00:10
factor (stringstream / string builder): 2.42857
```

So this basic tests show that string builder is up to 3 times faster when using full optimization
and still 2 times faster when using -O2 (default under Arch Linux). However, without optimization it way
slower. Results with clang++ were similar.
