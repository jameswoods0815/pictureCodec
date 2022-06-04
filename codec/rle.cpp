#include <vector>
#include <bitset>
#include <cmath>
#include <queue>
#include <iostream>
#include <cassert>
#include <memory>

static const uint8_t EOB = 0x00;

class HuffmanDictionary {
public:
	HuffmanDictionary(const std::vector<uint8_t> &data)
	{
		std::vector<size_t> frequencies(256, 0);

		for (const uint8_t x : data) {
			frequencies[x]++;
		}

		std::priority_queue<InternalHuffmanTreeNode::pointer, std::vector<InternalHuffmanTreeNode::pointer>, InternalHuffmanTreeNode::Comparer> pq;

		for (size_t i = 0; i < 256; i++) {
			pq.push(std::make_shared<InternalHuffmanTreeNode>(i, frequencies[i]));
		}

		while (pq.size() > 1) {
			InternalHuffmanTreeNode::pointer lchild = pq.top();
			pq.pop();

			InternalHuffmanTreeNode::pointer rchild = pq.top();
			pq.pop();

			InternalHuffmanTreeNode::pointer node = std::make_shared<InternalHuffmanTreeNode>(lchild, rchild);
			pq.push(std::move(node));
		}

		InternalHuffmanTreeNode::pointer root = pq.top();
		pq.pop();

		this->decodeTree_ = root->ConvertToHuffmanTreeNode();

		this->visitDecodeTree(this->decodeTree_);
	}
	virtual ~HuffmanDictionary()
	{
	}

	std::vector<uint8_t> encode(const std::vector<uint8_t> &data)
	{
		// The last byte in data must be EOB.
		assert(data.back() == EOB);

		std::vector<uint8_t> result(1, 0);
		size_t pos = 0;

		for (const uint8_t x : data) {

			const std::pair<std::vector<uint8_t>, size_t> &huffCode = this->encodeDic_[x];

			for (size_t i = 0; i < huffCode.first.size() - 1; i++) {
				result.back() |= huffCode.first[i] >> pos;
				result.push_back(huffCode.first[i] << (8 - pos));
			}

			result.back() |= huffCode.first.back() >> pos;

			if (pos + huffCode.second < 8) {
				pos += huffCode.second;
			} else {
				result.push_back(huffCode.first.back() << (8 - pos));
				pos -= 8 - huffCode.second;
			}
		}

		return result;
	}

	std::vector<uint8_t> decode(const std::vector<uint8_t> &coded)
	{
		std::vector<uint8_t> result;
		size_t i = 0;

		do {
			std::shared_ptr<HuffmanTreeNode> node = this->decodeTree_;

			do {
				node = (coded[i / 8] & (0x1 << (7 - (i % 8)))) == 0 ? node->lchild_ : node->rchild_;
				i++;
				assert(node != nullptr);
			} while (node->lchild_ != nullptr || node->rchild_ != nullptr);

			result.push_back(node->value_);
		} while (result.back() != EOB);

		return result;
	}

private:
	struct HuffmanTreeNode {
		uint8_t value_;
		std::shared_ptr<HuffmanTreeNode> lchild_ = nullptr;
		std::shared_ptr<HuffmanTreeNode> rchild_ = nullptr;
	};
	class InternalHuffmanTreeNode {
	public:
		typedef std::shared_ptr<InternalHuffmanTreeNode> pointer;

		bool isLeaf_ = false;
		uint8_t value_ = 0;
		size_t freq_ = 0;
		InternalHuffmanTreeNode::pointer lchild_ = nullptr;
		InternalHuffmanTreeNode::pointer rchild_ = nullptr;

		struct Comparer {
			bool operator()(const InternalHuffmanTreeNode::pointer &v1, const InternalHuffmanTreeNode::pointer &v2)
			{
				return v1->freq_ > v2->freq_;
			}
		};

		InternalHuffmanTreeNode(uint8_t v, size_t freq) : isLeaf_(true), value_(v), freq_(freq)
		{
		}

		InternalHuffmanTreeNode(const std::shared_ptr<InternalHuffmanTreeNode> &lchild, const std::shared_ptr<InternalHuffmanTreeNode> &rchild)
		 : freq_((lchild == nullptr? 0 : lchild->freq_) + (rchild == nullptr ? 0 : rchild->freq_)), lchild_(lchild), rchild_(rchild)
		{
		}

		std::shared_ptr<HuffmanTreeNode> ConvertToHuffmanTreeNode()
		{
			std::shared_ptr<HuffmanTreeNode> node = std::make_shared<HuffmanTreeNode>();

			node->value_ = this->value_;

			if (this->lchild_ != nullptr) {
				node->lchild_ = this->lchild_->ConvertToHuffmanTreeNode();
			}

			if (this->rchild_ != nullptr) {
				node->rchild_ = this->rchild_->ConvertToHuffmanTreeNode();
			}

			return node;
		}
	};
	std::shared_ptr<HuffmanTreeNode> decodeTree_;
	std::vector<std::pair<std::vector<uint8_t>, size_t>> encodeDic_ = std::vector<std::pair<std::vector<uint8_t>, size_t>>(256);
	std::vector<bool> signpost_;

	void visitDecodeTree(const std::shared_ptr<HuffmanTreeNode> &node)
	{
		if (node->lchild_ != nullptr) {
			this->signpost_.push_back(false);
			this->visitDecodeTree(node->lchild_);
		}

		if (node->rchild_ != nullptr) {
			this->signpost_.push_back(true);
			this->visitDecodeTree(node->rchild_);
		}

		if (node->lchild_ == nullptr && node->rchild_ == nullptr) {
			this->encodeDic_[node->value_].second = this->signpost_.size() % 8;
			this->encodeDic_[node->value_].first = std::vector<uint8_t>((this->signpost_.size() + 7) / 8, 0);

			for (size_t i = 0; i < this->signpost_.size(); i++) {
				this->encodeDic_[node->value_].first[i / 8] |= this->signpost_[i] << (7 - (i % 8));
			}
		}

		if (this->signpost_.size() > 0) {
			this->signpost_.pop_back();
		}
	}
};

uint8_t vli(const int16_t x, uint16_t &dst)
{
	if (x == 0) {
		dst = 0;
		return 0;
	}

	uint8_t group = 0;

	{
		const int16_t absX = x > 0 ? x : -x;

		for (int i = 3; i >= 0; i--) {

			const size_t expI = 0x1 << i;

			if ((0x1 << (group + expI)) <= absX) {
				group += expI;
			}
		}
	}

	group++;

	dst = x < 0 ? x + (0x1 << group) - 1 : x;

	return group;
}

int16_t devli(const uint16_t x, const uint8_t group)
{
	if (group == 0) {
		return 0;
	}

	const uint16_t mask = (0x1 << (group - 1)) - 1;
	const int16_t ret = (x & (0x1 << (group - 1))) != 0 ? x : (*(int16_t *)&x) - (0x1 << group) + 1;

	return ret;
}

std::pair<std::vector<uint8_t>, std::vector<uint16_t>> rle(const int16_t * mem, size_t cnt)
{
	std::vector<uint8_t> strides;
	std::vector<uint16_t> vliData(1, 0);
	uint8_t vliDataBitPos = 0;

	uint8_t zeros = 0;
	size_t lastNonZeroPos = 0;

	for (int i = cnt - 1; i >= 0; i--) {
		if (mem[i] != 0) {
			lastNonZeroPos = i;
			break;
		}
	}

	for (size_t i = 0; i <= lastNonZeroPos; i++) {
		if (mem[i] == 0) {
			zeros++;
			if (zeros == 16) {
				strides.push_back(0xF0);
				zeros = 0;
			}
		} else {

			uint16_t vliCode;

			const uint8_t vliLen = vli(mem[i], vliCode);

			strides.push_back((zeros << 4) | vliLen);

			if (vliLen + vliDataBitPos < 16) {
				vliData.back() |= vliCode << (16 - (vliLen + vliDataBitPos));
				vliDataBitPos += vliLen;
			} else {
				vliData.back() |= vliCode >> (vliLen + vliDataBitPos - 16);
				vliData.push_back(vliCode << (32 - (vliLen + vliDataBitPos)));
				vliDataBitPos -= 16 - vliLen;
			}

			zeros = 0;
		}
	}

	// Push EOB
	strides.push_back(EOB);

	return std::make_pair(strides, vliData);
}

std::vector<int16_t> rld(const std::vector<uint8_t> &strideData, std::vector<uint16_t> &vliData)
{
	assert(strideData.back() == EOB);

	std::vector<int16_t> result;
	size_t vliDataPos = 0;
	size_t vliDataBitPos = 0;

	for (const uint8_t x : strideData) {

		const uint8_t zeroCnt = x >> 4;
		const uint8_t vliLen = x & 0xF;

		for (size_t i = 0; i < zeroCnt; i++) {
			result.push_back(0);
		}

		uint16_t code = 0;

		if (vliDataBitPos + vliLen < 16) {
			code = (vliData[vliDataPos] >> (16 - (vliDataBitPos + vliLen))) & ((0x1 << vliLen) - 1);
			vliDataBitPos += vliLen;
		} else {
			code = ((vliData[vliDataPos] << (vliDataBitPos + vliLen - 16)) | (vliData[vliDataPos + 1] >> (32 - (vliDataBitPos + vliLen)))) & ((0x1 << vliLen) - 1);
			vliDataBitPos -= 16 - vliLen;
			vliDataPos++;
		}

		const int16_t origin = devli(code, vliLen);

		result.push_back(origin);
	}

	return result;
}

// int main(const int argc, const char * argv[], const char * envp[])
// {
// 	std::vector<int16_t> data = { 35, 7, 0, 0, 0, -6, -2, 0, 0, -9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0 };

// 	std::pair<std::vector<uint8_t>, std::vector<uint16_t>> coded = rle(data.data(), data.size());

// 	HuffmanDictionary huffDic(coded.first);

// 	std::vector<uint8_t> huffCodes = huffDic.encode(coded.first);

// 	std::cout << huffCodes.size() + coded.second.size() * 2 << std::endl;

// 	std::vector<uint8_t> huffDecodes = huffDic.decode(huffCodes);

// 	std::vector<int16_t> decodes = rld(huffDecodes, coded.second);

// 	std::cout << end - start << std::endl;

// 	std::copy(decodes.cbegin(), decodes.cend(), std::ostream_iterator<int16_t>(std::cout));
// 	std::cout << std::endl;

// 	return 0;
// }
