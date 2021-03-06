/*
The MIT License (MIT)

Copyright (c) 2016 Cynara Krewe

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software, hardware and associated documentation files (the "Solution"), to deal
in the Solution without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Solution, and to permit persons to whom the Solution is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Solution.

THE SOLUTION IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOLUTION OR THE USE OR OTHER DEALINGS IN THE
SOLUTION.
 */

#include <stdint.h>

#include "CppUTest/TestHarness.h"

#include "driverlib/debug.h"

#include "flow/queue.h"

#include "data.h"

using Flow::Queue;

const static unsigned int UNITS = 3;
const static unsigned int QUEUE_SIZE[UNITS] = {1, 10, 255};

TEST_GROUP(Queue_TestBench)
{
	Queue<Data>* unitUnderTest[UNITS];

	void setup()
	{
		for(unsigned int i = 0; i < UNITS; i++)
		{
			unitUnderTest[i] = new Queue<Data>(QUEUE_SIZE[i]);
		}
	}

	void teardown()
	{
		for(unsigned int i = 0; i < UNITS; i++)
		{
			delete unitUnderTest[i];
		}
	}
};

TEST(Queue_TestBench, IsEmptyAfterCreation)
{
	for(unsigned int i = 0; i < UNITS; i++)
	{
		CHECK(unitUnderTest[i]->isEmpty());
		Data response;
		CHECK(!unitUnderTest[i]->dequeue(response));
	}
}

TEST(Queue_TestBench, EnqueueDequeueItem)
{
	for(unsigned int i = 0; i < UNITS; i++)
	{
		CHECK(unitUnderTest[i]->isEmpty());
		Data stimulus = Data(123, true);
		CHECK(unitUnderTest[i]->enqueue(stimulus));
		CHECK(!unitUnderTest[i]->isEmpty());
		Data response;
		CHECK(unitUnderTest[i]->dequeue(response));
		CHECK(stimulus == response);
		CHECK(unitUnderTest[i]->isEmpty());
		CHECK(!unitUnderTest[i]->dequeue(response));
	}
}

TEST(Queue_TestBench, FullQueue)
{
	for(unsigned int i = 0; i < UNITS; i++)
	{
		// Queue should be empty.
		CHECK(unitUnderTest[i]->isEmpty());

		for(unsigned int c = 0; c < (QUEUE_SIZE[i] - 1); c++)
		{
			Data stimulus = Data(c, true);
			// Queue should accept another item.
			CHECK(unitUnderTest[i]->enqueue(stimulus));

			// Queue should not be empty.
			CHECK(!unitUnderTest[i]->isEmpty());

			// Queue shouldn't be full.
			CHECK(!unitUnderTest[i]->isFull());
		}

		Data lastStimulus = Data(QUEUE_SIZE[i], false);
		// Queue should accept another item.
		CHECK(unitUnderTest[i]->enqueue(lastStimulus));

		// Queue should not be empty.
		CHECK(!unitUnderTest[i]->isEmpty());

		// Queue should be full.
		CHECK(unitUnderTest[i]->isFull());

		// Queue shouldn't accept any more items.
		CHECK(!unitUnderTest[i]->enqueue(lastStimulus));

		Data response;

		for(unsigned int c = 0; c < (QUEUE_SIZE[i] - 1); c++)
		{
			// Should get another item from the Queue.
			CHECK(unitUnderTest[i]->dequeue(response));

			// Item should be the expected.
			Data expectedResponse = Data(c, true);
			CHECK(response == expectedResponse);

			// Queue should not be empty.
			CHECK(!unitUnderTest[i]->isEmpty());

			// Queue shouldn't be full.
			CHECK(!unitUnderTest[i]->isFull());
		}

		// Should get another item from the Queue.
		CHECK(unitUnderTest[i]->dequeue(response));

		// Item should be the expected.
		CHECK(lastStimulus == response);

		// Queue shouldn't be full.
		CHECK(!unitUnderTest[i]->isFull());

		// Queue should be empty.
		CHECK(unitUnderTest[i]->isEmpty());

		// Shouldn't get another item from the Queue.
		CHECK(!unitUnderTest[i]->dequeue(response));
	}
}

static const unsigned long long _count = 1000;
static unsigned long long _c = 0;
static Queue<Data>* _unitUnderTest = NULL;
extern void (*calledFromSysTickHandler)(void);

static void fromInterruptContext(void)
{
	ASSERT(_unitUnderTest != NULL);

	if(_c < _count)
	{
		if(_unitUnderTest->enqueue(Data(_c, ((_c % 2) == 0))))
		{
			_c++;
		}
	}
}

TEST(Queue_TestBench, Threadsafe)
{
	for(unsigned int i = 0; i < UNITS; i++)
	{
		// Queue should be empty.
		CHECK(unitUnderTest[i]->isEmpty());

		// Install unit under test.
		_unitUnderTest = unitUnderTest[i];
		_c = 0;

		// Install interrupt context functionality.
		calledFromSysTickHandler = &fromInterruptContext;

		unsigned long long c = 0;
		bool success = true;
		while(c < _count)
		{
			Data response;
			if(unitUnderTest[i]->dequeue(response))
			{
				Data expectedResponse = Data(c, ((c % 2) == 0));
				success = success && (response == expectedResponse);
				c++;
			}
		}

		CHECK(success);

		// Uninstall interrupt context functionality.
		calledFromSysTickHandler = NULL;

		// Uninstall unit under test.
		_unitUnderTest = NULL;

		// Queue should be empty.
		CHECK(unitUnderTest[i]->isEmpty());
	}
}
