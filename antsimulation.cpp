#include <iostream>
#include <vector>
#include <random>
#include <climits>
#if defined __cplusplus
using namespace std;
int di[8] = {-1, -1,  0,  1,  1,  1,  0, -1};
int dj[8] = {0,  -1, -1, -1,  0,  1,  1,  1};
//           L   UL   U  UR   R   DR  D  DL

// Parameters
int rows, cols;
int initialAnts = 5;
int antsPerWave = 0;
int timePerWave = 10;
int maxTime = 1000;
int dataInterval = 10;
int sourceX, sourceY;
int foodX, foodY;

// true if there is a block there, false if not
vector< vector<bool> > blocks;

class Ant {
    public:
    int x;
    int y;
    int currentDirection = 0;
    bool foundFood;
    int currentPath;
    int currentHealth = 50;
    vector<vector<bool>> v;
    Ant(int x, int y) : x(x), y(y) {
        foundFood = false; currentPath = 0;
        v.resize(rows,vector<bool>(cols, 0));
    };
};
class Pheromone {
    public:
    int strength[8];
    Pheromone() {
        for (int i = 0 ; i < 8 ; i++) strength[i] = 0;
    }
};

//data varaibles
double totalPathLength = 0;
int paths = 0;
int foodInColony = 0;
vector< vector<int> > density;
 
vector< vector<Pheromone> > pheromones;
vector< vector<Pheromone> > pheromonesToColony;
vector<Ant> ants;
vector<int> toRemove;

vector<pair<double,double>> datas;

static std::random_device rd;
static std::mt19937 rng;

int randomNum(int upper) {
   static std::uniform_int_distribution<int> uid(1,INT_MAX); // random dice
   return uid(rng) % upper; // use rng as a generator
}
void dump() {
    vector< vector<char> > display(rows, vector<char>(cols, ' '));
    /*
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            int sum = 0;
            for (int k = 0 ; k < 8 ; k++)
            sum += pheromones[i][j].strength[k];
            cout << sum << " ";
        }
        cout << endl;
    }*/
    for (Ant currentAnt : ants) {
        display[currentAnt.y][currentAnt.x] = 'a';
        if (currentAnt.foundFood) display[currentAnt.y][currentAnt.x] = 'f';
       
    }
    display[foodY][foodX] = 'F';
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            if (blocks[i][j]) display[i][j] = '#';
        }
    }
  
    for (int i = 0 ; i < rows ; i++) {
        cout << "-";
        for (int j = 0 ; j < cols ; j++) {
            cout << display[i][j] << "-";
        }
        if (i != rows-1) cout << endl;
    }
    //cout << "food found = " << foodInColony << endl;
}
void simulate() {
    int indOfCurrentAnt = 0;
    for (Ant &currentAnt : ants) {
        int moveWeights[] = {10,10,10,10,10,10,10,10};
//                           L  UL  U  UR  R  DR  D   DL
 
        //apply pheromones
        if (!currentAnt.foundFood) {
            for (int i = 0 ; i < 8 ; i++) {
                moveWeights[i] += pheromones[currentAnt.y][currentAnt.x].strength[i];
            }
        } else {
            for (int i = 0 ; i < 8 ; i++) {
                moveWeights[i] += pheromonesToColony[currentAnt.y][currentAnt.x].strength[i];
            }
        }

        //encourage the ant to keep walking the same direction as they are going
        int curind = (currentAnt.currentDirection + 7) % 8;
        for (int i = 0 ; i < 3 ; i++) {
            moveWeights[curind] += 15;
            curind = (curind+1) % 8;
        }
        moveWeights[currentAnt.currentDirection] += 40;

        //Check if the ant is at the food
        if (currentAnt.x == foodX && currentAnt.y == foodY) {
            currentAnt.foundFood = true;
            totalPathLength += currentAnt.currentPath;
            paths++;

            for (int i = 0 ; i < rows; i++) {
                for (int j = 0 ; j < cols ; j++) {
                    currentAnt.v[i][j] = false;
                }
            }
            currentAnt.currentHealth = 50;
            currentAnt.currentPath = 0;
        }

        // the ant has found food and has returned to the colony
        if (currentAnt.foundFood && currentAnt.x == sourceX && currentAnt.y == sourceY) {
            currentAnt.foundFood = false;
            foodInColony++;

            for (int i = 0 ; i < rows; i++) {
                for (int j = 0 ; j < cols ; j++) {
                    currentAnt.v[i][j] = false;
                }
            }
        }
        //bound checking
        cout << "bounds" << endl;
        for (int i = 0 ; i < 8 ; i++) {
            int newx = currentAnt.x + di[i];
            int newy = currentAnt.y + dj[i];
            if (newx < 0 || newx >= cols || newy < 0 || newy >= rows) {
                moveWeights[i] = 0;
            } else {
                
                int next = 0;
                next += pheromones[newy][newx].strength[i];
                moveWeights[i] += next / 10;
                if (currentAnt.v[newy][newx]) {
                    moveWeights[i] = 0;
                }
                if (blocks[newy][newx]) {
                    moveWeights[i] = 0;
                }
            }
        }
        cout << "weights" << endl;
        int total = 0;
        for (int i = 0 ; i < 8 ; i++) {
            total += moveWeights[i];
        }
        if (total == 0) {
            toRemove.push_back(indOfCurrentAnt);
            indOfCurrentAnt++;
            continue;
        }
        int chosen = randomNum(total);
        int move = 0;
        for (int i = 0 ; i < 8 ; i++) {
            if (moveWeights[i] <= chosen) {
                chosen -= moveWeights[i];
            } else {
                move = i;
                break;
            }
        }
        if (!currentAnt.v[currentAnt.y][currentAnt.x]) {
            if (currentAnt.foundFood) pheromones[currentAnt.y][currentAnt.x].strength[(move+4)%8] += 60;
            else pheromonesToColony[currentAnt.y][currentAnt.x].strength[(move+4)%8] += 60;
        }
        currentAnt.v[currentAnt.y][currentAnt.x] = true;
        currentAnt.x += di[move];
        currentAnt.y += dj[move];
        density[currentAnt.y][currentAnt.x]++;

        currentAnt.currentDirection = move;
        currentAnt.currentPath++;
        
        currentAnt.currentHealth--;
        if (currentAnt.currentHealth == 0) {
            toRemove.push_back(indOfCurrentAnt);
        }
        indOfCurrentAnt++;
        
        //cout << "ant moved " << currentAnt.y << " " << currentAnt.x << endl;
    }
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0 ; j < cols ; j++) {
            for (int k = 0 ; k < 8 ; k++) {
                pheromones[i][j].strength[k] = pheromones[i][j].strength[k]*19/20;
            }
        }
    }

}
int main() {
    rng = std::mt19937( rd() );
    cout << "Enter rows and cols -> ";
    cin >> rows >> cols;
    pheromones.resize(rows, vector<Pheromone>(cols, Pheromone()));
    pheromonesToColony.resize(rows, vector<Pheromone>(cols, Pheromone()));
    density.resize(rows, vector<int>(cols, 0));
    datas.resize(maxTime/dataInterval);
    blocks.resize(rows, vector<bool>(cols, false));
    for (int i = 0; i < rows; i++){
        for (int j = 0 ; j < cols ; j++) {
            char c;
            cin >> c;
            if (c == '#') {
                blocks[i][j] = true;
            }
        }
    }

    foodX = foodY = 0;
    sourceX = cols-1;
    sourceY = rows-1;
    for (int i = 0 ; i < initialAnts ; i++) {
        ants.push_back(Ant(sourceX, sourceY));
        ants.back().currentDirection = randomNum(8);
    }
    int time = 1;
    int current = 0;
    while ( time < maxTime ) {
        simulate();
        cout << endl;
        cout << time << "th frame" << endl;
        dump();
        cout << endl;
        
        char c;
        time++;
            //cout << time << " " << timePerWave << endl;
        if (time % timePerWave == 0) {
            //cout << "new ants" << endl;
            for (int i = 0 ; i < antsPerWave ; i++) {
                ants.push_back(Ant(sourceX, sourceY));
            }
        }
        for (int i = 0 ; i < toRemove.size() ; i++) {
            cout << "removal of " << toRemove[i] << endl;
            ants.erase(ants.begin() + (toRemove[i] - i));
            ants.push_back(Ant(sourceX, sourceY));
        }
        toRemove.clear();
        if (time % dataInterval == 0) {
            datas[current] = {foodInColony, totalPathLength/paths};
            current++;
        }

        
        //cout << time << endl;
        //cin.get();
    }
    for (int i = 0; i < datas.size() ; i++) {
        cout << datas[i].first << " ";
    }
    cout << endl;
    for (int i = 0; i < datas.size() ; i++) {
        cout << datas[i].second << " ";
    }
    cout << endl;
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0;  j < cols ; j++) {
            cout << density[i][j] << ", ";
        }
        cout << endl;
    }

    cout << endl;
    for (int i = 0 ; i < rows ; i++) {
        for (int j = 0; j < cols ;j++) {
            cout << blocks[i][j] << " ";
        }
        cout << endl;
    }
}
 
#endif /* __cplusplus */
 

