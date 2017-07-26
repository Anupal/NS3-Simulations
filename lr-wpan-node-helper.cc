#include "lr-wpan-node-helper.h"
#include <string>
#include <sstream>

namespace ns3 {

Mac16Address NodeHelperClass::getNeighbourTableAddress(int n) {
	return neighbour_table_address[n];
}

uint32_t NodeHelperClass::getNeighbourTableKeys(int n) {
	return neighbour_table_keys[n];
}

uint32_t NodeHelperClass::getNeighbourTableSize() {
	return neighbour_table_size;
}

void NodeHelperClass::setNeighbourTableAddress(int n, Mac16Address address) {
	neighbour_table_address[n] = address;
}

void NodeHelperClass::setNeighbourTableKeys(int n, uint32_t key) {
	neighbour_table_keys[n] = key;
}

uint32_t randomGenerator ()
{
	uint32_t a = 0;
	uint32_t b = 999;
  return uint32_t (rand() % (b-a) + a);
}

void swap(uint32_t *xp, uint32_t *yp)
{
	uint32_t temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// A function to implement bubble sort
void bubbleSort(uint32_t arr[], uint32_t n)
{
   uint32_t i, j;
   for (i = 0; i < n-1; i++)

       // Last i elements are already in place
       for (j = 0; j < n-i-1; j++)
           if (arr[j] > arr[j+1])
              swap(&arr[j], &arr[j+1]);
}

uint32_t
NodeHelperClass::RandAB (uint32_t a, uint32_t b)
{
  return uint32_t (rand() % (b-a) + a);
}
void NodeHelperClass::populateKeyArray(){
	uint32_t temp;
	uint32_t i,j;
	for(i=0;i<size;i++){
		temp = RandAB(1, poolSize);
		bool c = false;
		for(j=0;j<i;j++){
			if(temp == keys[j]){
				c = true;
				break;
			}
		}
		if(!c)
			keys[i] = temp;
		else
			i--;
	}

	bubbleSort(keys, size);
}

void NodeHelperClass::displayTable(){
	std::cout<<"\n" << Mac16Address::ConvertFrom(device.GetAddress()) << " Neighborhood Table is:\n";

		for(uint8_t i=0; i < neighbour_table_size; i++){
			std::cout<< "Device: " << getNeighbourTableAddress(i) << " :: Common key: " << getNeighbourTableKeys(i)<<std::endl;
		}
}


void NodeHelperClass::Broadcast(double seconds, uint8_t data[]){

		int l=0;
		while(data[l++]);

		Ptr<Packet> p0 = Create<Packet> (data, l);  // some test data
		McpsDataRequestParams params;
		params.m_srcAddrMode = SHORT_ADDR;
		params.m_dstAddrMode = SHORT_ADDR;
		params.m_dstPanId = 0;
		params.m_dstAddr = Mac16Address ("ff:ff");
		params.m_msduHandle = 0;
		params.m_txOptions = TX_OPTION_NONE;

			Simulator::ScheduleWithContext (1, Seconds(seconds),
							  &LrWpanMac::McpsDataRequest,
							  device.GetMac (), params, p0);
}

void NodeHelperClass::SendEncrypted(double seconds, uint8_t data[], Mac16Address address){

		int l=0;
		while(data[l++]);

		Ptr<Packet> p0 = Create<Packet> (data, l);  // some test data
		McpsDataRequestParams params;
		params.m_srcAddrMode = SHORT_ADDR;
		params.m_dstAddrMode = SHORT_ADDR;
		params.m_dstPanId = 0;
		params.m_dstAddr = address;
		params.m_msduHandle = 0;
		params.m_txOptions = TX_OPTION_NONE;

			Simulator::ScheduleWithContext (1, Seconds(seconds),
							  &LrWpanMac::McpsDataRequest,
							  device.GetMac (), params, p0);
}

bool
NodeHelperClass::checkNeighbourhoodTable(Mac16Address address) {
	for(uint8_t j=0; j < neighbour_table_size; j++){
		if(getNeighbourTableAddress(j) == address){
			return true;
		}

	}

	return false;
}

void numberParser(uint32_t val[],uint8_t in[]){
	uint8_t l = 0;
	uint8_t s = 0;
	uint8_t r=0;
	char temp[10];
	while(in[l]){
		if(in[l] == ' '){
			temp[s] = '\0';
			sscanf(temp, "%d", &val[r]);
			r++;
			s=0;
		}
		if(in[l] != ' '){
			temp[s] = in[l];
			s++;
		}
		l++;
	}
}

bool NodeHelperClass::checkMyKeys(uint32_t ikey){
	for(uint8_t i = 0; i < size; i++){
		if(ikey == keys[i])
			return true;
	}
	return false;
}

static
bool
DataReceive (Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address & sender){
	uint8_t temp[1000];
	p->CopyData(temp, 100);
	uint32_t recKeys[100];
	double delay;


	for(uint8_t i=0;i<numNodes;i++) {   // number of current devices to be passed : currently 6

		if(n[i]->GetNetDevice()->GetAddress() == device->GetAddress()) { // Find the device explicitly



			if((!n[i]->state) ){ // check state of the node - false - initial state

				if((!n[i]->checkNeighbourhoodTable(Mac16Address::ConvertFrom(sender))) && (n[i]->neighbour_table_size < numNodes) ){  // if address not in table then add it and the packet value

					numberParser(recKeys, temp);
					for(uint8_t j=0 ; j<n[i]->size ; j++) {
						if(n[i]->checkMyKeys(recKeys[j])) {

							n[i]->setNeighbourTableKeys(n[i]->neighbour_table_size, recKeys[j]);


							//std::cout<< Mac16Address::ConvertFrom(sender) << "  " << Mac16Address::ConvertFrom(n[i]->GetNetDevice()->GetAddress())<< " " <<  n[i]->getNeighbourTableKeys(n[i]->neighbour_table_size) <<std::endl;

							n[i]->setNeighbourTableAddress(n[i]->neighbour_table_size, Mac16Address::ConvertFrom(sender));
							n[i]->neighbour_table_size++;
							break;

						}
					}
				}
			}

			else {  // check state of the node - true - relay state
				if(!n[i]->checkCache(temp))
				{
					received[i]= true;
					totalTime = Simulator::Now().GetSeconds();
					//NS_LOG_UNCOND (totalTime << ": " <<Mac16Address::ConvertFrom(device->GetAddress()) <<  " received packet of size " << p->GetSize ()<< " from " << Mac16Address::ConvertFrom(sender) << " with data \'" << temp << "\'");


					for(uint8_t k=0; k < n[i]->neighbour_table_size; k++) {
						if(n[i]->getNeighbourTableAddress(k) != Mac16Address::ConvertFrom(sender)) {
							delay = 0.001*randomGenerator();
							n[i]->SendEncrypted( delay, temp, n[i]->getNeighbourTableAddress(k)); // forward packet after random delay
						}
					}
					n[i]->addToCache(temp);
				}

			}

			break; // no need to search further for nodes
		}
	}
	return true;
}
NodeHelperClass::NodeHelperClass(char address[], Ptr<MultiModelSpectrumChannel> channel, Vector position, uint16_t size, uint16_t poolSize){
		device.SetAddress(Mac16Address(address));
		device.SetChannel(channel);
		node.AddDevice(&device);
		device.GetPhy ()->TraceConnect ("TrxState", std::string ("phy0"), MakeCallback (&StateChangeNotification));
		mobility.SetPosition(position);
		device.GetPhy()->SetMobility(&mobility);
		//device.GetMac()->SetMcpsDataConfirmCallback(MakeCallback(&DataConfirm));
		//device.GetMac ()->SetMcpsDataIndicationCallback(MakeCallback(&DataIndication));

		device.SetReceiveCallback(MakeCallback(&DataReceive));
		this->size = size;
		this->poolSize = poolSize;
		neighbour_table_size = 0;
		state = false;
		populateKeyArray();
		memset(neighbour_table_keys, 0, numNodes);
		memset(neighbour_table_address, 0, numNodes);
}

bool
NodeHelperClass::compare(uint8_t* in1, uint8_t* in2){  //if equal then true
  int l1=0,l2=0;

  while(in1[l1])
    l1++;

  while(in2[l2])
    l2++;

   if (l1!=l2)
      return false;

   else
     while(l1>0){
       if(in1[l1]!=in2[l1])
         return false;
       l1--;
     }
   return true;
}

bool
NodeHelperClass::checkCache(uint8_t* in){  //if in cache then true
    if(compare(in, cache))
      return true;
  return false;
}

void NodeHelperClass::addToCache(uint8_t* in){
  int l=0;
  while(in[l]){
    cache[l] = in[l];
    l++;
  }
  cache[l] = '\0';

}


LrWpanNetDevice* NodeHelperClass::GetNetDevice(){
		return &device;
}

void NodeHelperClass::BroadcastKeys(double seconds){

	std::ostringstream fill;
	for (uint32_t i = 0; i < size; i++)
	{
		fill << int (keys[i]) << " ";
	}

	uint8_t temp [1000];
	char* a = new char[fill.str().size()+1];

	memset (a,0,fill.str().size()+1);
	memcpy (a,fill.str().c_str(),fill.str().size()+1);

	uint8_t l = 0;
	while(a[l]){
		temp[l] = a[l];
		l++;
	}
	temp[l] = '\0';

	Broadcast(seconds, temp);
}

}
