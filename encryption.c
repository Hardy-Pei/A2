#include "wsn.h"

int gcd(long p, long q) {
	int i, gcd;
	for (i = 1; i < p && i < q; i++) {
		if (p % i == 0 && q % i == 0) {
			gcd = i;
		}
	}
	return gcd;
}

int publicKey(long p, long q, long *exp, long *mod) {
	long sqt, temp;
	*mod = (p-1)*(q-1);
	sqt = (*mod)/2;
	temp = 1;
	while (sqt != temp) {
		temp = sqt;
		sqt = ((*mod)/temp+temp)/2;
	}
	*exp = (int)sqt;
	while (1!=gcd(*exp, *mod)){
		(*exp)++;
	}
	*mod = p*q;
	return 0;
}

int privateKey(long p, long q, long pubexp, long *exp, long *mod) {
	*mod = (p-1)*(q-1);
	*exp = 1;
	long tmp = pubexp;
	while (1 != tmp % *mod) {
		tmp += pubexp;
		tmp %= *mod;
		(*exp)++;
	}
	*mod = p*q;
	return 0;
}

int encrypt(long *in, long exp, long mod, long *out, size_t len) {
	int i;
	for (i=0; i< len; i++) {
		long c = in[i];
		int z;
		for (z=1; z<exp; z++) {
			c *= in[i];
			c %= mod;
		}
		out[i] = c;
	}
	out[len] = '\0';
	return 0;
}

int decrypt(long * in, long exp, long mod, long * out, size_t len) {
	int i;
	for (i=0; i<len; i++) {
		long int c = in[i];
		int z;
		for (z = 1; z<exp; z++) {
			c *= in[i];
			c %= mod;
		}
		out[i] = c;
	}
	out[len] = '\0';
	return 0;
}

int encrypt_p(long *in, long exp, long mod, long *out, size_t len) {
	int i, chunk = 1;
	printf("%ld\n", mod);
	#pragma omp parallel shared(in, out, exp, mod, chunk) private(i)
	{
		#pragma omp for schedule(dynamic, chunk)
		for (i=0; i< len; i++) {
			long c = in[i];
			int z;
			for (z=1; z<exp; z++) {
				// printf("%d\n", tid);
				c *= in[i];
				c %= mod;
			}
			out[i] = c;
		}
	}
	out[len] = '\0';
	return 0;
}

int decrypt_p(long * in, long exp, long mod, long * out, size_t len) {
	int i, chunk = 1;
	#pragma omp parallel shared(in, out, exp, mod, chunk) private(i)
	{
		#pragma omp for schedule(dynamic, chunk)
		for (i=0; i<len; i++) {
			long int c = in[i];
			int z;
			for (z = 1; z<exp; z++) {
				// printf("%d\n", tid);
				c *= in[i];
				c %= mod;
			}
			out[i] = c;
			// j++;
		}
	}
	out[len] = '\0';
	return 0;
}

// int long2string(long *in, char *out, size_t len) {
// 	for (int i=0; i<len; i++) {
// 		out[i] = in[i]+'0';
// 	}
// 	return 0;
// }

int long2char(long *in, char *out, size_t len) {
	int i;
	for (i=0; i<len; i++) {
		// printf("%ld\n", in[i]);
		out[i] = (char)in[i];
		// printf("%c\n", out[i]);
	}
	return 0;
}

int char2long(char *in, long *out, size_t len) {
	int i;
	for (i=0; i<len; i++) {
		out[i] = (int)in[i];
	}
	return 0;
}

int getKeys(int rank, long *pub, long *pri) {
	// long primeNum[42] = {5821, 5749, 1283, 1303, 1301, 1249, 1543, 1553, 1699, 1567, 2594, 2609, 2617, 2621, 2633, 2647, 2657, 2767, 2003, 2017, 2081,
	// 3001, 3023, 3037, 3109, 3163, 3221, 3191, 3361, 3371, 3529, 3533, 3539, 3671, 3673, 3677, 3793, 3821, 4729, 4831, 4073, 4001};

	//53, 71
	//83, 97
	//503, 557
	long primeNum[42] = {503, 557, 103, 107, 109, 113, 127, 131, 97, 89, 337, 101, 293, 347, 127, 389, 83, 113, 281, 193, 197,
	257, 211, 227, 229, 233, 223, 239, 241, 269, 263, 257, 251, 241, 271, 277, 307, 311, 331, 313, 103, 193};

	// long primeNum[42];
	// for (int i=0; i<42; i++) {
	// 	primeNum[i] = 109;
	// }

	long p = primeNum[rank*2];
	long q = primeNum[rank*2+1];
	// if (rank == 0) {
	// 	printf("%ld\n", p);
	// 	printf("%ld\n", q);
	// }


	publicKey(p, q, &pub[0], &pub[1]);
	// if (rank == 0) {
	// 	printf("%ld\n", pub[0]);
	// 	printf("%ld\n", pub[1]);
	// }
	privateKey(p, q, pub[0], &pri[0], &pri[1]);
	// printf("%d\n", rank);
	// if (rank == 0) {
	// 	printf("%ld\n", pri[0]);
	// 	printf("%ld\n", pri[1]);
	// }
}

// int main(int argc, char* argv[]) {
// 	long q = 503;
// 	long p = 557;
// 	long pube, pubmod, prive, privemod;
// 	int len;
// 	double start, end;
// 	long inmsg[maxlen];
// 	long outmsg[maxlen];
// 	long decrmsg[maxlen];
// 	char cinmsg[maxlen];
// 	char coutmsg[maxlen];
// 	char cdecrmsg[maxlen];

// 	sprintf(cinmsg, "%s", "abcde");
// 	len = strlen(cinmsg);
// 	printf("%s\n", cinmsg);
// 	// char2long(cinmsg, inmsg, len);
// 	// for (int i = 0; i<len; i++) {
// 	// 	printf("%ld", inmsg[i]);
// 	// }
// 	// long2char(inmsg, cinmsg, len);
// 	// printf("%s\n", cinmsg);

// 	publicKey(p, q, &pube, &pubmod);
// 	privateKey(p, q, pube, &prive, &privemod);

// 	printf("%ld, %ld\n", pube, pubmod);
// 	printf("%ld, %ld\n", prive, privemod);

// 	// start = omp_get_wtime();

// 	char2long(cinmsg, inmsg, len);
// 	encrypt(inmsg, pube, pubmod, outmsg, len);
// 	// for (int i = 0; i<len; i++) {
// 	// 	printf("%ld\t", outmsg[i]);
// 	// }
// 	decrypt(outmsg, prive, privemod, decrmsg, len);
// 	long2char(decrmsg, cdecrmsg, len);
// 	printf("%s\n", cdecrmsg);

// 	// end = omp_get_wtime();
//  //    printf("Process time Serial (s): %lf\n", end-start);



//     // start = omp_get_wtime();

// 	char2long(cinmsg, inmsg, len);
// 	encrypt_p(inmsg, pube, pubmod, outmsg, len);
// 	// for (int i = 0; i<len; i++) {
// 	// 	printf("%ld\t", outmsg[i]);
// 	// }
// 	decrypt_p(outmsg, prive, privemod, decrmsg, len);
// 	long2char(decrmsg, cdecrmsg, len);
// 	printf("%s\n", cdecrmsg);

// 	// end = omp_get_wtime();
//  //    printf("Process time Parallel (s): %lf\n", end-start);
// }