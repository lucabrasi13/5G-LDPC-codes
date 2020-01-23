#include<iostream>
#include<itpp/itbase.h>
#include<itpp/base/help_functions.h>
#include<itpp/itexports.h>

using namespace itpp;
using namespace std;

vec db2lin(vec valdB)
{
  // Function to convert linear scale
  // to dB
  return pow(10,valdB/10);
}

void write_file(vec a, vec b, vec c, int N){
// Function to write the vectors to a data file to plot
	ofstream data;
    data.open("data.txt");
    for(int i=0;i<N; i++){
        //data << fixed << a[i] << "    " << fixed << b[i] << endl;
        data << fixed << a[i] << "    " << fixed << b[i] << "    " << c[i]  << fixed << endl;
    }
    data.close();
}

void gnuplot_load(string fn)
{
// Function to pipeline and load gnuplot
	string comand="gnuplot -persist\n";
	FILE *pipe= popen(comand.c_str() , "w");
	ostringstream oss;
	oss.str(""); oss.clear();
	oss<<"load '"<< fn << "'" << endl;
	fprintf(pipe, "%s",oss.str().c_str());
	fflush(pipe);
	int r=pclose(pipe);
	if(r<0)cout<<"# error in pclose(), while using unix pipes!"<<endl;
}

void plot()
{
// Function to plot from a data file
	ofstream fout;
	fout.open("fgnuplot");
  fout << "set title 'BPSK BER comparison' " << endl;
	fout << "set grid" << endl;
	fout << "set xtics" << endl;
	fout << "set logscale y" << endl;
	fout << "set format y '10^{%L}'" << endl;
	fout << "set key right top box 3" << endl;
	fout <<	"#" << endl;
	fout << "set title 'BER comparison'" << endl;
	fout << "set xlabel 'Eb/N0(in dB) -->' font ',10'" << endl;
	fout << "set ylabel 'BER(dB) -->'  font ',10'" << endl;
	fout << "plot 'data.txt' u 1:2 title 'BER(theoretical)' w lp lc 7 lt 6 lw 1, 'data.txt' u 1:3 title 'BER(simulation)' w lp lt 8" << endl;
	fout << "#" << endl;
	fout << endl;
	fout.close();
	gnuplot_load("fgnuplot");
}

bvec genMsgBits(int n, double p)
{
  // Function to generate a vector
  // of Bernoulli random variables
  Bernoulli_RNG bernRV(p);
  return bernRV(n);
}

vec msg2symbol(int N, bvec msg)
{
  // Convert msg to BPSK modulated
  // symbols
  vec symbol(N);
  for(int i=0;i<N;i++){symbol(i) = 1-2*(double)msg(i);}
  return symbol;
}

vec awgn_channel(int N , double sigma, vec symbol)
{
  // AWGN channel
  Normal_RNG randn(0.0, pow(sigma,1));
  return symbol + randn(N);
}

vec compute_bpsk_theoretical_bound(vec EbNo){
  // Compute the theoretical BER
  return 0.5*erfc(sqrt(EbNo));
}

int main()
{
  // Initialize the RNG
  RNG_randomize();

  // Define parameter for Monte Carlo
  // simulation
  float R = 1;
  int Npoints = size(EbNo);
  int N = 100;
  long double Nerrs = 0;
  double Nblocks = 10000;
  double tmp = 0;
  vec EbNodB = linspace_fixed_step(0.0,8.0,0.5);
  vec EbNo = db2lin(EbNodB);

  vec BER_th = compute_bpsk_theoretical_bound(EbNo);
  vec sigma = pow(R*2*EbNo,-1);
  vec BER_sim(Npoints);

  bvec msg(N);
  vec symbol(N), rx_vec(N);
  vec decoded_msg(N), decoded_symbol(N);

  for(int i=0;i<Npoints;i++){
    for(int j=0;j<Nblocks;j++){
      // Generate the message bits
      msg = genMsgBits(N, 0.5);

      // Convert message to symbol
      symbol = msg2symbol(N, msg);

      // AWGN channel
      rx_vec = awgn_channel(N, sigma(i), symbol);

      // Bit flip checking
      for(int k=0;k<N;k++){
        // Convert recived signal
        // to symbol and bits
        if(rx_vec(i) >= 0){decoded_msg(i) = 0;}
        else{decoded_msg(i) = 1;}
        // Count the error
        if(decoded_msg(i)!=(double)msg(i)){tmp+=1;}
      }
      Nerrs += tmp;
      tmp = 0;
    }
    BER_sim(i) = Nerrs/N/Nblocks;
    Nerrs = 0;
  }

  // Write to the file to plot
  write_file(EbNodB,BER_th, BER_sim,size(BER_th));
  plot();

  return 0;
}
