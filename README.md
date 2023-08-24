# homelab2utils
Segédprogramok Homelab2 (Aircomp 16) számítógéphez készülő programokhoz
h2CreateHtp, htp2h2wav, htpinfo, h2bas2htp
A Windows programok a win32 mappában, a linux parancsok a bin mappában találhatók.

## h2CreateHtp
1 vagy 2 blokkos htp fájl generálására képes Homelab 2 emulátorok számára, vagy a WAV generálás forrásának. Lehet egyszerű BASIC forrásfájl htp konvertálására, vagy megadható második modulnak 
egy gépikódú memóriablokk is. BASIC forrásokban használható a sorszámnélküli mód is, ahol címkéket kell a sorszámok ehlyett írni. A címkék {} karakterek közé kerülnek.

## htp2h2wav
Homelab 2 kazettára menthető hangfájlját generálja, akár több-blokkos htp fájlból is. Egy bitet a dokumentáció szerinti 1.6ms időtartammal menti le, bár várhatóan ennél rövidebb bitekkel is működne a betöltés. 
Ez későbbi fejlesztési lehetőség lesz.

## h2bas2htp
Elavult egyszerű BASIC->htp konverter. Használata már nem javasolt. A h2CreateHtp váltja fel.

## htpinfo
Minimális htp információ megjelenítése egy htp fájlról.
