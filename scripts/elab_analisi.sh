#################################################################################
#
# FILE.......: 	elab_analisi.sh
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
. /home/meteo/conf/default.conf

#declare -x LANG="us_US.UTF-8"

# ===============================================================================
# DEFINIZIONE DEI PERCORSI
#

MSGDIR_IN=/data/msg/input/
MSGDIR_OT=/data/msg/tmp/
MSGDIR_TAR=/data/msg/europe/
MSGDIR_IMG=/data/msg/imgs/

export TMP=$MSGDIR_OT

# ===============================================================================
# INIZIO ELABORAZIONI
# ===============================================================================

echo 'Elab_analisi.sh: inizio elab ' `date +"%Y-%m-%d %H:%M"`

Rscript /home/meteo/R_batch/analisi_108.r $1

#scp $MSGDIR_IMG/ANALISI_IR10.8_$1.png meteo@$ECCELLENTE:/srv/www/prodottimeteo/analisi/ecmwf-msg/zt500 #&& rm -v $MSGDIR_IMG/ANALISI_IR10.8_$1.png
scp $MSGDIR_IMG/ANALISI_IR10.8_$1.png meteo@$APPRENDISTA:/var/www/html/prodottimeteo/analisi/ecmwf-msg/zt500 && rm -v $MSGDIR_IMG/ANALISI_IR10.8_$1.png

echo 'Elab_analisi: fine elab ' `date +"%Y-%m-%d %H:%M"`
