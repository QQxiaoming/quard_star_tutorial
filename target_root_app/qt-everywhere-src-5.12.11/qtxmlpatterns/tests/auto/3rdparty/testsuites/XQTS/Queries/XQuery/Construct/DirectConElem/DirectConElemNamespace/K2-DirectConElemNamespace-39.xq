(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-39                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Combine an empty default namespace, with a full namespace. :)
(:*******************************************************:)
declare namespace p = "http://example.com/";
<p:e xmlns=""/>,
count(in-scope-prefixes(<p:e xmlns=""/>))
