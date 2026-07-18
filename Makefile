NAME 	= 	minitel

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++17 -g -O0
ASANFLAGS   = -fsanitize=address
CXX 		=	c++
# Homebrew paths for macOS
BREW_PREFIX = $(shell brew --prefix 2>/dev/null)
ifneq ($(BREW_PREFIX),)
    INCS = -I$(BREW_PREFIX)/include
    LDFLAGS += -L$(BREW_PREFIX)/lib
endif

LDFLAGS += -lpng -lcurl
INCS            += -I$(INC_DIR) -I/usr/local/include -I$(HOME)/.local/include

LDFLAGS += -L/usr/local/lib -lhpdf -L$(HOME)/.local/lib -lhzd -lfreetype

SRC_DIR = src/
INC_DIR = inc/
OBJ_DIR	= .objs/
INCS            += -I$(INC_DIR) -I/usr/local/include -I$(HOME)/.local/include
SRCS	= 	main.cpp\
			server.cpp\
			story.cpp\
			Minitel.cpp\
			tools.cpp\
			ThermalPrinter.cpp\
			page/APage.cpp\
			page/IndexPage.cpp\
			page/ContactPage.cpp\
			page/MazePage.cpp\
			page/CadavrePage.cpp\
			page/Forty2Page.cpp\

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

clean			:
	@echo "Cleaning object..."
	@rm -rf $(OBJ_DIR)

fclean			: clean
	@echo "Cleaning everything..."
	@rm -f $(NAME)

re		: fclean all

.PHONY	: all clean fclean re 
