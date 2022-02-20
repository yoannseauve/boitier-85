#include <stdarg.h>

//return number of writed bytes
//buffer must be 10bytes long
//write right aligned text version of var, without '\0' at the end
unsigned char mini_itos(char* buffer, unsigned int var)
{
	char *p = buffer + 9;	
	do{
		*p-- = "0123456789"[var % 10];

	} while (var /= 10);
	return buffer + 9 - p;
}

int mini_snprintf(char buffer[], unsigned int bufferSize, const char* formatString, ...)
{
	va_list valist;

	/* initialize valist for num number of arguments */
	va_start(valist, formatString);

	unsigned int buffCount = 0;
	unsigned char count;
	char buffConvert[10];

	while(*formatString != '\0' && buffCount < bufferSize - 1)
	{
		if(*formatString == '%')
		{
			switch(*++formatString)
			{
				case 'u':
					count = mini_itos(buffConvert, va_arg(valist, unsigned int));	
					do {
						buffer[buffCount++] = buffConvert[10 - count--];
						if (buffCount >= bufferSize - 1)
							break;	
					} while(count > 0);
					break;

				case 'd':
					{
						int num = va_arg(valist, int);
						if (num <0)
						{
							buffer[buffCount++] = '-';
							num *= -1;
							if (buffCount >= bufferSize - 1)
								break;
						}
						count = mini_itos(buffConvert, num);
						do {
							buffer[buffCount++] = buffConvert[10 - count--];
							if (buffCount >= bufferSize - 1)
								break;	
						} while(count > 0);
						break;
					}

				case 's':
					{
						char *string = va_arg(valist, char*);
						while(*string != '\0')
						{
							buffer[buffCount++] = *string++;
							if (buffCount >= bufferSize - 1)
								break;
						}
						break;
					}
				default :
					return -1;
			}
		}
		else
		{
			buffer[buffCount++] = *formatString;
		}
		formatString++;
	} 
	buffer[buffCount] = '\0';

	/* clean memory reserved for valist */
	va_end(valist);

	return  buffCount;
}


int mini_snscanf(char buffer[], unsigned int bufferSize, const char* formatString, ...)
{
	va_list valist;

	/* initialize valist for num number of arguments */
	va_start(valist, formatString);

	unsigned int buffCount = 0;
	int numberBuffer;

	while(*formatString != '\0' && buffCount < bufferSize - 1)
	{

		if(*formatString == '%')
		{
			switch(*++formatString)
			{
				case 'u':
					numberBuffer = 0;
					while(buffer[buffCount] >= '0' && buffer[buffCount] <= '9')
					{
						numberBuffer = 10*numberBuffer + buffer[buffCount++] - '0';
						if (buffCount >= bufferSize - 1)
							break;
					}
					*va_arg(valist, unsigned int*) = numberBuffer;
					break;

				case 'x':
					numberBuffer = 0;
					char caracType;
					while(caracType = (buffer[buffCount] >= '0' && buffer[buffCount] <= '9')? '0' : (buffer[buffCount] >= 'a' && buffer[buffCount] <= 'f')? 'a' : (buffer[buffCount] >= 'A' && buffer[buffCount] <= 'F')? 'A' : 0)
					{
						numberBuffer = 16*numberBuffer + buffer[buffCount++] - caracType;
						if (buffCount >= bufferSize - 1)
							break;
					}
					*va_arg(valist, unsigned int*) = numberBuffer;
					break;
				default :
					return 0;
			}
			formatString++;
		}
		else
		{
			if(*(formatString++) != buffer[buffCount++])
			{
				va_end(valist);
				return 0;
			}
		}
	}
	va_end(valist);
	return buffCount;
}
