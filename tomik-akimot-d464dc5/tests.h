#include <cxxtest/TestSuite.h>
#include "utils.h"
#include "board.h"
#include "old_board.h"
#include "hash.h"
#include "uct.h"

#define TEST_DIR "./test"
#define START_POS "./data/startpos.txt"
#define INIT_TEST_DIR "./data/init/"
#define INIT_TEST_LIST "./data/init/list.txt"
#define RABBITS_TEST_DIR "./data/rabbits/"
#define RABBITS_TEST_LIST "./data/rabbits/list.txt"
#define TRAPCHECK_TEST_DIR "./data/trapcheck/"
#define TRAPCHECK_TEST_LIST "./data/trapcheck/list.txt"
#define STEP_KILL_PRINT_TEST_LIST "./data/step_kill_print/list.txt"
#define STEP_KILL_PRINT_TEST_DIR "./data/step_kill_print/"
#define HASH_TABLE_INSERTS 100

//mature level is defined in uct.h
#define UCT_TREE_TEST_MATURE 1 
#define UCT_TREE_NODES 100000
#define UCT_TREE_NODES_DELETE 100
#define UCT_TREE_MAX_CHILDREN 30

class FileRead;

//#define VERBOSE_TESTS
#ifdef VERBOSE_TESTS
  #define DEBUG_TESTS(x) x
#else
  #define DEBUG_TESTS(x) ((void) 0)
#endif 



class DebugTestSuite : public CxxTest::TestSuite 
{
  public:

    void setUp() { 
      cfg.loadFromFile(string(DEFAULT_CFG));
      globalStructuresInit();
    }

    /*
     * Tests consistency of normal board with bitboard.
     */ 
    void testBitboardConsistency(void)
    {
        StepArray s; 
        uint slen;
        StepArray bs; 
        uint bslen;
        bool c;
        bool bc;
        Step step;
        Step bstep;
        bool w;
        bool bw;
      
        for (int k = 0; k < 100; k++){
          Board * bb = new Board();
          OB_Board * b = new OB_Board(); 
          assert(bb->initFromPosition(START_POS));
          assert(b->initFromPosition(START_POS));
          assert(b->getSignature() == bb->getSignature());

          do { 

            do {
              slen = b->generateAllSteps(b->getPlayerToMove(), s);
              bslen = bb->genSteps(bb->getPlayerToMove(), bs);
              
             /* DEBUG_TESTS(
                cerr << "=============================";
                cerr << b->toString();
                cerr << slen;
                for (int i = 0; i < slen; i++){
                  cerr << s[i].toString() << " ";
                }
                cerr << endl;
                cerr << bb->toString();
                cerr << bslen;
                for (int i = 0; i < bslen; i++){
                  cerr << bs[i].toString() << " ";
                }
                  cerr << endl;
              );  
            */
                
              assert(slen == bslen); 
              if (slen == 0)
                break;
              int index = rand() % slen;
              int bindex = 0; 
              for (int i = 0; i < slen; i++){
                bool found = false;
                for (int j = 0; j < bslen; j++){
                  if (s[i].toNew() == bs[j]){
                    found = true;
                    if (i == index)
                      bindex = j;
                    break;
                  } 
                }
              }
              c = b->makeStepTryCommitMove(s[index]);
              bc = bb->makeStepTryCommit(bs[bindex]);
              assert(b->getSignature() == bb->getSignature());
              assert(c == bc); 
            }
            while (! c || slen == 0);
            if (slen == 0)
              break;

            w = b->gameOver();
            bw = bb->gameOver();
            assert(w == bw);
          
          } while (! w);
          delete b;
          delete bb;
        }
 
    } 


    void testRandom(void){

      #define SAMPLE 100000000
      srand((unsigned) time(NULL));
      Grand* g = new Grand(rand());
      float avg = 0;
      for (int i = 1; i < SAMPLE; i++){
        float f =g->get01();
        TS_ASSERT(f <= 1 && f >= 0);
        avg += (f - avg)/++i;
      }
      TS_ASSERT(avg > 0.4 && avg < 0.6);
    }

    /**
     * Bit stuff. 
     */
    void testBits(void)
    {
      using namespace bits;

      u64 v = u64(0);
      assert(lix(v) == -1);
      v = u64(1);
      assert(lix(v) == 0);
      v = u64(17);
      assert(lix(v) == 4 && lix(v) == 0 && lix(v) == -1);
      v = u64(0xff00000000000000ULL);
      assert(lixslow(v) == 63); 
      v = u64(0xff00000000000000ULL);
      assert(lix(v) == 63); 

      assert((str2bits(string("111")) == 7));
      assert((str2bits(string("0")) == 0));
      assert((str2bits(string("110010")) == 50));
    }

    /**
     * Utilities test - mostly string functions. 
     */
    void testUtils(void)
    {
      string s;

      s = " x ";
      TS_ASSERT_EQUALS(trimRight(trimLeft(s)), "x");

      s = "    x  ";
      TS_ASSERT_EQUALS(trimRight(trimLeft(s)), "x");

      stringstream ss("ab cd");
      ss >> s;
      TS_ASSERT_EQUALS(getStreamRest(ss), "cd");

      ss.clear();
      ss.str("makemove Ra1");
      ss >> s;
      TS_ASSERT_EQUALS(getStreamRest(ss), "Ra1");
    }

    /**
     * Board init test.
     *
     * Inits two boards from game record file and position file
     * and tests their equality.
     */
     void testBoardInit(void)
     {
        Board board1;
        Board board2;
        string fnPosition;
        string fnRecord;
        string s1, s2;
        FileRead* f = new FileRead(string(INIT_TEST_LIST));

        while (f->getLineAsPair(s1, s2)){
          fnPosition = INIT_TEST_DIR + s1;
          fnRecord = INIT_TEST_DIR + s2;

          if (! board1.initFromPosition(fnPosition.c_str()))
            TS_FAIL((string) "Init from " + fnPosition + (string) " failed");
          
          if (! board2.initFromRecord(fnRecord.c_str()))
            TS_FAIL((string) "Init from " + fnRecord + (string) " failed");

          TS_ASSERT_EQUALS( board1, board2);
        }
     }

    /**
     * Test step with kills print. 
     */
    void testStepWithKills(void)
    {
      string s1, s2, s3, s;

      FileRead* f = new FileRead(string(STEP_KILL_PRINT_TEST_LIST));
      f->ignoreLines("#");
      while (f->getLine(s)){
        stringstream ss(s);
        ss >> s1; ss >> s2; ss >> s3;

        s2 = replaceAllChars(s2,'_', ' ');
        s3 = replaceAllChars(s3,'_', ' ');

        string fn = string(STEP_KILL_PRINT_TEST_DIR) + s1;
        Board* b = new Board();
        b->initFromPosition(fn.c_str());
        
        TS_ASSERT_EQUALS(b->moveToStringWithKills(s2), s3);
        delete b;
      }
    }

    /**
     * Test for distance macro defined in [old]board.h. 
     */
    void testDistanceMacro(void)
    {
      //old_board
      TS_ASSERT_EQUALS(OB_SQUARE_DISTANCE(11, 12), 1);
      TS_ASSERT_EQUALS(OB_SQUARE_DISTANCE(11, 21), 1);
      TS_ASSERT_EQUALS(OB_SQUARE_DISTANCE(21, 18), 8);
      //diagonal trap distance
      TS_ASSERT_EQUALS(OB_SQUARE_DISTANCE(33, 66), 6);
      //neighbour trap distance
      TS_ASSERT_EQUALS(OB_SQUARE_DISTANCE(33, 63), 3);
      
      //board
      TS_ASSERT_EQUALS(SQUARE_DISTANCE(0, 2), 2);
      TS_ASSERT_EQUALS(SQUARE_DISTANCE(18, 21), 3);
      TS_ASSERT_EQUALS(SQUARE_DISTANCE(42, 18), 3);
    }

    /**
     * Hash table insert/hasItem/load test.
     */
    void testHashTable(void)
    {
      HashTable<int> hashTable; 
      u64 keys[HASH_TABLE_INSERTS];

      //store some random values into hash table
      for (int i = 0; i < HASH_TABLE_INSERTS; i++){
        keys[i] = getRandomU64();
        hashTable.insertItem(keys[i], i);
      }

      //positive-lookup test
      for (int i = 0; i < HASH_TABLE_INSERTS; i++){
        int item;
        TS_ASSERT_EQUALS(hashTable.hasItem(keys[i]), true); 
        if (! hashTable.loadItem(keys[i], item))
          TS_FAIL("positive-lookup failed");
      }
       
      //negative-lookup test
      for (int i = 0; i < HASH_TABLE_INSERTS; i++){
        int key = getRandomU64();
        int item;
        TS_ASSERT_EQUALS(hashTable.hasItem(key), false); 
        if (hashTable.loadItem(key, item))
          TS_FAIL("negative-lookup suceeded");
      }
    }

    /**
     * Uct test.
     *
     * Dummy version of Uct::doPlayout. Tests whether the uct tree is consistent 
     * after large number of inserts(UCT_TREE_NODES). Position in the bottom of the 
     * tree is evaluated randomly and randomDescend() is applied instead of uctDescend().  
     */
    void testUct(void)
    {
      //tree with random player in the root
      Tree* tree = new Tree(random() % 2);
      int winValue[2] = {1, -1};
      StepArray steps;
      int stepsNum;
      int i = 0;

      //build the tree in UCT manner
      for (int i = 0; i < UCT_TREE_MAX_CHILDREN; i++)
        steps[i] = Step(STEP_PASS, GOLD);

      while (true) {
        tree->historyReset();     //point tree's actNode to the root 
        while (true) { 
          if (! tree->actNode()->hasChildren()) { 
              stepsNum = (rand() % UCT_TREE_MAX_CHILDREN) + 1;
              tree->expandNode(tree->actNode(), steps, stepsNum);
              i++;
              break;
          }
          tree->randomDescend(); 
        } 
        if (i >= UCT_TREE_NODES)
          break;
      }
    } //testUct


  void testThirdRepetition(void)
  {
    //tree with random player in the root
    Board* board = new Board();
    board->initNewGame();
    thirdRep.update(board->getSignature(), 1 - board->getPlayerToMove());
    thirdRep.update(board->getSignature(), 1 - board->getPlayerToMove());
    TS_ASSERT_EQUALS(thirdRep.isThirdRep(board->getSignature(), 1 - board->getPlayerToMove()), true);
  }


  /**
   * Quick and Full Goal check. 
   */
  void testGoalChecks(void)
  {
    string s1, s2, s3, s;

    FileRead* f = new FileRead(string(RABBITS_TEST_LIST));
    f->ignoreLines("#");
    while (f->getLine(s)){
      stringstream ss(s);
      ss >> s1; ss >> s2; ss >> s3;
      bool quick_expected = bool(str2int(s2));
      bool full_expected = bool(str2int(s3));

      string fn = string(RABBITS_TEST_DIR) + s1;
      Board* b = new Board();
      b->initFromPosition(fn.c_str());
      Move move;
      assert(full_expected == b->goalCheck(b->getPlayerToMove(), STEPS_IN_MOVE));
      delete b;
          
    }

  }

  /**
   * Trap checking.
   */
  void testTrapCheck(void)
  {
    string s1, s;

    FileRead* f = new FileRead(string(TRAPCHECK_TEST_LIST));
    f->ignoreLines("#");
    while (f->getLine(s)){
      stringstream ss(s);
      ss >> s1; 
      string fn = string(TRAPCHECK_TEST_DIR) + s1;
      Board* b = new Board();
      b->initFromPosition(fn.c_str());

      DEBUG_TESTS(
        cerr << b->toString();
        cerr << getStreamRest(ss) << endl;
      );

      SoldierList trapableGold;
      SoldierList trapableSilver;
      SoldierList trapable;
      b->trapCheck(GOLD, NULL, &trapableGold);
      b->trapCheck(SILVER, NULL, &trapableSilver);
      trapable = trapableGold;
      trapable.splice(trapable.end(), trapableSilver);
      Move move;

      DEBUG_TESTS(
        for (SoldierList::iterator it = trapable.begin(); it != trapable.end(); it++) {
          cerr << "piece " << it->toString() << " is trapable " << endl;
        }
      );

      set<string> t1;
      set<string> t2;

      while (ss.good()) { 
        string pieceStr;
        ss >> pieceStr;    
        if (pieceStr == ""){
          continue;
        }
        t1.insert(trim(pieceStr));
        int steps_num;
        ss >> steps_num;
      }

      for (SoldierList::iterator it = trapable.begin(); it != trapable.end(); it++) {
        t2.insert(it->toString());
      }

      /*
      cerr << endl;
      for (set<string>::iterator it = t1.begin(); it != t1.end(); it++) {
        cerr << *it << " ";
      }
      cerr << endl;
      for (set<string>::iterator it = t2.begin(); it != t2.end(); it++) {
        cerr << *it << " ";
      }
      */

      //check bijection
      assert(t1 == t2);

      delete b;
    }

  }

};

