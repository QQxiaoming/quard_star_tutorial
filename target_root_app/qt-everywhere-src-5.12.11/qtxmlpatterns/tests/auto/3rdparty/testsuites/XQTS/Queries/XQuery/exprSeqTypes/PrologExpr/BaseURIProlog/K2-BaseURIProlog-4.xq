(:*******************************************************:)
(: Test: K2-BaseURIProlog-4                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The static base-uri must be absolute. Since the declaration supplies a relative URI, an implementation may fail with computing an absolute URI, hence XPST0001 is allowed. The test checks that the static base-uri is absolute. :)
(:*******************************************************:)
declare base-uri "abc";
declare function local:isAbsolute($uri as xs:string?) as xs:boolean
{
    fn:matches($uri, "[a-zA-Z0-9\-.]*:/")
};
local:isAbsolute(fn:static-base-uri())