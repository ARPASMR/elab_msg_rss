#################################################################################
#
# FILE.......: 	clean.sh
# -------------------------------------------------------------------------------
# PURPOSE....: 	Conversione immagini Meteosat-9 HRIT/LRIT
#		Lo script si aspetta in linea di comando: nome file prologo
#								
# -------------------------------------------------------------------------------
# CREATED....: 	Aprile 2011 (Cremonini)
#
#                  DATE                      DESCRIPTION
# MODIFIED...: 
#
# -------------------------------------------------------------------------------
# VERSION....: 	1.0 (05/04/2011)
#
# =======================================================================
# REFERENCES..:
#
# Pellegrini:   	ARPA Lombardia
#
#################################################################################
#
# ===============================================================================
# CONFIGURAZIONE DELL'AMBIENTE
#
. /conf/default.conf

declare -x LANG="us_US.UTF-8"

# ===============================================================================
# DEFINIZIONE DEI PERCORSI
#

MSGROOT=/data/msg
RSSROOT=/data/rss
MODELLIROOT=/data/modelli

# ===============================================================================
# INIZIO ELABORAZIONI
# ===============================================================================

echo "`basename $0`: inizio ore " `date +"%Y-%m-%d %H:%M"`

find $MSGROOT/tmp/ -mtime +1 -print -exec rm -fr {} \;

find $MSGROOT/archive/ -mtime +7 -print -exec rm -f {} \;

find $MSGROOT/imgs/ -mtime +7 -print -exec rm -f {} \;

find $MSGROOT/europe/ -mtime +90 -print -exec rm -f {} \;

find $RSSROOT/tmp/ -mtime +1 -print -exec rm -fr {} \;

find $RSSROOT/archive/ -mtime +7 -print -exec rm -f {} \;

find $RSSROOT/imgs/ -mtime +7 -print -exec rm -f {} \;

find $RSSROOT/europe/ -mtime +90 -print -exec rm -f {} \;

#find $MODELLIROOT/ecmwf/ -mtime +10 -print -exec rm -f {} \;

find $MSGROOT/analisi_msg/ -mtime +10 -print -exec rm -f {} \;

echo "$basename $0: fine ore " `date +"%Y-%m-%d %H:%M"`
