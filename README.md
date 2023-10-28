# exas

`exas` is a minimalist alternative for `sudo(8)`[^1] and `doas(1)`[^2] that
allows the execution of commands as another user.

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

* GNU Make Utility
* Clang (can be changed in the *config.mk*)

### Configuration
As mentioned in the intro text, `exas` if configured through a specific
header file that is called `config.h` (which is a copy of `config.def.h`).

By default the `config.h` looks like the following sniped. It allows **all**
users that are in the group of *wheel* to execute any command as root.

```c
static const Rule rules[] = {
    /* permit   user    group      target user   command     arguments */
    { true,     NULL,   "wheel",   "root",       NULL,       NULL},
};
```

Lets explain what which "parameter" does and what value it takes.

permit
: Whether the rule **allows** or **denys** the execution of a match.<br>
    Takes a bool value of <abbr title="(int) 1">`true`</abbr> or <abbr title="(int) 0">`false`</abbr>.
    

user
: Name of user that should be matched by the rule.<br>
    Takes an `char *`. If set to `NULL`, no user name matching will be done.

group
: Name of group that should be matched by the rule.<br>
    Takes an `char *`. If set to `NULL`, no group name matching will be done.

target user
: Name of user to execute as.<br>
    Takes an `char *`. If set to `NULL`, the rule will allow *any* target user.

command
: Command that should be matched by the rule.<br>
    Takes an `char *`. If set to `NULL`, the rule will not check for a command.

arguments
: Arguments for `command`.<br>
    Takes an array of `char **`. If set to `NULL`, the rule will not check for any arguments.

### Build Process
1. Clone exas repository
    ```shell
    git clone https://github.com/b3yc0d3/exas.git
    cd exas
    ```

2. Initial exas build using gmake
    ```shell
    gmake
    ```

3. Edit the configuration in `config.h`<br>
    The default configuration allows all members of the `wheels` group to execute any command as *root*.
    <br>
    *See [**Configuration**](#configuration) section*

4. Build exas using gmake
    ```shell
    gmake
    ```


### Installation
Installation requires the steps from [Build Process](#build-process).

<mark>You most likely need to run the following command(s) with root privileges</mark>

In order to install `exas` globally on your system, as any other software,
simple run the following command

```shell
gmake clean install
```

Optionally `DESTDIR` can be set.

```shell
gmake DESTDIR=/your/installation/path clean install
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

A few examples of how to use `exas`.

```
exas -- pacman -Syu
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
