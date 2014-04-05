#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "glib.h"
#include "markdown_peg.h"

int main(int argc, char *argv[])
{
    char* output = markdown_to_string("*it*", 0, 0);
    printf("%s\n", output);
    free(output);
}
