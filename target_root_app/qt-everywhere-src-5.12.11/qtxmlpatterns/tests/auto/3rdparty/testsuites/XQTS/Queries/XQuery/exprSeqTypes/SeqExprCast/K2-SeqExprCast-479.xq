(:*******************************************************:)
(: Test: K2-SeqExprCast-479                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast to xs:QName, using a namespace from an element constructor. :)
(:*******************************************************:)
<e xmlns:prefix="http://example.com/">
    {
        xs:QName("prefix:localName")
    }
</e>