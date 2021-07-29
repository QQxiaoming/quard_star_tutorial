(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-27                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check in scope namespaces.                   :)
(:*******************************************************:)
declare namespace a = "http://example.com/1";
declare namespace b = "http://example.com/2";
<e a:n1="content" b:n1="content"/>/(for $i in in-scope-prefixes(.)
                                    order by $i
                                    return $i)