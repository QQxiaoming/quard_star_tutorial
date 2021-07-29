(:*******************************************************:)
(: Test: K-SeqExprCast-1266                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "aaaa" . :)
(:*******************************************************:)
xs:base64Binary(xs:untypedAtomic(
      "aaaa"
    )) eq xs:base64Binary("aaaa")