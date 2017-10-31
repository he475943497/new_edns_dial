namespace c_glib dnsrpc
namespace java rpc.yamutech.com
namespace cpp rpc.yamutech.com
namespace * rpc.yamutech.com

exception Xception {
  1: i32 errorCode,
  2: string message
}

typedef string ObjectId 

enum ModuleType
{
  HOSTAGENT = 0,
  CRM,
  DIALING,
  NOTICE,
  COLLECT,
  AGENT
}

enum NetType
{
  MAIN = 0,
  BUSINESS,
  VIRTUAL,
  LOOPBACK
}

enum LogLevel
{
  NORMAL = 0,
  WARN,
  ERROR
}

enum DialStatus
{
  OK = 0,
  FAIL
}

enum RetCode
{
  OK = 0, 
  FAIL 
}

enum HaType
{
  HA_MASTER = 0, 
  HA_SLAVE
}

enum SysHaPolicy
{
  UNKNOWN=0,
  SYS_POLICY_STOP_OSPF, 
  SYS_POLICY_START_XPROXY
}

enum SecParamName
{
  SEC_PARAM_WHITELIST = 0,
  SEC_PARAM_ACL, 
  SEC_PARAM_QPSLIMIT, 
  SEC_PARAM_BLACKLIST, 
  SEC_PARAM_TCP, 
  SEC_PARAM_TCPLIMIT,
  SEC_PARAM_ICMPLIMIT,
  SEC_PARAM_CACHEVIRUSLIMIT,
  SEC_PARAM_FILTERIP,
  SEC_PARAM_FILTERUDP,
  SEC_PARAM_FILTERDNS,
  SEC_PARAM_DDOSDEFEND,
  SEC_PARAM_DNSFLOODDEFND  
}

enum DnsParamName
{
  DNS_PARAM_AUTH = 0, 
  DNS_PARAM_XFORWARD,
  DNS_PARAM_TTL,
  DNS_PARAM_REDIRECT,
  DNS_PARAM_SPREAD,
  DNS_PARAM_RECUSIVE,
  DNS_PARAM_CACHE
}

enum Status
{
  STATUS_INACTIVE = 0,
  STATUS_ACTIVE
}

enum DialMethod
{
  DIAL_TCPPORT = 0, 
  DIAL_IMCP,
  DIAL_HTTPGET,
  DIAL_DATABASE,
  DIAL_EXTHTTPGET
}

enum AlarmStatus
{
  ALARM_OFF = 0,
  ALARM_ON
}

enum AlarmType
{
  DNSysDeviceQpsOverAlarm = 0, 
  DNSysDeviceRequestIpOverAlarm,
  DNSysDeviceDomainQpsOverAlarm,
  DNSysDeviceDnameQpsOverAlarm,
  DNSysDeviceFocusDomainAlarm,
  DeviceCpuUsageAlarm,
  DeviceMemoryUsageAlarm,
  DevicePartitionUsageAlarm,
  DeviceNicFlowOverAlarm,
  NoneDeviceConnectionAlarm,
  DeviceProcessAlarm,
  DNSysDeviceServfailOverAlarm,
  ForwardFailAlarm,
  ProxyFailAlarm,
  ServiceFailAlarm,
  DiskFailAlarm,
  LogAlarm,
  RecordialFailAlarm,
  RedirectFailAlarm,
  NginxServerFailAlarm
}

enum ModuleState
{
  STARTUP=0,
  REGISTERED
}

enum SysCommand
{
  RestoreConfig = 0
}

enum DialServerType
{
  XPROXY=0,
  REDIRECT,
  XFORWARD
}

struct RetRecord
{
  1: string recordId,
  2: RetCode code
}

struct IpAddr
{
  1: i32 version,
  2: string addr  
}

struct DialNginxServer
{
  1: string localURL,
  2: i32 priority
}

struct DialRecordStatus 
{
  1: ObjectId rid,
  2: DialStatus status;
}

struct DialNginxStatus
{
  1: DialNginxServer server,
  2: DialStatus status;
}

struct DialRecordAlarm
{
  1: IpAddr ip,
  2: bool enable,
  3: bool dial
}
struct DialServerStatus
{
  1: IpAddr ip,
  2: DialStatus status
}

struct IpsecAddress
{
  1: IpAddr ip,
  2: i32 mask
}

struct StaticRoute
{
  1: IpAddr ip, 
  2: i32 mask, 
  3: IpAddr nextHop
}

struct OspfRoute
{
  1: string routeId, 
  2: string areaId, 
  3: string areaType, 
  4: string netName
}

struct HostHaConf
{
  1: string netName, 
  2: HaType typ, 
  3: IpAddr localIp, 
  4: IpAddr remoteIp, 
  5: IpAddr virtualIp, 
  6: string password, 
  7: i32 switchSeconds
}

struct DomainInfo
{
  1: string typ, 
  2: string name
} 

struct RangeInfo
{
  1: i16 start,
  2: i16 ends
}

struct AclInfo 
{
  1: ObjectId id,
  2: optional i32 priority,
  3: optional IpsecAddress srcIpSec,
  4: optional IpsecAddress dstIpSec,
  5: optional RangeInfo srcPortSec,
  6: optional RangeInfo dstPortSec,
  7: optional RangeInfo frameTypeSec,
  8: optional RangeInfo ipProtoSec,
  9: optional i32 qpsLimit  
}

struct ZoneInfo
{
  1: string name, 
  2: string viewName, 
  3: optional i32 typ,
  4: optional string nsRecord,
  5: optional string aRecord,
  6: optional string soaName,
  7: optional string soaHost,
  8: optional i32 serial,
  9: optional i32 refresh,
  10: optional i32 retries,
  11: optional i32 expire,
  12: optional i32 minTtl
}

struct DnsQueryResultSRV
{
  1: i16 priority, 
  2: i16 weight,
  3: i16 port,
  4: string target
}

struct DnsQueryResultNAPTR
{
  1: i16 order, 
  2: i16 pref,
  3: string flags,
  4: string svc,
  5: string regexp,
  6: string domainName
}

struct DnsQueryResultMX
{
  1: i16 pref,
  2: string domainName
}

union DnsQueryResult
{
  1: string a;
  2: string aaaa,
  3: string ns,
  4: string cname,
  5: string ptr,
  6: string txt,
  7: DnsQueryResultSRV srv,
  8: DnsQueryResultNAPTR naptr,
  9: DnsQueryResultMX mx
}

enum RecordType
{
	A = 0,
	AAAA,
	NS,
	CNAME,
	PTR,
	TXT,
	SRV,
	NAPTR,
	MX
}

struct RecordInfo
{
  1: string name, 
  2: string viewName, 
  3: string zoneName,
  4: i32 typ,
  5: DnsQueryResult result,
  6: optional i32 ttl,
  7: optional i32 weight,
  8: optional Status status,
  9: string recordId
}

struct ProxyServer
{
  1: IpAddr ip,
  2: i32 weight,
  3: Status status
}

struct ProxyPolicyInfo
{
  1: string viewName,
  2: DomainInfo domain, 
  3: optional string serverGroupName
}

struct TtlPolicyInfo
{
  1: DomainInfo domain,
  2: optional i32 minTtl, 
  3: optional i32 maxTtl
}

struct ForwardPolicyInfo
{
  1: IpAddr ip, 
  2: i16 port,
  3: optional i32 weight,
  4: optional Status status
}

struct DialRecord
{
  1: ObjectId rid,
  2: IpAddr ip,
  3: i32 ttl,
  4: i32 priority,
  5: bool enabled
}

struct HealthGroupInfo
{
  1: string zoneName,
  2: string viewName,
  3: string name,
}

struct DialOption
{
  1: string destUrl,
  2: string testMethod,
  3: list<i32> expectCode,
  4: string expectMatch
}

struct HealthPolicyInfo
{
  1: string name,
  2: DialMethod method,
  3: optional i16 port,
  4: i32 freq,
  5: i32 times,
  6: i32 passed,
  7: DialOption option
}

struct FilterReportInfo
{
  1: i32 total,
  2: i32 exceptIpLimit,
  3: i32 exceptUdpLimit,
  4: i32 exceptDnsLimit,
  5: i32 ipLimit,
  6: i32 domainLimit,
  7: i32 aclLimit,
  8: i32 userWhiteLimit,
  9: i32 userBlackLimit,
  10: i32 domainBlackLimit
}

struct RequestReportInfo
{
  1: i32 total,
  2: i32 a,
  3: i32 aaaa,
  4: i32 cname,
  5: i32 ptr,
  6: i32 txt,
  7: i32 srv,
  8: i32 naptr,
  9: i32 mx
}

struct GeneralReportInfo
{
  1: i32 total,
  2: i32 noerr,
  3: i32 servfail,
  4: i32 nxdomain,
  5: i32 refuse
}

struct XProxyReportInfo
{
  1: i32 total,
  2: i32 proxyRequest,
  3: i32 proxyAnswer,
  4: i32 proxyForwardRequest,
  5: i32 proxyForwardAnswer
}

struct XForwardReportInfo
{
  1: i32 total,
  2: i32 xforwardRequest,
  3: i32 xforwardAnswer
}

struct RecursiveReportInfo
{
  1: i32 total,
  2: i32 recursiveRequest,
  3: i32 recursiveAnswer
}

struct BackGroundReportInfo
{
  1: i32 total,
  2: i32 smartUpdate,
  3: i32 cacheUpdate,
  4: i32 limitDrop,
  5: i32 ttlExpire
}

struct TopnIpInfo
{
  1: string ip, 
  2: i32 access
}

struct TopnDomainInfo
{
  1: string domain, 
  2: i32 access
}

struct TopnDnameInfo
{
  1: string dname, 
  2: i32 access
}

struct HostInfo
{
  1: i32 cpu, 
  2: i32 memory,
  3: i32 nicin,
  4: i32 nicout
}
struct NotifyServer
{
  1:i32 port,
  2:string protocal,
  3:IpAddr ip,
  4:list<AlarmType> types
}

struct HeartBeatState
{
  1:ModuleState mState
}

struct IpOverAlarm
{
  1: IpAddr ip,
  2: i32 qps,
  3: AlarmStatus status
}

struct DomainOverAlarm
{
  1: string domain,
  2: i32 qps,
  3: AlarmStatus status
}

struct NginxServerAlarm
{
  1: string serverName,
  2: string localUrl,
  3: AlarmStatus status
}

struct ZoneRequestReport
{
  1: string zoneName, 
  2: string viewName, 
  3: RequestReportInfo data
}

struct ZoneAnswerReport
{
  1: string zoneName, 
  2: string viewName, 
  3: GeneralReportInfo data
}

struct XproxyReport
{
  1: string xproxy, 
  2: XProxyReportInfo data
}

struct XforwardReport
{
  1: string xforward, 
  2: XForwardReportInfo data
}

struct RedirectServerInfo
{
  1: IpAddr ip, 
  2: optional i32 weight,
  3: optional Status status
}

struct NginxProxyInfo
{
  1: string proxyDomain,
  2: i32 proxyPort,
  3: string targetIpAddr,
  4: string targetURL,
  5: string protocol
}

service Agent
{
  RetCode         registerModule(1: ModuleType typ) throws(1: Xception ex),
  RetCode         updateHealthStatus(1: HealthGroupInfo health, 2: list<DialRecordStatus> statusList) throws(1: Xception ex),
  RetCode         updateServerStatus(1: DialServerStatus status,2: DialServerType typ) throws(1: Xception ex)
  RetCode         addProxyPolicy(1: ProxyPolicyInfo policy) throws(1: Xception ex),
  RetCode         delProxyPolicy(1: ProxyPolicyInfo policy) throws(1: Xception ex),
  RetCode         updateNginxStatus(1: string groupName,2: list<DialNginxStatus> statusList) throws(1: Xception ex)
}

service HostAgent
{ 
  RetCode         systemCommand(1: SysCommand cmdType) throws(1: Xception ex),
  RetCode         isPrimary(1: bool isPrimary) throws(1: Xception ex),
  RetCode         setAlarmThreshHold(1: AlarmType typ, 2: i32 threshHold) throws(1: Xception ex),
  RetCode         setNtp(1: IpAddr server, 2:  bool enabled) throws(1: Xception ex),
  RetCode         setNet(1: string name, 2: IpAddr ip, 3: i32 mask, 4: Status status, 5: NetType type, 6: string note) throws(1: Xception ex),
  RetCode         delNet(1: string name) throws(1: Xception ex),
  RetCode         addStaticRoute(1: StaticRoute route) throws(1: Xception ex),
  RetCode         delStaticRoute(1: StaticRoute route) throws(1: Xception ex),
  RetCode         addOspfRoute(1: OspfRoute route) throws(1: Xception ex),
  RetCode         delOspfRoute(1: OspfRoute route) throws(1: Xception ex),
  RetCode         setHostHa(1:bool enabled,2: HostHaConf conf) throws(1: Xception ex),
  RetCode         setSysHa(1:bool enabled,2: string dialMethod, 3: SysHaPolicy policy) throws(1: Xception ex),
  RetCode         addFocusDomain(1: string dname, 2: RecordType type,3: string result) throws(1: Xception ex),
  RetCode         delFocusDomain(1: string dname, 2: RecordType type,3: string result) throws(1: Xception ex),
  RetCode         setProxyPolicyInfo(1: ProxyPolicyInfo policy) throws(1: Xception ex),  
  RetCode         addReverseProxy(1: list<NginxProxyInfo> proxyInfoList) throws(1: Xception ex),
  RetCode         delReverseProxy(1: list<NginxProxyInfo> proxyInfoList) throws(1: Xception ex),
  HeartBeatState  heartBeat() throws(1: Xception ex)
}

service Crm
{
  RetCode         systemCommand(1: SysCommand cmdType) throws(1: Xception ex),
  RetCode         setAlarmThreshHold(1: AlarmType typ, 2: i32 threshHold) throws(1: Xception ex),
  RetCode         addSysIpSec(1: string name, 2: IpsecAddress ipsec) throws(1: Xception ex),
  RetCode         delSysIpSec(1: string name, 2: IpsecAddress ipsec) throws(1: Xception ex),
  RetCode         setSecConfig(1: SecParamName name, 2: bool allowed, 3: i32 value) throws(1: Xception ex),
  RetCode         addSecWhiteList(1: string name) throws(1: Xception ex),
  RetCode         delSecWhiteList(1: string name) throws(1: Xception ex),
  RetCode         addSecIpQpsLimit(1: IpsecAddress ipsec, 2: i32 qps) throws(1: Xception ex),
  RetCode         modSecIpQpsLimit(1: IpsecAddress ipsec, 2: i32 qps) throws(1: Xception ex),
  RetCode         delSecIpQpsLimit(1: IpsecAddress ipsec) throws(1: Xception ex),
  RetCode         addSecDomainQpsLimit(1: DomainInfo domain, 2: i32 qps) throws(1: Xception ex),
  RetCode         modSecDomainQpsLimit(1: DomainInfo domain, 2: i32 qps) throws(1: Xception ex),
  RetCode         delSecDomainQpsLimit(1: DomainInfo domain) throws(1: Xception ex),
  RetCode         addSecIpBlackList(1: IpsecAddress ipsec) throws(1: Xception ex),
  RetCode         delSecIpBlackList(1: IpsecAddress ipsec) throws(1: Xception ex),
  RetCode         addSecDomainBlackList(1: DomainInfo domain) throws(1: Xception ex),
  RetCode         delSecDomainBlackList(1: DomainInfo domain) throws(1: Xception ex),
  RetCode         addSecAcl(1: AclInfo acl) throws(1: Xception ex),
  RetCode         modSecAcl(1: AclInfo acl) throws(1: Xception ex),
  RetCode         delSecAcl(1: AclInfo acl) throws(1: Xception ex),
  RetCode         setDnsConfig(1: DnsParamName name, 2: bool allowed) throws(1: Xception ex),
  RetCode         addDnsView(1: string name, 2: string ipGroupName) throws(1: Xception ex),
  RetCode         delDnsView(1: string name, 2: string ipGroupName) throws(1: Xception ex),
  RetCode         addDnsZone(1: ZoneInfo zone) throws(1: Xception ex),
  RetCode         modDnsZone(1: ZoneInfo zone) throws(1: Xception ex),
  RetCode         delDnsZone(1: ZoneInfo zone) throws(1: Xception ex),
  list<RetRecord>         addDnsRecord(1: list<RecordInfo> records) throws(1: Xception ex),
  list<RetRecord>         modDnsRecord(1: list<RecordInfo> records) throws(1: Xception ex),
  list<RetRecord>         delDnsRecord(1: list<RecordInfo> records) throws(1: Xception ex),
  RetCode         addProxyServerGroup(1: string groupName,2: ProxyServer server) throws(1: Xception ex),
  RetCode         modProxyServerGroup(1: string groupName,2: ProxyServer server) throws(1: Xception ex),
  RetCode         delProxyServerGroup(1: string groupName,2: ProxyServer server) throws(1: Xception ex),
  RetCode         addProxyPolicy(1: ProxyPolicyInfo policy) throws(1: Xception ex),
  RetCode         modProxyPolicy(1: ProxyPolicyInfo policy) throws(1: Xception ex),
  RetCode         delProxyPolicy(1: ProxyPolicyInfo policy) throws(1: Xception ex),
  RetCode         addTtlPolicy(1: TtlPolicyInfo policy) throws(1: Xception ex),
  RetCode         modTtlPolicy(1: TtlPolicyInfo policy) throws(1: Xception ex),
  RetCode         delTtlPolicy(1: TtlPolicyInfo policy) throws(1: Xception ex),
  RetCode         addRedirectServer(1: RedirectServerInfo server) throws(1: Xception ex),
  RetCode         modRedirectServer(1: RedirectServerInfo server) throws(1: Xception ex),
  RetCode         delRedirectServer(1: RedirectServerInfo server) throws(1: Xception ex),
  RetCode         addForwardPolicy(1: ForwardPolicyInfo policy) throws(1: Xception ex),
  RetCode         modForwardPolicy(1: ForwardPolicyInfo policy) throws(1: Xception ex),
  RetCode         delForwardPolicy(1: ForwardPolicyInfo policy) throws(1: Xception ex),
  HeartBeatState  heartBeat() throws(1: Xception ex)
}

service Dial
{
  RetCode         systemCommand(1: SysCommand cmdType) throws(1: Xception ex),
  RetCode		  isPrimary(1: bool isPrimary) throws(1: Xception ex),
  RetCode         addHealthGroup(1: HealthGroupInfo health,2: optional string policyName) throws(1: Xception ex),
  RetCode         modHealthGroup(1: HealthGroupInfo health,2: optional string policyName) throws(1: Xception ex),
  RetCode         delHealthGroup(1: HealthGroupInfo health) throws(1: Xception ex),
  RetCode         addHealthRecord(1: HealthGroupInfo health,2: DialRecord rec) throws(1: Xception ex),
  RetCode         modHealthRecord(1: HealthGroupInfo health,2: DialRecord rec) throws(1: Xception ex),
  RetCode         delHealthRecord(1: HealthGroupInfo health,2: DialRecord rec) throws(1: Xception ex),
  RetCode         addHealthPolicy(1: HealthPolicyInfo policy) throws(1: Xception ex),
  RetCode         modHealthPolicy(1: HealthPolicyInfo policy) throws(1: Xception ex),
  RetCode         delHealthPolicy(1: HealthPolicyInfo policy) throws(1: Xception ex),
  RetCode         dialServerConfig(1: DialServerType typ, 2: bool allowed) throws(1: Xception ex),
  RetCode         addDialServer(1: IpAddr ip,2: DialServerType typ) throws(1: Xception ex),
  RetCode         delDialServer(1: IpAddr ip,2: DialServerType typ) throws(1: Xception ex),
  RetCode         addNginxGroup(1: string groupName,2: string policyName, 3: bool dialing) throws(1:Xception ex),
  RetCode         modNginxGroup(1: string groupName,2: string policyName, 3: bool dialing) throws(1:Xception ex),
  RetCode         delNginxGroup(1: string groupName) throws(1:Xception ex),
  RetCode         addNginxServer(1: string groupName,2: DialNginxServer server) throws(1:Xception ex),
  RetCode         modNginxServer(1: string groupName,2: DialNginxServer server) throws(1:Xception ex),
  RetCode         delNginxServer(1: string groupName,2: DialNginxServer server) throws(1:Xception ex),
  HeartBeatState  heartBeat() throws(1: Xception ex)
}

service Collect
{
  void            sendDNSysDeviceQpsOverAlarm(1: i32 qps,2: AlarmStatus status),
  void            sendDNSysDeviceRequestIpOverAlarm(1: list<IpOverAlarm> ipAlarms),
  void            sendDNSysDeviceDomainQpsOverAlarm(1: list<DomainOverAlarm> domainAlarms),
  void            sendDNSysDeviceDnameQpsOverAlarm(1: list<DomainOverAlarm> dnameAlarms),
  void            sendDNSysDeviceFocusDomainAlarm(1: string dname,2: RecordType type,3: list<string>  expResults,4: list<string> curResults,5: AlarmStatus status),
  void            sendDeviceCpuUsageAlarm(1: i32 rate,2: AlarmStatus status),
  void            sendDeviceMemoryUsageAlarm(1: i32 rate,2: AlarmStatus status),
  void            sendDevicePartitionUsageAlarm(1: i32 rate,2: AlarmStatus status),
  void            sendDeviceNicFlowOverAlarm(1: i32 nicin,2: AlarmStatus status),
  void            sendNoneDeviceConnectionAlarm(1: ObjectId devid,2: string devname,3: IpAddr devip, 4: AlarmStatus status),
  void            sendDeviceProcessAlarm(1: string processName,2: AlarmStatus status),
  void            sendDNSysDeviceServfailOverAlarm(1: i32 rate,2: AlarmStatus status),
  void            sendForwardFailAlarm(1: IpAddr server, 2: AlarmStatus status),
  void            sendProxyFailAlarm(1: IpAddr server, 2: AlarmStatus status),
  void            sendRedirectFailAlarm(1: IpAddr server, 2: AlarmStatus status),
  void            sendServiceFailAlarm(1: AlarmStatus status),
  void            sendDiskFailAlarm(1: AlarmStatus status),
  void            sendNginxServerFailAlarm(1: list<NginxServerAlarm> alarms),
  void            sendLogAlarm(1: LogLevel level,2: ModuleType type,3: i32 code,4: string msg),
  void            sendRecordialFailAlarm(1:HealthGroupInfo health,2:list<DialRecordAlarm> dialRecords),
  void		      sendFilterReport(1: FilterReportInfo date),
  void            sendRequestReport(1: RequestReportInfo data),
  void            sendAnswerReport(1: GeneralReportInfo data),
  void            sendZoneRequestReport(1: list<ZoneRequestReport> reports),
  void            sendZoneAnswerReport(1: list<ZoneAnswerReport> reports),
  void            sendXProxyReport(1: list<XproxyReport> reports),
  void            sendXForwardReport(1: list<XforwardReport> reports),
  void            sendRecursiveReport(1: RecursiveReportInfo data),
  void            sendBackGroundReport(1: BackGroundReportInfo data),
  void            sendTopnIpReport(1: list<TopnIpInfo> data),
  void            sendTopnDomainReport(1: list<TopnDomainInfo> data),
  void            sendTopnDnameReport(1: list<TopnDomainInfo> data),
  void            sendHostReport(1: HostInfo data),
  RetCode         systemCommand(1: SysCommand cmdType) throws(1: Xception ex),
  RetCode		  devicePause(1: bool isPause) throws(1: Xception ex),
  RetCode         setAlarmThreshHold(1: AlarmType typ, 2: i32 threshHold) throws(1: Xception ex),
  RetCode         setAlarmEnabled(1: AlarmType typ, 2: bool enabled) throws(1:Xception ex),
  HeartBeatState  heartBeat() throws(1: Xception ex)
}

service Notice
{ 
  RetCode       systemCommand(1: SysCommand cmdType) throws(1: Xception ex),
  RetCode		isPrimary(1: bool isPrimary) throws(1: Xception ex),
  RetCode		addSnmpNotify(1: NotifyServer server,2:bool enabled) throws(1: Xception ex),
  RetCode		delSnmpNotify(1: NotifyServer server) throws(1: Xception ex),
  RetCode		delAllSnmpNotify() throws(1: Xception ex),
  RetCode		addSyslogNotify(1: NotifyServer server,2:bool enabled) throws(1: Xception ex),
  RetCode		delSyslogNotify(1: NotifyServer server) throws(1: Xception ex),
  RetCode		delAllSyslogNotify() throws(1: Xception ex),
  RetCode		setAlarmNotify(1: AlarmType typ, 2: bool notify) throws(1:Xception ex),
  HeartBeatState   heartBeat() throws(1: Xception ex)
}
