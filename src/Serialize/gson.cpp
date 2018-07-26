#include "stdafx.h"
#include "HttpJson.h"
#include "SocketJson.h"
#include "gson.h"
namespace acl
{
    acl::json_node& gson(acl::json &$json, const HttpJson &$obj)
    {
        acl::json_node &$node = $json.create_node();

        if (check_nullptr($obj.id))
            $node.add_null("id");
        else
            $node.add_number("id", acl::get_value($obj.id));

        if (check_nullptr($obj.speechOffset))
            $node.add_null("speechOffset");
        else
            $node.add_number("speechOffset", acl::get_value($obj.speechOffset));

        if (check_nullptr($obj.speechLen))
            $node.add_null("speechLen");
        else
            $node.add_number("speechLen", acl::get_value($obj.speechLen));

        if (check_nullptr($obj.asrFlag))
            $node.add_null("asrFlag");
        else
            $node.add_number("asrFlag", acl::get_value($obj.asrFlag));

        if (check_nullptr($obj.fileName))
            $node.add_null("fileName");
        else
            $node.add_text("fileName", acl::get_value($obj.fileName));

        if (check_nullptr($obj.asrSpeechPackStatus))
            $node.add_null("asrSpeechPackStatus");
        else
            $node.add_number("asrSpeechPackStatus", acl::get_value($obj.asrSpeechPackStatus));

        if (check_nullptr($obj.curSendPacketNum))
            $node.add_null("curSendPacketNum");
        else
            $node.add_number("curSendPacketNum", acl::get_value($obj.curSendPacketNum));

        if (check_nullptr($obj.totalSendPacketNum))
            $node.add_null("totalSendPacketNum");
        else
            $node.add_number("totalSendPacketNum", acl::get_value($obj.totalSendPacketNum));


        return $node;
    }
    
    acl::json_node& gson(acl::json &$json, const HttpJson *$obj)
    {
        return gson ($json, *$obj);
    }


    acl::string gson(const HttpJson &$obj)
    {
        acl::json $json;
        acl::json_node &$node = acl::gson ($json, $obj);
        return $node.to_string ();
    }


    std::pair<bool,std::string> gson(acl::json_node &$node, HttpJson &$obj)
    {
        acl::json_node *id = $node["id"];
        acl::json_node *speechOffset = $node["speechOffset"];
        acl::json_node *speechLen = $node["speechLen"];
        acl::json_node *asrFlag = $node["asrFlag"];
        acl::json_node *fileName = $node["fileName"];
        acl::json_node *asrSpeechPackStatus = $node["asrSpeechPackStatus"];
        acl::json_node *curSendPacketNum = $node["curSendPacketNum"];
        acl::json_node *totalSendPacketNum = $node["totalSendPacketNum"];
        std::pair<bool, std::string> $result;

        if(!id ||!($result = gson(*id, &$obj.id), $result.first))
            return std::make_pair(false, "required [HttpJson.id] failed:{"+$result.second+"}");
     
        if(speechOffset)
            gson(*speechOffset, &$obj.speechOffset);
     
        if(speechLen)
            gson(*speechLen, &$obj.speechLen);
     
        if(asrFlag)
            gson(*asrFlag, &$obj.asrFlag);
     
        if(fileName)
            gson(*fileName, &$obj.fileName);
     
        if(asrSpeechPackStatus)
            gson(*asrSpeechPackStatus, &$obj.asrSpeechPackStatus);
     
        if(curSendPacketNum)
            gson(*curSendPacketNum, &$obj.curSendPacketNum);
     
        if(totalSendPacketNum)
            gson(*totalSendPacketNum, &$obj.totalSendPacketNum);
     
        return std::make_pair(true,"");
    }


    std::pair<bool,std::string> gson(acl::json_node &$node, HttpJson *$obj)
    {
        return gson($node, *$obj);
    }


     std::pair<bool,std::string> gson(const acl::string &$str, HttpJson &$obj)
    {
        acl::json _json;
        _json.update($str.c_str());
        if (!_json.finish())
        {
            return std::make_pair(false, "json not finish error");
        }
        return gson(_json.get_root(), $obj);
    }


    acl::json_node& gson(acl::json &$json, const SocketJson &$obj)
    {
        acl::json_node &$node = $json.create_node();

        if (check_nullptr($obj.id))
            $node.add_null("id");
        else
            $node.add_number("id", acl::get_value($obj.id));

        if (check_nullptr($obj.speechOffset))
            $node.add_null("speechOffset");
        else
            $node.add_number("speechOffset", acl::get_value($obj.speechOffset));

        if (check_nullptr($obj.speechLen))
            $node.add_null("speechLen");
        else
            $node.add_number("speechLen", acl::get_value($obj.speechLen));

        if (check_nullptr($obj.ttsFlag))
            $node.add_null("ttsFlag");
        else
            $node.add_number("ttsFlag", acl::get_value($obj.ttsFlag));


        return $node;
    }
    
    acl::json_node& gson(acl::json &$json, const SocketJson *$obj)
    {
        return gson ($json, *$obj);
    }


    acl::string gson(const SocketJson &$obj)
    {
        acl::json $json;
        acl::json_node &$node = acl::gson ($json, $obj);
        return $node.to_string ();
    }


    std::pair<bool,std::string> gson(acl::json_node &$node, SocketJson &$obj)
    {
        acl::json_node *id = $node["id"];
        acl::json_node *speechOffset = $node["speechOffset"];
        acl::json_node *speechLen = $node["speechLen"];
        acl::json_node *ttsFlag = $node["ttsFlag"];
        std::pair<bool, std::string> $result;

        if(!id ||!($result = gson(*id, &$obj.id), $result.first))
            return std::make_pair(false, "required [SocketJson.id] failed:{"+$result.second+"}");
     
        if(speechOffset)
            gson(*speechOffset, &$obj.speechOffset);
     
        if(speechLen)
            gson(*speechLen, &$obj.speechLen);
     
        if(ttsFlag)
            gson(*ttsFlag, &$obj.ttsFlag);
     
        return std::make_pair(true,"");
    }


    std::pair<bool,std::string> gson(acl::json_node &$node, SocketJson *$obj)
    {
        return gson($node, *$obj);
    }


     std::pair<bool,std::string> gson(const acl::string &$str, SocketJson &$obj)
    {
        acl::json _json;
        _json.update($str.c_str());
        if (!_json.finish())
        {
            return std::make_pair(false, "json not finish error");
        }
        return gson(_json.get_root(), $obj);
    }


}///end of acl.
