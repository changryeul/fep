#include <stdio.h>
#include <string.h>
#include <stdlib.h>

double  AtoDf (char *, int);

int	main ()
{
/*
	double	a;
	a = 265.55;

	printf ("[%f] [%ld] [%1d][%02d] [%1d][%02d]\n", a, (long)a, 10/10+1, 10%10+1, 19/10+1, 19%10+1);
	printf ("1  [%1d][%02d] 11[%1d][%02d]\n", 1/10+1, 1%10, 11/10+1, 11%10);
	printf ("2  [%1d][%02d] 12[%1d][%02d]\n", 2/10+1, 2%10, 12/10+1, 12%10);
	printf ("3  [%1d][%02d] 13[%1d][%02d]\n", 3/10+1, 3%10, 13/10+1, 13%10);
	printf ("4  [%1d][%02d] 14[%1d][%02d]\n", 4/10+1, 4%10, 14/10+1, 14%10);
	printf ("5  [%1d][%02d] 15[%1d][%02d]\n", 5/10+1, 5%10, 15/10+1, 15%10);
	printf ("6  [%1d][%02d] 16[%1d][%02d]\n", 6/10+1, 6%10, 16/10+1, 16%10);
	printf ("7  [%1d][%02d] 17[%1d][%02d]\n", 7/10+1, 7%10, 17/10+1, 17%10);
	printf ("8  [%1d][%02d] 18[%1d][%02d]\n", 8/10+1, 8%10, 18/10+1, 18%10);
	printf ("9  [%1d][%02d] 19[%1d][%02d]\n", 9/10+1, 9%10, 19/10+1, 19%10);
	printf ("10 [%1d][%02d] 20[%1d][%02d]\n", 10/10+1, 10%10, 20/10+1, 20%10);
	printf ("10 [%1d][%02d] 20[%1d][%02d]\n", 10/10, 10, 20/10, 10);
	double a = 12.0, b=123.70;
	char	c[10];

	memset (c, 0, sizeof(c));
	sprintf (c, "%08.0f", a);
	printf ("01 c[%s]\n", c);

	memset (c, 0, sizeof(c));
	sprintf (c, "%08.02f", b);
	printf ("02 c[%s]\n", c);
*/
	double	ff;
	char	dk[20];

	memset (dk, 0, sizeof(dk));
	memcpy (dk, "213.55", 6);

	ff = AtoDf (dk, strlen(dk));
	printf ("dk[%s], ff[%f]\n", dk, ff);
	
}

/*----------------------------------------------------------------------*/
double  AtoDf (char *p_ascii, int p_len)
/*----------------------------------------------------------------------*/
{
    int     i, j, k, d;
    double  jj, js, ss;

    k = 0;
    js = ss = jj = 0.;

    for (i = 0; i < p_len; i ++)
    {
        for (j = 0; j < 10; j ++)
        {
            if (*(p_ascii+i) == ('0' + j))
                break;
        }

        if (j >= 10)
        {
            if (*(p_ascii+i) == ('.'))
                k = 1;
			continue;
        }
        else
        {
            if (k == 0)
                js = js * 10. + j;
            else
            {
                ss = ss * 10. + j;
                k ++;
            }
        }
    }

    if (k > 0)
        k --;

    d = 1;
    for (i = 0; i < k; i++)
        d = d * 10;

    jj = js + ss/d;

    return (jj);
}   /* End of AtoDf ()  */
