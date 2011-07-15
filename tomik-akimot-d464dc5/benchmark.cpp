#include "benchmark.h"
#include "old_board.h"

//---------------------------------------------------------------------
//  section Benchmark
//---------------------------------------------------------------------- 

Benchmark::Benchmark()
{
  board_ = new Board();
  board_->initNewGame(); 
  //TODO REPLACE LOADING FROM FILE WITH FUNCTION  
  board_->initFromPosition(START_POS_PATH);
  timer.setTimer(SEC_ONE);
}

//--------------------------------------------------------------------- 

Benchmark::Benchmark(Board* board, uint playoutCount)
{
	board_ = board;	
}


//--------------------------------------------------------------------- 

void Benchmark::benchmarkEval() 
{
  Eval eval; 
  float timeTotal;

  timer.start();

  int i = 0;
  while (! timer.timeUp()){
    i++; 
    eval.evaluate(board_);
  }

  timer.stop();
	timeTotal = timer.elapsed(); 

  logRaw("Evaluation performance: \n  %d evals\n  %3.2f seconds\n  %d eps\n", 
            i, timeTotal, int ( float(i) / timeTotal));
}

//--------------------------------------------------------------------- 

void Benchmark::benchmarkCopyBoard() 
{
  float	timeTotal;
  timer.start();

  int i = 0;
  while (! timer.timeUp()){
    i++;
    Board *playBoard = new Board(*board_);
    delete playBoard;
  }

  timer.stop();
	timeTotal = timer.elapsed(); 
  logRaw("Board copy performance: \n  %d copies\n  %3.2f seconds\n  %d cps\n", 
            i, timeTotal, int ( float(i) / timeTotal));
  
}

//--------------------------------------------------------------------- 

void Benchmark::benchmarkOldPlayout() 
{

  float timeTotal;

	uint playoutAvgLen = 0;

  timer.start();

  OB_Board * b = new OB_Board();
  b->initFromPosition(START_POS_PATH);

  int i = 0;
  while (! timer.timeUp()){
    i++;
    OB_Board *playBoard = new OB_Board(*b);

    OB_SimplePlayout sp(playBoard, PLAYOUT_DEPTH, 0);
    sp.doPlayout ();

		playoutAvgLen += sp.getPlayoutLength(); 
    delete playBoard;
  }

  timer.stop();
	timeTotal = timer.elapsed(); 
  logRaw("Old playouts performance: \n  %d playouts\n  %3.2f seconds\n  %d pps\n  %d average playout length\n", 
            i, timeTotal, int ( float(i) / timeTotal),int(playoutAvgLen/ float (i)));
  
}

//--------------------------------------------------------------------- 


void Benchmark::benchmarkPlayout() 
{
		
  float timeTotal;

  playoutStatus_e  playoutStatus;
	uint playoutAvgLen = 0;

  timer.start();

  int i = 0;
  while (! timer.timeUp()){
    i++;
    Board *playBoard = new Board(*board_);

    SimplePlayout simplePlayout(playBoard, PLAYOUT_DEPTH, 0);
    playoutStatus = simplePlayout.doPlayout ();

	playoutAvgLen += simplePlayout.getPlayoutLength(); 
    delete playBoard;
  }

  timer.stop();
	timeTotal = timer.elapsed(); 
  logRaw("Playouts performance: \n  %d playouts\n  %3.2f seconds\n  %d pps\n  %d average playout length\n", 
            i, timeTotal, int ( float(i) / timeTotal),int(playoutAvgLen/ float (i)));
  
}

//--------------------------------------------------------------------- 

void Benchmark::benchmarkUct() 
{
  //tree with random player in the root
  Tree* tree = new Tree(random() % 2);
  int winValue[2] = {1, -1};
  StepArray steps;
  int stepsNum;
  int i = 0;
  int walks = 0;

  float timeTotal;
 
  timer.start();

  stepsNum = UCT_CHILDREN_NUM;
  for (int i = 0; i < UCT_CHILDREN_NUM; i++)
    steps[i] = Step(STEP_PASS, GOLD);

  while (true) {
    walks++;
    if ( timer.timeUp() )
      break;
    tree->historyReset();     //point tree's actNode to the root 
    while (true) { 
      if (! tree->actNode()->hasChildren()) { 
        if (tree->actNode()->getDepth() < UCT_MAX_DEPTH - 1) {
          if (tree->actNode()->getVisits() > UCT_NODE_MATURE) {
            tree->expandNode(tree->actNode(), steps, stepsNum);
            i++;
            continue;
          }
        }
        tree->updateHistory(winValue[(random() % 2)]);
        break;
      }
      //tree->uctDescend(); 
      tree->randomDescend();
    } 
  }

  timer.stop();
  timeTotal = timer.elapsed();
  logRaw("Uct performance: \n  %d walks\n  %3.2f seconds\n  %d wps\n  %d nodes\n", 
            walks, timeTotal, int ( float(walks) / timeTotal), i);

}

//--------------------------------------------------------------------- 

void Benchmark::benchmarkSearch() const
{
  Engine* engine = new Engine();
  Board *playBoard = new Board(*board_);

  engine->doSearch(playBoard);
  logRaw(engine->getStats().c_str());
}

//--------------------------------------------------------------------- 

void Benchmark::benchmarkAll() 
{
  benchmarkCopyBoard();
  benchmarkEval();
  benchmarkOldPlayout();
  benchmarkPlayout();
  benchmarkUct();
  benchmarkSearch();
  
}

//--------------------------------------------------------------------- 
//--------------------------------------------------------------------- 

