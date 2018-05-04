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
# MODIFIED...:  Maggio 2018 (Paganotti)      Update for container development
#
# -------------------------------------------------------------------------------
# VERSION....: 	1.0 (05/04/2011)
#               1.1 (04/05/2018)
#
# =======================================================================
# REFERENCES..:
#
# Pellegrini:   	ARPA Lombardia
# Paganotti:            Softech s.r.l.
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

MSGDIR_IN=$MSGDATADIR/input/

# ===============================================================================
# DEFINIZIONE VARIABILI APPLICATIVI
#
MINUTES=$BINDIR/minutes


# ===============================================================================
# INIZIO ELABORAZIONI
# ===============================================================================

echo 'Inizio elab ' `date +"%Y-%m-%d %H:%M"`

ritardo=-15

if [ $# -lt 1 ]
then
	datatmp=`date +%Y%m%d%H%M`
	dataora="$MINUTES $datatmp $ritardo"
else
	dataora=$1
fi

echo "***variabile dataora vale: $dataora***"

sh $SCRIPTSDIR/get_msg.sh $dataora  >>$LOGDIR/get_msg_`date +%Y%m%d`.log 2>&1

nomepro=`find $MSGDIR_IN -name "*PRO*" | sort | tail -1`

echo "Elaboro il file prologo " $nomepro

sh $SCRIPTSDIR/elab_msg.sh $dataora

rm -f $MSGDIR_IN/*

echo 'Fine elab ' `date +"%Y-%m-%d %H:%M"`
