<div align="center">
    <h1>thrpl.c</h1>
    <p>An implementation of Thread Pool in C.</p>
</div>

## Usage

```bash
git clone https://github.com/zaiic/thrpl.c
cd thrpl.c
```

Put `thrpl.c` and `thrpl.h` in your project or generate a **header-only** file and put it in your project.

Generate **header-only** file:

```bash
./generate_header_only.sh
```

Notice: The *header-only* file is [stb style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt).
Don't forget to put `#define THRPL_IMLEMENTATION` in your file before including *header-only* file.

```c
#define THRPL_IMLEMENTATION

#include "thrpl.h" // header-only file
```

## License

[The MIT License](./LICENSE)
