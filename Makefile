NAME 	= 	minitel

CXXFLAGS	=	-Wall -Wextra -Werror -Wno-logical-not-parentheses -std=c++17 -g -O0
ASANFLAGS   = -fsanitize=address
CXX 		=	c++
# Homebrew paths for macOS
BREW_PREFIX = $(shell brew --prefix 2>/dev/null)
ifneq ($(BREW_PREFIX),)
    INCS = -I$(BREW_PREFIX)/include
    LDFLAGS += -L$(BREW_PREFIX)/lib
endif

# libpng via pkg-config (Linux + macOS Homebrew)
PNG_CFLAGS := $(shell pkg-config --cflags libpng 2>/dev/null)
PNG_LIBS   := $(shell pkg-config --libs libpng 2>/dev/null)
ifeq ($(PNG_LIBS),)
    PNG_LIBS := -lpng
endif
INCS += $(PNG_CFLAGS)
LDFLAGS += $(PNG_LIBS)

LDFLAGS += -lutil
INCS	+= -I$(INC_DIR) -I/usr/local/include -I$(HOME)/.local/include

# LDFLAGS += -L/usr/local/lib -lhpdf -L$(HOME)/.local/lib -lhzd -lfreetype

SRC_DIR = src/
INC_DIR = inc/
OBJ_DIR	= .objs/
INCS            += -I$(INC_DIR)/client \
				   -I$(INC_DIR)/minitel \
				   -I$(INC_DIR)/server \
				   -I/usr/local/include \
				   -I$(HOME)/.local/include \
				   -I$(INC_DIR)/term \

SRCS	= 	main.cpp \
			minitel/Minitel1B_Hard.cpp \
			minitel/HardwareSim.cpp \
			client/client.cpp \
			server/server.cpp \
			server/connexion_page.cpp \
			server/risographie_page.cpp \
			server/main_page.cpp \
			utils/utils.cpp \

SRCS 	:= 	$(addprefix $(SRC_DIR), $(SRCS))
OBJS	=	$(SRCS:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

all 			: $(NAME)

asan: CXXFLAGS += $(ASAN_CXXFLAGS)
asan: re

$(OBJ_DIR)%.o	: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

$(NAME)			: $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) $^ -o $(NAME) $(LDFLAGS)

run	: $(NAME)
	sudo ./$(NAME) /dev/ttyUSB0
clean			:
	@echo "Cleaning object..."
	@rm -rf $(OBJ_DIR)

fclean			: clean
	@echo "Cleaning everything..."
	@rm -f $(NAME)

re		: fclean all

.PHONY	: all clean fclean re 
