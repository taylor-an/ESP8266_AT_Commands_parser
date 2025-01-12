/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2015
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "buffer.h"

uint8_t BUFFER_Init(BUFFER_t* Buffer, uint16_t Size, uint8_t* BufferPtr) {
	/* Set buffer values to all zeros */
	memset(Buffer, 0, sizeof(BUFFER_t));
	
	/* Set default values */
	Buffer->Size = Size;
	Buffer->Buffer = BufferPtr;
	Buffer->StringDelimiter = '\n';
	
	/* Check if malloc should be used */
	if (!Buffer->Buffer) {
		/* Try to allocate */
		Buffer->Buffer = (uint8_t *) LIB_ALLOC_FUNC(Size * sizeof(uint8_t));
		
		/* Check if allocated */
		if (!Buffer->Buffer) {
			/* Reset size */
			Buffer->Size = 0;
			
			/* Return error */
			return 1;
		} else {
			/* Set flag for malloc */
			Buffer->Flags |= BUFFER_MALLOC;
		}
	}
	
	/* We are initialized */
	Buffer->Flags |= BUFFER_INITIALIZED;
	
	/* Initialized OK */
	return 0;
}

void BUFFER_Free(BUFFER_t* Buffer) {
	/* Check buffer structure */
	if (Buffer == NULL) {
		return;
	}
	
	/* If malloc was used for allocation */
	if (Buffer->Flags & BUFFER_MALLOC) {
		/* Free memory */
		LIB_FREE_FUNC(Buffer->Buffer);
	}
	
	/* Clear flags */
	Buffer->Flags = 0;
	Buffer->Size = 0;
}

uint16_t BUFFER_Write(BUFFER_t* Buffer, uint8_t* Data, uint16_t count) {
	uint8_t i = 0;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}

	/* Check input pointer */
	if (Buffer->In >= Buffer->Size) {
		Buffer->In = 0;
	}
	
	/* Go through all elements */
	while (count--) {
		/* Check if buffer full */
		if (
			(Buffer->In == (Buffer->Out - 1)) ||
			(Buffer->Out == 0 && Buffer->In == (Buffer->Size - 1))
		) {
			break;
		}
		
		/* Add to buffer */
		Buffer->Buffer[Buffer->In++] = *Data++;
		
		/* Increase pointers */
		i++;
		
		/* Check input overflow */
		if (Buffer->In >= Buffer->Size) {
			Buffer->In = 0;
		}
	}
	
	/* Return number of elements stored in memory */
	return i;
}

uint16_t BUFFER_Read(BUFFER_t* Buffer, uint8_t* Data, uint16_t count) {
	uint16_t i = 0;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}

	/* Check output pointer */
	if (Buffer->Out >= Buffer->Size) {
		Buffer->Out = 0;
	}
	
	/* Go through all elements */
	while (count--) {
		/* Check if pointers are same = buffer is empty */
		if (Buffer->Out == Buffer->In) {
			break;
		}
		
		/* Save to user buffer */
		*Data++ = Buffer->Buffer[Buffer->Out++];
		
		/* Increase pointers */
		i++;

		/* Check output overflow */
		if (Buffer->Out >= Buffer->Size) {
			Buffer->Out = 0;
		}
	}
	
	/* Return number of elements read from buffer */
	return i;
}

uint16_t BUFFER_GetFree(BUFFER_t* Buffer) {
	uint32_t size, in, out;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Save values */
	in = Buffer->In;
	out = Buffer->Out;
	
	/* Check if the same */
	if (in == out) {
		size = Buffer->Size;
	}

	/* Check normal mode */
	if (out > in) {
		size = out - in;
	}
	
	/* Check if overflow mode */
	if (in > out) {
		size = Buffer->Size - (in - out);
	}
	
	/* Return free memory */
	return size - 1;
}

uint16_t BUFFER_GetFull(BUFFER_t* Buffer) {
	uint32_t in, out, size;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Save values */
	in = Buffer->In;
	out = Buffer->Out;
	
	/* Pointer are same? */
	if (in == out) {
		size = 0;
	}
	
	/* Check pointers and return values */
	/* Buffer is not in overflow mode */
	if (in > out) {
		size = in - out;
	}
	
	/* Buffer is in overflow mode */
	if (out > in) {
		size = Buffer->Size - (out - in);
	}
	
	/* Return number of elements in buffer */
	return size;
}

void BUFFER_Reset(BUFFER_t* Buffer) {
	/* Check buffer structure */
	if (Buffer == NULL) {
		return;
	}
	
	/* Reset values */
	Buffer->In = 0;
	Buffer->Out = 0;
}

int16_t BUFFER_FindElement(BUFFER_t* Buffer, uint8_t Element) {
	uint16_t Num, Out, retval = 0;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return -1;
	}
	
	/* Create temporary variables */
	Num = BUFFER_GetFull(Buffer);
	Out = Buffer->Out;
	
	/* Go through input elements */
	while (Num > 0) {
		/* Check output overflow */
		if (Out >= Buffer->Size) {
			Out = 0;
		}
		
		/* Check for element */
		if ((uint8_t)Buffer->Buffer[Out] == (uint8_t)Element) {
			/* Element found, return position in buffer */
			return retval;
		}
		
		/* Set new variables */
		Out++;
		Num--;
		retval++;
	}
	
	/* Element is not in buffer */
	return -1;
}

int16_t BUFFER_Find(BUFFER_t* Buffer, uint8_t* Data, uint16_t Size) {
	uint16_t Num, Out, i, retval = 0;
	uint8_t found = 0;

	/* Check buffer structure and number of elements in buffer */
	if (Buffer == NULL || (Num = BUFFER_GetFull(Buffer)) < Size) {
		return -1;
	}

	/* Create temporary variables */
	Out = Buffer->Out;

	/* Go through input elements in buffer */
	while (Num > 0) {
		/* Check output overflow */
		if (Out >= Buffer->Size) {
			Out = 0;
		}

		/* Check if current element in buffer matches first element in data array */
		if ((uint8_t)Buffer->Buffer[Out] == (uint8_t)Data[0]) {
			found = 1;
		}

		/* Set new variables */
		Out++;
		Num--;
		retval++;

		/* We have found first element */
		if (found) {
			/* First character found */
			/* Check others */
			i = 1;
			while (i < Size) {
				/* Check output overflow */
				if (Out >= Buffer->Size) {
					Out = 0;
				}

				/* Check if current character in buffer matches character in string */
				if ((uint8_t)Buffer->Buffer[Out] != (uint8_t)Data[i]) {
					retval += i - 1;
					break;
				}

				/* Set new variables */
				Out++;
				Num--;
				i++;
			}

			/* We have found data sequence in buffer */
			if (i == Size) {
				return retval - 1;
			}
		}
	}

	/* Data sequence is not in buffer */
	return -1;
}

uint16_t BUFFER_WriteString(BUFFER_t* Buffer, char* buff) {
	/* Write string to buffer */
	return BUFFER_Write(Buffer, (uint8_t *)buff, strlen(buff));
}

uint16_t BUFFER_ReadString(BUFFER_t* Buffer, char* buff, uint16_t buffsize) {
	uint16_t i = 0;
	uint8_t ch;
	uint16_t freeMem;
	
	/* Check value buffer */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Get free */
	freeMem = BUFFER_GetFree(Buffer);
	
	/* Check for any data on USART */
	if (
		freeMem == 0 ||                                                /*!< Buffer empty */
		(
			BUFFER_FindElement(Buffer, Buffer->StringDelimiter) < 0 && /*!< String delimiter is not in buffer */
			freeMem != 0 &&                                            /*!< Buffer is not full */
			BUFFER_GetFull(Buffer) < buffsize                          /*!< User buffer size is larger than number of elements in buffer */
		)
	) {
		/* Return 0 */
		return 0;
	}
	
	/* If available buffer size is more than 0 characters */
	while (i < (buffsize - 1)) {
		/* We have available data */
		BUFFER_Read(Buffer, &ch, 1);
		
		/* Save character */
		buff[i] = (char)ch;
		
		/* Check for end of string */
		if ((char)buff[i] == (char)Buffer->StringDelimiter) {
			/* Done */
			break;
		}
		
		/* Increase */
		i++;
	}
	
	/* Add zero to the end of string */
	if (i == (buffsize - 1)) {
		buff[i] = 0;
	} else {
		buff[++i] = 0;
	}

	/* Return number of characters in buffer */
	return i;
}

int8_t BUFFER_CheckElement(BUFFER_t* Buffer, uint16_t pos, uint8_t* element) {
	uint16_t In, Out, i = 0;
	
	/* Check value buffer */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Read current values */
	In = Buffer->In;
	Out = Buffer->Out;
	
	/* Set pointers to right location */
	while (i < pos && (In != Out)) {
		/* Increase output pointer */
		Out++;
		i++;
		
		/* Check overflow */
		if (Out >= Buffer->Size) {
			Out = 0;
		}
	}
	
	/* If positions match */
	if (i == pos) {
		/* Save element */
		*element = Buffer->Buffer[Out];
		
		/* Return OK */
		return 1;
	}
	
	/* Return zero */
	return 0;
}
