#################################################################################
#
# FILE.......: 	get_rss.sh
# -------------------------------------------------------------------------------
# PURPOSE....: 	Recupera i file XRIT RSS dal server Meteosat
#		Lo script si accetta in linea di comando: dataora [YYYYMMDDHHMI]
#								
# -------------------------------------------------------------------------------
# CREATED....: 	Giugno 2011 (Cremonini)
#
#                  DATE                      DESCRIPTION
# MODIFIED...: 
#
# -------------------------------------------------------------------------------
# VERSION....: 	1.0 (17/06/2011)
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

declare -x LANG="us_US.UTF-8"

# ===============================================================================
# DEFINIZIONE DEI PERCORSI
#

MSGDIR_IN=/mnt/raw_msg/
MSGDIR_EPI=/mnt/epi/
MSGDIR_PRO=/mnt/pro/

MSGDIR_OT=/data/rss/input/
MSGDIR_ARC=/data/rss/archive

MSGHDR=H-000-MSG2__-MSG2
#MSGHDR=H-000-MSG1__-MSG1

# INIZIO ELABORAZIONI
# ===============================================================================

echo 'Get_msg.sh: inizio elab ' `date +"%Y-%m-%d %H:%M"`

#
# Definizione del ritardo ATTEZIONE ALLE MODIFCHE AL CRONTAB!!!!!
#

ritardo=-6

if [ $# -lt 1 ]
then
	datatmp=`date +%Y%m%d%H%M`
	dataora=`/home/meteo/bin/minutes $datatmp $ritardo`
else
	dataora=$1
fi

echo "Elaboro dataora: " $dataora

mv $MSGDIR_IN/$MSGHDR*$dataora* $MSGDIR_OT
cp $MSGDIR_EPI/$MSGHDR*$dataora* $MSGDIR_OT
cp $MSGDIR_PRO/$MSGHDR*$dataora* $MSGDIR_OT

tar -zcvf $MSGDIR_ARC/XRIT_$dataora.tar.gz $MSGDIR_OT/*$dataora*

echo 'Get_msg.sh: fine elab ' `date +"%Y-%m-%d %H:%M"`

