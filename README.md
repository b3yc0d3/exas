# exas

`exas` is a minimalist alternative to `sudo(8)`[^1] that
allows the execution of commands as another user.
It is highly inspired by `doas(1)`[^2].
The configuration is done before compiling, following the principles of
[suckless.org](https://suckless.org) software, through a header file.

> [!WARNING]
> This software is unfinished. Keep your expectations low.
> Help to improve exas by contributing.

## Getting Started

In this section we will guide you trough the easy process of building `exas`
from source, 'cuase there aren't any package versions of it.

### Prerequisites

The things you need before installing the software.

* Make Utility (e.g. gmake)
* Clang (can be changed in the *config.mk*)

### Configuration
As mentioned in the intro text, `exas` is configured through a header
file that is called `config.h` (which is a copy of `config.def.h`).

By default the `config.h` looks like the following sniped. It allows **all**
users of group *wheel* to execute any command as root.

```c
static const Rule rules[] = {
    /* permit   user    group      target user   command     arguments */
    { true,     NULL,   "wheel",   "root",       NULL,       NULL},
};
```

Lets explain what which "parameter" does and what value it takes:

| Parameter Name | Type   | Value                | Description                                                |
|:---------------|:-------|:---------------------|:-----------------------------------------------------------|
| permit         | int    | 0 or 1               | Whether to **allow** or **deny** the execution of a match. |
| user           | char*  | String or NULL       | Name of user the rules applies to.                         |
| group          | char*  | String or NULL       | Name of group the rules applies to.                        |
| target user    | char*  | String or NULL       | Name of user to execute as. (e.g. root)                    |
| command        | char*  | String or NULL       | Command that should be matched.                            |
| arguments      | char** | String array or NULL | Arguments that should be matched                           |

If a parameter is set to NULL, it will not be checked. Null is equal to "ignore that value".

### Build Process
1. Clone exas repository
    ```shell
    git clone https://github.com/b3yc0d3/exas.git
    cd exas
    ```

2. Initial exas build using make
    ```shell
    make
    ```

3. Edit the configuration in `config.h`<br>
    The default configuration allows all members of the `wheels` group to execute any command as *root*.
    <br>
    *See [**Configuration**](#configuration) section*

4. Build exas using make
    ```shell
    make
    ```


### Installation
Installation requires the steps from [Build Process](#build-process).

<mark>You most likely need to run the following command(s) with root privileges</mark>

In order to install `exas` globally on your system, as any other software,
simple run the following command

```shell
make clean install
```

Optionally `DESTDIR` can be set.

```shell
make DESTDIR=/your/installation/path clean install
```


## Usage

```
Usage: exas [-u user] -- command [args ...]

Options:
   -u user    specify user to execute as

Parameters
   command    command that should be executed
   args       zero or more arguments for ``command``
```

For more information, read the manual page of exas

```shell
man exas
```

## License
This project is licensed under the ISC-License. See `LICENSE` file for more
informations.

[^1]: [Linux Manual Page](https://man.archlinux.org/man/sudo.8.en)  - [Source](https://github.com/sudo-project/sudo) - [Project Site](https://www.sudo.ws/)
[^2]: [OpenBSD Manual Page](https://man.openbsd.org/doas) - [Source](https://cvsweb.openbsd.org/src/usr.bin/doas/)
