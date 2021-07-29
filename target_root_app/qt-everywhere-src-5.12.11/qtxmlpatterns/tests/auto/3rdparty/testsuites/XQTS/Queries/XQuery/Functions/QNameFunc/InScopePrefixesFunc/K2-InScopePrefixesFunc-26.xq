(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-26                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: .                                            :)
(:*******************************************************:)
let $i := <e>
{
    attribute {QName("http://example.com/", "prefix:attributeName")} {()}
}
</e>
return ($i, for $ps in in-scope-prefixes($i) order by $ps return $ps)