#!/usr/bin/env make

# MIT License
#
# Copyright (c) 2024 Štěpán Dvorský
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

define _PRINT_VARIABLE_HELP =
usage$(_COLON) make -f $(_THIS_MAKEFILE) <variable> -- <arguments ...>

Runs "make <arguments ...>", except instead of building any targets, the value
of a variable named <variable> is printed to stdout (followed by a newline).

Single-argument, long options (i.e. "--option=value") cannot be passed as
arguments directly. Use "ARGS=<arguments ...>" instead of "-- <arguments ...>"
or split them into two arguments (i.e. "--option value").

Example: extract TARGET_SONAME from ./luajit/src/Makefile
$$ make -f $(_THIS_MAKEFILE) TARGET_SONAME -- -C luajit/src
libluajit-5.1.so.2
$$

endef

ifdef __VARIABLE_TO_PRINT__ #==================================================#

# This branch is taken by a recursive invocation of make that reads both the
# target makefile and this one.

#==============================================================================#

# Include a default makefile, if set and present.
include $(firstword $(wildcard $(__INCLUDE_MAKEFILES__)))

# Print the variable.
$(info $($(__VARIABLE_TO_PRINT__)))

# Dummy target used as goal to prevent building the default one.
__just_print_the_variable_name__:
	@

else #=========================================================================#

# This branch is taken initially. It does some setup and then recurses into the
# branch above.

#==============================================================================#

# Additional arguments to pass to make.
ARGS :=

# Behavior of GNU make. Needed to detect if a default makefile would be used.

# Unless one of these options is used to set the makefile name ...
SHORT_FILE_OPTIONS := -f
LONG_FILE_OPTIONS  := --file --makefile
# ... then the first one of these files that exists is used.
DEFAULT_MAKEFILES := GNUmakefile Makefile makefile

#==============================================================================#

# Suppress "<goal> is up to date" messages and such.
MAKEFLAGS += --silent

# The filename of this makefile.
_THIS_MAKEFILE := $(lastword $(MAKEFILE_LIST))
# This is not necessary, but colons outside rules can confuse external parsers.
_COLON := :

# Show a help message when no goal was specified.
ifeq ($(words $(MAKECMDGOALS)),0)
$(info $(_PRINT_VARIABLE_HELP))
$(error No variable name specified)
endif

# The first goal is the name of the variable to print.
__VARIABLE_TO_PRINT__ := $(firstword $(MAKECMDGOALS))
# The rest are arguments to use when recursively invoking make.
_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
# Add arguments that were passed though the ARGS variable.
_ARGS += $(ARGS)

# Don't pass the ARGS variable down to the target makefile.
MAKEOVERRIDES := $(filter-out ARGS=%,$(MAKEOVERRIDES))
unexport ARGS

# Check if a makefile name is among the arguments ...
_FindShortOptions = $(foreach o,$1,$(filter $o%,$2))
_FindLongOptions = $(strip $(foreach o,$1,$(filter $o,$2) $(filter $o=%,$2)))
_SHORT_FILE_ARGS := $(call _FindShortOptions,$(SHORT_FILE_OPTIONS),$(_ARGS))
_LONG_FILE_ARGS := $(call _FindLongOptions,$(LONG_FILE_OPTIONS),$(_ARGS))
_IS_FILE_SET := $(if $(strip $(_SHORT_FILE_ARGS) $(_LONG_FILE_ARGS)),YES,NO)

# ... if not, we will need to include a default makefile to emulate normal make
# behavior.
ifeq ($(_IS_FILE_SET),NO)
__INCLUDE_MAKEFILES__ := $(DEFAULT_MAKEFILES)
else
__INCLUDE_MAKEFILES__ :=
endif

# Runs this makefile again, but with the specified arguments and variables set.
# The `--question` flag silently prevents building any targets and `cd .` is
# used as a cross platform NOOP.
$(__VARIABLE_TO_PRINT__):
	@$(MAKE) \
		$(_ARGS) \
		--file "$(abspath $(_THIS_MAKEFILE))" \
		--question \
		"__VARIABLE_TO_PRINT__=$(__VARIABLE_TO_PRINT__)" \
		"__INCLUDE_MAKEFILES__=$(__INCLUDE_MAKEFILES__)" \
		__just_print_the_variable_name__ \
	|| cd .

# Matches arguments that were passed as goals.
%:
	@

.PHONY: $(MAKECMDGOALS)

endif #========================================================================#
