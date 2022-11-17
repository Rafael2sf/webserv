 ________________________________________________
|             HEADER             |  EXPLANATION  |
|________________________________|_______________|
|       STANDARD REQUEST HEADERS                 |
|________________________________________________|
| A-IM                           | 
| Accept                         |
| Accept-Charset                 |
| Accept-Datetime                |
| Accept-Encoding                |
| Accept-Language                |
| Access-Control-Request-Method  |
| Access-Control-Request-Headers |
| Authorization                  |
| Cache-Control                  |
| Connection                     |
| Content-Encoding               |
| Content-Length                 |
| Content-MDS                    |
| Content-Type                   |
| Cookie                         |
| Date                           |
| Expect                         |
| Forwarded                      |
| From                           |
| Host                           |
| HTTP2-Settings                 |
| If-Match                       |
| If-Modified-Since              |
| If-None-Match                  |
| If-Range                       |
| If-Unmodified-Since            |
| Max-Forwards                   |
| Origin                         |
| Pragma                         |
| Prefer                         |
| Proxy-Authorization            |
| Range                          |
| Referer                        |
| TE                             |
| Trailer                        |
| Transfer-Encoding              |
| User-Agent                     |
| Upgrade                        |
| Via                            |
| Warning                        |
|________________________________|_____________________|
|             NON-STANDARD REQUEST HEADERS             |
|______________________________________________________|
| Upgrade-Insecure-Requests      |
| X-Requested-With               |
| DNT                            |
| X-Forwarded-For                |
| X-Forwarded-Host               |
| X-Forwarded-Proto              |
| Front-End-Https                |
| X-Http-Method-Override         |
| X-ATT-DeviceId                 |
| X-Wap-Profile                  |
| Proxy-Connection               |
| X-UIDH                         |
| X-Csrf-Token                   |
| X-Request-ID                   |
| X-Correlation-ID               |
| Correlation-ID                 |
| Save-Data                      |
|________________________________|_____________________|


Andre: Accept-Charset, Connection, Cookies(if bonus).

Old subject: Accept-Charsets, Accept-Language, Allow, Authorization, Content-Language, Content-Length, Content-Location, Content-Type, Date, Host, Last-Modified, Location, Referer, 
Retry-After, Server, Transfer-Encoding, User-Agent, WWW-Authenticate

Firefox request: Host, User-Agent, Accept, Accept-Language, Accept-Encoding, Connection, Referer(not in site GET), 
		Sec-Fetch-Dest, Sec-Fetch-Mode, Sec-Fetch-Site,
		Sec-Fetch-User(not in favicon GET), Upgrade-Insecure-Request:(not in site GET)
StackOverflow Response Advice: Date, Server, Content-Type, Content-Length, Transfer-Encoding