(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-18                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure prefix namespace declarations are counted as in-scope bindings. :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
<e xmlns:p="http://example.com/">
        {
        count(in-scope-prefixes(<e/>)),
        count(in-scope-prefixes(element e {()}))
        }
</e>