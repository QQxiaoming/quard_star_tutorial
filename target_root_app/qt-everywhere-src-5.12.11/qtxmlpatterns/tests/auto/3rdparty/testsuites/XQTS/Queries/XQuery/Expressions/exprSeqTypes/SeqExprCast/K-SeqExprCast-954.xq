(:*******************************************************:)
(: Test: K-SeqExprCast-954                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "1999-11" . :)
(:*******************************************************:)
xs:gYearMonth(xs:untypedAtomic(
      "1999-11"
    )) eq xs:gYearMonth("1999-11")