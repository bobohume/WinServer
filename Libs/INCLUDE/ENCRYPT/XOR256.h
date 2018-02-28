#ifndef __XOR256__
#define __XOR256__

class CXOR256
{
public:
	CXOR256(char* ucKey,int KeyLength)
	{
		if(KeyLength<1)
			throw exception("Incorrect key length");
		if(KeyLength>256)
			KeyLength = 256;
		memset(m_Key,0,sizeof(m_Key));
		memcpy(m_Key,ucKey,KeyLength);
	};
	
	void Crypt(char *InString, char* OutString, int BufferSize = 0)
	{
		char Sbox[257], Sbox2[257];

		char temp,k;
		temp = k = 0;

		unsigned long i,j,t;
		i = j = t = 0;

		memset(Sbox, 0,sizeof(Sbox));
		memset(Sbox2,0,sizeof(Sbox2));

		for(; i < 256U ; i++)
		{
			if(j == m_KeyLength)
				j = 0;
			Sbox[i] = (char)i;
			Sbox2[i] = m_Key[j++];
		} 

		j = 0 ;
		
		for(i = 0; i < 256U; i++)
		{
			j = (j + (unsigned long) Sbox[i] + (unsigned long) Sbox2[i]) % 256 ;
			temp =  Sbox[i];                    
			Sbox[i] = Sbox[j];
			Sbox[j] =  temp;
		}

		i = j  = 0;

		for(int x = 0; x < BufferSize; x++)
		{
			//increment i
			i = (i + 1U) % 256;
			//increment j
			j = (j + (unsigned long) Sbox[i]) % 256;

			//Scramble SBox #1 further so encryption routine will
			//will repeat itself at great interval
			temp = Sbox[i];
			Sbox[i] = Sbox[j] ;
			Sbox[j] = temp;

			//Get ready to create pseudo random  byte for encryption key
			t = ((unsigned long) Sbox[i] + (unsigned long) Sbox[j]) %  256 ;

			//get the random byte
			k = Sbox[t];

			//xor with the data and done
			OutString[x] = (InString[x] ^ k);
		}
	}

private:
	char	m_Key[257];
	int		m_KeyLength;
};

#endif