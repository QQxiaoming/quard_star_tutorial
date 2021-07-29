(:*******************************************************:)
(: Test: K-SeqExprCast-794                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:dateTime constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:dateTime(
      "2002-10-10T12:00:00-05:00"
    ,
                                                     
      "2002-10-10T12:00:00-05:00"
    )