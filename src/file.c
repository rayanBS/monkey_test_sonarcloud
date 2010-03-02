/*-*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2010, Eduardo Silva P.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "monkey.h"
#include "file.h"
#include "user.h"
#include "memory.h"

struct file_info *mk_file_get_info(char *path)
{
    struct file_info *f_info;
    struct stat f, target;

    /* Stat right resource */
    if (lstat(path, &f) == -1) {
        return NULL;
    }

    f_info = mk_mem_malloc(sizeof(struct file_info));
    f_info->is_link = MK_FILE_FALSE;
    f_info->is_directory = MK_FILE_FALSE;
    f_info->exec_access = MK_FILE_FALSE;
    f_info->read_access = MK_FILE_FALSE;

    if (S_ISLNK(f.st_mode)) {
        f_info->is_link = MK_FILE_TRUE;
        if (stat(path, &target) == -1) {
            return NULL;
        }
    }
    else {
        target = f;
    }

    f_info->size = target.st_size;
    f_info->last_modification = target.st_mtime;

    if (S_ISDIR(target.st_mode)) {
        f_info->is_directory = MK_FILE_TRUE;
    }

    /* Checking read access */
    if ((target.st_mode & S_IRUSR && target.st_uid == euid) ||
        (target.st_mode & S_IRGRP && target.st_gid == egid) ||
        (target.st_mode & S_IROTH)) {
        f_info->read_access = MK_FILE_TRUE;
    }

    /* Checking execution access */
    if ((target.st_mode & S_IXUSR && target.st_uid == euid) ||
        (target.st_mode & S_IXGRP && target.st_gid == egid) ||
        (target.st_mode & S_IXOTH)) {
        f_info->exec_access = MK_FILE_TRUE;

    }
    return f_info;
}

/* Read file content to a memory buffer,
 * Use this function just for really SMALL files
 */
char *mk_file_to_buffer(char *path)
{
    FILE *fp;
    char *buffer;
    long bytes;
    struct file_info *finfo;

    if (!(finfo = mk_file_get_info(path))) {
        return NULL;
    }

    if (!(fp = fopen(path, "r"))) {
        return NULL;
    }

    buffer = calloc(finfo->size + 1, sizeof(char));
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    bytes = fread(buffer, finfo->size, 1, fp);

    if (bytes < 1) {
        mk_mem_free(buffer);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return (char *) buffer;

}
