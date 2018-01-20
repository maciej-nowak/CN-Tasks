#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include "c4z2res.h"

//MACIEJ NOWAK 384013

const int tak = 1;
const int nie = 0;
unsigned short nrportu; //number port to listening from command line
HWND okno; //main window of program
#define WM_POLACZENIE (WM_USER + 100) //message for FD_ACCEPT event in WSAAsyncSelect
SOCKET listen_socket; //main socket listening

/************** FUNKCJE DO WYKORZYSTANIA ******************/

int czy_akceptujesz_polaczenie(const char *odkogo, struct sockaddr_in *adr);
void dodaj_komunikat(const char *odkogo, const char *komunikat);

/************** FUNKCJE DO ZAIMPLEMENTOWANIA ******************/

int wystartuj_gniazdo(HWND okno_programu, unsigned short port)
{
    struct sockaddr_in host; //socket structure for user
    listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //making the socket for IPv4 familly and TCP protocol
	host.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY special address, allows working without knowing IP address of machine
    host.sin_family = AF_INET; //address family = AF_INET for IPv4
    host.sin_port = htons(port); //port
    
	//trying bind 
	if(bind(listen_socket, (struct sockaddr*)&host, sizeof(host)) < 0)
	{
		printf("Bind failed \n");
        return 0;	
	}
	
	//trying listening
    if(listen(listen_socket, 10) < 0) //2 parametr is backlog
	{
		printf("Listening failed \n");
		return 0;	
	}
	
	//waiting for accept 
    WSAAsyncSelect(listen_socket, okno_programu, WM_POLACZENIE, FD_ACCEPT);
    return 1;
}

void obsluz_polaczenie(void)
{
	char nick[1024]; //store nick of sender
    char message[1024]; //store message of sender 
    long msgLength; //store length of message, uses by ntohl
	long nickLength; //store length of nick, uses by ntohl
    long partReceive, allReceive; //store temporary values
	struct sockaddr_in host; //socket structure for user
	int addressLength = sizeof(struct sockaddr_in); //address length
    SOCKET socketHost;

	//trying accept from socket
    if((socketHost = accept(listen_socket, (struct sockaddr*)&host, &addressLength)) < 0)
	{
		printf("Socket invalid \n");
        return;
	}
	
	//receiving length of the nick
    if(recv(socketHost, (char*)&nickLength, sizeof(long), 0) != sizeof(long))
    {
		printf("Receiving message failed \n");
        closesocket(socketHost);
        return;
    }
    nickLength = ntohl(nickLength); //convert values from network to host, from native byte to network byte order
    memset(nick, 0, 1024); //clearing nick
    allReceive = 0;
	
	//receiving the nick
    while(allReceive < nickLength)
    {
        partReceive = recv(socketHost, nick + (char)allReceive, nickLength - allReceive, 0);
        if (partReceive < 0) 
        {
			printf("Receiving message failed \n");
            closesocket(socketHost);
            return;
        }
        allReceive = allReceive + partReceive;
    }
    
	//trying accept connection
    if(czy_akceptujesz_polaczenie(nick, &host) == 0)
    {
		printf("Accept connection failed \n");
        closesocket(socketHost);
        return;
    }

	//receiving length of the message
    if(recv(socketHost, (char*)&msgLength, sizeof(long), 0) != sizeof(long))
    {
		printf("Receiving message failed \n");
        closesocket(socketHost);
        return;
    }
    msgLength = ntohl(msgLength); //convert values from network to host, from native byte to network byte order
    memset(message, 0, 1024); //clearing message
    allReceive = 0;
	
	//receiving the message
    while(allReceive < msgLength)
    {
        partReceive = recv(socketHost, message + (char)allReceive, msgLength - allReceive, 0);
        if(partReceive < 0) 
        {
			printf("Receiving message failed \n");
            closesocket(socketHost);
            return;
        }
        allReceive = allReceive + partReceive;
    }

    closesocket(socketHost);
    dodaj_komunikat(nick, message); //adding message to box
}

int wyslij_wiadomosc(struct sockaddr_in *odbiorca, char *mojnick, const char *msg)
{
	long nickLength = (long)strlen(mojnick); //store length of nick from parameter (in long)
    long msgLength = (long)strlen(msg); //store length of message from parameter (in long)
	int nickStringLength = (int)strlen(mojnick);
	int msgStringLength = (int)strlen(msg);
    SOCKET socketHost;
	
	nickLength = htonl(nickLength); //convert values from network to host, from native byte to network byte order
	msgLength = htonl(msgLength);
	
	//trying accept from socket	
    if((socketHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
	{
		printf("Socket invalid \n");
        return 0;
	}
	
	//trying connection
    if(connect(socketHost, (sockaddr*)odbiorca, sizeof(struct sockaddr_in)) < 0)
    {
		printf("Connection failed \n");
        closesocket(socketHost);
        return 0;
    }
    
	//trying send length of nick
    if(send(socketHost, (const char*)&nickLength, sizeof(long), 0) != sizeof(long))
    {
		printf("Sending message failed \n");
        closesocket(socketHost);
        return 0;
    }
	
	//trying send nick
    if(send(socketHost, mojnick, nickStringLength, 0) != nickStringLength)
    {
		printf("Sending message failed \n");
        closesocket(socketHost);
        return 0;
    }
	
	//trying send length of the message
    if(send(socketHost, (const char*)&msgLength, sizeof(long), 0) != sizeof(long))
    {
		printf("Sending message failed \n");
        closesocket(socketHost);
        return 0;
    }
	
	//trying send the message
    if(send(socketHost, msg, msgStringLength, 0) != msgStringLength)
    {
		printf("Sending message failed \n");
        closesocket(socketHost);
        return 0;
    }
    
    closesocket(socketHost);
    return 1;
}


/************************* PONIZEJ PROSZE JUZ NIC NIE ZMIENIAC *************/

BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    INITCOMMONCONTROLSEX cc;
    WORD wersja;
    WSADATA d;

    nrportu = atoi(lpCmdLine);
    if (nrportu < 1025 || nrportu > 65535) {
        MessageBox(NULL,
            "Podano nieprawid³owy numer portu lub nie podano go wcale.",
            "MINIGADU",
            MB_ICONERROR
            );
        return 1;
    }

    wersja = MAKEWORD(2,0);
    WSAStartup(wersja, &d);

    memset(&wc,0,sizeof(wc));
    wc.lpfnWndProc = DefDlgProc;
    wc.cbWndExtra = DLGWINDOWEXTRA;
    wc.hInstance = hinst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszClassName = "minigadu";
    RegisterClass(&wc);
    memset(&cc,0,sizeof(cc));
    cc.dwSize = sizeof(cc);
    cc.dwICC = 0xffffffff;
    InitCommonControlsEx(&cc);

    DialogBox(hinst, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, (DLGPROC) DialogFunc);

    WSACleanup();
    return 0;

}

int InitializeApp(HWND hDlg,WPARAM wParam, LPARAM lParam)
{
    okno = hDlg;
    return wystartuj_gniazdo(hDlg, nrportu);
}


/*
Deklaracja procedur, ktore sa zaimplementowane po petli
komunikatow, a sa w niej wywolywane.
*/
void wyczysc_clicked(void);
void wyslij_clicked(void);
void dodajp_clicked(void);
void usunp_clicked(void);

/*
Obsluga petli komunikatow okna.
*/
BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        if (InitializeApp(hwndDlg,wParam,lParam) == 0) exit(1);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDC_WYCZYSC:
                wyczysc_clicked();
                break;
            case IDC_DODAJP:
                dodajp_clicked();
                break;
            case IDC_USUNP:
                usunp_clicked();
                break;
            case IDC_WYSLIJ:
                wyslij_clicked();
                break;
        }
        break;
    case WM_POLACZENIE:
        obsluz_polaczenie();
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg,0);
        return TRUE;

    }
    return FALSE;
}

int liczba_przyjaciol(void)
{
    return (int) SendMessage(
        GetDlgItem(okno,IDC_PRZYJACIELE),
        LB_GETCOUNT,
        (WPARAM) 0,
        (LPARAM) 0
        );
}

int czy_jest_nick_na_liscie(const char *nick)
{
    int i;
    char buf[256];
    char tmpnick[256];
    char tmphost[256];
    int tmpport;
    for (i = 0; i < liczba_przyjaciol(); i++) {
        SendMessage(
            GetDlgItem(okno, IDC_PRZYJACIELE),
            LB_GETTEXT,
            (WPARAM) i,
            (LPARAM) buf
            );
        sscanf(buf,"%s",tmpnick);
        sscanf(buf+strlen(tmpnick)+3,"%s",tmphost);
        sscanf(buf+strlen(tmpnick)+strlen(tmphost)+6,"%d",&tmpport);
        if (! strcmp(tmpnick,nick)) return tak;
    }
    return nie;
}

void dodaj_przyjaciela(const char *nick, struct sockaddr_in *adr)
{
    char przyjaciel[512];
    sprintf(przyjaciel,"%s : %s : %u",
        nick,
        inet_ntoa(adr->sin_addr),
        ntohs(adr->sin_port)
        );
    SendMessage(
        GetDlgItem(okno,IDC_PRZYJACIELE),
        LB_ADDSTRING,
        (WPARAM) 0,
        (LPARAM) przyjaciel
        );
}

void usun_przyjaciela(void)
{
    int ktory;
    ktory = (int) SendMessage(
        GetDlgItem(okno, IDC_PRZYJACIELE),
        LB_GETCURSEL,
        (WPARAM) 0,
        (LPARAM) 0
        );
    if (ktory < 0) {
        MessageBox(
            NULL,
            "Nale¿y najpierw wskazaæ na liœcie przyjaciela do usuniêcia.",
            "MINIGADU",
            MB_OK|MB_ICONSTOP
            );
        return;
    }
    SendMessage(
        GetDlgItem(okno, IDC_PRZYJACIELE),
        LB_DELETESTRING,
        (WPARAM) ktory,
        (LPARAM) 0
        );
}

int czy_akceptujesz_polaczenie(const char *odkogo, struct sockaddr_in *adr)
{
    int jest;
    int decyzja;
    char buf[512];
    jest = czy_jest_nick_na_liscie(odkogo);
    if (jest) return tak;
    sprintf(buf,
        "Nadawcy %s nie ma wœród przyjació³. Czy chcesz przyj¹æ wiadomoœæ?",
        odkogo
        );
    decyzja = MessageBox(
        NULL,
        buf,
        "MINIGADU",
        MB_YESNO|MB_ICONQUESTION
        );
    if (decyzja == IDNO) return nie;
    else return tak;
}

void wyczysc_clicked(void)
{
    SendMessage(
        GetDlgItem(okno, IDC_WIADOMOSCI),
        LB_RESETCONTENT,
        (WPARAM) 0,
        (LPARAM) 0
        );
}

void usunp_clicked(void)
{
    if (liczba_przyjaciol() < 1)
        MessageBox(
            NULL,
            "Twoja lista przyjació³ jest pusta.",
            "MINIGADU",
            MB_ICONERROR
        );
    else usun_przyjaciela();
}

void dodajp_clicked(void)
{
    char nick[256];
    char host[256];
    char port[256];
    char buf[512];
    int p;
    struct hostent *he;
    struct sockaddr_in adr;

    GetDlgItemText(okno, IDC_NICKP, nick, 256);
    GetDlgItemText(okno, IDC_HOSTP, host, 256);
    GetDlgItemText(okno, IDC_PORTP, port, 256);

    if (strlen(nick)<1) {
        MessageBox(NULL, "Nie podano nick'a przyjaciela.",
            "MINIGADU", MB_ICONERROR);
        return;
    }

    p = atoi(port);
    if (p < 1025 || p > 65535) {
        MessageBox(NULL, "Numer portu jest nieprawid³owy.",
            "MINIGADU", MB_ICONERROR);
        return;
    }

    he = gethostbyname(host);
    if (he == NULL) {
        sprintf(buf,
            "Nazwa/adres hosta %s jest nieprawid³owa(y).",
            host);
        MessageBox(NULL, buf, "MINIGADU", MB_ICONERROR);
        return;
    }
    adr.sin_port = htons(p);
    adr.sin_addr = *((struct in_addr*) he->h_addr);

    if (czy_jest_nick_na_liscie(nick)) {
        sprintf(buf,
            "Przyjaciel o nick'u %s ju¿ znajduje siê na Twojej liœcie.",
            nick);
        MessageBox(NULL, buf, "MINIGADU", MB_ICONERROR);
        return;
    }

    dodaj_przyjaciela(nick, &adr);
}

void wyslij_clicked(void)
{
    char mojnick[256];
    char wiadomosc[512];
    struct sockaddr_in adr;
    int wybranyp;
    char przyjaciel[512];
    char nickp[128];
    char hostp[128];
    int portp;
    int wynik;

    GetDlgItemText(okno, IDC_MOJNICK, mojnick, 256);
    mojnick[255] = '\0';

    if (strlen(mojnick)<1) {
        MessageBox(NULL, "Musisz wprowadziæ swój nick.",
            "MINIGADU", MB_ICONERROR);
        return;
    }

    GetDlgItemText(okno, IDC_WIADOMOSC, wiadomosc, 512);
    wiadomosc[511] = '\0';

    if (strlen(wiadomosc) < 1) {
        MessageBox(NULL, "Nie mo¿esz wys³aæ pustej wiadomoœci.",
            "MINIGADU", MB_ICONERROR);
        return;
    }

    if (liczba_przyjaciol() < 1) {
        MessageBox(NULL, "Musisz mieæ co najmniej jednego przyjaciela.",
            "MINIGADU", MB_ICONERROR);
        return;
    }

    wybranyp = (int) SendMessage(
        GetDlgItem(okno, IDC_PRZYJACIELE),
        LB_GETCURSEL,
        (WPARAM) 0,
        (LPARAM) 0
        );
    if (wybranyp < 0) {
        MessageBox(NULL,
            "Wybierz przyjaciela, do którego chcesz wys³aæ wiadomoœæ.",
            "MINIGADU", MB_ICONERROR);
        return;
    }

    SendMessage(
        GetDlgItem(okno, IDC_PRZYJACIELE),
        LB_GETTEXT,
        (WPARAM) wybranyp,
        (LPARAM) przyjaciel
        );
    sscanf(przyjaciel,"%s",nickp);
    sscanf(przyjaciel+strlen(nickp)+3,"%s",hostp);
    sscanf(przyjaciel+strlen(nickp)+strlen(hostp)+6,"%d",&portp);
    adr.sin_family = AF_INET;
    adr.sin_port = htons(portp);
    adr.sin_addr.s_addr = inet_addr(hostp);
    wynik = wyslij_wiadomosc(&adr, mojnick, wiadomosc);
    if (wynik == nie) {
        MessageBox(NULL,
            "Wys³anie wiadomoœci nie powiod³o siê.",
            "MINIGADU",
            MB_ICONERROR
            );
    }
    dodaj_komunikat(mojnick, wiadomosc);
}

void dodaj_komunikat(const char *odkogo, const char *komunikat)
{
    char buf[512];
    sprintf(buf,"%s mowi: %s",odkogo,komunikat);
    SendMessage(
        GetDlgItem(okno, IDC_WIADOMOSCI),
        LB_ADDSTRING,
        (WPARAM) 0,
        (LPARAM) buf
        );
}

