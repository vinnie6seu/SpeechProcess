namespace acl
{
    //HttpJson
    acl::string gson(const HttpJson &$obj);
    acl::json_node& gson(acl::json &$json, const HttpJson &$obj);
    acl::json_node& gson(acl::json &$json, const HttpJson *$obj);
    std::pair<bool,std::string> gson(acl::json_node &$node, HttpJson &$obj);
    std::pair<bool,std::string> gson(acl::json_node &$node, HttpJson *$obj);
    std::pair<bool,std::string> gson(const acl::string &str, HttpJson &$obj);

    //SocketJson
    acl::string gson(const SocketJson &$obj);
    acl::json_node& gson(acl::json &$json, const SocketJson &$obj);
    acl::json_node& gson(acl::json &$json, const SocketJson *$obj);
    std::pair<bool,std::string> gson(acl::json_node &$node, SocketJson &$obj);
    std::pair<bool,std::string> gson(acl::json_node &$node, SocketJson *$obj);
    std::pair<bool,std::string> gson(const acl::string &str, SocketJson &$obj);

}///end of acl.
