/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2014 Yanzheng Li

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#include <algorithm>
#include <set>
#include <sneaker/testing/_unittest.h>
#include "../../include/dyobj/dynamic_object_container.h"
#include "../../include/dyobj/errors.h"


class dynamic_object_container_unittest : public ::testing::Test {
public:
  typedef struct dummy {
    int data;
  } T;

protected:
  virtual void TearDown() {
    // Make sure test cases clean up the container properly.
    ASSERT_EQ(m_container.end(), m_container.begin());
  }

  corevm::dyobj::dynamic_object_container<dummy> m_container;
};


TEST_F(dynamic_object_container_unittest, TestCreateAndUpdate)
{
  int data = 888;

  T* p = m_container.create();
  ASSERT_NE(nullptr, p);

  p->data = data;

  T* t = m_container[p];

  ASSERT_EQ(data, t->data);

  m_container.destroy(p);

  ASSERT_THROW(
    {
      m_container[p];
    },
    corevm::dyobj::invalid_address_error
  );
}

TEST_F(dynamic_object_container_unittest, TestIterator)
{
  int data1 = 666;
  int data2 = 777;
  int data3 = 888;

  T* p1 = m_container.create();
  ASSERT_NE(nullptr, p1);

  T* p2 = m_container.create();
  ASSERT_NE(nullptr, p2);

  T* p3 = m_container.create();
  ASSERT_NE(nullptr, p3);

  p1->data = data1;
  p2->data = data2;
  p3->data = data3;

  std::set<int> set;

  for(auto itr = m_container.begin(); itr != m_container.end(); ++itr) {
    T t = *itr;
    set.insert(t.data);
  }

  ASSERT_EQ(3, set.size());

  ASSERT_NE(set.end(), set.find(data1));
  ASSERT_NE(set.end(), set.find(data2));
  ASSERT_NE(set.end(), set.find(data3));

  m_container.destroy(p1);
  m_container.destroy(p2);
  m_container.destroy(p3);
}

TEST_F(dynamic_object_container_unittest, TestConstIterator)
{
  int data1 = 666;
  int data2 = 777;
  int data3 = 888;

  T* p1 = m_container.create();
  ASSERT_NE(nullptr, p1);

  T* p2 = m_container.create();
  ASSERT_NE(nullptr, p2);

  T* p3 = m_container.create();
  ASSERT_NE(nullptr, p3);

  p1->data = data1;
  p2->data = data2;
  p3->data = data3;

  std::set<int> set;

  for(auto itr = m_container.cbegin(); itr != m_container.cend(); ++itr) {
    T t = *itr;
    set.insert(t.data);
  }

  ASSERT_EQ(3, set.size());

  ASSERT_NE(set.end(), set.find(data1));
  ASSERT_NE(set.end(), set.find(data2));
  ASSERT_NE(set.end(), set.find(data3));

  m_container.destroy(p1);
  m_container.destroy(p2);
  m_container.destroy(p3);
}

TEST_F(dynamic_object_container_unittest, TestIteratorWithStdForEach)
{
  int data1 = 666;
  int data2 = 777;
  int data3 = 888;

  T* p1 = m_container.create();
  ASSERT_NE(nullptr, p1);

  T* p2 = m_container.create();
  ASSERT_NE(nullptr, p2);

  T* p3 = m_container.create();
  ASSERT_NE(nullptr, p3);

  p1->data = data1;
  p2->data = data2;
  p3->data = data3;

  std::set<int> set;

  std::for_each(
    m_container.begin(),
    m_container.end(),
    [&set](T& t) {
      set.insert(t.data);
    }
  );

  ASSERT_EQ(3, set.size());

  ASSERT_NE(set.end(), set.find(data1));
  ASSERT_NE(set.end(), set.find(data2));
  ASSERT_NE(set.end(), set.find(data3));

  m_container.destroy(p1);
  m_container.destroy(p2);
  m_container.destroy(p3);
}

TEST_F(dynamic_object_container_unittest, TestConstIteratorWithStdForEach)
{
  int data1 = 666;
  int data2 = 777;
  int data3 = 888;

  T* p1 = m_container.create();
  ASSERT_NE(nullptr, p1);

  T* p2 = m_container.create();
  ASSERT_NE(nullptr, p2);

  T* p3 = m_container.create();
  ASSERT_NE(nullptr, p3);

  p1->data = data1;
  p2->data = data2;
  p3->data = data3;

  std::set<int> set;

  std::for_each(
    m_container.cbegin(),
    m_container.cend(),
    [&set](const T& t) {
      set.insert(t.data);
    }
  );

  ASSERT_EQ(3, set.size());

  ASSERT_NE(set.end(), set.find(data1));
  ASSERT_NE(set.end(), set.find(data2));
  ASSERT_NE(set.end(), set.find(data3));

  m_container.destroy(p1);
  m_container.destroy(p2);
  m_container.destroy(p3);
}

TEST_F(dynamic_object_container_unittest, TestIteratorReflectsChange)
{
  T* p = m_container.create();
  ASSERT_NE(nullptr, p);

  int data = 1;
  p->data = data;

  auto itr = m_container.begin();

  ASSERT_NE(m_container.end(), itr);

  ASSERT_EQ(data, (*itr).data);

  int data2 = 2;
  p->data = data2;

  itr = m_container.begin();

  ASSERT_NE(m_container.end(), itr);

  ASSERT_EQ(data2, (*itr).data);

  m_container.destroy(p);
}

TEST_F(dynamic_object_container_unittest, TestPostIncrement)
{
  int data1 = 666;
  int data2 = 777;
  int data3 = 888;

  T* p1 = m_container.create();
  ASSERT_NE(nullptr, p1);
  p1->data = data1;

  T* p2 = m_container.create();
  ASSERT_NE(nullptr, p2);
  p2->data = data2;

  T* p3 = m_container.create();
  ASSERT_NE(nullptr, p3);
  p3->data = data3;

  auto itr = m_container.begin();
  auto itr1 = itr++;
  auto itr2 = itr++;
  auto itr3 = itr++;

  ASSERT_NE(m_container.end(), itr3);

  ASSERT_EQ(data1, itr1->data);
  ASSERT_EQ(data2, itr2->data);
  ASSERT_EQ(data3, itr3->data);

  m_container.destroy(p1);
  m_container.destroy(p2);
  m_container.destroy(p3);
}

TEST_F(dynamic_object_container_unittest, TestIteratorReflectsChange2)
{
  T* p = m_container.create();
  ASSERT_NE(nullptr, p);

  int data = 1;
  p->data = data;

  T* p2 = m_container.create();
  ASSERT_NE(nullptr, p2);

  auto itr = m_container.begin();
  ASSERT_NE(m_container.end(), itr);

  auto itr2 = itr;
  ASSERT_NE(m_container.end(), ++itr2);

  m_container.destroy(p2);

  ASSERT_EQ(m_container.end(), ++itr);

  m_container.destroy(p);
}

TEST_F(dynamic_object_container_unittest, TestErase)
{
  int data1 = 666;
  int data2 = 777;
  int data3 = 888;

  T* p1 = m_container.create();
  ASSERT_NE(nullptr, p1);
  p1->data = data1;

  T* p2 = m_container.create();
  ASSERT_NE(nullptr, p2);
  p2->data = data2;

  T* p3 = m_container.create();
  ASSERT_NE(nullptr, p3);
  p3->data = data3;

  ASSERT_EQ(3, m_container.size());

  auto itr = m_container.begin();

  m_container.erase(itr);

  ASSERT_EQ(2, m_container.size());

  itr = m_container.begin();

  ASSERT_EQ(data2, (*itr).data);

  m_container.erase(++itr);

  ASSERT_EQ(1, m_container.size());

  itr = m_container.begin();

  ASSERT_EQ(data2, (*itr).data);

  m_container.erase(itr);

  ASSERT_EQ(0, m_container.size());
}