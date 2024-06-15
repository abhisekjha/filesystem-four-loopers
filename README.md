# Filesystem Assignment

![Open in Codespaces](https://classroom.github.com/assets/launch-codespace-7f7980b617ed060a017424585567c406b6ee15c891e84e1186181d67ecf80aa0.svg)

We construct a user-space, portable, index-allocated file system. This system will afford users 2<sup>26</sup> bytes of drive space within a disk image. The functionality of your program includes creating the filesystem image, listing files, adding and removing files, and persisting the filesystem between sessions.

## Requirements

- The program shall prompt `mfs>` when ready for user input.
- Implement the following commands:

| Command   | Usage                                            | Description                                                                                                   |
|-----------|--------------------------------------------------|---------------------------------------------------------------------------------------------------------------|
| `insert`  | `insert <filename>`                              | Copies a specified file into the filesystem image.                                                            |
| `retrieve`| `retrieve <filename>` or `retrieve <filename> <newfilename>` | Retrieves a file from the filesystem image, placing it in the current directory with an optional new filename.|
| `read`    | `read <filename> <starting byte> <number of bytes>` | Outputs a specified number of bytes from a file in hexadecimal, starting at a given byte.                     |
| `delete`  | `delete <filename>`                              | Deletes a file from the filesystem image.                                                                     |
| `undel`   | `undelete <filename>`                            | Restores a deleted file in the filesystem image.                                                              |
| `list`    | `list [-h] [-a]`                                 | Lists files in the filesystem image, with options to show hidden files and display attributes as binary.      |
| `df`      | `df`                                             | Displays the remaining disk space in the filesystem image.                                                    |
| `open`    | `open <filename>`                                | Opens a filesystem image.                                                                                     |
| `close`   | `close`                                          | Closes the opened filesystem image.                                                                           |
| `createfs`| `createfs <filename>`                            | Creates a new filesystem image.                                                                               |
| `savefs`  | `savefs`                                         | Writes the currently opened filesystem to its file.                                                           |
| `attrib`  | `attrib [+attribute] [-attribute] <filename>`    | Sets or removes an attribute from a file.                                                                     |
| `encrypt` | `encrypt <filename> <cipher>`                    | Encrypts a file using a 1-byte XOR cipher.                                                                    |
| `decrypt` | `decrypt <filename> <cipher>`                    | Decrypts a file using a 1-byte XOR cipher.                                                                    |
| `quit`    | `quit`                                           | Exits the application.                                                                                        |

- The filesystem uses an index allocation method.
- Block size is set to 1024 bytes.
- There are 65536 blocks in total.
- File size can be up to 2<sup>20</sup> bytes.
- The system can hold a maximum of 256 files.
- Filenames up to 64 characters are supported, with alphanumeric and “.” characters, and no limit on characters before or after the “.”.
- A flat directory structure with a single level and no subdirectories is used.
- Directory data is stored in blocks 0-18.
- Block 19 is reserved for the free inode map.
- Blocks 20-276 are allocated for inodes.
- Block 277 is for the free block map.
- Blocks 278-65535 are used for file data storage.
- Files do not need to be stored contiguously.

## Command Details

Detailed usage and error handling for each command are specified within the assignment.

### `insert` Command

- Error handling for long filenames and insufficient disk space.

### `retrieve` Command

- Handling for non-existent files.

### `delete` and `undelete` Commands

- Procedures for deleting and undeleting files, along with error messages for non-existent files.

### `list` Command

- How to display file details and handle an empty filesystem.

### `df` Command

- Displaying available disk space.

### `open` and `close` Commands

- Opening and closing filesystem images with appropriate error messages.

### `savefs` Command

- Saving the filesystem to disk.

### `attrib` Command

- Setting or removing file attributes with messages for non-existent files.

### `createfs` Command

- Creating filesystem images with error handling for unspecified filenames.

### `encrypt` and `decrypt` Commands

- Encryption and decryption of files using XOR cipher with a 1-byte key.
