Dumping PS3 ISO on PC Step-by-step 

In case the easy way didn't work for you, here's a compiled list of the step-by-step instructions
we used for dumping disc-based PlayStation 3 titles. (Not recommended)

PS3-ISO-patcher.zip 78 KB | 3k3y_IsoTools.zip 3 MB | SetupImgBurn_2.5.8.0.exe 3 MB

1) DUMP THE ISO

* Insert a PlayStation 3 format disc title of your choice into your compatible Blu-ray drive.

* Create the .iso image using an .iso dumping program of your choosing, e.g. ImgBurn or IsoBuster.

2) GET THE IRD FILE

* Use http://ps3.aldostools.org/ird.html to download the appropriate .ird file that matches your title ID.
  If there isn't an .ird file that matches your title ID, you cannot use this method to dump your selected PlayStation 3 disc
  at this time and will need to use your PS3.

* Be sure to check the title ID in case there is a different edition of that title.
  e.g. Uncharted 2 Game of the Year Edition. You must use the correct .ird with the same title ID, otherwise it will not work.
  (Example: .ird file for Demon's Souls US disc does not work with Demon's Souls EU disc).

3) PATCH & DECRYPT THE ISO

* Using PS3 ISO Patcher by BlackDaemon, select the matching .iso and .ird files,
  then press Patch to apply the decryption keys to the .iso file.

* Using 3K3Y IsoTools press Decrypt button and select the .iso with patched-in decryption keys.
  This will produce a decrypted .dec.iso file.

---
* Or use PS3Dec to decrypt the .iso using a .dkey or hex key
  Use http://ps3.aldostools.org/dkey.html to get the disc key needed to decrypt the ISO.

  PS3Dec d key <32-hex-chars-disc_key> <full-path-iso> [<out-file-iso>]

  PS3Dec d 3k3y <full-path-iso> [<out-file-iso>]
  

4) VERIFY THE ISO (OPTIONAL)

* Optionally, validate that you have successfully obtained the correct copy of your game.
  You will need to install PS3 ISO Rebuilder tool from http://archive.org/download/ps3-iso-rebuilder-1.0.4.1
  Load your .dec.iso file and your .ird file in the program and let it verify the dump.
  All of your files must be either Valid or Not required.

5) EXTRACT THE ISO (OPTIONAL)

* In 3K3Y ISO Tools, use the Tools drop-down menu to select ISO > Extract ISO
  and then select the decrypted .iso file to extract its files.
   
  Another option is to use 7-zip or any other software that is capable of extracting .iso images
  and extract to a folder.

* You are now able to use the extracted .iso files with RPCS3 or PS3 HEN or CFW.
