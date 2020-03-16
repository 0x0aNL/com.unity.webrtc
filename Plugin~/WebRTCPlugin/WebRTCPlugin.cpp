#include "pch.h"
#include "WebRTCPlugin.h"
#include "SetSessionDescriptionObserver.h"
#include "PeerConnectionStatsCollectorCallback.h"
#include "PeerConnectionObject.h"
#include "MediaStreamObserver.h"
#include "Context.h"
#include "Codec/EncoderFactory.h"

using namespace WebRTC;
namespace WebRTC
{
    DelegateDebugLog delegateDebugLog = nullptr;
    DelegateSetResolution delegateSetResolution = nullptr;

    void debugLog(const char* buf)
    {
        if (delegateDebugLog != nullptr)
        {
            if(rtc::ThreadManager::Instance()->IsMainThread())
            {
                delegateDebugLog(buf);
            }
        }
    }

    void SetResolution(int32* width, int32* length)
    {
        if (delegateSetResolution != nullptr)
        {
            delegateSetResolution(width, length);
        }
    }
}

template<typename T>
T** ConvertArray(std::vector<rtc::scoped_refptr<T>> vec, int* length)
{
#pragma warning(suppress: 4267)
    *length = vec.size();
    const auto buf = CoTaskMemAlloc(sizeof(T*) * vec.size());
    const auto ret = static_cast<T**>(buf);
    for (uint32_t i = 0; i < vec.size(); i++)
    {
        ret[i] = vec[i].get();
    }
    return ret;
}

template<typename T>
T* ConvertArray2(std::vector<T> vec, int* length)
{
#pragma warning(suppress: 4267)
    *length = vec.size();
    const auto buf = CoTaskMemAlloc(sizeof(T*) * vec.size());
    const auto ret = static_cast<T*>(buf);
    for (uint32_t i = 0; i < vec.size(); i++)
    {
        ret[i] = vec[i];
    }
    return ret;

}


const char* ConvertString(const std::string& str)
{
    auto label = static_cast<char*>(CoTaskMemAlloc(str.size() + sizeof(char)));
    str.copy(label, str.size());
    label[str.size()] = '\0';
    return label;

}


extern "C"
{
    UNITY_INTERFACE_EXPORT bool GetHardwareEncoderSupport()
    {
        return EncoderFactory::GetHardwareEncoderSupport();
    }

    UNITY_INTERFACE_EXPORT UnityEncoderType ContextGetEncoderType(Context* context)
    {
        return context->GetEncoderType();
    }

    UNITY_INTERFACE_EXPORT void ContextSetVideoEncoderParameter(Context* context, webrtc::MediaStreamTrackInterface* track, int width, int height, UnityEncoderType type)
    {
        context->SetEncoderParameter(track, width, height, type);
    }


    UNITY_INTERFACE_EXPORT webrtc::MediaStreamInterface* ContextCreateMediaStream(Context* context, const char* streamId)
    {
        return context->CreateMediaStream(streamId);
    }

    UNITY_INTERFACE_EXPORT void ContextDeleteMediaStream(Context* context, webrtc::MediaStreamInterface* stream)
    {
        context->DeleteMediaStream(stream);
    }

    UNITY_INTERFACE_EXPORT webrtc::MediaStreamTrackInterface* ContextCreateVideoTrack(Context* context, const char* label, void* rt, int32 width, int32 height, int32 bitRate)
    {
        return context->CreateVideoTrack(label, rt, width, height, bitRate);
    }

    UNITY_INTERFACE_EXPORT void ContextDeleteMediaStreamTrack(Context* context, webrtc::MediaStreamTrackInterface* track)
    {
        context->DeleteMediaStreamTrack(track);
    }

    UNITY_INTERFACE_EXPORT void ContextStopMediaStreamTrack(Context* context, webrtc::MediaStreamTrackInterface* track)
    {
        context->StopMediaStreamTrack(track);
    }

    UNITY_INTERFACE_EXPORT webrtc::MediaStreamTrackInterface* ContextCreateAudioTrack(Context* context, const char* label)
    {
        return context->CreateAudioTrack(label);
    }

    UNITY_INTERFACE_EXPORT bool MediaStreamAddTrack(webrtc::MediaStreamInterface* stream, webrtc::MediaStreamTrackInterface* track)
    {
        if (track->kind() == "audio")
        {
            return stream->AddTrack(static_cast<webrtc::AudioTrackInterface*>(track));
        }
        else
        {
            return stream->AddTrack(static_cast<webrtc::VideoTrackInterface*>(track));
        }
    }
    UNITY_INTERFACE_EXPORT bool MediaStreamRemoveTrack(webrtc::MediaStreamInterface* stream, webrtc::MediaStreamTrackInterface* track)
    {
        if (track->kind() == "audio")
        {
            return stream->RemoveTrack(static_cast<webrtc::AudioTrackInterface*>(track));
        }
        else
        {
            return stream->RemoveTrack(static_cast<webrtc::VideoTrackInterface*>(track));
        }
    }

    UNITY_INTERFACE_EXPORT char* MediaStreamGetID(webrtc::MediaStreamInterface* stream)
    {
        const auto idStr = stream->id();
        const auto id = static_cast<char*>(CoTaskMemAlloc(idStr.size() + sizeof(char)));
        idStr.copy(id, idStr.size());
        id[idStr.size()] = '\0';
        return id;
    }

    UNITY_INTERFACE_EXPORT void MediaStreamRegisterOnAddTrack(Context* context, webrtc::MediaStreamInterface* stream, DelegateMediaStreamOnAddTrack callback)
    {
        context->GetObserver(stream)->RegisterOnAddTrack(callback);
    }

    UNITY_INTERFACE_EXPORT void MediaStreamRegisterOnRemoveTrack(Context* context, webrtc::MediaStreamInterface* stream, DelegateMediaStreamOnRemoveTrack callback)
    {
        context->GetObserver(stream)->RegisterOnRemoveTrack(callback);
    }

    UNITY_INTERFACE_EXPORT webrtc::VideoTrackInterface** MediaStreamGetVideoTracks(webrtc::MediaStreamInterface* stream, int* length)
    {
        return ConvertArray<webrtc::VideoTrackInterface>(stream->GetVideoTracks(), length);
    }

    UNITY_INTERFACE_EXPORT webrtc::AudioTrackInterface** MediaStreamGetAudioTracks(webrtc::MediaStreamInterface* stream, int* length)
    {
        return ConvertArray<webrtc::AudioTrackInterface>(stream->GetAudioTracks(), length);
    }

    UNITY_INTERFACE_EXPORT TrackKind MediaStreamTrackGetKind(webrtc::MediaStreamTrackInterface* track)
    {
        const auto kindStr = track->kind();
        if (kindStr == "audio")
        {
            return TrackKind::Audio;
        }
        else
        {
            return TrackKind::Video;
        }
    }

    UNITY_INTERFACE_EXPORT webrtc::MediaStreamTrackInterface::TrackState MediaStreamTrackGetReadyState(webrtc::MediaStreamTrackInterface* track)
    {
        return track->state();
    }

    UNITY_INTERFACE_EXPORT char* MediaStreamTrackGetID(webrtc::MediaStreamTrackInterface* track)
    {
        const auto idStr = track->id();
        const auto id = static_cast<char*>(CoTaskMemAlloc(idStr.size() + sizeof(char)));
        idStr.copy(id, idStr.size());
        id[idStr.size()] = '\0';
        return id;
    }

    UNITY_INTERFACE_EXPORT bool MediaStreamTrackGetEnabled(webrtc::MediaStreamTrackInterface* track)
    {
        return track->enabled();
    }

    UNITY_INTERFACE_EXPORT void MediaStreamTrackSetEnabled(webrtc::MediaStreamTrackInterface* track, bool enabled)
    {
        track->set_enabled(enabled);
    }

    UNITY_INTERFACE_EXPORT void RegisterDebugLog(DelegateDebugLog func)
    {
        delegateDebugLog = func;
    }

    UNITY_INTERFACE_EXPORT void RegisterSetResolution(DelegateSetResolution func)
    {
        delegateSetResolution = func;
    }

    UNITY_INTERFACE_EXPORT Context* ContextCreate(int uid, UnityEncoderType encoderType)
    {
        auto ctx = ContextManager::GetInstance()->GetContext(uid);
        if (ctx != nullptr)
        {
            DebugLog("Already created context with ID %d", uid);
            return ctx;
        }
        ctx = ContextManager::GetInstance()->CreateContext(uid, encoderType);
        return ctx;
    }

    UNITY_INTERFACE_EXPORT void ContextDestroy(int uid)
    {
        ContextManager::GetInstance()->DestroyContext(uid);
    }

    PeerConnectionObject* _ContextCreatePeerConnection(Context* context, const webrtc::PeerConnectionInterface::RTCConfiguration& config)
    {
        const auto obj = context->CreatePeerConnection(config);
        context->AddObserver(obj->connection, SetSessionDescriptionObserver::Create(obj));
        context->AddStatsCallback(obj->connection, PeerConnectionStatsCollectorCallback::Create(obj));
        return obj;
    }

    UNITY_INTERFACE_EXPORT PeerConnectionObject* ContextCreatePeerConnection(Context* context)
    {
        webrtc::PeerConnectionInterface::RTCConfiguration config;
        config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
        return _ContextCreatePeerConnection(context, config);
    }

    UNITY_INTERFACE_EXPORT PeerConnectionObject* ContextCreatePeerConnectionWithConfig(Context* context, const char* conf)
    {
        webrtc::PeerConnectionInterface::RTCConfiguration config;
        Convert(conf, config);
        return _ContextCreatePeerConnection(context, config);
    }

    UNITY_INTERFACE_EXPORT void ContextDeletePeerConnection(Context* context, PeerConnectionObject* obj)
    {
        obj->Close();
        context->RemoveObserver(obj->connection);
        context->DeletePeerConnection(obj);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionClose(PeerConnectionObject* obj)
    {
        obj->Close();
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpSenderInterface* PeerConnectionAddTrack(PeerConnectionObject* obj, webrtc::MediaStreamTrackInterface* track, const char* streamId)
    {
        return obj->connection->AddTrack(rtc::scoped_refptr <webrtc::MediaStreamTrackInterface>(track), { streamId }).value().get();
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpTransceiverInterface* PeerConnectionAddTransceiver(PeerConnectionObject* obj, webrtc::MediaStreamTrackInterface* track)
    {
        return obj->connection->AddTransceiver(track).value().get();
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpTransceiverInterface* PeerConnectionAddTransceiverWithInit(PeerConnectionObject* obj, webrtc::MediaStreamTrackInterface* track, webrtc::RtpTransceiverInit* init)
    {
        return obj->connection->AddTransceiver(track, *init).value().get();
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRemoveTrack(PeerConnectionObject* obj, webrtc::RtpSenderInterface* sender)
    {
        obj->connection->RemoveTrack(sender);
    }

    UNITY_INTERFACE_EXPORT webrtc::RTCErrorType PeerConnectionSetConfiguration(PeerConnectionObject* obj, const char* conf)
    {
        return obj->SetConfiguration(std::string(conf)); 
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionGetConfiguration(PeerConnectionObject* obj, char** conf, int* len)
    {
        std::string _conf;
        obj->GetConfiguration(_conf);
#pragma warning(suppress: 4267)
        *len = _conf.size();
        *conf = static_cast<char*>(::CoTaskMemAlloc(_conf.size() + sizeof(char)));
        _conf.copy(*conf, _conf.size());
        (*conf)[_conf.size()] = '\0';
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionSetRemoteDescription(Context* context, PeerConnectionObject* obj, const RTCSessionDescription* desc)
    {
        obj->SetRemoteDescription(*desc, context->GetObserver(obj->connection));
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionSetLocalDescription(Context* context, PeerConnectionObject* obj, const RTCSessionDescription* desc)
    {
        obj->SetLocalDescription(*desc, context->GetObserver(obj->connection));
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionGetStats(Context* context, PeerConnectionObject* obj)
    {
        obj->connection->GetStats(context->GetStatsCallback(obj->connection));
    }

    UNITY_INTERFACE_EXPORT const webrtc::RTCStats** StatsReportGetList(const webrtc::RTCStatsReport* report, int* length)
    {
        *length = report->size();
        const auto buf = CoTaskMemAlloc(sizeof(webrtc::RTCStats*) * report->size());
        const auto ret = static_cast<const webrtc::RTCStats**>(buf);
        int i = 0;
        for (auto it = report->begin(); it != report->end(); ++it, i++)
        {
            ret[i] = &*it;
        }
        return ret;
    }

    UNITY_INTERFACE_EXPORT const char* StatsGetJson(const webrtc::RTCStats* stats)
    {
        return ConvertString(stats->ToJson());
    }

    UNITY_INTERFACE_EXPORT int64_t StatsGetTimestamp(const webrtc::RTCStats* stats)
    {
        return stats->timestamp_us();
    }

    UNITY_INTERFACE_EXPORT const char* StatsGetId(const webrtc::RTCStats* stats)
    {
        return ConvertString(stats->id());
    }

    UNITY_INTERFACE_EXPORT const char* StatsGetType(const webrtc::RTCStats* stats)
    {
        return ConvertString(stats->type());
    }

    UNITY_INTERFACE_EXPORT const webrtc::RTCStatsMemberInterface** StatsGetMembers(const webrtc::RTCStats* stats, int* length)
    {
        auto vec = stats->Members();
        *length = vec.size();
        const auto buf = CoTaskMemAlloc(sizeof(webrtc::RTCStatsMemberInterface*) * vec.size());
        const auto ret = static_cast<const webrtc::RTCStatsMemberInterface**>(buf);
        for (uint32_t i = 0; i < vec.size(); i++)
        {
            ret[i] = vec[i];
        }
        return ret;
    }

    UNITY_INTERFACE_EXPORT const char* StatsMemberGetName(const webrtc::RTCStatsMemberInterface* member)
    {
        return ConvertString(std::string(member->name()));
    }

    UNITY_INTERFACE_EXPORT bool StatsMemberGetBool(const webrtc::RTCStatsMemberInterface* member)
    {
        return *member->cast_to<webrtc::RTCStatsMember<bool>>();
    }

    UNITY_INTERFACE_EXPORT int32_t StatsMemberGetInt(const webrtc::RTCStatsMemberInterface* member)
    {
        return *member->cast_to<webrtc::RTCStatsMember<int32_t>>();
    }

    UNITY_INTERFACE_EXPORT uint32_t StatsMemberGetUnsignedInt(const webrtc::RTCStatsMemberInterface* member)
    {
        return *member->cast_to<webrtc::RTCStatsMember<uint32_t>>();
    }

    UNITY_INTERFACE_EXPORT int64_t StatsMemberGetLong(const webrtc::RTCStatsMemberInterface* member)
    {
        return *member->cast_to<webrtc::RTCStatsMember<uint64_t>>();
    }

    UNITY_INTERFACE_EXPORT uint64_t StatsMemberGetUnsignedLong(const webrtc::RTCStatsMemberInterface* member)
    {
        return *member->cast_to<webrtc::RTCStatsMember<uint64_t>>();
    }

    UNITY_INTERFACE_EXPORT double StatsMemberGetDouble(const webrtc::RTCStatsMemberInterface* member)
    {
        return *member->cast_to<webrtc::RTCStatsMember<double>>();
    }

    UNITY_INTERFACE_EXPORT const char* StatsMemberGetString(const webrtc::RTCStatsMemberInterface* member)
    {
        return ConvertString(member->ValueToString());
    }

    UNITY_INTERFACE_EXPORT bool* StatsMemberGetBoolArray(const webrtc::RTCStatsMemberInterface* member, int* length)
    {
        return ConvertArray2(*member->cast_to<webrtc::RTCStatsMember<std::vector<bool>>>(), length);
    }

    UNITY_INTERFACE_EXPORT webrtc::RTCStatsMemberInterface::Type StatsMemberGetType(const webrtc::RTCStatsMemberInterface* member)
    {
        return member->type();
    }


    /*
    UNITY_INTERFACE_EXPORT unsigned long StatsMemberGetUnsignedLong(const webrtc::RTCStatsMemberInterface* member)
    {
        return member->cast_to<unsigned long>();
    }
    */


    UNITY_INTERFACE_EXPORT bool PeerConnectionGetLocalDescription(PeerConnectionObject* obj, RTCSessionDescription* desc)
    {
        return obj->GetSessionDescription(obj->connection->local_description(), *desc);
    }

    UNITY_INTERFACE_EXPORT bool PeerConnectionGetRemoteDescription(PeerConnectionObject* obj, RTCSessionDescription* desc)
    {
        return obj->GetSessionDescription(obj->connection->remote_description(), *desc);
    }

    UNITY_INTERFACE_EXPORT bool PeerConnectionGetPendingLocalDescription(PeerConnectionObject* obj, RTCSessionDescription* desc)
    {
        return obj->GetSessionDescription(obj->connection->pending_local_description(), *desc);
    }

    UNITY_INTERFACE_EXPORT bool PeerConnectionGetPendingRemoteDescription(PeerConnectionObject* obj, RTCSessionDescription* desc)
    {
        return obj->GetSessionDescription(obj->connection->pending_remote_description(), *desc);
    }

    UNITY_INTERFACE_EXPORT bool PeerConnectionGetCurrentLocalDescription(PeerConnectionObject* obj, RTCSessionDescription* desc)
    {
        return obj->GetSessionDescription(obj->connection->current_local_description(), *desc);
    }

    UNITY_INTERFACE_EXPORT bool PeerConnectionGetCurrentRemoteDescription(PeerConnectionObject* obj, RTCSessionDescription* desc)
    {
        return obj->GetSessionDescription(obj->connection->current_remote_description(), *desc);
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpReceiverInterface** PeerConnectionGetReceivers(PeerConnectionObject* obj, int* length)
    {
        return ConvertArray<webrtc::RtpReceiverInterface>(obj->connection->GetReceivers(), length);
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpSenderInterface** PeerConnectionGetSenders(PeerConnectionObject* obj, int* length)
    {
        return ConvertArray<webrtc::RtpSenderInterface>(obj->connection->GetSenders(), length);
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpTransceiverInterface** PeerConnectionGetTransceivers(PeerConnectionObject* obj, int* length)
    {
        return ConvertArray<webrtc::RtpTransceiverInterface>(obj->connection->GetTransceivers(), length);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionCreateOffer(PeerConnectionObject* obj, const RTCOfferOptions* options)
    {
        obj->CreateOffer(*options);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionCreateAnswer(PeerConnectionObject* obj, const RTCAnswerOptions* options)
    {
        obj->CreateAnswer(*options);
    }

    UNITY_INTERFACE_EXPORT DataChannelObject* ContextCreateDataChannel(Context* ctx, PeerConnectionObject* obj, const char* label, const RTCDataChannelInit* options)
    {
        return ctx->CreateDataChannel(obj, label, *options);
    }

    UNITY_INTERFACE_EXPORT void ContextDeleteDataChannel(Context* ctx, DataChannelObject* channel)
    {
        ctx->DeleteDataChannel(channel);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterIceConnectionChange(PeerConnectionObject* obj, DelegateOnIceConnectionChange callback)
    {
        obj->RegisterIceConnectionChange(callback);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterOnIceCandidate(PeerConnectionObject*obj, DelegateIceCandidate callback)
    {
        obj->RegisterIceCandidate(callback);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterCallbackCollectStats(Context* context, PeerConnectionObject* obj, DelegateCollectStats onGetStats)
    {
        context->GetStatsCallback(obj->connection)->SetCallback(onGetStats);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterCallbackCreateSD(PeerConnectionObject* obj, DelegateCreateSDSuccess onSuccess, DelegateCreateSDFailure onFailure)
    {
        obj->RegisterCallbackCreateSD(onSuccess, onFailure);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterOnSetSessionDescSuccess(Context* context, PeerConnectionObject* obj, DelegateSetSessionDescSuccess onSuccess)
    {
        context->GetObserver(obj->connection)->RegisterDelegateOnSuccess(onSuccess);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterOnSetSessionDescFailure(Context* context, PeerConnectionObject* obj, DelegateSetSessionDescFailure onFailure)
    {
        context->GetObserver(obj->connection)->RegisterDelegateOnFailure(onFailure);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionAddIceCandidate(PeerConnectionObject* obj, const RTCIceCandidate* candidate)
    {
        return obj->AddIceCandidate(*candidate);
    }

    UNITY_INTERFACE_EXPORT RTCPeerConnectionState PeerConnectionState(PeerConnectionObject* obj)
    {
        return obj->GetConnectionState();
    }

    UNITY_INTERFACE_EXPORT RTCIceConnectionState PeerConnectionIceConditionState(PeerConnectionObject* obj)
    {
        return obj->GetIceCandidateState();
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterOnDataChannel(PeerConnectionObject* obj, DelegateOnDataChannel callback)
    {
        obj->RegisterOnDataChannel(callback);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterOnRenegotiationNeeded(PeerConnectionObject* obj, DelegateOnRenegotiationNeeded callback)
    {
        obj->RegisterOnRenegotiationNeeded(callback);
    }

    UNITY_INTERFACE_EXPORT void PeerConnectionRegisterOnTrack(PeerConnectionObject* obj, DelegateOnTrack callback)
    {
        obj->RegisterOnTrack(callback);
    }
    UNITY_INTERFACE_EXPORT webrtc::MediaStreamTrackInterface* TransceiverGetTrack(webrtc::RtpTransceiverInterface* transceiver)
    {
        return transceiver->receiver()->track().get();
    }

    UNITY_INTERFACE_EXPORT bool TransceiverGetCurrentDirection(webrtc::RtpTransceiverInterface* transceiver, webrtc::RtpTransceiverDirection* direction)
    {
        if(transceiver->current_direction().has_value())
        {
            *direction = transceiver->current_direction().value();
            return true;
        }
        return false;
    }

    UNITY_INTERFACE_EXPORT void TransceiverStop(webrtc::RtpTransceiverInterface* transceiver)
    {
        transceiver->Stop();
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpReceiverInterface* TransceiverGetReceiver(webrtc::RtpTransceiverInterface* transceiver)
    {
        return transceiver->receiver().get();
    }

    UNITY_INTERFACE_EXPORT webrtc::RtpSenderInterface* TransceiverGetSender(webrtc::RtpTransceiverInterface* transceiver)
    {
        return transceiver->sender().get();
    }

    UNITY_INTERFACE_EXPORT int DataChannelGetID(DataChannelObject* dataChannelObj)
    {
        return dataChannelObj->GetID();
    }

    UNITY_INTERFACE_EXPORT const char* DataChannelGetLabel(const DataChannelObject* dataChannelObj)
    {
        return ConvertString(dataChannelObj->GetLabel());
    }

    UNITY_INTERFACE_EXPORT void DataChannelSend(DataChannelObject* dataChannelObj, const char* msg)
    {
        dataChannelObj->Send(msg);
    }

    UNITY_INTERFACE_EXPORT void DataChannelSendBinary(DataChannelObject* dataChannelObj, const byte* msg, int len)
    {
        dataChannelObj->Send(msg, len);
    }

    UNITY_INTERFACE_EXPORT void DataChannelClose(DataChannelObject* dataChannelObj)
    {
        dataChannelObj->Close();
    }

    UNITY_INTERFACE_EXPORT void DataChannelRegisterOnMessage(DataChannelObject* dataChannelObj, DelegateOnMessage callback)
    {
        dataChannelObj->RegisterOnMessage(callback);
    }

    UNITY_INTERFACE_EXPORT void DataChannelRegisterOnOpen(DataChannelObject* dataChannelObj, DelegateOnOpen callback)
    {
        dataChannelObj->RegisterOnOpen(callback);
    }

    UNITY_INTERFACE_EXPORT void DataChannelRegisterOnClose(DataChannelObject* dataChannelObj, DelegateOnClose callback)
    {
        dataChannelObj->RegisterOnClose(callback);
    }

    UNITY_INTERFACE_EXPORT void SetCurrentContext(Context* context)
    {
        ContextManager::GetInstance()->curContext = context;
    }

    UNITY_INTERFACE_EXPORT void ProcessAudio(float* data, int32 size)
    {
        if (ContextManager::GetInstance()->curContext)
        {
            ContextManager::GetInstance()->curContext->ProcessAudioData(data, size);
        }
    }
}



