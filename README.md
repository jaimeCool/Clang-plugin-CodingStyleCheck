# Clang-plugin-CodingStyleCheck
>coding style check plugin

- download CodingStyleCheck.dylib
```
clang \
-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator11.2.sdk \
/path/to/test/*.m \
-fsyntax-only \
-v \
-Xclang -load \
-Xclang /path/to/CodingStyleCheck.dylib \
-Xclang -plugin \
-Xclang coding-style-check
```
- 如何编写参考：[【clang】高效开发一个clang plugin]() 或者直接看[wiki](https://github.com/jaimeCool/Clang-plugin-CodingStyleCheck/wiki/%E3%80%90clang%E3%80%91%E9%AB%98%E6%95%88%E5%BC%80%E5%8F%91%E4%B8%80%E4%B8%AAclang-plugin)
