(:*******************************************************:)
(: Test: K-SeqExprCast-1110                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "---03" . :)
(:*******************************************************:)
xs:gDay(xs:untypedAtomic(
      "---03"
    )) eq xs:gDay("---03")