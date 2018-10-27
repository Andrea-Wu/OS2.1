//function prototypes
char* encrypt(char* key, char* str, char* enc);
void makeCapitalized(char* str);

char* encrypt(char* key, char* str, char* enc){
	//parameters:
		//key: self explanatory
		//str: the string to be encrypted
		//enc: pointer to encrypted string (assume already allocated)

    //TODO: for now the global userMessage == str because I'm not 100% sure
    //  how this will be implemented- will writing 2 strings consecutively to file concat them,
    //  or will second string overwrite the first?
    int key_len = strlen(key);
    int enc_len = strlen(enc); //the length of the old encryptedMessage that we will clear
    int str_len = strlen(str);
    int key_counter = 0; //this is used to cycle thru characters in key string
    char row; //this is a row of the "table" for ciphering
    char col; //this is a column of the "table" for ciphering
    //(note that we're not creating an actual table)

    int new_enc_len = 0; //this is for keeping track of the new encryptedMessage length
    char str_i;

    int i;
    //clear enc;
    for(i=0; i<enc_len; i++){
        enc[i] = '\0';
    }

    //make sure all letters in key are capitalized or else this algorithm will break
    makeCapitalized(key);

    //make sure all letters in user input string are capitalized
    makeCapitalized(str);

    //the cipher algorithm

    for(i=0;i<str_len; i++){
        //if character is non-alphabetic, just ignore it
        //this is essentially isalpha(), which isn't available in kernel
        str_i = str[i];
        if(!(str_i >= 97 && str_i <= 122) && !(str_i >= 65 && str_i <=90)){
            continue;
        }

        //value @ key[key_counter] = row -> row #
        //value @ str[i] = col -> col#

        //mapping characters to numbers
        //for example, A:0, B:1, ... Z:25
        row = key[key_counter] - 65;
        col = str[i] -65;

        //the value of the char in the return string is (( row + col) % 26) + 65
        enc[new_enc_len] = (char)(((row+col)%26) + 65);

        new_enc_len++;
        key_counter++;
        //if counter exceeds index of key, reset to 0
        if(key_counter == key_len){
            key_counter = 0;
        }
    }

    //append null terminator to encrypted string
    enc[new_enc_len] = '\0';
    return enc;
}

char* decrypt(char* key, char* encr, char* decr){
    /*parameters:
     * key: self-explanatory
     * encr: encrypted string (user input)
     * decr: location of string to return 
     */ 

    int encr_len = strlen(encr);
    int key_len = strlen(key);
    int key_counter = 0; //used to cycle thru letters in "key"       
    int new_dec_len = 0; //used to add to decrypted string
    int i;
    char key_char;
    char enc_char;
    char dec_char;

    makeCapitalized(key);
    makeCapitalized(encr);

    for(i=0;i<encr_len;i++){
        //if character is non-alphabetic, just ignore it
        //this is essentially isalpha(), which isn't available in kernel
        enc_char = encr[i];
        if(!(enc_char >= 97 && enc_char <= 122) && !(enc_char >= 65 && enc_char <=90)){
            continue;
        }

        key_char = key[key_counter];
        dec_char = enc_char - key_char;

        if(dec_char < 0){
            dec_char += 26;
        }
        
        decr[new_dec_len] = dec_char + 65;

        new_dec_len++;
        key_counter++;

        //if counter exceeds index of key, reset to 0
        if(key_counter == key_len){
            key_counter = 0;
        }
    }

    //append null terminator to encrypted string
    decr[new_dec_len] = '\0';
    return decr;
}

void makeCapitalized(char* str){
    int len = (int)strlen(str); 
    int i;
    for(i=0; i< len;i++){
         if(str[i] >= 97 && str[i] <= 122){
            str[i] = str[i] - 32;
        }
    }
}




