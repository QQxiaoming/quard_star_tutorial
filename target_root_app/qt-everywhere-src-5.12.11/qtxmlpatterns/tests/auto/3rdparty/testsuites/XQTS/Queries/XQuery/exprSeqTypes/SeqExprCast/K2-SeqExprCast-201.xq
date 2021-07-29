(:*******************************************************:)
(: Test: K2-SeqExprCast-201                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: When casting to xs:QName, the declared default element namespace is considered. :)
(:*******************************************************:)
declare default element namespace "http://example.com/defelementns";
        namespace-uri-from-QName(xs:QName("ncname"))