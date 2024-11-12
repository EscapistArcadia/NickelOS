__attribute__((naked, noinline)) void NickelMain() {
    for (int i = 0; i < 200000; ++i) {                      /* sets the first 200000 pixels to my favorite color */
        *((unsigned int *)0x80000000 + i) = 0x00FF96;
    }
    while (0xECEBCAFE);
}
