Program (Server oraz Client) mozna uruchomic bez parametrow. Wowczas zostana im przypisane domyslne ustawienia
tj.: 
host: localhost 
port: 5566

Poza tym, program mozna uruchomic z parametrami.
W przypadku klienta 2 parametry, pierwszy host, drugi port.
W przypadku serwera 1 parametr, port.

Program zostal napisany do obslugi wielu watkow. Program wiec obsluguje komunikacje dla wielu uzytkownikow.
Mozliwe dzialania:

- wyslanie komunikatu do wszystkich uzytkownikow (w komendzie wiadomosc do wyslania)
Przyklad uzycia:
ta wiadomosc trafia do wszystkich

- wyslanie komunikatu do konkretnego uzytkownika (w komendzie nick odbiorcy, i wiadomosc, oddzielone ":")
Przyklad uzycia:
nick: ta wiadomosc trafi do uzytkownika o nicku nick

- wyjscie
Przyklad uzycia:
EXIT

- wypisanie aktywnych uzytkownikow
Przyklad uzycia:
LIST