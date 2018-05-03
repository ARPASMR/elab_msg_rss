#################################################################################
#
# FILE.......: 	elab_msg_batch.sh
# -------------------------------------------------------------------------------
# PURPOSE....: 	Conversione immagini Meteosat-9 HRIT/LRIT
#		Prende i file dal server e li converte in raw
#								
# -------------------------------------------------------------------------------
# CREATED....: 	Aprile 2011 (Pellegrini, Cremonini)
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

declare -x LANG="us_US.UTF-8"

# ===============================================================================
# DEFINIZIONE DEI PERCORSI
#

MSGDIR_IN=/data/msg/input/

# ===============================================================================
# INIZIO ELABORAZIONI
# ===============================================================================

echo 'Inizio elab ' `date +"%Y-%m-%d %H:%M"`

ritardo=-15

if [ $# -lt 1 ]
then
	datatmp=`date +%Y%m%d%H%M`
	dataora=`/home/meteo/bin/minutes $datatmp $ritardo`
else
	dataora=$1
fi

echo "***variabile dataora vale: $dataora***"

sh /home/meteo/scripts/get_msg.sh $dataora  >>/home/meteo/log/get_msg_`date +%Y%m%d`.log 2>&1

nomepro=`find $MSGDIR_IN -name "*PRO*" | sort | tail -1`

echo "Elaboro il file prologo " $nomepro

sh /home/meteo/scripts/elab_msg.sh $dataora
#sh /home/meteo/scripts/elab_msg_test.sh $dataora

rm -f $MSGDIR_IN/*

echo 'Fine elab ' `date +"%Y-%m-%d %H:%M"`
