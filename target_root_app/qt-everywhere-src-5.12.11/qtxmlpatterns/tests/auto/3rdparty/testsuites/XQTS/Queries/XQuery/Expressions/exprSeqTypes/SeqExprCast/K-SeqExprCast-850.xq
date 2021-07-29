(:*******************************************************:)
(: Test: K-SeqExprCast-850                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "03:20:00-05:00" . :)
(:*******************************************************:)
xs:time(xs:untypedAtomic(
      "03:20:00-05:00"
    )) eq xs:time("03:20:00-05:00")