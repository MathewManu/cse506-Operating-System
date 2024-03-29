#include <sys/tarfs.h>
#include <sys/defs.h>
#include <sys/utils.h>
#include <sys/kprintf.h>
#include <sys/dir.h>
#include <sys/vmm.h>

#define DUMP_TARFS_TREE {\
              kprintf("TARFS Tree : \n");\
              dump_tarfs_tree(tarfs_tree, 1);\
             }

/* tarfs file structure tree */
file_t *tarfs_tree;

/*
 * Dumps the tarfs file structure tree
 * When invoked, the value for 'level' should be 1
 */
void dump_tarfs_tree(file_t *temp, int level) {

  if (temp) {
    int j = level;
    while(j--) {
      kprintf("   ");
    }

    kprintf("%s\n", temp->file_name);

    int i = 0;
    while (i < temp->num_children) {
      dump_tarfs_tree(temp->child_node[i], level + 1);
      i++;
    }
  }
}

uint64_t get_size_tar_octal(char *data, size_t size) {
  char *curr = (char *)data + size;
  char *ptr = curr;
  uint64_t sum = 0;
  uint64_t multiply = 1;

  while (ptr >= (char *) data) {
    if ((*ptr) == 0 || (*ptr) == ' ') {
      curr = ptr - 1;
    }
    ptr--;
  }

  while (curr >= (char *) data) {
    sum += ASCII_TO_NUM(*curr) * multiply;
    multiply *= 8;
    curr--;
  }

  return sum;
}

/* Given a binary filename, return the pointer to its ELF header */
Elf64_Ehdr *get_elf_header(char *bin_filename) {
  uint64_t align_512  = 0;
  uint64_t total_size = 0;
  struct posix_header_ustar *hdr = (struct posix_header_ustar *)&_binary_tarfs_start;

  while ((char *)hdr < &_binary_tarfs_end) {
    uint64_t file_size = get_size_tar_octal(hdr->size, 12);
    uint64_t pad_size  = get_size_tar_octal(hdr->pad, 12);

    total_size = sizeof(*hdr) + file_size + pad_size;

    if (!strncmp(hdr->name, bin_filename, strlen(bin_filename))) {
      return (Elf64_Ehdr *)(hdr + 1);
    }

    if (total_size % 512)
      align_512 = 512 - (total_size % 512);
    else
      align_512 = 0;
      
    hdr = (struct posix_header_ustar *)((char *)hdr + total_size + align_512);
  }

  return NULL;
}

/* Go through tarfs and print the names of all directories and files
 */
void browse_tarfs() {
  uint64_t align_512  = 0;
  uint64_t total_size = 0;
  struct posix_header_ustar *hdr = (struct posix_header_ustar *)&_binary_tarfs_start;

  while ((char *)hdr < &_binary_tarfs_end) {
    uint64_t file_size = get_size_tar_octal(hdr->size, 12);
    uint64_t pad_size  = get_size_tar_octal(hdr->pad, 12);

    total_size = sizeof(*hdr) + file_size + pad_size;

    /* Skipping the entries with empty names. TODO: find out why they are present (Piazza 429) */
    if (strlen(hdr->name))
      kprintf("tarfs content : name = %s, size = %d\n", hdr->name, file_size);

    if (total_size % 512)
      align_512 = 512 - (total_size % 512);
    else
      align_512 = 0;
      
    hdr = (struct posix_header_ustar *)((char *)hdr + total_size + align_512);
  }
}

void update_tarfs_tree(char *file_name, uint64_t file_size, uint8_t file_type) {

  if (!file_name)
    return;

  char *saveptr;
  char *sep = "/";
  char arr[256] = {0};
  strcpy(arr, file_name);

  file_t *temp = tarfs_tree;
  char *token = strtok_r(arr, sep, &saveptr);
  while (token != NULL) {
    int i = 0;
    while (i < temp->num_children) {
      if (!strncmp(temp->file_name, token, strlen(token))) {

        temp = temp->child_node[i];
        break;
      }
      i++;
    }

    if (i == temp->num_children) {
      temp->child_node[temp->num_children++] = create_child_node(temp, token, file_size, file_type);
      temp = temp->child_node[temp->num_children - 1];
    }

    token = strtok_r(NULL, sep, &saveptr);
  }
}

void init_tarfs_tree() {
  uint64_t align_512  = 0;
  uint64_t total_size = 0;
  struct posix_header_ustar *hdr = (struct posix_header_ustar *)&_binary_tarfs_start;

  tarfs_tree = (file_t *)vmm_alloc_page();
  strcpy(tarfs_tree->file_name, "rootfs");
  tarfs_tree->file_type     = FILE_TYPE_DIR;
  tarfs_tree->file_size     = 0;
  tarfs_tree->parent_node   = tarfs_tree;
  tarfs_tree->num_children  = 0;

  while ((char *)hdr < &_binary_tarfs_end) {
    uint64_t file_size = get_size_tar_octal(hdr->size, 12);
    uint64_t pad_size  = get_size_tar_octal(hdr->pad, 12);

    total_size = sizeof(*hdr) + file_size + pad_size;

    /* Skipping the entries with empty names. TODO: find out why they are present (Piazza 429) */
    if (strlen(hdr->name)) {
      update_tarfs_tree(hdr->name, file_size, hdr->typeflag[0]);
    }

    if (total_size % 512)
      align_512 = 512 - (total_size % 512);
    else
      align_512 = 0;
      
    hdr = (struct posix_header_ustar *)((char *)hdr + total_size + align_512);
  }
}

file_t *create_child_node(file_t *parent_node, char *file_name, uint64_t file_size, uint8_t file_type) {

  file_t *child_node = (file_t *)vmm_alloc_page();
  strcpy(child_node->file_name, file_name);
  child_node->file_type     = file_type;
  child_node->file_size     = file_size;
  child_node->parent_node   = parent_node;
  child_node->num_children  = 0;

  return child_node;
}
