#pragma once

#include <torch/torch.h>
#include "NoisyLinear.h"


struct DQNImpl: torch::nn::Module {
    DQNImpl(size_t input_channels, size_t num_actions):
        conv1(torch::nn::Conv2dOptions(input_channels,32,8).stride(4)),
        conv2(torch::nn::Conv2dOptions(32, 64, 4).stride(2)),
        conv3(torch::nn::Conv2dOptions(64, 64, 3).stride(1)),
        linear1(64 * 7 * 7, 512),
        output(512, num_actions)
    {
        register_module("conv1", conv1);
        register_module("conv2", conv2);
        register_module("conv3", conv3);
        register_module("linear1", linear1);
        register_module("output", output);
    }

    torch::Tensor forward(torch::Tensor input) {

        input = torch::relu(conv1->forward(input));
        input = torch::relu(conv2->forward(input));
        input = torch::relu(conv3->forward(input));
        input = input.view({ input.size(0), -1 });
        input = torch::relu(linear1->forward(input));
        input = output->forward(input);

        return input;
    }

    //void reset_noise()
    //{
    //    linear1->reset_noise();
    //    output->reset_noise();
    //}

    torch::nn::Conv2d conv1{ nullptr }, conv2{ nullptr }, conv3{nullptr};
    torch::nn::Linear linear1{ nullptr }, output{nullptr};
   
};

TORCH_MODULE(DQN);