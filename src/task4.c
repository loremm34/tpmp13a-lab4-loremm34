#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    for(int i = 0; i < argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int readBlobFromFile(const char* filename, char** buffer, int* size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Ошибка открытия файла %s\n", filename);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);
    *buffer = (char*)malloc(*size);
    if (!*buffer) {
        fprintf(stderr, "Ошибка выделения памяти для чтения файла\n");
        fclose(file);
        return 0;
    }
    fread(*buffer, 1, *size, file);
    fclose(file);
    return 1;
}

int main(int argc, char* argv[]) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    
    rc = sqlite3_open("company.db", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    int choice = 0;
    do {
        printf("1. Показать всех сотрудников\n");
        printf("2. Добавить нового сотрудника\n");
        printf("3. Удалить сотрудника по ID\n");
        printf("4. Выйти\n");
        printf("Выберите операцию: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                rc = sqlite3_exec(db, "SELECT * FROM Employees;", callback, 0, &zErrMsg);
                if( rc != SQLITE_OK ){
                    fprintf(stderr, "SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                }
                break;
            case 2: {
                printf("Введите имя сотрудника: ");
                char firstName[50];
                scanf("%s", firstName);
                printf("Введите фамилию сотрудника: ");
                char lastName[50];
                scanf("%s", lastName);
                printf("Введите зарплату сотрудника: ");
                double salary;
                scanf("%lf", &salary);
                char* photoData = NULL;
                int photoSize = 0;
                printf("Введите путь к файлу с фотографией: ");
                char photoPath[100];
                scanf("%s", photoPath);
                if (readBlobFromFile(photoPath, &photoData, &photoSize)) {
                    sqlite3_stmt *stmt;
                    const char *sql = "INSERT INTO Employees (FirstName, LastName, Salary, Photo) VALUES (?, ?, ?, ?);";
                    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
                    if (rc != SQLITE_OK) {
                        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
                    } else {
                        sqlite3_bind_text(stmt, 1, firstName, -1, SQLITE_STATIC);
                        sqlite3_bind_text(stmt, 2, lastName, -1, SQLITE_STATIC);
                        sqlite3_bind_double(stmt, 3, salary);
                        sqlite3_bind_blob(stmt, 4, photoData, photoSize, SQLITE_STATIC);
                        rc = sqlite3_step(stmt);
                        if (rc != SQLITE_DONE) {
                            fprintf(stderr, "Ошибка выполнения запроса: %s\n", sqlite3_errmsg(db));
                        } else {
                            printf("Сотрудник успешно добавлен.\n");
                        }
                        sqlite3_finalize(stmt);
                    }
                    free(photoData);
                }
                break;
            }
            case 3:
                printf("Введите ID сотрудника для удаления: ");
                int id;
                scanf("%d", &id);
                sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
                sqlite3_exec(db, "DELETE FROM Employees WHERE ID = %d;", 0, 0, &zErrMsg);
                sqlite3_exec(db, "COMMIT;", 0, 0, 0);
                printf("Сотрудник успешно удален.\n");
                break;
            case 4:
                break;
            default:
                printf("Неверный выбор. Пожалуйста, выберите существующий пункт меню.\n");
        }
    } while(choice != 4);

    sqlite3_close(db);
    return 0;
}
