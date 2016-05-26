------------------------------------------------------------------------
Generic Ogg Ripper v1.4                                              .O.
(c)Felix de las Pozas Alvarez                                        ..O
August 14 2004                                                       OOO
------------------------------------------------------------------------

License
------------------------------------------------------------------------
This work is under the GPL license included in the distribution, read
it for detailed information about what you can do or don't with the
source code and the binaries. 


Program Information
------------------------------------------------------------------------
This program extracts Ogg files from game datafiles. I coded it to 
extract the music from the game "Hitman 2: Silent Assasin" by IO 
Interactive, but it will work with any file so it will probably works
with games like "Soul Reaver 2", "Blood Omen 2" and any other game 
that uses the Ogg format.
 
So far I've used it to extract the music and sounds from :
- Hitman 2
- Freedom Fighters
- Hitman Contracts
- Thief Deadly Shadows. 

If you know of another game, please contact me.


OggRipper Win32 Binary Usage
------------------------------------------------------------------------
Usage: 
	OggRipper [-snnn] [-ennn] [-znnn] file1 [file2] ... [fileN]

Options: 
	-snnn		Skip the FIRST nnn ogg files found. If ommited
			no files will be skipped.
	-ennn		Extract the NEXT nnn ogg files, not more. If 
			ommited all found files will be extracted
			except if 'size' is specified.
	-znnn           Only extracts files bigger than nnn kb.
	file            File(s) to inspect, wildcards accepted in each.

'Skip', 'Extract' and 'Size' options cross file boundaries. 'file'
parameter is required, others are optional.
Remember that a file name with spaces in it must be written between
commas, like this "d:\file name". 

Examples : 
	OggRipper streams.wav
	OggRipper *.dat ..\*.pak "d:\game dir\data file.dat"
	OggRipper *.* ..\*.*
	OggRipper -s5 *.wav ..\*.pak
	OggRipper -e10 *.wav
	OggRipper -s100 -e50 streams.wav
	OggRipper -s10 -e5 -z1024 *.lib


History
------------------------------------------------------------------------
v1.4  - August 14, 2004
	Changed some internal structure and fixed a bug where the
	program aborted when encountered a I/O error in a input file,
	now the program notifies the error and continues until all
	files had been inspected. Maybe final version until GUI version.
	Uses 'getopt' so probably only compiles with Mingw and not with
	Digital Mars or Microsoft, but not tested with those.

v1.3  - August 10, 2004
	Added 'Size' option. 

v1.2  - August 4, 2004
	Added 'Skip' and 'Extract' options. Number of found files now
	has a 8 digit limit. Changed some error messages. 

v1.1  - February 27, 2004
	Cleaned the source code and did a bit of type checking in
	comparisons, assignations and return values.

v1.01 - January 7, 2004
	More descriptive error messages.

v1.0  - December 15, 2003
	Revised the old source code and made some changes. It will
	compile without problems under Mingw and Digital Mars Compilers.
	

Contact (Don't send me spam or we won't be friends ;-)
------------------------------------------------------------------------
email  : FelixdelasPozas@ToughGuy.net
website: http://perso.wanadoo.es/longcoldwalk/


Thanks to...
------------------------------------------------------------------------
IO Interactive for their great games.
Composer of the Hitman music Jesper Kyd. 


F.A.Q.
------------------------------------------------------------------------
Q: This program does not work for the game <your favourite game here>!!
A: Probably it does not use the ogg format or it's encrypted.

Q: This program gave me an error while writing/reading!!
A: Probably there is a fault of your hard drive or you don't have 
   enough free space.

Q: I've tested it with the game <blah blah> and I can't listen to the 
   Ogg extracted files!
A: Maybe the data stored on the hard disk in not a pure Ogg format. This
   happens for example with "Commandos 3: Destination Berlin" (datafiles
   have some bytes changed as some sort of protection).

