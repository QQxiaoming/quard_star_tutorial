(:*******************************************************:)
(: Test: K-SeqExprCast-694                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "P1Y12M" . :)
(:*******************************************************:)
xs:yearMonthDuration(xs:untypedAtomic(
      "P1Y12M"
    )) eq xs:yearMonthDuration("P1Y12M")