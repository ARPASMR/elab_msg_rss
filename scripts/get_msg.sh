#################################################################################
#
# FILE.......: 	get_msg.sh
# -------------------------------------------------------------------------------
# PURPOSE....: 	Recupera i file XRIT dal server Meteosat
#		Lo script si accetta in linea di comando: dataora [YYYYMMDDHHMI]
#								
# -------------------------------------------------------------------------------
# CREATED....: 	Aprile 2011 (Cremonini)
#
#                  DATE                      DESCRIPTION
# MODIFIED...: ATTENZIONE! Nel caso di passaggio a MSG3 fare un trova/sostituisci 
#		       di ti tutti i "MSG1" in "MSG3"		(gpm - 20151118)
#              Idem per passaggio a MSG4            (gpm - 20180222)
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

declare -x LANG="us_US.UTF-8"

# ===============================================================================
# DEFINIZIONE DEI PERCORSI
#

MSGDIR_IN=/mnt/raw_msg/
MSGDIR_EPI=/mnt/epi/
MSGDIR_PRO=/mnt/pro/

MSGDIR_OT=/data/msg/input/
MSGDIR_ARC=/data/msg/archive

# ===============================================================================
# INIZIO ELABORAZIONI
# ===============================================================================

echo 'Get_msg.sh: inizio elab ' `date +"%Y-%m-%d %H:%M"`

#
# Definizione del ritardo ATTEZIONE ALLE MODIFCHE AL CRONTAB!!!!!
#

ritardo=-14

if [ $# -lt 1 ]
then
	datatmp=`date +%Y%m%d%H%M`
	dataora=`/home/meteo/bin/minutes $datatmp $ritardo`
else
	dataora=$1
fi

echo "Elaboro dataora: " $dataora

# GPM: in caso di necessità (es.eumetsat cambia MSG3 con MSG1 o MSG4, o viceversa) modificare opportunamente le righe sotto:
mv $MSGDIR_IN/H-000-MSG4__-MSG4*$dataora* $MSGDIR_OT
#cp $MSGDIR_IN/H-000-MSG3__-MSG3*$dataora* $MSGDIR_OT
cp -v $MSGDIR_EPI/H-000-MSG4__-MSG4*$dataora* $MSGDIR_OT
cp -v $MSGDIR_PRO/H-000-MSG4__-MSG4*$dataora* $MSGDIR_OT

tar -zcvf $MSGDIR_ARC/XRIT_$dataora.tar.gz $MSGDIR_OT/*$dataora*

echo 'Get_msg.sh: fine elab ' `date +"%Y-%m-%d %H:%M"`

