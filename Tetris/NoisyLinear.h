#include <torch/torch.h>
#include <cmath>

struct NoisyFactorizedLinearImpl : torch::nn::Module {
    NoisyFactorizedLinearImpl(int64_t in_features, int64_t out_features, double sigma_zero = 1.0) :

        weight(torch::empty({ out_features, in_features })),
        sigma_weight(torch::full({ out_features, in_features }, sigma_zero / sqrt(in_features))),
        epsilon_weight(torch::zeros({ out_features, in_features })),

        bias(torch::empty({ out_features })),
        sigma_bias(torch::full({ out_features }, sigma_zero / sqrt(in_features))),
        epsilon_bias(torch::zeros({ out_features })),

        in_features(in_features), out_features(out_features)
    {
        register_parameter("weight", weight);
        register_parameter("sigma_weight", sigma_weight);

        register_parameter("bias", bias);
        register_parameter("sigma_bias", sigma_bias);

        register_buffer("epsilon_bias", epsilon_bias);
        register_buffer("epsilon_weight", epsilon_weight);

        reset_parameters();
        reset_noise();
    }
    void reset_parameters() {
        double std = 1 / sqrt(in_features);
        torch::nn::init::uniform_(weight, -std, std);
        torch::nn::init::uniform_(bias, -std, std);
    }

    torch::Tensor forward(torch::Tensor input) {

        return torch::nn::functional::linear(input, weight + sigma_weight * epsilon_weight, bias + sigma_bias * epsilon_bias);

    }

    void reset_noise()
    {
        torch::Tensor epsilon_in = scale_noise(in_features);
        torch::Tensor epsilon_out = scale_noise(out_features);
   
        epsilon_weight.copy_(epsilon_out.ger(epsilon_in));
        epsilon_bias.copy_(epsilon_out);
    }


    torch::Tensor scale_noise(size_t size)
    {
        auto x = torch::randn(size);
        return x.sign().mul(x.abs().sqrt());
    }

     
    int64_t in_features, out_features;
    torch::Tensor weight{nullptr}, sigma_weight{ nullptr }, epsilon_weight{ nullptr },
        bias{ nullptr }, sigma_bias{ nullptr }, epsilon_bias{ nullptr };
};


TORCH_MODULE(NoisyFactorizedLinear);