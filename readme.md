Wav2Mp3 converter by HELF Szilárd

Development environment:
HW:
Intel Core i5-10400F CPU @ 2.90GHz 6 Cores, 12 Logical processors
Installed Physical Memory (RAM)	16,0 GB

SW (as of 2022.02.12.):
Microsoft Windows 10 Pro
Visual Studio Code 1.64.2 
Windows SDK v142
Doxygen 1.9.2.windows.x64
lame-3.100
Dirent interface for Microsoft Visual Studio by Toni Ronkko
PTHREADS-WIN32 (2.9.0)
Remote WSL v0.64.2
Ubuntu 20.04.3 LTS (WSL)
Microsoft Visual Studio Community 2019 (v142) Version 16.11.10

The linux version was made with WSL, compiled with the settings in tasks.json of Visual Studio Code, and executed with launch.json file. Compilers: 
	g++ 9.3.0
	g++-9 9.3.0
	clang++-10 10.0.0

The Windows version was made with Visual Studio 2019.
Compiler:
	Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30140 for x86

There are still some small TODO comments in the code, which could be examined later.

Unit testing:
Using the project UnitTest_Wav2mp3 in Visual Studio. Check the "Test Explorer" view in VS. Classes have separate testcases. 
Some of them can result in an exception (eg: TestBadNumSamples -> Exception Code: C0000005).

Valgrind:
Valgrind tests convert small and big wav files. 
Memory leaks were not detected: "All heap blocks were freed -- no leaks are possible"
The following warning occures with big files:
==6757== Warning: set address range perms: large range

Doxygen documentation is available.

