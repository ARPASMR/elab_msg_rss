#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//program cumulata.c        Umberto Pellegrini

//programma per cumulare i dati di precipitazione del radar del Monte Lema.
//Legge i files ogni 5 minuti, decluttera e li somma dando la precipitazione oraria in formato matrice GRADS.
//Modificato per leggere i files del monte lema senza informazioni di orografia e quanto altro...
//17 giugno 2009

//Azioni decluttering: 
// 1) su ogni mappa di 5 minuti, con algoritmo tradizionale
// 2) su ogni mappa di 5 minuti utilizzando anche algoritmo per individuare differenze nei valori
// 3) su cumulata oraria con algoritmo tradizionale



int main()
{
  char   nom[19];
  float fval;
  int   val,m,n;
  int   i = 0;
  int   j = 0;
  int   k = 0;
  int   dati[160000];
  int   istart,
        jstart,
        iend,
        jend,
        np_elim,
        np_clutter,
        ls, 
        le,
        ms,
        me,
        nc,
        ncc,
        l;
  int   matrice_dati[400][400];
  float matrix[400][400];
  int   matrice_diff[8][8];
  int   matrice_sum[400][400];
  float matrice_cumulata[400][400];
  FILE* in;
  FILE* out;
  //FILE *outascii;
  FILE* para;
  FILE* info;
  
  info = fopen("info.txt", "w");
  if( info == NULL )
  {
    printf("Unable to open file info.txt. Exiting.\n");
    exit(EXIT_FAILURE);
  }
  out  = fopen("cumulata_oraria.dat", "wb");
  if( out == NULL )
  {
    printf("Unable to open file cumulata_oraria.dat. Exiting.\n");
    fclose(info);
    exit(EXIT_FAILURE);
  }
  //outascii = fopen ("cumulata_ascii.dat","w");
  para = fopen("listaUIL.txt", "r");
  if( para == NULL )
  {
      printf("Unable to open file listaUIL.txt. Exiting.\n");
      fclose(info);
      fclose(out);
      exit(EXIT_FAILURE);
  }

  //lettura dei nomi dei files dal file elenco.txt da aprire...
  while( fscanf(para, "%s", &nom) != EOF )
  {
    i = 0;
    k = k + 1;
    printf("%d\n",k);
    in = fopen(nom,"rb");
    printf("apro il file %s numero %d\n", nom, k);
    fprintf(info,"Nome file esaminato: %s  Numero file: %d\n", nom, k);
	
    //lettura dell'header della matrice contenente i valori di riflettivit�..
    //        fseek(in,32,SEEK_SET);

    //lettura dati contenuti nel file...
    while( (fread((void*)(&fval), sizeof(fval), 1, in)) )
    {
      dati[i] = fval;
      //    printf("%f   %d %d\n",fval,i,sizeof(fval));
      i++;
    }
    fclose(in);
    //        printf("chiudo %s\n",nom);

    //carico la matrice del radar da declutterare
    m = 0;
    n = 0;
    i = 0;
    for( m = 0; m < 400; m++ )
    {
      for( n = 0; n < 400; n++ )
      {
        matrice_dati[m][n] = dati[i];
        i++;
      }
    }
    // ripulisco i bordi della matrice
    m = 0;
    for( m = 0; m < 400; m++ )
    {
      matrice_dati[m][0]   = 0;
      matrice_dati[m][399] = 0;
    }
    n = 0;
    for( n = 0; n < 400; n++ )
     {
       matrice_dati[0][n]   = 0;
       matrice_dati[399][n] = 0;
     }

    // declutter su differenza valori, con ciclo
    /*                istart=1;
                jstart=1;
                iend=398;
                jend=398;
                n=0;
                np_clutter=0;
                for (n=0; n<3; n++)
                {
		np_clutter=0;
                        for (i=istart; i<=iend; i++)
                        {
                                for (j=jstart; j<=jend; j++)
                                {
                                        ls=i-1;
                                        le=i+1;
                                        ms=j-1;
                                        me=j+1;
                                        ncc=0;
                                        for (l=ls; l<=le; l++)
                                                {
                                                for (m=ms; m<=me; m++)
                                                        {
							matrice_diff[l][m]=abs(matrice_dati[i][j] - matrice_dati[l][m]);
							if (matrice_diff[l][m]>=6-n)
								{
								ncc=ncc+1;
								matrice_sum[i][j]=matrice_sum[i][j] + matrice_dati[l][m];
                                                        	}
                                                        }
                                                }
                                        if(ncc>2)
                                                {
                                                matrice_dati[i][j] = matrice_sum[i][j]/(ncc);
                                                np_clutter=np_clutter+1;
                                                matrice_sum[i][j]=0;
                                                ncc=0;
                                                }
                                        matrice_sum[i][j]=0;                                        
                                }
                        }
			printf("punti rielaborati con ND al giro %d: %d\n",n,np_clutter);
                }
    */


    // declutter tradizionale su singola mappa 5 minuti 
    istart = 1;
    jstart = 1;
    iend   = 398;
    jend   = 398;
    n      = 0;
    for( n = 0; n < 16; n++ )
    {
      np_elim = 0;
      for( i = istart; i <= iend; i++ )
      {
        for( j = jstart; j <= jend; j++ )
        {
          if( matrice_dati[j][i] > 0 )
          {
            ls = i - 1;
            le = i + 1;
            ms = j - 1;
            me = j + 1;
            nc = 0;
            for( l = ls; l <= le; l++ )
            {
              for( m = ms; m <= me; m++ )
              {
                if( matrice_dati[m][l] > 0 )
                {
                  nc=nc+1;
                }
              }
            }
            if( nc < 5 )
            {
              matrice_dati[j][i] = 0;
              np_elim            = np_elim + 1;
            }
            /*
            if((nc>=5) && (matrice_dati[j][i]<=1))
            {
              matrice_dati[j][i]=0;
              np_elim=np_elim+1;
            }
            */
          }
        }
      }
      printf("punti eliminati con TD al giro %d:  %d\n", n, np_elim);
    }

    /*

    //associazione valori originari con dati di precipitazione...
                i=0;
                j=0;
        for (i=0; i<400; i++)
                {
                for (j=0; j<400; j++)
                        {
                        if (matrice_dati[i][j]==0.)
                        matrix[i][j]=0.;
                        else if (matrice_dati[i][j]==1.)
                        matrix[i][j]=0.25/12.;
                        else if (matrice_dati[i][j]==2.)
                        matrix[i][j]=0.40/12.;
                        else if (matrice_dati[i][j]==3.)
                        matrix[i][j]=0.63/12.;
                        else if (matrice_dati[i][j]==4.)
                        matrix[i][j]=1./12.;
                        else if (matrice_dati[i][j]==5.)
                        matrix[i][j]=1.6/12.;
                        else if (matrice_dati[i][j]==6.)
                        matrix[i][j]=2.5/12.;
                        else if (matrice_dati[i][j]==7.)
                        matrix[i][j]=4./12.;
                        else if (matrice_dati[i][j]==8.)
                        matrix[i][j]=6.3/12.;
                        else if (matrice_dati[i][j]==9.)
                        matrix[i][j]=10./12.;
                        else if (matrice_dati[i][j]==10.)
                        matrix[i][j]=16./12.;
                        else if (matrice_dati[i][j]==11.)
                        matrix[i][j]=25./12.;
                        else if (matrice_dati[i][j]==12.)
                        matrix[i][j]=40./12.;
                        else if (matrice_dati[i][j]==13.)
                        matrix[i][j]=63./12.;
                        else if (matrice_dati[i][j]==14.)
                        matrix[i][j]=100./12.;
                        else if (matrice_dati[i][j]==15.)
                        matrix[i][j]=150./12.;
                        else {
                        printf("ciao!\n");}
                        matrice_cumulata[i][j]= matrice_cumulata[i][j] + matrix[i][j];
                        }
                }
	
    */
	
    // azzero matrice e vettore...
    i = 0;
    for( i = 0; i < 160000; i++ )
    {
      dati[i] = 0;
    }
    // azzero matrice dati...
    i = 0;
    j = 0;
    for( i = 0; i < 400; i++ )
    {
      for( j = 0; j < 400; j++ )
      {
        matrice_cumulata[i][j] = matrice_dati[i][j];
        matrice_dati[i][j]     = 0;
        matrix[i][j]           = 0.;
      }
    }
	
  }	//end while
	
  printf("ho finito di leggere tutto e pispolare...mo scrivo\n");
	
  // scrittura matrice output in ascii
  /*
        for (i=0; i<400; i++)
    	{
    	  for (j=0; j<400; j++)
		  {
		    fprintf(outascii,"%02d ",matrice_cumulata[i][j]);
		    printf("i,j,matrice %d,%d,%f\n",i,j,matrice_cumulata[i][j]);
		    if (j ==399)
			{
			  fprintf(outascii,"\n");
			  k++;
			}
		  }
	    }
	    fclose(outascii);	
  */	
		
	
  // scrittura matrice di output in binario	
	
  i = 0;
  j = 0;
  for( i = 0; i < 400; i++ )
  {
    for (j = 0; j < 400; j++ )
    {
      //                        printf("matrice output= %d\n",matrice_dati[i][j]);
      fwrite(&matrice_cumulata[i][j], sizeof(matrice_cumulata[i][j]), 1, out);
    }
  }
  //        fprintf(info,"punti eliminati e clutter tolto = %d -- %d\n",np_elim,np_clutter);
  fclose(out);
  //        fclose(info);
  return 0;
        
}
