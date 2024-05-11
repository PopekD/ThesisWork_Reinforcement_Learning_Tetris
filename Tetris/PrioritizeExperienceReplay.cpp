#include "PrioritizeExperienceReplay.h"

void ExperienceReplayMemory::push(torch::Tensor state,torch::Tensor action,torch::Tensor next_state, torch::Tensor reward, torch::Tensor done)
{
    if (memory.size() == capacity) {
        memory.pop_front();
        priorities.pop_front();
    }
    memory.push_back(Transition(state, action, next_state, reward, done));
    priorities.push_back(std::max(maxPriority, 1.0f));
}

std::tuple<std::vector<Transition>, std::vector<size_t>, std::vector<float>> ExperienceReplayMemory::sample(size_t batch_size) {

    std::vector<Transition> batch(batch_size);
    std::vector<size_t> indices(batch_size);
    std::vector<float> probabilities(memory.size());
    std::vector<float> weights(batch_size);

    
    float p_Sum = std::accumulate(priorities.begin(), priorities.end(), 0.0f);

    for (size_t i = 0; i < priorities.size(); ++i) {
        probabilities[i] = std::pow(priorities[i], a) / std::pow(p_Sum, a); 
    }

    std::discrete_distribution<> dist(probabilities.begin(), probabilities.end());

    float max_weight = 0;
    beta = std::min(1.0f, beta + beta_increment);

    for (uint16_t i = 0; i < batch_size; i++) {

        size_t index = dist(gen);
        batch[i] = memory[index];
        indices[i] = index;
        weights[i] = std::pow(batch_size * probabilities[index] , -beta);
        max_weight = std::max(max_weight, weights[i]);
    }


    for (float& element : weights) { element /= max_weight; }

    return { batch, indices, weights};

}

void ExperienceReplayMemory::updatePriorities(const std::vector<torch::Tensor>& p_vector, const std::vector<size_t>& idx)
{
    
    uint8_t iterator = 0;
    for (torch::Tensor p : p_vector)
    {
        priorities[idx[iterator]] = p.item<float>();
        maxPriority = std::max(maxPriority, p.item<float>());
        iterator++;
    }


}





