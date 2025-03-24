# ----------------------------
# Makefile Options
# ----------------------------

NAME = LORENZ
ICON = icon.png
DESCRIPTION = "fun"
COMPRESSED = NO
ARCHIVED ?= NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
