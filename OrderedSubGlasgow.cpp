#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <igraph.h>
#include <sys/time.h>
using namespace std;
int create_data_graph(char DataGraphPath[], igraph_t *data_graph);
int create_query_graph(char QueryGraphPath[], igraph_t *query_graph);
int CandVerify(igraph_integer_t *res, igraph_vector_int_t *v,igraph_t *data_graph, igraph_t *query_graph);
int PivotVertexSelection(igraph_t *data_graph,igraph_t *query_graph, long int *pivot_node, long int *pivot_node_eccentricity,igraph_vector_int_t *pivot_node_match);
long int RegionFindindAndSISolver(igraph_t *data_graph,igraph_t *query_graph,long int *pivot_node, long int *pivot_node_eccentricity,igraph_vector_int_t *v, char timeout[]);
int igraph_write_graph_edgelist_redefined(const igraph_t *graph, FILE *outstream);
void print_int_vector(igraph_vector_int_t *v, FILE *f){
	long int i;
	for (i=0; i<igraph_vector_int_size(v); i++) {
		fprintf(f, " %li", (long int) VECTOR(*v)[i]);
	}
	fprintf(f, "\n");
}
void print_vector(igraph_vector_t *v, FILE *f) {
	long int i;
	for (i=0; i<igraph_vector_size(v); i++) {
		fprintf(f, " %li", (long int) VECTOR(*v)[i]);
	}
	fprintf(f, "\n");
}
int comp(void *extra, const void *a, const void *b) {
	igraph_vector_t *v=(igraph_vector_t*) extra;
	int *aa=(int*) a;
	int *bb=(int*) b;
	igraph_real_t aaa=VECTOR(*v)[*aa];
	igraph_real_t bbb=VECTOR(*v)[*bb];
  
	if (aaa < bbb) { 
		return -1;
	} else if (aaa > bbb) { 
		return 1;
	}
  
	return 0;
}
int input_file_format_vertexlabelledlad(const igraph_t *graph, FILE *outstream){
	fprintf(outstream, "%d\n",(int)igraph_vcount(graph));
	for(int i=0;i<igraph_vcount(graph);i++){
		igraph_vector_t vertexNeighbor;
		igraph_vector_init(&vertexNeighbor, 0);
		igraph_neighbors(graph, &vertexNeighbor, i, IGRAPH_IN);
		fprintf(outstream, "%ld %ld",(long int)VAN(graph, "label", i), (long int)igraph_vector_size(&vertexNeighbor));
		print_vector(&vertexNeighbor, outstream);
	}
}
float timedifference_msec(struct timeval t0, struct timeval t1)
{
	return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}
int main(int argc, char** argv) 
{ 
	struct timeval t0;
  	struct timeval t1;
  	float elapsed_msec=0;	
	long int pivot_node,pivot_node_eccentricity;
	igraph_vector_int_t pivot_node_match;
	
	/* turn on attribute handling */
 	igraph_i_set_attribute_table(&igraph_cattribute_table);
	igraph_t data_graph,query_graph;

	create_data_graph( argv[2], &data_graph);
	create_query_graph(argv[1], &query_graph);
	//printf("number of vertices in data graph  %d\n",(int)igraph_vcount(&data_graph));
	//printf("number of edges in data graph  %d\n",(int)igraph_ecount(&data_graph));
	//printf("number of vertices in query graph  %d\n",(int)igraph_vcount(&query_graph));
	//printf("number of edges in query graph  %d\n",(int)igraph_ecount(&query_graph));
	igraph_vector_int_init(&pivot_node_match,0); 
	gettimeofday(&t0, 0);// time start
	PivotVertexSelection(&data_graph,&query_graph,&pivot_node,&pivot_node_eccentricity,&pivot_node_match);
	//printf("Pivot Node in Query Graph %ld\n", pivot_node);
	//printf("Pivot Node's Ecc in Query Graph %ld\n", pivot_node_eccentricity);
	//printf("Total possible match of pivot node %ld\n ", igraph_vector_int_size(&pivot_node_match));print_int_vector(&pivot_node_match, stdout); 
	RegionFindindAndSISolver(&data_graph, &query_graph, &pivot_node, &pivot_node_eccentricity,&pivot_node_match, argv[3]);	
	gettimeofday(&t1, 0);// time end
	elapsed_msec = timedifference_msec(t0, t1);
	printf("Code executed in %f milliseconds.\n", elapsed_msec);
	igraph_destroy(&query_graph);
	igraph_destroy(&data_graph);
    	return 0;
}
long int RegionFindindAndSISolver(igraph_t *data_graph,igraph_t *query_graph,long int *pivot_node, long int *pivot_node_eccentricity,igraph_vector_int_t *v, char timeout[]){
	igraph_vector_ptr_t ptr_region;
	igraph_vector_t visited_node, CandidateRegionSize;
	igraph_vector_init(&CandidateRegionSize, 0);
	for(int i=0; i<igraph_vector_int_size(v); i++){//Varies over Candidate Regions.
		igraph_vector_ptr_init(&ptr_region, 0);
		igraph_vector_init(&visited_node, 0);
		igraph_neighborhood(data_graph, &ptr_region,igraph_vss_1(VECTOR(*v)[i]),*pivot_node_eccentricity, IGRAPH_ALL);
		//printf("%dth region pivot match %d \n", i,VECTOR(*v)[i]);
		//printf("%dth region size %ld \n ",i, igraph_vector_ptr_size(&ptr_region));
		//VECTOR(ptr_region)[0];
		//print_vector((igraph_vector_t*)VECTOR(ptr_region)[0], stdout); 
		igraph_vector_copy(&visited_node,(igraph_vector_t*)VECTOR(ptr_region)[0]);
		igraph_vector_ptr_clear(&ptr_region);
		//printf("#V(CandidateRegion) %d\n",(int)igraph_vector_size(&visited_node));
		
		//printf("S.No. %d NodeId %d #V(R) %d\n", i, (int )VECTOR(*v)[i], (int)igraph_vector_size(&visited_node));
		igraph_t CandidateRegion;
		igraph_vs_t vs;
		igraph_vs_vector(&vs,&visited_node);
		igraph_induced_subgraph(data_graph, &CandidateRegion, vs, IGRAPH_SUBGRAPH_AUTO);
		//printf("number of vertices in CandidateRegion  %d\n",(int)igraph_vcount(&CandidateRegion));
		//printf("number of edges in CandidateRegion  %d\n",(int)igraph_ecount(&CandidateRegion));
		igraph_vector_push_back(&CandidateRegionSize, igraph_vcount(&CandidateRegion));
		igraph_destroy(&CandidateRegion);
		igraph_vector_destroy(&visited_node);
		igraph_vs_destroy(&vs);
		
	}
		//printf("CandidateRegion Size ");print_vector(&CandidateRegionSize, stdout); 
		igraph_vector_int_t idx;
		igraph_vector_int_init_seq(&idx, 0, igraph_vector_int_size(v)) ;
		igraph_qsort_r(VECTOR(idx), igraph_vector_int_size(v), sizeof(VECTOR(idx)[0]), (void*) &CandidateRegionSize, comp);
		//cout<<"Sorted Candidate Regions"<<endl;
		//for (int i = 0; i < igraph_vector_int_size(v); i++) {
        	//	printf("Vertex id %d Size %g\n ", (int)VECTOR(*v)[VECTOR(idx)[i]], VECTOR(CandidateRegionSize)[ VECTOR(idx)[i] ]);
    		//}
		igraph_vector_destroy(&CandidateRegionSize);
		static int TotalSolCount=0; 
		static int TotalRuntime=0; 
		for (int i = 0; i < igraph_vector_int_size(v); i++) {//accesssing sorted candidate region
			igraph_vector_ptr_init(&ptr_region, 0);	
			//printf("%dth region pivot match %d \n", i,VECTOR(*v)[VECTOR(idx)[i]]);
			igraph_neighborhood(data_graph, &ptr_region,igraph_vss_1(VECTOR(*v)[VECTOR(idx)[i]]),*pivot_node_eccentricity, IGRAPH_ALL);
			igraph_vector_init(&visited_node, 0);
			igraph_vector_copy(&visited_node,(igraph_vector_t*)VECTOR(ptr_region)[0]);
			igraph_vector_ptr_clear(&ptr_region);
			igraph_t CandidateRegion;
			igraph_vs_t vs;
			igraph_vs_vector(&vs,&visited_node);
			igraph_induced_subgraph(data_graph, &CandidateRegion, vs, IGRAPH_SUBGRAPH_AUTO);
			igraph_vector_destroy(&visited_node);
			
			int CandidateRegionRuntime=0;
			int CandidateRegionSolCount=0;
			char CandidateRegionStatus[10];
			FILE *fp = fopen("CandidateRegionLAD.txt", "w");
			input_file_format_vertexlabelledlad(&CandidateRegion, fp) ;
			fclose(fp);
			//fp = fopen("CandidateRegionIgraph.txt", "w");
			//fprintf(fp,"t # 0\n");	
			//for(int j=0; j<(int)igraph_vcount(&CandidateRegion); j++){
			//	fprintf(fp,"v %d %d\n", j,(int)VAN(&CandidateRegion, "label", j)); 
			//}
			//igraph_write_graph_edgelist_redefined(&CandidateRegion, fp);		
			//fclose(fp);
			//exit(0);
			igraph_destroy(&CandidateRegion);
			string str1="";
			//str1 = str1 + "./glasgow_subgraph_solver --timeout "+timeout+" --count-solutions --induced --format vertexlabelledlad "+ "QueryGraphVertexLAD.txt" + " "+"CandidateRegionLAD.txt";
			str1 = str1 + "./glasgow_subgraph_solver --timeout "+timeout+" --count-solutions --format vertexlabelledlad "+ "QueryGraphVertexLAD.txt" + " "+"CandidateRegionLAD.txt";
			const char *command1 = str1.c_str();
			system(command1);
			fp = fopen("SolutionZaid.txt", "r");
			if(fp != NULL){
				fscanf(fp, "%s %d %d", CandidateRegionStatus, &CandidateRegionSolCount, &CandidateRegionRuntime);
			}	
			fclose(fp);
			TotalSolCount+=CandidateRegionSolCount;
			TotalRuntime+=CandidateRegionRuntime;
			if(TotalSolCount > 1000 || TotalRuntime > atoi(timeout)){//Not counting current Candidate Region
				TotalSolCount-=CandidateRegionSolCount;
				TotalRuntime-=CandidateRegionRuntime;
				break;
			}
		}
		
		cout<<"TotalSolCount "<<TotalSolCount<<", TotalRuntime "<<TotalRuntime<<endl;
		igraph_vector_int_destroy(&idx);
	 
	return 0;
}
int PivotVertexSelection(igraph_t *data_graph,igraph_t *query_graph, long int *pivot_node, long int *pivot_node_eccentricity, igraph_vector_int_t *pivot_node_match){
	igraph_vector_t v,y,query_node_eccentricity,eccentricity;
	igraph_vector_int_t idx;
	igraph_adjlist_t adjlist;	  
	
	igraph_adjlist_init_empty(&adjlist, igraph_vcount(query_graph));
	igraph_vector_init(&v, igraph_vcount(query_graph));
  	igraph_vector_int_init(&idx, igraph_vcount(query_graph)); 
	//int i=157;
	//printf(" %d id = %d label = %d degree = %d \n",i,(int)VAN(data_graph, "id", i),(int)VAN(data_graph, "label", i),(int)VAN(data_graph, "deg", i));
	for (int j=0; j<igraph_vcount(query_graph); j++) {
		//printf("%d %d %d \n",j,(int)VAN(query_graph, "label", j),(int)VAN(query_graph, "deg", j));
		for (int i=0; i<igraph_vcount(data_graph); i++) {
			if(VAN(query_graph, "label", j) == VAN(data_graph, "label", i) && VAN(query_graph, "deg", j) <= VAN(data_graph, "deg", i) && VAN(query_graph, "mnd", j) <= VAN(data_graph, "mnd", i)){
			//if(VAN(query_graph, "label", j) == VAN(data_graph, "label", i) && VAN(query_graph, "deg", j) <= VAN(data_graph, "deg", i)){
				//if(VAN(query_graph, "mnd", j) <= VAN(data_graph, "mnd", i)){break;}
			igraph_vector_int_push_back(igraph_adjlist_get(&adjlist,j), i);//printf(" %d %d %d \n",i,(int)VAN(data_graph, "label", i),(int)VAN(data_graph, "deg", i));
			}
		}
		//printf("%d match in data graph  ",j);
		//print_int_vector(igraph_adjlist_get(&adjlist,j), stdout);
		VECTOR(idx)[j] = j;
		VECTOR(v)[j] = igraph_vector_int_size(igraph_adjlist_get(&adjlist,j));
		if(VECTOR(v)[j]==0){
			//printf("Match not found due to query node %d;",j);
			return 7;
		}
	}
	igraph_qsort_r(VECTOR(idx), igraph_vcount(query_graph), sizeof(VECTOR(idx)[0]), (void*) &v, comp);
	//print_vector(&v, stdout);
	/*for (int i=0; i<igraph_vcount(query_graph); i++) { 
		printf("%d ", VECTOR(idx)[i]);
    		printf("%g ", VECTOR(v)[ VECTOR(idx)[i] ]);
		printf("\n");
 	}*/
	igraph_vector_destroy(&v); 

	igraph_vector_init(&y, 0);
	igraph_vector_init(&v, 0);
	igraph_vector_init(&query_node_eccentricity, 0);
	igraph_real_t turbo_f=0;
	for (int j=0; j< 3; j++) { 
		igraph_vector_init(&eccentricity, 1);//convarify avoided
		//print_int_vector(igraph_adjlist_get(&adjlist,VECTOR(idx)[j]), stdout);
		//printf("size before %ld ", igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])));
		CandVerify(&VECTOR(idx)[j], igraph_adjlist_get(&adjlist,VECTOR(idx)[j]), data_graph, query_graph);
		//printf("after candverify call");
		//printf("size %ld ", igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])));
		//print_int_vector(igraph_adjlist_get(&adjlist,VECTOR(idx)[j]), stdout);
		igraph_eccentricity(query_graph,&eccentricity,igraph_vss_1((igraph_integer_t)VECTOR(idx)[j]), IGRAPH_ALL);
		//we can use bfs to find eccentricity.
		//printf("Ecentricity");
		//print_vector(&eccentricity, stdout);
		//printf("size match %ld \n", igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])));
		//igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])) * VECTOR(eccentricity)[0];
		if(igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])) * VECTOR(eccentricity)[0]==0){
		//if(igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])) / VAN(query_graph, "deg", VECTOR(idx)[j])==0){
			//printf("pivot node not exist;");		
		 	return 8;
		}
		igraph_vector_push_back(&y, igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])) * VECTOR(eccentricity)[0]);
		//igraph_vector_push_back(&y, igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])) / VAN(query_graph, "deg", VECTOR(idx)[j]));//for turbo
		//turbo_f = igraph_vector_int_size(igraph_adjlist_get(&adjlist,VECTOR(idx)[j])) / VAN(query_graph, "deg", VECTOR(idx)[j]) ;
		//printf("deg %f", turbo_f);
		igraph_vector_push_back(&v, VECTOR(idx)[j]);
		igraph_vector_push_back(&query_node_eccentricity, VECTOR(eccentricity)[0]);
		//printf("\n");
		igraph_vector_destroy(&eccentricity);
	}
	//printf("Rank Score");
	//print_vector(&y, stdout);
	//VECTOR(v)[igraph_vector_which_min(&y)];
	//print_vector(&v, stdout);
	 
	*pivot_node=VECTOR(v)[igraph_vector_which_min(&y)];
	*pivot_node_eccentricity=VECTOR(query_node_eccentricity)[igraph_vector_which_min(&y)];
	igraph_vector_int_update(pivot_node_match, igraph_adjlist_get(&adjlist,*pivot_node));
	//printf("Pivot Node in Query Graph %ld\n", *pivot_node);
	//printf("Pivot Node's Ecc in Query Graph %d\n", *pivot_node_eccentricity);
	//printf("Pivot Node's deg in Query Graph %d\n", (int)VAN(query_graph, "deg", *pivot_node));
	//printf("Total possible match of pivot node %ld\n ", igraph_vector_int_size(igraph_adjlist_get(&adjlist,*pivot_node)));
	//printf("id's of matched node of pivot node in data graph");
	//print_int_vector(pivot_node_match, stdout);
	 
 	igraph_adjlist_destroy(&adjlist);
	igraph_vector_destroy(&query_node_eccentricity);
	igraph_vector_int_destroy(&idx);
	igraph_vector_destroy(&y);
  	igraph_vector_destroy(&v); 
	
	return 9;

}
int igraph_write_graph_edgelist_redefined(const igraph_t *graph, FILE *outstream) {

  igraph_eit_t it;
  
  IGRAPH_CHECK(igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_FROM), 
				 &it));
  IGRAPH_FINALLY(igraph_eit_destroy, &it);

  while (!IGRAPH_EIT_END(it)) {
    igraph_integer_t from, to;
    int ret;
    igraph_edge(graph, IGRAPH_EIT_GET(it), &from, &to);
    ret=fprintf(outstream, "e %li %li 0\n", 
		(long int) from,
		(long int) to);
    if (ret < 0) {
      IGRAPH_ERROR("Write error", IGRAPH_EFILE);
    }
    IGRAPH_EIT_NEXT(it);
  }
  
  igraph_eit_destroy(&it);
  IGRAPH_FINALLY_CLEAN(1);
  return 0;
}

int CandVerify(igraph_integer_t *v_q, igraph_vector_int_t *v,igraph_t *data_graph, igraph_t *query_graph){
	igraph_vector_t adj_v_q,adj_v_g, label_adj_v_q, label_adj_v_g,common_label;
	igraph_vector_int_t y;
	igraph_vector_int_init(&y,0);
	igraph_vector_init(&adj_v_q, 0);
	igraph_vector_init(&label_adj_v_q, 0);
	igraph_neighbors(query_graph, &adj_v_q,*v_q, IGRAPH_ALL);
	igraph_cattribute_VANV(query_graph, "label",igraph_vss_vector(&adj_v_q), &label_adj_v_q);
	igraph_vector_destroy(&adj_v_q);
	igraph_vector_sort(&label_adj_v_q);
	//printf("neighbour_query");
	//print_vector(&adj_v_q, stdout);
	//printf("neighbour_label_query");
	//print_vector(&label_adj_v_q, stdout);
	for(int i=0; i<igraph_vector_int_size(v); i++){
		igraph_vector_init(&adj_v_g, 0);
		igraph_vector_init(&label_adj_v_g, 0);
		igraph_vector_init(&common_label, 0);
		igraph_neighbors(data_graph, &adj_v_g,VECTOR(*v)[i], IGRAPH_ALL);
		igraph_cattribute_VANV(data_graph, "label",igraph_vss_vector(&adj_v_g), &label_adj_v_g);igraph_vector_sort(&label_adj_v_g); 
		//printf("neighbour_data");
		//print_vector(&adj_v_g, stdout);
		//printf("neighbour_label_query");
		//print_vector(&label_adj_v_q, stdout);
		//printf("neighbour_label_data");
		//print_vector(&label_adj_v_g, stdout);
		//igraph_vector_intersect_sorted(&label_adj_v_g,&label_adj_v_q,&common_label);
		//printf("Common_label");
		//print_vector(&common_label, stdout);
		igraph_vector_difference_sorted(&label_adj_v_q,&label_adj_v_g,&common_label);
		igraph_vector_difference_sorted(&label_adj_v_q,&common_label, &label_adj_v_g); 
		if(igraph_vector_all_e(&label_adj_v_q,&label_adj_v_g)){		
			//printf("YES");
			//igraph_vector_int_remove(v, i);
			igraph_vector_int_push_back(&y,VECTOR(*v)[i]);
			
		}
		igraph_vector_destroy(&adj_v_g);
		igraph_vector_destroy(&label_adj_v_g);
		igraph_vector_destroy(&common_label);
	}
	igraph_vector_int_update(v,&y);
	//printf("after removal");printf("size %ld \n", igraph_vector_int_size(v));
	//print_int_vector(v, stdout);	
		igraph_vector_destroy(&label_adj_v_q);igraph_vector_int_destroy(&y);
	return 0;
}

int create_query_graph(char QueryGraphPath[], igraph_t *query_graph){
	char line[250];
	char ans;
  	FILE *fpw;
	FILE *fp   ;
	igraph_bool_t res;
	igraph_vector_t y,t_nodeLabels;
	//igraph_vector_init(&y,0);
	igraph_vector_init(&t_nodeLabels,0);
	int lineNo=1;
	char ifile[100];
	
		int noOfNodes = 0;
		fp  = fopen( QueryGraphPath, "r"); 
		if(fp == NULL){
			printf("Error! opening file1");
			return 0;
		}
		fpw = fopen("queryGraphEdgeList.txt", "w");
		if(fpw == NULL){
			printf("Error! opening file2");
			return 0;
	 	}
		while(fgets(line, 255, (FILE*) fp)) {
		lineNo++;
		//printf("\nZubair:%s::" , line);
	  	if(line[0] == 'v'){
			 //geting node labels
		 	 int i;
		 	int j = 0;
		 	int n = strlen(line)-1;
			//printf("\ntest:%c::%d:",line[n], line[n]);
                 	while(line[n] == ' ' || line[n] == 10){
                   		n = n - 1;
                 	}
               
		//printf("\nZaid::%d::", n);
		 i = n ;
		 while(line[i] != ' ')
			 i =  i - 1;
		 i = i + 1;

		 char token[10];

		 //getting query graph number
		 do{
			 token[j] = line[i];
			 j = j + 1;
			 i = i + 1;
		 }while(i <= n);
		 token[j] = '\0';
		 igraph_vector_push_back(&t_nodeLabels,atoi(token));noOfNodes++;
		//printf("\n node = %d label = %d ",noOfNodes-1, t_nodeLabels[noOfNodes-1]);
		//printf("%d",t_nodeLabels[noOfNodes-1]);

	 	}
	 	if(line[0] == 'e'){
		 //geting edge
		 int s, d;
		 int i = 2;
		 int j = 0;
		 char token[6];
		//reading sourse node id
		do{
			token[j] = line[i];
			j = j + 1;
			i = i + 1;
		}while(line[i] != ' ');
		token[j] = '\0';
		s = atoi(token);
		//reading destination node id
		i = i + 1;
		j = 0;
		do{
			token[j] = line[i];
			j = j + 1;
			i = i + 1;
		}while(line[i] != ' ');
		token[j] = '\0';
		d = atoi(token);
         //	printf("%d\t%d\n",d,s);
		fprintf(fpw,"%d %d\n", s, d);
	}
}
		fclose(fp);
		fclose(fpw);
		if ((fpw = fopen("queryGraphEdgeList.txt", "r")) == NULL)
    		{
        		printf("Error! opening file1");
        		// Program exits if file pointer returns NULL.
        		exit(1);         
    		}
		igraph_read_graph_edgelist(query_graph, fpw,0, 0) ;
		fclose(fpw);
		igraph_simplify(query_graph, 1, 1, /*edge_comb=*/ 0);
		igraph_is_connected(query_graph, &res, IGRAPH_WEAK); 
  		if (!res) {
    			printf(" query graph not connected1\n");
    			//return 0;
		}
		//printf("number of vertices in query graph  %d\n",(int)igraph_vcount(query_graph));
		//printf("number of edges in query graph  %d\n",(int)igraph_ecount(query_graph));
		SETGAN(query_graph, "vertices", igraph_vcount(query_graph));
		igraph_vector_init_seq(&y, 0, igraph_vcount(query_graph)-1);
		SETVANV(query_graph, "id", &y);
		igraph_vector_destroy(&y);
		for (int i=0; i<igraph_vcount(query_graph); i++){
			if (VAN(query_graph, "id", i) != i){
	      			return 22;
			}
		}
		//print_vector(&t_nodeLabels, stdout); 
		SETVANV(query_graph, "label", &t_nodeLabels);
		for (int i=0; i<igraph_vcount(query_graph); i++) {
			if (VAN(query_graph, "label", i) !=VECTOR(t_nodeLabels)[i]) {
				//printf("node  %d query label %d \n",i,(int)VAN(&query_graph, "label", i))		      ;
				return 23;
			}
		}
		
		igraph_vector_destroy(&t_nodeLabels);
		igraph_integer_t mnd_v_q;
		for (int i=0; i<igraph_vcount(query_graph); i++){
			igraph_vector_init(&y,0);
			igraph_neighbors(query_graph, &y,i, IGRAPH_ALL);	
			igraph_maxdegree(query_graph, &mnd_v_q,igraph_vss_vector(&y), IGRAPH_ALL,0);	
			SETVAN(query_graph,"mnd",i,mnd_v_q);
			if (VAN(query_graph, "mnd", i) != mnd_v_q){
				//printf("mnd not set");
		      		return 2;
				//printf("node  %d mnd %d \n",i,(int)VAN(&query_graph, "mnd", i));
		    	}
			igraph_vector_destroy(&y);
		}  
		igraph_vector_init(&y, 0);
		igraph_degree(query_graph, &y,igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);
		SETVANV(query_graph, "deg", &y);
		//return 0;
		igraph_vector_destroy(&y);
		fp = fopen("QueryGraphVertexLAD.txt", "w");
		input_file_format_vertexlabelledlad(query_graph, fp) ;
		fclose(fp);
	return 0;
}

int create_data_graph(char DataGraphPath[],igraph_t *data_graph){
/* turn on attribute handling */
 	igraph_i_set_attribute_table(&igraph_cattribute_table);
	char line[250];
	char ans;
  	FILE *fpw;
	FILE *fp   ;
	igraph_bool_t res;
	
	fp  = fopen(DataGraphPath, "r");
	if(fp == NULL){
		printf("Error! opening file 1");
		return 0;
	}
	int lineNo=1;
	igraph_vector_t y,t_nodeLabels;
	//igraph_vector_init(&y,0);
	igraph_vector_init(&t_nodeLabels,0);
	int noOfNodes = 0;
	fpw = fopen("dataGraphEdgeList.txt", "w");
	if(fpw == NULL){
		printf("Error! opening file for writing");
		return 0;
	}
	while(fgets(line, 255, (FILE*) fp)) {
		lineNo++;
	//printf("\nZubair:%s::" , line);
	  if(line[0] == 'v'){
		 //geting node labels
		 int i;
		 int j = 0;
		 int n = strlen(line)-1;
		//printf("\ntest:%c::%d:",line[n], line[n]);
                 while(line[n] == ' ' || line[n] == 10){
                   n = n - 1;
                 }
               
		//printf("\nZaid::%d::", n);
		 i = n ;
		 while(line[i] != ' ')
			 i =  i - 1;
		 i = i + 1;

		 char token[10];

		 //getting query graph number
		 do{
			 token[j] = line[i];
			 j = j + 1;
			 i = i + 1;
		 }while(i <= n);
		 token[j] = '\0';
		igraph_vector_push_back(&t_nodeLabels,atoi(token));noOfNodes++;
		 //VECTOR(t_nodeLabels)[noOfNodes++] = atoi(token);
		//printf("\n node = %d label = %d ",noOfNodes-1, t_nodeLabels[noOfNodes-1]);
		//printf("%d",t_nodeLabels[noOfNodes-1]);

	 }
	 if(line[0] == 'e'){
		 //geting edge
		 int s, d;
		 int i = 2;
		 int j = 0;
		 char token[6];
		//reading sourse node id
		do{
			token[j] = line[i];
			j = j + 1;
			i = i + 1;
		}while(line[i] != ' ');
		token[j] = '\0';
		s = atoi(token);
		//reading destination node id
		i = i + 1;
		j = 0;
		do{
			token[j] = line[i];
			j = j + 1;
			i = i + 1;
		}while(line[i] != ' ');
		token[j] = '\0';
		d = atoi(token);
         //	printf("%d\t%d\n",d,s);
		fprintf(fpw,"%d %d\n", s, d);
	}
}
//exit(0);
		fclose(fp);
		fclose(fpw);
	if ((fpw = fopen("dataGraphEdgeList.txt", "r")) == NULL)
    	{
        printf("Error! opening file1");
        // Program exits if file pointer returns NULL.
        exit(1);         
    	}
	igraph_read_graph_edgelist(data_graph, fpw,0, 0) ;
	//igraph_read_graph_edgelist(data_graph, fpw,82670, 0) ;//for wordnet only.
	fclose(fpw);
	igraph_simplify(data_graph, 1, 1, /*edge_comb=*/ 0);
	igraph_is_connected(data_graph, &res, IGRAPH_WEAK); 
  	if (!res) {
    		//printf(" Daya graph not connected\n");
    		//return 0;
	}
	//printf("number of vertices in Data graph  %d\n",(int)igraph_vcount(data_graph));
	//printf("number of edges in Data graph  %d\n",(int)igraph_ecount(data_graph));
	igraph_vector_init_seq(&y, 0, igraph_vcount(data_graph)-1);
 	SETVANV(data_graph, "id", &y);
	igraph_vector_destroy(&y);
  	for (int i=0; i<igraph_vcount(data_graph); i++){
    		if (VAN(data_graph, "id", i) != i){
     			return 1;
    		}
  	}
	SETVANV(data_graph, "label", &t_nodeLabels);
	for (int i=0; i<igraph_vcount(data_graph); i++){
    		if (VAN(data_graph, "label", i) != VECTOR(t_nodeLabels)[i]){
			//printf("id %d label %d \n",i,(int)VAN(data_graph, "label", i));
	      		return 2;
	    	}
  	}
	igraph_vector_destroy(&t_nodeLabels);
	igraph_integer_t mnd_v_g;
	
	for (int i=0; i<igraph_vcount(data_graph); i++){
		igraph_vector_init(&y,0);
		igraph_neighbors(data_graph, &y,i, IGRAPH_ALL);	
		igraph_maxdegree(data_graph, &mnd_v_g,igraph_vss_vector(&y), IGRAPH_ALL,0);	
		SETVAN(data_graph,"mnd",i,mnd_v_g);
		if (VAN(data_graph, "mnd", i) != mnd_v_g){
			//printf("mnd not set");
	      		return 2;
			//printf("node  %d mnd %d \n",i,(int)VAN(&data_graph, "mnd", i));
	    	}
		igraph_vector_destroy(&y);
	}  
	igraph_vector_init(&y, 0);
	igraph_degree(data_graph, &y,igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);
	SETVANV(data_graph, "deg", &y);
	igraph_vector_destroy(&y);
	
	return 0;
}

  

