class Generate_Graph
{
public:
	Generate_Graph(int nodes) : n(nodes){};
	graph create();
	void print_Graph(graph G);

private:
	int n;

};

graph Generate_Graph::create()
{
	graph 	G;
	node 	v;

	for (int i = 0; i < n; i++)
		{
			G.new_node();
		}	

	return G;
}


void Generate_Graph::print_Graph(graph G)
{
	node v;
	edge e;

	std::cout<<"\n";
	std::cout <<"/**********************************************/"<< "\n";
	std::cout <<"/*---------- RANDOM GRAPH GENERATED ----------*/"<< "\n";
	std::cout <<"/**********************************************/"<< "\n";

	std::cout <<"/*--- Nodes in Graph G ---*/"<< std::endl;

	forall_nodes(v,G)
	{
		std::cout<<" ( "<<v->id()<<" ) "<<"\n";
	}

	std::cout<<"\n";
	// std::cout <<"/*--- Edges in Graph G ---*/"<< std::endl;

	// forall_edges(e,G)
	// {
	// 	node s= G.source(e);
	// 	node t= G.target(e);
	// 	std::cout<<s->id()<<" -> "<<t->id()<<"\n";
	// }
	// std::cout<<"\n";

}
