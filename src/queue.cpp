/* queue.cpp - implementation of the game studio challenge
 * Copyright (C) 2012  Paulo Pinto
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

// Maximum available storage size
const int MAX_STORAGE = 2048;


// Bits to shift around
const int BIT_SHIFT = 12;

// The set of bits that contain the value of the next pointer
const int LOWER_BITS = 0x0FFF;

// storage area
unsigned char data[MAX_STORAGE];

// The queue data type
typedef void Q;

Q * create_queue(); //Creates a FIFO byte queue, returning a handle to it.
void destroy_queue(Q * q); //Destroy an earlier created byte queue.
void enqueue_byte(Q * q, unsigned char b); //Adds a new byte to a queue.
unsigned char dequeue_byte(Q * q); //Pops the next byte off the FIFO queue.
void initialize_storage();
void on_out_of_memory();
void on_illegal_operation();


/**
 * Helper function. Dumps the queue contents in a sequence
 * of (value, next) elements.
 *
 * @param q The queue to dump. Cannot be null.
 */
void dump_queue (Q *q)
{
    assert(q != 0);
    printf ("Printing Queue Debugging Info\n");
    int *queue = reinterpret_cast<int*>(q);
    if (*queue == 0) {
        printf ("Empty queue!\n");
    }
    int next = *queue & LOWER_BITS;
    printf ("(%d, %d) ", *queue >> BIT_SHIFT, next);
    while (next != 0 && next != LOWER_BITS) {
        queue = reinterpret_cast<int*>(data + next);
        next = *queue & LOWER_BITS;
        printf ("(%d, %d) ", *queue >> BIT_SHIFT, next);

    }
    printf("\n");
}

int main(void) {
    initialize_storage();
    Q * q0 = create_queue();
    enqueue_byte(q0, 0);
    enqueue_byte(q0, 1);
    Q * q1 = create_queue();
    enqueue_byte(q1, 3);
    enqueue_byte(q0, 2);
    enqueue_byte(q1, 4);
    printf("%d ", dequeue_byte(q0));
    printf("%d\n", dequeue_byte(q0));
    enqueue_byte(q0, 5);
    enqueue_byte(q1, 6);
    printf("%d ", dequeue_byte(q0));
    printf("%d\n", dequeue_byte(q0));
    destroy_queue(q0);
    printf("%d ", dequeue_byte(q1));
    printf("%d ", dequeue_byte(q1));
    printf("%d\n", dequeue_byte(q1));
    destroy_queue(q1);
}

void on_out_of_memory() {
    printf("Not enough memory available. Exiting application\n");
    exit(-1);
}

void on_illegal_operation() {
    printf("An invalid queue operation has been requested. Exiting application\n");
    exit(-2);
}

void initialize_storage() {
    memset(data, 0, MAX_STORAGE);
    int* free_list = reinterpret_cast<int*>(data);
    *free_list = sizeof(int); // Make the free list point for the first free element
}

void* allocate_storage() {
    int* free_list = reinterpret_cast<int*>(data);
    if (*free_list == 0) {
        on_out_of_memory(); // cannot fullfil request
    }

    int *cell = reinterpret_cast<int*>(data + *free_list);
    if (*cell == 0) {
        *free_list += sizeof(int);
    } else {
        *free_list = *cell;
        *cell = 0;
    }

    return cell;
}

void release_storage(void* mem) {
    int* free_list = reinterpret_cast<int*>(data);
    int* cell = reinterpret_cast<int*>(mem);
    *cell = *free_list;
    *free_list = reinterpret_cast<unsigned char*>(mem) - data;
}

Q * create_queue() {
    int *queue = reinterpret_cast<int*>(allocate_storage());
    *queue = 0;
    return queue;
}

void destroy_queue(Q * q) {
    assert(q != 0);
    int *queue = reinterpret_cast<int*>(q);
    int next = *queue & LOWER_BITS;
    while (next != 0) {
        unsigned char *mem = data + next;
        next = *(reinterpret_cast<int*>(mem)) & LOWER_BITS;
        release_storage(mem);
    }

    release_storage(q);
}

void enqueue_byte(Q * q, unsigned char b) {
    assert(q != 0);
    int *queue = reinterpret_cast<int*>(q);
    if (*queue == 0) {
        *queue = b << BIT_SHIFT | LOWER_BITS;
    } else {
        int next = *queue & LOWER_BITS;
        int previous = 0;
        if (next == LOWER_BITS) {
            previous = 0;
        } else {
            while (next != 0) {
                unsigned char *mem = data + next;
                previous = next;
                next = *(reinterpret_cast<int*>(mem)) & LOWER_BITS;
            }
        }
        int *cell = reinterpret_cast<int*>(allocate_storage());
        *cell = b << BIT_SHIFT;
        int *last_cell;
        if (previous == 0) {
            last_cell = queue;
        } else {
            last_cell = reinterpret_cast<int*>(data + previous);
        }
        *last_cell = (*last_cell & 0xF000) | (reinterpret_cast<unsigned char*>(cell) - data);
    }
}

unsigned char dequeue_byte(Q * q) {
    assert(q != 0);
    int *queue = reinterpret_cast<int*>(q);
    if (*queue == 0) {
        on_illegal_operation();
    }
    unsigned char value = *queue >> BIT_SHIFT;

    int next = *queue & LOWER_BITS;
    if (next != 0 && next != LOWER_BITS) {
        int *cell = reinterpret_cast<int*>(data + next);
        *queue = *cell;
        release_storage(cell);
    } else {
        *queue = 0;
    }

    return value;
}
