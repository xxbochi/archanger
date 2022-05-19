# archanger

AR Changer for osu sort of, done in a kind of shit way

Targets this region of memory:

![image](https://user-images.githubusercontent.com/89066726/169220463-9b543f9b-2a14-4b59-ab97-c40a9399a895.png)

# method
The idea is to replace the function called with an "fld [address]", with the address being an empty address that our program can write a float to (allowing us to write to the float stack)

The call shown is called from multiple places to determine different values used in osu! difficulty calculation, so we can't modify the function; we can only stop it from being called in this case and divert it to our own code to be executed. Right now we just patch in the fld instruction bytes manually, but this has caused a lot of crashing issues and is generally a messy solution. Soon I'll write a hook function so we can write to empty space and jmp / ret back and forth from the regular code execution, so stability shouldn't be an issue.

# detection
Program should be UD as far as osuauth.dll goes and you don't use cheat engine to inject or something stupid

# additional

there are lots of header files/imgui stuff missing from this repo, I'll add them later once I find the motivation to make this a good program

# planned fixes
Fixes I'll implement when I'm bothered, in order of difficulty:

1. choose an address to freeze from specific offset from ar instruction sig (to avoid the multiple sig scans this does)
2. make my shit byte patching function take arguments to patch multiple bytes at once instead of calling it like 6 times
3. use hooking instead of just byte patching to fix stability (use push (addy))

