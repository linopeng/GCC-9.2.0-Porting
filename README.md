# GCC-9.2.0 for Posit&Porting
This repo divided into two parts. One of GCC porting for spim. The other is for GCC internal transform floating-point format.
# Porting
In this part, we porting GCC into spim target. In GCC backend, MD(machine description) used to defien target architecture and output assembly code. 
It divid three file of MD(target.c/target.g/target.md).
* Target.c contain functions used by target.h and target.md.
* Target.h define macros to describe target architecture like storage layout, register usage,...
* Target.md match RTL codes to output target assembly codes.
![](https://i.imgur.com/WkqmzK0.png)

We want to add multiple descriptions:
1. In the file gcc/config.sub
    * Add to the case $cpu in
        * spim* in the part following
        * Recognize the canonical CPU types that are allowed with any company name.
2. In the file gcc/config.gcc
    * In the case ${target}, after the case for mcore-*-elf), add the following
        ```
        spim*-*-*)
        file_base="`echo ${target} | sed 's/-.*$//'`"
        tm_file="${tm_file} dbxelf.h elfos.h gnu-user.h glibc-stdint.h"
        md_file="${cpu_type}/${file_base}.md"
        out_file="${cpu_type}/${file_base}.c"
        tm_p_file="${cpu_type}/${file_base}-protos.h"
        tmake_file="${tmake_file} spim/t-spim"
        ;;
        ```
### How to build
For target spim, you can refer as following step to build in your machine.

    > git clone https://github.com/linopeng/GCC-9.2.0-Porting
    > mkdir build
    > cd build
    > ../GCC-9.2.0-Porting/configure --prefix=/path --target=spim --enable-languages=c --with-gnu-ld --with-gnu-as --disable-shared --disable-threads --disable-libmudflap --disable-libgomp --disable-libssp --disable-libquadmath --disable-libatomic --with-float=soft --disable-multilib
    > make all-gcc -j8 CFLAG="-O2 -fpermissive -g" CXXFLAGS="-O2 -fpermissive -g"
    > make install 
    
### Examples and Compile
After building and install that we can test the example code. Using spim-gcc to compile assembly code and running Qtspim simulator to verify result.

    > spim-gcc -S example.c

```c=
int add(int a, int b, int c, int d, int e, int f)
{
        c = a + b;
        return c;
}

void main()
{
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7;
        g = add(a, b, c, d, e, f);
}
```
# Posit
In this part, we implement Posit format in GCC. And we via compiler option fposit to switch IEEE 754 or Posit. You can reffer as below picture.

![](https://i.imgur.com/wL1xBhe.png)

To switch IEEE 754 or Posit, we add fposit option in gcc/common.opt for suitable environment. More infomation about commont.opt parameter, you can refer GCC internals manual (https://gcc.gnu.org/onlinedocs/gccint/Options.html#Options).  
Inorder to transform floating-point format, gcc via internal macro REAL_VALUE_TYPE to transform when parsing float senmatic statment. And in the assembly stage Posit format represent as decimal. The REAL_VALUE_TYPE lives in gcc/real.c and real.h. The architecture of compile float flow as below picture. 

![image](https://user-images.githubusercontent.com/51993200/125903994-78018b47-3b7b-44aa-bf10-73290237d3bf.png)

In the work, we use software emulation Posit compute. GCC provides a low-level runtime library, libgcc.a or libgcc_s.so.1 on some platforms. GCC generates calls to routines in this library automatically, whenever it needs to perform some operation that is too complicated to emit inline code for. About the function implement under gcc/libgcc/soft-fp folder.
### How to build
For x86-64, you can refer as following step to build GCC with Posit format in your machine.

    > git clone https://github.com/linopeng/GCC-9.2.0-Porting
    > cd GCC-9.2.0-Porting
    > mkdir build
    > cd build
    > ../configure --target=$TARGET --disable-nls --enable-languages=c,c++ --without-headers --prefix=/path/
    > make all-gcc
    > make all-target-libgcc
    > make all-target-libstdc++-v3
    > make install-gcc
    > make install-target-libgcc
    > make install-target-libstdc++-v3
    > make clean
Besides we need to build Posit library call libposit.a under gcc/libgcc/soft-fp folder. You can refer as following step that get the libposit.a library for Posit computing.

    > cd GCC-9.2.0-Porting/libgcc/soft-fp-2/
    > gcc -c -O2 -mno-sse -msoft-float -I../config/i386/ -I.. *.c
    > ar -crv libposit.a *.o
    > rm -f *.o
    
### Examples and Compile
After above build compiler and library that you can test the example code to validate the works. 

    > Posit-gcc -fposit -msoft-float example-code.c GCC-9.2.0-Porting/libgcc/soft-fp-2/libposit.a

```c=
void main(){
    float A = 1.323;
    float B = -0.792;
    float C = A + B;
    printf("Result A + B = %f\n", C); // Answer = 0.531
}
```
## Refference work
[Essential Abstractions in GCC '13](http://www.cse.iitb.ac.in/grc/gcc-workshop-13/index.php?page=slides)  
[Cerlane Leong - SoftPosit](https://gitlab.com/cerlane/SoftPosit)  
[stillwater-sc - universal](https://github.com/stillwater-sc/universal)
