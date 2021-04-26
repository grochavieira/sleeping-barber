#include <stdio.h>
#include <chrono>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <time.h>

using namespace std;

#define Barbeiros 1
#define Clientes 16

int Barbeiro[Barbeiros];
int Cliente[Clientes];

pthread_cond_t condicao[Barbeiros];
pthread_cond_t condicaoClientes[Clientes];

bool clienteFoiAtendido[Clientes] = {
  false, false, false, false, false,
  false, false, false, false, false,
  false, false, false, false, false,
  false
};


bool cadeiras[5] = {false, false, false, false, false};
bool cadeiraDoBarbeiro = false;
char tipoPessoaSentadaNaCadeira;

queue<int> filaClientes;

int tempoTotalDeCortes = 0;
float minutosDormindo = 0;
int clientesAtendidos = 0;


void *threadBarbearia(void *arg);

int main(int argc, char *argv[]){ //main

  pthread_t ph_thread[Barbeiros];
  pthread_t ph_threadCliente[Clientes];

  for (int i = 0; i < Barbeiros; i++) {
    pthread_cond_init(&condicao[i], NULL);
    Barbeiro[i] = i;
  }

  for (int i = 0; i < Clientes; i++) {
    pthread_cond_init(&condicaoClientes[i], NULL);
    Cliente[i] = i + 1;
  }
  
  for (int i = 0; i < Barbeiros; i++) {
    printf("%4d - Barbeiro #%d começou a trabalhar.\n",i, i + 1);
    pthread_create(&ph_thread[i], NULL, threadBarbearia, &Barbeiro[i]);
    sleep(0);
  }
  
  int tempo = 0;
  int novoTempo = 0;
  for (int i = 0; i < Clientes; i++) {
    if((i + 1)%2 == 0){
      novoTempo = rand() % (6000-tempo);
      usleep(1000 * novoTempo);
    } else {
      tempo = (rand() % 5999);
      usleep(1000 * tempo);
    }

    printf("%4d - Cliente #%d chegou no estabelecimento.\n",Cliente[i], i + 1);
    pthread_create(&ph_threadCliente[i], NULL, threadBarbearia, &Cliente[i]);

    if((i + 1)%2 == 0) {
      int restoTempo = (6000 - (tempo + novoTempo));
      usleep(1000 * restoTempo);
    }
  }

  //liberar threads
  for (int i = 0; i < Barbeiros; i++)
    pthread_join(ph_thread[i], NULL);

  for (int i = 0; i < Clientes; i++)
    pthread_join(ph_threadCliente[i], NULL);

  for (int i = 0; i < Barbeiros; i++)
    pthread_cond_destroy(&condicao[i]);
  
  for (int i = 0; i < Clientes; i++)
    pthread_cond_destroy(&condicaoClientes[i]);

  return(0);
}

void *threadBarbearia(void *arg)
{
  int valorThread = *(int *)arg;
  bool entrouBarbearia = false;

  if (valorThread == 0) {
    auto t_start = std::chrono::high_resolution_clock::now();

    double elapsed_time_ms = 0;

    while(elapsed_time_ms <= 48000) {
      if (!cadeiraDoBarbeiro && filaClientes.empty()) {
        cout << "Barbeiro sentou" << endl;
        cadeiraDoBarbeiro = true;
        tipoPessoaSentadaNaCadeira = 'B';
      } 
      else if (!cadeiraDoBarbeiro && filaClientes.size() > 0) {
        cout << "Cliente " << filaClientes.front() << " sentou" << endl;
        cadeiraDoBarbeiro = true;
        tipoPessoaSentadaNaCadeira = 'C';
      } 
      else if (cadeiraDoBarbeiro && tipoPessoaSentadaNaCadeira == 'C' && filaClientes.size() > 0) {
        int clienteAtual = filaClientes.front(); 

        cout << "Barbeiro esta cortando o cabelo do cliente - " << clienteAtual << endl;

        int tempoDeCorte = (2500 + (-500 + rand() % 1000));
        tempoTotalDeCortes += tempoDeCorte;
        usleep(1000 * tempoDeCorte);

        filaClientes.pop();
        cout << "Barbeiro terminou o cabelo do cliente - " << clienteAtual << endl;

        cadeiraDoBarbeiro = false;
        clienteFoiAtendido[clienteAtual - 1] = true;
        clientesAtendidos++;
      }
      
      auto t_end = std::chrono::high_resolution_clock::now();
      elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end-t_start).count();
    }

    for (int i = 0; i < filaClientes.size(); i++){
      int clienteAtual = filaClientes.front();
      clienteFoiAtendido[clienteAtual - 1] = true;
      filaClientes.pop();
    }

    cout << "\nBarbeiro terminou seu expediente...\n";

    cout << "\nO barbeiro atendeu " << clientesAtendidos << " clientes\n";

    int horasTrabalhadas = (tempoTotalDeCortes/100)/60;
    int minutosTrabalhados = (tempoTotalDeCortes/100)%60;
    cout << "O barbeiro trabalhou durante: " << horasTrabalhadas << ":" << minutosTrabalhados << endl;

    int tempoAtual = (elapsed_time_ms - tempoTotalDeCortes);
    int horasDormido = (tempoAtual/100)/60;
    int minutosDormindo = (tempoAtual/100)%60;

    cout << "O barbeiro dormiu durante: " << horasDormido << ":" << minutosDormindo << endl;

  } else {
    while (!clienteFoiAtendido[valorThread - 1]) {
      if (!entrouBarbearia) {
        if (filaClientes.size() < 5) {
          filaClientes.push(valorThread);
          if (tipoPessoaSentadaNaCadeira == 'B') {
              cadeiraDoBarbeiro = false;
          }
        } else {
          clienteFoiAtendido[valorThread - 1] = true;
          cout << endl << "não tinha acento para o cliente: " << valorThread << endl;
        }
        entrouBarbearia = true;
      }  
    }
    cout << endl << "Cliente " << valorThread << " foi embora..." << endl;
  }

  return 0;
}
