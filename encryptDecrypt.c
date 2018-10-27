//function prototypes
char* encrypt(char* key, char* str, char* enc);

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
    for(i=0; i<key_len;i++){
         if(key[i] >= 97 && key[i] <= 122){
            key[i] = key[i] - 22;
        }
    }

    //the cipher algorithm

    for(i=0;i<str_len; i++){
        //if character is non-alphabetic, just ignore it
        //this is essentially isalpha(), which isn't available in kernel
        str_i = str[i];
        if(!(str_i >= 97 && str_i <= 122) && !(str_i >= 65 && str_i <=90)){
            continue;
        }

        //if letter is not capitalized, make it capitalized
        if(str_i >= 97 && str_i <= 122){
            str[i] = str_i - 32;
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

