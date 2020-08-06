# UnRAR PS3 library

This library is a port of [unRAR](https://www.rarlab.com/rar_add.htm) to the PlayStation 3 using the [PSL1GHT SDK](https://github.com/ps3dev/PSL1GHT/), 
that provides .RAR archive handling and extraction.

## Source Version

UnRAR [5.9.2](https://www.rarlab.com/rar/unrarsrc-5.9.2.tar.gz)

### Exports

 - RAROpenArchive
 - RAROpenArchiveEx
 - RARCloseArchive
 - RARReadHeader
 - RARReadHeaderEx
 - RARProcessFile
 - RARProcessFileW
 - RARSetCallback
 - RARSetChangeVolProc
 - RARSetProcessDataProc
 - RARSetPassword
 - RARGetDllVersion

## Build/Install

Build the library with: 
```
make
```

Install the library to your PSL1GHT setup with:
```
make install
```

## Documentation

 - [UnRAR manual](./manual.txt)
 - [HTML manual](http://goahomepage.free.fr/article/2000_09_17_unrar_dll/UnRARDLL.html)

## Sample app

You can find a sample PSL1GHT app using the library [here](./example).

## License

See the original [UnRAR license](./license.txt) by Alexander L. Roshal.
