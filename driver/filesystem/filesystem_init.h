#pragma once

#ifdef __cplusplus
extern "C" {
#endif


// Init allocates the VFS and DevFS objects,
// mounts the default folders,
// and initializes device nodes.
void filesystem_Init();

// Ls lists the contents of a folder to io_Printf.
void filesystem_Ls(const char *path);


#ifdef __cplusplus
}
#endif
