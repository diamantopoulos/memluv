
/**
 * @file   src/gen_strmatch.c
 * @author Dionysios Diamantopoulos(diamantd@microlab.ntua.gr)
 * @date   April 2014
 *
 * @brief   Generating string match accelerators for memluv evaluation.
 */

#include "../memluv.h"
#include "../gen_memluv_wrappers.h"
#include "gen_accelerators.h"
#include "gen_strmatch.h"


void GenKernelStrmatch(FILE *fd) {
	if (NUM_STRMATCH_ACCELERATORS != 0) {
		fprintf(fd, "#include \"../src/gen_kernels/gen_strmatch.h\"				\n\n");
		fprintf(fd, "#define SALT_SIZE 2							\n");
		fprintf(fd, "#define MAX_REC_LEN STRMATCH_DEF_MAX_REC_LEN				\n");
		fprintf(fd, "#define OFFSET 5								\n\n");
		
		fprintf(fd, "typedef struct {								\n");
		fprintf(fd, "\tint chars_per_line;							\n");
		fprintf(fd, "\tint lines_per_file;							\n");
		fprintf(fd, "\tint keys_file_len;							\n");
		fprintf(fd, "\t} str_data_t;								\n\n");

		fprintf(fd, "/** strlen_syn()				\n");
		fprintf(fd, " *  Get string length - HLS version	\n");
		fprintf(fd, " */					\n");
		fprintf(fd, "size_t strlen_syn(const char *s);		\n");
		fprintf(fd, "size_t strlen_syn(const char *s) {		\n");
		fprintf(fd, "#ifdef __SYNTHESIS__			\n");
		fprintf(fd, "#pragma AP inline				\n");
		fprintf(fd, "#endif					\n");
		fprintf(fd, "\tconst char *p = s;			\n");
		fprintf(fd, "\t/* Loop over the data in s.  */		\n");
		fprintf(fd, "\twhile (*p != '\\0')			\n");
		fprintf(fd, "\t\tp++;					\n");
		fprintf(fd, "\treturn (size_t)(p - s);			\n");
		fprintf(fd, "}						\n\n");

		fprintf(fd, "/** strcmp_syn()						\n");
		fprintf(fd, " *  Compare two strings  - HLS version			\n");
		fprintf(fd, " */							\n");
		fprintf(fd, "int strcmp_syn(const char *s1, const char *s2);		\n");
		fprintf(fd, "int strcmp_syn(const char *s1, const char *s2) {		\n");
		fprintf(fd, "#ifdef __SYNTHESIS__					\n");
		fprintf(fd, "#pragma AP inline						\n");
		fprintf(fd, "#endif							\n");
		fprintf(fd, "\tunsigned char uc1, uc2;					\n");
		fprintf(fd, "\t/* Move s1 and s2 to the first differing characters 	\n");
		fprintf(fd, "\tin each string, or the ends of the strings if they	\n");
		fprintf(fd, "\tare identical.  */					\n");
		fprintf(fd, "\twhile (*s1 != '\\0' && *s1 == *s2) {			\n");
		fprintf(fd, "\t\ts1++;							\n");
		fprintf(fd, "\t\ts2++;							\n");
		fprintf(fd, "\t}							\n");
		fprintf(fd, "\t/* Compare the characters as unsigned char and		\n");
		fprintf(fd, "\treturn the difference.  */				\n");
		fprintf(fd, "\tuc1 = (*(unsigned char *) s1);				\n");
		fprintf(fd, "\tuc2 = (*(unsigned char *) s2);				\n");
		fprintf(fd, "\treturn ((uc1 < uc2) ? -1 : (uc1 > uc2));			\n");
		fprintf(fd, "}								\n");
		
		
		fprintf(fd, "/** bzero_syn()					\n");
		fprintf(fd, " *  Write zero-valued bytes - HLS version		\n");
		fprintf(fd, " */						\n");
		fprintf(fd, "void bzero_syn(void * s1, size_t n);		\n");
		fprintf(fd, "void bzero_syn(void * s1, size_t n) {		\n");
		fprintf(fd, "#ifdef __SYNTHESIS__				\n");
		fprintf(fd, "#pragma AP inline					\n");
		fprintf(fd, "#endif						\n");
		fprintf(fd, "\tchar *dst = s1;					\n");
		fprintf(fd, "\t/* Loop and copy.  */				\n");
		fprintf(fd, "\twhile (n-- != 0)					\n");
		fprintf(fd, "\t\t*dst++ = '\\0';				\n");
		fprintf(fd, "}							\n");
		
		fprintf(fd, "/** generate_keys_file()							\n");
		fprintf(fd, " *  Generate the keys_file							\n");
		fprintf(fd, " */									\n");
		fprintf(fd, "void generate_keys_file(char* keys_file, str_data_t *str_data);		\n");
		fprintf(fd, "void generate_keys_file(char* keys_file, str_data_t *str_data) {		\n");
		fprintf(fd, "#ifdef __SYNTHESIS__							\n");
		fprintf(fd, "#pragma AP inline								\n");
		fprintf(fd, "#endif									\n");
		fprintf(fd, "\tint i;									\n");
#if RANDOM_STRMATCH_VALUES == 1	
		fprintf(fd, "\tunsigned short heap_lfsr_ptr;						\n");
		fprintf(fd, "\theap_lfsr_ptr = 0xACE1u;							\n");
#endif
		fprintf(fd, "\tfor (i=0; i<str_data->keys_file_len; i++) {				\n");
#if RANDOM_STRMATCH_VALUES == 1
		fprintf(fd, "\t\t\tkeys_file[i] = (char)RandMinMaxSyn(97, 122, &heap_lfsr_ptr, ");
#if USE_MEMLUV == 1
	fprintf(fd, "1);	\n");
#else
	fprintf(fd, "0);	\n");
#endif
#else
		fprintf(fd, "\t\tkeys_file[i] = (char)(i);	\n");
#endif
		fprintf(fd, "\t\tif (i %% (str_data->chars_per_line + 1) == 0)		\n");
		fprintf(fd, "\t\t\tkeys_file[i] = \'\\n\';				\n");
		fprintf(fd, "\t}							\n");
		fprintf(fd, "}								\n\n");

		fprintf(fd, "/** show_keys_file()						\n");
		fprintf(fd, " *  Show the keys_file						\n");
		fprintf(fd, " */								\n");
		fprintf(fd, "void show_keys_file(char* keys_file, str_data_t *str_data);	\n");
		fprintf(fd, "void show_keys_file(char* keys_file, str_data_t *str_data) {	\n");
		fprintf(fd, "\tint i;								\n");
		fprintf(fd, "\tprintf(\"Dumping keys_file: chars_per_line = %%d\\tlines_per_file = %%d\\tkeys_file_len = %%d\\n\", str_data->chars_per_line, str_data->lines_per_file, str_data->keys_file_len);\n");
		fprintf(fd, "\tfor (i=0; i<str_data->keys_file_len; i++) {			\n");
		fprintf(fd, "\t\tprintf(\"%%c\", keys_file[i]);					\n");
		fprintf(fd, "\t}								\n");
		fprintf(fd, "}									\n\n");
		
		fprintf(fd, "/** getnextline()						\n");
		fprintf(fd, " *  Function to get the next word				\n");
		fprintf(fd, " */							\n");
		fprintf(fd, "int getnextline(char* output, int max_len, char* file);	\n");
		fprintf(fd, "int getnextline(char* output, int max_len, char* file) {	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__					\n");
		fprintf(fd, "#pragma AP inline						\n");
		fprintf(fd, "#endif							\n");
		fprintf(fd, "\tint i=0;							\n");
		fprintf(fd, "\twhile(i<max_len-1) {					\n");
		fprintf(fd, "\t\tif( file[i] == '\\0') {				\n");
		fprintf(fd, "\t\t\tif(i==0)						\n");
		fprintf(fd, "\t\t\t\treturn -1;						\n");
		fprintf(fd, "\t\t\telse							\n");
		fprintf(fd, "\t\t\t\treturn i;						\n");
		fprintf(fd, "\t\t}							\n");
		fprintf(fd, "\t\tif( file[i] == '\\r')					\n");
		fprintf(fd, "\t\t\treturn (i+2);					\n");
		fprintf(fd, "\t\tif( file[i] == '\\n' )					\n");
		fprintf(fd, "\t\t\treturn (i+1);					\n");
		fprintf(fd, "\t\toutput[i] = file[i];					\n");
		fprintf(fd, "\t\ti++;							\n");
		fprintf(fd, "\t}							\n");
		fprintf(fd, "\tfile+=i;							\n");
		fprintf(fd, "\treturn i;						\n");
		fprintf(fd, "}								\n\n");
		
		fprintf(fd, "/** compute_hashes()					\n");
		fprintf(fd, " *  Simple Cipher to generate a hash of the word 		\n");
		fprintf(fd, " */							\n");
		fprintf(fd, "void compute_hashes(const char* word, char* final_word);	\n");
		fprintf(fd, "void compute_hashes(const char* word, char* final_word) {	\n");
		fprintf(fd, "#ifdef __SYNTHESIS__					\n");
		fprintf(fd, "#pragma AP inline						\n");
		fprintf(fd, "#endif							\n");
		fprintf(fd, "\tunsigned int i;						\n");
		fprintf(fd, "\tfor(i=0;i<strlen_syn(word);i++) {				\n");
		fprintf(fd, "\t\tfinal_word[i] = (char)(word[i]+OFFSET);		\n");
		fprintf(fd, "\t}							\n");
		fprintf(fd, "\tfinal_word[i] = '\\0';					\n");
		fprintf(fd, "}								\n\n");
	}
	int i;
	for (i = 0; i < NUM_STRMATCH_ACCELERATORS; i++) {
		fprintf(fd, "\n/* STRMATCH KERNEL %d */								\n", i);
		fprintf(fd, "int STRMATCH_k%d(void);								\n", i);
		fprintf(fd, "int STRMATCH_k%d(void) {								\n", i);
		fprintf(fd, "#ifdef __SYNTHESIS__								\n");
		fprintf(fd, "#pragma AP inline ");
		if (STRMATCH_HLS_INLINE== 0) 
			fprintf(fd, "off									\n");
		else if (STRMATCH_HLS_INLINE== 2) 
			fprintf(fd, "recursive									\n");
		else
			fprintf(fd, "										\n");
		fprintf(fd, "#endif										\n");
		fprintf(fd, "\tint main_result_STRMATCH_k%d=0, N_STRMATCH_k%d = N_STRMATCH_MAX;			\n", i, i);
		fprintf(fd, "\tchar *keys_file;									\n");
#if USE_MEMLUV == 1
		fprintf(fd, "\tstr_data_t *str_data = (str_data_t *)MemluvAlloc((uint_t)sizeof(str_data_t), %d);\n", i%NUM_HW_HEAPS);
#else
		fprintf(fd, "\tstr_data_t str_data_static; 							\n");
		fprintf(fd, "\tstr_data_t *str_data = &str_data_static; 					\n");
#endif
		fprintf(fd, "\tstr_data->chars_per_line = STRMATCH_DEF_CHARS_PER_LINE;				\n");
		fprintf(fd, "\tstr_data->lines_per_file = STRMATCH_DEF_LINES_PER_FILE;								\n");
		fprintf(fd, "\tstr_data->keys_file_len = (str_data->chars_per_line + 1) * str_data->lines_per_file;				\n");
#if USE_MEMLUV == 1
		fprintf(fd, "\tkeys_file = (char *)MemluvAlloc((uint_t)((unsigned int)str_data->keys_file_len * (uint_t)sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
#else
		fprintf(fd, "\tchar keys_file_s[str_data->keys_file_len];									\n");
		fprintf(fd, "\tkeys_file =  keys_file_s;											\n");

#endif
		fprintf(fd, "\tgenerate_keys_file(keys_file, str_data); 									\n");
#ifndef __SYNTHESIS__
#if STRMATCH_DEF_SHOW_KEYS == 1
		fprintf(fd, "\tshow_keys_file(keys_file, str_data); 										\n");
#endif
#endif
                fprintf(fd, "\tunsigned int i;                                          \n");
		fprintf(fd, "\tunsigned int key1_found=0, key2_found=0, key3_found=0, key4_found=0;	\n");
		fprintf(fd, "\tconst char *key1 = \"Helloworld\";					\n");
		fprintf(fd, "\tconst char *key2 = \"howareyou\";					\n");
		fprintf(fd, "\tconst char *key3 = \"ferrari\";						\n");
		fprintf(fd, "\tconst char *key4 = \"whotheman\";					\n");
#if USE_MEMLUV == 1
		fprintf(fd, "\tchar *key1_final = (char *)MemluvAlloc((uint_t)(strlen_syn(key1) + 1)*sizeof(char), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tchar *key2_final = (char *)MemluvAlloc((uint_t)(strlen_syn(key2) + 1)*sizeof(char), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tchar *key3_final = (char *)MemluvAlloc((uint_t)(strlen_syn(key3) + 1)*sizeof(char), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tchar *key4_final = (char *)MemluvAlloc((uint_t)(strlen_syn(key4) + 1)*sizeof(char), %d);	\n", i%NUM_HW_HEAPS);
#else
 		fprintf(fd, "\tchar key1_final[11];							\n");
		fprintf(fd, "\tchar key2_final[10];							\n");
		fprintf(fd, "\tchar key3_final[8];							\n");
		fprintf(fd, "\tchar key4_final[10];							\n");
#endif
		fprintf(fd, "\tcompute_hashes(key1, key1_final);					\n");
		fprintf(fd, "\tcompute_hashes(key2, key2_final);					\n");
		fprintf(fd, "\tcompute_hashes(key3, key3_final);					\n");
		fprintf(fd, "\tcompute_hashes(key4, key4_final);					\n");

		fprintf(fd, "\tunsigned int key_len=0;								\n");
#if USE_MEMLUV == 1
		fprintf(fd, "\tchar * cur_word = (char *)MemluvAlloc((uint_t)MAX_REC_LEN*sizeof(char), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tchar * cur_word_final = (char *)MemluvAlloc((uint_t)MAX_REC_LEN*sizeof(char), %d);	\n", i%NUM_HW_HEAPS);
#else
		fprintf(fd, "\tchar cur_word[MAX_REC_LEN];						\n");
		fprintf(fd, "\tchar cur_word_final[MAX_REC_LEN];					\n");
#endif
		fprintf(fd, "\tbzero_syn(cur_word, MAX_REC_LEN);					\n");
		fprintf(fd, "\tbzero_syn(cur_word_final, MAX_REC_LEN);					\n");

		//fprintf(fd, "\twhile( (key_len = getnextline(cur_word, MAX_REC_LEN, keys_file))>=0)  {	\n");
		fprintf(fd, "\twhile(key_len<STRMATCH_DEF_CHARS_PER_LINE*STRMATCH_DEF_LINES_PER_FILE/STRMATCH_DEF_MAX_REC_LEN)  {	\n");
                
                fprintf(fd, "\t\tfor(i=0;i<STRMATCH_DEF_MAX_REC_LEN;i++)                                \n");
                fprintf(fd, "\t\t\tcur_word[i] = keys_file[i+key_len];                            \n");
                
		fprintf(fd, "\t\tcompute_hashes(cur_word, cur_word_final);				\n");
		fprintf(fd, "\t\tif(!strcmp_syn(key1_final, cur_word_final))				\n");
		fprintf(fd, "\t\t\tkey1_found++;							\n");
		fprintf(fd, "\t\tif(!strcmp_syn(key2_final, cur_word_final))				\n");
		fprintf(fd, "\t\t\tkey2_found++;							\n");
		fprintf(fd, "\t\tif(!strcmp_syn(key3_final, cur_word_final))				\n");
		fprintf(fd, "\t\t\tkey3_found++;							\n");
		fprintf(fd, "\t\tif(!strcmp_syn(key4_final, cur_word_final))				\n");
		fprintf(fd, "\t\t\tkey4_found++;							\n");
                fprintf(fd, "\t\tkey_len+=STRMATCH_DEF_MAX_REC_LEN;                                     \n");
		fprintf(fd, "\t\t/* keys_file = keys_file + key_len;	FIXME: How we process next line withour pointer arithmetic? */	\n");
		fprintf(fd, "\t\tbzero_syn(cur_word, MAX_REC_LEN);					\n");
		fprintf(fd, "\t\tbzero_syn(cur_word_final, MAX_REC_LEN);				\n");
		fprintf(fd, "\t}									\n");
#if USE_MEMLUV == 1 && FREE_INPUTS == 1
		fprintf(fd, "\tMemluvFree((void*)cur_word, (uint_t)MAX_REC_LEN*(sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tMemluvFree((void*)cur_word_final, (uint_t)MAX_REC_LEN*(sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tMemluvFree((void*)key1_final, (uint_t)(strlen_syn(key1) + 1)*(sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tMemluvFree((void*)key2_final, (uint_t)(strlen_syn(key2) + 1)*(sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tMemluvFree((void*)key3_final, (uint_t)(strlen_syn(key3) + 1)*(sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tMemluvFree((void*)key4_final, (uint_t)(strlen_syn(key4) + 1)*(sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
#endif
		fprintf(fd, "\tmain_result_STRMATCH_k%d = (int)(key1_found + key2_found + key3_found + key4_found);	\n", i);
#if USE_MEMLUV == 1 && FREE_INPUTS == 1
		fprintf(fd, "\tMemluvFree((void*)keys_file, (uint_t)((unsigned int)str_data->keys_file_len * (uint_t)sizeof(char)), %d);	\n", i%NUM_HW_HEAPS);
		fprintf(fd, "\tMemluvFree((void*)str_data, (uint_t)(sizeof(str_data_t)), %d);			\n", i%NUM_HW_HEAPS);
#endif
		fprintf(fd, "\treturn (main_result_STRMATCH_k%d + N_STRMATCH_k%d);				\n", i, i);
		fprintf(fd, "}										\n");
	}
}
