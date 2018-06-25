/*
 * predictor.hpp
 *
 *  Created on: Dec 12, 2017
 *      Author: colinliang
 *
 *      how to write a c++ xgboost predictor https://github.com/dmlc/xgboost/issues/2371
 *      modified from https://gist.github.com/wugh/25c156009cb02ce94d1c268ffd5c2db7
 *      没有使用任何的regex库， 因为未修改的代码无法编译（boost库regex部分）， 而g++ 4.4.6还不支持std regex
 *      将 unordered_map 改为了vector， 并去除了shared_pointer封装， 加快预测速度
 */

#ifndef LC_MACHINELEARNING_XGBOOST_PREDICTOR_HPP_
#define LC_MACHINELEARNING_XGBOOST_PREDICTOR_HPP_
#include "../../utility/StringUtil.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
namespace LC {
namespace xgboost {
class Node {
public:
	int current_;

	int left_;
	int right_;
	int miss_;

	int split_feature_;
	float split_threshold_;
	float value_;

	bool isleaf;

	// internal node
	Node(int current, int left, int right, int miss, int split_feature, float threshold) :
			current_(current), left_(left), right_(right), miss_(miss), split_feature_(split_feature), split_threshold_(
					threshold), value_(0.0), isleaf(false) {
	}

	// leaf node
	Node(int current = -1, float value = 0.0) :
			current_(current), left_(-1), right_(-1), miss_(-1), split_feature_(-1), split_threshold_(0.0), value_(
					value), isleaf(true) {
	}
};

class Tree {
public:
	int add_line(const std::string &line) {
		if (line.find("leaf") == std::string::npos) {
			return add_internal_node(line);
		} else {
			return add_leaf_node(line);
		}
	}

	float predict(const std::vector<float> &fv, float missing = NAN) const {
		if (tree_.empty())
			return 0.0f;
		const Node* pn = &tree_[0];
		while (!pn->isleaf) {
			if (std::isnan(missing) && std::isnan(fv[pn->split_feature_])) {
				pn = &tree_[pn->miss_];
			} else if (fv[pn->split_feature_] == missing) {
				pn = &tree_[pn->miss_];
			} else if (fv[pn->split_feature_] < (pn->split_threshold_)) {
				pn = &tree_[pn->left_];
			} else {
				pn = &tree_[pn->right_];
			}
		}
		return pn->value_;
	}

	/**
	 * 预测，在确保没有missingvalue的前提下， 该方法会快很多！
	 * @param fv
	 * @return
	 */
	inline float predict_no_missing_value(const std::vector<float> &fv) const {
//		if (tree_.empty())
//			return 0.0f;
		const Node* pn = &tree_[0];
		while (!pn->isleaf) {
			if (fv[pn->split_feature_] >= (pn->split_threshold_)) { //这里改成 >=是因为 NAN和任何数字比都为false，可以兼容一般情况下有missing value的情况
				pn = &tree_[pn->right_];
			} else {
				pn = &tree_[pn->left_];
			}
		}
		return pn->value_;
	}

	inline unsigned int size() const {
		return tree_.size();
	}

	void to_lua_script(std::stringstream& ss, Node* pn = NULL, int level = 1, int idx_start_from = 1, string indent =
			"  ", string var_name = "score") {
		if (pn == NULL)
			pn = &tree_[0];

		if (pn->isleaf) {
			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << var_name << " = " << var_name << " + " << pn->value_ << std::endl;
		} else {
			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "if fv[" << (pn->split_feature_ + idx_start_from) << "] >= " << pn->split_threshold_ << " then"
					<< std::endl;

			to_lua_script(ss, &tree_[pn->right_], level + 1, idx_start_from, indent, var_name);

			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "else" << std::endl;

			to_lua_script(ss, &tree_[pn->left_], level + 1, idx_start_from, indent, var_name);

			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "end" << std::endl;

		}
	}

	void to_java_code(std::stringstream& ss, Node* pn = NULL, int level = 1, int idx_start_from = 1, string indent =
			"  ", string var_name = "score") {
		if (pn == NULL)
			pn = &tree_[0];

		if (pn->isleaf) {
			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << var_name << " += " << pn->value_ << "f;" << std::endl;
		} else {
			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "if (fv[" << (pn->split_feature_ + idx_start_from) << "] >= " << pn->split_threshold_ << "f ){"
					<< std::endl;

			to_java_code(ss, &tree_[pn->right_], level + 1, idx_start_from, indent, var_name);

			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "} else {" << std::endl;

			to_java_code(ss, &tree_[pn->left_], level + 1, idx_start_from, indent, var_name);

			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "}" << std::endl;

		}
	}
private:
	std::vector<Node> tree_;

	int add_leaf_node(const std::string &line) {
//		std::cout << "add leaf node: " << line << std::endl;
		//叶子结点， 如 "63:leaf=-0.379265"
		std::vector<double> dv = LC::Str::str2doublevec_ignore_letters(line.data(), line.size());

		if (dv.size() != 2u)
			throw std::string("invalid xgboost leaf node: ") + line;
		int id = (int) (dv[0]);
		if (id < 0)
			throw std::string("invalid xgboost leaf node: ") + line;
		float value = (float) (dv[1]);

		unsigned int min_size = id + 1;
		if (tree_.size() < min_size) {
			tree_.resize(min_size);
		}
		tree_[id] = Node(id, value);
		return -1;
	}

	int add_internal_node(const std::string &line) {
//		std::cout << "add branch node: " << line << std::endl;

		//中间节点，如   "1:[f19<41.5] yes=3,no=4,missing=3"
		std::vector<double> dv = LC::Str::str2doublevec_ignore_letters(line.data(), line.size());
		if (dv.size() != 6u)
			throw std::string("invalid xgboost branch node: ") + line;
		int id = (int) (dv[0]);
		auto split_feature = (int) (dv[1]);
		auto threshold = (float) (dv[2]);
		auto left = (int) (dv[3]);
		auto right = (int) (dv[4]);
		auto missing = (int) (dv[5]);

		if (id < 0 || split_feature < 0 || left < 0 || right < 0 || missing < 0)
			throw std::string("invalid xgboost branch node: ") + line;
		unsigned int min_size = id + 1;
		if (tree_.size() < min_size) {
			tree_.resize(min_size);
		}

		tree_[id] = Node(id, left, right, missing, split_feature, threshold);

		return split_feature;
	}
};

class GBDT_predictor {

private:
	std::vector<Tree> trees_;
	int num_features_;
public:
	inline int num_features() const {
		return num_features_;
	}
	inline unsigned int num_trees() const {
		return trees_.size();
	}
	inline unsigned int num_nodes() const {
		unsigned int n = 0;
		for (unsigned int i = 0; i < trees_.size(); ++i) {
			n += trees_[i].size();
		}
		return n;
	}

	inline bool is_valid() const {
		return !trees_.empty();
	}

	string toString() const {
		std::stringstream ss;
		ss << "GBDT: num_features: " << num_features() << "; \tnum_trees: " << num_trees() << "; \tnum_nodes: "
				<< num_nodes() << std::endl;
		return ss.str();
	}

	GBDT_predictor() :
			num_features_(-1) {

	}

	GBDT_predictor(const string& filename, bool verbose = false) :
			num_features_(-1) {
		std::string line;
		std::ifstream infile(filename.c_str());
		if (!infile) {
			std::cout << "can not open gbdt model file: " << filename << std::endl;
			return;
		}
//		// omit first line
//		std::getline(infile, line);
		Tree tree;
		while (std::getline(infile, line)) {
			if (line.find("booster[") != std::string::npos) { //e.g. "booster[0]:"

				if (tree.size() > 0u) {
					if (verbose)
						std::cout << "current tree size: " << tree.size() << std::endl;

					trees_.emplace_back(tree);
					if (verbose)
						std::cout << "num_trees: " << trees_.size() << std::endl;
				} else {
					if (verbose)
						std::cout << "new tree: " << line << std::endl;
					tree = Tree();
				}
			} else {
				int split_feature = tree.add_line(line);
				if (split_feature > num_features_)
					num_features_ = split_feature;
			}
		}
		if (tree.size() > 0u) {
//			std::cout << "current tree size: " << tree.size() << std::endl;
			trees_.emplace_back(tree);
//			std::cout << "num_trees: " << trees_.size() << std::endl;
		}
		if (num_features_ >= 0)
			num_features_ += 1;
	}

	float predict(std::vector<float> &fv, float missing = NAN) const {
		float score = 0.0f;
		for (unsigned int i = 0; i < trees_.size(); ++i) {
			score += trees_[i].predict(fv, missing);
		}
//		return score;
		return 1.0f / (1.0f + std::exp(-score));
	}

	inline float predict_no_missing_value(std::vector<float> &fv) const {
		float score = 0.0f;
		for (unsigned int i = 0; i < trees_.size(); ++i) {
			score += trees_[i].predict_no_missing_value(fv);
		}
		return 1.0f / (1.0f + std::exp(-score));
//		return score;
	}

	string to_lua_script(int level = 0, int idx_start_from = 1, string indent = "  ", string var_name = "score") {

		std::stringstream ss;
		for (int i = 0; i < level; ++i)
			ss << indent;
		ss << "function xgboost_predict(fv)" << std::endl;

		level += 1;
		for (int i = 0; i < level; ++i)
			ss << indent;
		ss << "local " << var_name << " = 0.0" << std::endl;
		for (unsigned int i = 0; i < trees_.size(); ++i) {
			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "-- tree " << (i + 1) << " / " << trees_.size() << std::endl;
			trees_[i].to_lua_script(ss, NULL, level, idx_start_from, indent, var_name);
			ss << std::endl;
		}
		for (int i = 0; i < level; ++i)
			ss << indent;
		ss << "return 1.0 / (1.0 + math.exp(-" << var_name << ") )" << std::endl;

		level -= 1;
		for (int i = 0; i < level; ++i)
			ss << indent;
		ss << "end" << std::endl;
		return ss.str();
	}

	string to_java_code(bool cpp_version = false, int level = 0, int idx_start_from = 0, string indent = "  ",
			string var_name = "score") {

		std::stringstream ss;
		for (int i = 0; i < level; ++i)
			ss << indent;
		if (cpp_version) {

			ss << "float xgboost_predict(const std::vector<float>& fv){" << std::endl;
		} else {
			ss << "public static float xgboost_predict(float[] fv){" << std::endl;
		}
		level += 1;
		for (int i = 0; i < level; ++i)
			ss << indent;
		ss << "float " << var_name << " = 0.0f;" << std::endl;
		for (unsigned int i = 0; i < trees_.size(); ++i) {
			for (int i = 0; i < level; ++i)
				ss << indent;
			ss << "// tree " << (i + 1) << " / " << trees_.size() << std::endl;
			trees_[i].to_java_code(ss, NULL, level, idx_start_from, indent, var_name);
			ss << std::endl;
		}
		for (int i = 0; i < level; ++i)
			ss << indent;

		if (cpp_version)
			ss << "return 1.0f / (1.0f + (float)(std::exp(-" << var_name << ")) );" << std::endl;
		else
			ss << "return 1.0f / (1.0f + (float)(Math.exp(-" << var_name << ")) );" << std::endl;

		level -= 1;
		for (int i = 0; i < level; ++i)
			ss << indent;
		ss << "}" << std::endl;
		return ss.str();
	}
};

}/*xgboost*/
}

#endif /* LC_MACHINELEARNING_XGBOOST_PREDICTOR_HPP_ */
