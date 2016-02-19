#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char** split_command( char const* const );
char*** spilt_pipes( char const* const, unsigned int* );
void free_command( char** );
unsigned int count_words( char const* const );
unsigned int count_pipes( char const* const );


unsigned int count_words( char const* const string )
{
    unsigned int counter = 1;
    for( unsigned int i = 0; i < strlen( string ); ++ i )
    {
        if( string[ i ] == ' ' )
            ++counter;
    }

    return counter;
}

unsigned int count_pipes( char const* const string )
{
    unsigned int counter = 0;
    for( unsigned int i = 0; i < strlen( string ); ++i )
    {
        if( string[ i ] == '|' )
            ++counter;
    }

    return counter;
}

char*** spilt_pipes( char const* const piped_commands, unsigned int* commands_number )
{
    char* tmp = strdup( piped_commands );

    *commands_number = count_pipes( piped_commands ) + 1;
    char*** commands_array = malloc( *commands_number * sizeof( char** ) );

    unsigned int last_pos = 0;
    unsigned int counter = 0;

    unsigned int i = 0;
    for( ; tmp[ i ] != '\0'; ++i )
    {
        if( piped_commands[ i ] == '|' )
        {
            char* tmp_command;
            if( counter == 0 )
            {
                tmp_command = strndup( tmp, i );
            }
            else
            {
                tmp_command = strndup( tmp + last_pos + 1, i - last_pos - 1 );
            }

            commands_array[ counter ] = split_command( tmp_command );
            last_pos = i;
            ++counter;
            free( ( char* ) tmp_command );
        }
    }

    char* tmp_command;
    if( counter == 0 )
        tmp_command = strndup( tmp, i );
    else
        tmp_command = strndup( tmp + last_pos + 1, i - last_pos );

    commands_array[ counter ] = split_command( tmp_command );

    free( tmp_command );
    free( tmp );

    return commands_array;
}

char** split_command( char const* const command )
{
    char* tmp = strdup( command );
    unsigned int const buffer_len = count_words( tmp ) + 1;
    unsigned int const buffer_size = buffer_len * sizeof( char* );
    char** parsed  = malloc( buffer_size );
    memset( parsed, '\0', buffer_size );

    char* token = NULL;
    char* save_ptr = NULL;
    unsigned int i = 0;

    for( token = strtok_r( tmp, " ", &save_ptr ); token; token = strtok_r( NULL, " ", &save_ptr ) )
    {
        parsed[ i ] = strdup( token );
        i++;
    }

    free( tmp );
    return parsed;
}

void free_command( char** command )
{
    for( int i = 0; command[ i ] != NULL; ++i )
        free( command[ i ] );

    free( command );
}

int main( void )
{
    struct sigaction sigint_act_new;
    struct sigaction sigint_act_old;

    memset( &sigint_act_new, '\0', sizeof( sigint_act_new ) );
    sigemptyset( &sigint_act_new.sa_mask );
    sigint_act_new.sa_handler = SIG_IGN;

    if( sigaction( SIGINT, &sigint_act_new, &sigint_act_old ) )
    {
        perror( "sigaction" );
        exit( EXIT_FAILURE );
    }

    char* command_with_args = NULL;
    ssize_t command_len = 0;
    bool background_command = false;
    size_t size;

    while( 1 )
    {
        printf( "lsh> " );
        if( ( command_len = getline( &command_with_args, &size, stdin ) ) == -1 )
        {//TODO: replace by fread to better handle EOF
            if( errno != 0 )
            {
                perror( "getline" );
            }
            break; //TODO: return code
        }

        if( command_len == 1 ) //ignore enter
            continue;

        command_with_args[ command_len - 1 ] = '\0'; //remove \n

        if( strcmp( command_with_args, "exit" ) == 0 )
        {
            break; //return code
        }

        if( command_with_args[ command_len - 2 ] == '&' )
        {
            background_command = true;
            command_with_args[ command_len - 2 ] = '\0';
        }
        else
            background_command = false;

        unsigned int array_size;
        char*** splited_commands_array = spilt_pipes( command_with_args, &array_size );
        assert( splited_commands_array[ 0 ] );


        int input, fd[ 2 ];
        input = STDIN_FILENO;

        for( unsigned int i = 0; i < array_size; ++i )
        {
            pipe( fd );

            pid_t child_pid;

            if( ( child_pid = fork() ) == 0 )
            {
                if( i + 1 != array_size )
                {
                    dup2( input, STDIN_FILENO );
                    dup2( fd[ STDOUT_FILENO ], STDOUT_FILENO );
                }
                else
                    dup2( input, STDIN_FILENO );

                sigaction( SIGINT, &sigint_act_old, NULL );
                execvp( splited_commands_array[ i ][ 0 ], splited_commands_array[ i ] );
            }
            else
            {
                assert( child_pid != -1 );

                free_command( splited_commands_array[ i ] );

                int status;
                if( background_command && i + 1 == array_size )
                {
                    long finished_pid = waitpid( -1, &status, WNOHANG );

                    while( finished_pid > 0 )
                    {//BACKGROUND
                        fprintf( stdout, "%ld exited normally\n", ( long )finished_pid );
                        finished_pid = waitpid( -1, &status, WNOHANG );
                    }
                }
                else {
                    waitpid( child_pid, &status, 0 );
                }
            }

            close( fd[ STDOUT_FILENO ] );
            input = fd[ STDIN_FILENO ];
        }

        free( splited_commands_array );
    }

    free( command_with_args );

    return EXIT_SUCCESS;
}

