(:*******************************************************:)
(: Test: K-SeqExprCast-1054                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:gMonthDay constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:gMonthDay(
      "--11-13"
    ,
                                                     
      "--11-13"
    )