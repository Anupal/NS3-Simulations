#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/packet.h>
#include <cmath>

#include <iostream>



using namespace ns3;


void convert (char address[], int decimalNumber) {
    int quotient;
    int i=1,j,temp, k;
    char hexadecimalNumber[100];

    quotient = decimalNumber;

    while(quotient!=0)
    {
         temp = quotient % 16;

      //To convert integer into character
      if( temp < 10)
           temp =temp + 48;
      else
         temp = temp + 55;

      hexadecimalNumber[i++]= temp;
      quotient = quotient / 16;
    }

    address[2] = ':';
    address[5] = '\0';
     k = 4;


    for(j = 1 ; j< i;j++ ) {
        if ( k == 2 )
            k--;
        address[k] = hexadecimalNumber[j];
        k--;
    }

    while(k>=0) {
        if ( k == 2 )
            k--;
        address[k] = '0';
        k--;
    }

}

int randomInt (int a, int b)
{
  return int (rand() % (b-a) + a);
}

uint8_t randomGenerator ()
{
	uint8_t a = 1;
	uint8_t b = 99;
  return uint8_t (rand() % (b-a) + a);
}


///////////////////////////////////////////

const int numAttack = 180;
int attackedNodes[numAttack];

///////////////////////////////////////////


void attack () {
	int i, j, temp;

	std::cout<<"Silenced nodes are: ";
	for(i=0;i<numAttack;i++) {

		temp = randomInt(2, numNodes);
		bool c = false;
		for(j=0;j<i;j++) {
			if ( temp == attackedNodes[j]) {
				c = true;
				break;
			}
		}

		if(!c) {
			attackedNodes[i] = temp;
			//std::cout<<Mac16Address::ConvertFrom( n[attackedNodes[i]]->GetNetDevice()->GetAddress() )<<',';
			n[attackedNodes[i]]->neighbour_table_size = 0;
		}
		else
			i--;

	}
	std::cout<<'\n';
}


void changeStates(){
	for(uint8_t i=0; i<numNodes; i++){
		n[i]->state = true;
		//std::cout<< "\nposition: " << n[i]->mobility.GetPosition();
		//n[i]->displayTable();
		//std::cout<<Simulator::Now().GetSeconds() << Mac16Address::ConvertFrom(n[i]->GetNetDevice()->GetAddress()) << " state true!\n";
	}

	attack();
}





int main (int argc, char *argv[])
{
  srand(time(NULL));
  bool verbose = false;

  ns3::PacketMetadata::Enable ();

  CommandLine cmd;

  cmd.AddValue ("verbose", "turn on all log components", verbose);

  cmd.Parse (argc, argv);

  LrWpanHelper lrWpanHelper;
  if (verbose)
    {
      lrWpanHelper.EnableLogComponents ();
    }

  //Setup of channel
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel> ();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->AddPropagationLossModel (propModel);
  channel->SetPropagationDelayModel (delayModel);

  uint32_t K,k;

  K = numNodes*log(numNodes);
  k = sqrt(17) * log(numNodes);

  for(int i =0; i<numNodes;i++)
  		received[i] = false;


  std::cout<< "\nK = " << K;
  std::cout<< "\nk = " << k<<'\n';

  char address[8];

  	int max = 500;
  	int  x0 = randomInt(0,max);
  	int  y0 = randomInt(0,max);

  	convert(address, 1);

  	n[0] = new NodeHelperClass (address, channel, Vector (x0, y0, 0), k, K);

  	for (int i = 1;i<numNodes; i++) {
  		convert(address, i+1);
  		n[i] = new NodeHelperClass (address, channel, Vector (randomInt(0, max), randomInt(0, max), 0), k, K);

  	}
  	convert(address, numNodes+1);
  	NodeHelperClass* ntemp = new NodeHelperClass (address, channel, Vector (abs(x0 - 10), abs(y0 - 10), 0) ,k, K); //dummy for initial packet

  //schedule state change
  	Simulator::Schedule(Seconds(90), &changeStates);


  // Initial phase
  	for (int i = 0;i<numNodes; i++)
  		n[i]->BroadcastKeys(3*0.01*randomGenerator());
  	for (int i = 0;i<numNodes; i++)
  	  		n[i]->BroadcastKeys(4 + 3*0.01*randomGenerator());
  	for (int i = 0;i<numNodes; i++)
  	  	  		n[i]->BroadcastKeys(8 + 3*0.01*randomGenerator());


	//Flooding phase

	uint8_t data[] = "test";
	convert(address, 1);
	ntemp->SendEncrypted(100, data, address);


	lrWpanHelper.EnablePcapAll (std::string ("lr-wpan-data"), true);

	Simulator::Run ();


	double compromised = 0;

	/*for(int i =0; i<numAttack;i++)
		received[attackedNodes[i]] = false;*/

	for(int i =0; i<numNodes;i++) {
		//std::cout <<  Mac16Address::ConvertFrom(n[i]->GetNetDevice()->GetAddress()) << "     " <<  received[i] << "\n";
		if (!received[i])
			compromised++;
	}
	uint32_t avg = 0;
	for(int i =0; i<numNodes;i++)
		avg += n[i]->neighbour_table_size;

	avg/=numNodes;
	std::cout << "\nAverage Table Size: " << avg;
	std::cout<< "\nOrphan percentage: " <<(float)(compromised/numNodes*100)<<'\n';
	std::cout<< "\nTotal Time: " <<totalTime-100<<'\n';
	Simulator::Stop ();
	Simulator::Destroy ();
	return 0;
}





