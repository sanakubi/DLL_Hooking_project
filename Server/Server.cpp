#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <sqlite3.h>

sqlite3* db;
int rc;
//SOCKET client;

static int callback(void*, int, char**, char**);
void client_handle(SOCKET);

int main(int argc, char* argv[]){
    char* zErrMsg = 0;
    const char* sql;

    //** Open database **//

    const char* dir = R"(C:\\Data Bases\\SQLlite\\logs.db)";

    rc = sqlite3_open(dir, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    } else fprintf(stderr, "Opened database successfully\n");

    //** Create or ? table **//

    sql = "CREATE TABLE LOGS("  \
          "FUNCTION     TEXT    NOT NULL," \
          "TIME         CHAR(20) );";

    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL message: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else fprintf(stdout, "Table created successfully\n");

    //** Network settings**//

    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(1, 2);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        printf("Error. Server is not running. \n");
    } else printf("Server started.\n");

    SOCKADDR_IN addr;
    int addr_ = sizeof(addr);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //Прослушка на любой адресс, расчитываем на прямую видимость; заменен с inet_addr("127.0.0.1");
    addr.sin_port = htons(708);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);

    //** Network connection**//

    SOCKET client;
    client = accept(sListen, (SOCKADDR*)&addr, &addr_);
    printf("Accepted.\n");
    if (!client) {
        printf("Error.\n");
    } else {
        //CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)client_handle, NULL, NULL, NULL);
        client_handle(client);
    }

    //** Temp code **// 
    /* */
    sql = "SELECT * from LOGS";

    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else fprintf(stdout, "Records created successfully\n");
    /* */
    //** Temp code **///

    sqlite3_close(db); 
    closesocket(client);
    return 0;
}

 static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
     int i;
     for (i = 0; i < argc; i++) {
         printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
     }
     printf("\n");
     return 0;
 }

 void client_handle(SOCKET client) {
     const char* sql;
     char* zErrMsg = 0;

     char msg[256];

     while (1) {
         if (SOCKET_ERROR == recv(client, msg, sizeof(msg), NULL)) {
             std::cout << "Error receiving " << WSAGetLastError() << std::endl;
             break;
         }

         std::string temp = msg;
         std::string func = temp.substr(0, temp.find("#") - 1);
         std::string data = temp.substr(temp.find("#") + 1);
         std::cout << func << " " << data << std::endl;

         std::string tempsql = "INSERT INTO LOGS (FUNCTION, TIME) VALUES ('" + func + "', '" + data + "')";

         sql = const_cast<char*>(tempsql.c_str());

         rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
         if (rc != SQLITE_OK) {
             fprintf(stderr, "SQL message: %s\n", zErrMsg);
             sqlite3_free(zErrMsg);
         } else fprintf(stdout, "Records created successfully\n");
     }
 } 
