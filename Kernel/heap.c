#include <heap.h>
#include <string.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <video_driver.h>
#include <converter.h>
#include <lib.h>

extern ProcessSlot * tableProcess;
kernelHeapHeader kernelHeader;

void initializeKernelHeap(){
	int i;

	kernelHeader.nextPage = 0;
	for(i = 0; i < PAGES_KERNEL_HEAP; i++){
		kernelHeapPage heapPageK;
		heapPageK.occupiedBytes = 0;
		heapPageK.freeBytes = PAGE_SIZE;
		heapPageK.pageAddress = allocPage();
		kernelHeader.kernelHeapPages[i] = heapPageK;
	}
}

void printKernelHeap(){
	int i;
	for(i = 0; i < PAGES_KERNEL_HEAP; i++){
		print_string("Pagina ");
		print_int(i);
		print_string(" del heap kernel: ");
		printHex((qword) kernelHeader.kernelHeapPages[i].pageAddress);
		nextLine();
	}
}

p_heapPage createHeapPage(){
	heapPage newHeap;
	p_heapPage newHeapPointer;
	newHeap.currentPage = allocPage();
	newHeap.occupiedBytes = 0;
	newHeap.nextHeapPage = NULL;
	newHeap.freeBytes = PAGE_SIZE;
	newHeapPointer = &newHeap;
	int sizeNewHeapStruct = sizeof(heapPage);
	void * kernelHeapPage = findAvaiableHeapKernelPage(sizeNewHeapStruct);
	newHeapPointer = memcpy(kernelHeapPage, newHeapPointer, sizeNewHeapStruct);
	return newHeapPointer;
}

void * findAvaiableHeapKernelPage(int size){
	int nextPage = kernelHeader.nextPage;
	kernelHeapPage heapPageK = kernelHeader.kernelHeapPages[nextPage];
	if(heapPageK.freeBytes >= size){
		return (heapPageK.pageAddress) + (heapPageK.occupiedBytes);
	}else{
		heapPageK = kernelHeader.kernelHeapPages[nextPage + 1];
		kernelHeader.nextPage++;
		return heapPageK.pageAddress;
	}
}

void * malloc_heap(int size, char * processName){
	Process currentProcess = searchRunningProcess();
	p_heapPage heap = findAvaiableHeapPage(currentProcess.heap, size);
	void * freePointer = findFreePointer(heap);
	heap->freeBytes -= size;
	heap->occupiedBytes += size;
	//printHex((qword)heap->currentPage);
	//nextLine();
	//print_string("El inicio del heap del proceso guardado en kernel es:");
	//printHex((qword)tableProcess[numberProcess].heap);
	//nextLine();
	//print_string("El inicio del heap del proceso es:");
	//printHex((qword)tableProcess[numberProcess].heap->currentPage);
	//nextLine();
	print_string("El puntero que reservo el usuario esta en la posicion:");
	printHex((qword)freePointer);
	nextLine();
	return freePointer;

}	

Process searchRunningProcess(){
	ProcessSlot * aux = tableProcess;

	while(aux != NULL && aux->process.status != RUNNING){
		aux = aux->next;
	}

	return aux->process;
}

p_heapPage findAvaiableHeapPage(p_heapPage firstPage, int size){
	if(firstPage == NULL){
		firstPage = createHeapPage();
		Process currentProcess = searchRunningProcess();
		currentProcess.heap = firstPage;
		return firstPage;
	}
	if(firstPage->freeBytes >= size)
		return firstPage;
	
	p_heapPage currentHeapPage = firstPage->nextHeapPage;
	p_heapPage previousPage = firstPage;

	while(currentHeapPage != NULL && currentHeapPage->freeBytes < size){
		previousPage = currentHeapPage;
		currentHeapPage = currentHeapPage->nextHeapPage;
	}

	if(currentHeapPage == NULL)
		previousPage->nextHeapPage = createHeapPage();
	else
		return currentHeapPage;
}

void * findFreePointer(p_heapPage heapPage){
	int occupied = heapPage->occupiedBytes;
	void * startingAddress = heapPage->currentPage;
	return startingAddress + occupied;
}