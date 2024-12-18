#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <windows.h>
#include <ctype.h>
// ansi colours
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define MAGENTA "\033[35m"
int main(int argc, char *argv[])
{
    // declare passing arguments
    // technically the executable name is argv[0] so it doesn't realy count
    char *username = argv[1];
    char *db = argv[2];
    char *passwd = argv[3];
    int i = 0;
    char textbuffer[512] = {0};
    char full_path[512];
    int result = 0;
    char continuar[1];
    char error_buffer[512];
    if (argc == 4)
    {
        printf(GREEN "Parametros correctos! Ejecutando...\n" RESET);
    }
    else if (argc > 4)
    {
        printf(RED "Demasiados argumentos.\n" RESET);
        printf("pSQLexec\n");
        printf("Uso: psqlexec <usuario_db> <nombre_db> <contra_db>");
        return 1;
    }
    else
    {
        printf(RED "Faltan argumentos.\n" RESET);
        printf("pSQLexec\n");
        printf("Uso: psqlexec <usuario_db> <nombre_db> <contra_db>");
        return 1;
    }
    DIR *currentDir = opendir(".");
    // validate if the directory is not null
    if (currentDir == NULL)
    {
        perror(RED "Directorio inválido." RESET);
        return EXIT_FAILURE;
    }
    // The dirent structure in C, defined in the <dirent.h> header file, represents a single directory entry when reading the contents of a directory. This structure is typically used with functions like opendir(), readdir(), and closedir() to work with directories and their files.
    // accordingly, the "entry" variable uses the current directory and its files.
    struct dirent *entry;
    // The stat() function in C is used to retrieve information about a file or directory. It fills a struct stat with details about the file, such as its size, permissions, modification time, and file type. This information can then be used to determine various properties of the file or directory.
    // in this case, we'll use it to populate entry.
    struct stat file_stat;
    while ((entry = readdir(currentDir)) != NULL)
    {

        snprintf(full_path, sizeof(full_path), "%s/%s", ".", entry->d_name);

        // time to start fuckin around with C pseudo-objects!!!
        // validation if stat got all the info from files and could store it on file_stat
        if (stat(full_path, &file_stat) == 0)
        {

            // btw the difference between using . and -> to reference an attribute from a struct is that . uses it to reference directly and -> uses it to reference a pointer
            //  to human: if the found object is a regular file AND strstr finds .sql files... do:

            if (S_ISREG(file_stat.st_mode) && (strstr(entry->d_name, ".sql")) != NULL)
            {
                // sprintf stores formatted values a-la-printf on a string. it takes 3 values: first value takes the char array that will store the string, the second takes the size of the string, and the third is the actual string that will get stored.
                snprintf(textbuffer, sizeof(textbuffer),
                         "psql --dbname=postgresql://%s:%s@localhost:5432/%s -f \"%s\" 2>&1 1>nul",
                         username, passwd, db, entry->d_name);
                // popen opens a pipe to or from a command. It returns a pointer to a stream that can be used to either read from or write to the pipe.
                // what is a pipe, you may ask? well, a pipe is like a form of redirection that is used in Linux and other Unix-like operating systems to send the output of one command/program/process to another command/program/process for further processing. (also works for winblows ;) )
                // to do all of the things we are going to pull off, first we redirect psql's stderr to our stdout, so we can read it and print it out.
                FILE *pipe = popen(textbuffer, "r");
                // you may ask, where's pipe pointing to? well, it's pointing to the output of the command we just ran.
                // popen gets 2 parameters: the string command we're storing in textbuffer, and the way we're opening the stream, in this case, read only.
                // if the pipe is null, it means that the command blew up, so we print an error message and return an exit failure.
                if (!pipe)
                {
                    perror(RED "Error al abrir el pipe." RESET);
                    return EXIT_FAILURE;
                }
                bool success = true;
                // if the program gets something from psql's stderr, it'll halt the program and print the error message.
                while (fgets(error_buffer, sizeof(error_buffer), pipe) != NULL)
                {
                    printf(RED "Error: %s" RESET, error_buffer);
                    success = false;
                }
                result = pclose(pipe);
                if (success && result == 0)
                {
                    printf(GREEN "%s procesado correctamente.\n" RESET, entry->d_name);
                    i++;
                }
                else
                {
                    printf("%s no se pudo procesar...\n", entry->d_name);
                    while (continuar[0] != 's' && continuar[0] != 'n')
                    {
                        printf("continuar ejecutando? (S/N): \n");
                        scanf(" %c", &continuar[0]);
                        continuar[0] = tolower(continuar[0]);
                        if (continuar[0] == 's')
                        {
                            printf("bajo su propia discreción.\n");
                            break;
                        }
                        else if (continuar[0] == 'n')
                        {
                            (i > 1) ? printf("%d archivos procesados antes de llegar al error.", i) : printf("%d archivo procesado antes de llegar al error.", i);
                            return 0;
                        }
                        else
                        {
                            printf("elige una opción válida\n");
                        }
                    }
                }
            }
        }
        else
        {
            perror("paila caravana, stat se explotó");
        }
    }
    (i > 1) ? printf("%d archivos procesados.", i) : printf("%d archivo procesado.", i);
}
