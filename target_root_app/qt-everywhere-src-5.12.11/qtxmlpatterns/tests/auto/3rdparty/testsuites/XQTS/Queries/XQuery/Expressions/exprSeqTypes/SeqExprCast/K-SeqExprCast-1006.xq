(:*******************************************************:)
(: Test: K-SeqExprCast-1006                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "1999" . :)
(:*******************************************************:)
xs:gYear(xs:untypedAtomic(
      "1999"
    )) eq xs:gYear("1999")