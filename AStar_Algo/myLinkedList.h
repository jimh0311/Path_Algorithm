
/*
����
��ũ�� ����Ʈ�� ����ϴ�.
���ͷ����ʹ� ��� �ø��� �����Ͽ� ��ũ�� ����Ʈ�� �ּҰ��� �����Ͽ� �����մϴ�.
�ʱ� ���ð��� ��ũ�� ����Ʈ�� ���;
���ͷ����͸� ����ϱ� �� GotoHead()�� �������ָ� �����մϴ�.
GetValue �� ���, ���� ����� ���� �������� false �̹Ƿ� if���� �ɾ� �������θ� Ȯ���ϴ� ���� �����ϴ�.

GotoNext() �� Tail�� �����ϸ� false�� ���Ƿ� ������ ���� ����ϼ���.

** �����͸� �̰� �ش� ��带 ��� �����ϴ� ���ù� **

myIterator<int> iter(myList);
int	data;
while (iter.GotoNext())
{
if (iter.GetValue(&data))
{
printf("%d\n", data);
}
iter.DeleteThisNode();
}

�����Ҵ��� ��ũ�� ����Ʈ�� ������ �� ���� ���鸸 �����ְ� �ֽ��ϴ�.
��� �ȿ� �����Ҵ�� �����ʹ� ����ڰ� ������ ������߸� �մϴ�.

~myLinkedList()
{
while(DeleteHeadNode()) // pHead->pNext = pTail �� �� ������ ��� �� ��带 ����
}
*/
template <typename T>
struct Node
{
	Node* pPrev;
	Node* pNext;
	T value;
};

template <typename T>
class myLinkedList
{
private:
	Node<T> *pHead;
	Node<T> *pTail;
public:
	myLinkedList();
	~myLinkedList();
	Node<T>* GetInstance();
	bool InsertHead(T value);
	bool InsertTail(T value);
	bool DeleteTailNode(void);
	bool DeleteHeadNode(void);
	//bool InsertHead_Heap(T value);
};

template <typename T>
myLinkedList<T>::myLinkedList()
{
	//-----------------------------
	//	�ʱ�ȭ
	//-----------------------------
	pHead = new Node<T>;
	pTail = new Node<T>;
	pHead->pPrev = nullptr;
	pHead->pNext = pTail;
	pTail->pPrev = pHead;
	pTail->pNext = nullptr;
}
template <typename T>
myLinkedList<T>::~myLinkedList()
{
	while (DeleteHeadNode());
	delete pHead;
	delete pTail;
}
template <typename T>
Node<T>* myLinkedList<T>::GetInstance()
{
	return pHead;
}
template <typename T>
bool myLinkedList<T>::InsertHead(T value)
{
	Node<T> *ret = new Node<T>;
	ret->value = value;

	pHead->pNext->pPrev = ret;
	ret->pNext = pHead->pNext;
	ret->pPrev = pHead;
	pHead->pNext = ret;

	return true;
}

template <typename T>
bool myLinkedList<T>::InsertTail(T value)
{
	Node<T> *ret2 = new Node<T>;
	ret2->value = value;

	ret2->pPrev = pTail->pPrev;
	pTail->pPrev->pNext = ret2;
	ret2->pNext = pTail;
	pTail->pPrev = ret2;

	return true;
};

template <typename T>
bool myLinkedList<T>::DeleteTailNode(void)
{

	Node<T>* delNode = pTail->pPrev;
	if (delNode == pHead)
	{
		return false;
	}
	pTail->pPrev = delNode->pPrev;
	delNode->pPrev->pNext = pTail;
	delete delNode;
	return true;
};

template <typename T>
bool myLinkedList<T>::DeleteHeadNode(void)
{
	Node<T> *delNode = pHead->pNext;
	if (delNode == pTail)
	{
		return false;
	}
	pHead->pNext = delNode->pNext;
	delNode->pNext->pPrev = pHead;
	delete delNode;
	return true;
};

//------------------------------------
//	Iterator �����;
//------------------------------------

template <typename T>
class myIterator
{
private:
	Node<T> *pHead;
	Node<T> *pPosition;
public:
	myIterator(myLinkedList<T> *myList);
	bool GotoHead();
	bool GotoNext();
	bool GotoPrev();
	bool GetValue(T* temp);
	bool DeleteThisNode();
	bool SearchNode(T data);
};
template <typename T>
bool myIterator<T>::SearchNode(T data)
{
	Node<T> *pNow = pHead->pNext;
	while (1)
	{
		if (pNow->pNext == nullptr)
			break;

		if (memcmp(data, &pNow->value, sizeof(DWORD)*2)==0)
			return true;
	}
	return false;
}

template <typename T>
myIterator<T>::myIterator(myLinkedList<T> *myList)
{
	pPosition = myList->GetInstance();
	pHead = myList->GetInstance();
}
template <typename T>
bool myIterator<T>::GotoHead()
{
	if (pHead != nullptr)
	{
		pPosition = pHead;
		return true;
	}
	else
	{
		return false;
	}
}
template <typename T>
bool myIterator<T>::GotoNext()
{
	if (pPosition->pNext == nullptr)
	{
		return false;
	}
	else
	{
		pPosition = pPosition->pNext;
		return true;
	}
}

template <typename T>
bool myIterator<T>::GotoPrev()
{
	if (pPosition->pPrev == nullptr)
	{
		return false;
	}
	else
	{
		pPosition = pPosition->pPrev;
		return true;
	}
}

template <typename T>
bool myIterator<T>::GetValue(T *temp)
{
	if (pPosition->pPrev == nullptr || pPosition->pNext == nullptr)
	{
		return false;
	}
	else
	{
		*temp = pPosition->value;
		return true;
	}
}

template <typename T>
bool myIterator<T>::DeleteThisNode()
{
	if (pPosition->pPrev == nullptr || pPosition->pNext == nullptr)
	{
		return false;
	}
	else
	{
		Node<T> *delNode;
		Node<T> *tempPrev = pPosition->pPrev;
		Node<T> *tempNext = pPosition->pNext;
		delNode = pPosition;
		pPosition->pPrev->pNext = pPosition->pNext;
		pPosition->pNext->pPrev = pPosition->pPrev;
		delete delNode;
		pPosition->pPrev = tempPrev;
		pPosition->pNext = tempNext;
	}
}