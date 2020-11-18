/*********************************************************************************
 *
 * $Id: score.cpp,v 1.8 1999/01/02 08:09:06 kiecza Exp $
 *
 *********************************************************************************/

#include "score.moc"

#include <stdlib.h>
#include <math.h>

#include <qlist.h>

#include <kapp.h>

#include <iostream>

#define UNENDLISCH 10000000
#define RAND       6

#define MATSIZE  2000

Score::Score()
{
  //int I = MATSIZE;
  //int J = MATSIZE;

  adjustment_width = 70;
  threshold = 15.0;
  min_distance = 2.4;
  
  /*
  DTW_matrix = new float* [I];
  for (int ii = 0; ii < MATSIZE; ii++)
    DTW_matrix[ii] = new float[J];
  */
}


Score::~Score()
{
  /*
  for (int ii = 0; ii < 2000; ii++)
    delete DTW_matrix[ii];
  delete DTW_matrix;
  */
}


float Score::euklid_distance(const float *a, const float *b)
{
  float result=0;
  for (int i = 0; i < 16; i++)
    result += (a[i]-b[i])*(a[i]-b[i]);
  result = sqrt(result);
  return result;
}

QString *Score::branchNbound_score(const Utterance *test, QList<Reference> *ref_list)
{
  QString *recog_result = 0;

  //cerr << "using parallel warping planes for B&B-scoring ..." << endl;

  uint n;
  uint j;
  uint i;

  // ***** we got a total of N sample utterance
  uint N = 0;
  for (uint r=0; r < ref_list->count(); r++)
    N += ref_list->at(r)->count();

  // **** test size
  uint I = test->get_size();

  // ***** generate compact list of utterances
  
  const Utterance **ref_utts;
  ref_utts           = new const Utterance*[N];
  uint *ref_utts_map = new uint[N];
  n = 0;
  for (uint r=0; r < ref_list->count(); r++)
    for (uint rs=0; rs < ref_list->at(r)->count(); rs++)
    { 
      KApplication::flushX();
      kapp->processEvents();

      // ***** ignore those utterances that cannot be aligned due to
      // ***** slope (e [0.5,2]) and adjustment_window constraints
      
      uint J = (uint)ref_list->at(r)->at(rs)->get_size();
      
      if ( ((J-1)*2 + (RAND-1) < I-1)     ||  // min slope is 0.5
	   ((J-RAND)/2 > I-1)             ||  // max slope is 2
	   (J-1 + adjustment_width < I-1) ||  // adjustment window right side
	   (J-1 - adjustment_width > I-1))    // adjustment window left side
	continue;

      ref_utts[n] = ref_list->at(r)->at(rs);
      ref_utts_map[n++] = r;
    }

  N = n;

  // **** set up the needed warping planes

  float ***planes = new float**[N]; // ***** N planes  
  for (n = 0 ; n < N ; n++)
  {
    KApplication::flushX();
    kapp->processEvents();

    planes[n] = new float*[I]; // ***** width of each plane = I

    for (i = 0 ; i < I ; i++)
      planes[n][i] = new float[ref_utts[n]->get_size()];
  }
  
  for (n = 0 ; n < N ; n++)
  {
    for (i = 0 ; i < I ; i++)
      for (j = 0 ; j < (uint)ref_utts[n]->get_size() ; j++)
	planes[n][i][j] = UNENDLISCH;
  }

  KApplication::flushX();
  kapp->processEvents();

  // ***** calculate the special entries (lower left corner) for all planes

  float act_dist;
  
  for (n = 0 ; n < N ; n++) // ***** for all planes
  {
    planes[n][0][0] = 2*euklid_distance((*test)[0], (*ref_utts[n])[0]);
    
    for (j = 1; j < RAND; j++)
      planes[n][0][j] = planes[n][0][j-1]+euklid_distance((*test)[0], (*ref_utts[n])[j]);
    for (i = 1; i < RAND; i++)
      planes[n][i][0] = planes[n][i-1][0]+euklid_distance((*test)[i], (*ref_utts[n])[0]);
    
    act_dist = euklid_distance((*test)[1], (*ref_utts[n])[1]);
    planes[n][1][1] = MIN3(planes[n][1][0]+act_dist,
			   planes[n][0][0]+2*act_dist,
			   planes[n][0][1]+act_dist);
    
    for (j = 2; j < RAND+1; j++)
    {
      act_dist = euklid_distance((*test)[1], (*ref_utts[n])[j]);
      planes[n][1][j] = MIN3(planes[n][0][j]+act_dist,
			     planes[n][0][j-1]+2*act_dist,
			     planes[n][0][j-2]+2*euklid_distance((*test)[1], (*ref_utts[n])[j-1])+act_dist);
    }
    for (i = 2; i < RAND+1; i++)
    {
      act_dist = euklid_distance((*test)[i], (*ref_utts[n])[1]);
      planes[n][i][1] = MIN3(planes[n][i][0]+act_dist,
			     planes[n][i-1][0]+2*act_dist,
			     planes[n][i-2][0]+2*euklid_distance((*test)[i-1], (*ref_utts[n])[1])+act_dist);
    }
    
    // ***** force processing of nonhandled events (LED)
    KApplication::flushX();
    kapp->processEvents();
  }
  
  // ***** get minimum of each column for each plane
  // ***** and sort the planes by increasing minimum
  
  QList<BBQueueItem> bb_queue;
  bb_queue.setAutoDelete( FALSE );
  for (n = 0 ; n < N ; n++)
  {
    KApplication::flushX();
    kapp->processEvents();

    // ***** calc min score

    float minimum = UNENDLISCH;
    for (j = 0 ; j < (uint)ref_utts[n]->get_size() ; j++)
      if (minimum > planes[n][0][j]/((j+1)+(0+1)))
	minimum = planes[n][0][j]/((j+1)+(0+1));

    // ***** find place in queue
    uint index=0;
    while (index < bb_queue.count() &&
	   bb_queue.at(index)->score()<=minimum)
      index++;
    // ***** insert new item there
    bb_queue.insert( index, new BBQueueItem(minimum, n, 2) );
  }

  BBQueueItem *item = 0;
  uint nbest = 2;
  uint *nbest_ref_index = new uint[nbest];
  float *nbest_ref_score = new float[nbest];
  uint nbestfound = 0;

  while (nbestfound<nbest && !bb_queue.isEmpty())
  {
    KApplication::flushX();
    kapp->processEvents();

    // ***** remove first (minimum score) element from list
    
    item = bb_queue.getFirst();
    bb_queue.remove(item);

    // ***** if (at right corner) -> best item found
    // *****   (decrease N-best counter, if ==0, stop)

    if (item->pos() == I)
    {
      nbest_ref_index[nbestfound] = ref_utts_map[item->plane_index()];
      // ***** just take min-score (item->score()) ?!
      nbest_ref_score[nbestfound] = item->score();

      // **** get "real score", not necessary ..... !! saves time!!
      //uint n_tmp = item->plane_index();      
      //nbest_ref_score[nbestfound] = planes[n_tmp][I-1][(uint)ref_utts[n_tmp]->get_size()];
      //nbest_ref_score[nbestfound] = planes[n_tmp][I-1][(uint)ref_utts[n_tmp]->get_size()]/(I+"J"));
      //cerr << ref_list->at(nbest_ref_index[nbestfound])->get_name() << ", " << nbest_ref_score[nbestfound] << endl;

      nbestfound++;
      delete item;
    }
    else
    {
      // ***** else expand by one column ...

      n = item->plane_index();
      i = item->pos();
      uint J = (uint)ref_utts[n]->get_size();

      for (j = (uint)MAX2(2,(int)i-adjustment_width); j < MIN2(J,i+adjustment_width); j++)
      {
	float act_dist =  euklid_distance((*test)[i], (*ref_utts[n])[j]);
	
	if (planes[n][i-1][j-1] < UNENDLISCH ||
	    planes[n][i-1][j-2] < UNENDLISCH ||
	    planes[n][i-2][j-1] < UNENDLISCH)
	  planes[n][i][j] = MIN3(planes[n][i-1][j-1]+2*act_dist,
				  planes[n][i-1][j-2]+2*euklid_distance((*test)[i], (*ref_utts[n])[j-1])+act_dist,
				  planes[n][i-2][j-1]+2*euklid_distance((*test)[i-1], (*ref_utts[n])[j])+act_dist);
	else
	  planes[n][i][j] = UNENDLISCH;
      }

      // ***** ... and reinsert into sorted list
      
      float minimum = UNENDLISCH;
      for (j = 0 ; j < J ; j++)
	if (minimum > planes[n][i][j]/((j+1)+(i+1)))
	  minimum = planes[n][i][j]/((j+1)+(i+1));
      
      if (minimum > threshold)
      {
	//cerr << "oops, " << minimum << " even " << (nbestfound+1) << "-best above threshold!" << endl;
	break;
      }
      
      // ***** find place in queue
      uint index=0;
      while (index < bb_queue.count() &&
	     bb_queue.at(index)->score()<=minimum)
	index++;
      // ***** insert new item there
      item->set_score(minimum);
      item->step_right();
      bb_queue.insert( index, item );
    }
  }

  //cerr << "1. " << nbest_ref_index[0] << endl;
  //cerr << "2. " << nbest_ref_index[1] << endl;

  if (nbestfound>=nbest &&
      (nbest_ref_index[0] == nbest_ref_index[1] || (nbest_ref_score[1]-nbest_ref_score[0])>=min_distance))
    recog_result = new QString(ref_list->at(nbest_ref_index[0])->get_command());
  else
    recog_result = 0;

  KApplication::flushX();
  kapp->processEvents();

  // ***** free memory

  for (n = 0 ; n < N ; n++)
  {
    for (i = 0 ; i < I ; i++)
      delete planes[n][i];
    delete planes[n];
  }
  delete planes;
  
  delete ref_utts;
  delete ref_utts_map;

  delete nbest_ref_index;
  delete nbest_ref_score;

  for ( item=bb_queue.first(); item != 0; item=bb_queue.next() )
    delete item;
  
  return recog_result;
}



QString *Score::score(const Utterance *test, QList<Reference> *ref_list)
{
  QString *recog_result = 0;
  
  //cerr << "please wait ... recognition in progress" << endl;

  QString hypo;
  QString command;
  float best_score = 1000000;
  float secondbest_score = 1000000;

  QString hypo2;

  // ***** test every utterance

  for (uint i=0; i < ref_list->count(); i++)
  {
    Reference *r = ref_list->at(i);
   
    // ***** test every sampleUtterance

    for (uint j=0; j < r->count(); j++)
    {
      float score_act = score(test, r->at(j));

      //cerr << "    - scoreAct: " << score_act << endl;

      if (score_act < best_score)
      {
	secondbest_score = best_score;

	hypo2 = hypo.copy();
	best_score = score_act;
	hypo = r->get_name();
	command = r->get_command();
      }
      else if (score_act < secondbest_score)
      {
	secondbest_score = score_act;
	hypo2 = r->get_name();
      }
    }
  }

  //cerr << "  1best: " << best_score << endl;
  //cerr << "  2best: " << secondbest_score << endl;
  //cerr << "  Hypo1: " << hypo << endl;
  //cerr << "  Hypo2: " << hypo2 << endl;

  if (best_score > threshold)
  {
    recog_result = 0;
    //cerr << "RESULT: Nothing matched !!!!!" << endl;
  }
  else if (hypo == hypo2)
  {
    recog_result = new QString(command);
    //cerr << "RESULT: OK -> " << hypo << endl;
    //KApplication::flushX();
    //kapp->processEvents();
  }
  else
  {
    float dist = secondbest_score - best_score;

    if (dist < min_distance)
    {
      //cerr << "RESULT: different first and second hypo have too similar scores (" << dist << ")" << endl;
      recog_result = 0;
    }
    else
    {
      recog_result = new QString(command);
      //cerr << "RESULT: OK (" << dist << ") -> " << hypo << endl;
    }
  }

  return recog_result;
}


float Score::score(const Utterance *test, const Utterance *ref)
{
  // ***** calculate DTW-distance between test and ref ...

  int i = 0; int I = test->get_size();
  int j = 0; int J = ref->get_size();
  
  // ***** force processing of nonhandled events (LED)
  KApplication::flushX();
  kapp->processEvents();

  // ***** init DTW matrix ...

  for (int ii = 0; ii < I; ii++)
    for (int jj = 0; jj < J; jj++)
      DTW_matrix[ii][jj] = UNENDLISCH;

  // ***** now calculate the DTW matrix using symmetric Sahoe&Chiba warp

  float act_dist;
  
  // ***** matrix border

  DTW_matrix[0][0] = 2*euklid_distance((*test)[0], (*ref)[0]);

  for (j = 1; j < RAND; j++)
    DTW_matrix[0][j] = DTW_matrix[0][j-1]+euklid_distance((*test)[0], (*ref)[j]);
  for (i = 1; i < RAND; i++)
    DTW_matrix[i][0] = DTW_matrix[i-1][0]+euklid_distance((*test)[i], (*ref)[0]);

  // ***** force processing of nonhandled events (LED)
  KApplication::flushX();
  kapp->processEvents();

  act_dist = euklid_distance((*test)[1], (*ref)[1]);
  DTW_matrix[1][1] = MIN3(DTW_matrix[1][0]+act_dist,
			  DTW_matrix[0][0]+2*act_dist,
			  DTW_matrix[0][1]+act_dist);

  for (j = 2; j < RAND+1; j++)
  {
    act_dist = euklid_distance((*test)[1], (*ref)[j]);
    DTW_matrix[1][j] = MIN3(DTW_matrix[0][j]+act_dist,
			    DTW_matrix[0][j-1]+2*act_dist,
			    DTW_matrix[0][j-2]+2*euklid_distance((*test)[1], (*ref)[j-1])+act_dist);
  }
  for (i = 2; i < RAND+1; i++)
  {
    act_dist = euklid_distance((*test)[i], (*ref)[1]);
    DTW_matrix[i][1] = MIN3(DTW_matrix[i][0]+act_dist,
			    DTW_matrix[i-1][0]+2*act_dist,
			    DTW_matrix[i-2][0]+2*euklid_distance((*test)[i-1], (*ref)[1])+act_dist);
  }
  
  // ***** force processing of nonhandled events (LED)
  KApplication::flushX();
  kapp->processEvents();

  // ***** second to last column

  float min = UNENDLISCH;
      
  for (i = 2; i < I; i++)
  {
    //for (j = 2; j < J; j++)
    for (j = MAX2(2,i-adjustment_width); j < MIN2(J,i+adjustment_width); j++)
    {
      float act_dist =  euklid_distance((*test)[i], (*ref)[j]);

      if (DTW_matrix[i-1][j-1] < UNENDLISCH ||
	  DTW_matrix[i-1][j-2] < UNENDLISCH ||
	  DTW_matrix[i-2][j-1] < UNENDLISCH)
	DTW_matrix[i][j] = MIN3(DTW_matrix[i-1][j-1]+2*act_dist,
 			        DTW_matrix[i-1][j-2]+2*euklid_distance((*test)[i], (*ref)[j-1])+act_dist,
			        DTW_matrix[i-2][j-1]+2*euklid_distance((*test)[i-1], (*ref)[j])+act_dist);
      else
	DTW_matrix[i][j] = UNENDLISCH;
    }

    // **** check threshold

    for (j = 0 ; j < J ; j++)
      if (min > DTW_matrix[i][j]/(i+1+j+1))
    	min = DTW_matrix[i][j]/(i+1+j+1);
    if (min > threshold)
      return((float)UNENDLISCH);
  }
  
  // ***** force processing of nonhandled events (LED)
  KApplication::flushX();
  kapp->processEvents();

  return(DTW_matrix[I-1][J-1]/(I+J));
}

