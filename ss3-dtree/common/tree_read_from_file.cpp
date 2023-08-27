#include "tree_read_from_file.h"

/**
 * Node constructor that takes its number num, by default it has no children
 * and parents, is not colored.
 * @param num the number that identifies the node
 */
DecTree::Node::Node()
  : left(0)
  , right(0)
  , parent(0)
  , level(0)
  , leaf(false)
  , threshold(0)
  , attribute_index(0)
  , classification(0)
  {}

DecTree::Node::Node(const Node& other){
    left = new Node(*(other.left));
    right = new Node(*(other.right));
    parent = new Node(*(other.parent));
    level = other.level;
    leaf = other.leaf;
    threshold = other.threshold;
    attribute_index = other.attribute_index;
    classification = other.classification;
}

/**
 * Gamma2 constructor that creates a Gamma2 graph with specified number of Nodes
 * @param num the number specifying the number of Nodes
 */
DecTree::DecTree()
  {
    num_attributes = 0;
    num_dec_nodes = 0;
    depth = 0;
    num_of_leaves = 0;
    dummy_non_full = 0;
}

DecTree::DecTree(const DecTree& other){
    node_vec = other.node_vec;
    decnode_vec = other.decnode_vec;
    attributes = other.attributes;
    thresholds = other.thresholds;
    num_attributes = other.num_attributes;
    num_dec_nodes = other.num_dec_nodes;
    depth = other.depth;
    num_of_leaves = other.num_of_leaves;
    dummy_non_full = other.dummy_non_full;
}

/**
 * Add a node
 * @param node the Node to be added to the tree
 */
void DecTree::add_node(DecTree::Node* node){
    node_vec.push_back(node);
}

/**
 * Add a decision node
 * @param node the Node to be added to the tree
 */
void DecTree::add_decnode(DecTree::Node* node){
    decnode_vec.push_back(node);
}

/**
 * Add an edge between two nodes
 * @param n1 the starting Node of the edge
 * @param n2 the ending Node of the edge
 */
void DecTree::add_edge(DecTree::Node* n1, DecTree::Node* n2, int64_t node1, int64_t node2){
    n2->parent = n1;
    n2->level = n1->level + 1;
	if(n1->left == NULL){
        n1->left = n2;
        left[node1] = node2;
	}
	else if(n1->right == NULL){
        n1->right = n2;
        right[node1] = node2;
	}
}

void DecTree::add_edge_pack(DecTree::Node* n1, DecTree::Node* n2, int64_t node1, int64_t node2){
    // std::cout << "this is " << node1 << "->" << node2 << std::endl;
    n2->parent = n1;
    n2->level = n1->level + 1;
    level[node2] = n2->level;
    //set the children's point in advance
    if(n2->level%2 == 1){//circle the parent
        left[node2] = node1;
        right[node2] = node1;
    }else{//circle itself
        left[node2] = node2;
        right[node2] = node2;
    }

	if(n1->left == NULL){
        n1->left = n2;
        left[node1] = node2;
	}
	else if(n1->right == NULL){
        n1->right = n2;
        right[node1] = node2;
	}
}

/**
 * Takes a string in the Format "i i i ..." separated by ' '
 * @param str the string to tokenize
 * @param tokens the result vector of wire id
 */
void tokenize(const std::string& str, std::vector<string>& tokens) {
	std::size_t prev = 0, pos;
    //if there is any " " [] \\ in str, it will return 1 //"string::npos"means find failed
    while ((pos = str.find_first_of(" [] \\ ", prev)) != std::string::npos)
    {
        if (pos > prev)
            tokens.push_back(str.substr(prev, pos-prev));
        prev = pos+1;
    }
    if (prev < str.length()){
        tokens.push_back(str.substr(prev, std::string::npos));
    }
}

/**
 * Erase First Occurrence of given  substring from main string.
 */
void eraseSubStr(std::string & mainStr, const std::string & toErase)
{
	// Search for the substring in string
	size_t pos = mainStr.find(toErase);

	if (pos != std::string::npos)
	{
		// If found then erase it from string
		mainStr.erase(pos, toErase.length());
	}
}

class SearchFunction {
    public:
        SearchFunction(uint32_t item): item_(item) {}
        bool operator()(std::uint64_t const & ref) {
            return ref == item_;
        }
    private:
        uint32_t item_;
};

//root node will be in decnode_vec[0] and node_vec[0]
void DecTree::read_from_file(string string_file){
    const char* filename = string_file.c_str();
    ifstream file;
    file.open(filename);
#ifdef DTREE_DEBUG
    cout << filename << endl;
#endif 
    if (!file)  //条件成立，则说明文件打开出错
        cout << "open errors" << endl;
    //ofstream file2;
    //file2.open("dectree.txt", std::ios_base::app);

    uint64_t node1;
    uint64_t node2;

    DecTree::Node* node;
    uint64_t index;
    string line;
    vector<string> tokens;
    vector<uint64_t>::iterator it;
    float thres;
    //init left and right
    for(int i = 0; i < 100000; i++){
        left.push_back(0);
        right.push_back(0);
        // level.push_back(0);
    }
    while (getline(file, line)){
        tokenize(line, tokens);
        // //print the tree
        // for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); it++){
        //     cout << *it << " ";
        // }
        // cout << endl;

        if(tokens[1] == "label=\"gini"){
            node1 = atoi(tokens[0].c_str());
            // cout << "node1 of label=\"gini is: " << node1 << endl;
            this->add_node(new DecTree::Node());
            this->node_vec[node1]->leaf = true;
            this->num_of_leaves++;
            this->node_vec[node1]->classification = atoi(tokens[6].c_str());
            //
            this->thres.push_back(0);
            this->map.push_back(0);
            for(int i = 9; i < tokens.size()-2; i++){
                if(atoi(tokens[i].c_str()) != 0){
                    this->label.push_back(i-8);
                }
            }
        }
        else if(tokens[1] == "label=\"X"){
            node1 = atoi(tokens[0].c_str());
            // cout << "node1 of label=\"X is: " << node1 << endl;
            node = new DecTree::Node();
            index = atoi(tokens[2].c_str());
            //映射在这里
            node->attribute_index = index;
            //cout << index << endl;
            thres = atof(tokens[4].c_str());
            //门限值在这里
            node->threshold = thres*1000;
            //cout << node->threshold << endl; //Thresholds are converted so that we only compare integers
            this->num_dec_nodes++;
            this->add_node(node);
            this->add_decnode(node);

            it = find_if(this->attributes.begin(), this->attributes.end(), SearchFunction(index));
            //未找到
            if(it == this->attributes.end()){
                this->num_attributes++;
            }
            this->attributes.push_back(index);
            this->thresholds.push_back(node->threshold);
            //
            this->thres.push_back(node->threshold);
            this->map.push_back(index);
            this->label.push_back(0);
        }
        else if(tokens[1] == "->"){
            node1 = atoi(tokens[0].c_str());
            node2 = atoi(tokens[2].c_str());
            // cout << node1 << " ";
            // cout << node2 << endl;
            this->add_edge(this->node_vec[node1], this->node_vec[node2], node1, node2);
        }
    }
    //init left and right arraies
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        if(left[i] == 0){//label nodes, circle itself
        // for(int j = 0; j < (num_dec_nodes + num_of_leaves); j++){
        //     if(left_tmp[j].get_ui() == i || right_tmp[j].get_ui() == i){
        //         left[i] = j; 
        //         right[i] = j;
        //         break;
        //         }
        //     } 
            left[i] = i;
            right[i] = i;
        }
        
    }

#ifdef DTREE_DEBUG
    //output data list of the tree
    std::cout << "**********threshold**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << this->thres[i] << " ";
    }
    std::cout << "\n**********left**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << left[i] << " ";
    }
    std::cout << "\n**********right**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << right[i] << " ";
    }
    std::cout << "\n**********mapping**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << this->map[i] << " ";
    }
    std::cout << "\n**********label**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << this->label[i] << " ";
    }
    cout << endl;
#endif 

    for(uint32_t i = 0; i < this->node_vec.size(); ++i){
        node = this->node_vec[i];
        if(node->leaf){ //Right is also 0 in this case
	    if(node->level > this->depth){
                this->depth = node->level;
            }
        }
    }
#ifdef DTREE_DEBUG
    cout << "Total number of attributes " << this->num_attributes << endl;
    cout << "Depth of the tree " << this->depth << endl;
    cout << "Total number of decision nodes " << this->num_dec_nodes << endl;
    cout << "Total number of leaves " << this->num_of_leaves << endl;
#endif 
    for(uint32_t i = 0; i < this->node_vec.size(); ++i){
        node = this->node_vec[i];
        if(node->leaf){
            this->dummy_non_full += this->depth - node->level;
        }
    }
#ifdef DTREE_DEBUG
    cout << "Total number of dummy nodes in non-full tree " << this->dummy_non_full << endl;
#endif 
    uint32_t total_number_of_edges = 0;
    for(uint32_t i = 0; i < this->node_vec.size(); ++i){
        node = this->node_vec[i];
        if(node->leaf){
            total_number_of_edges += node->level;
        }
    }
#ifdef DTREE_DEBUG
    cout << "Total number of edges " << total_number_of_edges << endl;
#endif 
    file.close();
    //file2.close();
}

//root node will be in decnode_vec[0] and node_vec[0]
void DecTree::read_from_file_pack(string string_file){
    const char* filename = string_file.c_str();
    ifstream file;
    file.open(filename);
#ifdef DTREE_DEBUG
    cout << filename << endl;
#endif 
    if (!file)  //条件成立，则说明文件打开出错
        cout << "open errors" << endl;
    //ofstream file2;
    //file2.open("dectree.txt", std::ios_base::app);

    uint64_t node1;
    uint64_t node2;

    DecTree::Node* node;
    uint64_t index;
    string line;
    vector<uint64_t>::iterator it;
    float thres;
    //init left and right
    for(int i = 0; i < 100000; i++){
        left.push_back(0);
        right.push_back(0);
        level.push_back(0);
    }
    while (getline(file, line)){
        vector<string> tokens;
        tokenize(line, tokens);
        // //print the tree
        // for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); it++){
        //     cout << *it << " ";
        // }
        // cout << endl;
        if(tokens.size() > 1){
            if(tokens[1] == "label=\"gini"){
                node1 = atoi(tokens[0].c_str());
                // cout << "node1 of label=\"gini is: " << node1 << endl;
                this->add_node(new DecTree::Node());
                this->node_vec[node1]->leaf = true;
                this->num_of_leaves++;
                this->node_vec[node1]->classification = atoi(tokens[6].c_str());
                //
                this->thres.push_back(0);
                this->map.push_back(0);
                for(int i = 9; i < tokens.size()-2; i++){
                    if(atoi(tokens[i].c_str()) != 0){
                        this->label.push_back(i-8);
                    }
                }
            }
            else if(tokens[1] == "label=\"X"){
                node1 = atoi(tokens[0].c_str());
                // cout << "node1 of label=\"X is: " << node1 << endl;
                node = new DecTree::Node();
                index = atoi(tokens[2].c_str());
                //映射在这里
                node->attribute_index = index;
                //cout << index << endl;
                thres = atof(tokens[4].c_str());
                //门限值在这里
                node->threshold = thres*1000;
                //cout << node->threshold << endl; //Thresholds are converted so that we only compare integers
                this->num_dec_nodes++;
                this->add_node(node);
                this->add_decnode(node);

                it = find_if(this->attributes.begin(), this->attributes.end(), SearchFunction(index));
                //未找到
                if(it == this->attributes.end()){
                    this->num_attributes++;
                }
                this->attributes.push_back(index);
                this->thresholds.push_back(node->threshold);
                //
                this->thres.push_back(node->threshold);
                this->map.push_back(index);
                this->label.push_back(0);
            }
            else if(tokens[1] == "->"){
                node1 = atoi(tokens[0].c_str());
                node2 = atoi(tokens[2].c_str());
                // cout << node1 << " ";
                // cout << node2 << endl;
                this->add_edge_pack(this->node_vec[node1], this->node_vec[node2], node1, node2);
            }
            vector <string>().swap(tokens);//delete tokens
        }
    }
    

#ifdef DTREE_DEBUG
    //output data list of the tree
    std::cout << "**********threshold**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << this->thres[i] << " ";
    }
    std::cout << "\n**********left**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << left[i] << " ";
    }
    std::cout << "\n**********right**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << right[i] << " ";
    }
    std::cout << "\n**********mapping**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << this->map[i] << " ";
    }
    std::cout << "\n**********label**************" << std::endl;
    for(int i = 0; i < (num_dec_nodes + num_of_leaves); i++){
        std::cout << this->label[i] << " ";
    }
    cout << endl;
#endif 

    for(uint32_t i = 0; i < this->node_vec.size(); ++i){
        node = this->node_vec[i];
        if(node->leaf){ //Right is also 0 in this case
	    if(node->level > this->depth){
                this->depth = node->level;
            }
        }
    }
#ifdef DTREE_DEBUG
    cout << "Total number of attributes " << this->num_attributes << endl;
    cout << "Depth of the tree " << this->depth << endl;
    cout << "Total number of decision nodes " << this->num_dec_nodes << endl;
    cout << "Total number of leaves " << this->num_of_leaves << endl;
#endif 
    for(uint32_t i = 0; i < this->node_vec.size(); ++i){
        node = this->node_vec[i];
        if(node->leaf){
            this->dummy_non_full += this->depth - node->level;
        }
    }
#ifdef DTREE_DEBUG
    cout << "Total number of dummy nodes in non-full tree " << this->dummy_non_full << endl;
#endif 
    uint32_t total_number_of_edges = 0;
    for(uint32_t i = 0; i < this->node_vec.size(); ++i){
        node = this->node_vec[i];
        if(node->leaf){
            total_number_of_edges += node->level;
        }
    }
#ifdef DTREE_DEBUG
    cout << "Total number of edges " << total_number_of_edges << endl;
#endif 
    file.close();
    //file2.close();
}


void DecTree::fullTree(uint32_t num_att, uint32_t tree_depth){
    this->num_attributes = num_att;
    this->num_dec_nodes = pow(2.0, num_att);
    this->depth = tree_depth;
    this->num_of_leaves = pow(2.0, num_att + 1);
    vector<uint64_t> attr(num_att, 0);
    vector<uint64_t> thres(this->num_dec_nodes, 0);
    this->attributes = attr;
    this->thresholds = thres;

    for(uint32_t i = 0; i < this->num_dec_nodes + this->num_of_leaves; ++i){
        DecTree::Node* newNode = new DecTree::Node();
        if(i == 0){
            newNode->parent = NULL;
            newNode->level = 0;
        }
        else if(i%2 == 0){
            this->node_vec[i/2 - 1]->right = newNode;
            newNode->parent = this->node_vec[i/2 - 1];
            newNode->level = newNode->parent->level + 1;
        }
        else if(i%2 == 1){ //odd case
            this->node_vec[i/2]->left = newNode;
            newNode->parent = this->node_vec[i/2];
            newNode->level = newNode->parent->level + 1;
        }

        if(i < this->num_dec_nodes){
            this->decnode_vec.push_back(newNode);
        }
        else{
            newNode->leaf = true;
        }
        this->node_vec.push_back(newNode);
    }
}

void DecTree::depthPad(){
    DecTree::Node* tmp;
    for(uint32_t i = 0; i < this->node_vec.size(); ++i){
        if(this->node_vec[i]->leaf){
            if(this->node_vec[i]->level != this->depth){
                tmp = this->node_vec[i];
                this->node_vec[i] = new DecTree::Node();
                this->node_vec[i]->parent = tmp->parent;
                this->node_vec[i]->level = tmp->level; //leaf is false, threshold is 0, attribute_index is 0
                this->decnode_vec.push_back(this->node_vec[i]);
                this->attributes.push_back(this->node_vec[i]->attribute_index);
                this->thresholds.push_back(this->node_vec[i]->threshold);
                this->num_dec_nodes++;

                tmp->level++;
                tmp->parent = this->node_vec[i];
                this->node_vec[i]->left = tmp;
                this->node_vec[i]->right = tmp;
                this->node_vec.push_back(tmp);
            }
        }
    }
    //cout << "Number of decision nodes " << this->num_dec_nodes << endl;
}

void DecTree::evaluate(vector<uint64_t> inputs){
    DecTree::Node* this_node = this->node_vec[0];
    while(!(this_node->leaf)){
        //cout << inputs[this_node->attribute_index] << endl;
        //cout << this_node->threshold << endl;
        if(inputs[this_node->attribute_index] <= this_node->threshold){
            this_node = this_node->left;
        }
        else{
            this_node = this_node->right;
        }
    }
}

/**
 * Destructor DecTree
 */
DecTree::~DecTree(){
	node_vec.clear();
}