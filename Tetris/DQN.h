#pragma once

#include <torch/torch.h>

struct DQN : torch::nn::Module {
    DQN(size_t input_channels, size_t num_actions)
    {
        conv1 = register_module("conv1", torch::nn::Conv2d(torch::nn::Conv2dOptions(input_channels, 32, 8).stride(4)));
        conv2 = register_module("conv2", torch::nn::Conv2d(torch::nn::Conv2dOptions(32, 64, 4).stride(2)));
        conv3 = register_module("conv3", torch::nn::Conv2d(torch::nn::Conv2dOptions(64, 64, 3).stride(1)));
        linear1 = register_module("linear1", torch::nn::Linear(64 * 7 * 7, 512));
        output = register_module("output", torch::nn::Linear(512, num_actions));
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

    torch::nn::Conv2d conv1{ nullptr }, conv2{ nullptr }, conv3{nullptr};
    torch::nn::Linear linear1{ nullptr }, output{nullptr};
};