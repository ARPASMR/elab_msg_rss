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

MSGDIR_IN=/data/msg/input/
MSGDIR_OT=/data/msg/tmp/
MSGDIR_TAR=/data/msg/europe/
MSGDIR_IMG=/data/msg/imgs/
MSGDIR_ANALISI=/data/msg/analisi_msg

export TMP=$MSGDIR_OT

# ===============================================================================
# INIZIO ELABORAZIONI
# ===============================================================================
echo "***************************************************************"
echo 'Elab_msg.sh: inizio elab ' `date +"%Y-%m-%d %H:%M"`

if [ $# -lt 1 ]
then
	echo "usage: elab_msg.sh [dataora]"
        exit
fi

dataora=$1

ora_analisi=${dataora:8:12} && echo "ora di elaborazione: $ora_analisi"

nomefile_prologo=`find $MSGDIR_IN -name "*PRO*" | sort | tail -1`

# Metto insieme tutti i segmenti

/home/meteo/bin/put_xrit_segments_together.pl -v -D $MSGDIR_OT  $nomefile_prologo

# Converto in formato binario piatto

OLDDIR=$PWD

# lavoro nella directory /data/msg/tmp

cd $MSGDIR_OT

echo "Elaboro HRV e rimuovo"

for nomecycle in `ls -1c $MSGDIR_OT/*HRV*CYCLE*-__`;
do
	echo `date +"%Y-%m-%d %H:%M"`" > Elaboro il file " $nomecycle

	/home/meteo/bin/xrit2raw -f -v $nomecycle $nomecycle".raw"

	/home/meteo/bin/navig MSG2 H 42 48 5 14 0.006 $nomecycle".raw" $nomecycle

	rm $nomecycle $nomecycle".raw"

done

echo "Elaboro i canali a bassa risoluzione e rimuovo"

for nomecycle in `ls -1c $MSGDIR_OT/*CYCLE*-__`;
do
	echo `date +"%Y-%m-%d %H:%M"`" > Elaboro il file " $nomecycle

	/home/meteo/bin/xrit2raw -f -v $nomecycle $nomecycle".raw"

	/home/meteo/bin/navig MSG2 L 20 70 -30 30 0.04 $nomecycle".raw" $nomecycle

	rm $nomecycle $nomecycle".raw"

done

echo "Generazione bi-spettrale HRV"

Rscript /home/meteo/R_batch/elab_hrv.r $dataora

#scp $MSGDIR_IMG/HRV_$dataora.png  meteo@$ECCELLENTE:/srv/www/prodottimeteo/msg1/IRHRV

#
# Copia su Apprendista
#

scp $MSGDIR_IMG/HRV_$dataora.png  meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/IRHRV

echo `date +"%Y-%m-%d %H:%M"`" > Generazione mappe"

Rscript /home/meteo/R_batch/elab_msg.r $dataora

echo `date +"%Y-%m-%d %H:%M"`" > Fine generazione mappe"


cd $OLDDIR

#
# Copia su Apprendista
#

scp $MSGDIR_IMG/EIR_108_ZOOM_$dataora.png  meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/E-IR108-ZOOM
scp $MSGDIR_IMG/IR_$dataora.png            meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/E-IR108
scp $MSGDIR_IMG/WV_$dataora.png            meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/E-WV62
scp $MSGDIR_IMG/NATCOL_$dataora.png        meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/NATCOL
scp $MSGDIR_IMG/AIRMASS_$dataora.png       meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/AIRMASS
scp $MSGDIR_IMG/AVHRR_$dataora.png         meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/AVHRR
scp $MSGDIR_IMG/SOLARDAY_$dataora.png      meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/SOLARDAY
scp $MSGDIR_IMG/FIRE_$dataora.png          meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/FIRE
scp $MSGDIR_IMG/MICRO24H_$dataora.png      meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/MICRO24H
scp $MSGDIR_IMG/SNDWICH_$dataora.png       meteo@$APPRENDISTA:/var/www/html/prodottimeteo/msg1/SNDWICH

echo archivio e pulizia

tar -zcvf  $MSGDIR_TAR/MSG_$dataora.tar.gz $MSGDIR_OT/*$dataora*.hdr $MSGDIR_OT/*$dataora*.flt

if [ "$ora_analisi" == "0000" -o "$ora_analisi" == "0600" -o "$ora_analisi" == "1200" -o "$ora_analisi" == "1800" ]
then
	echo "elaboro l'analisi per l'ora sinottica: $ora_analisi"
	cp -v $MSGDIR_OT/*IR_108*$dataora*.flt $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*IR_108*$dataora*.hdr $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*IR_097*$dataora*.flt $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*IR_097*$dataora*.hdr $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*IR_087*$dataora*.flt $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*IR_087*$dataora*.hdr $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*IR_120*$dataora*.flt $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*IR_120*$dataora*.hdr $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*WV_062*$dataora*.flt $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*WV_062*$dataora*.hdr $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*WV_073*$dataora*.flt $MSGDIR_ANALISI
	cp -v $MSGDIR_OT/*WV_073*$dataora*.hdr $MSGDIR_ANALISI

	if [ -f $MSGDIR_IMG/IRWEB_$dataora.png ]
	then
		echo "invio dell'immagine per il web IRWEB_$dataora.png"
		sh /home/meteo/scripts/invio_web.sh $MSGDIR_IMG/IRWEB_$dataora.png irweb
	fi
fi

rm -fr $MSGDIR_OT/*$dataora*.hdr $MSGDIR_OT/*$dataora*.flt

echo 'Elab_msg: fine elab ' `date +"%Y-%m-%d %H:%M"`
