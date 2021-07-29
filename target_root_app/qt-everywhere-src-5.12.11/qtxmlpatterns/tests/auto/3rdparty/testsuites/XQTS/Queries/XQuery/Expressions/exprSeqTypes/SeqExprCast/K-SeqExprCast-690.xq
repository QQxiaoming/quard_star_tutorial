(:*******************************************************:)
(: Test: K-SeqExprCast-690                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:yearMonthDuration constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:yearMonthDuration(
      "P1Y12M"
    ,
                                                     
      "P1Y12M"
    )