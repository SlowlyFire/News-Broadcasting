// Gal Giladi
// for "config.txt" I chose Option 2 (numbers only)
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <random>
#include <unistd.h>
#include <queue>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

class BoundedQueue {
    public:
    queue<string> *boundedQueue;
    sem_t full, empty;
    pthread_mutex_t mutex;

    // constructor
    BoundedQueue(int size) {
        // initial our 2 semaphores - empty (with size) and full (with 0)
	    sem_init(&empty,0,size);
	    sem_init(&full,0,0);
        pthread_mutex_init(&mutex,NULL);
        boundedQueue = new queue<string>;
    }

    void enqueue(string item) {
        // if empty is greater than 0, then we decrement the semaphore by 1.
        // if empty is zero, then we block the action, untill empty will be greater than 0.
        sem_wait(&empty);
		pthread_mutex_lock(&mutex);
        boundedQueue->push(item);
		//cout<<"Bounded produced: "<<item<<endl;
		pthread_mutex_unlock(&mutex);
        // if full is greater than 0, then we increment the semaphore by 1.
        // if full is zero, so we don't have any items in our bounded buffer, so the consumer
        // can't deququq (remove an item), so we block the consumer action untill full is greater than 0.
		sem_post(&full);
    }

    string dequeue() {
        // if full is greater than 0, then we decrement the semaphore by 1.
        // if full is zero, then we block the action, untill full will be greater than 0.
        sem_wait(&full);
		pthread_mutex_lock(&mutex);
        // insert to item the oldest element
        string item = boundedQueue->front();
        boundedQueue->pop();
        //cout<<"Bounded Consumed: "<<item<<endl;
		pthread_mutex_unlock(&mutex);
        // if empty is greater than 0, then we increment the semaphore by 1.
        // if empty is zero, then our bounded buffer is full, so the producer
        // can't enqueue (add an item), so we block the producer action untill empty is greater than 0.
		sem_post(&empty);
        return item;
    }
};

class UnboundedQueue {
    public:
    queue<string> *unboundedQueue;
    sem_t full;
    pthread_mutex_t mutex;

    // constructor
    UnboundedQueue() {
        // initial our semaphore - full (with 0)
	    sem_init(&full,0,0);
        pthread_mutex_init(&mutex,NULL);
        unboundedQueue = new queue<string>;
    }

    void enqueue(string item) {
		pthread_mutex_lock(&mutex);
        unboundedQueue->push(item);
		pthread_mutex_unlock(&mutex);
        // if full is greater than 0, then we increment the semaphore by 1.
        // if full is zero, so we don't have any items in our unbounded buffer, so the consumer
        // can't deququq (remove an item), then we block the consumer action untill full is greater than 0.
		sem_post(&full);
    }

    string dequeue() {
        // if full is greater than 0, then we decrement the semaphore by 1.
        // if full is zero, then we block the action, untill full will be greater than 0.
        sem_wait(&full);
		pthread_mutex_lock(&mutex);
        // insert to item the oldest element
        string item = unboundedQueue->front();
        unboundedQueue->pop();
		//cout<<"Unbounded Consumed: "<<item<<endl;
		pthread_mutex_unlock(&mutex);
        return item;
    }
};

// struct of producer's arguments, depends on "config.txt" sections (3 lines in ea section)
struct producerArgs {
    // first line
    int id;
    // second line
    int countOfNews;
    // third line
    int sizeQueue;
};

// globals
// vector of thread type variables - vector of producers
vector<pthread_t*> producers;
// vector of BoundedQueue type variables - vector of bounded queues for ea producer
vector<BoundedQueue*> producersBQ;
// 3 UnboundedQueue objects
UnboundedQueue* sportUBQ = new UnboundedQueue();
UnboundedQueue* newsUBQ = new UnboundedQueue();
UnboundedQueue* weatherUBQ = new UnboundedQueue();
// BoundedQueue object of screen
BoundedQueue* screenBQ;

void* producerCreation(void *arg) {
    int id = ((producerArgs*)arg)->id;
    int countOfNews = ((producerArgs*)arg)->countOfNews;
    int sportCounter = 0;
    int newsCounter = 0;
    int weatherCounter = 0;
    // create news
    for (int i = 0; i < countOfNews + 3; i++) {
        // rand a type of news
        int kindOfNews = rand() % 3;
        string item;
        switch(kindOfNews) {
            case 0:
                item = "Producer " + to_string(id) + " SPORTS " + to_string(sportCounter++);
                producersBQ[id - 1]->enqueue(item);
                break;
            case 1:
                item = "Producer " + to_string(id) + " NEWS " + to_string(newsCounter++);
                producersBQ[id - 1]->enqueue(item);
                break;
            case 2:
                item = "Producer " + to_string(id) + " WEATHER " + to_string(weatherCounter++);
                producersBQ[id - 1]->enqueue(item);
                break;
        }
    }

    // insert last demi news
    producersBQ[id - 1]->enqueue("DONE");
    pthread_exit(NULL);
    return 0;
}

void* dispatcherCreation(void* args) {
    int readenProducers = 0;
    int numOfProducers = producers.size();
    // keep looping as long we haven't finished reading all producers items
    while (readenProducers < numOfProducers) {
        // using round-robin algorithem
        for (int i = 0; i < numOfProducers; i++) {
            if (producersBQ[i] != NULL) {
                string item = producersBQ[i]->dequeue();
                // if we have finished to read all news in a producer, then change the queue of the producer to NULL
                if (item == "DONE") {
                    //cout<< "item is DONE in dispatcherCreation func"<<endl;
                    readenProducers ++;
                    producersBQ[i] = NULL;
                } else {
                    // else we get the third word (Sport, Weather, News) and enqueue to its queue
                    stringstream s(item);
                    string input;

                    while(getline(s, input, ' ')) {
                        if (input == "WEATHER"){
                            weatherUBQ->enqueue(item);
                            //cout<<"inserting to weather queue"<<endl;
                        } else if (input == "SPORTS"){
                            sportUBQ->enqueue(item);
                            //cout<<"inserting to sport queue"<<endl;
                        } else if (input == "NEWS") {
                            newsUBQ->enqueue(item);
                            //cout<<"inserting to news queue"<<endl;
                        }
                    }
                }
            }
        }
    }
    // enqueue last demi news
    //cout<<"inserting DONE to all unbounded queues"<<endl;
    sportUBQ->enqueue("DONE");
    newsUBQ->enqueue("DONE");
    weatherUBQ->enqueue("DONE");
    pthread_exit(NULL);
    return 0;
}

void* coEditNews(void* args) {
        while(1) {
            string item = newsUBQ->dequeue();
            if (item == "DONE") {
                screenBQ->enqueue(item);
                break;
            } else {
                screenBQ->enqueue(item);
            }
        }
        pthread_exit(NULL);
        return 0;
}

void* coEditSport(void* args) {
        while(1) {
            string item = sportUBQ->dequeue();
            if (item == "DONE") {
                screenBQ->enqueue(item);
                break;
            } else {
                screenBQ->enqueue(item);
            }
        }
        pthread_exit(NULL);
        return 0;
}

void* coEditWeather(void* args) {
        while(1) {
            string item = weatherUBQ->dequeue();
            if (item == "DONE") {
                screenBQ->enqueue(item);
                break;
            } else {
                screenBQ->enqueue(item);
            }
        }
        pthread_exit(NULL);
        return 0;
}

void* screenManager(void *arg) {
    int counter = 0;
    while (true) {
        string item = screenBQ->dequeue();
        if (item == "DONE"){
            counter++;
            //cout<< "counted DONE in screenManager" <<endl;
            if (counter == 3) {
                break;
            }
        } else {
            cout  << item << endl;
        }
        sleep(1);
    }
    pthread_exit(NULL);
    return 0;
}

int main(int argc, char *argv[]){
    if(argv[1] == NULL) {
        cout<< "Please run with a configuration file"<<endl;
        exit(-1);
    }
    // opens config.txt
    string filename(argv[1]);
    ifstream input_file(filename);
    if (!input_file.is_open()) {
        cout<< "Could not open the file - '" << filename << "'" << endl;
        exit(-1);
    }

    string producerID;
    string countNews;
    string queueSize;
    string spaceLine;
    string coEditorQueueSize;

    while(1) {
        // reads first line of each section from "config.txt" and inserts to producerID
        getline(input_file, producerID);
        // reads second line of each section from "config.txt" and inserts to countNews
        getline(input_file, countNews);
        // if there is no line to read, then we have got to the last section, where we have
        // only one line, which is the co-editor queue size. then we copy the producerID from
        // the previous line to coEditorQueueSize, and break the loop, because we finished reading.
        if (countNews == "") {
            coEditorQueueSize = producerID;
            break;
        }
        // reads third line of each section from "config.txt" and inserts to queueSize
        getline(input_file, queueSize);
        // reads fourth line of each section from "config.txt" and inserts to line
        getline(input_file, spaceLine);

        // create producer thread params.
        struct producerArgs *params;
        params = (producerArgs *)malloc(sizeof(*params));
        params->id = stoi(producerID);
        params->countOfNews = stoi(countNews);
        params->sizeQueue = stoi(queueSize);

        // push new BoundedQueue object to producersBQ (vector of BoundedQueue type variable)
        producersBQ.push_back(new BoundedQueue(stoi(queueSize) + 1));
        // push new thread to producers (vector of thread type variable)
        producers.push_back(new pthread_t());
        // create the producer thread
        int err =  pthread_create(producers[stoi(producerID) - 1], NULL, producerCreation, (void*)params);   
        if(err) {
            cout<<"pthread creation have failed"<<endl;
            exit(-1);
        } 
    }
 
    // declares dispatcher thread
	pthread_t consumer;
    // create the consumer thread
    int err =  pthread_create(&consumer, NULL, dispatcherCreation, NULL);
    if(err) {
        cout<<"pthread creation have failed"<<endl;
        exit(-1);
    } 

    // declare coEditors threads
    pthread_t coEditorNews;
    pthread_t coEditorSport;
    pthread_t coEditorWeather;
    // creates shared bounded queue to coEditors and screen manager threads 
    screenBQ = new BoundedQueue(stoi(coEditorQueueSize));
    
    // create 3 co-editors threads
    int err1 = pthread_create(&coEditorNews, NULL, coEditNews, NULL);
    if(err1) {
        cout<<"pthread creation have failed"<<endl;
        exit(-1);
    } 
    int err2 = pthread_create(&coEditorSport, NULL, coEditSport, NULL);
    if(err2) {
        cout<<"pthread creation have failed"<<endl;
        exit(-1);
    } 
    int err3 = pthread_create(&coEditorWeather, NULL, coEditWeather, NULL);
    if(err3) {
        cout<<"pthread creation have failed"<<endl;
        exit(-1);
    } 

    // declare screen thread
    pthread_t screen;
    // creates screen manager thread
   // cout<<"screen sharing:"<<endl;
    int errScreen = pthread_create(&screen, NULL, screenManager, NULL);
     if (errScreen) {
        cout<<"pthread creation have failed"<<endl;
        exit(-1);
    } 

    int joinErr = pthread_join(screen, NULL);
    if (joinErr) {
        cout<<"pthread creation have failed"<<endl;
        exit(-1);
    }
    cout << "DONE" << endl;
    pthread_exit(NULL);
    return 0;
}
