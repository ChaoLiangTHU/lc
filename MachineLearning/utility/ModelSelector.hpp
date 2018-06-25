/*
 * ModelSelector.hpp
 *
 *  Created on: May 10, 2017
 *      Author: colinliang
 */

#ifndef LC_MACHINELEARNING_UTILITY_MODELSELECTOR_HPP_
#define LC_MACHINELEARNING_UTILITY_MODELSELECTOR_HPP_

#include <unistd.h>  // 更新到gcc 4.8后 ::sleep(second)    可以更换为c++11的标准库 std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds))
#include <functional>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
//#include <shared_mutex>
#include <algorithm>
#include <sstream>
#include "../../utility/LogUtil.hpp"
#include "../../utility/FileUtil.hpp"
namespace LC {

/**
 * 双模型切换系统， 模型类需要有
 * 1、默认构造函数，小开销的，也会作为清楚旧的模型的操作；
 * 2、loadConfigs函数， 用于加载模型，该函数接受一个字符串变量（文件夹）作为参数，加载磁盘上的文件
 * 模型文件夹内 要求有一个文件（对应参数modelCheckUpFile（为文件的basename））作为模型完整性的验证
 * 模型的使用：
 * 1、通过 getMutex()获取对应的mutex；  （当前采用timed_mutex实现，如果能用C++14，可以换为采用shared_timed_mutex实现）
 * 2、再进行lock；
 * 3、通过getModel获得当前model的reference， 使用模型
 * 4、释放锁
 * 	示例用法：

	LC::ModelSelector<ModelT> ms;
	ms.checkNewModelPerSeconds = 10;
	ms.modelIsDir = true;
	ms.modelParentFolder = "/home/colinliang/tmp/models";

	ms.modelPrefix = "net_model";
	ms.modelCheckUpFile = "modelVersion.txt";
//	ms.modelVersionCheckUpFile="v4.txt";

	ms.process_idx=1;
	ms.process_num=2;

	ms.loadModelAndStartDetectingNewModel();
	while (true) {
		::sleep(3);
		LC::Timer t;
		for (int i = 0; i < 1; ++i){
			auto& mut=ms.getMutex();
			if (mut.try_lock_for(std::chrono::milliseconds(100))) {
				std::cout << "PCTR: " << ms.getModel(&mut).predict() << std::endl;
				mut.unlock();
			} else {
				lclogf("Fail to get lock");
			}
		}
//		std::cout<<t.getElapseTime_in_second_and_restart()<<"s"<<std::endl;

	}
 */
template<typename ModelClass>
class ModelSelector {
	int numModelLoad;
public:

	std::string modelParentFolder;
	std::string modelPrefix;

	std::string modelVersionCheckUpFile; // 模型版本好check file， 默认是空，表示不检测任何文件
	std::string modelCheckUpFile; // 含有该文件的文件夹才是有效的模型文件夹

	ModelClass model1, model2;
	long long model1_load_time, model2_load_time; //模型状态监控，当重置模型为空后，会将对应值设为true
	std::timed_mutex model1mutex, model2mutex;  //更新到C++14后可以换为shared_timed_mutex
//	string model1version, model2version;

	string modelVersion; // 当前已经加载的模型所在的路径
	ModelClass* pCurModel;

	bool modelIsDir;
	std::thread* pLoadModelThread; //由于析构时必须时unjoinable状态（工作已经做完，或者转交给其他thread，或detach了工作），
	//考虑到该线程使用了ModelSelector的资源，detach会出bug；如果自己做join，有必须经常检测是否要退出，所以，ModelSelector类的析构函数中不对此进行析构（毕竟每个ModelSelector的生命周期应该和进程一样长），而是交给系统回收
	// 参见http://www.w2bc.com/article/224795

//	std::string lastModelFileOrDir; //用于删除旧的模型文件
	int numModel2keep; //保留多少份模型文件
	int checkNewModelPerSeconds; //每隔多少秒检测一次新模型
	int process_idx;  // 当前进程在这组需要加载同一个模型的进程中的序号， 该值和process_num是为了分散加载模型，防止内存溢出
	int process_num;  // 一共有多少个模型会加载同一个模型

	int tryReleaseOldModelAfterSeconds; //成功加载新模型后，多少秒之后尝试释放旧的模型，，该段时间用于保证外部调用时不采用mutex模式下的“安全”释放

//	string modelState; //当前模型的状态信息，每次加载模型后会进行更新
	long long next_model_load_time;
	ModelSelector() :
			numModelLoad(0), modelParentFolder(""), modelPrefix("net_model"), modelVersionCheckUpFile(""), modelCheckUpFile(
					"ModelSentinel.txt"), model1_load_time(-1), model2_load_time(-1), modelIsDir(true), pCurModel(
			NULL), pLoadModelThread(NULL), numModel2keep(2), checkNewModelPerSeconds(1800), process_idx(0), process_num(
					1), tryReleaseOldModelAfterSeconds(1), next_model_load_time(-1) {
		lclogfl("ModelSelector constructor at ptr: %ulld", (unsigned long long )(this));
	}

	~ModelSelector() {
		lclogfl("ModelSelector destructor at ptr: %ulld", (unsigned long long )(this));
//		if (pLoadModelThread) {
//			lclogfl("Delete load model thread at ptr: %ulld",(unsigned long long)(pLoadModelThread));
//			delete pLoadModelThread;
//			pLoadModelThread = NULL;
//		}
	}
	ModelClass& getModel() {
		return *pCurModel;
	}

	ModelClass& getModel(std::timed_mutex* pMutex){
		if(pMutex==&model2mutex){
			return model2;
		}else {
			return model1;
		}
	}

	std::timed_mutex& getMutex() {
		if (pCurModel == &model2)
			return model2mutex;
		else
			return model1mutex;
	}

	std::timed_mutex& getMutex(ModelClass* pModel) {
		if (pModel == &model2)
			return model2mutex;
		else
			return model1mutex;
	}

//	void update_model_state(long long next_model_load_time=-1) {
//		std::stringstream ss;
//		if (pCurModel == NULL) {
//			ss << "cur model: empty; ";
//		} else if (pCurModel == &model1) {
//			ss << "cur model: model1; ";
//		} else if (pCurModel == &model2) {
//			ss << "cur model: model2; ";
//		} else {
//			ss << "cur model: invalid state; ";
//		}
//		ss << "model version: " << modelVersion << ";\n";
//
////		long long cur_time = std::time(NULL);
//		if (model1_load_time < 0) {
//			ss << "model1 is empty; ";
//		} else {
//			ss << "model1 load time: " << model1_load_time << "; ";
//		}
//
//		if (model2_load_time < 0) {
//			ss << "model2 is empty; ";
//		} else {
//			ss << "model1 load time: " << model2_load_time << "; ";
//		}
//
//		ss << "load next model at: " << next_model_load_time << ";";
//		modelState = ss.str();
//	}
//
//	inline string get_model_state() {
//		auto cur_time = std::time(NULL);
//		std::stringstream ss;
//		ss << modelState << "\ncur_time: " << cur_time;
//		return ss.str();
//	}

	inline string get_model_state() {
		std::stringstream ss;
		if (pCurModel == NULL) {
			ss << "cur model: empty; ";
		} else if (pCurModel == &model1) {
			ss << "cur model: model1; ";
		} else if (pCurModel == &model2) {
			ss << "cur model: model2; ";
		} else {
			ss << "cur model: invalid state; ";
		}
		ss << "model version: " << modelVersion << ";\n";

		long long cur_time = std::time(NULL);
		if (model1_load_time < 0) {
			ss << "model1 is empty; ";
		} else {
			ss << "model1 load time: " << model1_load_time << " (" << (cur_time - model1_load_time) << "s ago)" << "; ";
		}

		if (model2_load_time < 0) {
			ss << "model2 is empty; ";
		} else {
			ss << "model2 load time: " << model2_load_time << " (" << (cur_time - model2_load_time) << "s ago)" << "; ";
		}

		ss << "load next model at: " << next_model_load_time << " (" << (next_model_load_time - cur_time) << "s later)"
				<< "; ";
		ss << "model update serial: " << process_idx << " / " << process_num << "; ";

		return ss.str();
	}

	void operator()() {
		while (true) {
			long long cur_time = std::time(NULL);
			long long time2wait = checkNewModelPerSeconds - cur_time % checkNewModelPerSeconds;
			while (time2wait < 0)
				time2wait += checkNewModelPerSeconds;
			time2wait += process_idx * checkNewModelPerSeconds / process_num;
			if (time2wait - checkNewModelPerSeconds > 10)
				time2wait -= checkNewModelPerSeconds;
			next_model_load_time = cur_time + time2wait;
#ifdef LCDebug
			lclognvl(get_model_state());
			lclognvl(cur_time);
			lclognvl(time2wait);
#endif

			::sleep(time2wait); // 更新到gcc 4.8后 ::sleep(second)    可以更换为c++11的标准库 std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds))
			loadNewModel();
			::sleep(3); // 如果外部使用model时忘记加锁了，sleep 1秒可以防止一定的误操作
			try_to_releaseUnusedModel(10000);
			if (model1_load_time > 0 && model2_load_time > 0) {
				lclogvl("try 2nd time to release unused model");
				try_to_releaseUnusedModel(100000);
			}
		}
	}

	/**
	 *
	 * @param models
	 */
	void remove_old_model_on_disk(const vector<string>& models) {
		int modelsize = models.size();
		if (numModel2keep < 0 || modelsize > numModel2keep) {
			int n2rm = models.size() - numModel2keep;
			for (int i = 0; i < n2rm; ++i) {
				int state = LC::Directory::rm(models[i]);
				if (state == 0) {
					lclogfl("SUCCESS: delete model on disk: %s", models[i].c_str());
				} else {
					lclogfl("FAIL: delete model on disk: %s", models[i].c_str());
				}
			}
		}

	}

	void try_to_releaseUnusedModel(int timeout_milliseconds = 10000) {
		if (pCurModel == &model1) {
			if (model2mutex.try_lock_for(std::chrono::milliseconds(timeout_milliseconds))) {
				lclogfl("set model2 to empty model");
				model2 = ModelClass();
				model2_load_time = -1;
				model2mutex.unlock();
			} else {
				lclogfl("FAIL: time out for set model2 to empty model.");
			}
		} else if (pCurModel == &model2) {
			if (model1mutex.try_lock_for(std::chrono::milliseconds(timeout_milliseconds))) {
				lclogfl("set model1 to empty model");
				model1 = ModelClass();
				model1_load_time = -1;
				model1mutex.unlock();
			} else {
				lclogfl("FAIL: time out for set model1 to empty model.");
			}
		}
	}

	void loadNewModel() {
		if (!modelIsDir) {
			lclogfl("only support model in a folder now");
			lclogpos();
			std::cerr << "only support model in a folder now" << std::endl;
		}

		vector<string> modelCandidates = LC::Directory::listDirs_startswith_fullpath(modelParentFolder, modelPrefix);
#ifdef LCDebug
		std::sort(modelCandidates.data(), modelCandidates.data() + modelCandidates.size());
		std::cout << "candidate models: " << LC::Str::vec2str(modelCandidates) << std::endl;
#endif
		vector<string> models;
		vector<string> invalidModels;
		for (unsigned int i = 0; i < modelCandidates.size(); ++i) {
			string m = modelCandidates[i];
			bool is_valid_model = true;

			string chk = LC::Directory::join(m, modelCheckUpFile);
			is_valid_model &= LC::Directory::isfile(chk);

			string chk_version = LC::Directory::join(m, modelVersionCheckUpFile);
			is_valid_model &= modelVersionCheckUpFile.size() == 0u || LC::Directory::isfile(chk_version);
//			std::cout << chk << std::endl;

			if (is_valid_model)
				models.push_back(m);
			else
				invalidModels.push_back(m);

		}

		std::sort(models.data(), models.data() + models.size());
		if (invalidModels.size() > 1)
			std::sort(invalidModels.data(), invalidModels.data() + invalidModels.size());
#ifdef LCDebug
		std::cout << "valid models: " << LC::Str::vec2str(models) << std::endl;
		std::cout<<"invalid models: "<<LC::Str::vec2str(invalidModels)<<std::endl;
		std::cout.flush();
#endif
		if (models.size() == 0) {
//			std::cerr << "no models in folder: " << modelParentFolder << std::endl;
			lclogf("no models in folder: %s\n", modelParentFolder.c_str());
			lclogpos();
			throw "no model";
		}

		string newestModel = models[models.size() - 1];
		int state = newestModel.compare(modelVersion);
		if (modelVersion.compare("") == 0)
			state = 1;
		if (state < 0) {
			std::cerr << "newest model older than current model......" << std::endl;
			lclogf("newest model older than current model......\n");
			return;
		} else if (state == 0) {
//			lclogf("no new model\n");
			if (pCurModel == &model1) {
				lclogf("no new model, cur model is model1\n");
			} else if (pCurModel == &model2) {
				lclogf("no new model, cur model is model2\n");
			}
			return;
		}

		auto cur_time = std::time(NULL);
		if (!pCurModel) {
			lclogf("no model loaded, load the first model into model1\n");
			model1 = ModelClass();
			int ret = model1.loadConfigs(newestModel);
			if (ret == 0) {
				pCurModel = &model1;
				model1_load_time = cur_time;
				modelVersion = newestModel;
				numModelLoad += 1;
				remove_old_model_on_disk(models);

				lclogf("SUCCESS: load model1: %s \n", newestModel.c_str());
			} else {
				lclogf("FAIL: load model1: %s, keep last loaded model in model2 \n", newestModel.c_str());
				model1 = ModelClass();
				throw "no model loaded!";
			}

		} else if (pCurModel == &model1) {
			lclogf("cur model is model1, loading into model2 now.\n");
			model2 = ModelClass();
			try {
				int ret = model2.loadConfigs(newestModel);
				if (ret == 0) {
					model2_load_time = cur_time;
					pCurModel = &model2;
					modelVersion = newestModel;
					numModelLoad += 1;
					remove_old_model_on_disk(models);
					remove_old_model_on_disk(invalidModels);
					lclogf("SUCCESS: load model2: %s \n", newestModel.c_str());
				} else {
					lclogf("FAIL: load model2: %s, keep using last loaded model in model1\n", newestModel.c_str());
					model2 = ModelClass();
				}
			} catch (...) {
				model2 = ModelClass();
			}

		} else if (pCurModel == &model2) {
			lclogf("cur model is model2, loading into model1 now.\n");
			model1 = ModelClass();
			try {
				int ret = model1.loadConfigs(newestModel);
				if (ret == 0) {
					model1_load_time = cur_time;
					pCurModel = &model1;
					numModelLoad += 1;
					modelVersion = newestModel;
					remove_old_model_on_disk(models);
					remove_old_model_on_disk(invalidModels);
					lclogf("SUCCESS: load model1: %s \n", newestModel.c_str());
				} else {
					lclogf("FAIL: load model1: %s, keep using last loaded model in model2\n", newestModel.c_str());
					model1 = ModelClass();
				}
			} catch (...) {
				model1 = ModelClass();
			}

		} else {
			lclogf("INVALID pCurmodel: %ulld\n", (unsigned long long ) (pCurModel));
			lclogf("\tmodel1: %ulld\n", (unsigned long long ) (&model1));
			lclogf("\tmodel2: %ulld\n", (unsigned long long ) (&model2));
			lclogpos();
			std::cerr << "INVALID pCurmodel:" << (unsigned long long) (pCurModel) << std::endl;
			std::cerr << "pModel1 is " << (unsigned long long) (&model1) << std::endl;
			std::cerr << "pModel2 is " << (unsigned long long) (&model2) << std::endl;
			throw "INVALID pCurmodel";
		}
	}

	void loadModelAndStartDetectingNewModel() {
		if (pCurModel == NULL)
			loadNewModel();
		if (pLoadModelThread == NULL)
			pLoadModelThread = new std::thread(std::ref(*this));
	}

};

}
#endif /* LC_MACHINELEARNING_UTILITY_MODELSELECTOR_HPP_ */
