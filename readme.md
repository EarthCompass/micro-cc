## Micro-cc

A toy compiler made for compiler principle course's lab, using flex/bison/LLVM.

## How to compile micro-cc
My development environment:
```
macOS Catalina 10.15.7

Apple clang version 12.0.0 (clang-1200.0.32.2)
Target: x86_64-apple-darwin19.6.0

flex 2.5.35 Apple(flex-32)

bison (GNU Bison) 3.7

LLVM 11.0.0 (prebuilt binarys)
```
Only macOS is tested, but it should run on Linux.

Set `LLVM_DIR` in `CMakeList.txt`before compiling.
## Reference

1. https://gnuu.org/2009/09/18/writing-your-own-toy-compiler/
2. https://github.com/stardust95/TinyCompiler
3. http://web.iitd.ac.in/~sumeet/flex__bison.pdf
4. http://www.calvinneo.com/2016/07/29/flex%E5%92%8Cbison%E4%BD%BF%E7%94%A8/
5. https://www.gnu.org/software/bison/manual/html_node/Prologue-Alternatives.html#Prologue-Alternatives
6. http://llvm.org/docs/tutorial/MyFirstLanguageFrontend/
7. https://clarazhang.gitbooks.io/compiler-f2018/content/llvmIRGen.html#24-assembly-builder
