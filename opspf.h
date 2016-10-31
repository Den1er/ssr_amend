#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <malloc.h>
//***************
#include <math.h>
#include <fstream>
#include <limits>
//***************
#include "api.h"
using namespace std;

//================================================================================================================================================================================
#define 	DATA						69168//48*(24*60+1)
#define 	PI 							3.1415926
#define 	N 							10000
#define 	MAX 						0XFFFF
#define 	ANY_ADDRESS     			0xffffffff
//#define	IPPROTO_UDP	17
#define	MAX_CLOCK_STRING_LENGTH	24

typedef long long clocktype;
typedef unsigned int NodeAddress;
typedef unsigned short RandomSeed[3];

const int TotallNode = 48;
const double OpspfBeita = 70;
//================================================================================================================================================================================
enum {
	MSG_NETWORK_OPSPF_UPDATEROUTINGTABLE = 672,
	MSG_NETWORK_OPSPF_UPDATESATABLE = 673,
	MSG_NETWORK_OPSPF_BROADCAST = 675,
	MSG_NETWORK_OPSPF_RECEIVEREGISTER = 676,
};

struct Coord {
	double x;
	double y;
};

struct IpHeaderType
{
	int ip_v_hl_tos_len;//ip_v:4,        /* version */
						//ip_hl:4;       /* header length */
						//unsigned char ip_tos;       /* type of service */
						//UInt16 ip_len;      /* total length */

	short ip_id;
	short ipFragment;
	//ip_reserved:1,
	//ip_dont_fragment:1,
	//ip_more_fragments:1,
	//ip_fragment_offset:13;

	unsigned char  ip_ttl;         /* time to live */
	unsigned char  ip_p;           /* protocol */
	unsigned short ip_sum;         /* checksum */
	unsigned       ip_src, ip_dst;  /* source and dest address */

};

struct registerRow {
	NodeAddress usrip;
	bool isRelay;
	int RelaySTId;
	registerRow* next;
};

struct registerInfo {
	NodeAddress usrip;
	bool isRelay;
	int RelaySTId;
};

struct RegisterPacket {
	bool isRelay;
	int RelaySTId;
	//bool isFirst;
};

struct Lon {
	double reallon;
	double calculatelon;
};

struct STCoordinates {
	Lon lon;//����
	double lat;//γ��
	double phi;//ӳ���
};

struct Neighbor
{
	STCoordinates ST_leftneighbor;
	STCoordinates ST_rightneighbor;
};

struct ST {
	int orbitnum;//�����
	int index;//���
	STCoordinates STC;
	Neighbor neighbor;
};


struct LSA
{
	int SourceSatellite_ID;
	int SourceSatellite_interface;
	int DestinationSatellite_ID;
	int DestinationSatellite_interface;
	int Metric;
	LSA* NextLSA;
};


struct OpspfVertex
{
	NodeAddress             vertexId;
	unsigned int            distance;
	OpspfVertex*				NextVertex; // �����

};

struct OpspfShortestVertex
{
	NodeAddress     DestinationVertex;//vertexId;//��ǰ�ڵ�ID	
	NodeAddress		AdvertisingVertext;//ForwadVertexId;//ǰ���ڵ�ID
	unsigned int	distance;//��ǰ�ڵ����Դ�ڵ����
	OpspfShortestVertex*  NextShortestVertex;
};

struct LSDB
{
	//	int I = 0;
	LSA* LSAList;
};

//���ǵĹ�����ŵ����ֱ�ŵ�ӳ��
struct SatelitteNumMap
{
	string SatelliteIndex;
	int Num;
};

struct registerList {
	registerRow* head;
	int size;
};

struct OpspfData {
	//�������
	RandomSeed seed;

	//������Ϣ���ͼ��
	clocktype UpdateInterval;
	clocktype UpdateSAInterval;

	//���м̹�ʱ
	clocktype LastNoRelay;

	//ST�ṹ
	ST st[48];
	ST oldst[48];

	//LSDB
	LSDB* lsdb;
    //
	//���·����
	OpspfVertex GraphLinkList[TotallNode];
	OpspfShortestVertex* opspfShortestVertex;

	//ӡ���
	NodeAddress idtoaddress[48][6];
	NodeAddress linktable[48][6];
	NodeAddress linknode[128];
	NodeAddress linkid[128];
    
    NodeAddress LinkTable_Forward[TotallNode][4];
    NodeAddress LinkTable_Reverse[TotallNode][4];
    //ssr_added
    //�Ƿ���
    bool Emissioned[TotallNode];

    //���ǽڵ�����
    NodeAddress Link_Index[TotallNode][TotallNode];

	//ע���ñ�
	double s_a[48];
	registerList* rList;

	//�жϵ�һ��
	bool isfirst;
    //�жϷ���ߵĵ�һ��
    bool isfirst_allocate;
    //ssr_added;
};

//================================================================================================================================================================================
//��ʼ������
void OpspfInit(
	Node* node,
	int interfaceIndex);

//�¼�������
void OpspfHandleProtocolEvent(
	Node* node,
	Message* msg);

//����·�ɱ��ܺ���
void UpdateOpspfRoutingTable(
	Node* node,
	Message* msg,
	int i);

//����S_A  ӳ�����
void UpdateSATable(
	Node* node,
	int j);

//�õ�����γ��
ST GetST(
	Node* node);

//�õ�����γ��
void GetSTLat(
	ST* st,
	ST currentst);

//
void GetSTLatByFile(
	Node* node,
	int j);

//�������Ⱥ���
double CalculateLon(
	double lat,
	double Gamma);

//��γ��ӳ�䵽phi
double CoordinatesToPhi(
	STCoordinates coordinates);

//phiӳ��ؾ�γ��
STCoordinates PhiToCoordinates(
	double Phi,
	double oldCalculatelon,
	double oldReallon);


void freeLSAList(
	LSA* LSAListhead);

void AllocateIndex();

//��LSDB�и���SourceSatelliteIDea��ȡLSA
LSA* OpspfLookupLSAList(
	LSDB* lsdb,
	string SatelliteIndex);

//����SLA��Ϣ��ʼ��ͼ���ڽӱ��ʾ
void ReadLSAToGraph(
	LSDB* lsdb,
	OpspfVertex GraphLinkList[TotallNode]);

//��������
void FreeList(
	OpspfShortestVertex** List);

//��LSA
OpspfShortestVertex* SearchVertex(
	OpspfShortestVertex* Path,
	NodeAddress VertexId);

//Ϊÿ���ڵ����Dijkstra�㷨���������·������
//���pathΪ���·��ǰ���ڵ��š�
void Dijstra(
	OpspfVertex GraphLinkList[TotallNode],
	NodeAddress Start,
	OpspfShortestVertex** Path);

//����LSDB
void UpdateLSDB(
	Node* node);

//�õ����·����
void	FindShortestPath(
	Node* node);

void VertexHopForRouting(
	NodeAddress AdvertisingVertex,
	NodeAddress DestinationVertex,
	OpspfShortestVertex* Path,
	OpspfShortestVertex** Router);

//��д·�ɱ�
void FillOpspfRoutingTable(
	Node* node);//UpdateIpForwardingTable()

				//
void OpspfAddRoutingTableRowById(
	Node* node,
	NodeAddress nodeId,
	NodeAddress dstAddr);

//
int GetAddressID(
	double lon,
	double lat);

//
double Dec(
	double lat);

//
void FixLon(
	int t,
	Coord sat[]);

//
double radian(
	double d);

//
void Fin(
	int t,
	Coord mat[48]);

//
void ChangeOPSPFAdderss(
	Node* node);

//
void ChangeUserAdderss(
	Node* node);

//
void SendRigesterPacket(
	Node* node,
	Message* msg);

//
void SendBroadcastPacket(
	Node* node);

//
void InitRegisterList(
	Node* node);

//
void AddRegisterListRow(
	Node* node,
	NodeAddress usrip,
	bool isRelay,
	int RelaySTId);

//
registerRow* FindRegisterRowByUsrIp(
	Node* node,
	NodeAddress usrip);

//
void FreeRegisterList(
	Node* node);

//
void OpspfAddReigsterRow(
	Node* node,
	Message* msg);

//��������
void OpspfFinalize(
	Node* node);

void PrintOpspfDataInFile(
	Node* node);

void NetworkIpReceivePacketFromMacLayer(
	Node *node,
	Message *msg,
	NodeAddress previousHopAddress,
	int incomingInterface);
	
void EmptyRouteTable();

void NetworkUpdateForwardingTable(
	Node *node, 
	NodeAddress destAddress, 
	NodeAddress destAddressMask, 
	NodeAddress nextHopAddress, 
	int interfaceIndex, 
	int cost);

//int getNodetime();
