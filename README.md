# archanger

AR Changer for osu sort of, done in a kind of shit way

Fixes I'll implement when I'm bothered, in order of difficulty:

1. choose an address to freeze from specific offset from ar instruction sig (to avoid the multiple sig scans this does)
2. make my shit byte patching function take arguments to patch multiple bytes at once instead of calling it like 6 times
3. use hooking instead of just byte patching to fix stability (use push (addy))

