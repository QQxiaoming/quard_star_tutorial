(:*******************************************************:)
(: Test: K-SeqExprCast-1058                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "--11-13" . :)
(:*******************************************************:)
xs:gMonthDay(xs:untypedAtomic(
      "--11-13"
    )) eq xs:gMonthDay("--11-13")