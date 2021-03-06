#include "graphmodel.h"

#include <shv/chainpack/rpcvalue.h>
#include <shv/coreqt/log.h>

namespace shv {
namespace visu {
namespace timeline {

GraphModel::GraphModel(QObject *parent)
	: Super(parent)
{

}

void GraphModel::clear()
{
	m_pathToChannelCache.clear();
	m_samples.clear();
	m_channelsInfo.clear();
}

int GraphModel::count(int channel) const
{
	if(channel < 0 || channel > channelCount())
		return 0;
	return m_samples.at(channel).count();
}

Sample GraphModel::sampleAt(int channel, int ix) const
{
	return m_samples.at(channel).at(ix);
}

Sample GraphModel::sampleValue(int channel, int ix) const
{
	if(channel < 0 || channel >= channelCount())
		return Sample();
	if(ix < 0 || ix >= m_samples[channel].count())
		return Sample();
	return sampleAt(channel, ix);
}

XRange GraphModel::xRange() const
{
	XRange ret;
	for (int i = 0; i < channelCount(); ++i) {
		ret = ret.united(xRange(i));
	}
	return ret;
}

XRange GraphModel::xRange(int channel_ix) const
{
	XRange ret;
	if(count(channel_ix) > 0) {
		ret.min = sampleAt(channel_ix, 0).time;
		ret.max = sampleAt(channel_ix, count(channel_ix) - 1).time;
	}
	return ret;
}

YRange GraphModel::yRange(int channel_ix) const
{
	YRange ret;
	auto mtid = channelInfo(channel_ix).metaTypeId;
	for (int i = 0; i < count(channel_ix); ++i) {
		QVariant v = sampleAt(channel_ix, i).value;
		bool ok;
		double d = valueToDouble(v, mtid, &ok);
		if(ok) {
			ret.min = qMin(ret.min, d);
			ret.max = qMax(ret.max, d);
		}
	}
	return ret;
}

double GraphModel::valueToDouble(const QVariant v, int meta_type_id, bool *ok)
{
	if(ok)
		*ok = true;
	if(meta_type_id == QMetaType::UnknownType)
		meta_type_id = v.userType();
	switch (meta_type_id) {
	case QVariant::Invalid:
	case QVariant::Map:
		return 0;
	case QVariant::Double:
		return v.toDouble();
	case QVariant::LongLong:
	case QVariant::ULongLong:
	case QVariant::UInt:
	case QVariant::Int:
		return v.toLongLong();
	case QVariant::Bool:
		return v.toBool()? 1: 0;
	case QVariant::String:
		return v.toString().isEmpty()? 0: 1;
	default:
		if(ok)
			*ok = false;
		else
			shvWarning() << "cannot convert variant:" << v.typeName() << "to double";
		return 0;
	}
}

int GraphModel::lessOrEqualIndex(int channel, timemsec_t time) const
{
	if(channel < 0 || channel > channelCount())
		return -1;

	int first = 0;
	int cnt = count(channel);
	bool found = false;
	while (cnt > 0) {
		int step = cnt / 2;
		int pivot = first + step;
		if (sampleAt(channel, pivot).time <= time) {
			first = pivot;
			if(step)
				cnt -= step;
			else
				cnt = 0;
			found = true;
		}
		else {
			cnt = step;
			found = false;
		}
	};
	int ret = found? first: -1;
	//shvInfo() << time << "-->" << ret;
	return ret;
}

void GraphModel::beginAppendValues()
{
	m_begginAppendXRange = xRange();
}

void GraphModel::endAppendValues()
{
	XRange xr = xRange();
	if(xr.max > m_begginAppendXRange.max)
		emit xRangeChanged(xr);
	m_begginAppendXRange = XRange();
	for (int i = 0; i < channelCount(); ++i) {
		ChannelInfo &chi = channelInfo(i);
		if(chi.metaTypeId == QMetaType::UnknownType) {
			chi.metaTypeId = guessMetaType(i);
		}
	}
}

void GraphModel::appendValue(int channel, Sample &&sample)
{
	if(channel < 0 || channel > channelCount()) {
		shvError() << "Invalid channel index:" << channel;
		return;
	}
	if(sample.time <= 0) {
		shvWarning() << "ignoring value with timestamp <= 0, timestamp:" << sample.time;
		return;
	}
	ChannelSamples &dat = m_samples[channel];
	if(!dat.isEmpty() && dat.last().time > sample.time) {
		shvWarning() << channelInfo(channel).shvPath << "channel:" << channel
					 << "ignoring value with lower timestamp than last value (check possibly wrong short-time correction):"
					 << dat.last().time << shv::chainpack::RpcValue::DateTime::fromMSecsSinceEpoch(dat.last().time).toIsoString()
					 << "val:"
					 << sample.time << shv::chainpack::RpcValue::DateTime::fromMSecsSinceEpoch(sample.time).toIsoString();
		return;
	}
	//m_appendSince = qMin(sampleAt.time, m_appendSince);
	//m_appendUntil = qMax(sampleAt.time, m_appendUntil);
	dat.push_back(std::move(sample));
}

void GraphModel::appendValueShvPath(const std::string &shv_path, Sample &&sample)
{
	int ch_ix = pathToChannelIndex(shv_path);
	if(ch_ix < 0) {
		if(isAutoCreateChannels()) {
			appendChannel(shv_path, std::string());
			ch_ix = channelCount() - 1;
		}
		else {
			shvMessage() << "Cannot find channel with shv path:" << shv_path;
			return;
		}
	}
	appendValue(ch_ix, std::move(sample));
}

int GraphModel::pathToChannelIndex(const std::string &path) const
{
	auto it = m_pathToChannelCache.find(path);
	if(it == m_pathToChannelCache.end()) {
		for (int i = 0; i < channelCount(); ++i) {
			const ChannelInfo &chi = channelInfo(i);
			if(chi.shvPath == QLatin1String(path.data(), (int)path.size())) {
				m_pathToChannelCache[path] = i;
				return i;
			}
		}
		return -1;
	}
	return it->second;
}

void GraphModel::appendChannel(const std::string &shv_path, const std::string &name, const TypeDescr &type_descr)
{
	m_pathToChannelCache.clear();
	m_channelsInfo.append(ChannelInfo());
	m_samples.append(ChannelSamples());
	auto &chi = m_channelsInfo.last();
	if(!shv_path.empty())
		chi.shvPath = QString::fromStdString(shv_path);
	if(!name.empty())
		chi.name = QString::fromStdString(name);
	chi.typeDescr = type_descr;
	emit channelCountChanged(channelCount());
}

int GraphModel::guessMetaType(int channel_ix)
{
	Sample s = sampleValue(channel_ix, 0);
	return s.value.userType();
}

TypeDescr::TypeDescr(const core::utils::ShvLogTypeDescr &d)
	: Super(d)
{
	for (const auto &field : Super::fields) {
		fields.push_back(TypeDescrField(field));
	}
	Super::fields.clear();
}

}}}
