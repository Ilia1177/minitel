0x07 (BEL)  : Bip sonore
0x08 (BS)   : Curseur à gauche
0x09 (HT)   : Curseur à droite
0x0A (LF)   : Curseur en bas
0x0B (VT)   : Curseur en haut
0x0C (FF)   : Efface l'écran (Form Feed)
0x0D (CR)   : Retour chariot
0x0E (SO)   : Charset G1 (graphiques)
0x0F (SI)   : Charset G0 (texte normal)
0x11 (DC1)  : Curseur ON
0x14 (DC4)  : Curseur OFF
0x18 (CAN)  : Annule la ligne courante
0x1E (RS)   : Home (curseur en haut à gauche)

ESC 0x0C (FF)  : Efface l'écran
ESC 0x58       : Efface jusqu'en fin de ligne
ESC 0x59       : Efface du début de ligne jusqu'au curseur

ESC 0x48       : Home (curseur en 1,1)
ESC 0x41       : Curseur haut
ESC 0x42       : Curseur bas
ESC 0x43       : Curseur droite
ESC 0x44       : Curseur gauche

// Position absolue
ESC 0x48 Y X   : Position (Y et X sont des valeurs + 0x40)
               : Exemple: ligne 5, col 10 = ESC H E J
               : (0x45 = 0x40+5, 0x4A = 0x40+10)

ESC 0x45       : Curseur visible
ESC 0x46       : Curseur invisible
ESC 0x4C       : Normal
ESC 0x4D       : Clignotant
ESC 0x48       : Hauteur normale
ESC 0x49       : Hauteur double
ESC 0x4A       : Largeur normale
ESC 0x4B       : Largeur double
ESC 0x5C       : Taille normale
ESC 0x5D       : Hauteur double
ESC 0x5E       : Largeur double
ESC 0x5F       : Double hauteur + largeur

ESC 0x40       : Noir
ESC 0x41       : Rouge
ESC 0x42       : Vert
ESC 0x43       : Jaune
ESC 0x44       : Bleu
ESC 0x45       : Magenta
ESC 0x46       : Cyan
ESC 0x47       : Blanc

ESC 0x50       : Fond noir
ESC 0x51       : Fond rouge
ESC 0x52       : Fond vert
ESC 0x53       : Fond jaune
ESC 0x54       : Fond bleu
ESC 0x55       : Fond magenta
ESC 0x56       : Fond cyan
ESC 0x57       : Fond blanc

0x0E           : Active mode graphique (G1)
0x0F           : Retour mode texte (G0)

// En mode graphique, les caractères 0x20-0x7F 
// dessinent des blocs de pixels 2x3
