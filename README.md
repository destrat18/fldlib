FLDLib -- An instrumentation library based on affine forms for accuracy analysis
================================================================================

# Context

The instrumentation library FLDLib replaces the floating-point types 
in the C, C++ source code by a comparison between an ideal
computation and the effective computation.

The library propagates domains within the program execution. That
means that each stored variable is a float/double/long double but
also a domain of values that propagates along the program execution.

The library considers two worlds: the continuous world defined by the
floating point types and the discrete world defined by the integer
types. The discrete world contains only the execution value. The
continuous world contains the execution value, the floating-point
domain, the real domain and the error domain with the constraint
floating-point value = real value + error value.

Passing from the discrete world to the continuous world (ex cast from
int to float) is straightforward.  But passing from the continuous
world to the discrete world (ex comparison operators, cast from float
to int) may create unsolved branches and unstable branches. 

Unsolved branches appear when the discrete world receives at least
two potential distinct values. For example if the floating-point
variable x has a floating-point value and a real value in the
interval `[0.0, 2.0]`, the comparison expression `x < 1` generates
both the discrete `true` or `1` value and the discrete `false` or
`0` value.
To solve this problem, the abstract execution successively explores
all possible branches with one set of values for each branch: the
`then` branch is explored with a domain intersected with
`[0.0, 1.0[` and the `else` branch is explored with a domain
intersected with `[1.0, 2.0]`.

Unstable branches appear when the discrete world receives different
values for the floating-point value and for the real value. For
example, if the floating-point variable x has a floating-point value
equal to {0.99} and a real value equal to {1.01}, the comparison
expression `x < 1` generates the discrete `false` or `0` value from
the floating-point value; but it generates the discrete `true` or
`1` value from the real value.

Unstable branches and unsolved branches can live together and so it
needs to be managed by specific execution branches. To synchronize
unstable branches, the user needs to insert specific split/merge
macros around the operations from the discrete world to the concrete
world. You can find more about split/merge in the examples and in
the documentation below. With the same macros, a specific option
`-loop` explores all the unsolved/unstable branches until all local
paths are covered.

# Build and use the library

You can build the library with different options. For each package
of options, the library provides three kinds of instrumentation:

  * an exact instrumentation that compares a floating point value
    and a real value approximated with (e,m) bits for the couple
    (exponent,mantissa). By default e = 16 and m = 123 - the library
    uses 5 more bits to print the values. This exact instrumentation
    needs only to manage unstable branches.

  * an interval instrumentation that contains all the floating-point
    and all the real values. This instrumentation is not very
    interesting but since it manages only unsolved branches, it has
    helped to debug this functionality.

  * an affine instrumentation that defines the real domains and the
    error domains with affine forms that share common symbols, which
    expresses some linear dependencies within the domains.

To build and install the FLDLib library, you can type the following
commands

```sh
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=TheInstallationDirectory -DFLDLIB_ENABLE_TESTS=ON ..
make
make install
```

To check the library

```sh
ctest
```
 
To generate a diagnostic for a project (it should automatically
build the adequate library at top level)

```sh
export FLOATDIAGNOSISHOME=TheInstallationDirectory
cd ../tests
$FLOATDIAGNOSISHOME/bin/comp_float_diagnosis.sh -affine -optim \
  -atomic absorption.cpp -o absorption.instr_diagnosis_affine
```

To generate the diagnosis, simply execute

```sh
./absorption.instr_diagnosis_affine
```

The diagnosis file is then the new generated file
`absorption_diag_aff_out`.

# Experiment with your own code

First, take a look at the examples in the `tests` directory and the
commands run when instrumentating these unit tests.

If your code contains a single source file with a `main` function,
you just need to add

* the macro `DECLARE_RESOURCES` to the beginning of the source file,  
* the macro `INIT_MAIN` to the beginning of the `main` function and
  the macro `END_MAIN` to the end of the `main` function, just
  before the `return 0;` instruction. `END_MAIN` aims to catch some
  exceptions that the instrumentation library may throw,  
* the macro `DPRINT(output)` for the variables of interest from the
  accuracy point of view,
* the macro `DBETWEEN(min, max)` to broaden the check with
  ranges on inputs rather than single values. For example,
  if `double in = 3.5;` appears in your test case,
  you can replace it with `double in = DBETWEEN(3.0, 4.0);` to check
  the accuracy of the computations for all input values between `3.0`
  and `4.0`.

Then the command `$FLOATDIAGNOSISHOME/bin/comp_float_diagnosis.sh -affine
-optim -atomic file.c(pp) -o file.instr_diagnosis_affine` will
instrument your code with the `FLDLib` library, and running
`file.instr_diagnosis_affine` will produce a file `file.instr_diagnosis_affine`
containing the variables marked with a `DPRINT` along with their ideal range,
their floating-point range and the (guaranteed/conservative) roundoff error
accumulation range.

If your code has multiple source files with a `cmake` build system, examine the
instrumentation command `comp_float_diagnosis.sh` run on a single source file.
For example, on the test file `absorption.cpp`, the command

```sh
$FLOATDIAGNOSISHOME/bin/comp_float_diagnosis.sh -affine -optim -atomic absorption.cpp -o absorption.instr_diagnosis_affine
```

would display the following line

```
g++ -DPROG_NAME=absorption -DFLOAT_DIAGNOSIS -ITheInstallationDirectory/include/fldlib -std=c++20 -ITheInstallationDirectory/include/fldlib -ITheInstallationDirectory/include/fldlib/utils -ITheInstallationDirectory/include/fldlib/algorithms -ITheInstallationDirectory/include/fldlib/applications -DFLOAT_ATOMIC -include std_header.h -DFLOAT_AFFINE -DFLOAT_FIRST_FOLLOW_EXE -O3 -DDefineDebugLevel=1 -DFLOAT_SILENT_COMPUTATIONS absorption.cpp -o absorption.instr_diagnosis_affine -L/home/vedrine/git/fldlib_new/bil/install/lib/fldlib -lm -lFloatDiagnosis
```

You can then edit the `CMakeLists.txt` file with
new verification targets for `FLDLib` instrumentation.
These verification targets must contain the compilation options
listed above.

You must then modify the source file containing the `main` function
with the `DECLARE_RESOURCES`, `INIT_MAIN` and `END_MAIN` macros.
The `FLOAT_DIAGNOSIS` or `FLDLIB_VERSION_MAJOR` macros should protect
their use like

```c++
#ifdef FLOAT_DIAGNOSIS
#DECLARE_RESOURCES
#endif
```

to continue compiling the original targets.

The instrumentation modifies the `double` and `float` types with structured
types. Hence, the compilation may fail, such as the `printf` function
with the `%f`, `%e` flags. In this case, you can modify your source
code with the methodes provided by `FLDLib` for instrumented structures while
protecting them with the `FLOAT_DIAGNOSIS` or `FLDLIB_VERSION_MAJOR` macros.

Unstable branches are likely to occur when running instrumented
code. To cover all the possible cases, you should add the `FLOAT_SPLIT_ALL`
and `FLOAT_MERGE_ALL` macros to synchronize these branches.
For this, see the `tests/simple.cpp` file.

```c++
  FLOAT_SPLIT_ALL(1, y >> double::end(), x << double::end())
  if (x < 1.0f) {
    FPRINT(x);
    y = 4*x-3;
  }
  else {
    FPRINT(x);
    y = x;
  }
  FLOAT_MERGE_ALL(1, y << double::end(), x >> double::end())
```

The macros create a loop to cover all branches and synchronize the
results.

* The variable `y` requires synchronization because it is modified
  differently depending on the branch and is read after the
  synchronization point.  
* The variable `x` does not require synchronization because it is not read after
  the synchronization point.  
* The variable `x` must be restored before the loop body since its value
  before the split section is read in the split section and `x` is modified
  by the `if (x < 1)` statement: `x` $\in [0.0, 2.0]$ becomes `x` $\in [0.0, 1.0[$
  in the `then` branch.  
* The variable `y` does not need to be restored because its value before the
  split section is not read in the split section.

The FLDLib generic options do not allow synchronization/restoration of integer
domains in the `FLOAT_SPLIT_ALL` and `FLOAT_MERGE_ALL` macros. An experimental
version (see the cmake variable `FLDLIB_SUPPORT_INT_DOMAIN`) removes this limitation, but
you must use the type `NumericalDomains::TFldlibIntegerBranchOption<int>`
instead of the `int` type at certain points in your code, which significantly
increases the instrumentation effort.

## Credits

The library has been developed thanks to the feedback from the experiments
carried out jointly with the ASNR (Nuclear Safety and Radiation Protection
Authority) and the partners of the ANR projects CAFEIN and INTERFLOP to
improve the diagnosis of accuracy in numerical software.

# Options for the instrumentation library

More precisely, the instrumentation `comp_float_diagnosis.sh` comes
with many options. Each set of options comes with a specific way to
compile the instrumentation library. The simpler way to experiment
with the options is to try a specific set of options. In case of
failure at link-time, then the user should look at error message,
identify the missing library, go into the parent directory ..,
compile the missing library with 'make -j libFloat...a', go into the
`test_compiler` directory and play back the specific set of options.

The following options are supported:

  * -interval or -exact or -affine 
  * -optim
  * -atomic
  * -loop
  * -verbose
  * -print-path

The last options in the command line `comp_float_diagnosis.sh` are the
compilation options of the project.

You can see some macros in the source code

  * `DECLARE_RESOURCES`  
    To put just after the header of the translation unit file.c(pp)
    containing the main function.  
  * `INIT_MAIN`, `END_MAIN`  
    To put at the beginning and at the end of the main function.  
  * `FPRINT`, `DPRINT`  
    To define the variables whose accuracy is interesting to follow.  
  * `FLOAT_SPLIT_ALL`, `FLOAT_MERGE_ALL`  
    The content of the these macros is defined as follow

    ```c++
    FLOAT_SPLIT_ALL(ident, out1 >> ... >> double::end(),
        intermediate1 << ... << double::end())

    /* code containing the unstable branch */

    FLOAT_MERGE_ALL(ident, ... << out1 << double::end(),
        ... >> intermediate1 >> double::end())
    ```

    The variables out1, ... are floating-point variables that are
    assigned by at least one of the branches and that are used after
    the MERGE macro. The variables intermediate1, ... are variables
    that are potentially modified or constrained by a branch and
    then used by another branch. By listing them, the user enables
    to store their value before the unstable branch and to restore
    their original value before the abstract execution goes in the
    other branch.

    The SPLIT/MERGE macros are first defined around the unstable
    comparisons. If the comparisons are in the condition of an if
    instruction, then the MERGE macro moves just after the immediate
    post-dominator of the if. If some discrete variables (integers,
    pointers) may escape from the MERGE with potential different
    values, then the MERGE should move until the discrete variable
    is no more useful (in the def/use sense used in Static Single
    Assignment transforms).

# Targetted numerical systems

This library is likely to be used to deliver a diagnosis verdict
on numerical algorithms to medium size numerical systems (some ten
thousands lines of code) with an analysis methodology from tests
to sound static analyses.

If you have questions about

 * the methodology  
 * how to obtain less overapproximated accuracy results, with
   less analysis time on bigger numerical systems

do not hesitate to contact us at name@cea.fr with name =
franck.vedrine

