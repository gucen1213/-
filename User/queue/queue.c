#include "system.h"
#include "./app_cfg.h"

#define this (*ptThis)

typedef volatile struct{
    uint8_t  *pchBuffer;
    uint16_t hwSize;
    uint16_t hwHead;
    uint16_t hwTail;
    uint16_t hwLength;
	uint16_t hwPeek;
	uint16_t hwPeekLength;
}byte_queue_t;


bool init_byte_queue(byte_queue_t *ptThis,uint8_t *pchFIFOBuff ,uint16_t hwSize)
{
    if((NULL == ptThis)||(NULL == pchFIFOBuff)||(0 == hwSize)){
        return false;
    }
    this.hwHead = 0;
    this.hwTail = 0;
    this.hwLength = 0;
    this.hwSize = hwSize;
    this.pchBuffer = pchFIFOBuff;
	this.hwPeek = 0;
	this.hwPeekLength = 0;
    return true;
}

bool is_byte_queue_empty(byte_queue_t *ptThis)
{
	if(NULL == ptThis){
		return true;
	}
    if((this.hwHead == this.hwTail)&&(0 == this.hwLength)){
        return true;
    }
    return false;
}
//��Ӳ���
bool enqueue_byte(byte_queue_t *ptThis,uint8_t chByte)
{
    if(NULL == ptThis){
        return false;
    }
    //������
    if((this.hwHead == this.hwTail)&&(0 !=this.hwLength)){
        return false;
    }
    this.pchBuffer[this.hwTail] = chByte;
    this.hwLength++;
    this.hwTail++;
	this.hwPeekLength++;
    if(this.hwSize <= this.hwTail){
        this.hwTail = 0;
    }
    return true;
    
}
//���Ӳ���
bool dequeue_byte(byte_queue_t *ptThis,uint8_t *pchByte)
{
    if((NULL == ptThis) || (NULL == pchByte)){
        return false;
    }
    //����Ϊ��
    if((this.hwHead == this.hwTail) && (0 == this.hwLength)){
        return false;
    }else{
        *pchByte = this.pchBuffer[this.hwHead];
        this.hwLength--;
        this.hwHead++;
        if(this.hwSize <= this.hwHead){
            this.hwHead = 0;
        }
		this.hwPeek = this.hwHead;    //��λpeekָ��
		this.hwPeekLength = this.hwLength;  //��λpeek����
        return true;
    }
}


//��λpeekָ���ͷ��ʼ
bool reset_peek_byte(byte_queue_t *ptThis)
{
	if(NULL == ptThis){
		return false;
	}
	this.hwPeekLength = this.hwLength;
	this.hwPeek = this.hwHead;
	return true;
}

//�Ӷ�����peekһ������
bool peek_byte_queue(byte_queue_t *ptThis,uint8_t *pchByte)
{
	if((NULL == ptThis) ||(NULL == pchByte)){
		return false;
	}
	if(0 == this.hwPeekLength){
		return false;
	}
	*pchByte = this.pchBuffer[this.hwPeek];
	this.hwPeek++;
	if(this.hwPeek >= this.hwSize){
		this.hwPeek = 0;
	}
	this.hwPeekLength--;
	return true;
}


//������peek�������ݣ��Ӷ�����ɾ��
bool get_all_peeked_byte(byte_queue_t *ptThis)
{
	if(NULL == ptThis){
		return false;
	}
	this.hwHead = this.hwPeek;
	this.hwLength = this.hwPeekLength;
	return true;
}





