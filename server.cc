#include <iostream>
#include <string>
#include <cassert>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <list>
#include <vector>
#include <math.h>
#include <algorithm>
#include <map>

#include <unistd.h>


using namespace std;
using namespace zmqpp;

class nodo{

    private:
    	int id;
        double pageRank;			
    public:
        //CONSTRUCTORES
        nodo(){}
        nodo(const int &id,const double &pageRank){
            this->id = id;
            this->pageRank = pageRank;
        }
        //GETTERS
        const int &getid() const{
            return id;
        }
        const double &getpageRank() const{
            return pageRank;
        }
};

bool comparator2(nodo a,nodo b)
{
    return (a.getpageRank() > b.getpageRank());
}

vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while (getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal;
}


map<int,vector<int>> GenListaPi(map<int,vector<int>> nodos){// lista cada nodo con la cantidad de enlaces salientes y me retorna el numerode nodos nulos 
	
		for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
			if(indx->second.size() == 0){
				for(map<int,vector<int>>::iterator indx2=nodos.begin();indx2!=nodos.end();++indx2){
					if(indx->first!=indx2->first )
						indx->second.push_back(indx2->first);
			
					}
					
			}
		}
	return nodos;
}

map<int, vector<int>> loadFile(string name) {
	int index,post;
	map<int, vector<int>> dic;
	ifstream fe(name);
	
	while(fe >>index >> post){
      dic[index].push_back(post);
	  dic[post];
	}
	fe.close();
  	return dic;
}

vector<string>  InicialEnv(map<int,vector<int>>  dic,int Numk){
	int partes=2;
	int it=0;
	int part=1;
	bool ban;
	string aux="";
	vector<string> vec; 

	partes =1;	
	
	for (map<int,vector<int>>::iterator indx=dic.begin();indx!=dic.end();++indx ) {
		ban=true;
		aux += to_string(indx->first )+':';
		for (const int &l : indx->second){
			if(ban){
				aux= aux+to_string(l);
				ban=false;
			}else{
				aux= aux +','+to_string(l);
			}
			//cout<<indx->first <<":"<<l<<endl;
		}
		it++;	
		if(it>=Numk*part && part<partes){
			part++;
			vec.push_back(aux);
			aux="";
		}else{
			aux=aux+'&';
		}	
	}
	vec.push_back(aux);
	//cout<<vec.size()<<endl;
	return vec;
}


int main(int argc, char **argv) {

	int N;			
	map<int,vector<int>>  nodos;
	map<int,double>  probabilidades;

	nodos= loadFile("Wiki-Vote.txt");
	nodos= GenListaPi(nodos);
	N=nodos.size();
	cout<<"tamDic= "+to_string(N)<<endl;

	double probInit = float(1)/float(N);
	cout<<"prob init->"<<probInit<<endl;

	bool flagone = true;		//para poner la probabilidad del primer nodo en 1
	string probToSend = "";		//probabilidades iniciales
	string ndo = "";			//se guardan los nodos que trabajara determinado worker
	int partes = 2;				//cantidad de partes en que se repartiran los nodos
	int cantPar = int(N/partes);//cantidad de nodos por envio
	int j = 1;

	//Inicializo y parto probabilidades
	vector<string> vecToSend;
	for(map<int,vector<int>>::iterator indx=nodos.begin();indx!=nodos.end();++indx){
		if(flagone){
			probabilidades[indx->first]=1;
			probToSend += to_string(indx->first) + ":" +to_string(1.00)+"&";
			flagone=false;
		}else{
			probabilidades[indx->first]=0;
			probToSend += to_string(indx->first) + ":" +to_string(0.00)+"&";
		}
		ndo += to_string(indx->first) + "&";

		if(j < cantPar){
			j++;
		}else{
			vecToSend.push_back(ndo);
			j = 0;
			ndo ="";
		}
	}


	cout<<"Probabilidades= "<<probabilidades.size()<<endl;
	int sizeNodos = vecToSend.size();
	cout<<"vecToSend= "<<sizeNodos<<endl;

	//PUERTO DE ENVIO
	message m;
	context ctx;
	socket r(ctx,socket_type::push);
	cout << "Publico por socket tcp puerto 5220\n";
	r.bind("tcp://*:5220"); 
	//PUERTO DE RECIBO
	context ctx2;
	socket s(ctx2,socket_type::xrep);
	cout << "Escuchando socket tcp puerto 5450\n";
	s.bind("tcp://*:5450"); //DIRECCION POR LA QUE ESCUCHA EL BROKER*/

	poller p;
	p.add(s, poller::poll_in);

	cout<<"Algoritmo pageRank \n";

	//Envio probabilidades iniciales y nodos que trabajara cada worker
	for (const string &parte :vecToSend){
		m<<probToSend;
		m<<parte;
		r.send(m);
	}
	cout<<"END ONEEEE"<<endl;

	cout<<"Esperando respuesas de los worker.."<<endl;
	cout<<endl;

	vector<string> separatorSum;
	vector<string> separatorConver;		//extraer un binario (convergio o no convergio) en el W
	vector<string> separatorError;
	string newsRP="";
	int entrantes=0;
	double error=0;
	double sum=0;						//la sumatoria de los pageRank debe ser igual a 1
	int convergentes = 0;				//debe ser igual al numero de partes enviadas
	int iteracion = 0;

	while (true) {

	    if(p.poll(500)){
	    	cout<< "\nLLEGO ALGO!!"<<endl;
	    	string fromW;
			string id;

		    message n;
		    s.receive(n);
		    n>>id;
		    n>>fromW;
		    //cout<<fromW<<endl;
	    	entrantes++;
	    	// &8297:2.10822e-05&!1.4233/0_0.090751
    		separatorSum = split(fromW,'_');
    		sum += stod(separatorSum[1]);	
    		separatorConver = split(separatorSum[0],'/');
			convergentes += atoi(separatorConver[1].c_str());
			separatorError = split(separatorConver[0],'!');
			error += stod(separatorError[1]);
			newsRP += separatorError[0];

	    	if(entrantes == sizeNodos){	//espero a que lleguen todos para caclular el error total
	    		if(convergentes == sizeNodos){
	    			cout<<"CONVERGIO"<<endl;
	    			break;
	    		}else{
	    			iteracion++;
	    			cout<<"iteracion "<<iteracion<<" error = "<<error<<" sumatoria de PR = "<<sum<<endl;

	    			message newPR;
	    			error=0;
					sum=0;	
					convergentes = 0;
					entrantes=0;

	    			for(const string &parte :vecToSend){
						newPR<<newsRP;		//nuevas probabilidades
						newPR<<parte;
						r.send(newPR);
					}
					string newsRP="";
	    		}
	    	}



	   	}
	}

	vector<string> auxres = split(newsRP,'&');
	vector<string> auxN;
	vector<nodo> results;

	//guardo resultados finales en un vector para ordenarlo
	for(const string &nod : auxres){
		auxN = split(nod,':');
		nodo n(atoi(auxN[0].c_str()),stod(auxN[1]));
		results.push_back(n);
	}

	make_heap (results.begin(),results.end(),comparator2);
	sort_heap(results.begin(),results.end(),comparator2);
	//t = clock() - t;
  	//printf ("Tiempo del algoritmo distribuido: (%f seconds).\n",((float)t)/CLOCKS_PER_SEC);
  	//imprimo resultados
  	for(const nodo &n : results){
  		cout<<"nodo : "<<n.getid()<<" Probabilidad :"<<n.getpageRank()<<endl;
  	}

	cout<<"FIN DEL ALGORITMO"<<endl;

	return 0;
}