#################################################################################
#
# FILE.......: 	elab_msg.sh
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

declare -x LANG="us_US.UTF-8"

# ===============================================================================
# DEFINIZIONE DEI PERCORSI
#

MSGDIR_IN=/data/rss/input/
MSGDIR_OT=/data/rss/tmp/
MSGDIR_TAR=/data/rss/europe/
MSGDIR_IMG=/data/rss/imgs/

export TMP=$MSGDIR_OT

# ===============================================================================
# INIZIO ELABORAZIONI
# ===============================================================================

echo 'Elab_msg.sh: inizio elab ' `date +"%Y-%m-%d %H:%M"`

if [ $# -lt 1 ]
then
	echo "usage: elab_msg.sh [dataora]"
        exit
fi

dataora=$1

nomefile_prologo=`find $MSGDIR_IN -name "*PRO*" | sort | tail -1`

# Metto insieme tutti i segmenti

/home/meteo/bin/put_xrit_segments_together_msg1.pl -v -D $MSGDIR_OT  $nomefile_prologo

# Converto in formato binario piatto

OLDDIR=$PWD

# lavoro nella directory /data/msg/tmp

cd $MSGDIR_OT

echo "Elaboro HRV e rimuovo"

for nomecycle in `ls -1c $MSGDIR_OT/*HRV*CYCLE*-__`;
do
	echo "Elaboro il file " $nomecycle

	/home/meteo/bin/xrit2raw -f -v $nomecycle $nomecycle".raw"

	/home/meteo/bin/navig MSG1 H 42 48 5 14 0.006 $nomecycle".raw" $nomecycle

	rm $nomecycle $nomecycle".raw"

done

echo "Elaboro i canali a bassa risoluzione e rimuovo"

for nomecycle in `ls -1c $MSGDIR_OT/*CYCLE*-__`;
do
	echo "Elaboro il file " $nomecycle

	/home/meteo/bin/xrit2raw -f -v $nomecycle $nomecycle".raw"

	/home/meteo/bin/navig MSG1 L 20 70 -30 30 0.04 $nomecycle".raw" $nomecycle

	rm $nomecycle $nomecycle".raw"

done

echo "Generazione bi-spettrale HRV"

Rscript /home/meteo/R_batch/elab_hrv_rss.r $dataora

#
# Copia su Apprendista
#

scp $MSGDIR_IMG/HRV_$dataora.png           meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/RSS-IRHRV
scp $MSGDIR_IMG/EIR_108_ZOOM_$dataora.png  meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/RSS-E-IR108-ZOOM
scp $MSGDIR_IMG/IR_$dataora.png            meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/RSS-E-IR108
scp $MSGDIR_IMG/SNDWICH_$dataora.png       meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/RSS-SNDWICH

echo "Generazione mappe"

Rscript /home/meteo/R_batch/elab_rss.r $dataora

cd $OLDDIR

echo archivio e pulizia

tar -zcvf  $MSGDIR_TAR/MSG_$dataora.tar.gz $MSGDIR_OT/*$dataora*.hdr $MSGDIR_OT/*$dataora*.flt

rm -fr $MSGDIR_OT/*$dataora*.hdr $MSGDIR_OT/*$dataora*.flt

echo 'Elab_msg: fine elab ' `date +"%Y-%m-%d %H:%M"`
