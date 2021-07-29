(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-16                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that a default namespace declaration attribute on a direct element constructor correctly affect a computed child constructor. :)
(:*******************************************************:)
let $i := <e xmlns="http://example.com/">
    {element a {()}}
</e>
return (count(in-scope-prefixes($i)), count(in-scope-prefixes(exactly-one($i/*))))