(:*******************************************************:)
(: Test: K-SeqExprCast-1162                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "--11" . :)
(:*******************************************************:)
xs:gMonth(xs:untypedAtomic(
      "--11"
    )) eq xs:gMonth("--11")