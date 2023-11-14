#include <iostream>
#include "LEDA/graph/ugraph.h"
#include "LEDA/graph/node_list.h"
#include <LEDA/graph/node_array.h>
#include "LEDA/core/queue.h"
#include <LEDA/core/array.h>


using namespace leda;

#define NODES 9
#define EDGES 10

#define INSERT_EDGES 8
#define DELETE_EDGES 2

#define MAX_NODES 100

#include "Generate_Graph.hpp"



/********************************************************************************/
/*------------------------------ Data Structures -------------------------------*/
/********************************************************************************/

/*--------- List for all Reached Nodes -----------*/
/*------------------------------------------------*/
struct ReachesList
{
	list <node> Reaches;

	// ReachesList()
	// {
	// 	Reaches = NULL;
	// }
};


/*--------- List for all Adjacent Nodes ----------*/
/*------------------------------------------------*/
struct AdjacentList
{
	list <node> Adjacent;

	// AdjacentList()
	// {
	// 	Adjacent = NULL;
	// }
};


/*----------------- Index Array ------------------*/
/*------------------------------------------------*/
struct Index
{
	node 	edgeTarget;
	node 	closureSource;
	node 	SGnode;
	int 	refcount;
	int 	id;

	Index()
	{
		edgeTarget		= NULL;
		closureSource	= NULL;
		refcount		= 0;
		SGnode			= NULL;
		id 				= -1;
	}
};

struct NodeInfo
{
	bool 	closure_node;
	bool 	join_node;
	int 	x;
	int 	y;
	int 	z;

	NodeInfo()
	{
	closure_node 	= false;
	join_node 		= false;
	x 				= NULL;
	y 				= NULL;
	z 				= NULL;
	}
};


node_array<NodeInfo> add_closure_vertex(node s_gr, node_array<NodeInfo> sup_info, node x, node y)
{
	
	sup_info[s_gr].closure_node 	= true;
	sup_info[s_gr].x 				= x -> id();
	sup_info[s_gr].y 				= y -> id();
	std::cout << "----------------------------------------------------------\n";
	std::cout << "New closure vertex added in support graph : <"<< x->id() << "," << y->id() << "> |\n";
	std::cout << "----------------------------------------------------------\n";

	return sup_info;
}

node_array<NodeInfo> add_join_vertex(node s_gr, node_array<NodeInfo> sup_info, node x, node y, node z)
{

	// node a = s_gr.new_node();

	sup_info[s_gr].join_node 	= true;
	sup_info[s_gr].x 			= x -> id();
	sup_info[s_gr].y 			= y -> id();
	sup_info[s_gr].z 			= z -> id();

	std::cout << "**********************************************************\n";
	std::cout << "New join vertex added in support graph : <" << x->id() << "," << y->id() << "," << z->id() << ">\n";
	std::cout << "**********************************************************\n";

	return sup_info;
}

/********************************************************************************/
/*--------------------------- Insert Edge Functions ----------------------------*/
/********************************************************************************/


void MakeEdge(node a, node b, AdjacentList Adjacent_ar[NODES], Index Index_array[NODES][NODES])
{
	list<node>::item adj_it;					//iterators for lists

	Adjacent_ar[a->id()].Adjacent.append(b);
	forall_items(adj_it, Adjacent_ar[a->id()].Adjacent)
	{
		if (Adjacent_ar[a->id()].Adjacent[adj_it] == b)
		{
			Index_array[a->id()][b->id()].edgeTarget = Adjacent_ar[a->id()].Adjacent[adj_it];
		}
	}

}


void MakeClosure(node a, node b, ReachesList Reaches_ar[NODES], Index Index_array[NODES][NODES])
{
	list<node>::item rch_it;					//iterators for lists

	Reaches_ar[b->id()].Reaches.append(a);
	forall_items(rch_it, Reaches_ar[b->id()].Reaches)
	{
		if (Reaches_ar[b->id()].Reaches[rch_it] == a)
		{
			Index_array[a->id()][b->id()].closureSource = Reaches_ar[b->id()].Reaches[rch_it];
		}
	}

}


node_array<NodeInfo> InsertEdge(graph& G, graph& sup_graph, /*node sup_closure_node,*/ node_array<NodeInfo> sup_info, ReachesList Reaches_ar[NODES], AdjacentList Adjacent_ar[NODES], Index Index_array[NODES][NODES])
{

	list<node> 			worklist;
	list<node>::item 	reach_it, adj_it;	//iterators for lists

	node a, b;
	node x, y, z;
	node v;
	edge e;

	a = G.choose_node();
	b = G.choose_node();

	// std::cout << "Chosen Nodes: a=(" << a->id() << ") b=("<<b->id() <<")\n";

	while ( (a == b) || (a > b))
	{
		// std::cout << "Try again!\n";
		a = G.choose_node();
		b = G.choose_node();
		// std::cout << "New Chosen Nodes: a=(" << a->id() << ") b=("<<b->id() <<")\n";
	}

	e = G.new_edge(a, b);
	std::cout << "Adding Edge : e = <" << a->id() << ","<<b->id() <<">\n";
	MakeEdge(a,b, Adjacent_ar, Index_array);

	if (Index_array[a->id()][b->id()].refcount == 0) 	//check if <a,b> is a tc edge (recount>0)
	{
		MakeClosure(a,b, Reaches_ar, Index_array);		//<a,b> is a new tc edge / new closure_vertex (a,b)

		node sup_closure_node = sup_graph.choose_node();
		sup_info = add_closure_vertex(sup_closure_node, sup_info, a, b);
		Index_array[a->id()][b->id()].SGnode = b;

		worklist.push_back(a);							//add <a,b> to tc worklist
		worklist.push_back(b);
	}

	Index_array[a->id()][b->id()].refcount += 1;		//<a,b> new closure / refcount++

	// find nodes (x) that reach node a looking for [x -> a -> b]
	forall_items(reach_it, Reaches_ar[a->id()].Reaches)	//find all nodes x so that <x,a> is already a tc edge
	{
		x = Reaches_ar[a->id()].Reaches[reach_it];

		if (Index_array[x->id()][b->id()].refcount == 0)	//check if <x,b> is a new tc edge
		{
			MakeClosure(x, b, Reaches_ar, Index_array);		//<x,b> new tc edge / new closure_vertex (x,b)

			node sup_closure_node = sup_graph.choose_node();
			while (sup_info[sup_closure_node].x != 0 || sup_info[sup_closure_node].y != 0 || sup_info[sup_closure_node].z != 0)
				{
					node sup_closure_node = sup_graph.choose_node();
				}
			sup_info = add_closure_vertex(sup_closure_node, sup_info, x, b);
			Index_array[x->id()][b->id()].SGnode = b;

			node sup_join_node = sup_graph.choose_node();
			while (sup_info[sup_join_node].x != 0 || sup_info[sup_join_node].y != 0 || sup_info[sup_join_node].z != 0)
				{
					node sup_join_node = sup_graph.choose_node();
				}
			sup_info = add_join_vertex(sup_join_node, sup_info, x, a, b); //add edges to join_vertex in(x,a) in(a,b) out(x,b)

			// e = sup_graph.new_edge();
			forall_nodes (v,sup_graph)
			{
if(v!=sup_join_node)
	{
				if ((sup_info[v].x == x-> id()) && (sup_info[v].y == a -> id()) && (sup_info[v].closure_node == true))
				{
					sup_graph.new_edge(v, sup_join_node);
					std::cout << "New supporting edge <" <<sup_info[v].x <<","<< sup_info[v].y <<","<< sup_info[v].z<<"> --> <"<<sup_info[sup_join_node].x <<","<<sup_info[sup_join_node].y <<","<< sup_info[sup_join_node].z <<">\n\n";
				}
				if ((sup_info[v].x == a-> id()) && (sup_info[v].y == b -> id()) && (sup_info[v].closure_node == true))
				{
					sup_graph.new_edge(v, sup_join_node);
					std::cout << "New supporting edge <" <<sup_info[v].x <<","<< sup_info[v].y <<","<< sup_info[v].z<<"> --> <"<<sup_info[sup_join_node].x <<","<<sup_info[sup_join_node].y <<","<< sup_info[sup_join_node].z <<">\n\n";
				}
				if ((sup_info[v].x == x-> id()) && (sup_info[v].y == b -> id()) && (sup_info[v].closure_node == true))
				{
					sup_graph.new_edge(sup_join_node, v);
					std::cout << "New supporting edge <" <<sup_info[sup_join_node].x <<","<< sup_info[sup_join_node].y <<","<< sup_info[sup_join_node].z<<"> --> <"<<sup_info[v].x<< ","<<sup_info[v].y <<","<<sup_info[v].z<<">\n\n";
				}
	}
			}

			worklist.push_back(x);
			worklist.push_back(b);

		}
		Index_array[x->id()][b->id()].refcount += 1; //<x,b> new closure / refcount++
	}

	while (!worklist.empty())		// choosing every new tc edge iterativelly
	{

		x = worklist.pop();			// choosing edge (x,y)
		y = worklist.pop();

		// find adjacent target nodes to new tc edges
		forall_items(adj_it, Adjacent_ar[y->id()].Adjacent) // finding all adjacent nodes to y (z,y)
		{
			z = Adjacent_ar[y->id()].Adjacent[adj_it];

			if (Index_array[x->id()][z->id()].refcount == 0)
			{
				MakeClosure(x, z, Reaches_ar, Index_array);	//<x,z> new tc edge / new closure_vertex (x,z)

				node sup_closure_node = sup_graph.choose_node();
				while (sup_info[sup_closure_node].x != 0 || sup_info[sup_closure_node].y != 0 || sup_info[sup_closure_node].z != 0)
					{
						node sup_closure_node = sup_graph.choose_node();
					}
				sup_info = add_closure_vertex(sup_closure_node, sup_info, x, z);
				Index_array[x->id()][z->id()].SGnode = z;

				node sup_join_node = sup_graph.choose_node();
			while (sup_info[sup_join_node].x != 0 || sup_info[sup_join_node].y != 0 || sup_info[sup_join_node].z != 0)
				{
					node sup_join_node = sup_graph.choose_node();
				}
				sup_info = add_join_vertex(sup_join_node, sup_info, x, y, z);

			forall_nodes (v,sup_graph)
			{
if(v!=sup_join_node)
{
				if ((sup_info[v].x == x-> id()) && (sup_info[v].y == y -> id()) && (sup_info[v].closure_node == true))
				{
					sup_graph.new_edge(v, sup_join_node);
					std::cout << "New supporting edge <" <<sup_info[v].x <<","<< sup_info[v].y <<","<< sup_info[v].z<<"> --> <"<<sup_info[sup_join_node].x <<","<<sup_info[sup_join_node].y <<","<< sup_info[sup_join_node].z <<">\n\n";
				}
				if ((sup_info[v].x == y-> id()) && (sup_info[v].y == z -> id()) && (sup_info[v].closure_node == true))
				{
					sup_graph.new_edge(v, sup_join_node);
					std::cout << "New supporting edge <" <<sup_info[v].x <<","<< sup_info[v].y <<","<< sup_info[v].z<<"> --> <"<<sup_info[sup_join_node].x <<","<<sup_info[sup_join_node].y <<","<< sup_info[sup_join_node].z <<">\n\n";
				}
				if ((sup_info[v].x == x-> id()) && (sup_info[v].y == z -> id()) && (sup_info[v].closure_node == true))
				{
					sup_graph.new_edge(sup_join_node, v);
					std::cout << "New supporting edge <" <<sup_info[sup_join_node].x <<","<< sup_info[sup_join_node].y <<","<< sup_info[sup_join_node].z<<"> --> <"<<sup_info[v].x<< ","<<sup_info[v].y<< ","<<sup_info[v].z<<">\n\n";
				}
}
			}

				worklist.push_back(x);
				worklist.push_back(z);

			}
			Index_array[x->id()][z->id()].refcount += 1; //<x,z> new closure / refcount++
		}

	}

	return sup_info;
}


/********************************************************************************/
/*--------------------------- Delete Edge Functions ----------------------------*/
/********************************************************************************/

void RemoveEdge(node a, node b, AdjacentList Adjacent_ar[NODES], Index Index_array[NODES][NODES])
{
	list<node>::item adj_it;					//iterators for lists

	node node_pointer;

	node_pointer = Index_array[a->id()][b->id()].edgeTarget;

	forall_items(adj_it, Adjacent_ar[a->id()].Adjacent)
		{
			if (Adjacent_ar[a->id()].Adjacent[adj_it] == node_pointer)
			{
				Adjacent_ar[a->id()].Adjacent[adj_it] = NULL;
			}
		}
	
	Index_array[a->id()][b->id()].edgeTarget = NULL;
}



void RemoveClosure(node a, node b, ReachesList Reaches_ar[NODES], Index Index_array[NODES][NODES])
{
	list<node>::item rch_it;					//iterators for lists

	node node_pointer;

	node_pointer = Index_array[a->id()][b->id()].closureSource;

	forall_items(rch_it, Reaches_ar[a->id()].Reaches)
		{
			if (Reaches_ar[a->id()].Reaches[rch_it] == node_pointer)
			{
				Reaches_ar[a->id()].Reaches[rch_it] = NULL;
			}
		}

	Index_array[a->id()][b->id()].closureSource = NULL;
}



node_array<NodeInfo> DeleteEdge(graph& G, graph& sup_graph, node_array<NodeInfo> sup_info, ReachesList Reaches_ar[NODES], AdjacentList Adjacent_ar[NODES], Index Index_array[NODES][NODES])
{
	bool delete_closure_node = true;

	list<node> 			worklist;
	list<node>::item 	reach_it, adj_it;	//iterators for lists

	node a, b;
	node x, y, z;
	node v, n;
	edge e, e_join, e_join_out;

	e = G.choose_edge();
	a = source(e);
	b = target(e);

	std::cout << "Deleting Edge : e = <" << a->id() << ","<<b->id() <<">\n";

	RemoveEdge(a, b, Adjacent_ar, Index_array);

	forall_nodes (v, sup_graph)
	{
		if ((sup_info[v].closure_node == true) && (sup_info[v].x == a -> id()) && (sup_info[v].y == b -> id()))	// <a,b> --> <a, b, irr>
			{
				std::cout << "Deleting closure node : n = <" << sup_info[v].x << "," << sup_info[v].y <<">\n";
				sup_graph.del_node(v);
			}
		if ((sup_info[v].closure_node == true) && (sup_info[v].y == a -> id()) && (sup_info[v].z == b -> id())) // <a,b> --> <irr, a, b>	den uparxei periptosi
			{
				std::cout << "Deleting closure node : n = <" << sup_info[v].x << "," << sup_info[v].y <<">\n";
				sup_graph.del_node(v);
			}
		if ((sup_info[v].join_node == true) && (sup_info[v].x == a -> id()) && (sup_info[v].y == b -> id()))		// <x,y,z> <-- <a, b, irr>
			{

				forall_out_edges (e_join, v)
				{
					n = target(e_join);
					forall_in_edges	(e_join_out, n)		
					{
						if((source(e_join_out) != v) && (sup_info[source(e_join_out)].closure_node == true))
						{
							delete_closure_node = false;
							std::cout << "NOT Deleting closure node c = <" << sup_info[n].x << "," << sup_info[n].y << ">, succesor of join node: n = <" << sup_info[v].x << "," << sup_info[v].y << "," << sup_info[v].z <<">\n";
							std::cout << "Other ansestor of closure node detected! ---> Node:"<< sup_info[source(e_join_out)].x << "," << sup_info[source(e_join_out)].y <<" \n";

						}
					}
					if (delete_closure_node)
					{
						std::cout << "Deleting succesor of join node: n = <" << sup_info[v].x << "," << sup_info[v].y << "," << sup_info[v].z <<">\n";
						std::cout << "Deleting closure node : n = <" << sup_info[n].x << "," << sup_info[n].y <<">\n";
						sup_graph.del_node(n);
					}
				}

				std::cout << "Deleting join node due to left edge: e = <" << sup_info[v].x << "," << sup_info[v].y << "," << sup_info[v].z << ">\n";
				sup_graph.del_node(v);
			}
		if ((sup_info[v].join_node == true) && (sup_info[v].y == a -> id()) && (sup_info[v].z == b -> id()))		// <x,y,x> <-- <irr, a, b>
			{

				forall_out_edges (e_join, v)
				{
					n = target(e_join);
					forall_in_edges	(e_join_out, n)		
					{
						if((source(e_join_out) != v) && (sup_info[source(e_join_out)].closure_node == true))
						{
							delete_closure_node = false;
							std::cout << "NOT Deleting closure node c = <" << sup_info[n].x << "," << sup_info[n].y << ">, succesor of join node: n = <" << sup_info[v].x << "," << sup_info[v].y << "," << sup_info[v].z <<">\n";
							std::cout << "Other ansestor of closure node detected! ---> Node:"<< sup_info[source(e_join_out)].x << "," << sup_info[source(e_join_out)].y <<" \n";

						}
					}
					if (delete_closure_node)
					{
						std::cout << "Deleting succesor of join node: n = <" << sup_info[v].x << "," << sup_info[v].y << "," << sup_info[v].z <<">\n";
						std::cout << "Deleting closure node : n = <" << sup_info[n].x << "," << sup_info[n].y <<">\n";
						sup_graph.del_node(n);
					}
				}

				std::cout << "Deleting join node due to right edge: e = <" << sup_info[v].x << "," << sup_info[v].y << "," << sup_info[v].z << ">\n";
				sup_graph.del_node(v);
			}
	}


	G.del_edge(e);
	Index_array[a->id()][b->id()].refcount -= 1;

	if (Index_array[a->id()][b->id()].refcount == 0)
	{
		RemoveClosure(a, b, Reaches_ar, Index_array);
		worklist.push_back(a);
		worklist.push_back(b);
	}

	forall_items(reach_it, Reaches_ar[a->id()].Reaches)
	{
		x = Reaches_ar[a->id()].Reaches[reach_it];

		Index_array[x->id()][b->id()].refcount -= 1;


		if (Index_array[x->id()][b->id()].refcount == 0)
		{
			RemoveClosure(x, b, Reaches_ar, Index_array);
			worklist.push_back(x);
			worklist.push_back(b);
		}
	}

	while (!worklist.empty())
	{
		x = worklist.pop();
		y = worklist.pop();

		forall_items(adj_it, Adjacent_ar[y->id()].Adjacent)
		{
			z = Adjacent_ar[y->id()].Adjacent[adj_it];
			Index_array[x->id()][z->id()].refcount -= 1;

			if (Index_array[x->id()][z->id()].refcount == 0)
			{
				RemoveClosure(x, z, Reaches_ar, Index_array);
				worklist.push_back(x);
				worklist.push_back(b);
			}
		}
	}

	return sup_info;
}

/********************************************************************************/
/*----------------------------- Findpath Functions -----------------------------*/
/********************************************************************************/

void findpath(graph& G, graph& sup_graph, node_array<NodeInfo> sup_info, node x, node z, Index Index_array [NODES] [NODES])
{

	node j, tar;
	edge e;

	if (Index_array[x->id()][z->id()].edgeTarget != NULL)
	{
		std::cout << "There is an edge (" << x -> id() << ") -> (" << z -> id() << ") in G!";
	}
	else
	{
		forall_nodes (j , sup_graph)
		{
			if (sup_info[j].join_node)
			{
				forall_out_edges (e, j)
				{
					tar = target(e);
					if ((sup_info[tar].x == x -> id()) && (sup_info[tar].y == z -> id()))
					{
						std::cout << "There is a path from (" << x -> id() << ") to (" << z -> id() << ") through <"  << sup_info[j].x << "," << sup_info[j].y << "," <<  sup_info[j].z << "> in G!\n";
					}
				}
			}
		}
	}
}

/********************************************************************************/
/*------------------------------ Print Functions -------------------------------*/
/********************************************************************************/

void print_Graph(graph& G)
{

	std::cout<<"\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout <<"/*------------ Printing Graph ------------*/" << "\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout<<"\n";

	edge e;

	forall_edges (e, G)
	{


		node s= G.source(e);
		node t= G.target(e);
		std::cout<<s->id()<<" -> "<<t->id()<<"\n";
	}
	std::cout<<"\n";

}


/*---- Prints the List for all Adjacent Nodes ----*/
/*------------------------------------------------*/

void print_Adjacent(graph G, AdjacentList Adjacent_ar [NODES])
{

	node 				v;
	list<node>::item 	adj_it;	//iterators for lists

	std::cout<<"\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout <<"/*------- Printing Adjacency lists -------*/" << "\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout<<"\n";

	forall_nodes (v,G)
	{

		std::cout << "Adjacent[" << v -> id() << "] = ";
		std::cout << "[ ";
		forall_items(adj_it, Adjacent_ar[v -> id()].Adjacent)
		std::cout << Adjacent_ar[v->id()].Adjacent[adj_it] -> id()<< " ";
		std::cout<<"]" << "\n";

	}

	std::cout<<"\n";
}

/*---- Prints the List for all Reached Nodes -----*/
/*------------------------------------------------*/

void print_Reaches(graph G, ReachesList Reaches_ar [NODES])
{

	node 				v;
	list<node>::item 	reach_it;	//iterators for lists

	std::cout<<"\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout <<"/*-------- Printing Reaches lists --------*/" << "\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout<<"\n";

	forall_nodes (v,G)
	{

		std::cout << "Reaches[" << v -> id() << "] = ";
		std::cout << "[ ";
		forall_items(reach_it, Reaches_ar[v -> id()].Reaches)
		std::cout << Reaches_ar[v->id()].Reaches[reach_it] -> id()<< " ";
		std::cout<<"]" << "\n";

	}

	std::cout<<"\n";
}

/*-------- Prints Array Index[][].refcount -------*/
/*------------------------------------------------*/

void print_Index_refcount(Index Index_array [NODES] [NODES])
{

	std::cout<<"\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout <<"/*-------- Printing Index.refcount -------*/" << "\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout<<"\n";

	for (int i = 0; i < NODES; i++)
	{
		for (int j = 0; j < NODES; j++)
		{
			std::cout <<"In.ref["<<i<<"]["<<j<<"]="<<Index_array[i][j].refcount<<" | ";
		}
		std::cout<<"\n";
	}

	std::cout<<"\n";
}

/*-------------- Prints Support Graph ------------*/
/*------------------------------------------------*/

void print_Support_Graph(graph& G, node_array<NodeInfo> sup_info)
{

	node v;
	edge e;

	std::cout <<"/*--- Closure Nodes in Support Graph G ---*/"<< std::endl;
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	forall_nodes(v, G)
	{
		if (sup_info[v].closure_node == 1)
		{
			std::cout <<"Closure node : ";
			std::cout<<"( x = " << sup_info[v].x << ", y = " << sup_info[v].y << " ) \n";
		}
	}
	
	std::cout<<"\n";

	std::cout <<"/*----- Join Nodes in Support Graph G ----*/"<< std::endl;
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	forall_nodes(v,G)
	{
		if (sup_info[v].join_node == true)
		{
		std::cout <<"Join node : ";
		std::cout<<" ( x = " << sup_info[v].x << ", y = " << sup_info[v].y << ", z = " << sup_info[v].z << " ) \n";
			if (!((sup_info[v].x < sup_info[v].y) && (sup_info[v].y < sup_info[v].z)))
			{
				std::cout <<"Join node ERROR!!!! ";
			}
		}
	}

	std::cout<<"\n";
	std::cout <<"/*-------- Edges in Support Graph --------*/"<< std::endl;
	std::cout <<"/*----------------------------------------*/"<< std::endl;

	forall_edges(e,G)
	{
		node s= G.source(e);
		node t= G.target(e);

		std::cout<< sup_info[s].x <<","<< sup_info[s].y<<","<< sup_info[s].z <<" ---> "<< sup_info[t].x <<","<< sup_info[t].y<<","<< sup_info[t].z <<"\n";
	}
	std::cout<<"\n";
}


int main()
{

	ReachesList 	Reaches_ar 			[NODES];	//an array of lists type of <ReachesList>
	AdjacentList 	Adjacent_ar 		[NODES];	//an array of lists type of <AdjacentList>
	Index			Index_array [NODES] [NODES];	//2-D array of objects "Index"

	list<node>::item r_it;							//iterators for lists

	node v, w;
	node x, z;
	edge e;

	/*-----------------------------------------------------------------*/
	/*--------------- GENERATE NDOES FOR A RANDOM GRAPH ---------------*/
	/*-----------------------------------------------------------------*/

	Generate_Graph *generate_graph	= 	new Generate_Graph(NODES);
	graph or_Graph					= 	generate_graph -> create();	//Original Graph generated
	generate_graph 					->	print_Graph(or_Graph);

	graph s_graph;								//Support Graph declared

	for (int i = 0; i < MAX_NODES; i++)
	{
		s_graph.new_node();
	}

	node_array<NodeInfo> sup_info(s_graph);		//sup_info appended to each Support Graph node

	/*-----------------------------------------------------------------*/
	/*----------------- INSERTING EDGES TO THE GRAPH ------------------*/
	/*-----------------------------------------------------------------*/

	std::cout<<"\n";
	std::cout <<"/*------ Inserting Edges to Graph G ------*/" << "\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout<<"\n";

	for (int i = 0; i < INSERT_EDGES; i++)
	{
		sup_info = InsertEdge(or_Graph, s_graph, sup_info, Reaches_ar, Adjacent_ar, Index_array);
	}

	std::cout<<"\n";
	std::cout <<"/*----- Insertion of Edges completed -----*/" << "\n";
	std::cout <<"/*----------------------------------------*/"<< std::endl;
	std::cout<<"\n";

	/*-------- Adjacent() lists printed -------*/
	/*-----------------------------------------*/
	print_Adjacent(or_Graph, Adjacent_ar);


	/*-------- Reaches() lists printed --------*/
	/*-----------------------------------------*/
	print_Reaches(or_Graph, Reaches_ar);


	/*-------- Index Array lists printed ------*/
	/*-----------------------------------------*/
	print_Index_refcount(Index_array);

	/*---------- Support Graph printed-- ------*/
	/*-----------------------------------------*/
	print_Support_Graph(s_graph, sup_info);


	/*-----------------------------------------------------------------*/
	/*----------------- DELETING EDGES TO THE GRAPH -------------------*/
	/*-----------------------------------------------------------------*/

	for (int i = 0; i < DELETE_EDGES; i++)
	{
		sup_info = DeleteEdge(or_Graph, s_graph, sup_info, Reaches_ar, Adjacent_ar, Index_array);
	}


	/*---------- Support Graph printed-- ------*/
	/*-----------------------------------------*/
	print_Support_Graph(s_graph, sup_info);


	x = or_Graph.first_node();
	z = or_Graph.last_node();
	while (x -> id() >= z -> id())
	{
		x = or_Graph.choose_node();
		z = or_Graph.choose_node();
	}

	std::cout <<"/*Path for nodes :  "<< x->id()<< " and " << z -> id() <<"\n";

	findpath(or_Graph, s_graph, sup_info, x, z, Index_array);

	print_Graph(or_Graph);

	return 0;

}