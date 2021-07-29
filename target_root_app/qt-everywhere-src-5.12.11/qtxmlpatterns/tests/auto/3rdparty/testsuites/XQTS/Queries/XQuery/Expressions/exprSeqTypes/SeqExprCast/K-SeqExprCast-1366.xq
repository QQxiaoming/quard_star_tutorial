(:*******************************************************:)
(: Test: K-SeqExprCast-1366                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:anyURI constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:anyURI(
      "http://www.example.com/an/arbitrary/URI.ext"
    ,
                                                     
      "http://www.example.com/an/arbitrary/URI.ext"
    )