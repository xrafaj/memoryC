#include <stdio.h>

#define ADJUSTOR 8

void* pointer;

// funkcia, ktorá premení znamienka zavolanému pointeru
void mem_free(void *ptr)
{
	*(int *)ptr = abs(*(int *)ptr );
	*(int *)(ptr + sizeof(int) + *(int*)ptr) = abs(*(int *)(ptr + sizeof(int) + *(int*)ptr));
}

//funkcia na spájanie blockov, case = 1 spájanie vpravo, 2 v¾avo, 3 do oboch strán
void merge_ptr(void *ptr, int caseOfMerge)
{
	void* temp_ptr = ptr;
	int temp;
	int tempSize = ADJUSTOR;
	if(caseOfMerge == 1 )
	{		
		tempSize += *(int *)ptr;
		
		temp_ptr = (char *)temp_ptr + *(int *)temp_ptr + ADJUSTOR;
		temp = *(int *)temp_ptr;
		tempSize += *(int *)temp_ptr;
		
		*(int *)temp_ptr = 0;
		temp_ptr = (char *)temp_ptr - sizeof(int);
		*(int *)temp_ptr = 0;
		
		temp_ptr = (char *)temp_ptr + temp + 2*sizeof(int);
		*(int *)temp_ptr = tempSize;
		*(int *)ptr = tempSize;
	}
	if(caseOfMerge == 2 )
	{
		if(ptr==pointer)
		{
			return;
		}
		ptr = (char *)ptr - sizeof(int);
		ptr = (char *)ptr - ADJUSTOR - abs(*(int *)ptr) + sizeof(int);
		merge_ptr(ptr,1);
	}
	if(caseOfMerge == 3 )
	{
		if(ptr == pointer)
		{
			merge_ptr(ptr,1);
			return;
		}
		temp_ptr = (char *)temp_ptr - sizeof(int);
		temp_ptr = (char *)temp_ptr - ADJUSTOR - *(int *)temp_ptr + sizeof(int);
		
		merge_ptr(ptr,1);
		ptr -= sizeof(int);
		merge_ptr(ptr-sizeof(int)-*(int *)ptr,1);
		return;
	}
}

int memory_free(void* valid_ptr)
{
	//kontrola podmienok
	if(pointer==NULL || valid_ptr == NULL){
		return 1;
	}
	if(*(int*)valid_ptr>0)return 1;
	void *new_ptr = valid_ptr;
	
	// kontrolujem strany, kde sú vo¾né bloky
	if( (*(int *)(new_ptr-sizeof(int)) < 0) && (*(int*)(new_ptr+ADJUSTOR+abs(*(int*)(new_ptr))) < 0 ) ) 
	{
		mem_free(new_ptr);
		return 0;
	}
	else if( (*(int *)(new_ptr-sizeof(int)) < 0) )
	{
		mem_free(new_ptr);
		merge_ptr(new_ptr,1);
		return 0;
	}
	else if( *(int*)(new_ptr+ADJUSTOR+abs(*(int*)(new_ptr))) < 0  )
	{
		mem_free(new_ptr);
		merge_ptr(new_ptr,2);
		return 0;
	}
	else if ( (*(int*)(new_ptr+ADJUSTOR+abs(*(int*)(new_ptr))) > 0 ) && (*(int *)(new_ptr-sizeof(int)) > 0) )
	{
		mem_free(new_ptr);
		merge_ptr(new_ptr,3);
		return 0;
	}
	else return 1;
}

int decide(void *ptr)
{
	if(*(int *)ptr < 0)return 0;
	else return 1;
}

//rekurzívne h¾adanie vo¾ného bloku pre alokáciu pamäte
void *firstFit(void* ptr, unsigned int size)
{
	if( decide(ptr) )
	{
		if( (size) <= *(int *)ptr )
		{
			return ptr;
		}
		else 
		{		
			if( ( (char *)ptr + size + ADJUSTOR ) < (char *)pointer + *(int *)(pointer-4)	){
				firstFit((char *)ptr + size + ADJUSTOR,size);
			}
			else {
				return NULL;
			}
		}
	}
	
	if( ( ptr + abs(*(int *)ptr) ) < ( pointer + *(int*)(pointer-sizeof(int)) ) )
	{
		firstFit ( ptr + abs(*(int*)ptr) + ADJUSTOR, size);
		return;
	}
	return NULL;
}

void* memory_alloc(unsigned int size)
{
	if(pointer==NULL){
		return NULL;
	}
	void* freePointer = NULL;
	void* helpPointer = NULL;
	void* temp = NULL;
	freePointer = firstFit(pointer,size);
	if(freePointer == NULL)
	{
		return NULL;
	}
	
	//ak je presne rovnaká ve¾kos ušetrenie aritmetiky
	if(size==*(int*)freePointer)
	{
		helpPointer = freePointer;
		*(int *)freePointer *= (-1);
		freePointer = (char *)freePointer + sizeof(int) + abs(*(int*)freePointer);
		*(int *)freePointer *= (-1);
		
		return helpPointer;
	}
	
	// zaokrúhlovanie ve¾kosti, v prípade, že by sa tam už nedala urobi hlavièka a pätièka
	if( ( ( ADJUSTOR + *(int*)freePointer) - ( ADJUSTOR + size ) ) < 10 ) 
	{
		memory_alloc(*(int*)freePointer);
		return;
	}
	
	// bežný malloc bez špeciálneho prípadu, aritmetika hlavièiek a pätièiek
	
	temp = (char *)freePointer + abs(size) + ADJUSTOR;
	*(int *)temp = *(int *)freePointer - abs(size) - ADJUSTOR;
	
	temp = (char *)temp + sizeof(int) + abs(*(int*)temp);
	*(int *)temp = *(int *)freePointer - size - ADJUSTOR;
	
	*(int *)freePointer = -size;
	helpPointer = freePointer;
	freePointer = (char *)freePointer + sizeof(int) + size;
	*(int *)freePointer = -size;

	return helpPointer;
}

int searchArray(void* ptr,void* temp)
{
	if(ptr==temp && (char *)temp < (char *)pointer + *(int *)(pointer-4))
	{
		if(*(int *)ptr<0)return 0;
		return 1;
	}
	if(ptr!=temp)
	{
		if( ( (char *)temp + abs(*(int *)temp) + ADJUSTOR ) < (char *)pointer + *(int *)(pointer-4) )searchArray(ptr,(char *)temp + abs(*(int *)temp) + ADJUSTOR);
		else return 1;
	}
}

int memory_check(void* ptr)
{
	// kontrola je popísaná v dokumentácii 
	int vysledok = searchArray(ptr,pointer);
	return vysledok;
}

void* memory_init(void *ptr,unsigned int size)
{
	// popísané v dokumentácii, v skratke vytvorím na zaèiatku špec znak, o 1B dalej uložím pôvodnú ve¾kos array-u, o 4B prvý vo¾ný blok
	int pom = (int)size;
	pointer = ptr;
	*(char *)pointer = '$';
	pointer = (char *)pointer + sizeof(char);
	
	*(int *)pointer = size - 3*sizeof(int) - sizeof(char);
	pointer = (char *)pointer + sizeof(int);
	
	*(int *)pointer = size - 3*sizeof(int) - sizeof(char);
	pointer = (char *)pointer + sizeof(int);
	pointer = (char *)pointer + size - 3*sizeof(int) - sizeof(char);
	*(int *)pointer = size - 3*sizeof(int) - sizeof(char);
	
	pointer = (char *)pointer - *(int *)pointer - sizeof(int);
}

int main(int argc, char *argv[]) {
	char region[30000];
	memory_init(region,30000);
	int amount = 0;

	void *ptr1 = NULL;
	void *ptr2 = NULL;
	ptr1 = memory_alloc(7000);
	ptr2 = memory_alloc(8000);
	memory_free(ptr1);
	memory_free(ptr2);

	printf("Hodnota prveho blocku %d\n",*(int*)(pointer));
	printf("Hodnota poslednej hlavicky %d\n",*(int*)(pointer+30000-4-5));

	return 0;
}
