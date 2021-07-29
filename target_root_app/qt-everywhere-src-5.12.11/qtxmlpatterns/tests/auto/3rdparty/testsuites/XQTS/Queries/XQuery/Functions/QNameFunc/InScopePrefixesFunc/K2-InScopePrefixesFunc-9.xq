(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-9                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that a prefix declaration is in scope in a sub-node. :)
(:*******************************************************:)
for $i in fn:in-scope-prefixes(<e xmlns:p="http://example.com"
xmlns:a="http://example.com">
<b/>
</e>/b)
order by $i
return $i