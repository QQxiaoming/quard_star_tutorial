(:*******************************************************:)
(: Test: K2-ComputeConAttr-50                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Duplicate attributes whose names are non-prefixed while non-empty namespace URIs. :)
(:*******************************************************:)
<e>
    {
        attribute {QName("http://example.com/", "attr")} {()},
        attribute {QName("http://example.com/", "attr")} {()}
    }
</e>