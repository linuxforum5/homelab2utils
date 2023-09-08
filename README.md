# homelab2utils
Segédprogramok Homelab2 (Aircomp 16) számítógéphez készülő programokhoz
h2CreateHtp, htp2h2wav, htpinfo, h2bas2htp
A Windows programok a win32 mappában, a linux parancsok a bin mappában találhatók.

## h2CreateHtp
Egyelőre egy- vagy kétrekordos htp fájlok generálására, ahol az 1. rekord mindenképpen egy BASIC porgram, a további rekordok már gépikódú vagy direk memória adatok.
A BASIC rekord speciális jellemzőkkel is ellátható, mint például az automatikus indítással együttjáró programvédelem, vagy a megszakítás tiltása.
A tipikus konverzió a következő:
h2CreateHtp -t txtFile -o outputHtpFile
A forrás egy txt fájl, ami a teljes BASIC kódot tartalmazhatja.
Az output pedig egy htp fájl lesz. Figyelem! Ez a htp fájl emulátorba betöltve nem fog tudni automatikusan elindulni. Az automatikus indítás csak a LOAD parancs után aktivizálódik.
Mivel a BASIc sorszámozás nehézkessé teszi a kódolást, a megjegyzések pedig felesleges helyet foglalhatnak, ráadásul korlátozott a használható karakterek száma, ezért a txt fájl további lehetőségewket is megenged:
1 - Sorszámok elhagyása. A BASIC forrásprogramban nem kötelező a sorszámok használata. Ha nem adunk meg sorszámot, akkor az előző sortól 10-esével növekszik majd automatikusan a sorszám.
2 - Címkék használhatósága. Sorszám helyett lehetőség van címkék megadására. Egy címke {} karakterek közötti szóköz nélküli karaktersorozat.
Például: {CIMKE} GOTO {CIMKE}
Mind a sorszám megadásának helyén, mind a sorszámra való hivatkozás helyén használható címke. A címkében nem csak a homelab karakterei használhatóak.
3 - Speciális megjegyzásek. A homelab 2 a '" karaktersorozatoto használja a megjegyzések jelölésére. A konverter a megjegyzés utáni első karaktert is figyelembe veszi, és ha az a felkiáltó jel, akkor a
megjegyzést automatikusan kihagyja a konverzióból. Annak tartalma már nem kerül a htp fájlba. Emiatt a '"! megjegyzés jel után nem csak a homelab karaktereit használhatjuk.

Mivel az így elkészített forráskód már erősen eltérhet a gépben futó végleges GASIC programtól, ezért a konverziónál megadható a "-B basicfile" paraméter, ahol a basicfajl annak a fájlnak a neve, amibe a 
konverter a végleges htp-be kerülő BASIC programot belementi. Így betöltés nélkül is látható a htp fájl BASIC tartalma.

További rekord is megadható a -b paraméterrel. Ez egy közvetlen memóriatartalom, ami a kezdőcímtől kezdődően töltődik be. A kezdőcímet a -L paraméterrel adhatja meg decimális számokkal. Ha nincs megadva, 
alapértelmezett értéke 0x6000.

## htp2h2wav
Homelab 2 kazettára menthető hangfájlját generálja, akár több-blokkos htp fájlból is. Egy bitet a dokumentáció szerinti 1.6ms időtartammal menti le, bár várhatóan ennél rövidebb bitekkel is működne a betöltés. 
Ez későbbi fejlesztési lehetőség lesz.

## h2bas2htp
Elavult egyszerű BASIC->htp konverter. Használata már nem javasolt. A h2CreateHtp váltja fel.

## htpinfo
Minimális htp információ megjelenítése egy htp fájlról.
