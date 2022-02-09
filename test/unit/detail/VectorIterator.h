/*******************************************************************************
 *
 * MIT License
 *
 * Copyright 2021-2022 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/

#ifndef WMMA_DETAIL_VECTOR_ITERATOR_H
#define WMMA_DETAIL_VECTOR_ITERATOR_H

#include "UnitKernelBase.h"
#include "device/VectorIterator.h"

namespace rocwmma
{

    // Wrapper into the actual device function
    template <uint32_t BlockM, uint32_t BlockN, typename DataT, typename Layout>
    struct VectorIteratorKernel final : public UnitKernelBase<BlockM, BlockN, DataT, Layout>
    {
    private:
        using Base = UnitKernelBase<BlockM, BlockN, DataT, Layout>;

    public:
        VectorIteratorKernel()        = default;
        ~VectorIteratorKernel() final = default;

        void setupImpl(typename Base::DataStorage::ProblemSize const& probsize) final
        {
            auto& dataInstance = Base::DataStorage::instance();

            // Initialize matrix storage
            const int64_t sizeD = Base::mM * Base::mN;
            dataInstance->resizeStorage(probsize);
        }

        void validateResultsImpl() final
        {
            auto& dataInstance = Base::DataStorage::instance();

            // Allocated managed memory for results on host
            const int64_t sizeD        = Base::mM * Base::mN;
            auto          kernelResult = dataInstance->template allocHost<DataT>(sizeD);

            // Cache current kernel result from device
            dataInstance->copyData(kernelResult, dataInstance->deviceOut(), sizeD);

            if(static_cast<double>(static_cast<float>(kernelResult[0]))
               != static_cast<double>(ERROR_VALUE))
            {
                Base::mValidationResult = true;
            }
        }

        typename Base::KernelFunc kernelImpl() const final
        {
            return typename Base::KernelFunc(VectorIterator<BlockM, DataT>);
        }
    };

    // This is the GeneratorImpl class
    struct VectorIteratorGenerator
    {
        // Indices to test parameters
        enum : uint32_t
        {
            BlockM = 0,
            BlockN = 1,
            DataT  = 2,
            Layout = 3
        };

        using ResultT = std::shared_ptr<KernelI>;

        template <typename... Ts>
        static ResultT generate(std::tuple<Ts...> testParams)
        {
            // Map GTest params to Kernel params
            using TestParamsT = std::tuple<Ts...>;
            using KernelT
                = VectorIteratorKernel<std::tuple_element_t<BlockM, TestParamsT>::value, // BlockM
                                       std::tuple_element_t<BlockN, TestParamsT>::value, // BlockN
                                       std::tuple_element_t<DataT, TestParamsT>, // DataT
                                       std::tuple_element_t<Layout, TestParamsT> // Layout
                                       >;

            return std::make_shared<KernelT>();
        }
    };

} // namespace rocwmma

#endif // WMMA_DETAIL_VECTOR_ITERATOR_H
