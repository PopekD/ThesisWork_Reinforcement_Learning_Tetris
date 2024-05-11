#pragma once

#include <torch/torch.h>
#include <chrono>
#include "PrioritizeExperienceReplay.h"
#include "DQN.h"
#include "RL_Play.h"
#include "random"
#include <fstream>


class Train {

private:
	size_t BATCH_SIZE;
	float GAMMA;
	float EPS_START;
	float EPS_END;
	float EPS_DECAY;
	float TAU;
	float LR;
	float OFFSET;
	size_t TRAIN_START_STP;
	size_t UPDATE_TARGET_STP;
	size_t TRAIN_EVERY_STP;
	uint8_t FRAME_SKIP;

	torch::Device device;
	ExperienceReplayMemory buffer;

	DQN policy_net;
	DQN target_net;

	uint64_t num_steps;
	int steps_done = 0;
	float n_updates = 0;
	float eps_threshold;

	torch::optim::Adam dqn_optimizer;
	torch::nn::SmoothL1Loss criterion;
	torch::Tensor loss;
	std::vector<float> losses;
	std::vector<sf::Uint8>output;

	std::random_device rd;
	std::mt19937 gen;
	std::uniform_real_distribution<> dis;

	std::chrono::time_point<std::chrono::system_clock> start;
	std::chrono::duration<double> elapsed_seconds;

	int num_actions;


public:
	Train(size_t input_channels, size_t num_actions, size_t capacity, uint64_t num_steps,
		const size_t BATCH_SIZE = 32,
		const float GAMMA = 0.99f,
		const float EPS_START = 1.0f,
		const float EPS_END = 0.05f,
		const size_t EPS_DECAY = 150000,
		const float LR = 0.0001f,
		const float OFFSET = 0.000001f,
		const float A = 0.4f,
		const float BETA = 0.1f,
		const float BETA_INCREMENT = 0.00000009,
		const size_t TRAIN_START_STP = 100,
		const size_t UPDATE_TARGET_STP = 10000,
		const size_t TRAIN_EVERY_STP = 4,
		const uint8_t FRAME_SKIP = 4
	) :
		BATCH_SIZE(BATCH_SIZE), GAMMA(GAMMA), EPS_START(EPS_START), EPS_END(EPS_END), EPS_DECAY(EPS_DECAY), TAU(TAU), LR(LR), OFFSET(OFFSET), TRAIN_START_STP(TRAIN_START_STP), UPDATE_TARGET_STP(UPDATE_TARGET_STP), TRAIN_EVERY_STP(TRAIN_EVERY_STP), FRAME_SKIP(FRAME_SKIP),
		device(torch::kCUDA),
		buffer(capacity, BETA, BETA_INCREMENT, A),
		num_steps(num_steps),
		num_actions(num_actions),
		eps_threshold(TAU),
		gen(rd()),
		policy_net(DQN(input_channels, num_actions)),
		target_net(DQN(input_channels, num_actions)),
		dqn_optimizer(policy_net->parameters(), torch::optim::AdamOptions(LR)),
		criterion(torch::nn::SmoothL1LossOptions().reduction(torch::kNone).beta(1.0f)) {
		policy_net->to(device);
		target_net->to(device);
		load_state_dict();

	}

	void optimize_model() {

		if (buffer.size() < BATCH_SIZE) { return; };

		std::tuple<std::vector<Transition>, std::vector<size_t>, std::vector<float> > sample = buffer.sample(BATCH_SIZE);
		std::vector<Transition> batch = std::get<0>(sample);
		std::vector<size_t> idx = std::get<1>(sample);
		std::vector<float> weights = std::get<2>(sample);


		std::vector<torch::Tensor> next_state_batch, state_batch, action_batch, reward_batch, non_final_mask, done_batch;
		torch::Tensor weights_Tensor = torch::from_blob(weights.data(), weights.size()).clone();
		weights_Tensor = weights_Tensor.to(device);


		for (const Transition t : batch) {

			state_batch.push_back(t.state);
			action_batch.push_back(t.action);
			reward_batch.push_back(t.reward);
			next_state_batch.push_back(t.next_state);
			done_batch.push_back(t.done);

		}

		torch::Tensor state_batch_concatenated = torch::cat(state_batch, 0).to(device);
		torch::Tensor reward_batch_concatenated = torch::cat(reward_batch, 0).to(device);
		torch::Tensor action_batch_concatenated = torch::cat(action_batch, 0).to(device);
		torch::Tensor next_state_concatenated = torch::cat(next_state_batch, 0).to(device);
		torch::Tensor done_batch_concatenated = torch::cat(done_batch, 0).to(device);


		torch::Tensor q_values = policy_net->forward(state_batch_concatenated).gather(1, action_batch_concatenated).to(device);
		q_values = q_values.squeeze(1);
		torch::Tensor next_q_values;
		{
			torch::NoGradGuard no_grad;
			next_q_values = std::get<0>(torch::max(target_net->forward(next_state_concatenated), 1));
		}

		torch::Tensor expected_q_values = reward_batch_concatenated + (GAMMA * next_q_values * (1 - done_batch_concatenated));

		torch::Tensor deltas = expected_q_values - q_values;

		torch::Tensor p = torch::abs(deltas) + OFFSET;
		std::vector<torch::Tensor> seq = torch::unbind(p);
		buffer.updatePriorities(seq, idx);

		loss = criterion(q_values, expected_q_values);
		loss = loss * weights_Tensor;
		loss = loss.mean();

		losses.push_back(loss.item<float>());

		policy_net->zero_grad();

		loss.backward();

		torch::nn::utils::clip_grad_norm_(policy_net->parameters(), 10);

		dqn_optimizer.step();

		//policy_net->reset_noise();
		//target_net->reset_noise();


	};

	torch::Tensor selectAction(torch::Tensor& state) {

		float sample = dis(gen);
		eps_threshold = EPS_END + (EPS_START - EPS_END) *
			std::exp(-1.0f * static_cast<float>(steps_done) / EPS_DECAY);
		steps_done++;

		if (sample > eps_threshold) {
			{
				torch::NoGradGuard no_grad;
				torch::Tensor action = policy_net->forward(state).to(device);
				auto valueMax = torch::max(action, 1);
				torch::Tensor value = std::get<1>(valueMax);

				value = value.view({ 1,1 });

				value = value.to(device);

				return value;
			}
		}
		else {
			return torch::tensor({ { rand() % num_actions } }, torch::kLong).to(device);
		}
	}


	void train() {

		RL_Play env;
		torch::Tensor action;
		float cum_reward = 0;
		int frame = 0;

		for (uint64_t i = 0; ; i++)
		{
			if (steps_done >= num_steps) { break; }

			torch::Tensor state = env.restart();

			bool done = false;

			float episode_reward = 0;

			start = std::chrono::system_clock::now();


			for (uint64_t t = 0; ; t++) {

				if (frame % FRAME_SKIP == 0) { action = selectAction(state); }

				std::tuple<torch::Tensor, float, bool, bool> step_result;

				step_result = env.step(action.item<int>(), frame % FRAME_SKIP != 0);

				bool done = std::get<2>(step_result);
				bool scored = std::get<3>(step_result);
				float reward = std::get<1>(step_result);


				cum_reward += reward;

				if (frame % FRAME_SKIP == 0)
				{

					torch::Tensor observation = std::get<0>(step_result);
					torch::Tensor reward_tensor = torch::tensor({ cum_reward }, device);
					torch::Tensor done_tensor = torch::tensor({ done });

					done_tensor = done_tensor.to(torch::kFloat32);
					done_tensor = done_tensor.to(device);

					buffer.push(state, action, observation, reward_tensor, done_tensor);


					episode_reward += cum_reward;

					cum_reward = 0.f;

					state = observation;

				}

				if (steps_done > TRAIN_START_STP && frame % TRAIN_EVERY_STP == 0) { optimize_model(); }

				if (steps_done % UPDATE_TARGET_STP == 0) { load_state_dict(); }

				frame++;

				if (done)
				{
					elapsed_seconds = std::chrono::system_clock::now() - start;
					break;
				}
			}
			std::cout << "Episodes Number: " << i << std::endl;
			std::cout << "Overall Steps Done: " << steps_done << std::endl;
			std::cout << "Episode Reward: " << episode_reward << std::endl;
			std::cout << "Episode Length: " << elapsed_seconds.count() << std::endl;
			std::cout << "Mean Loss: " << mean() << std::endl;
			std::cout << "Epsilon Value: " << eps_threshold << std::endl;
			std::cout << std::endl;

			appendToLogFile("training_log.txt", i, steps_done, episode_reward, elapsed_seconds.count(), mean(), eps_threshold);

		}
		torch::save(policy_net, "model.pt");
		torch::save(dqn_optimizer, "optim.pt");

		std::cout << "COMPLITED";

	}

	bool are_networks_identical(auto param1, auto param2) {

		auto policy_params = policy_net->parameters();
		auto target_params = target_net->parameters();


		auto policy_param_it = policy_params.begin();
		auto target_param_it = target_params.begin();

		while (policy_param_it != policy_params.end() && target_param_it != target_params.end()) {
			if (!(*policy_param_it).allclose(*target_param_it)) {
				return false;
			}
			++policy_param_it;
			++target_param_it;
		}


		return true;
	}

	void load_state_dict()
	{
		torch::NoGradGuard no_grad;

		auto new_params = target_net->named_parameters();
		auto params = policy_net->named_parameters(true);
		auto buffers = policy_net->named_buffers(true);

		for (auto& val : new_params) {
			auto name = val.key();
			auto* t = params.find(name);
			if (t != nullptr) {
				t->copy_(val.value());
			}
			else {
				t = buffers.find(name);
				if (t != nullptr) {
					t->copy_(val.value());
				}
			}
		}
	}

	double mean()
	{
		double sum = std::accumulate(losses.begin(), losses.end(), 0.0f);
		return sum / losses.size();
	}

	void appendToLogFile(const std::string& filename, int episode_number, int steps_done, float episode_reward, int episode_length, float mean_loss, float epsilon_value) {
		std::ofstream file;
		file.open(filename, std::ios_base::app); 
		if (file.is_open()) {
			file << "Episodes Number: " << episode_number << std::endl;
			file << "Overall Steps Done: " << steps_done << std::endl;
			file << "Episode Reward: " << episode_reward << std::endl;
			file << "Episode Length: " << episode_length << std::endl;
			file << "Mean Loss: " << mean_loss << std::endl;
			file << "Epsilon Value: " << epsilon_value << std::endl;
			file << std::endl;
			file.close();
		}
		else {
			std::cerr << "Unable to open file for appending!" << std::endl;
		}
	}

};