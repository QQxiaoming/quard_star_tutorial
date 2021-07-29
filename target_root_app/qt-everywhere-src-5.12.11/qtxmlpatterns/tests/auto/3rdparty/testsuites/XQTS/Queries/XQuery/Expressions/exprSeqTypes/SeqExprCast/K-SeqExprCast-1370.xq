(:*******************************************************:)
(: Test: K-SeqExprCast-1370                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "http://www.example.com/an/arbitrary/URI.ext" . :)
(:*******************************************************:)
xs:anyURI(xs:untypedAtomic(
      "http://www.example.com/an/arbitrary/URI.ext"
    )) eq xs:anyURI("http://www.example.com/an/arbitrary/URI.ext")