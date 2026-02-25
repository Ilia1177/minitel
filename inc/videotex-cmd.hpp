#ifndef VIDEOTEX_CMD
# define VIDEOTEX_CMD

# define BS "\x08"
# define ESC "\x1B"
# define DC3 "\x13"
# define CLS "\x0C"
# define CLEOL "\x18"

# define CON "\x11"
# define COFF "\x14"

# define G0 "\x0F"
# define SI "\x0f"
# define G1 "\x0E"
# define SO "\x0e"
# define G2 "\x19"
# define SS2 (0x19)

# define CUR "\x1F%c%c"
# define CUR_DELTA_V 64
# define CUR_DELTA_H 64
# define LINE0 "\x1F\x40\x41"

# define INV "\x1B\x5D"
# define NORMAL "\x1B\x5C"
# define BLINK "\x1B\x48"

# define INK "\x1B%c"
# define INK1 "\x1B\x41"
# define INK7 "\x1B\x47"
# define INK_DELTA 0x40

# define PAPER "\x1B%c"
# define PAPER_DELTA 0x50

// Protocole
# define PRO1 "\x1B\x39"
# define PRO2 "\x1B\x3A"
# define PRO3 "\x1B\x3B"

# define P_OFF "\x60"
# define P_ON "\x61"
# define P_NON_RETOUR_ACQUITEMENT "\x64"

# define P_CLAVIER_TX "\x51"
# define P_MODEM_RX "\x5A"
# define P_PRISE_TX "\x53"

// Non retour d'acquitement sur prise
# define P_ACK_OFF_PRISE PRO2 P_NON_RETOUR_ACQUITEMENT P_PRISE_TX

// Echo ON/OFF en mode local
# define P_LOCAL_ECHO_ON PRO3 P_ON P_MODEM_RX P_CLAVIER_TX
# define P_LOCAL_ECHO_OFF PRO3 P_OFF P_MODEM_RX P_CLAVIER_TX

# define P_ROULEAU_ON PRO2 "\x69\x43"
# define P_ROULEAU_OFF PRO2 "\x6A\x43"

# define P_CLAVIER_MINUSCULE PRO2 "\x69\x45"
# define P_CLAVIER_MAJUSCULE PRO2 "\x6A\x45"
# define P_CLAVIER_ETENDU PRO3 "\x69\x59\x41"
# define P_CLAVIER_VIDEOTEX PRO3 "\x6A\x59\x41"


# define MODE_INIT_STRING P_ACK_OFF_PRISE P_LOCAL_ECHO_OFF P_ROULEAU_ON P_CLAVIER_MINUSCULE P_CLAVIER_ETENDU
#endif
