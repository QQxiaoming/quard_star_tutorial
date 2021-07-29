(:*******************************************************:)
(: Test: K-SeqExprCast-586                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The xs:integer constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:integer(
      "6789"
    ,
                                                     
      "6789"
    )