#include <deque>
#include <random>
#include <vector>
#include <torch/torch.h>

struct Transition {
    torch::Tensor state;
    torch::Tensor action;
    torch::Tensor next_state;
    torch::Tensor reward;   
    torch::Tensor done;
};

class ExperienceReplayMemory {
private:
    std::deque<Transition> memory;
    std::deque<float> priorities;
    size_t capacity;
    float a;
    std::random_device rd;
    std::mt19937 gen;
    float maxPriority;
    float sumProbabilities;
    float beta;
    float beta_increment;

public:
    ExperienceReplayMemory(size_t cap, float beta, float beta_increment, float a)
        : capacity(cap), gen(rd()), a(a),  maxPriority(1.0f), beta(beta), beta_increment(beta_increment)
    {}

    void push(torch::Tensor state,torch::Tensor action,torch::Tensor next_state, torch::Tensor reward, torch::Tensor done);

    std::tuple<std::vector<Transition>, std::vector<size_t>, std::vector<float>> sample( size_t batch_size);

    size_t size() const {
        return memory.size();
    }

    void updatePriorities(const std::vector<torch::Tensor>& priorities, const std::vector<size_t>& idx);




};