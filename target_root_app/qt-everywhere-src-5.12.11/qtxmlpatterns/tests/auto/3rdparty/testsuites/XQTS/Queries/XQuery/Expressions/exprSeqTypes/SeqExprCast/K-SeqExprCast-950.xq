(:*******************************************************:)
(: Test: K-SeqExprCast-950                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:gYearMonth constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:gYearMonth(
      "1999-11"
    ,
                                                     
      "1999-11"
    )